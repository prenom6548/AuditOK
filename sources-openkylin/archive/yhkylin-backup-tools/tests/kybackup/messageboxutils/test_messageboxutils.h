#ifndef TEST_MESSAGEBOXUTILS_H
#define TEST_MESSAGEBOXUTILS_H

#include <QObject>
#include <QtTest/QtTest>
#include <QApplication>
#include "../../../kybackup/messageboxutils.h"

class TestMessageBoxUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testMyMessageBoxConstructor();
    void testSetThemeIcon();
    void testSetMsgIcon();
    void testMessageBoxUtilsInformation();
    void testMessageBoxUtilsWarning();
    void testMessageBoxUtilsWarningCancel();
    void testMessageBoxUtilsCritical();

private:
    QWidget* m_testWidget;
};

#endif // TEST_MESSAGEBOXUTILS_H