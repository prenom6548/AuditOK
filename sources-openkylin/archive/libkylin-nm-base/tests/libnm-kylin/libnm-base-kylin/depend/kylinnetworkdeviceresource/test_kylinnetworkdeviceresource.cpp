#include "test_kylinnetworkdeviceresource.h"

TestKyNetworkDeviceResourse::TestKyNetworkDeviceResourse()
    : QObject(nullptr)
    , m_deviceResource(nullptr)
{
}

TestKyNetworkDeviceResourse::~TestKyNetworkDeviceResourse()
{
}

void TestKyNetworkDeviceResourse::SetUp()
{
}

void TestKyNetworkDeviceResourse::TearDown()
{
    if (m_deviceResource) {
        delete m_deviceResource;
        m_deviceResource = nullptr;
    }
}

TEST_F(TestKyNetworkDeviceResourse, TestConstructor)
{
    KyNetworkDeviceResourse deviceResourse;
    EXPECT_NE(&deviceResourse, nullptr);
}

TEST_F(TestKyNetworkDeviceResourse, TestDestructor)
{
    {
        KyNetworkDeviceResourse deviceResourse;
    }
    SUCCEED() << "Destructor called without crash";
}
