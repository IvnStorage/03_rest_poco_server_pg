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

    class CreateUserHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest &request,
                           Poco::Net::HTTPServerResponse &response) override
        {
            Poco::Timestamp start;

            if (g_httpRequests)
            {
                g_httpRequests->inc();
            }

            try
            {
                std::stringstream bodyStream;
                bodyStream << request.stream().rdbuf();

                Poco::JSON::Parser parser;
                Poco::Dynamic::Var parsed = parser.parse(bodyStream.str());
                Poco::JSON::Object::Ptr json = parsed.extract<Poco::JSON::Object::Ptr>();

                const std::string login = json->optValue<std::string>("login", "");
                const std::string password = json->optValue<std::string>("password", "");
                const std::string firstName = json->optValue<std::string>("firstName", "");
                const std::string lastName = json->optValue<std::string>("lastName", "");

                if (login.empty() || password.empty() || firstName.empty() || lastName.empty())
                {
                    sendError(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        "login, password, firstName and lastName are required");
                    markError();
                    writeMetrics(start);
                    return;
                }

                UserDto existingUser;
                if (PostgresStorage::instance().findUserByLogin(login, existingUser))
                {
                    sendError(
                        response,
                        Poco::Net::HTTPResponse::HTTP_CONFLICT,
                        "user already exists");
                    markError();
                    writeMetrics(start);
                    return;
                }

                UserDto createdUser;
                const bool created = PostgresStorage::instance().createUser(
                    login,
                    password,
                    firstName,
                    lastName,
                    createdUser);

                if (!created)
                {
                    sendError(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        "failed to create user");
                    markError();
                    writeMetrics(start);
                    return;
                }

                Poco::JSON::Object result;
                result.set("id", createdUser.id);
                result.set("login", createdUser.login);
                result.set("firstName", createdUser.firstName);
                result.set("lastName", createdUser.lastName);

                sendJson(response, Poco::Net::HTTPResponse::HTTP_CREATED, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("CreateUserHandler failed: %s", ex.what());
                sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid json");
                markError();
                writeMetrics(start);
            }
        }

    private:
        void markError()
        {
            if (g_httpErrors)
            {
                g_httpErrors->inc();
            }
        }

        void writeMetrics(const Poco::Timestamp &start)
        {
            Poco::Timespan duration = Poco::Timestamp() - start;
            const double seconds =
                static_cast<double>(duration.totalMicroseconds()) / 1000000.0;

            if (g_httpDuration)
            {
                g_httpDuration->observe(seconds);
            }
        }
    };

}