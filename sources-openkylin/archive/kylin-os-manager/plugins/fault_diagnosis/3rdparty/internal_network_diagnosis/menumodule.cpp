/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QTime>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include "menumodule.h"
#include "size_para.h"
#include "usermanual.h"
//#include "gsettings_monitor.h"

// using namespace kdk::kabase;

static const QString APP_NAME = "kylin-netcheck-tools";

MenuModule::MenuModule(QWidget *parent) : QWidget(parent), m_mainWin(parent)
{
    init();
}

void MenuModule::init()
{
    initAction();
    setStyle();
}

void MenuModule::initAction()
{
    m_bodySupport = new QLabel();
    m_titleText = new QLabel();
    m_bodyAppName = new QLabel();
    m_bodyAppVersion = new QLabel();
    m_bodyAppDescribe = new QLabel();
    m_iconSize = QSize(30, 30);
    m_menuButton = new QToolButton(this);
    m_menuButton->setProperty("isWindowButton", 0x1);
    m_menuButton->setProperty("useIconHighlightEffect", 0x2);
    m_menuButton->setPopupMode(QToolButton::InstantPopup);
    m_menuButton->setFixedSize(30, 30);
    m_menuButton->setIconSize(QSize(16, 16));
    m_menuButton->setAutoRaise(true);
    m_menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));

    m_menu = new QMenu();

    QList<QAction *> actions;

    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    QAction *actionConfig = new QAction(m_menu);
    actionConfig->setText(tr("Configure"));
    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));
    actions << actionConfig << actionHelp << actionAbout << actionQuit;

    m_menu->addActions(actions);

    m_menuButton->setMenu(m_menu);
    connect(m_menu, &QMenu::triggered, this, &MenuModule::triggerMenu);
    initGsetting();
    setStyleByThemeGsetting();
}


void MenuModule::setStyleByThemeGsetting()
{
    //    QString nowThemeStyle = GsettingsMonitor::getInstance()->sysStyleNameGet();
    //    if("ukui-dark" == nowThemeStyle || "ukui-black" == nowThemeStyle)
    //    {
    //        setThemeDark();
    //    }else{
    //        setThemeLight();
    //    }
}

void MenuModule::triggerMenu(QAction *act)
{
    QString str = act->text();
    if (tr("Quit") == str) {
        emit menuModuleClose();
    } else if (tr("About") == str) {
        aboutAction();
    } else if (tr("Help") == str) {
        //        helpAction();
        showUserManual();
    } else if (tr("Configure") == str) {
        emit showConfigureWin();
    }
}

