#pragma once

#include <memory>

#include <Poco/Data/SessionPool.h>
#include <Poco/Data/PostgreSQL/Connector.h>

namespace handlers
{
    class DbSession
    {
    public:
        static DbSession &instance();

        Poco::Data::SessionPool &pool();

    private:
        DbSession();
        ~DbSession();

        DbSession(const DbSession &) = delete;
        DbSession &operator=(const DbSession &) = delete;

    private:
        std::unique_ptr<Poco::Data::SessionPool> _pool;
    };
}