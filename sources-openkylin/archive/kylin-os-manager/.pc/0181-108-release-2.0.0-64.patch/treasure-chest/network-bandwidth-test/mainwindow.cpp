#include <QMessageBox>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QRect>
#include "mainwindow.h"

namespace networkbt
{


MainWindow::MainWindow(KWidget *parent)
    : KWidget(parent),
      m_btnServer(new QPushButton(this)),
      m_btnClient(new QPushButton(this)),
      m_stackWidget(new QStackedWidget(this)),
      m_clientPage(new PageClient(this)),
      m_serverPage(new PageServer(this))
{
    setFixedSize(452, 564);
    this->setIcon(QIcon::fromTheme("kylin-iperf")); // 设置图标
    this->setWidgetName(tr("Network BandWidth Test Tool"));
    this->setBackgroundRole(QPalette::Base);
    windowButtonBar()->maximumButton()->hide(); 
    windowButtonBar()->menuButton()->hide();

    strServer = tr("server");
    strClient = tr("client");

    m_btnClient->setText(strClient);
    m_btnServer->setText(strServer);

    m_btnServer->setProperty("isImportant", true);
    m_btnServer->setFlat(false);
    m_btnClient->setFlat(true);

    m_stackWidget->addWidget(m_serverPage);
    m_stackWidget->addWidget(m_clientPage);    
    m_stackWidget->setCurrentIndex(0);

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_btnServer);
    hboxLayout->addWidget(m_btnClient);
    hboxLayout->addStretch();
    hboxLayout->setSpacing(0);
    hboxLayout->setMargin(0);

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(hboxLayout);
    vboxLayout->addWidget(m_stackWidget);

    baseBar()->setLayout(vboxLayout);

    connect(m_btnClient, &QPushButton::clicked, this, &MainWindow::slotClickClient);
    connect(m_btnServer, &QPushButton::clicked, this, &MainWindow::slotClickServer);

    // 移动到屏幕中间
    auto primary_screen = QGuiApplication::primaryScreen()->geometry();
    auto x = primary_screen.width() / 2 - this->width() / 2;
    auto y = primary_screen.height() / 2 - this->height() / 2;
    move(x, y);
}

MainWindow::~MainWindow()
{
}

void MainWindow::slotClickServer()
{
    m_stackWidget->setCurrentIndex(0);	
    m_btnServer->setProperty("isImportant", true);
    m_btnServer->setFlat(false);
    m_btnClient->setFlat(true);												
}

void networkbt::MainWindow::slotClickClient()
{
    m_stackWidget->setCurrentIndex(1);	
    m_btnClient->setProperty("isImportant", true);
    m_btnClient->setFlat(false);
    m_btnServer->setFlat(true);

}

}