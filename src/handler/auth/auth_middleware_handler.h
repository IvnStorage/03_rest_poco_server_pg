#pragma once

#include <memory>

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Timestamp.h>

#include "../common/http_json_utils.h"
#include "jwt_utils.h"
#include "../common/request_counter.h"

namespace handlers
{

    class AuthMiddlewareHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        explicit AuthMiddlewareHandler(Poco::Net::HTTPRequestHandler *nextHandler)
            : _next(nextHandler)
        {
        }

        void handleRequest(Poco::Net::HTTPServerRequest &request,
                           Poco::Net::HTTPServerResponse &response) override
        {
            Poco::Timestamp start;

            Poco::JSON::Object::Ptr payload;
            if (!verifyJwtAndGetPayload(request, payload))
            {
                if (g_httpRequests)
                {
                    g_httpRequests->inc();
                }

                sendError(
                    response,
                    Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED,
                    "missing or invalid bearer token");

                if (g_httpErrors)
                {
                    g_httpErrors->inc();
                }

                Poco::Timespan duration = Poco::Timestamp() - start;
                const double seconds =
                    static_cast<double>(duration.totalMicroseconds()) / 1000000.0;

                if (g_httpDuration)
                {
                    g_httpDuration->observe(seconds);
                }

                return;
            }

            _next->handleRequest(request, response);
        }

    private:
        std::unique_ptr<Poco::Net::HTTPRequestHandler> _next;
    };

}