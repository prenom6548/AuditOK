/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "dialog.h"

#include "utils.h"
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>

#include "wayland/ukui/ukui-decoration-manager.h"
#include "wayland/xdg/XdgManager.h"
#include "sessionsettings.h"

using namespace kmre;

Dialog::Dialog(QWidget *parent):
    QDialog(parent)
{
    installEventFilter(this);
}

Dialog::~Dialog()
{

}

void Dialog::waylandMove()
{
    if (SessionSettings::getInstance().hasWaylandUKUIDecorationSupport()) {
        UKUIDecorationManager::getInstance()->moveWindow(this->windowHandle());
    } else if (SessionSettings::getInstance().supportsWaylandXdgShell()) {
        XdgManager::getInstance()->moveWindow(this->windowHandle());
    }
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::MouseButtonPress){
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                waylandMove();
            }
        }
    }
    return false;
}

void Dialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    UKUIDecorationManager::getInstance()->removeHeaderBar(windowHandle());
}
