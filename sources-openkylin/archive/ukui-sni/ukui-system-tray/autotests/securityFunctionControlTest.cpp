#include <QtTest>

#include "function-control.h"

#define private public
#define protected public
#include "plugin/security-function-control.h"
#undef protected
#undef private

class securityFunctionControl_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void test_watchAndBasicState();
    void test_permissionChecks();
    void test_compareListBySort();
    void test_onFunctionControlChangedIgnoreName();
    void test_onFunctionControlChangedApplyConfig();
    void test_onFunctionControlChangedExitReset();

private:
    SecurityFunctionControl *m_control = nullptr;
};

void securityFunctionControl_test::initTestCase()
{
    m_control = SecurityFunctionControl::instance();
    QVERIFY(m_control != nullptr);
}

void securityFunctionControl_test::cleanup()
{
    m_control->m_disableMouseLeftButton = false;
    m_control->m_disableMouseRightButton = false;
    m_control->m_disableMouseHover = false;
    m_control->m_appWhiteList.clear();
    m_control->m_appBlackList.clear();
    m_control->m_leftClickBlackList.clear();
    m_control->m_rightClickBlackList.clear();
    m_control->m_controlEnabled = false;
}

void securityFunctionControl_test::test_watchAndBasicState()
{
    QVERIFY(UkuiQuick::FunctionControl::instance()->watches().contains(QStringLiteral("ukui-sni-v11")));

    QCOMPARE(m_control->disableMouseLeftButton(), false);
    QCOMPARE(m_control->disableMouseRightButton(), false);
    QCOMPARE(m_control->disableMouseHover(), false);
    QCOMPARE(m_control->controlEnabled(), false);
}

void securityFunctionControl_test::test_permissionChecks()
{
    m_control->m_appWhiteList = QStringList{QStringLiteral("allow.desktop")};
    m_control->m_appBlackList = QStringList{QStringLiteral("deny.desktop")};
    m_control->m_leftClickBlackList = QStringList{QStringLiteral("left.desktop")};
    m_control->m_rightClickBlackList = QStringList{QStringLiteral("right.desktop")};

    QVERIFY(m_control->canAppDisplay(QStringLiteral("/tmp/app-allow.desktop")));
    QVERIFY(!m_control->canAppDisplay(QStringLiteral("/tmp/app-other.desktop")));
    QVERIFY(!m_control->canAppDisplay(QStringLiteral("/tmp/app-deny.desktop")));

    QVERIFY(!m_control->canLeftClick(QStringLiteral("/tmp/app-left.desktop")));
    QVERIFY(m_control->canLeftClick(QStringLiteral("/tmp/app-ok.desktop")));

    QVERIFY(!m_control->canRightClick(QStringLiteral("/tmp/app-right.desktop")));
    QVERIFY(m_control->canRightClick(QStringLiteral("/tmp/app-ok.desktop")));
}

void securityFunctionControl_test::test_compareListBySort()
{
    QStringList list1 {QStringLiteral("b"), QStringLiteral("a")};
    QStringList list2 {QStringLiteral("a"), QStringLiteral("b")};
    QStringList list3 {QStringLiteral("a")};

    QVERIFY(m_control->compareListBySort(list1, list2));
    QVERIFY(!m_control->compareListBySort(list1, list3));
}

void securityFunctionControl_test::test_onFunctionControlChangedIgnoreName()
{
    m_control->m_disableMouseLeftButton = true;
    m_control->m_appWhiteList = QStringList{QStringLiteral("keep.desktop")};

    m_control->onFunctionControlChanged(QStringLiteral("other-name"), false);
    QVERIFY(m_control->m_disableMouseLeftButton);
    QCOMPARE(m_control->m_appWhiteList, QStringList{QStringLiteral("keep.desktop")});
}

