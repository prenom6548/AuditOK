/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
#include "../../src/lock-backend/personalizeddata.h"

#include "gtest/gtest.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "stubext.h"
using namespace stub_ext;

// 测试夹具类，用于初始化和清理测试环境
class PersonalizedDataTest : public ::testing::Test {
protected:
    PersonalizedData* personalizedData;
//    PersonalizedDataMng* personalizedDataMng;

    virtual void SetUp() override {
        personalizedData = new PersonalizedData("testUser");
//        personalizedDataMng = new PersonalizedDataMng();
    }

    virtual void TearDown() override {
        delete personalizedData;
//        delete personalizedDataMng;
    }
};


// 测试 PersonalizedData 类的构造函数和析构函数
TEST_F(PersonalizedDataTest, PersonalizedDataConstructorAndDestructor) {
    // 构造函数应该正确设置用户
    KYLINUSERDATAMNG::getInstance()->GetUserPtr(getenv("USER"));
    KYLINUSERDATAMNG::getInstance()->GetConfInformation(getenv("USER"));
//    KYLINUSERDATAMNG::getInstance()->__getUserInfomation();
//    EXPECT_EQ(personalizedData->m_user, "testUser");
}


// 测试 PersonalizedData::getJsonData 函数
TEST_F(PersonalizedDataTest, PersonalizedDataGetJsonData) {
    QJsonObject json;
    personalizedData->getJsonData(json);

    // 检查生成的 JSON 对象是否包含正确的信息
    EXPECT_EQ(json["user"].toString(), "testUser");
    EXPECT_EQ(json["dateType"].toString(), "testDateType");
    EXPECT_EQ(json["fontSize"].toInt(), 12);
    EXPECT_EQ(json["timeType"].toInt(), 24);
    EXPECT_EQ(json["backgroundPath"].toString(), "/path/to/background");
    EXPECT_EQ(json["color"].toString(), "red");
}


// 测试 PersonalizedData::setJson 函数
TEST_F(PersonalizedDataTest, PersonalizedDataSetJson) {
    QJsonObject json;
    json["user"] = "testUser";
    json["dateType"] = "newDateType";
    json["fontSize"] = 14;
    json["timeType"] = 12;
    json["backgroundPath"] = "/new/path/to/background";
    json["color"] = "blue";
    personalizedData->setJson(json);

    // 检查属性是否被正确设置
//    EXPECT_EQ(personalizedData->m_dateType, "newDateType");
//    EXPECT_EQ(personalizedData->m_fontSize, 14);
//    EXPECT_EQ(personalizedData->m_timeType, 12);
//    EXPECT_EQ(personalizedData->m_backgroundPath, "/new/path/to/background");
//    EXPECT_EQ(personalizedData->m_color, "blue");
}


//// 测试 PersonalizedDataMng::GetConfInformation 函数
//TEST_F(PersonalizedDataTest, PersonalizedDataMngGetConfInformation) {
//    // 先添加一些用户信息
//    personalizedDataMng->m_userPersonalizedData["testUser"] = KylinUserDatePtr(new PersonalizedData("testUser"));
//    personalizedDataMng->m_userPersonalizedData["testUser"]->m_dateType = "testDateType";
//    personalizedDataMng->m_userPersonalizedData["testUser"]->m_fontSize = 12;
//    personalizedDataMng->m_userPersonalizedData["testUser"]->m_timeType = 24;
//    personalizedDataMng->m_userPersonalizedData["testUser"]->m_backgroundPath = "/path/to/background";
//    personalizedDataMng->m_userPersonalizedData["testUser"]->m_color = "red";

//    QString jsonStr = personalizedDataMng->GetConfInformation("testUser");
//    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
//    QJsonObject json = doc.object();
//    QJsonObject userJson = json["testUser"].toObject();

//    // 检查生成的 JSON 对象是否包含正确的信息
//    EXPECT_EQ(userJson["greeter"]["dateType"].toString(), "testDateType");
//    EXPECT_EQ(userJson["greeter"]["fontSize"].toInt(), 12);
//    EXPECT_EQ(userJson["greeter"]["timeType"].toInt(), 24);
//    EXPECT_EQ(userJson["greeter"]["backgroundPath"].toString(), "/path/to/background");
//    EXPECT_EQ(userJson["greeter"]["color"].toString(), "red");
//}


//// 测试 PersonalizedDataMng::GetUserPtr 函数
//TEST_F(PersonalizedDataTest, PersonalizedDataMngGetUserPtr) {
//    personalizedDataMng->m_userPersonalizedData["testUser"] = KylinUserDatePtr(new PersonalizedData("testUser"));
//    KylinUserDatePtr userPtr = personalizedDataMng->GetUserPtr("testUser");
//    // 检查是否能正确获取用户指针
//    EXPECT_NE(userPtr, nullptr);
//    EXPECT_EQ(userPtr->m_user, "testUser");
//}


//// 测试 PersonalizedDataMng::updateUserInformation 函数
//TEST_F(PersonalizedDataTest, PersonalizedDataMngUpdateUserInformation) {
//    QString jsonString = "{\"user\":\"testUser\",\"operation\":0,\"greeter\":{\"dateType\":\"newDateType\",\"fontSize\":14,\"timeType\":12,\"backgroundPath\":\"/new/path/to/background\",\"color\":\"blue\"}}";
//    personalizedDataMng->updateUserInformation(jsonString);
//    KylinUserDatePtr userPtr = personalizedDataMng->GetUserPtr("testUser");
//    // 检查用户信息是否被正确更新
//    EXPECT_EQ(userPtr->m_dateType, "newDateType");
//    EXPECT_EQ(userPtr->m_fontSize, 14);
//    EXPECT_EQ(userPtr->m_timeType, 12);
//    EXPECT_EQ(userPtr->m_backgroundPath, "/new/path/to/background");
//    EXPECT_EQ(userPtr->m_color, "blue");
//}


//// 测试 PersonalizedDataMng::__getUserInfomation 函数
//TEST_F(PersonalizedDataTest, PersonalizedDataMngGetUserInfomation) {
//    // 模拟从 DBus 获取的用户信息
//    QString jsonString = "{\"testUser\":{\"greeter\":{\"dateType\":\"newDateType\",\"fontSize\":14,\"timeType\":12,\"backgroundPath\":\"/new/path/to/background\",\"color\":\"blue\"}}";
//    QDBusInterface iface(
//        static_dbus_service_name, static_dbus_path, static_dbus_interface, QDBusConnection::systemBus());
//    QDBusPendingCall pcall;
//    QDBusPendingReply<QString> reply(jsonString);
//    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, personalizedDataMng);
//    personalizedDataMng->wait_for_finish(watcher);
//    personalizedDataMng->__getUserInfomation();
//    KylinUserDatePtr userPtr = personalizedDataMng->GetUserPtr("testUser");
//    // 检查用户信息是否被正确设置
//    EXPECT_EQ(userPtr->m_dateType, "newDateType");
//    EXPECT_EQ(userPtr->m_fontSize, 14);
//    EXPECT_EQ(userPtr->m_timeType, 12);
//    EXPECT_EQ(userPtr->m_backgroundPath, "/new/path/to/background");
//    EXPECT_EQ(userPtr->m_color, "blue");
//}
