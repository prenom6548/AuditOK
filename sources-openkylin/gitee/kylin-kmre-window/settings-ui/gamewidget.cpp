/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gamewidget.h"

#include "addgamewidget.h"
#include "removegamewidget.h"
#include "navgationbutton.h"
#include "global.h"
#include "utils.h"
#include "ukui-wayland/ukui-decoration-manager.h"

#include <QStackedWidget>
#include <QStackedLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <sys/syslog.h>

#define TITLE_WIDGET_HEIGHT 46
using namespace Global;
using namespace kmre;
GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
//    , m_stackedWidget(new QStackedWidget(this))
    , m_titleWidget(new QWidget)
//    , m_navButtonsGroup(new QButtonGroup)
    , m_contentFrame(new QFrame(this))
    , m_removeGameWidget(new RemoveGameWidget)
    , m_refreshBtn(new QPushButton(tr("Refresh")))
{
//    m_stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const QString &configPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    if (!QDir(configPath).exists("kmre")) {
        QDir(configPath).mkdir("kmre");
    }
    m_confName = QDir::homePath() + "/.config/kmre/render_gles";
    m_qsettings = new QSettings(m_confName, QSettings::IniFormat, this);
    m_qsettings->setIniCodec("UTF-8");

//    m_addGameWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_removeGameWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_contentFrame->setStyleSheet("QFrame{background-color: rgba(255, 255, 255, .03);}");
    m_removeGameWidget->setFixedSize(528,344);
    QFrame *centralWidget = new QFrame;
    m_view = new GameListView;
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_view->refreshList();
//    m_view->setFrameShape(QFrame::NoFrame);
    QWidget *titlebar = new QWidget(this);
    titlebar->setFixedSize(400,36);
    QHBoxLayout *layout = new QHBoxLayout(titlebar);
    QLabel *appName = new QLabel;
    QLabel *pkgName = new QLabel;
    QPushButton *line = new QPushButton;
    appName->setText(tr("appName"));
    pkgName->setText(tr("pkgName"));
    line->setFixedSize(1,11);
    line->setEnabled(false);
    layout->addWidget(appName);
    layout->addSpacing(70);
    layout->addWidget(line);
    layout->addWidget(pkgName);
    layout->addStretch();
//    layout->setMargin(0);
    titlebar->setLayout(layout);
    QVBoxLayout *m_vLayout = new QVBoxLayout;
    m_vLayout->setSpacing(0);
    m_vLayout->setMargin(0);
    m_vLayout->addWidget(titlebar);
    m_vLayout->addWidget(m_view);
    centralWidget->setLayout(m_vLayout);
    centralWidget->setObjectName("central");
    centralWidget->setStyleSheet("QFrame#central{border: 1px solid #E9E9E9;}");
    //title
    m_titleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleWidget->setFixedHeight(TITLE_WIDGET_HEIGHT);

    QPushButton *addTabBtn = new QPushButton;
//    QPushButton *addTabBtn = new NavgationButton(NavgationButton::FringePosition::Bottom);
//    m_navButtonsGroup->addButton(addTabBtn);
//    addTabBtn->setStyleSheet("QPushButton{border-radius: 6px;background:#E6E6E6;}");
    addTabBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));
    addTabBtn->setFixedSize(36, 36);
    QPushButton *remomveTabBtn = new QPushButton;
//    QPushButton *remomveTabBtn = new NavgationButton(NavgationButton::FringePosition::Bottom);
//    m_navButtonsGroup->addButton(remomveTabBtn);
//    remomveTabBtn->setStyleSheet("QPushButton{border-radius: 6px;background:#E6E6E6;}");
    remomveTabBtn->setIcon(QIcon::fromTheme("list-remove-symbolic"));
    remomveTabBtn->setFixedSize(36, 36);
    addTabBtn->setFocusPolicy(Qt::NoFocus);
    remomveTabBtn->setFocusPolicy(Qt::NoFocus);
    m_refreshBtn->setFocusPolicy(Qt::NoFocus);
    m_refreshBtn->setFixedSize(96, 36);
    //addTabBtn->setStyleSheet("QPushButton{background:transparent;border:none;text-align:center;color:palette(Text);}");//font-size:14px;
    //remomveTabBtn->setStyleSheet("QPushButton{background:transparent;border:none;text-align:center;color:palette(Text);}");//font-size:14px;
    QHBoxLayout *titlelayout = new QHBoxLayout(m_titleWidget);
    titlelayout->setMargin(0);
    titlelayout->setSpacing(8);
