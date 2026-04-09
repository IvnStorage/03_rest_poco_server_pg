#pragma once

#include <Poco/Timestamp.h>

namespace handlers
{
    struct DummyCounter
    {
        void inc()
        {
        }
    };

    struct DummyHistogram
    {
        void observe(double)
        {
        }
    };

    extern DummyCounter *g_httpRequests;
    extern DummyCounter *g_httpErrors;
    extern DummyHistogram *g_httpDuration;

    inline void markError()
    {
        if (g_httpErrors)
        {
            g_httpErrors->inc();
        }
    }

    inline void writeMetrics(const Poco::Timestamp &)
    {
        if (g_httpDuration)
        {
            g_httpDuration->observe(0.0);
        }
    }
}