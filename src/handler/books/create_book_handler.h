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

    class CreateBookHandler : public Poco::Net::HTTPRequestHandler
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
                std::stringstream bodyStream;
                bodyStream << request.stream().rdbuf();

                Poco::JSON::Parser parser;
                Poco::Dynamic::Var parsed = parser.parse(bodyStream.str());
                Poco::JSON::Object::Ptr json = parsed.extract<Poco::JSON::Object::Ptr>();

                const std::string title = json->optValue<std::string>("title", "");
                const std::string author = json->optValue<std::string>("author", "");
                const int year = json->optValue<int>("year", 0);

                if (title.empty() || author.empty())
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                              "title and author are required");
                    markError();
                    writeMetrics(start);
                    return;
                }

                BookDto createdBook;
                if (!PostgresStorage::instance().addBook(title, author, year, createdBook))
                {
                    sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "failed to create book");
                    markError();
                    writeMetrics(start);
                    return;
                }

                Poco::JSON::Object result;
                result.set("id", createdBook.id);
                result.set("title", createdBook.title);
                result.set("author", createdBook.author);
                result.set("year", createdBook.year);
                result.set("available", createdBook.available);

                sendJson(response, Poco::Net::HTTPResponse::HTTP_CREATED, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("CreateBookHandler failed: %s", ex.what());
                sendError(response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "invalid json");
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