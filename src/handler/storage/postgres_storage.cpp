#include "postgres_storage.h"

#include <string>
#include <vector>

#include <Poco/Data/RecordSet.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Nullable.h>

#include "db_session.h"

using namespace Poco::Data::Keywords;

namespace handlers
{
    PostgresStorage &PostgresStorage::instance()
    {
        static PostgresStorage instance;
        return instance;
    }

    bool PostgresStorage::createUser(const std::string &login,
                                     const std::string &password,
                                     const std::string &firstName,
                                     const std::string &lastName,
                                     UserDto &createdUser)
    {
        if (login.empty() || password.empty() || firstName.empty() || lastName.empty())
        {
            return false;
        }

        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            std::string loginValue = login;
            std::string passwordValue = password;
            std::string firstNameValue = firstName;
            std::string lastNameValue = lastName;

            int id = 0;
            std::string outLogin;
            std::string outPassword;
            std::string outFirstName;
            std::string outLastName;

            session << R"(
                INSERT INTO users (login, password_hash, first_name, last_name)
                VALUES ($1, $2, $3, $4)
                RETURNING id, login, password_hash, first_name, last_name
            )",
                use(loginValue),
                use(passwordValue),
                use(firstNameValue),
                use(lastNameValue),
                into(id),
                into(outLogin),
                into(outPassword),
                into(outFirstName),
                into(outLastName),
                now;

