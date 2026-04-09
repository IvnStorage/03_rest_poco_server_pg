#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Timestamp.h>

#include "../common/http_json_utils.h"
#include "../common/request_counter.h"

namespace handlers
{

    class NotFoundHandler : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest &,
                           Poco::Net::HTTPServerResponse &response) override
        {
            Poco::Timestamp start;

            if (g_httpRequests)
            {
                g_httpRequests->inc();
            }

            sendError(
                response,
                Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                "route not found");

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
        }
    };

}