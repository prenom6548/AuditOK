/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QGuiApplication>
#include "screens-manager.h"

namespace UkuiPanel {
Screen::Screen(QScreen *screen, QObject *parent): QObject(parent), m_screen(screen)
{

}

QRect Screen::geometry() const
{
    return m_screen->geometry();
}

QScreen *Screen::internal()
{
    return m_screen;
}

bool Screen::enable()
{
    return m_enable;
}

void Screen::setEnable(bool enable)
{
    if(m_enable != enable) {
        m_enable = enable;
        Q_EMIT enableChanged(m_enable);
    }
}

bool Screen::isValid()
{
    return (m_screen && m_screen->geometry().width() > 0 && m_screen->geometry().height() > 0);
}

ScreensManager::ScreensManager(QObject *parent) : QObject(parent)
{
    for(auto screen : qApp->screens()) {
        onScreenAdded(screen);
    }
    connect(qApp, &QGuiApplication::screenAdded, this, &ScreensManager::onScreenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &ScreensManager::onScreenRemoved);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &ScreensManager::onPrimaryScreenChanged);
}

ScreensManager::~ScreensManager()
{
    qDeleteAll(m_screens);
}

QList<Screen *> ScreensManager::screens()
{
    QList<Screen *> tmp;
    for(auto screen : m_screens) {
        if(screen->enable()) {
            tmp.append(screen);
        }
    }
    return tmp;
}

Screen *ScreensManager::primaryScreen()
{
    return m_primaryScreen;
}

void ScreensManager::onScreenAdded(QScreen *screen)
{
    auto s = new Screen(screen, this);
    if(screen == qApp->primaryScreen()) {
        m_primaryScreen = s;
        Q_EMIT primaryScreenChanged(s);
    }
    m_screens.append(s);
    connect(s->internal(), &QScreen::geometryChanged, this, [&, s](){
        checkScreensGeometryStatus(s);
    });
    connect(s, &Screen::enableChanged, this, [&, s](bool enable){
        onEnableChanged(s, enable);
    });
    checkScreensGeometryStatus(s);
}

void ScreensManager::onScreenRemoved(QScreen *screen)
{
    int i = 0;
    for(; i < m_screens.size(); ++i) {
        if(m_screens.at(i)->internal() == screen) {
            break;
        }
    }
    if(i < m_screens.size()) {
        auto s = m_screens.takeAt(i);
        Q_EMIT screenRemoved(s);
        s->deleteLater();
    }
}

void ScreensManager::onPrimaryScreenChanged(QScreen *screen)
{
    if(m_primaryScreen->internal() != screen) {
        for(auto s : m_screens) {
            if(s->internal() == screen) {
                if(!s->enable()) {
                    s->setEnable(true);
                }
                m_primaryScreen = s;
                Q_EMIT primaryScreenChanged(s);
                return;
            }
        }
    }
}

void ScreensManager::onEnableChanged(Screen *screen, bool enable)
{
    if(enable) {
        Q_EMIT screenAdded(screen);
    } else {
        Q_EMIT screenRemoved(screen);
    }
}

void ScreensManager::checkScreensGeometryStatus(Screen* screen)
{
    if(screen == nullptr || m_primaryScreen == nullptr) {
        return;
    }

    for(auto s : m_screens) {
        if(s == nullptr) {
            continue;
        }
        if(s->internal()->geometry() == screen->geometry() && s != screen) {
            if(s == m_primaryScreen) {
                screen->setEnable(false);
                Q_EMIT s->geometryChanged(s->internal()->geometry());
                return;
            } else {
                s->setEnable(false);
                Q_EMIT screen->geometryChanged(screen->internal()->geometry());
                return;
            }
        } else {
            if(!s->enable()) {
                s->setEnable(true);
            }
            Q_EMIT s->geometryChanged(s->internal()->geometry());
        }
    }
}

} // UkuiPanel