            createdUser.id = id;
            createdUser.login = outLogin;
            createdUser.password = outPassword;
            createdUser.firstName = outFirstName;
            createdUser.lastName = outLastName;
            return true;
        }
        catch (const Poco::Exception &)
        {
            return false;
        }
    }

    bool PostgresStorage::findUserByLogin(const std::string &login, UserDto &result) const
    {
        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            std::string loginValue = login;

            int count = 0;
            session << "SELECT COUNT(*) FROM users WHERE login = $1",
                use(loginValue),
                into(count),
                now;

            if (count == 0)
            {
                return false;
            }

            int id = 0;
            std::string outLogin;
            std::string outPassword;
            std::string outFirstName;
            std::string outLastName;

            session << R"(
                SELECT id, login, password_hash, first_name, last_name
                FROM users
                WHERE login = $1
                LIMIT 1
            )",
                use(loginValue),
                into(id),
                into(outLogin),
                into(outPassword),
                into(outFirstName),
                into(outLastName),
                now;

            result.id = id;
            result.login = outLogin;
            result.password = outPassword;
            result.firstName = outFirstName;
            result.lastName = outLastName;

            return true;
        }
        catch (const Poco::Exception &)
        {
            return false;
        }
    }

    std::vector<UserDto> PostgresStorage::findUsersByNameMask(const std::string &firstNameMask,
                                                              const std::string &lastNameMask) const
    {
        std::vector<UserDto> result;

        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            std::string firstPattern = "%" + firstNameMask + "%";
            std::string lastPattern = "%" + lastNameMask + "%";

            Poco::Data::Statement select(session);
            select << R"(
                SELECT id, login, password_hash, first_name, last_name
                FROM users
                WHERE ($1 = '%%' OR LOWER(first_name) LIKE LOWER($1))
                  AND ($2 = '%%' OR LOWER(last_name) LIKE LOWER($2))
                ORDER BY id
            )",
                use(firstPattern),
                use(lastPattern),
                now;

            Poco::Data::RecordSet rs(select);

            bool more = rs.moveFirst();
            while (more)
            {
                UserDto user;
                user.id = rs[0].convert<int>();
                user.login = rs[1].convert<std::string>();
                user.password = rs[2].convert<std::string>();
                user.firstName = rs[3].convert<std::string>();
                user.lastName = rs[4].convert<std::string>();
                result.push_back(user);
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception &)
        {
        }

        return result;
    }

    bool PostgresStorage::validateUserCredentials(const std::string &login,
                                                  const std::string &password,
                                                  UserDto &result) const
    {
        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            std::string loginValue = login;
            std::string passwordValue = password;

            int count = 0;
            session << "SELECT COUNT(*) FROM users WHERE login = $1 AND password_hash = $2",
                use(loginValue),
                use(passwordValue),
                into(count),
                now;

            if (count == 0)
            {
                return false;
            }

            int id = 0;
            std::string outLogin;
            std::string outPassword;
            std::string outFirstName;
            std::string outLastName;

            session << R"(
                SELECT id, login, password_hash, first_name, last_name
                FROM users
                WHERE login = $1 AND password_hash = $2
                LIMIT 1
            )",
                use(loginValue),
                use(passwordValue),
                into(id),
                into(outLogin),
                into(outPassword),
                into(outFirstName),
                into(outLastName),
                now;

            result.id = id;
            result.login = outLogin;
            result.password = outPassword;
            result.firstName = outFirstName;
            result.lastName = outLastName;
            return true;
        }
        catch (const Poco::Exception &)
        {
            return false;
        }
    }

    bool PostgresStorage::addBook(const std::string &title,
                                  const std::string &author,
                                  int year,
                                  BookDto &createdBook)
    {
        if (title.empty() || author.empty())
        {
            return false;
        }

        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            std::string titleValue = title;
            std::string authorValue = author;

            int id = 0;
            std::string outTitle;
            std::string outAuthor;
            int outYear = 0;
            bool outAvailable = true;

            session << R"(
                INSERT INTO books (title, author, publication_year, available)
                VALUES ($1, $2, $3, TRUE)
                RETURNING id, title, author, publication_year, available
            )",
                use(titleValue),
                use(authorValue),
                use(year),
                into(id),
                into(outTitle),
                into(outAuthor),
                into(outYear),
                into(outAvailable),
                now;

            createdBook.id = id;
            createdBook.title = outTitle;
            createdBook.author = outAuthor;
            createdBook.year = outYear;
            createdBook.available = outAvailable;

            return true;
        }
        catch (const Poco::Exception &)
        {
            return false;
        }
    }

    std::vector<BookDto> PostgresStorage::findBooksByTitle(const std::string &titleMask) const
    {
        std::vector<BookDto> result;

        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());
            std::string pattern = "%" + titleMask + "%";

            Poco::Data::Statement select(session);
            select << R"(
                SELECT id, title, author, publication_year, available
                FROM books
                WHERE LOWER(title) LIKE LOWER($1)
                ORDER BY id
            )",
                use(pattern),
                now;

            Poco::Data::RecordSet rs(select);

            bool more = rs.moveFirst();
            while (more)
            {
                BookDto book;
                book.id = rs[0].convert<int>();
                book.title = rs[1].convert<std::string>();
                book.author = rs[2].convert<std::string>();
                book.year = rs[3].convert<int>();
                book.available = rs[4].convert<bool>();
                result.push_back(book);
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception &)
        {
        }

        return result;
    }

    std::vector<BookDto> PostgresStorage::findBooksByAuthor(const std::string &authorMask) const
    {
        std::vector<BookDto> result;

        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());
            std::string pattern = "%" + authorMask + "%";

            Poco::Data::Statement select(session);
            select << R"(
                SELECT id, title, author, publication_year, available
                FROM books
                WHERE LOWER(author) LIKE LOWER($1)
                ORDER BY id
            )",
                use(pattern),
                now;

            Poco::Data::RecordSet rs(select);

            bool more = rs.moveFirst();
            while (more)
            {
                BookDto book;
                book.id = rs[0].convert<int>();
                book.title = rs[1].convert<std::string>();
                book.author = rs[2].convert<std::string>();
                book.year = rs[3].convert<int>();
                book.available = rs[4].convert<bool>();
                result.push_back(book);
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception &)
        {
        }

        return result;
    }

    bool PostgresStorage::userExists(int userId) const
    {
        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            int count = 0;
            session << "SELECT COUNT(*) FROM users WHERE id = $1",
                use(userId),
                into(count),
                now;

            return count > 0;
        }
        catch (const Poco::Exception &)
        {
            return false;
        }
    }

    bool PostgresStorage::bookExists(int bookId) const
    {
        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            int count = 0;
            session << "SELECT COUNT(*) FROM books WHERE id = $1",
                use(bookId),
                into(count),
                now;

            return count > 0;
        }
        catch (const Poco::Exception &)
        {
            return false;
        }
    }

    bool PostgresStorage::createLoan(int userId,
                                     int bookId,
                                     const std::string &issuedAt,
                                     LoanDto &createdLoan,
                                     std::string &errorMessage)
    {
        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());
            session.begin();

            int userCount = 0;
            session << "SELECT COUNT(*) FROM users WHERE id = $1",
                use(userId),
                into(userCount),
                now;

            if (userCount == 0)
            {
                session.rollback();
                errorMessage = "user not found";
                return false;
            }

            int bookCount = 0;
            session << "SELECT COUNT(*) FROM books WHERE id = $1",
                use(bookId),
                into(bookCount),
                now;

            if (bookCount == 0)
            {
                session.rollback();
                errorMessage = "book not found";
                return false;
            }

            bool available = false;
            session << "SELECT available FROM books WHERE id = $1 FOR UPDATE",
                use(bookId),
                into(available),
                now;

            if (!available)
            {
                session.rollback();
                errorMessage = "book is already issued";
                return false;
            }

            std::string issuedAtValue = issuedAt;

            int loanId = 0;
            int outUserId = 0;
            int outBookId = 0;
            std::string outIssuedAt;
            Poco::Nullable<std::string> outReturnedAt;
            bool returned = false;

            session << R"(
                INSERT INTO loans (user_id, book_id, issued_at, returned, returned_at)
                VALUES ($1, $2, $3, FALSE, NULL)
                RETURNING id, user_id, book_id, issued_at, returned_at, returned
            )",
                use(userId),
                use(bookId),
                use(issuedAtValue),
                into(loanId),
                into(outUserId),
                into(outBookId),
                into(outIssuedAt),
                into(outReturnedAt),
                into(returned),
                now;

            session << "UPDATE books SET available = FALSE WHERE id = $1",
                use(bookId),
                now;

            session.commit();

            createdLoan.id = loanId;
            createdLoan.userId = outUserId;
            createdLoan.bookId = outBookId;
            createdLoan.issuedAt = outIssuedAt;
            createdLoan.returnedAt = outReturnedAt.isNull() ? "" : outReturnedAt.value();
            createdLoan.returned = returned;
            return true;
        }
        catch (const Poco::Exception &ex)
        {
            errorMessage = ex.displayText();
            return false;
        }
    }

    std::vector<LoanDto> PostgresStorage::getLoansByUserId(int userId) const
    {
        std::vector<LoanDto> result;

        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());

            Poco::Data::Statement select(session);
            select << R"(
                SELECT id, user_id, book_id, issued_at, returned_at, returned
                FROM loans
                WHERE user_id = $1
                ORDER BY issued_at DESC, id DESC
            )",
                use(userId),
                now;

            Poco::Data::RecordSet rs(select);

            bool more = rs.moveFirst();
            while (more)
            {
                LoanDto loan;
                loan.id = rs[0].convert<int>();
                loan.userId = rs[1].convert<int>();
                loan.bookId = rs[2].convert<int>();
                loan.issuedAt = rs[3].convert<std::string>();
                loan.returnedAt = rs[4].isEmpty() ? "" : rs[4].convert<std::string>();
                loan.returned = rs[5].convert<bool>();
                result.push_back(loan);
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception &)
        {
        }

        return result;
    }

    bool PostgresStorage::returnBook(int loanId,
                                     const std::string &returnedAt,
                                     LoanDto &updatedLoan,
                                     std::string &errorMessage)
    {
        try
        {
            Poco::Data::Session session(DbSession::instance().pool().get());
            session.begin();

            int count = 0;
            session << "SELECT COUNT(*) FROM loans WHERE id = $1",
                use(loanId),
                into(count),
                now;

            if (count == 0)
            {
                session.rollback();
                errorMessage = "loan not found";
                return false;
            }

            int bookId = 0;
            bool alreadyReturned = false;
            session << "SELECT book_id, returned FROM loans WHERE id = $1 FOR UPDATE",
                use(loanId),
                into(bookId),
                into(alreadyReturned),
                now;

            if (alreadyReturned)
            {
                session.rollback();
                errorMessage = "book already returned";
                return false;
            }

            std::string returnedAtValue = returnedAt;

            session << "UPDATE loans SET returned = TRUE, returned_at = $2 WHERE id = $1",
                use(loanId),
                use(returnedAtValue),
                now;

            session << "UPDATE books SET available = TRUE WHERE id = $1",
                use(bookId),
                now;

            int outLoanId = 0;
            int userId = 0;
            int outBookId = 0;
            std::string issuedAtValue;
            Poco::Nullable<std::string> returnedAtValueOut;
            bool returnedValue = false;

            session << R"(
                SELECT id, user_id, book_id, issued_at, returned_at, returned
                FROM loans
                WHERE id = $1
            )",
                use(loanId),
                into(outLoanId),
                into(userId),
                into(outBookId),
                into(issuedAtValue),
                into(returnedAtValueOut),
                into(returnedValue),
                now;

            session.commit();

            updatedLoan.id = outLoanId;
            updatedLoan.userId = userId;
            updatedLoan.bookId = outBookId;
            updatedLoan.issuedAt = issuedAtValue;
            updatedLoan.returnedAt = returnedAtValueOut.isNull() ? "" : returnedAtValueOut.value();
            updatedLoan.returned = returnedValue;
            return true;
        }
        catch (const Poco::Exception &ex)
        {
            errorMessage = ex.displayText();
            return false;
        }
    }
}