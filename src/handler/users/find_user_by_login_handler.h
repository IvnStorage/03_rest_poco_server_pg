#pragma once

#include <string>

#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Timestamp.h>

#include "../common/http_json_utils.h"
#include "../storage/postgres_storage.h"
#include "../common/request_counter.h"

namespace handlers
{

    class FindUserByLoginHandler : public Poco::Net::HTTPRequestHandler
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
                const std::string prefix = "/api/v1/users/by-login/";

                if (uri.rfind(prefix, 0) != 0 || uri.size() <= prefix.size())
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "login is required");
                    markError();
                    writeMetrics(start);
                    return;
                }

                const std::string login = uri.substr(prefix.size());

                UserDto user;
                if (!PostgresStorage::instance().findUserByLogin(login, user))
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND, "user not found");
                    markError();
                    writeMetrics(start);
                    return;
                }

                Poco::JSON::Object result;
                result.set("id", user.id);
                result.set("login", user.login);
                result.set("firstName", user.firstName);
                result.set("lastName", user.lastName);

                sendJson(response, Poco::Net::HTTPResponse::HTTP_OK, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("FindUserByLoginHandler failed: %s", ex.what());
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