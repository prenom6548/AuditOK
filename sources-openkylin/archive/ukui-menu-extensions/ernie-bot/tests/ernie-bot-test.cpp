#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "../src/ernie-bot.h"
#include "../src/message-model.h"
#include "../src/session.h"
#include "../src/session-manager.h"

#include <ukui-menu/widget-extension.h>

using namespace UkuiMenu;

namespace {

void useTemporaryHome(QTemporaryDir &temporaryDir)
{
    QVERIFY(temporaryDir.isValid());
    qputenv("HOME", temporaryDir.path().toUtf8());
}

}

class ernieBot_test : public QObject
{
    Q_OBJECT

private slots:
    void test_ErnieBot();
    void test_Plugin();
};

void ernieBot_test::test_ErnieBot()
{
    QTemporaryDir temporaryHome;
    useTemporaryHome(temporaryHome);

    ErnieBot extension;
    QCOMPARE(extension.index(), 100);
    QCOMPARE(extension.metadata().value(WidgetMetadata::Id), QString("ernie-bot"));
    QCOMPARE(extension.metadata().value(WidgetMetadata::Version), QString("1.0.0"));
    QCOMPARE(extension.metadata().value(WidgetMetadata::Description), QString("ernie-bot"));
    QCOMPARE(extension.metadata().value(WidgetMetadata::Main), QString("qrc:///ernie-bot/content/ui/Main.qml"));
    QCOMPARE(extension.metadata().value(WidgetMetadata::Type), WidgetMetadata::Widget);
    QCOMPARE(extension.metadata().value(WidgetMetadata::Flag), WidgetMetadata::OnlySmallScreen);

    QVariantMap exportedData = extension.data();
    auto *messageModel = exportedData.value("messageModel").value<MessageModel *>();
    auto *sessionManager = exportedData.value("sessionManager").value<SessionManager *>();
    QVERIFY(messageModel != nullptr);
    QVERIFY(sessionManager != nullptr);

    extension.receive({{"message", "ignored without session"}});
    QCOMPARE(messageModel->currentSessionId(), -1);

    QSignalSpy currentSessionSpy(messageModel, &MessageModel::currentSessionIdChanged);
    QSignalSpy sessionAddedSpy(sessionManager, &SessionManager::sessionAdded);
    QSignalSpy sessionRemovedSpy(sessionManager, &SessionManager::sessionRemoved);

    sessionManager->setSessionKeys("key", "secret");
    QCOMPARE(sessionManager->apiReady(), true);

    const int sessionId = sessionManager->newSession();
    QCOMPARE(sessionId, 0);
    QCOMPARE(sessionAddedSpy.count(), 1);
    QCOMPARE(currentSessionSpy.count(), 1);
    QCOMPARE(messageModel->currentSessionId(), sessionId);

    Session *session = sessionManager->getSession(sessionId);
    QVERIFY(session != nullptr);
    const int rowCountBeforeReceive = session->rowCount(QModelIndex());
    extension.receive({{"message", "<hello from ernie bot>"}});
    QCOMPARE(session->rowCount(QModelIndex()), rowCountBeforeReceive + 3);

    sessionManager->exitSessionApi();
    QCOMPARE(sessionRemovedSpy.count(), 1);
    QCOMPARE(messageModel->currentSessionId(), -1);
}

void ernieBot_test::test_Plugin()
{
    ErnieBotPlugin plugin;
    QCOMPARE(plugin.id(), QString("ernie-bot"));
    QCOMPARE(plugin.createContextMenuExtension(), nullptr);

    WidgetExtension *widgetExtension = plugin.createWidgetExtension();
    QVERIFY(widgetExtension != nullptr);
    QCOMPARE(widgetExtension->metadata().value(WidgetMetadata::Id), QString("ernie-bot"));
    delete widgetExtension;
}

QTEST_MAIN(ernieBot_test)

#include "ernie-bot-test.moc"