void MenuModule::aboutAction()
{
    //修改bug，改为使用SDK控件
    initAboutSDK();
    //    关于点击事件处理
    //    if (m_aboutWindow != nullptr) {
    //        m_aboutWindow->hide();

    //        QTime dieTime = QTime::currentTime().addMSecs(50);
    //        while( QTime::currentTime() < dieTime )
    //            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    //        m_aboutWindow->move(mainWinCenter - m_aboutWindow->rect().center());
    //        m_aboutWindow->show();
    //        return ;
    //    }
    //    initAbout();
}
//
void MenuModule::initAboutSDK()
{
    KAboutDialog m_aboutWindowSdk;
    m_aboutWindowSdk.setAppIcon(QIcon(":/data/Applogo.png"));
    m_aboutWindowSdk.setAppName(m_appShowingName);
    m_aboutWindowSdk.setAppVersion(tr("Version: ") + m_appVersion);
    m_aboutWindowSdk.setBodyText(tr("Network-check-tool is a software that can quickly detect,diagnose,"
                                    "and optimize networks."));
    m_aboutWindowSdk.setBodyTextVisiable(true);
    QPoint mainWinCenter =
        QPoint(m_mainWin->mapToGlobal(QPoint(0, 0)).x() + m_mainWin->width() / 2, m_mainWin->mapToGlobal(QPoint(0, 0)).y() + m_mainWin->height() / 2);
    m_aboutWindowSdk.move(mainWinCenter - m_aboutWindowSdk.rect().center());
    m_aboutWindowSdk.exec();
}
void MenuModule::helpAction()
{
    //    帮助点击事件处理
    //    m_helpWindow = new HelpManual(this);

    //    MotifWmHints hints;
    //    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    //    hints.functions = MWM_FUNC_ALL;
    //    hints.decorations = MWM_DECOR_BORDER;
    //    XAtomHelper::getInstance()->setWindowMotifHint(m_helpWindow->winId(), hints);

    //    QPoint mainWinCenter = QPoint(m_mainWin->mapToGlobal(QPoint(0,0)).x() + m_mainWin->width()/2,
    //                                  m_mainWin->mapToGlobal(QPoint(0,0)).y() + m_mainWin->height()/2);
    //    m_helpWindow->move(mainWinCenter - m_helpWindow->rect().center());
    //    m_helpWindow->activateWindow();
    //    m_helpWindow->show();
}
void MenuModule::showUserManual()
{
    kdk::UserManual userManualTest;
    if (!userManualTest.callUserManual(APP_NAME)) {
        qCritical() << "user manual call fail!";
    }

    return;
}
void MenuModule::initAbout()
{
    m_aboutWindow = new QDialog(this);
    m_aboutWindow->setWindowModality(Qt::ApplicationModal);
    m_aboutWindow->setWindowFlag(Qt::Tool);
    m_aboutWindow->setAutoFillBackground(true);
    m_aboutWindow->setBackgroundRole(QPalette::Base);

    //修改bug122163 关于界面
    m_aboutWindow->setFixedSize(420, 384);
    m_aboutWindow->setMinimumHeight(384);
    QVBoxLayout *mainlyt = new QVBoxLayout();
    QHBoxLayout *titleLyt = initTitleBar();
    QVBoxLayout *bodylyt = initBody();
    mainlyt->setContentsMargins(0, 0, 0, 0);
    mainlyt->setSpacing(0);
    mainlyt->addLayout(titleLyt);
    mainlyt->addLayout(bodylyt);
    mainlyt->addStretch();
    m_aboutWindow->setLayout(mainlyt);
    QPoint mainWinCenter =
        QPoint(m_mainWin->mapToGlobal(QPoint(0, 0)).x() + m_mainWin->width() / 2, m_mainWin->mapToGlobal(QPoint(0, 0)).y() + m_mainWin->height() / 2);
    m_aboutWindow->move(mainWinCenter - m_aboutWindow->rect().center());

    refreshThemeBySystemConf();
    m_aboutWindow->show();
}

QHBoxLayout *MenuModule::initTitleBar()
{
    m_titleIconBtn = new QPushButton();
    m_titleIconBtn->setIcon(QIcon(":/data/titleIcon.png"));
    m_titleIconBtn->setIconSize(QSize(24, 24));
    m_titleIconBtn->setFixedSize(QSize(24, 24));
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_titleIconBtn->setStyleSheet(btnStyle);



    QPushButton *titleBtnClose = new QPushButton();
    titleBtnClose->setFixedSize(30, 30);
    titleBtnClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    titleBtnClose->setFocusPolicy(Qt::NoFocus);
    titleBtnClose->setProperty("isWindowButton", 0x2);
    titleBtnClose->setProperty("useIconHighlightEffect", 0x8);
    titleBtnClose->setFlat(true);
    connect(titleBtnClose, &QPushButton::clicked, [=]() {
        m_aboutWindow->close();
    });

    m_titleText->setText(tr(m_appShowingName.toLocal8Bit()));


    QHBoxLayout *hlyt = new QHBoxLayout();
    hlyt->setSpacing(0);
    hlyt->setContentsMargins(4, 4, 4, 4);
    hlyt->addSpacing(4);
    hlyt->addWidget(m_titleIconBtn); //居下显示
    hlyt->addSpacing(8);
    hlyt->addWidget(m_titleText);
    hlyt->addStretch();
    hlyt->addWidget(titleBtnClose);

    return hlyt;
}

