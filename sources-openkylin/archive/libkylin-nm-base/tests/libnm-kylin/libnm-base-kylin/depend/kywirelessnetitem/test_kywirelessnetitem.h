#ifndef TEST_KYWIRELESSNETITEM_H
#define TEST_KYWIRELESSNETITEM_H

#include <gtest/gtest.h>
#include <QString>
#include "kywirelessnetitem.h"

class TestKyWirelessNetItem : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

protected:
    KyWirelessNetItem* m_testItem;
};

#endif // TEST_KYWIRELESSNETITEM_H
