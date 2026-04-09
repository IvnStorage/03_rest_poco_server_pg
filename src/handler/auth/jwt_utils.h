#pragma once

#include <string>

#include <Poco/JWT/Signer.h>
#include <Poco/JWT/Token.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "auth_config.h"

namespace handlers
{

    inline bool extractBearerToken(const Poco::Net::HTTPServerRequest &request,
                                   std::string &token)
    {
        if (!request.has("Authorization"))
        {
            return false;
        }

        const std::string header = request.get("Authorization", "");
        const std::string prefix = "Bearer ";

        if (header.rfind(prefix, 0) != 0)
        {
            return false;
        }

        token = header.substr(prefix.size());
        return !token.empty();
    }

    inline bool verifyJwtAndGetPayload(const Poco::Net::HTTPServerRequest &request,
                                       Poco::JSON::Object::Ptr &payload)
    {
        if (g_jwtSecret.empty())
        {
            return false;
        }

        std::string tokenString;
        if (!extractBearerToken(request, tokenString))
        {
            return false;
        }

        try
        {
            Poco::JWT::Signer signer(g_jwtSecret);
            Poco::JWT::Token token = signer.verify(tokenString);

            payload = new Poco::JSON::Object();

            if (token.payload().has("userId"))
            {
                payload->set("userId", token.payload().getValue<int>("userId"));
            }

            if (token.payload().has("login"))
            {
                payload->set("login", token.payload().getValue<std::string>("login"));
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

}