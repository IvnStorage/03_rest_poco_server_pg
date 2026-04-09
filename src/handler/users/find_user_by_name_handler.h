#pragma once

#include <string>
#include <vector>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/URI.h>
#include <Poco/Timestamp.h>

#include "../common/http_json_utils.h"
#include "../storage/postgres_storage.h"
#include "../common/request_counter.h"

namespace handlers
{

    class FindUserByNameHandler : public Poco::Net::HTTPRequestHandler
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
                Poco::URI uri(request.getURI());
                const auto queryParameters = uri.getQueryParameters();

                std::string firstName;
                std::string lastName;

                for (const auto &[key, value] : queryParameters)
                {
                    if (key == "firstName")
                    {
                        firstName = value;
                    }
                    else if (key == "lastName")
                    {
                        lastName = value;
                    }
                }

                std::vector<UserDto> users = PostgresStorage::instance().findUsersByNameMask(firstName, lastName);

                Poco::JSON::Array items;
                for (const auto &user : users)
                {
                    Poco::JSON::Object::Ptr userJson = new Poco::JSON::Object();
                    userJson->set("id", user.id);
                    userJson->set("login", user.login);
                    userJson->set("firstName", user.firstName);
                    userJson->set("lastName", user.lastName);
                    items.add(userJson);
                }

                Poco::JSON::Object result;
                result.set("items", items);
                result.set("count", static_cast<int>(users.size()));

                sendJson(response, Poco::Net::HTTPResponse::HTTP_OK, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("FindUserByNameHandler failed: %s", ex.what());
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