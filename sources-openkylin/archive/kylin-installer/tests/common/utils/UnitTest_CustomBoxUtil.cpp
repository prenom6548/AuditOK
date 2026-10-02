#include <gtest/gtest.h>
#include <QMessageBox>
#include "CustomBoxUtil.h"

class UnitTest_CustomBoxUtil : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up code if needed
    }

    void TearDown() override {
        // Clean up code if needed
    }

};

TEST_F(UnitTest_CustomBoxUtil, TestCreateCritialBoxWithAllParameters) {
    QString windowTitle = "Critical Error";
    QString text = "An error has occurred.";
    QString info = "Please contact support.";
//    QWidget parent;

    QMessageBox* msgBox = CustomBoxUtil::createCritialBox(windowTitle, text, info);

    ASSERT_NE(msgBox, nullptr);
    EXPECT_EQ(msgBox->windowTitle().toStdString(), windowTitle.toStdString());
    EXPECT_EQ(msgBox->text().toStdString(), text.toStdString());
    EXPECT_EQ(msgBox->informativeText().toStdString(), info.toStdString());
    EXPECT_EQ(msgBox->icon(), QMessageBox::Critical);
    EXPECT_EQ(msgBox->buttons().size(), 1);

    QPushButton* button = qobject_cast<QPushButton*>(msgBox->buttons().at(0));
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(button->text().toStdString(), QObject::tr("OK").toStdString());

    delete msgBox;
}

// TEST_F(UnitTest_CustomBoxUtil, TestCreateCritialBoxWithoutInfo) {
//     QString windowTitle = "Critical Error";
//     QString text = "An error has occurred.";
//     QString info = "";
//     QWidget parent;

//     QMessageBox* msgBox = CustomBoxUtil::createCritialBox(windowTitle, text, info, &parent);

//     ASSERT_NE(msgBox, nullptr);
//     EXPECT_EQ(msgBox->windowTitle(), windowTitle);
//     EXPECT_EQ(msgBox->text(), text);
//     EXPECT_TRUE(msgBox->informativeText().isEmpty());
//     EXPECT_EQ(msgBox->icon(), QMessageBox::Critical);
//     EXPECT_EQ(msgBox->buttons().size(), 1);

//     QPushButton* button = qobject_cast<QPushButton*>(msgBox->buttons().at(0));
//     ASSERT_NE(button, nullptr);
//     EXPECT_EQ(button->text(), QObject::tr("OK"));

//     delete msgBox;
// }

// TEST_F(UnitTest_CustomBoxUtil, TestCreateCritialBoxWithoutText) {
//     QString windowTitle = "Critical Error";
//     QString text = "";
//     QString info = "Please contact support.";
//     QWidget parent;

//     QMessageBox* msgBox = CustomBoxUtil::createCritialBox(windowTitle, text, info, &parent);

//     ASSERT_NE(msgBox, nullptr);
//     EXPECT_EQ(msgBox->windowTitle(), windowTitle);
//     EXPECT_TRUE(msgBox->text().isEmpty());
//     EXPECT_EQ(msgBox->informativeText(), info);
//     EXPECT_EQ(msgBox->icon(), QMessageBox::Critical);
//     EXPECT_EQ(msgBox->buttons().size(), 1);

//     QPushButton* button = qobject_cast<QPushButton*>(msgBox->buttons().at(0));
//     ASSERT_NE(button, nullptr);
//     EXPECT_EQ(button->text(), QObject::tr("OK"));

//     delete msgBox;
// }

// TEST_F(UnitTest_CustomBoxUtil, TestCreateCritialBoxWithoutWindowTitle) {
//     QString windowTitle = "";
//     QString text = "An error has occurred.";
//     QString info = "Please contact support.";
//     QWidget parent;

//     QMessageBox* msgBox = CustomBoxUtil::createCritialBox(windowTitle, text, info, &parent);

//     ASSERT_NE(msgBox, nullptr);
//     EXPECT_TRUE(msgBox->windowTitle().isEmpty());
//     EXPECT_EQ(msgBox->text(), text);
//     EXPECT_EQ(msgBox->informativeText(), info);
//     EXPECT_EQ(msgBox->icon(), QMessageBox::Critical);
//     EXPECT_EQ(msgBox->buttons().size(), 1);

//     QPushButton* button = qobject_cast<QPushButton*>(msgBox->buttons().at(0));
//     ASSERT_NE(button, nullptr);
//     EXPECT_EQ(button->text(), QObject::tr("OK"));

//     delete msgBox;
// }

// TEST_F(UnitTest_CustomBoxUtil, TestCreateCritialBoxWithDefaultParent) {
//     QString windowTitle = "Critical Error";
//     QString text = "An error has occurred.";
//     QString info = "Please contact support.";

//     QMessageBox* msgBox = CustomBoxUtil::createCritialBox(windowTitle, text, info);

//     ASSERT_NE(msgBox, nullptr);
//     EXPECT_EQ(msgBox->windowTitle(), windowTitle);
//     EXPECT_EQ(msgBox->text(), text);
//     EXPECT_EQ(msgBox->informativeText(), info);
//     EXPECT_EQ(msgBox->icon(), QMessageBox::Critical);
//     EXPECT_EQ(msgBox->buttons().size(), 1);

//     QPushButton* button = qobject_cast<QPushButton*>(msgBox->buttons().at(0));
//     ASSERT_NE(button, nullptr);
//     EXPECT_EQ(button->text(), QObject::tr("OK"));

//     delete msgBox;
// }
