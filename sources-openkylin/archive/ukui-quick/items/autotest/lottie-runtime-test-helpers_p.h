/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef UKUI_QUICK_LOTTIE_RUNTIME_TEST_HELPERS_P_H
#define UKUI_QUICK_LOTTIE_RUNTIME_TEST_HELPERS_P_H

#include "icon.h"
#include "lottie-attached.h"
#include "lottie-runtime-service_p.h"

#include <QImage>
#include <QVariant>

#include <functional>
#include <thread>
#include <utility>

namespace UkuiQuick {
namespace LottieRuntimeTest {

inline LottieRuntimeFrameSchedulerStats frameSchedulerDiagnostics(
    LottieRuntimeService *service)
{
    LottieRuntimeFrameSchedulerStats stats;
    QMetaObject::invokeMethod(service, "frameSchedulerDiagnostics", Qt::DirectConnection,
                              Q_RETURN_ARG(LottieRuntimeFrameSchedulerStats, stats));
    return stats;
}

inline void resetFrameSchedulerDiagnostics(LottieRuntimeService *service)
{
    QMetaObject::invokeMethod(service, "resetFrameSchedulerDiagnostics", Qt::DirectConnection);
}

inline qint64 sourceLoadingP95Nanoseconds(LottieRuntimeService *service)
{
    qint64 nanoseconds = 0;
    QMetaObject::invokeMethod(service, "sourceLoadingP95Nanoseconds", Qt::DirectConnection,
                              Q_RETURN_ARG(qint64, nanoseconds));
    return nanoseconds;
}

inline bool testResetFrameSchedulerCache(LottieRuntimeService *service)
{
    bool reset = false;
    QMetaObject::invokeMethod(service, "testResetFrameSchedulerCache", Qt::DirectConnection,
                              Q_RETURN_ARG(bool, reset));
    return reset;
}

inline int testAssetWorkerCount(LottieRuntimeService *service)
{
    int workerCount = 0;
    QMetaObject::invokeMethod(service, "testAssetWorkerCount", Qt::DirectConnection,
                              Q_RETURN_ARG(int, workerCount));
    return workerCount;
}

inline int testSourceCompletionCount(LottieRuntimeService *service)
{
    int completionCount = 0;
    QMetaObject::invokeMethod(service, "testSourceCompletionCount", Qt::DirectConnection,
                              Q_RETURN_ARG(int, completionCount));
    return completionCount;
}

inline int testSubscriptionCount(LottieRuntimeService *service)
{
    int subscriptionCount = 0;
    QMetaObject::invokeMethod(service, "testSubscriptionCount", Qt::DirectConnection,
                              Q_RETURN_ARG(int, subscriptionCount));
    return subscriptionCount;
}

inline int testSourceRequestCount(LottieRuntimeService *service)
{
    int requestCount = 0;
    QMetaObject::invokeMethod(service, "testSourceRequestCount", Qt::DirectConnection,
                              Q_RETURN_ARG(int, requestCount));
    return requestCount;
}

inline int testFrameRequestCount(LottieRuntimeService *service)
{
    int requestCount = 0;
    QMetaObject::invokeMethod(service, "testFrameRequestCount", Qt::DirectConnection,
                              Q_RETURN_ARG(int, requestCount));
    return requestCount;
}

inline bool setStoppingTransitionHook(LottieRuntimeService *service,
                                      const std::function<void()> *hook)
{
    return QMetaObject::invokeMethod(
        service, "testSetStoppingTransitionHook", Qt::DirectConnection,
        Q_ARG(quintptr, reinterpret_cast<quintptr>(hook)));
}

inline void setTransparentFallback(Icon *icon)
{
    QImage fallback(QSize(1, 1), QImage::Format_ARGB32_Premultiplied);
    fallback.fill(Qt::transparent);
    icon->setFallbackSource(QVariant::fromValue(fallback));
}

class ScopedThreadJoin
{
public:
    ScopedThreadJoin(std::thread *thread, std::function<void()> failSafe)
        : m_thread(thread)
        , m_failSafe(std::move(failSafe))
    {
    }

    ~ScopedThreadJoin()
    {
        join();
    }

    void join()
    {
        if (!m_thread || !m_thread->joinable()) {
            return;
        }
        if (m_failSafe) {
            m_failSafe();
        }
        m_thread->join();
    }

    ScopedThreadJoin(const ScopedThreadJoin &) = delete;
    ScopedThreadJoin &operator=(const ScopedThreadJoin &) = delete;

private:
    std::thread *m_thread = nullptr;
    std::function<void()> m_failSafe;
};

} // namespace LottieRuntimeTest
} // namespace UkuiQuick

#endif // UKUI_QUICK_LOTTIE_RUNTIME_TEST_HELPERS_P_H
