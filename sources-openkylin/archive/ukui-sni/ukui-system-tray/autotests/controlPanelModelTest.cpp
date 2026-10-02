#include <QtTest>
#include <QDBusConnection>

#define private public
#define protected public
#include "plugin/control-panel-model.h"
#undef protected
#undef private

class controlPanelModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void test_selfAndEnableState();
    void test_basicInvokableAndSortHelpers();
};

void controlPanelModel_test::initTestCase()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session DBus is not available.");
    }
}

void controlPanelModel_test::test_selfAndEnableState()
{
    ControlPanelModel *model = ControlPanelModel::self();
    QVERIFY(model != nullptr);

    const bool before = model->enable();
    model->setEnable(!before);
    QCOMPARE(model->enable(), !before);
    model->setEnable(before);
    QCOMPARE(model->enable(), before);
}

void controlPanelModel_test::test_basicInvokableAndSortHelpers()
{
    ControlPanelModel *model = ControlPanelModel::self();
    QVERIFY(model != nullptr);

    model->activate(0);
    model->showContextMenu(QModelIndex(), nullptr);

    QCOMPARE(model->filterAcceptsRow(-1, QModelIndex()), false);
    QCOMPARE(model->lessThan(QModelIndex(), QModelIndex()), false);
}

QTEST_MAIN(controlPanelModel_test)

#include "controlPanelModelTest.moc"
