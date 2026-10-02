#ifndef TEST_CLIPBUTTON_H
#define TEST_CLIPBUTTON_H

#include <QObject>
#include <QtTest/QTest>
#include "clipbutton.h"

class TestClipButton : public QObject
{
    Q_OBJECT

public:
    explicit TestClipButton(QObject *parent = nullptr);

private slots:
    void test_construct();
    void test_paintEvent();
    void test_mousePressEvent();
    void test_mouseReleaseEvent();
    void test_mouseMoveEvent();

private:
    ClipButton* clipButton;
};

#endif // TEST_CLIPBUTTON_H