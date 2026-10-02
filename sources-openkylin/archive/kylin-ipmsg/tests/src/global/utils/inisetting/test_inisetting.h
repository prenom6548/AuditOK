/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TEST_INISETTING_H
#define TEST_INISETTING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include "global/utils/inisetting.h"

class TestIniSettings : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    // Helper functions
    void createTestConfigFile();
    
    // Test data
    QTemporaryDir tempDir;
    QString configPath;
};

#endif // TEST_INISETTING_H