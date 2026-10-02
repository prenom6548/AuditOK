/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStringList>
#include <QFont>
#include "titlebar.h"
#include "mainwindow.h"
#include "funclist.h"
#include "InputSymbols.h"
#include "data_warehouse.h"
#include <QColor>
#include <QDesktopWidget>

#include <gsettingmonitor.h>

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    DataWarehouse::getInstance()->platform;
    /* handle intel ui */
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        createInterUi();
        createInterStyle();
    } else {
        // 初始化组件
        setWidgetUi();
        // 设置组件样式
        setWidgetStyle();
    }
}

TitleBar::~TitleBar() {}

/* intel title ui */
void TitleBar::createInterUi()
{
    this->setFixedHeight(TITLEH);

    this->STANDARD_LABEL = tr("standard");
    this->SCIENTIFIC_LABEL = tr("scientific");

    this->m_Icon = new QPushButton(this);
    m_Icon->setAccessibleName("kylin-calculator_QPushButton_m_Icon");
    this->m_Icon->setIconSize(QSize(SPECIAL_ICON_W, SPECIAL_ICON_H));
    this->m_Icon->setIcon(QIcon::fromTheme("kylin-calculator"));
    QString iconStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                        "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                        "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_Icon->setStyleSheet(iconStyle);

    this->m_mode = new QPushButton(this);
    m_mode->setAccessibleName("kylin-calculator_QPushButton_m_mode");
    this->m_mode->setFixedSize(68, 35);

    this->m_modeText = new QLabel(this);
    m_modeText->setAccessibleName("kylin-calculator_QLabel_m_modeText");
    this->m_modeText->setText(tr("standard"));

    this->m_modeIcon = new QLabel(this);
    m_modeIcon->setAccessibleName("kylin-calculator_QLabel_m_modeIcon");
    this->m_modeIcon->setFixedSize(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
    QPixmap icon(":/image/intelStandLight/ic-open.svg");
    icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
    this->m_modeIcon->setScaledContents(true);
    this->m_modeIcon->setPixmap(icon);

    this->m_vlayout = new QVBoxLayout();
    this->m_vlayout->setMargin(0);
    this->m_vlayout->addSpacing(8);
    this->m_vlayout->addWidget(this->m_Icon);

    this->m_hlayout1 = new QHBoxLayout();
    this->m_hlayout1->setMargin(0);
    this->m_hlayout1->addStretch();
    this->m_hlayout1->addWidget(this->m_modeText);
    this->m_hlayout1->addSpacing(2);
    this->m_hlayout1->addWidget(this->m_modeIcon);
    this->m_hlayout1->addStretch();

    this->m_mode->setLayout(this->m_hlayout1);
    this->m_mode->setFlat(true);

    this->m_min = new QPushButton();
    m_min->setAccessibleName("kylin-calculator_QPushButton_m_min");
    this->m_min->setFixedSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    this->m_min->setIcon(QIcon(":/image/intelScientific/min.svg"));
    this->m_min->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    this->m_min->setProperty("isWindowButton", 0x1);
    this->m_min->setProperty("useIconHighlightEffect", 0x2);
    this->m_min->setFlat(true);

    this->m_max = new QPushButton();
    m_max->setAccessibleName("kylin-calculator_QPushButton_m_max");
    this->m_max->setFixedSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    this->m_max->setIcon(QIcon(":/image/intelScientific/max.svg"));
    this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    this->m_max->setProperty("isWindowButton", 0x1);
    this->m_max->setProperty("useIconHighlightEffect", 0x2);
    this->m_max->setFlat(true);

    this->m_close = new QPushButton();
    m_close->setAccessibleName("kylin-calculator_QPushButton_m_close");
    this->m_close->setFixedSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    this->m_close->setIcon(QIcon(":/image/intelScientific/close.svg"));
    this->m_close->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    this->m_close->setProperty("isWindowButton", 0x2);
    this->m_close->setProperty("useIconHighlightEffect", 0x8);
    this->m_close->setFlat(true);

    // 设置悬浮提示
    this->m_min->setToolTip(tr("Minimize"));
    this->m_max->setToolTip(tr("Maximize"));
    this->m_close->setToolTip(tr("Close"));

    this->hlayout = new QHBoxLayout();
    this->hlayout->setContentsMargins(0, 4, 0, 4);

    this->hlayout->addSpacing(16);
    this->hlayout->addLayout(this->m_vlayout);
    this->hlayout->addSpacing(4);

    this->hlayout->addWidget(this->m_mode);
    this->hlayout->addStretch(0);

    this->hlayout->addWidget(this->m_min);
    this->hlayout->addSpacing(4);

    this->hlayout->addWidget(this->m_max);
    this->hlayout->addSpacing(4);

    this->hlayout->addWidget(this->m_close);
    this->hlayout->addSpacing(4);

    connect(this->m_min, &QPushButton::clicked, this, &TitleBar::onClicked);
    connect(this->m_max, &QPushButton::clicked, this, &TitleBar::onClicked);
    connect(this->m_close, &QPushButton::clicked, this, &TitleBar::onClicked);
    connect(this->m_mode, &QPushButton::clicked, this, &TitleBar::slotModeChange);

    this->setLayout(this->hlayout);
    QWidget *pWindow = this->window();
    this->m_modeList = new IntelModeList(pWindow);
    m_modeList->setAccessibleName("kylin-calculator_IntelModeList_m_modeList");
    this->m_modeList->setWidgetStyle();
    this->m_modeList->slotStandardClick();
    this->m_modeList->hide();

    connect(this->m_modeList->m_standard, &IntelModeButton::clicked, this, &TitleBar::slotChangeStandard);
    connect(this->m_modeList->m_science, &IntelModeButton::clicked, this, &TitleBar::slotChangeScientific);

    return;
}

void TitleBar::createInterStyle(void)
{
    QPixmap icon;
    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        this->m_mode->setStyleSheet("QPushButton::menu-indicator{image:None;}");
        this->m_mode->setStyleSheet("QPushButton{border-radius:12px;}"
                                    "QPushButton:hover{background-color:#FFFFFF;}"
                                    "QPushButton:pressed{background-color:#FFFFFF;}");
        icon.load(":/image/intelStandLight/ic-open.svg");
        icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
        this->m_modeIcon->setScaledContents(true);
        m_modeIcon->setPixmap(icon);

        this->m_min->setIcon(QIcon(":/image/intelScientific/min.svg"));
        this->m_min->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        this->m_max->setIcon(QIcon(":/image/intelScientific/max.svg"));
        this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        this->m_close->setIcon(QIcon(":/image/intelScientific/close.svg"));
        this->m_close->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
        this->m_mode->setStyleSheet("QPushButton::menu-indicator{image:None;}");
        this->m_mode->setStyleSheet("QPushButton{border-radius:12px;}"
                                    "QPushButton:hover{background-color:#3A3A3D;}"
                                    "QPushButton:pressed{background-color:#3A3A3D;}");
        icon.load(":/image/intelStandDark/ic-open.svg");
        icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
        this->m_modeIcon->setScaledContents(true);
        m_modeIcon->setPixmap(icon);

        this->m_min->setIcon(QIcon(":/image/intelScientificDark/min.svg"));
        this->m_min->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        this->m_max->setIcon(QIcon(":/image/intelScientificDark/max.svg"));
        this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        this->m_close->setIcon(QIcon(":/image/intelScientificDark/close.svg"));
        this->m_close->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
    }
    if (this->m_modeList != nullptr) {
        this->m_modeList->setWidgetStyle();
    }
    return;
}

