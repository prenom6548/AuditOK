#ifndef TEST_KYLINNETWORKDEVICERESOURCE_H
#define TEST_KYLINNETWORKDEVICERESOURCE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include "kylinnetworkdeviceresource.h"
#include "kylinnetworkresourcemanager.h"

class TestKyNetworkDeviceResourse : public QObject, public ::testing::Test {
    Q_OBJECT

public:
    TestKyNetworkDeviceResourse();
    ~TestKyNetworkDeviceResourse();

protected:
    virtual void SetUp();
    virtual void TearDown();

    KyNetworkDeviceResourse *m_deviceResource;
};

#endif // TEST_KYLINNETWORKDEVICERESOURCE_H