QVBoxLayout *MenuModule::initBody()
{
    m_bodyIcon = new QPushButton();
    m_bodyIcon->setIcon(QIcon(":/data/Applogo.png"));
    m_bodyIcon->setIconSize(QSize(96, 96));
    m_bodyIcon->setFixedSize(QSize(96, 96));
    QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                       "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_bodyIcon->setStyleSheet(btnStyle);


    m_bodyAppName->setFixedHeight(28);
    m_bodyAppName->setText(tr(m_appShowingName.toLocal8Bit()));

    m_bodyAppVersion->setFixedHeight(24);
    m_bodyAppVersion->setText(tr("Version: ") + m_appVersion);
    m_bodyAppVersion->setAlignment(Qt::AlignLeft);

    m_bodyAppDescribe->setFixedSize(356, 60);
    m_bodyAppDescribe->setText(tr("Network-check-tool is a software that can quickly detect, diagnose, "
                                  "and optimize networks. "));
    m_bodyAppDescribe->setAlignment(Qt::AlignLeft);
    m_bodyAppDescribe->setWordWrap(true);
    m_bodyAppDescribe->adjustSize();

    connect(m_bodySupport, &QLabel::linkActivated, this, [=](const QString url) {
        QDesktopServices::openUrl(QUrl(url));
    });
    m_bodySupport->setContextMenuPolicy(Qt::NoContextMenu);
    m_bodySupport->setFixedSize(356, 28);
    m_bodySupport->setMinimumHeight(28);
    m_bodySupport->setText(tr("Service & Support: ")
                           + "<a href=\"mailto://support@kylinos.cn\""
                             "style=\"color:rgba(225,225,225,1)\">"
                             "support@kylinos.cn</a>");
    //    m_bodySupport->setText("服务支持");
    m_bodySupport->setAlignment(Qt::AlignLeft);
    QVBoxLayout *vlyt = new QVBoxLayout();
    vlyt->setContentsMargins(32, 0, 32, 40);
    vlyt->setSpacing(0);
    vlyt->addSpacing(20);
    vlyt->addWidget(m_bodyIcon, 0, Qt::AlignHCenter);
    vlyt->addSpacing(16);
    vlyt->addWidget(m_bodyAppName, 0, Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(m_bodyAppVersion, 0, Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(m_bodyAppDescribe, 0, Qt::AlignHCenter);
    vlyt->addSpacing(12);
    vlyt->addWidget(m_bodySupport, 0, Qt::AlignHCenter);
    vlyt->addStretch();
    return vlyt;
}
/*
 * if（系统代理打开）
 * {
 *     if(打开内网检测）{
 *          return ping内网result
 *      } else {
 *          return ping baidu.com result
 *      }
 * } else {
 *      return 未启动系统代理
 * }
 * 1.检查是否打开系统代理
 *   - 系统代理打开
 *      - 检查内网检测是否开启
 *          - 内网检测开启
 */
void MenuModule::setStyle()
{
    m_menuButton->setObjectName("menuButton");
    // qDebug() << "menuButton->styleSheet" << menuButton->styleSheet();
    m_menuButton->setStyleSheet("QPushButton::menu-indicator{image:None;}");
}

void MenuModule::initGsetting()
{
    const QByteArray idd(UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(idd)) {
        m_themeSettings = new QGSettings(UKUI_STYLE);
    }

    if (m_themeSettings) {
        connect(m_themeSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                auto curStyle = m_themeSettings->get("styleName");
                QString tmpStyleName = curStyle.toString();
                if (tmpStyleName != m_curStyle) {
                    m_curStyle = tmpStyleName;
                    refreshThemeBySystemConf();
                }
            }
        });
        auto curStyle = m_themeSettings->get("styleName");
        QString tmpStyleName = curStyle.toString();
        if (tmpStyleName != m_curStyle) {
            m_curStyle = tmpStyleName;
            refreshThemeBySystemConf();
        }
    }
}

void MenuModule::refreshThemeBySystemConf()
{
    if ("ukui-dark" == m_curStyle || "ukui-black" == m_curStyle) {
        setThemeDark();
    } else {
        setThemeLight();
    }
}

void MenuModule::setThemeDark()
{
    m_bodySupport->setText(tr("Service & Support: ")
                           + "<a href=\"mailto://support@kylinos.cn\""
                             "style=\"color:rgba(225,225,225,1)\">"
                             "support@kylinos.cn</a>");
}

void MenuModule::setThemeLight()
{
    m_bodySupport->setText(tr("Service & Support: ")
                           + "<a href=\"mailto://support@kylinos.cn\""
                             "style=\"color:rgba(0,0,0,1)\">"
                             "support@kylinos.cn</a>");
}

void MenuModule::setQSSFontSize(QFont curFont)
{
    //带QSS的控件设置字体ui->xx->setFont(font);
    double Font14Size = curFont.pointSizeF() / 15 * 14;
    QFont font14 = curFont;
    font14.setPointSizeF(Font14Size);

    m_titleText->setFont(font14);
    m_bodySupport->setFont(font14);
    m_bodyAppName->setFont(font14);
    m_bodyAppVersion->setFont(font14);
    m_bodyAppDescribe->setFont(font14);
    m_menu->setFont(font14);
}
