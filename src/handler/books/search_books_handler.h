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

    class SearchBooksHandler : public Poco::Net::HTTPRequestHandler
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

                std::string title;
                std::string author;

                for (const auto &[key, value] : queryParameters)
                {
                    if (key == "title")
                    {
                        title = value;
                    }
                    else if (key == "author")
                    {
                        author = value;
                    }
                }

                std::vector<BookDto> books;
                if (!title.empty())
                {
                    books = PostgresStorage::instance().findBooksByTitle(title);
                }
                else
                {
                    books = PostgresStorage::instance().findBooksByAuthor(author);
                }

                Poco::JSON::Array items;
                for (const auto &book : books)
                {
                    Poco::JSON::Object::Ptr bookJson = new Poco::JSON::Object();
                    bookJson->set("id", book.id);
                    bookJson->set("title", book.title);
                    bookJson->set("author", book.author);
                    bookJson->set("year", book.year);
                    bookJson->set("available", book.available);
                    items.add(bookJson);
                }

                Poco::JSON::Object result;
                result.set("items", items);
                result.set("count", static_cast<int>(books.size()));

                sendJson(response, Poco::Net::HTTPResponse::HTTP_OK, result);
                writeMetrics(start);
            }
            catch (const std::exception &ex)
            {
                Poco::Logger::get("Server").warning("SearchBooksHandler failed: %s", ex.what());
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