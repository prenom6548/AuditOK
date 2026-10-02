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

#include "addgamewidget.h"

#include "settingsgroup.h"
#include "inputitem.h"
#include "global.h"
#include "ukui-wayland/ukui-decoration-manager.h"
#include "xatom-helper.h"
#include "messagebox.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QDBusReply>
#include <QtDBus>
#include <QMessageBox>
#include <QDBusInterface>
#include <QCheckBox>
#include <QSettings>
#include <QPalette>
#include <QX11Info>
#include <QMouseEvent>

using namespace Global;
//TODO: write /usr/share/kmre/games.json with root
bool setGameWhiteList(const QString &appName, const QString &pkgName)
{
    bool ret = false;

    return ret;
}


AddGameWidget::AddGameWidget(QWidget * parent)
    : SettingScrollContent(parent)
    , m_group(new SettingsGroup)
    , m_resetButton(new QPushButton)
    , m_okButton(new QPushButton)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0x00,0xff,0x00,0x00));
    setPalette(pal);

    this->addItem();
    this->setBackgroundRole(QPalette::NoRole);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFixedSize(480,268);
    this->setWindowFlags(Qt::FramelessWindowHint);

    this->setAutoFillBackground(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setMouseTracking(true);

#ifndef KYLIN_V10
#ifndef UKUI_WAYLAND
    // 添加窗管协议
    if (QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
#endif
#endif

    initUi();
    initConnection();
#ifdef UKUI_WAYLAND
        installEventFilter(this);
#endif
}

AddGameWidget::~AddGameWidget()
{
    for (InputItem *item :m_itemList) {
        if (item) {
            item->deleteLater();
        }
    }
}

void AddGameWidget::initUi()
{
    m_resetButton->setText(tr("Reset"));
    m_resetButton->setFixedSize(96,36);
//    m_resetButton->setStyleSheet("QPushButton{background: #E6E6E6;} QPushButton:hover{color:");
    m_okButton->setText(tr("OK"));
    m_okButton->setFixedSize(96,36);
    m_okButton->setStyleSheet("QPushButton{background:#3790FA;color:#FFFFFF;}");
    if (!isKylinOSPro() && !isOpenKylin()) {
        m_okButton->setStyleSheet("");
    }

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(m_okButton);
    hlayout->addSpacing(16);
    hlayout->addWidget(m_resetButton);
    hlayout->addSpacing(17);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_closeBtn->setFlat(true);
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setToolTip(tr("Close"));

    QLabel *tiplabel = new QLabel;
    tiplabel->setText(tr("App supporting game keys"));

//    m_group->setStyleSheet("QFrame{background:#FFFFFF;}");
    m_group->setAttribute(Qt::WA_TranslucentBackground, true);
//    QFrame *frame = new QFrame;
    mainWid = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;//(frame);
    layout->setContentsMargins(24,6,7,24);
    layout->addWidget(m_closeBtn,0,Qt::AlignRight);
    layout->addSpacing(12);
    layout->addWidget(tiplabel);
//    layout->addSpacing(20);
    layout->addWidget(m_group);
    layout->addSpacing(32);
    layout->addLayout(hlayout);
    mainWid->setLayout(layout);
    mainWid->setFixedSize(480,268);
    mainWid->setObjectName("mainWid");
    mainWid->setStyleSheet("#mainWid{background:palette(Base);border-radius:12px;border: 1px solid rgba(0, 0, 0, 0.14901960784313725);}");
}

void AddGameWidget::initConnection()
{
    connect(m_okButton, &QPushButton::clicked, this, [=] () {
        for (InputItem *item : m_itemList) {
            if (item) {
                QPair<QString, QString> result = item->getInformation();
                if (result.first.isEmpty() || result.second.isEmpty()) {
                    continue;
                }
//                auto future = QtConcurrent::run(setGameWhiteList, result.first, result.second);
//                auto *wather = new QFutureWatcher<bool>();
//                wather->setFuture(future);
//                connect(wather, &QFutureWatcher<bool>::finished, this, &GameWidget::onSetValueFinished);

                QDBusInterface interface("cn.kylinos.Kmre.Pref", "/cn/kylinos/Kmre/Pref", "cn.kylinos.Kmre.Pref", QDBusConnection::systemBus());
                QDBusReply<bool> reply = interface.call("addGameToWhiteList", result.first, result.second);
                if (reply.isValid()) {
                    bool ret = reply.value();
                    if (ret) {
                        KylinUI::MessageBox::information(this, tr("Tips"), tr("Add successfully, restart the Android application set to take effect!"));
                    }
                }

                break;
            }
        }
    });

    connect(m_resetButton, &QPushButton::clicked, this, [=] () {
        for (InputItem *item : m_itemList) {
            if (item) {
                item->clear();
            }
        }
    });

    connect(m_closeBtn, &QPushButton::clicked, this, [=]() {
//        this->close();
        this->hide();
    });
}

void AddGameWidget::addItem()
{
    InputItem *item = new InputItem();
    item->setContentsMargins(0, 0, 10, 0);
    item->setTitle(tr("Game name"), tr("Game package name"));
    item->setStyleSheet("QFrame{background:pallete(Base);}");
    m_group->appendItem(item);
    m_itemList.append(item);
}

void AddGameWidget::removeItem(const QString &pkg)
{
    for (InputItem *item : m_itemList) {
        if (item->package() == pkg) {
            m_group->removeItem(item);
            m_itemList.removeOne(item);
            item->deleteLater();
        }
    }
}

void AddGameWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
#ifdef UKUI_WAYLAND
    // wayland 窗口无边框，需要在窗口show()之后执行
    UKUIDecorationManager::getInstance()->removeHeaderBar(windowHandle());
    UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
#endif
}

#ifdef UKUI_WAYLAND
bool AddGameWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                UKUIDecorationManager::getInstance()->moveWindow(this->windowHandle());
            }
        }
    }
    return false;
}
#endif
