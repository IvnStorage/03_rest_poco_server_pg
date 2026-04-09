#include "db_session.h"

#include <cstdlib>
#include <sstream>
#include <string>

namespace
{
    std::string getEnvOrDefault(const char *name, const std::string &defaultValue)
    {
        const char *value = std::getenv(name);
        if (!value)
        {
            return defaultValue;
        }
        return std::string(value);
    }
}

namespace handlers
{
    DbSession &DbSession::instance()
    {
        static DbSession instance;
        return instance;
    }

    Poco::Data::SessionPool &DbSession::pool()
    {
        return *_pool;
    }

    DbSession::DbSession()
    {
        Poco::Data::PostgreSQL::Connector::registerConnector();

        const std::string host = getEnvOrDefault("DB_HOST", "postgres");
        const std::string port = getEnvOrDefault("DB_PORT", "5432");
        const std::string dbName = getEnvOrDefault("DB_NAME", "library_db");
        const std::string user = getEnvOrDefault("DB_USER", "postgres");
        const std::string password = getEnvOrDefault("DB_PASSWORD", "postgres");

        std::ostringstream connectionString;
        connectionString
            << "host=" << host << " "
            << "port=" << port << " "
            << "dbname=" << dbName << " "
            << "user=" << user << " "
            << "password=" << password;

        _pool = std::make_unique<Poco::Data::SessionPool>(
            "PostgreSQL",
            connectionString.str(),
            1,
            8,
            16);
    }

    DbSession::~DbSession()
    {
        Poco::Data::PostgreSQL::Connector::unregisterConnector();
    }
}