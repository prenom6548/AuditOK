#ifndef TEST_KYLINSECRETAGENT_H
#define TEST_KYLINSECRETAGENT_H

#include <gtest/gtest.h>
#include <QString>
#include <QVariantMap>
#include <QDBusObjectPath>
#include "kylinsecretagent.h"

class TestKylinSecretAgent : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    KylinSecretAgent *m_agent;
};

class TestKylinSecretAgentFunctions : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    KylinSecretAgent *m_agent;
};

#endif // TEST_KYLINSECRETAGENT_H
