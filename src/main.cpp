#include <iostream>
#include <string>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Environment.h>
#include <Poco/Logger.h>
#include <Poco/NumberParser.h>
#include <Poco/Util/ServerApplication.h>

#include "handler/auth/auth_config.h"
#include "handler/common/router_factory.h"
#include "handler/common/request_counter.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

namespace
{
    void configureLogging()
    {
        std::string level = Environment::get("LOG_LEVEL", "information");

        Message::Priority prio = Message::PRIO_INFORMATION;
        if (level == "trace")
            prio = Message::PRIO_TRACE;
        else if (level == "debug")
            prio = Message::PRIO_DEBUG;
        else if (level == "information" || level == "info")
            prio = Message::PRIO_INFORMATION;
        else if (level == "notice")
            prio = Message::PRIO_NOTICE;
        else if (level == "warning" || level == "warn")
            prio = Message::PRIO_WARNING;
        else if (level == "error")
            prio = Message::PRIO_ERROR;
        else if (level == "critical")
            prio = Message::PRIO_CRITICAL;
        else if (level == "fatal")
            prio = Message::PRIO_FATAL;
        else if (level == "none")
            prio = static_cast<Message::Priority>(Message::PRIO_FATAL + 1);
        Logger::root().setLevel(prio);
    }
<<<<<<< HEAD
=======

>>>>>>> 4d6bb9b039b9a87671d30bb4d0ed499d549249b2
}

namespace handlers
{
    DummyCounter *g_httpRequests = nullptr;
    DummyCounter *g_httpErrors = nullptr;
    DummyHistogram *g_httpDuration = nullptr;
    std::string g_jwtSecret;
<<<<<<< HEAD
=======

>>>>>>> 4d6bb9b039b9a87671d30bb4d0ed499d549249b2
}

class ServerApp : public ServerApplication
{
protected:
    int main(const std::vector<std::string> &) override
    {
        configureLogging();
        auto &logger = Logger::get("Server");

        unsigned short port = 8080;
        if (Environment::has("PORT"))
        {
            try
            {
                port = static_cast<unsigned short>(NumberParser::parse(Environment::get("PORT")));
            }
            catch (const Exception &e)
            {
                logger.warning("Invalid PORT, using default 8080: %s", e.displayText());
            }
        }

        static handlers::DummyCounter httpRequests;
        static handlers::DummyCounter httpErrors;
        static handlers::DummyHistogram httpDuration;

        handlers::g_httpRequests = &httpRequests;
        handlers::g_httpErrors = &httpErrors;
        handlers::g_httpDuration = &httpDuration;

        handlers::g_jwtSecret = Environment::get("JWT_SECRET", "");

        logger.information("Starting server on port %hu", port);

        ServerSocket svs(port);
        HTTPServer srv(new handlers::RouterFactory(), svs, new HTTPServerParams);
        srv.start();

        logger.information("Server started. Endpoints: POST /api/v1/auth/register, POST /api/v1/auth/login, POST /api/v1/users, GET /api/v1/users/by-login/{login}, GET /api/v1/users/search, POST /api/v1/books, GET /api/v1/books/search, POST /api/v1/loans, GET /api/v1/users/{userId}/loans, PATCH /api/v1/loans/{loanId}/return, GET /swagger.yaml");
        waitForTerminationRequest();
        srv.stop();

        return Application::EXIT_OK;
    }
};

POCO_SERVER_MAIN(ServerApp)