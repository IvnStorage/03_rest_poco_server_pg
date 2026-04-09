#pragma once

#include <string>
#include <vector>

#include "library_dto.h"

namespace handlers
{
    class PostgresStorage
    {
    public:
        static PostgresStorage &instance();

        bool createUser(const std::string &login,
                        const std::string &password,
                        const std::string &firstName,
                        const std::string &lastName,
                        UserDto &createdUser);

        bool findUserByLogin(const std::string &login, UserDto &result) const;

        std::vector<UserDto> findUsersByNameMask(const std::string &firstNameMask,
                                                 const std::string &lastNameMask) const;

        bool validateUserCredentials(const std::string &login,
                                     const std::string &password,
                                     UserDto &result) const;

        bool addBook(const std::string &title,
                     const std::string &author,
                     int year,
                     BookDto &createdBook);

        std::vector<BookDto> findBooksByTitle(const std::string &titleMask) const;
        std::vector<BookDto> findBooksByAuthor(const std::string &authorMask) const;

        bool userExists(int userId) const;
        bool bookExists(int bookId) const;

        bool createLoan(int userId,
                        int bookId,
                        const std::string &issuedAt,
                        LoanDto &createdLoan,
                        std::string &errorMessage);

        std::vector<LoanDto> getLoansByUserId(int userId) const;

        bool returnBook(int loanId,
                        const std::string &returnedAt,
                        LoanDto &updatedLoan,
                        std::string &errorMessage);

    private:
        PostgresStorage() = default;
    };
}