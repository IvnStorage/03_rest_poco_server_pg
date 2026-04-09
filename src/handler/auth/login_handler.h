#pragma once

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/Token.h>
#include <Poco/Logger.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Timestamp.h>

#include "auth_config.h"
#include "../storage/postgres_storage.h"
#include "../common/request_counter.h"

namespace handlers
{

    class LoginHandler : public Poco::Net::HTTPRequestHandler
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

            response.setContentType("application/json");

            try
            {
                std::stringstream bodyStream;
                bodyStream << request.stream().rdbuf();
                const std::string body = bodyStream.str();

                Poco::JSON::Parser parser;
                Poco::Dynamic::Var parsed = parser.parse(body);
                Poco::JSON::Object::Ptr json = parsed.extract<Poco::JSON::Object::Ptr>();

                const std::string login = json->optValue<std::string>("login", "");
                const std::string password = json->optValue<std::string>("password", "");

                if (login.empty() || password.empty())
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                    Poco::JSON::Object errorJson;
                    errorJson.set("error", "login and password are required");
                    std::ostream &out = response.send();
                    Poco::JSON::Stringifier::stringify(errorJson, out);
                    if (g_httpErrors)
                    {
                        g_httpErrors->inc();
                    }
                    writeMetrics(start);
                    return;
                }

                if (g_jwtSecret.empty())
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                    Poco::JSON::Object errorJson;
                    errorJson.set("error", "JWT_SECRET not configured");
                    std::ostream &out = response.send();
                    Poco::JSON::Stringifier::stringify(errorJson, out);
                    if (g_httpErrors)
                    {
                        g_httpErrors->inc();
                    }
                    writeMetrics(start);
                    return;
                }

                UserDto user;
                const bool valid = PostgresStorage::instance().validateUserCredentials(login, password, user);

                if (!valid)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
                    Poco::JSON::Object errorJson;
                    errorJson.set("error", "invalid login or password");
                    std::ostream &out = response.send();
                    Poco::JSON::Stringifier::stringify(errorJson, out);
                    if (g_httpErrors)
                    {
                        g_httpErrors->inc();
                    }
                    writeMetrics(start);
                    return;
                }

                Poco::JWT::Token token;
                token.setType("JWT");
                token.setSubject(user.login);
                token.setIssuedAt(Poco::Timestamp());
                token.payload().set("userId", user.id);
                token.payload().set("login", user.login);

                Poco::JWT::Signer signer(g_jwtSecret);
                const std::string jwt = signer.sign(token, Poco::JWT::Signer::ALGO_HS256);

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

                Poco::JSON::Object result;
                result.set("token", jwt);

                std::ostream &out = response.send();
                Poco::JSON::Stringifier::stringify(result, out);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                Poco::JSON::Object errorJson;
                errorJson.set("error", "invalid json");
                std::ostream &out = response.send();
                Poco::JSON::Stringifier::stringify(errorJson, out);

                if (g_httpErrors)
                {
                    g_httpErrors->inc();
                }

                Poco::Logger::get("Server").warning("LoginHandler failed: %s", ex.what());
                writeMetrics(start);
            }
        }

    private:
        void writeMetrics(const Poco::Timestamp &start)
        {
            Poco::Timespan duration = Poco::Timestamp() - start;
            const double seconds = static_cast<double>(duration.totalMicroseconds()) / 1000000.0;
            if (g_httpDuration)
            {
                g_httpDuration->observe(seconds);
            }
        }
    };

}