void TitleBar::slotModeChange()
{
    if (!this->m_modeList->isVisible()) {
        QPoint pos = this->m_mode->pos();
        this->m_modeList->move(pos.x(), pos.y() + 40);
        this->m_modeList->show();
        this->m_modeList->raise();

        this->changeModeIcon();
    } else {
        this->m_modeList->hide();

        this->changeModeIcon();
    }

    return;
}

void TitleBar::changeModeIcon(void)
{
    QPixmap icon;

    if (this->m_modeList->isVisible()) {
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            icon.load(":/image/intelStandLight/ic-close.svg");
            icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
        } else {
            icon.load(":/image/intelStandDark/ic-close.svg");
            icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
        }

        this->m_modeIcon->setScaledContents(true);
        this->m_modeIcon->setPixmap(icon);

    } else {
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            icon.load(":/image/intelStandLight/ic-open.svg");
            icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
        } else {
            icon.load(":/image/intelStandDark/ic-open.svg");
            icon.scaled(SPECIAL_ICON_W / 2, SPECIAL_ICON_H / 2);
        }

        this->m_modeIcon->setScaledContents(true);
        this->m_modeIcon->setPixmap(icon);
    }

    return;
}

void TitleBar::slotChangeStandard(void)
{
    qDebug() << "Info : change mode to standard";
    this->m_modeText->setText(tr("standard"));
    emit sigModeChange(QString("standard"));

    this->m_modeList->hide();
    this->changeModeIcon();

    return;
}

