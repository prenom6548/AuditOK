#ifndef TEST_KYLINAPCONNECTITEM_H
#define TEST_KYLINAPCONNECTITEM_H

#include <QObject>
#include <QString>
#include <QtTest>
#include "kylinapconnectitem.h"

class TestKyApConnectItem : public QObject
{
    Q_OBJECT
private slots:
    void testDefaultConstructor();
    void testMemberVariables();
};

#endif // TEST_KYLINAPCONNECTITEM_H
