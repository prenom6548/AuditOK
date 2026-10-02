#ifndef TEST_MENUFACTORY_H
#define TEST_MENUFACTORY_H

#include <QObject>
#include <QtTest/QtTest>
#include <QApplication>
#include <QMenu>
#include <QToolButton>
#include <QAction>
#include <QSize>
#include <QList>
#include "../../../../../UIControl/src/include/menufactory.h"

class TestMenuFactory : public QObject
{
    Q_OBJECT

public:
    TestMenuFactory();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testInitFunction();
    void testMenuButtonProperties();
    void testMenuActions();
    void testUnusedMemberVariables();
    void testMultipleInstances();

private:
    MenuFactory *m_menuFactory;
};

#endif // TEST_MENUFACTORY_H