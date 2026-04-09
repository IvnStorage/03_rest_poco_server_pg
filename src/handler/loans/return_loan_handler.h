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

    class ReturnLoanHandler : public Poco::Net::HTTPRequestHandler
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
                const std::string uri = request.getURI();
                const std::string prefix = "/api/v1/loans/";
                const std::string suffix = "/return";

                if (uri.rfind(prefix, 0) != 0)
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid return path");
                    markError();
                    writeMetrics(start);
                    return;
                }

                const std::size_t suffixPos = uri.find(suffix);
                if (suffixPos == std::string::npos)
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid return path");
                    markError();
                    writeMetrics(start);
                    return;
                }

                const std::string idPart = uri.substr(prefix.size(), suffixPos - prefix.size());
                const int loanId = std::stoi(idPart);

                std::stringstream bodyStream;
                bodyStream << request.stream().rdbuf();

                Poco::JSON::Parser parser;
                Poco::Dynamic::Var parsed = parser.parse(bodyStream.str());
                Poco::JSON::Object::Ptr json = parsed.extract<Poco::JSON::Object::Ptr>();

                const std::string returnedAt = json->optValue<std::string>("returnedAt", "");
                if (returnedAt.empty())
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "returnedAt is required");
                    markError();
                    writeMetrics(start);
                    return;
                }

                LoanDto updatedLoan;
                std::string errorMessage;
                if (!PostgresStorage::instance().returnBook(loanId, returnedAt, updatedLoan, errorMessage))
                {
                    if (errorMessage == "loan not found")
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
                result.set("id", updatedLoan.id);
                result.set("userId", updatedLoan.userId);
                result.set("bookId", updatedLoan.bookId);
                result.set("issuedAt", updatedLoan.issuedAt);
                result.set("returnedAt", updatedLoan.returnedAt);
                result.set("returned", updatedLoan.returned);

                sendJson(response, Poco::Net::HTTPResponse::HTTP_OK, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("ReturnLoanHandler failed: %s", ex.what());
                sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid request");
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