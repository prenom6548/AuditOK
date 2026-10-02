/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include "../../uniauth-backend/src/personalizeddata.h"

// Global instance for testing
PersonalizedDataMng* g_personalizedDataMng = nullptr;

// Test case for PersonalizedData
TEST(PersonalizedDataTest, GetJsonData) {
    // Create a PersonalizedData instance
    PersonalizedData pd("testuser");

    // Set some values (using direct access for simplicity in this test)
    pd.dateType("yyyy-MM-dd");
    pd.fontSize(12);
    pd.timeType(24);
    pd.backgroundPath("/path/to/background");
    pd.color("#FFFFFF");
    pd.cursor_size(10);
    pd.cursor_theme("");
    pd.scaling_factor(1);

    // Get JSON data
    QJsonObject json;
    pd.getJsonData(json);

    // Verify JSON data
    EXPECT_EQ(json["user"].toString(), "testuser");
    EXPECT_EQ(json["dateType"].toString(), "yyyy-MM-dd");
    EXPECT_EQ(json["fontSize"].toInt(), 12);
    EXPECT_EQ(json["timeType"].toInt(), 24);
    EXPECT_EQ(json["backgroundPath"].toString(), "/path/to/background");
    EXPECT_EQ(json["color"].toString(), "#FFFFFF");

    pd.dateType();
    pd.fontSize();
    pd.timeType();
    pd.backgroundPath();
    pd.color();
    pd.cursor_size();
    pd.cursor_theme();
    pd.scaling_factor();
    pd.getJsonData();

    pd.fileChanged("");
    pd.__load_conf_usd_conf();
    pd.__copy_file("/usr/share/backgrounds/1-openkylin.jpg");
    pd.__copy_file("");

}

// Test case for PersonalizedDataMng
TEST(PersonalizedDataMngTest, GetConfInformation) {
    // Initialize PersonalizedDataMng with mock dependencies if needed
    if (!g_personalizedDataMng) {
        g_personalizedDataMng = new PersonalizedDataMng();
        // Mock QLightDM::UsersModel and other dependencies if necessary
    }

    // Add a user to the model (simulating onUserAdded)
    QString userName = "testuser";
    PersonalizedData* pd = new PersonalizedData(userName);
    QJsonObject json;
    pd->getJsonData(json);
    json["operation"] = "testoperation";
    QJsonDocument document;
    document.setObject(json);
    QString jsonStr = document.toJson(QJsonDocument::Compact);

    // Emit the conf_changed signal (simulating onUserAdded)
    emit g_personalizedDataMng->conf_changed(jsonStr);

    // Retrieve configuration information
    QString confInfo = g_personalizedDataMng->GetConfInformation(userName);

    // Verify configuration information (parse JSON and check values)
    QJsonDocument confDoc = QJsonDocument::fromJson(confInfo.toUtf8());
    QJsonObject confJson = confDoc.object();

    EXPECT_EQ(confJson["user"].toString(), userName);

//    EXPECT_EQ(confJson["operation"].toInt(), "testoperation");
    EXPECT_EQ(confJson["dateType"].toString(), "cn"); // Default value from PersonalizedData
    // Add more verifications as needed

    g_personalizedDataMng->SetLoginSynInformation("");

    g_personalizedDataMng->GetConfInformation("kylin");
    g_personalizedDataMng->GetConfInformation("lightdm");
    g_personalizedDataMng->GetConfInformation("");

    QModelIndex index;
    g_personalizedDataMng->onUserAdded(index,0,0);
    g_personalizedDataMng->onUserRemoved(index,0,0);
    delete g_personalizedDataMng;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
