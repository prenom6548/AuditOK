#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <kwidget.h>
#include "pageclient.h"
#include "pageserver.h"

using namespace kdk;

namespace networkbt
{

class MainWindow : public KWidget
{
    Q_OBJECT

public:
    MainWindow(KWidget *parent = nullptr);
    ~MainWindow();

private:
    QPushButton *m_btnServer = nullptr;
    QPushButton *m_btnClient = nullptr;
    QStackedWidget *m_stackWidget = nullptr;
    PageClient *m_clientPage = nullptr;
    PageServer *m_serverPage = nullptr;
    

    QString strServer;
    QString strClient;

    void slotClickServer();
    void slotClickClient();
};

}

#endif