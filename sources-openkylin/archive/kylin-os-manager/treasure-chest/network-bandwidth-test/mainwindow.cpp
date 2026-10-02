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
      m_tabBar(new kdk::KTabBar(kdk::KTabBarStyle::SegmentLight, this)),
      m_stackWidget(new QStackedWidget(this)),
      m_clientPage(new PageClient(this)),
      m_serverPage(new PageServer(this))
{
    setFixedSize(452, 564);
    setIcon(QIcon::fromTheme("kylin-iperf")); // 设置图标
    setWidgetName(tr("Network BandWidth Test Tool"));
    setBackgroundRole(QPalette::Base);

    windowButtonBar()->maximumButton()->hide(); 
    windowButtonBar()->menuButton()->hide();

    m_tabBar->setFixedSize(200, 36);
    m_tabBar->addTab(tr("server"));
    m_stackWidget->addWidget(m_serverPage);
    m_tabBar->addTab(tr("client"));
    m_stackWidget->addWidget(m_clientPage);    
    m_tabBar->setCurrentIndex(0);
    m_stackWidget->setCurrentIndex(0);

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->setSpacing(0);
    hboxLayout->setMargin(0);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_tabBar);
    hboxLayout->addStretch();

    QVBoxLayout *vboxLayout = new QVBoxLayout;
    vboxLayout->addLayout(hboxLayout);
    vboxLayout->addWidget(m_stackWidget);

    baseBar()->setLayout(vboxLayout);

    connect(m_tabBar, &kdk::KTabBar::currentChanged, this, &MainWindow::on_indexChanged);

    // 移动到屏幕中间
    auto primary_screen = QGuiApplication::primaryScreen()->geometry();
    auto x = primary_screen.width() / 2 - this->width() / 2;
    auto y = primary_screen.height() / 2 - this->height() / 2;
    move(x, y);
}

MainWindow::~MainWindow() {

}

void MainWindow::on_indexChanged(int index) {
    m_stackWidget->setCurrentIndex(index);
}

}
