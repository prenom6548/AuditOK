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
 */

#include "desktopmanager.h"

#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QCursor>
#include <QQuickItem>
#include <QQuickWindow>
#include <QDir>
#include <QSettings>

#ifdef KWINEFFECT
#include "ukui-window-switch-kwineffect/kwindesktopmanager.h"
#else
#include "ukui-window-switch-wlcom/kywcdesktopmanager.h"
#endif

static constexpr int MAX_DESKTOP_COUNT = 15;
const QString DESKTOP_CONFIG_DIR = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-window-switch/");
const QString DESKTOP_CONFIG = DESKTOP_CONFIG_DIR + QStringLiteral("multitaskview.conf");

DesktopManager *DesktopManager::getInstance()
{
    static DesktopManager s_interface;
    return &s_interface;
}

DesktopManager::DesktopManager(QObject *parent)
    : QObject(parent)
    , m_impl(nullptr)
{
#ifdef KWINEFFECT
    m_impl = new KWinDesktopManager(this);
#else
    m_impl = new KywcDesktopManager(this);
#endif
    connect(m_impl, &AbstractDesktopManager::workspaceManagerInitialized, this, &DesktopManager::initialized, Qt::QueuedConnection);
    connect(m_impl, &AbstractDesktopManager::desktopRemoved, this, &DesktopManager::desktopRemoved, Qt::QueuedConnection);
    connect(m_impl, &AbstractDesktopManager::desktopAppended, this, &DesktopManager::desktopAppended, Qt::QueuedConnection);
    connect(m_impl, &AbstractDesktopManager::desktopMoved, this, &DesktopManager::desktopMoved, Qt::QueuedConnection);
    connect(m_impl, &AbstractDesktopManager::currentDesktopChanged, this, &DesktopManager::currentDesktopChanged, Qt::QueuedConnection);
}

void DesktopManager::removeDesktop(const QString &desktop)
{
    if (m_impl) {
        m_impl->removeDesktop(desktop);
    }
}

void DesktopManager::moveDesktop(const QString &srcDesktop, int destDesktopIndex)
{
    if (m_impl) {
        m_impl->moveDesktop(srcDesktop, destDesktopIndex);
    }
}

void DesktopManager::appendDesktop()
{
    if (m_impl) {
        if (getNumberOfDesktops() >= MAX_DESKTOP_COUNT) {
            qWarning() << "[DesktopManager] already reached max desktop count";
            return;
        }
        m_impl->appendDesktop();
    }
}

void DesktopManager::setCurrentDesktop(const QString &desktop)
{
    if (m_impl) {
        m_impl->setCurrentDesktop(desktop);
    }
}

QString DesktopManager::getCurrentDesktop()
{
    if (m_impl) {
        return m_impl->getCurrentDesktop();
    }
    return "";
}

int DesktopManager::getNumberOfDesktops()
{
    if (m_impl) {
        return m_impl->getNumberOfDesktops();
    }
    return 1;
}

bool DesktopManager::isCurrentDesktopShowingDesktop()
{
    if (m_impl) {
        return m_impl->isCurrentDesktopShowingDesktop();
    }
    return false;
}

QRect DesktopManager::getFullScreenGeometry(const QString &desktop, int screenIndex)
{
    if (m_impl) {
        return m_impl->getFullScreenGeometry(desktop, screenIndex);
    }
    return QRect();
}

void DesktopManager::setDesktopName(const QString &desktop, const QString name)
{
    if (m_impl) {
        m_impl->setDesktopName(desktop, name);
    }
}

QString DesktopManager::getDesktopUuid(int desktopIndex)
{
    if (m_impl) {
        return m_impl->getDesktopUuid(desktopIndex);
    }
    return QString();
}

QString DesktopManager::getOutputUuid(int outputIndex)
{
    if (m_impl) {
        return m_impl->getOutputUuid(outputIndex);
    }
    return QString();
}

bool DesktopManager::isInitialized()
{
    if (m_impl) {
        return m_impl->isInitialized();
    }
    return false;
}

void DesktopManager::openDesktopMenu(const QString &desktop, QQuickItem *item)
{
    if (!m_impl) {
        return;
    }

    int numberOfDesktops = m_impl->getNumberOfDesktops();
    int desktopInt = m_impl->getDesktopIndex(desktop);

    if (desktopInt < 1 || desktopInt > numberOfDesktops) {
        return;
    }

    if (m_desktopContextMenu) {
        m_desktopContextMenu->close();
    }

    auto *menu = new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // 重命名
    auto *renameAction = new QAction(QIcon::fromTheme("document-edit-symbolic"), tr("Rename"), menu);
    menu->addAction(renameAction);
    QPointer<QQuickItem> itemGuard = item;
    connect(renameAction, &QAction::triggered, this, [this, itemGuard]{
        if (itemGuard) {
            Q_EMIT renameDesktopSignal(itemGuard);
        }
    });

    // 向右移动
    if (desktopInt < numberOfDesktops) {
        auto *moveRightAction = new QAction(QIcon::fromTheme("go-previous-rtl-symbolic"), tr("Move Right"), menu);
        menu->addAction(moveRightAction);
        connect(moveRightAction, &QAction::triggered, this, [this, desktop, desktopInt]{
            m_impl->moveDesktop(desktop, desktopInt + 1);
        });
    }

    // 向左移动
    if (desktopInt > 1) {
        auto *moveLeftAction = new QAction(QIcon::fromTheme("go-next-rtl-symbolic"), tr("Move Left"), menu);
        menu->addAction(moveLeftAction);
        connect(moveLeftAction, &QAction::triggered, this, [this, desktop, desktopInt]{
            m_impl->moveDesktop(desktop, desktopInt - 1);
        });
    }

    // 删除
    if (numberOfDesktops > 1) {
        auto *deleteAction = new QAction(QIcon::fromTheme("edit-delete-symbolic"), tr("Delete"), menu);
        menu->addAction(deleteAction);
        connect(deleteAction, &QAction::triggered, this, [this, desktop]{
            m_impl->removeDesktop(desktop);
        });
    }

    m_desktopContextMenu = menu;
    if (m_desktopContextMenu->windowHandle() && item && item->window()) {
        m_desktopContextMenu->windowHandle()->setTransientParent(item->window());
    }
    m_desktopContextMenu->popup(QCursor::pos());
}

QStringList DesktopManager::getDesktopNames() const
{
    QSettings desktopSettings(DESKTOP_CONFIG, QSettings::IniFormat);
    desktopSettings.setIniCodec("utf-8");
    QStringList nameList = desktopSettings.value("desktops/name").toStringList();
    return nameList;
}

void DesktopManager::saveDesktopNames(const QStringList &nameList)
{
    if (!QDir().mkpath(DESKTOP_CONFIG_DIR)) {
        qWarning() << "[DesktopManager] desktop config dir makepath failed!";
        return;
    }

    QSettings desktopSettings(DESKTOP_CONFIG, QSettings::IniFormat);
    desktopSettings.setIniCodec("utf-8");
    desktopSettings.beginGroup("desktops");
    desktopSettings.setValue("name", QVariant(nameList));
    desktopSettings.endGroup();
    //qDebug() << "[DesktopManager] save custom desktop names" << desktopSettings.value("desktops/name").toStringList();
}