void securityFunctionControl_test::test_onFunctionControlChangedApplyConfig()
{
    QSignalSpy controlEnabledSpy(m_control, &SecurityFunctionControl::controlEnabledChanged);
    QSignalSpy leftSpy(m_control, &SecurityFunctionControl::disableMouseLeftButtonChanged);
    QSignalSpy rightSpy(m_control, &SecurityFunctionControl::disableMouseRightButtonChanged);
    QSignalSpy hoverSpy(m_control, &SecurityFunctionControl::disableMouseHoverChanged);
    QSignalSpy whiteSpy(m_control, &SecurityFunctionControl::appWhiteListChanged);
    QSignalSpy blackSpy(m_control, &SecurityFunctionControl::appBlackListChanged);
    QSignalSpy leftListSpy(m_control, &SecurityFunctionControl::leftClickBlackListChanged);
    QSignalSpy rightListSpy(m_control, &SecurityFunctionControl::rightClickBlackListChanged);

    QJsonObject config {
        {QStringLiteral("disableMouseLeftButton"), true},
        {QStringLiteral("disableMouseRightButton"), true},
        {QStringLiteral("disableMouseHover"), true},
        {QStringLiteral("appWhiteList"), QJsonArray{QStringLiteral("allow.desktop")}},
        {QStringLiteral("appBlackList"), QJsonArray{QStringLiteral("deny.desktop")}},
        {QStringLiteral("leftClickBlackList"), QJsonArray{QStringLiteral("left.desktop")}},
        {QStringLiteral("rightClickBlackList"), QJsonArray{QStringLiteral("right.desktop")}}
    };
    UkuiQuick::FunctionControl::instance()->setFunctionControlObject(QStringLiteral("ukui-sni-v11"),
                                                                     QJsonObject{{QStringLiteral("ukui-sni-v11"), config}});
    m_control->onFunctionControlChanged(QStringLiteral("ukui-sni-v11"), false);

    QVERIFY(m_control->controlEnabled());
    QVERIFY(m_control->disableMouseLeftButton());
    QVERIFY(m_control->disableMouseRightButton());
    QVERIFY(m_control->disableMouseHover());
    QCOMPARE(m_control->appWhiteList(), QStringList{QStringLiteral("allow.desktop")});
    QCOMPARE(m_control->appBlackList(), QStringList{QStringLiteral("deny.desktop")});
    QCOMPARE(m_control->leftClickBlackList(), QStringList{QStringLiteral("left.desktop")});
    QCOMPARE(m_control->rightClickBlackList(), QStringList{QStringLiteral("right.desktop")});

    QVERIFY(controlEnabledSpy.count() >= 1);
    QVERIFY(leftSpy.count() >= 1);
    QVERIFY(rightSpy.count() >= 1);
    QVERIFY(hoverSpy.count() >= 1);
    QVERIFY(whiteSpy.count() >= 1);
    QVERIFY(blackSpy.count() >= 1);
    QVERIFY(leftListSpy.count() >= 1);
    QVERIFY(rightListSpy.count() >= 1);

    const int whiteCount = whiteSpy.count();
    const int blackCount = blackSpy.count();
    m_control->onFunctionControlChanged(QStringLiteral("ukui-sni-v11"), false);
    QCOMPARE(whiteSpy.count(), whiteCount);
    QCOMPARE(blackSpy.count(), blackCount);
}

void securityFunctionControl_test::test_onFunctionControlChangedExitReset()
{
    m_control->m_disableMouseLeftButton = true;
    m_control->m_disableMouseRightButton = true;
    m_control->m_disableMouseHover = true;
    m_control->m_appWhiteList = QStringList{QStringLiteral("allow.desktop")};
    m_control->m_appBlackList = QStringList{QStringLiteral("deny.desktop")};
    m_control->m_leftClickBlackList = QStringList{QStringLiteral("left.desktop")};
    m_control->m_rightClickBlackList = QStringList{QStringLiteral("right.desktop")};
    m_control->m_controlEnabled = true;

    m_control->onFunctionControlChanged(QStringLiteral("ukui-sni-v11"), true);

    QVERIFY(!m_control->controlEnabled());
    QVERIFY(!m_control->disableMouseLeftButton());
    QVERIFY(!m_control->disableMouseRightButton());
    QVERIFY(!m_control->disableMouseHover());
    QVERIFY(m_control->appWhiteList().isEmpty());
    QVERIFY(m_control->appBlackList().isEmpty());
    QVERIFY(m_control->leftClickBlackList().isEmpty());
    QVERIFY(m_control->rightClickBlackList().isEmpty());
}

QTEST_MAIN(securityFunctionControl_test)

#include "securityFunctionControlTest.moc"
