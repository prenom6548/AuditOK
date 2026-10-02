#include <QCoreApplication>
#include <QTest>
#include "test_kyenterpricesettinginfo.h"
#include "test_kywirelessconnectoperation.h"
#include "test_kylinnetworkresourcemanager.h"
#include "test_kynetloadratethread.h"
#include "test_kylinconnectoperation.h"
#include "test_kywirelessnetresource.h"
#include "test_kylinnetworkmanagerutil.h"
#include "test_kylinconnectsetting.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    TestKyEnterpriceSettingInfo tca;
    QTest::qExec(&tca, argc, argv);

    TestKyWirelessConnectOperation tcb;
    QTest::qExec(&tcb, argc, argv);

    TestKyNetworkResourceManager tcc;
    QTest::qExec(&tcc, argc, argv);

    TestKyNetLoadRateThread tcd;
    QTest::qExec(&tcd, argc, argv);

/*
    TestKyConnectOperation tce;
    QTest::qExec(&tce, argc, argv);
*/

    TestKyWirelessNetResource tcf;
    QTest::qExec(&tcf, argc, argv);

    TestKylinNetworkManagerUtil tcg;
    QTest::qExec(&tcg, argc, argv);

    TestKylinConnectSetting tc;
    return QTest::qExec(&tc, argc, argv);
}
