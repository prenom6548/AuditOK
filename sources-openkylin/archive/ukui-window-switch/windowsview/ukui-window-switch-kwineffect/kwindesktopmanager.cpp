/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */

#include "kwindesktopmanager.h"

#include <QTimer>

KWinDesktopManager::KWinDesktopManager(QObject *parent) : AbstractDesktopManager(parent)
{
    // 先显式指定信号的函数指针类型，消除重载歧义
    using DesktopChangedSignalType = void(KWin::EffectsHandler::*)(int, int, KWin::EffectWindow*);

    // 用static_cast明确转换，再传入connect
    QObject::connect(KWin::effects,
                     static_cast<DesktopChangedSignalType>(&KWin::EffectsHandler::desktopChanged),
                     this,
                     [this](int oldD, int newD, KWin::EffectWindow* win) {
        Q_UNUSED(win); // 标记未使用的指针参数，避免额外警告
        emit currentDesktopChanged(QString::number(oldD), QString::number(newD));
    });
}

void KWinDesktopManager::removeDesktop(const QString &desktop)
{
    if (!validateDesktopIndex(desktop)) {
        return;
    }
    KWin::desktopManager->removeDesktop(desktop.toInt());
    Q_EMIT desktopRemoved(desktop);
}

void KWinDesktopManager::moveDesktop(const QString &srcDesktop, int destDesktopIndex)
{
    if (!validateDesktopIndex(srcDesktop)
            || destDesktopIndex < 1 || destDesktopIndex > getNumberOfDesktops()) {
        return;
    }
    KWin::desktopManager->moveDesktop(srcDesktop.toInt(), destDesktopIndex);
    Q_EMIT desktopMoved(srcDesktop, destDesktopIndex);
    // kwin环境，工作区标识为序号，CurrentDesktop是正在拖动的工作区，
    // 交换位置后，工作区标识变化，因此需要立即设置setCurrentDesktop
    setCurrentDesktop(QString::number(destDesktopIndex));
}

void KWinDesktopManager::appendDesktop()
{
    auto desktopCount = KWin::effects->numberOfDesktops();
    KWin::effects->setNumberOfDesktops(desktopCount + 1);
    Q_EMIT desktopAppended(QString::number(desktopCount + 1));
}

void KWinDesktopManager::setCurrentDesktop(const QString &desktop)
{
    if (!validateDesktopIndex(desktop)) {
        return;
    }

    // 在990环境下将wayland窗口拖入新工作区后，立刻调用接口切换至对应工作区会导致kwin崩溃
    // setCurrentDesktop与windowToDesktops不在一个事件循环内执行则无此问题
    QTimer::singleShot(0, this, [desktop]() {
        KWin::effects->setCurrentDesktop(desktop.toInt());
    });
}

QString KWinDesktopManager::getCurrentDesktop()
{
    return QString::number(KWin::effects->currentDesktop());
}

int KWinDesktopManager::getNumberOfDesktops()
{
    return KWin::effects->numberOfDesktops();
}

bool KWinDesktopManager::isCurrentDesktopShowingDesktop()
{
    return KWin::desktopManager->showingDesktop();
}

QRect KWinDesktopManager::getFullScreenGeometry(const QString &desktop, int screenIndex)
{
    KWin::EffectScreen* screen = KWin::effects->findScreen(screenIndex);
    return KWin::effects->clientArea(KWin::clientAreaOption::FullScreenArea,
                                     screen, desktop.toInt());
}

void KWinDesktopManager::setDesktopName(const QString &desktop, const QString name)
{
    Q_UNUSED(desktop);
    Q_UNUSED(name);
}

int KWinDesktopManager::getDesktopIndex(const QString &id)
{
    return id.toUInt();
}

QString KWinDesktopManager::getDesktopUuid(int desktopIndex)
{
    return QString::number(desktopIndex);
}

QString KWinDesktopManager::getOutputUuid(int outputIndex)
{
    Q_UNUSED(outputIndex);
    return "";
}

bool KWinDesktopManager::validateDesktopIndex(const QString &desktop)
{
    bool ok = false;
    int index = desktop.toInt(&ok);
    if (!ok) {
        return false;
    }
    return index >= 1 && index <= getNumberOfDesktops();
}

bool KWinDesktopManager::isInitialized()
{
    return true;
}
