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

#include "quad_btns_title_bar.h"
#include "mainwindow.h"
#include "size_para.h"
QuadBtnsTitleBar::QuadBtnsTitleBar(QWidget *mainWin, bool hasMenuBtn, bool hasMinBtn, bool hasMaxBtn, bool hasCloseBtn)
    : m_parentWin(mainWin)
{
    if (hasMenuBtn) {
        m_menuBtn = new MenuModule(mainWin);
        m_menuBtn->setObjectName("m_menuBtn");
        m_menuBtn->setFixedSize(WIN_BTN_SIZE);
        m_menuBtn->setToolTip(tr("menu"));
    }

    if (hasMinBtn) {
        m_minBtn = new QPushButton(mainWin);
        m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic")); //主题库的最小化图标
        m_minBtn->setObjectName("minBtn");
        m_minBtn->setFixedSize(WIN_BTN_SIZE);
        m_minBtn->setToolTip(tr("minimize"));
        m_minBtn->setFocusPolicy(Qt::NoFocus);
        m_minBtn->setProperty("isWindowButton", 0x1);
        m_minBtn->setProperty("useIconHighlightEffect", 0x2);
        m_minBtn->setFlat(true);
        m_minBtn->setIconSize(BTN_ICON_SIZE);
    }

    if (hasMaxBtn) {
        m_maxBtn = new QPushButton(mainWin);
        m_maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic")); //主题库的最大化图标
        m_maxBtn->setObjectName("maxBtn");
        m_maxBtn->setFixedSize(WIN_BTN_SIZE);
        m_maxBtn->setToolTip(tr("full screen"));
        m_maxBtn->setFocusPolicy(Qt::NoFocus);
        m_maxBtn->setProperty("isWindowButton", 0x1);
        m_maxBtn->setProperty("useIconHighlightEffect", 0x2);
        m_maxBtn->setFlat(true);
        m_maxBtn->setIconSize(BTN_ICON_SIZE);
    }

    if (hasCloseBtn) {
        m_closeBtn = new QPushButton(mainWin);
        m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的关闭图标
        m_closeBtn->setObjectName("closeBtn");
        m_closeBtn->setFixedSize(WIN_BTN_SIZE);
        m_closeBtn->setToolTip(tr("close"));
        m_closeBtn->setFocusPolicy(Qt::NoFocus);
        m_closeBtn->setProperty("isWindowButton", 0x2);
        m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
        m_closeBtn->setFlat(true);
        m_closeBtn->setIconSize(BTN_ICON_SIZE);
    }

    m_HwholeLayout = new QHBoxLayout(this);
    m_HwholeLayout->setSpacing(0);
    m_HwholeLayout->setContentsMargins(0, 4, 4, 0);
    m_HwholeLayout->addStretch();
    if (hasMenuBtn) {
        m_HwholeLayout->addWidget(m_menuBtn);
    }
    if (hasMinBtn) {
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_minBtn);
    }
    if (hasMaxBtn) {
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_maxBtn);
    }
    if (hasCloseBtn) {
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_closeBtn);
    }

    this->setLayout(m_HwholeLayout);
    initConnect(mainWin, hasMenuBtn, hasMinBtn, hasMaxBtn, hasCloseBtn);
}

void QuadBtnsTitleBar::initConnect(QWidget *mainWin, bool hasMenuBtn, bool hasMinBtn, bool hasMaxBtn, bool hasCloseBtn)
{
    if (hasMenuBtn) {
        connect(m_menuBtn, &MenuModule::menuModuleClose, mainWin, &QWidget::close);
        connect(m_menuBtn, SIGNAL(showConfigureWin()), this, SIGNAL(showConfigureWin()), Qt::UniqueConnection);
    }
    if (hasMinBtn) {
        connect(m_minBtn, &QPushButton::clicked, mainWin, [=] {
            mainWin->setWindowState(Qt::WindowMinimized);
        });
    }
    if (hasMaxBtn) {
        connect(m_maxBtn, &QPushButton::clicked, mainWin, [=] {
            mainWin->setWindowState(Qt::WindowMaximized);
        });
    }
    if (hasCloseBtn) {
        connect(m_closeBtn, &QPushButton::clicked, mainWin, &QWidget::close);
    }
}
void QuadBtnsTitleBar::setCloseHide()
{
    disconnect(m_closeBtn, 0, 0, 0);
    connect(m_closeBtn, &QPushButton::clicked, m_parentWin, &QWidget::close);
}
void QuadBtnsTitleBar::setShowBtnsMode(QuadBtnsShowMode mode, QWidget *mainWin)
{
    m_parentWin = mainWin;
    delete m_HwholeLayout;

    m_HwholeLayout = new QHBoxLayout(this);
    m_HwholeLayout->setSpacing(0);
    m_HwholeLayout->setContentsMargins(0, 4, 4, 0);
    m_HwholeLayout->addSpacing(0);
    m_HwholeLayout->addStretch();

    switch (mode) {
    case (QuadBtnsShowMode::ALL):
        m_HwholeLayout->addWidget(m_menuBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_minBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_maxBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_closeBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_closeBtn);

        this->setLayout(m_HwholeLayout);
        break;
    case (QuadBtnsShowMode::NO_MAX_BTN):
        m_maxBtn->hide();
        m_HwholeLayout->addWidget(m_menuBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_minBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_closeBtn);

        this->setLayout(m_HwholeLayout);
        break;
    case (QuadBtnsShowMode::NO_MENU_BTN):
        m_menuBtn->hide();
        m_HwholeLayout->addWidget(m_minBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_maxBtn);
        m_HwholeLayout->addSpacing(4);
        m_HwholeLayout->addWidget(m_closeBtn);

        this->setLayout(m_HwholeLayout);
        break;
    case (QuadBtnsShowMode::ONLY_CLOSE_BTN):
        m_maxBtn->hide();
        m_minBtn->hide();
        m_menuBtn->hide();
        m_HwholeLayout->addWidget(m_closeBtn);

        this->setLayout(m_HwholeLayout);
        break;

    default:
        break;
    }
    setCloseHide();
}
void QuadBtnsTitleBar::setQSSFontSize(QFont curFont)
{
    m_menuBtn->setQSSFontSize(curFont);
}


QPushButton *QuadBtnsTitleBar::getCloseButton()
{
    return m_closeBtn;
}
