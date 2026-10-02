#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <kwidget.h>
#include <ktabbar.h>
#include "pageclient.h"
#include "pageserver.h"

using namespace kdk;

namespace networkbt {

class MainWindow : public KWidget {
    Q_OBJECT
public:
    MainWindow(KWidget *parent = nullptr);
    ~MainWindow();

private:
    void on_indexChanged(int index);

    kdk::KTabBar *  m_tabBar      = nullptr;
    QStackedWidget *m_stackWidget = nullptr;
    PageClient *    m_clientPage  = nullptr;
    PageServer *    m_serverPage  = nullptr;
};

}

#endif
