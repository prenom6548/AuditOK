/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "qmlmanager.h"
#include "osdbasicwindow.h"
#include <QQuickItem>

QmlManager::QmlManager(QObject* parent)
    : QObject(parent)
{
    Q_INIT_RESOURCE(qml);

    initHideTimer();
    initDataManager();
}

QmlManager *QmlManager::instance()
{
    static QmlManager qmlManager;
    return &qmlManager;
}

// [新增] 初始化 D-Bus 监听
void QmlManager::connectMouseMovedSignal()
{
    bool connected = QDBusConnection::systemBus().connect(
        DBUS_USD_SERVICE,
        DBUS_USD_PATH,
        DBUS_USD_INTERFACE,
        DBUS_USD_MOTHOD,
        this,
        SLOT(onDbusMouseMoved(int, int))
    );

    if (!connected) {
        qDebug() << "[QmlManager] Connected to MouseMoved signal failed!";
    }
}

// [新增] 初始化 D-Bus 监听
void QmlManager::disconnectMouseMovedSignal()
{
    QDBusConnection::systemBus().disconnect(
        DBUS_USD_SERVICE,
        DBUS_USD_PATH,
        DBUS_USD_INTERFACE,
        DBUS_USD_MOTHOD,
        this,
        SLOT(onDbusMouseMoved(int, int))
    );
}

// [新增] 处理鼠标移动信号
void QmlManager::onDbusMouseMoved(int dx, int dy)
{
    //连续显示OSD时不会被鼠标移动隐藏OSD
    if (qAbs(QDateTime::currentDateTime().msecsTo(m_lastShowOsdTime)) < OSD_MINIMUM_SHOWTIME) {
        return;
    }

    //只有当 OSD 窗口存在且可见时，才触发隐藏
    if (m_osdBasic) {
        // 停止自动隐藏定时器，避免冲突
        if (m_osdHideTimer) {
            m_osdHideTimer->stop();
        }

        m_osdBasic->disableBlurEffect();
        // 发射信号通知 QML 开始渐隐动画
        emit requestHideAnimation();
    }
}

void QmlManager::showOsdView(int width, int height, const QString &url)
{
    m_lastShowOsdTime = QDateTime::currentDateTime();

    //相同布局的OSD窗口不必重复创建，解决更新带进度条的OSD时重复创建会造成OSD闪烁的问题
    if (!isNeedCreateOsdView(url)) {
        // 复用模式
        if (m_osdBasic) {
            // 确保窗口是显示的（防止之前动画做完还没销毁时的状态，虽然逻辑上不应该发生）
            m_osdBasic->show();
            if (m_osdHideTimer) {
                m_osdHideTimer->stop();
                m_osdHideTimer->start(HIDE_TIME_COMMON);
            }
        }
        return;
    }

    createOsdView(width, height, url);

    if (m_osdHideTimer) {
        m_osdHideTimer->stop();
        m_osdHideTimer->start(HIDE_TIME_COMMON);
    }
}

void QmlManager::createOsdView(int width, int height, const QString &url)
{
    // 1. 清理旧窗口
    if (m_osdBasic) {
        m_osdBasic.reset();
    }


    m_osdBasic = std::unique_ptr<OsdBasicWindow>(new OsdBasicWindow(width, height, m_pDataManager));

    // 3. 加载 QML
    m_osdBasic->setQuickViewSource(url);

    //仅在创建OSD时监听鼠标移动信号，OSD销毁时不再监听信号，降低系统开销
    connectMouseMovedSignal();

    // 4. 连接信号
    QQuickItem *rootItem = m_osdBasic->rootObject();
    if (rootItem) {
        QObject *rootObj = static_cast<QObject*>(rootItem);

        connect(rootObj, SIGNAL(animationFinished()),
                this, SLOT(onHideAnimationFinished()),
                Qt::UniqueConnection);

        connect(this, SIGNAL(requestHideAnimation()),
                rootObj, SLOT(startHideAnimation()),
                Qt::UniqueConnection);
    }

    // 5. 显示
    m_osdBasic->show();
}

bool QmlManager::isNeedCreateOsdView(const QString& url)
{
    if (!m_osdBasic) {
        return true;
    }

    if (m_lastUrl == url) {
        return false;
    }

    m_lastUrl = url;
    return true;
}

void QmlManager::onShowIcon(const QString& sourceName)
{
    emit initIconData(sourceName);
    showOsdView(ICON_WINDOW_SIZE, ICON_WINDOW_SIZE, OSD_QML_PATH("icon_window.qml"));
}

void QmlManager::onShowBar(const QString& sourceName, double currentData, double maxData)
{
    //解决外部接口传参超出预期问题
    currentData = currentData > maxData ? maxData : currentData;
    emit initBarData(sourceName, currentData, maxData);
    showOsdView(BAR_WINDOW_WIDTH, BAR_WINDOW_HEIGHT, OSD_QML_PATH("bar_window.qml"));
}

void QmlManager::initHideTimer()
{
    m_osdHideTimer = new QTimer(this);
    m_osdHideTimer->setSingleShot(true);
    connect(m_osdHideTimer, &QTimer::timeout, this, [this] () {
        // [修改] 不再直接 reset (销毁)，而是通知 QML 开始渐隐
        if (m_osdBasic) {
            m_osdBasic->disableBlurEffect();
            emit requestHideAnimation();
        } else {
            // 如果窗口已经不存在了，确保清理 timer
            m_osdHideTimer->stop();
        }
    });
}

// [新增] 供 QML 调用的槽函数，动画结束后真正销毁窗口
void QmlManager::onHideAnimationFinished()
{
    if (m_osdBasic) {
        //OSD销毁时断开信号
        disconnectMouseMovedSignal();
        // 使用 release + deleteLater 避免崩溃
        OsdBasicWindow* win = m_osdBasic.release();
        win->deleteLater();
        // m_osdBasic 现在是 nullptr
    }
    if (m_osdHideTimer) {
        m_osdHideTimer->stop();
    }
}

void QmlManager::initDataManager()
{
    m_pDataManager = new DataManager(this);

    connect(this, &QmlManager::initIconData, m_pDataManager, &DataManager::onInitIconData);
    connect(this, &QmlManager::initBarData, m_pDataManager, &DataManager::onInitBarData);
}