void TitleBar::slotChangeScientific(void)
{
    qDebug() << "Info : change mode to scientific";
    this->m_modeText->setText(tr("scientific"));
    emit sigModeChange(QString("scientific"));

    this->m_modeList->hide();
    this->changeModeIcon();

    return;
}

void TitleBar::setFuncLabel(QString label)
{
    this->m_pFuncLabel->setText(label);
}

// 初始化组件
void TitleBar::setWidgetUi()
{
    this->setFixedHeight(TITLEH);

    // 初始化模式或功能名称
    STANDARD_LABEL = tr("Standard");
    SCIENTIFIC_LABEL = tr("Scientific");
    EXCHANGE_RATE_LABEL = tr("Exchange Rate");
    PROGRAMMER_LABEL = tr("Programmer");

    // 按钮初始化
    m_pIconBtn = new QPushButton(this);
    m_pIconBtn->setAccessibleName("kylin-calculator_QPushButton_m_pIconBtn");
    m_pFuncLabel = new QLabel(this);
    m_pFuncLabel->setAccessibleName("kylin-calculator_QLabel_m_pFuncLabel");
    m_pFuncLabel->setProperty("isWindowLabel", true);
    m_pTopButton = new QPushButton(this);
    m_pTopButton->setAccessibleName("kylin-calculator_QPushButton_m_pTopButton");
    m_pMinimizeButton = new QPushButton(this->parentWidget());
    m_pMinimizeButton->setAccessibleName("kylin-calculator_QPushButton_m_pMinimizeButton");
    m_pMaximizeButton = new QPushButton(this);
    m_pMaximizeButton->setAccessibleName("kylin-calculator_QPushButton_m_pMaximizeButton");
    m_pCloseButton = new QPushButton(this->parentWidget());
    m_pCloseButton->setAccessibleName("kylin-calculator_QPushButton_m_pCloseButton");

    // 设置空间大小
    m_pFuncLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_pTopButton->setFixedSize(TITLE_BTN_W, TITLE_BTN_H);
    m_pMinimizeButton->setFixedSize(TITLE_BTN_W, TITLE_BTN_H);
    m_pMaximizeButton->setFixedSize(TITLE_BTN_W, TITLE_BTN_H);
    m_pCloseButton->setFixedSize(TITLE_BTN_W, TITLE_BTN_H);

    // 设置对象名
    m_pFuncLabel->setObjectName("whiteLabel");
    m_pTopButton->setObjectName("topButton");
    m_pMinimizeButton->setObjectName("minimizeButton");
    m_pMaximizeButton->setObjectName("maximizeButton");
    m_pCloseButton->setObjectName("closeButton");

    // 设置悬浮提示
    m_pTopButton->setToolTip(tr("StayTop"));
    m_pMinimizeButton->setToolTip(tr("Minimize"));
    m_pMaximizeButton->setToolTip(tr("Maximize"));
    m_pCloseButton->setToolTip(tr("Close"));

    // 设置图片
    m_pIconBtn->setIconSize(QSize(SPECIAL_ICON_W, SPECIAL_ICON_H));
    m_pIconBtn->setFixedSize(QSize(SPECIAL_ICON_W, SPECIAL_ICON_H));
    m_pIconBtn->setIcon(QIcon::fromTheme("kylin-calculator"));
    m_pIconBtn->setFlat(true);
    m_pIconBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_pTopButton->setIcon(QIcon::fromTheme("ukui-unfixed-symbolic"));
    m_pTopButton->setIconSize(QSize(STANDARD_ICON_W, STANDARD_ICON_H));
    m_pTopButton->setProperty("isWindowButton", 0x1);
    m_pTopButton->setProperty("useIconHighlightEffect", 0x2);
    m_pTopButton->setFlat(true);

    m_pMinimizeButton->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_pMinimizeButton->setIconSize(QSize(STANDARD_ICON_W, STANDARD_ICON_H));
    m_pMinimizeButton->setProperty("isWindowButton", 0x1);
    m_pMinimizeButton->setProperty("useIconHighlightEffect", 0x2);
    m_pMinimizeButton->setFlat(true);

    m_pMaximizeButton->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    m_pMaximizeButton->setIconSize(QSize(STANDARD_ICON_W, STANDARD_ICON_H));
    m_pMaximizeButton->setProperty("isWindowButton", 0x1);
    m_pMaximizeButton->setProperty("useIconHighlightEffect", 0x2);
    m_pMaximizeButton->setFlat(true);


    m_pCloseButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_pCloseButton->setIconSize(QSize(STANDARD_ICON_W, STANDARD_ICON_H));
    m_pCloseButton->setProperty("isWindowButton", 0x2);
    m_pCloseButton->setProperty("useIconHighlightEffect", 0x8);
    m_pCloseButton->setFlat(true);

    menuBar = new menuModule(this);
    menuBar->setAccessibleName("kylin-calculator_menuModule_menuBar");

    // 设置按钮布局
    QHBoxLayout *pLayout = new QHBoxLayout(this);
//    pLayout->setContentsMargins(8, 4, 4, 4);
//    pLayout->setSpacing(0);
    // 这里是有问题的 应该是pLayout->addSpacing(4);但是还不知道是哪里的问题
    pLayout->addSpacing(4);
    pLayout->addWidget(m_pIconBtn);
    pLayout->addSpacing(4);
    pLayout->addWidget(m_pFuncLabel);
    pLayout->addStretch();
    pLayout->addWidget(m_pTopButton);
    pLayout->addWidget(menuBar->menuButton);
    pLayout->addWidget(m_pMinimizeButton);
    pLayout->addWidget(m_pMaximizeButton);
    pLayout->addWidget(m_pCloseButton);
    pLayout->setSpacing(4);
    pLayout->setMargin(4);

    /* 主线版本 放大功能暂时不上  , 暂时隐藏按钮 */
    this->m_pMaximizeButton->hide();

    /* xc 平板 隐藏置顶按钮 */
    if (DataWarehouse::getInstance()->platform == QString("xc-tablet")) {
        this->m_pTopButton->hide();
    }

    this->setLayout(pLayout);

    // 设置信号和槽函数
    connect(m_pMinimizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(m_pMaximizeButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(m_pCloseButton, SIGNAL(clicked(bool)), this, SLOT(onClicked()));

    return;
}

// 设置组件样式
void TitleBar::setWidgetStyle()
{
//    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
//        m_pFuncLabel->setStyleSheet("color:#000000;");
//        QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
//                           "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
//                           "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
//        m_pIconBtn->setStyleSheet(btnStyle);
//    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
//        m_pFuncLabel->setStyleSheet("color:#A6A6A6;");
//        QString btnStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
//                           "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
//                           "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
//        m_pIconBtn->setStyleSheet(btnStyle);
//    }
}

void TitleBar::setMaxBtnMode(bool isShowMax)
{
    if (isShowMax) {
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            this->m_max->setIcon(QIcon(":/image/intelScientific/max.svg"));
            this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        } else {
            this->m_max->setIcon(QIcon(":/image/intelScientificDark/max.svg"));
            this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        }
        this->m_max->setToolTip(tr("Maximize"));
    } else {
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            this->m_max->setIcon(QIcon(":/image/intelScientific/restore.svg"));
            this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        } else {
            this->m_max->setIcon(QIcon(":/image/intelScientificDark/restore.svg"));
            this->m_max->setIconSize(QSize(TITLE_BTN_W, TITLE_BTN_H));
        }
        this->m_max->setToolTip(tr("Restore"));
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        if (!this->m_max->isHidden()) {
            this->m_max->clicked();
        }
    }
}

void TitleBar::onClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel()) {
        if (pButton == m_pMinimizeButton) {
            pWindow->showMinimized();
            m_pMinimizeButton->update();
            m_pCloseButton->update();
        } else if (pButton == m_pCloseButton) {
            pWindow->close();
        } else if (pButton == this->m_min) {
            pWindow->showMinimized();
            m_min->update();
            m_close->update();
        } else if (pButton == this->m_max) {
            if (pWindow->windowState() == Qt::WindowNoState) {
                pWindow->showMaximized();
            } else if (pWindow->windowState() == Qt::WindowMaximized) {
                pWindow->showNormal();
            }
            emit sigFontUpdate();
        } else if (pButton == this->m_close) {
            pWindow->close();
        }
    }
    kdk_dia_upload_program_exit();
}