//    titlelayout->addStretch();
    titlelayout->addWidget(addTabBtn, 0, Qt::AlignLeft);
    titlelayout->addWidget(remomveTabBtn, 0, Qt::AlignLeft);
    titlelayout->addStretch();
    titlelayout->addWidget(m_refreshBtn, 0,Qt::AlignVCenter | Qt::AlignRight);

    /*QStackedLayout *layout = new QStackedLayout;
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(m_removeGameWidget);
    layout->addWidget(m_addGameWidget);
    layout->setCurrentWidget(m_removeGameWidget);
    m_contentFrame->setLayout(layout);

    m_stackedWidget->addWidget(m_contentFrame);
    m_stackedWidget->setCurrentWidget(m_contentFrame);*/

    QLabel *m_title = new QLabel;
    m_title->setText(tr("Game Setting"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    m_title->setMargin(0);
    m_astcCheckBox = new QCheckBox();
    m_astcCheckBox->setChecked(false);
    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(tr("Enable ASTC texture support(When turned on, the picture quality is clearer)"), Qt::ElideRight, 500);
    m_astcCheckBox->setText(elided_text);
    if (fm.width(tr("Enable ASTC texture support(When turned on, the picture quality is clearer)")) > 500) {
        m_astcCheckBox->setToolTip(tr("Enable ASTC texture support(When turned on, the picture quality is clearer)"));
    }
    QLabel *m_tiplabel = new QLabel;
    m_tiplabel->setText(tr("When added to the list, the app will enable the Game button feature:"));
    m_tiplabel->setFixedHeight(20);
//    m_tiplabel->setContentsMargins(8,0,0,0);
//    m_tiplabel->setMargin(0);
    QVBoxLayout *mainlayout = new  QVBoxLayout;
//    mainlayout->setContentsMargins(30,0,39,20);
    mainlayout->setContentsMargins(30, 2, 24, 40);
    mainlayout->addWidget(m_title);
//    mainlayout->setMargin(0);
    mainlayout->setSpacing(0);
    mainlayout->addSpacing(16);
    mainlayout->addWidget(m_astcCheckBox);
//    mainlayout->addSpacing(24);
//    mainlayout->addWidget(m_tiplabel);
//    mainlayout->addSpacing(8);
//    mainlayout->addWidget(centralWidget);
//    mainlayout->addSpacing(8);
//    mainlayout->addWidget(m_titleWidget,0 ,Qt::AlignBottom);
    mainlayout->addStretch();
    this->setLayout(mainlayout);
    this->loadAtscValue();
    connect(addTabBtn, &QPushButton::clicked, this, [=] {
        m_addGameWidget = new AddGameWidget;
        m_addGameWidget->setFixedSize(480,268);
        m_addGameWidget->move(mapToGlobal(QPoint(0,0)) + QPoint(width() - m_addGameWidget->width(), height() - m_addGameWidget->height())/2);
        m_addGameWidget->show();
#ifdef UKUI_WAYLAND
        UKUIDecorationManager::getInstance()->removeHeaderBar(m_addGameWidget->windowHandle());
        UKUIDecorationManager::getInstance()->setCornerRadius(m_addGameWidget->windowHandle(), 12, 12, 12, 12);
        m_addGameWidget->setFixedSize(m_addGameWidget->size());
#endif
    });
    connect(m_astcCheckBox, &QCheckBox::toggled, [&](bool checked) {
        if (m_qsettings) {
            m_qsettings->beginGroup("astc");
            m_qsettings->setValue("astc", checked);
            m_qsettings->endGroup();
            m_qsettings->sync();
        }
    });
    connect(m_view,SIGNAL(Clickeditem(QString,QString)),this,SLOT(onClickeditem(QString,QString)));
    connect(m_refreshBtn, SIGNAL(clicked()), m_view, SLOT(refreshList()));
//    connect(remomveTabBtn, &QPushButton::clicked, this, [=] () {
//    });
    connect(this, SIGNAL(requestClearGame(QString)), m_view, SLOT(onResponseClearGame(QString)));
}

void GameWidget::onClickeditem(const QString &pkgname,const QString &appname)
{
    m_pkgName = pkgname;
    m_appName = appname;
}

void GameWidget::loadAtscValue()
{
    if (m_qsettings) {
        m_qsettings->beginGroup("astc");
        bool b = m_qsettings->value("astc", false).toBool();
        m_qsettings->endGroup();
        m_astcCheckBox->setChecked(b);
    }
}

GameWidget::~GameWidget()
{
    if (m_contentFrame) {
        delete m_contentFrame;
        m_contentFrame = nullptr;
    }
    if (m_qsettings) {
        delete m_qsettings;
        m_qsettings = nullptr;
    }
//    if (m_stackedWidget) {
//        delete m_stackedWidget;
//        m_stackedWidget = nullptr;
//    }
//    QLayoutItem *child;
//    while ((child = m_layout->takeAt(0)) != 0) {
//        if (child->widget())
//            child->widget()->deleteLater();
//        delete child;
//    }
}

//void GameWidget::resizeEvent(QResizeEvent *e)
//{
//    Q_UNUSED(e)

//    if (m_titleWidget) {
//        m_titleWidget->resize(width() - 2, TITLE_WIDGET_HEIGHT);
//    }
//    if (m_stackedWidget) {
//        m_stackedWidget->resize(width() - 2, this->height() - TITLE_WIDGET_HEIGHT - 2);
//        m_stackedWidget->move(1, TITLE_WIDGET_HEIGHT + 1);
//    }
//}
