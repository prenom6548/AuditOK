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
 *
 */

#include <QtTest>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QTimer>

namespace TestPanelFocusBridgeDbus {

struct ReplyData
{
    bool value {false};
    bool error {false};
};

struct PendingCallData
{
    QString method;
    QString argument;
};

inline QList<PendingCallData> g_asyncCalls;
inline QList<PendingCallData> g_syncCalls;
inline QHash<QString, ReplyData> g_asyncReplies;
inline QHash<QString, ReplyData> g_syncReplies;

inline void reset()
{
    g_asyncCalls.clear();
    g_syncCalls.clear();
    g_asyncReplies.clear();
    g_syncReplies.clear();
}

inline void setAsyncReply(const QString &method, bool value, bool error = false)
{
    g_asyncReplies.insert(method, ReplyData{value, error});
}

inline ReplyData asyncReplyFor(const QString &method)
{
    return g_asyncReplies.value(method, ReplyData{});
}

inline ReplyData syncReplyFor(const QString &method)
{
    return g_syncReplies.value(method, ReplyData{});
}

inline QString firstArgument(const QDBusMessage &message)
{
    if (message.arguments().isEmpty()) {
        return {};
    }

    return message.arguments().constFirst().toString();
}

inline QDBusMessage buildReply(const QDBusMessage &message, const ReplyData &replyData)
{
    if (replyData.error) {
        return QDBusMessage::createError(
                    QStringLiteral("org.ukui.test.Error"),
                    QStringLiteral("dbus-error"));
    }

    QDBusMessage reply = message.createReply();
    reply << replyData.value;
    return reply;
}

} // namespace TestPanelFocusBridgeDbus

class TestFakePendingCall
{
public:
    TestFakePendingCall() = default;
    TestFakePendingCall(QString method, QString argument)
        : method(std::move(method))
        , argument(std::move(argument))
    {
    }

    QString method;
    QString argument;
};

class TestFakePendingCallWatcher : public QObject
{
    Q_OBJECT
public:
    explicit TestFakePendingCallWatcher(const TestFakePendingCall &call, QObject *parent = nullptr)
        : QObject(parent)
        , m_call(call)
    {
        QTimer::singleShot(0, this, [this] {
            Q_EMIT finished(this);
        });
    }

    QString method() const
    {
        return m_call.method;
    }

    QString argument() const
    {
        return m_call.argument;
    }

Q_SIGNALS:
    void finished(TestFakePendingCallWatcher *self);

private:
    TestFakePendingCall m_call;
};

template<typename T>
class TestFakePendingReply
{
public:
    TestFakePendingReply() = default;

    explicit TestFakePendingReply(const TestFakePendingCallWatcher &watcher)
        : m_reply(TestPanelFocusBridgeDbus::asyncReplyFor(watcher.method()))
    {
    }

    bool isError() const
    {
        return m_reply.error;
    }

    T value() const
    {
        return static_cast<T>(m_reply.value);
    }

    template<int Index>
    T argumentAt() const
    {
        static_assert(Index == 0, "Only the first return argument is supported.");
        return value();
    }

private:
    TestPanelFocusBridgeDbus::ReplyData m_reply;
};

class TestFakeDBusConnection
{
public:
    static TestFakeDBusConnection sessionBus()
    {
        return {};
    }

    QDBusMessage call(const QDBusMessage &message, QDBus::CallMode, int) const
    {
        TestPanelFocusBridgeDbus::g_syncCalls.append(
                    {message.member(), TestPanelFocusBridgeDbus::firstArgument(message)});
        return TestPanelFocusBridgeDbus::buildReply(
                    message,
                    TestPanelFocusBridgeDbus::syncReplyFor(message.member()));
    }

    TestFakePendingCall asyncCall(const QDBusMessage &message) const
    {
        const TestPanelFocusBridgeDbus::PendingCallData callData {
            message.member(),
            TestPanelFocusBridgeDbus::firstArgument(message)
        };
        TestPanelFocusBridgeDbus::g_asyncCalls.append(callData);
        return TestFakePendingCall(callData.method, callData.argument);
    }
};

#define private public
#include "../../../src/utils/panel-focus-bridge.h"
#undef private

#define QDBusConnection TestFakeDBusConnection
#define QDBusPendingCall TestFakePendingCall
#define QDBusPendingCallWatcher TestFakePendingCallWatcher
#define QDBusPendingReply TestFakePendingReply
#include "../../../src/utils/panel-focus-bridge.cpp"
#undef QDBusPendingReply
#undef QDBusPendingCallWatcher
#undef QDBusPendingCall
#undef QDBusConnection

using namespace UkuiMenu;

class PanelFocusBridgeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void hideWithoutScheduleClearsAnchorAsynchronously();
    void escHideUsesAsyncRestoreInsteadOfBlockingCall();
    void restoreFailureFallsBackToClear();
};

void PanelFocusBridgeTest::init()
{
    TestPanelFocusBridgeDbus::reset();
    auto *bridge = PanelFocusBridge::instance();
    bridge->m_restorePanelFocusOnNextHide = false;
}

void PanelFocusBridgeTest::cleanup()
{
    QTRY_VERIFY(PanelFocusBridge::instance()->children().isEmpty());
}

void PanelFocusBridgeTest::hideWithoutScheduleClearsAnchorAsynchronously()
{
    PanelFocusBridge::instance()->handleMenuHidden();

    QCOMPARE(TestPanelFocusBridgeDbus::g_syncCalls.size(), 0);
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.size(), 1);
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.constFirst().method,
             QStringLiteral("clearFocusReturnByExternalKey"));
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.constFirst().argument,
             QStringLiteral("org.ukui.panel.menu.window"));
}

void PanelFocusBridgeTest::escHideUsesAsyncRestoreInsteadOfBlockingCall()
{
    TestPanelFocusBridgeDbus::setAsyncReply(QStringLiteral("restoreFocusByExternalKey"), true);

    auto *bridge = PanelFocusBridge::instance();
    bridge->scheduleRestorePanelFocusOnHide();
    bridge->handleMenuHidden();

    QTRY_COMPARE(TestPanelFocusBridgeDbus::g_syncCalls.size(), 0);
    QTRY_COMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.size(), 1);
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.constFirst().method,
             QStringLiteral("restoreFocusByExternalKey"));
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.constFirst().argument,
             QStringLiteral("org.ukui.panel.menu.window"));
}

void PanelFocusBridgeTest::restoreFailureFallsBackToClear()
{
    TestPanelFocusBridgeDbus::setAsyncReply(QStringLiteral("restoreFocusByExternalKey"), false);

    auto *bridge = PanelFocusBridge::instance();
    bridge->scheduleRestorePanelFocusOnHide();
    bridge->handleMenuHidden();

    QTRY_COMPARE(TestPanelFocusBridgeDbus::g_syncCalls.size(), 0);
    QTRY_COMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.size(), 2);
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.at(0).method,
             QStringLiteral("restoreFocusByExternalKey"));
    QCOMPARE(TestPanelFocusBridgeDbus::g_asyncCalls.at(1).method,
             QStringLiteral("clearFocusReturnByExternalKey"));
}

QTEST_MAIN(PanelFocusBridgeTest)

#include "panel-focus-bridge-test.moc"
