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

#include "clearmanager.h"
#include "publicstatus.h"
#include "cleartrash.h"
#include <QMap>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QThread>
namespace KylinRubbishClear
{

ClearManager::ClearManager(QObject *parent) : QObject(parent) {}
ClearManagerPri *ClearManager::m_pri = nullptr;
ClearManager *ClearManager::m_instance = nullptr;
QThread *ClearManager::m_thread = nullptr;

ClearManager *ClearManager::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new ClearManager();
        m_pri = new ClearManagerPri();
        m_thread = new QThread(m_instance);
        connect(m_instance, &ClearManager::slotScanSystem, m_pri, &ClearManagerPri::slotScanSystem);
        connect(m_instance, &ClearManager::slotCleanSystm, m_pri, &ClearManagerPri::slotCleanSystm);
        connect(m_pri, &ClearManagerPri::sigScanDetailData, m_instance, &ClearManager::sigScanDetailData);
        connect(m_pri, &ClearManagerPri::sigScanDetailStatus, m_instance, &ClearManager::sigScanDetailStatus);
        connect(m_pri, &ClearManagerPri::sigCleanStatus, m_instance, &ClearManager::sigCleanStatus);
        connect(m_pri, &ClearManagerPri::sigCleanTrashStatus, m_instance, &ClearManager::sigCleanTrashStatus);
        m_pri->moveToThread(m_thread);
        m_thread->start();
    }
    return m_instance;
}

ClearManagerPri::ClearManagerPri(QObject *parent) : QObject(parent) {}

void ClearManagerPri::slotScanSystem(QMap<QString, QVariant> itemsMap)
{
    if (itemsMap.empty()) {
        qDebug() << "ClearManager::slotScanSystem map is empty!";
    }
    QString homePath = QDir::homePath();
    qDebug() << "ClearManager::slotScanSystem HOME path:" << homePath;
    mCookieList.clear();
    if (itemsMap.contains("Cookies")) {
        qDebug() << "ClearManager::slotScanSystem 包含cookies";
        QStringList cookieList;
        cookieList.clear();

        cookieList = itemsMap.value("Cookies").toStringList();
        if (cookieList.contains("firefox")) {
            qDebug() << "ClearManager::slotScanSystem firefox";
            QString path = "";
            QDir dir(QString("%1/.mozilla/").arg(homePath));
            if (dir.cd("firefox")) {
                path = dir.absolutePath();
            } else if (dir.cd("firefox-esr")) {
                path = dir.absolutePath();
            }
            if (!path.isEmpty()) {
                for (QString &ls : dir.entryList()) {
                    if (ls.contains(".default")) {
                        QString tmp = path + "/" + ls + "/cookies.sqlite";
                        if (QFile(tmp).exists()) {
                            qDebug() << "ClearManager::slotScanSystem firefox文件存在";
                            mCookieList.append(tmp);
                        }
                    }
                }
            }
        }
        if (cookieList.contains("chromium")) {
            qDebug() << "ClearManager::slotScanSystem chromium";
            QString path = QString("%1/.config/chromium/Default/Cookies").arg(homePath);
            QFile file(path);
            if (file.exists()) {
                qDebug() << "ClearManager::slotScanSystem chromium文件存在";
                mCookieList.append(path);
            }
        }
        if (cookieList.contains("qaxbrowser")) {
            qDebug() << "ClearManager::slotScanSystem qaxbrowser";
            QString path = QString("%1/.config/qaxbrowser/Default/Cookies").arg(homePath);
            QFile file(path);
            if (file.exists()) {
                qDebug() << "ClearManager::slotScanSystem qaxbrowser文件存在";
                mCookieList.append(path);
            }
        }
        Q_EMIT sigScanDetailData("Cookies", mCookieList);
    }
    QString str = "Complete:Cookies";
    Q_EMIT sigScanDetailStatus(str);

    if (itemsMap.contains("Trash")) {
        qDebug() << "ClearManager::slotScanSystem 包含Trash";
        //进行trash的扫描
        ClearTrash trash;
        connect(&trash, &ClearTrash::trashStatus, this, &ClearManagerPri::sigCleanTrashStatus);
        mTrashList = trash.scanTrashFiles();
        Q_EMIT sigScanDetailData("Trash", mTrashList);
    }
    QString str1 = "Complete:Trash";
    Q_EMIT sigScanDetailStatus(str1);
}

void ClearManagerPri::slotCleanSystm(QMap<QString, QVariant> itemsMap)
{
    qDebug() << "ClearManager::slotCleanSystm";
    if (itemsMap.empty()) {
        qDebug() << "ClearManager::slotCleanSystm map is empty!";
    }
    if (itemsMap.contains("cookie")) {
        qDebug() << "ClearManager::slotCleanSystm 包含cookie";
        QStringList cookieList;
        cookieList.clear();

        cookieList = itemsMap.value("cookie").toStringList();
        if (!cookieList.isEmpty()) {
            for (int i = 0; i < cookieList.size(); i++) {
                qDebug() << "ClearManager::slotCleanSystm cookieList:" << cookieList.at(i);
                QFile file(cookieList.at(i));
                if (file.exists()) {
                    bool result = file.remove();
                    if (!result) {
                        qDebug() << "ClearManager::slotCleanSystm 删除失败！";
                    }
                } else {
                    qDebug() << "ClearManager::slotCleanSystm 文件不存在！";
                }
            }
        } else {
            qDebug() << "ClearManager::slotCleanSystm cookieList is empty";
        }
    }

    QString str = "Complete:cookie";
    QString str1 = "cookie";
    qDebug() << "ClearManager::slotCleanSystm cookies完成";
    Q_EMIT sigCleanStatus(str, str1);

    if (itemsMap.contains("trash")) {
        ClearTrash trash;
        trash.cleanup();
    }

    QString strTrash = "Complete:Trash";
    QString strTrash1 = "trash";
    Q_EMIT sigCleanStatus(strTrash, strTrash1);

    QString all = "Complete:all";
    QString allf = "all";
    qDebug() << "ClearManager::slotCleanSystm all全部完成";
    Q_EMIT sigCleanStatus(all, allf);
}
} // namespace KylinRubbishClear
