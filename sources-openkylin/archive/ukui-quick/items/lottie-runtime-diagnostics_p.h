/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_DIAGNOSTICS_P_H
#define UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_DIAGNOSTICS_P_H

#include <QtGlobal>

#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
#include <algorithm>
#include <chrono>
#include <vector>
#endif

namespace UkuiQuick {

// 计时和计数只用于内部测试与 benchmark。生产构建仍调用同一组小方法，
// 但这些类型没有数据成员，编译器会把调用全部消掉。
class LottieRuntimeDiagnosticsClock
{
public:
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    using TimePoint = std::chrono::steady_clock::time_point;

    static TimePoint now()
    {
        return std::chrono::steady_clock::now();
    }

    static bool isValid(const TimePoint &timePoint)
    {
        return timePoint != TimePoint();
    }

    static qint64 elapsedNanoseconds(const TimePoint &start, const TimePoint &end)
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    }
#else
    struct TimePoint
    {
    };

    static TimePoint now()
    {
        return {};
    }

    static bool isValid(const TimePoint &)
    {
        return false;
    }

    static qint64 elapsedNanoseconds(const TimePoint &, const TimePoint &)
    {
        return 0;
    }
#endif
};

class LottieRuntimeTimingSamples
{
public:
    void record(qint64 value)
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        if (value < 0) {
            return;
        }
        if (m_values.size() < kMaximumSamples) {
            m_values.push_back(value);
            return;
        }
        m_values[m_cursor] = value;
        m_cursor = (m_cursor + 1) % kMaximumSamples;
#else
        Q_UNUSED(value)
#endif
    }

    qint64 percentile95() const
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        if (m_values.empty()) {
            return 0;
        }
        std::vector<qint64> sorted = m_values;
        std::sort(sorted.begin(), sorted.end());
        const size_t index = (sorted.size() * 95 + 99) / 100 - 1;
        return sorted.at(index);
#else
        return 0;
#endif
    }

    void clear()
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        m_values.clear();
        m_cursor = 0;
#endif
    }

private:
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    static constexpr size_t kMaximumSamples = 2048;
    std::vector<qint64> m_values;
    size_t m_cursor = 0;
#endif
};

class LottieRuntimeSourceDiagnostics
{
public:
    using Clock = LottieRuntimeDiagnosticsClock;
    using TimePoint = Clock::TimePoint;

    void recordLoading(const TimePoint &startedAt, const TimePoint &completedAt)
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        m_loading.record(Clock::elapsedNanoseconds(startedAt, completedAt));
#else
        Q_UNUSED(startedAt)
        Q_UNUSED(completedAt)
#endif
    }

    void recordCompletion()
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        ++m_completionCount;
#endif
    }

    void recordSubscription()
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        ++m_subscriptionCount;
#endif
    }

    void recordSourceRequest()
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        ++m_sourceRequestCount;
#endif
    }

    void recordFrameRequest()
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        ++m_frameRequestCount;
#endif
    }

    qint64 loadingP95Nanoseconds() const
    {
        return m_loading.percentile95();
    }

    int completionCount() const
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        return m_completionCount;
#else
        return 0;
#endif
    }

    int subscriptionCount() const
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        return m_subscriptionCount;
#else
        return 0;
#endif
    }

    int sourceRequestCount() const
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        return m_sourceRequestCount;
#else
        return 0;
#endif
    }

    int frameRequestCount() const
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        return m_frameRequestCount;
#else
        return 0;
#endif
    }

private:
    [[no_unique_address]] LottieRuntimeTimingSamples m_loading;
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    int m_completionCount = 0;
    int m_subscriptionCount = 0;
    int m_sourceRequestCount = 0;
    int m_frameRequestCount = 0;
#endif
};

class LottieRuntimeRasterDiagnostics
{
public:
    using Clock = LottieRuntimeDiagnosticsClock;
    using TimePoint = Clock::TimePoint;

    void finish(const TimePoint &startedAt)
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        m_executionNanoseconds = Clock::elapsedNanoseconds(startedAt, Clock::now());
#else
        Q_UNUSED(startedAt)
#endif
    }

    qint64 executionNanoseconds() const
    {
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
        return m_executionNanoseconds;
#else
        return 0;
#endif
    }

private:
#if defined(UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM)
    qint64 m_executionNanoseconds = 0;
#endif
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_ITEMS_LOTTIE_RUNTIME_DIAGNOSTICS_P_H
