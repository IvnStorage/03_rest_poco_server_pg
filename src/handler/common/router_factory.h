#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "../auth/auth_middleware_handler.h"
#include "../auth/login_handler.h"
#include "../auth/register_handler.h"

#include "../users/create_user_handler.h"
#include "../users/find_user_by_login_handler.h"
#include "../users/find_user_by_name_handler.h"

#include "../books/create_book_handler.h"
#include "../books/search_books_handler.h"

#include "../loans/create_loan_handler.h"
#include "../loans/get_user_loans_handler.h"
#include "../loans/return_loan_handler.h"

#include "../system/not_found_handler.h"
#include "../system/swagger_handler.h"

namespace handlers
{

    class RouterFactory : public Poco::Net::HTTPRequestHandlerFactory
    {
    public:
        Poco::Net::HTTPRequestHandler *createRequestHandler(
            const Poco::Net::HTTPServerRequest &request) override
        {
            const std::string uri = request.getURI();
            const std::string method = request.getMethod();

            if (uri == "/api/v1/auth/register" && method == "POST")
            {
                return new RegisterHandler();
            }

            if (uri == "/api/v1/auth/login" && method == "POST")
            {
                return new LoginHandler();
            }

            if (uri == "/api/v1/users" && method == "POST")
            {
                return new CreateUserHandler();
            }

            if (uri.rfind("/api/v1/users/by-login/", 0) == 0 && method == "GET")
            {
                return new FindUserByLoginHandler();
            }

            if (uri.rfind("/api/v1/users/search", 0) == 0 && method == "GET")
            {
                return new FindUserByNameHandler();
            }

            if (uri == "/api/v1/books" && method == "POST")
            {
                return new AuthMiddlewareHandler(new CreateBookHandler());
            }

            if (uri.rfind("/api/v1/books/search", 0) == 0 && method == "GET")
            {
                return new SearchBooksHandler();
            }

            if (uri == "/api/v1/loans" && method == "POST")
            {
                return new AuthMiddlewareHandler(new CreateLoanHandler());
            }

            if (uri.rfind("/api/v1/users/", 0) == 0 && uri.find("/loans") != std::string::npos && method == "GET")
            {
                return new GetUserLoansHandler();
            }

            if (uri.rfind("/api/v1/loans/", 0) == 0 && uri.find("/return") != std::string::npos && method == "PATCH")
            {
                return new AuthMiddlewareHandler(new ReturnLoanHandler());
            }

            if (uri == "/swagger.yaml" && method == "GET")
            {
                return new SwaggerHandler();
            }

            return new NotFoundHandler();
        }
    };

}