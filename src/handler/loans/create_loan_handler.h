#pragma once

#include <sstream>
#include <string>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Timestamp.h>

#include "../common/http_json_utils.h"
#include "../storage/postgres_storage.h"
#include "../common/request_counter.h"

namespace handlers
{

    class CreateLoanHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest &request,
                           Poco::Net::HTTPServerResponse &response) override
        {
            Poco::Timestamp start;
            if (g_httpRequests)
                g_httpRequests->inc();

            try
            {
                std::stringstream bodyStream;
                bodyStream << request.stream().rdbuf();

                Poco::JSON::Parser parser;
                Poco::Dynamic::Var parsed = parser.parse(bodyStream.str());
                Poco::JSON::Object::Ptr json = parsed.extract<Poco::JSON::Object::Ptr>();

                const int userId = json->optValue<int>("userId", 0);
                const int bookId = json->optValue<int>("bookId", 0);
                const std::string issuedAt = json->optValue<std::string>("issuedAt", "");

                if (userId <= 0 || bookId <= 0 || issuedAt.empty())
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                              "userId, bookId and issuedAt are required");
                    markError();
                    writeMetrics(start);
                    return;
                }

                LoanDto createdLoan;
                std::string errorMessage;
                if (!PostgresStorage::instance().createLoan(userId, bookId, issuedAt, createdLoan, errorMessage))
                {
                    if (errorMessage == "user not found" || errorMessage == "book not found")
                    {
                        sendError(response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND, errorMessage);
                    }
                    else
                    {
                        sendError(response, Poco::Net::HTTPResponse::HTTP_CONFLICT, errorMessage);
                    }
                    markError();
                    writeMetrics(start);
                    return;
                }

                Poco::JSON::Object result;
                result.set("id", createdLoan.id);
                result.set("userId", createdLoan.userId);
                result.set("bookId", createdLoan.bookId);
                result.set("issuedAt", createdLoan.issuedAt);
                result.set("returnedAt", createdLoan.returnedAt);
                result.set("returned", createdLoan.returned);

                sendJson(response, Poco::Net::HTTPResponse::HTTP_CREATED, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("CreateLoanHandler failed: %s", ex.what());
                sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid json");
                markError();
                writeMetrics(start);
            }
        }

    private:
        void markError()
        {
            if (g_httpErrors)
                g_httpErrors->inc();
        }

        void writeMetrics(const Poco::Timestamp &start)
        {
            Poco::Timespan duration = Poco::Timestamp() - start;
            const double seconds = static_cast<double>(duration.totalMicroseconds()) / 1000000.0;
            if (g_httpDuration)
                g_httpDuration->observe(seconds);
        }
    };

}