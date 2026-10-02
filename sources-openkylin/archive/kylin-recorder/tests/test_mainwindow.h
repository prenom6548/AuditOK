#ifndef TESTMAINWINDOW_H
#define TESTMAINWINDOW_H
#include <QObject>
#include <QtTest/QTest>
#include "playcontrol.h"
#include "menumodule.h"
#include "mainwindow.h"

class TestMainWindow : public QObject
{
    Q_OBJECT
public:
    explicit TestMainWindow(QObject *parent = nullptr);
//    MainWindow* window { nullptr };
    PlayControl *playcon = nullptr;
    menuModule *m_menuModule = nullptr;
private slots:
    void test_construct();
   void test_record();
    void test_fileitem();
    void test_mpv();
    void test_menu();
    void test_mini();
    void test_database();
    void test_rename();
   void test_save();
    void test_label();
    void test_all();
    void unit_log();

    void test_tool();
private:
    MainWindow* window { nullptr };
};
#endif // TEST_MAINWINDOW_H
