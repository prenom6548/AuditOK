#ifndef TEST_LINEEDIT_H
#define TEST_LINEEDIT_H

#include <QObject>
#include <QtTest/QTest>
#include "lineedit.h"

class TestLineEdit : public QObject
{
    Q_OBJECT

public:
    explicit TestLineEdit(QObject *parent = nullptr);

private slots:
    void test_construct();
    void test_mousePressEvent();
};

#endif // TEST_LINEEDIT_H