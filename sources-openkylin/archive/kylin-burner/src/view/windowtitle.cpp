/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "windowtitle.h"
#include "ui_windowtitle.h"
#include "src/model/logicmain.h"
#include "src/po/translation.h"
#include "src/frame/logger.h"
#include "src/frame/signals.h"

#include <QEvent>
#include <QMouseEvent>

#if UKUI_SESSION_TYPE == 2
    #include <ukuiwindowhelper/ukuiwindowhelper.h>

#endif

#define ICON_SIZE QSize(16, 16)

WindowTitle::WindowTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WindowTitle),
    m_bShowTitle(true),
    m_bAppRunning(true)
{
    this->setObjectName("kylin-burner_WindowTitle");
    ui->setupUi(this);

    ui->logo->installEventFilter(this);
    ui->title->installEventFilter(this);
#if UKUI_SESSION_TYPE == 2
    ui->max->installEventFilter(this);
#else
    ui->max->setToolTip(_("Max"));
#endif

    ui->close->setAccessibleName("kylin-burner_QPushButton_WindowTitle_close");
    ui->logo->setAccessibleName("kylin-burner_QLabel_WindowTitle_logo");
    ui->max->setAccessibleName("kylin-burner_QPushButton_WindowTitle_max");
    ui->menu->setAccessibleName("kylin-burner_QToolButton_WindowTitle_menu");
    ui->min->setAccessibleName("kylin-burner_QPushButton_WindowTitle_min");
    ui->title->setAccessibleName("kylin-burner_QLabel_WindowTitle_title");

    ui->close->setProperty("isWindowButton", 0x2);
    ui->close->setToolTip(_("Close"));
    ui->min->setProperty("isWindowButton", 0x1);
    ui->min->setToolTip(_("Min"));
    ui->max->setProperty("isWindowButton", 0x1);
    ui->menu->setProperty("isWindowButton", 0x1);
    ui->menu->setToolTip(_("Options"));
    ui->menu->setPopupMode(QToolButton::InstantPopup);
    ui->close->setFocusPolicy(Qt::NoFocus); // 当为子窗口时，避免与回车关闭窗口冲突
    configTitleLabel();

    // 统一图标文件大小
    ui->close->setIconSize(ICON_SIZE);
    ui->max->setIconSize(ICON_SIZE);
    ui->min->setIconSize(ICON_SIZE);
    ui->menu->setIconSize(ICON_SIZE);

    // 采用系统图标
    ui->close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->min->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    ui->max->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    ui->menu->setIcon(QIcon::fromTheme("open-menu-symbolic")); // 特定此主题menu图标，否则会有下拉箭头

    connect(ui->close, SIGNAL(clicked(bool)), this, SIGNAL(close()));
    connect(ui->max, SIGNAL(clicked(bool)), this, SIGNAL(max()));
    connect(ui->min, SIGNAL(clicked(bool)), this, SIGNAL(min()));
    connect(LogicMain::instance(), &LogicMain::exit, [=](){m_bAppRunning = false;});
    connect(LogicMain::instance(), &LogicMain::functionMenuChanged, this, [=](void *d){
        QMenu *menu = static_cast<QMenu *>(d);
        if(menu)
        {
            this->ui->menu->setMenu(menu);
        }

    });
    setFocusPolicy(Qt::NoFocus);

    slotChangeUIFontSize(LogicMain::instance()->GetSystemFontSize());
    connect(LogicMain::instance(), &LogicMain::systemFontSizeChanged, this, &WindowTitle::slotChangeUIFontSize);
}

void WindowTitle::onlyShowButton()
{
    ui->logo->hide();
    ui->title->setText("");
    m_bShowTitle = false;
}

void WindowTitle::onlyShowLogo()
{
    ui->close->hide();
    ui->min->hide();
    ui->max->hide();
    ui->menu->hide();
}

void WindowTitle::onlyShowInDialog()
{
    ui->min->hide();
    ui->max->hide();
    ui->menu->hide();
}

void WindowTitle::onlyVisibleClose(bool b)
{
    ui->close->setVisible(b);
}

void WindowTitle::setIconAndTitle(QIcon icon, QString title)
{
    ui->logo->setPixmap(icon.pixmap(ui->logo->size()));
    ui->title->setText(title);
}

void WindowTitle::onWindowIconChange(QIcon icon)
{
    ui->logo->setPixmap(icon.pixmap(ui->logo->size()));
}

void WindowTitle::onWindowTitleChange(QString title)
{
    if (m_bShowTitle)
        ui->title->setText(title);
}

void WindowTitle::slotChangeUIFontSize(int fontSize)
{
    QFont font = ui->title->font();
    font.setPointSize(fontSize);
    ui->title->setFont(font);
}

void WindowTitle::configTitleLabel()
{
    ui->title->setProperty("isWindowLabel",true);
}

void WindowTitle::onWindowShowMaxed(bool b)
{
    if (b) {
        ui->max->setIcon(QIcon::fromTheme("window-restore-symbolic"));

#if UKUI_SESSION_TYPE < 2
        ui->max->setToolTip(_("Restore"));
#endif

    } else {
        ui->max->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
#if UKUI_SESSION_TYPE < 2
        ui->max->setToolTip(_("Max"));
#endif
    }
}

void WindowTitle::setIsTabletMode(bool tablet)
{
    ui->max->setVisible(!tablet);
}

void WindowTitle::slotsSetCloseEnabel(bool e)
{
    ui->close->setEnabled(e);
}

bool WindowTitle::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->max) {
        if (event->type() == QEvent::HoverEnter) {
            QRect maxBtnRect = QRect(ui->max->mapToGlobal(QPoint(0, 0)), ui->max->size());
            emit g_user_signal->sigMaxButtonHoverEnter(maxBtnRect);
        } else if (event->type() == QEvent::HoverLeave) {
            emit g_user_signal->sigMaxButtonHoverLeave();
        }
    } else {
        QMouseEvent *mouseEvent = nullptr;
        switch (event->type())
        {
        case QEvent::MouseButtonDblClick:
            mouseEvent = static_cast<QMouseEvent *>(event);
            if ( (watched == ui->logo || watched == ui->title || watched == this)
                 && (mouseEvent->button() == Qt::LeftButton) ) emit max();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

WindowTitle::~WindowTitle()
{
    delete ui;
}
