#include <QDebug>
#include <KWindowEffects>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <kaboutdialog.h>

#include "mainwidget.h"


MainWidget::MainWidget(QWidget *parent)
    : kdk::KWidget(parent),
      m_stackedWid(new QStackedWidget(this))
{
    initWidgetUI();
    initLayout();
    // initSetStyle();
    initConn();
}

MainWidget::~MainWidget()
{

}

MainWidget *MainWidget::getInstance()
{
    static MainWidget *instance = nullptr;
    if (nullptr == instance) {
        instance = new MainWidget();
    }
    return instance;
}

void MainWidget::adjustBackground()
{
    QColor sideColor = QApplication::palette().base().color();
    sideColor.setAlphaF(tranSparency);
    QPalette sidePalette = this->palette();
    sidePalette.setColor(QPalette::Base, sideColor);

    iconBar()->setAutoFillBackground(true);
    iconBar()->setBackgroundRole(QPalette::Base);
    iconBar()->setPalette(sidePalette);

    baseBar()->setAutoFillBackground(true);
    baseBar()->setBackgroundRole(QPalette::Base);
    baseBar()->setPalette(sidePalette);

    windowButtonBar()->setAutoFillBackground(true);
    windowButtonBar()->setBackgroundRole(QPalette::Base);
    windowButtonBar()->setPalette(sidePalette);
}

void MainWidget::initWidgetUI()
{
    this->setFixedSize(GlobalData::g_sizeMainWidget);
    windowButtonBar()->maximumButton()->hide(); // 最大按钮隐藏
    auto themeAction = windowButtonBar()->menuButton()->themeAction();
    auto settingAction = windowButtonBar()->menuButton()->settingAction();
    // auto aboutAction = windowButtonBar()->menuButton()->aboutAction();
    // auto quitAction = windowButtonBar()->menuButton()->quitAction();
    auto assistAction = windowButtonBar()->menuButton()->assistAction();
    windowButtonBar()->menuButton()->menu()->removeAction(themeAction);
    windowButtonBar()->menuButton()->menu()->removeAction(settingAction);
    // windowButtonBar()->menuButton()->menu()->removeAction(aboutAction);
    // windowButtonBar()->menuButton()->menu()->removeAction(quitAction);
    windowButtonBar()->menuButton()->menu()->removeAction(assistAction);

    connect(windowButtonBar()->menuButton()->menu(), &QMenu::triggered, this, &MainWidget::slotTriggerMenu);


    m_homePage = HomePageWidget::getInstance();
    // m_chooseDevice = ChooseDeviceWidget::getInstance();
    m_chooseWidget = new ChooseWidget(this);
}

void MainWidget::initLayout()
{
    m_stackedWid->addWidget(m_homePage);
    m_stackedWid->addWidget(m_chooseWidget);
    m_stackedWid->setCurrentIndex(0);
    auto *vBoxLayout = new QVBoxLayout;
    vBoxLayout->addWidget(m_stackedWid);
    vBoxLayout->setSpacing(0);
    vBoxLayout->setMargin(0);
    baseBar()->setLayout(vBoxLayout);
}

void MainWidget::initSetStyle()
{
    if (QGSettings::isSchemaInstalled("org.ukui.control-center.personalise")) {
        m_pTransparency = new QGSettings("org.ukui.control-center.personalise");
        tranSparency = m_pTransparency->get("transparency").toDouble();
        connect(m_pTransparency, &QGSettings::changed, this, [=](QString value) {
            if (value == "transparency") {
                tranSparency = m_pTransparency->get("transparency").toDouble();
                adjustBackground();
            }
        });
        adjustBackground();
    }
}

void MainWidget::initConn()
{
    connect(m_homePage, &HomePageWidget::sinStatClick, this, [this](){
        m_stackedWid->setCurrentIndex(1);
        initSetStyle();
    });

}

void MainWidget::slotTriggerMenu(QAction *act)
{
    QString str = act->text();
    qDebug() << str;
    if (windowButtonBar()->menuButton()->aboutAction()->text() == str) {
        initAbout();
    } else if (windowButtonBar()->menuButton()->quitAction()->text() == str) {
        initQuit();
    } else if (windowButtonBar()->menuButton()->assistAction()->text() == str) {
        initHelp();
    }
    return;
}

void MainWidget::initQuit()
{
    this->close();
}

void MainWidget::initHelp()
{
}

void MainWidget::initAbout()
{
    kdk::KAboutDialog aboutWindow(this, QIcon::fromTheme("kylin-replacement"), tr("replacement"), tr("Version: ") + "0.0.0");
    aboutWindow.setBodyText(tr("Message provides text chat and file transfer functions in the LAN. "
                      "There is no need to build a server. "
                      "It supports multiple people to interact at the same time "
                      "and send and receive in parallel."));
    aboutWindow.setBodyTextVisiable(false);
    aboutWindow.exec();
}