#pragma once

#include <string>

namespace handlers
{
    struct UserDto
    {
        int id = 0;
        std::string login;
        std::string password;
        std::string firstName;
        std::string lastName;
    };

    struct BookDto
    {
        int id = 0;
        std::string title;
        std::string author;
        int year = 0;
        bool available = true;
    };

    struct LoanDto
    {
        int id = 0;
        int userId = 0;
        int bookId = 0;
        std::string issuedAt;
        std::string returnedAt;
        bool returned = false;
    };
}