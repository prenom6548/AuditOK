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
#include "titlewidget.h"

titleWidget::titleWidget(QWidget *parent) : QWidget(parent)
{
    const QByteArray style_id(ORG_UKUI_STYLE_FONT);
    m_pGsettingFontSize = new QGSettings(style_id);
    this->setFixedHeight(30);
    this->setFocusPolicy(Qt::StrongFocus);

    m_pTitleLayout  = new QHBoxLayout;
    m_pTitleLayout->setContentsMargins(0, 0, 0, 0);
    m_pTitleLayout->setSpacing(0);

    m_pLogoLabel = new QLabel();
    m_pLogoLabel->setAccessibleName("kylin-installer_titleWidget_QLabel_m_pLogoLabel");
    m_pLogoLabel->setPixmap(QIcon::fromTheme("kylin-installer").pixmap(QSize(24,24)));

    m_pNameLabel = new QLabel();
    m_pNameLabel->setAccessibleName("kylin-installer_titleWidget_QLabel_m_pNameLabel");
    m_pNameLabel->setText(tr("Installer"));
    m_pNameLabel->adjustSize();

    m_pClostBtn = new QPushButton();
    m_pClostBtn->setAccessibleName("kylin-installer_titleWidget_QPushButton_m_pClostBtn");
    m_pClostBtn->setFlat(true);
    m_pClostBtn->setProperty("isWindowButton", 0x2);
    m_pClostBtn->setProperty("useIconHighlightEffect", 0x8);
    m_pClostBtn->setFixedSize(30,30);
    m_pClostBtn->setToolTip(tr("Close"));

    m_pMinBtn = new QPushButton();
    m_pMinBtn->setAccessibleName("kylin-installer_titleWidget_QPushButton_m_pMinBtn");
    m_pMinBtn->setFlat(true);
    m_pMinBtn->setProperty("isWindowButton", 0x1);
    m_pMinBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pMinBtn->setFixedSize(30,30);
    m_pMinBtn->setToolTip(tr("Min"));

    m_pSetBtn = new QToolButton();
    m_pSetBtn->setAccessibleName("kylin-installer_titleWidget_QToolButton_m_pSetBtn");
    m_pSetBtn->setFixedSize(30,30);
    m_pSetBtn->setProperty("isWindowButton", 0x1);
    m_pSetBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pSetBtn->setPopupMode(QToolButton::InstantPopup);
    m_pSetBtn->setAutoRaise(true);
    m_pSetBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_pSetBtn->setToolTip(tr("Menu"));

    m_pTitleLayout->addItem(new QSpacerItem(8, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pLogoLabel);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(8, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pNameLabel);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(366, 1, QSizePolicy::Expanding));
    m_pTitleLayout->addWidget(m_pSetBtn);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(4, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pMinBtn);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(4, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pClostBtn);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(4, 1, QSizePolicy::Fixed));
    this->setLayout(m_pTitleLayout);


    //初始化下拉菜单
    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-installer_titleWidget_QMenu_m_menu");
    QList<QAction *> actions ;

    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));

    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));

    kdk::KMenuButton* menubutton = new kdk::KMenuButton();
    menubutton->setText(tr("Feedback"));

    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));

    actionAbout->setFont(getSystemFont());
    actionHelp->setFont(getSystemFont());
    actionQuit->setFont(getSystemFont());
    actions << actionHelp << actionAbout << menubutton->feedbackAction() << actionQuit;
    m_menu->addActions(actions);

    m_pSetBtn->setMenu(m_menu);

    m_pMinBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_pClostBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_pSetBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));

    m_pGsettings = new QGSettings("org.ukui.style");
    connect (m_pGsettings, &QGSettings::changed, this, [=] (const QString &key) {
        m_pLogoLabel->setPixmap(QIcon::fromTheme("kylin-installer").pixmap(QSize(24,24)));
        if (key == STYLE_NAME) {
            m_pMinBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
            m_pClostBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
            m_pSetBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
        }

        if (key == GSETTING_FONT_KEY) {
            actionAbout->setFont(getSystemFont());
            actionHelp->setFont(getSystemFont());
            actionQuit->setFont(getSystemFont());
        }
    });

    this->hide();
}

QFont titleWidget::getSystemFont()
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat()* 8/7);

    double lableBaseFontSize = 14 * 9/12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setBold(false);
    font.setPointSizeF(nowFontSize);
    return font;
}

void titleWidget::focuschangeEvent(bool hasFocus)
{
    if (!m_pNameLabel)
        return;

    QString themeName = m_pGsettings->get("style-name").toString();

    // qDebug() << (hasFocus ? "Widget got focusInEvent"
    //                       : "Widget got focusOutEvent");

    static const QStringList lightThemes = {
        "ukui-default",
        "ukui-light",
        "ukui-white"
    };

    bool isLightTheme = lightThemes.contains(themeName, Qt::CaseInsensitive);

    if (!hasFocus) {
        m_pNameLabel->setStyleSheet("color: gray; font-weight: normal;");
        return;
    }

    if (isLightTheme) {
        m_pNameLabel->setStyleSheet("color: black; font-weight: normal;");
    } else {
        m_pNameLabel->setStyleSheet("color: white; font-weight: normal;");
    }
}
