#pragma once

#include <string>
#include <vector>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Timestamp.h>

#include "../common/http_json_utils.h"
#include "../storage/postgres_storage.h"
#include "../common/request_counter.h"

namespace handlers
{

    class GetUserLoansHandler : public Poco::Net::HTTPRequestHandler
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
                const std::string prefix = "/api/v1/users/";
                const std::string suffix = "/loans";

                if (uri.rfind(prefix, 0) != 0 || uri.size() <= prefix.size() + suffix.size())
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid user loans path");
                    markError();
                    writeMetrics(start);
                    return;
                }

                const std::size_t suffixPos = uri.find(suffix);
                if (suffixPos == std::string::npos)
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid user loans path");
                    markError();
                    writeMetrics(start);
                    return;
                }

                const std::string idPart = uri.substr(prefix.size(), suffixPos - prefix.size());
                const int userId = std::stoi(idPart);

                if (!PostgresStorage::instance().userExists(userId))
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND, "user not found");
                    markError();
                    writeMetrics(start);
                    return;
                }

                const std::vector<LoanDto> loans = PostgresStorage::instance().getLoansByUserId(userId);

                Poco::JSON::Array items;
                for (const auto &loan : loans)
                {
                    Poco::JSON::Object::Ptr loanJson = new Poco::JSON::Object();
                    loanJson->set("id", loan.id);
                    loanJson->set("userId", loan.userId);
                    loanJson->set("bookId", loan.bookId);
                    loanJson->set("issuedAt", loan.issuedAt);
                    loanJson->set("returnedAt", loan.returnedAt);
                    loanJson->set("returned", loan.returned);
                    items.add(loanJson);
                }

                Poco::JSON::Object result;
                result.set("items", items);
                result.set("count", static_cast<int>(loans.size()));

                sendJson(response, Poco::Net::HTTPResponse::HTTP_OK, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("GetUserLoansHandler failed: %s", ex.what());
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