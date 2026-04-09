#pragma once

#include <ostream>
#include <string>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace handlers
{

    inline void sendJson(Poco::Net::HTTPServerResponse &response,
                         Poco::Net::HTTPResponse::HTTPStatus status,
                         const Poco::JSON::Object &object)
    {
        response.setStatus(status);
        response.setContentType("application/json");
        std::ostream &out = response.send();
        Poco::JSON::Stringifier::stringify(object, out);
    }

    inline void sendError(Poco::Net::HTTPServerResponse &response,
                          Poco::Net::HTTPResponse::HTTPStatus status,
                          const std::string &errorMessage)
    {
        Poco::JSON::Object errorJson;
        errorJson.set("error", errorMessage);
        sendJson(response, status, errorJson);
    }

}