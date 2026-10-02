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

#include "osdbasicwindow.h"
#include <QScreen>
#include <QGuiApplication>
#include <QSize>
#include <QPainterPath>

OsdBasicWindow::OsdBasicWindow(int width, int height, DataManager *manager, QObject *parent)
    : QObject(parent)
{
    initOsdView();

    // 【必须】先设置数据上下文
    setDataManager(manager);

    // 再设置大小（这会触发一些几何计算，但不影响 QML 加载）
    setOsdSize(width, height);
}

void OsdBasicWindow::setDataManager(DataManager *manager)
{
    if (m_osdView && manager) {
        m_osdView->rootContext()->setContextProperty("dataManager", manager);
    }}

void OsdBasicWindow::show()
{
    m_osdView->show();
}

void OsdBasicWindow::initOsdView()
{
    // [修复] C++11 兼容写法
    // 原代码: m_osdView = std::make_unique<QQuickView>();
    m_osdView = std::unique_ptr<QQuickView>(new QQuickView());

    m_osdView->setFlags(Qt::FramelessWindowHint);
    m_osdView->setColor("transparent");
    m_osdView->setResizeMode(QQuickView::SizeRootObjectToView);

    initWindowHelper();
}

void OsdBasicWindow::initWindowHelper()
{
    m_windowHelper = new UkuiWindowHelper(m_osdView.get());
    m_windowHelper->setWindowState(UkuiWindowHelper::State::KeepAbove);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::OnScreenDisplay);
}

void OsdBasicWindow::setQuickViewSource(const QString &url)
{
    if (url.isEmpty()) {
        return;
    }

    // 再次确认 dataManager 是否存在（防御性编程）
    if (m_osdView->rootContext()->contextProperty("dataManager").isNull()) {
        qDebug() << "Warning: dataManager not found in context before loading QML!";
    }

    m_osdView->setSource(QUrl(url));

    if (m_osdView->status() == QQuickView::Error) {
        qDebug() << "QML Load Error:" << m_osdView->errors();
    }
}

void OsdBasicWindow::setOsdSize(uint width, uint height)
{
    setGeometryByWindowSize(width, height);
    blurWindow();
}

void OsdBasicWindow::setGeometryByWindowSize(int width, int height)
{
    int screenStartX = QGuiApplication::primaryScreen()->geometry().x();
    int screenStartY = QGuiApplication::primaryScreen()->geometry().y();
    int osdScreenWidth = QGuiApplication::primaryScreen()->size().width();
    int osdScreenHeight = QGuiApplication::primaryScreen()->size().height();

    int osdStartX = screenStartX + osdScreenWidth * 0.5 - width / 2;
    int osdStartY = screenStartY + osdScreenHeight * 0.88  - height;

    m_osdView->setGeometry(osdStartX, osdStartY, width, height);
}

void OsdBasicWindow::blurWindow()
{
    int blurStartX = BLUR_SIZE / 2 -1;
    int blurStartY = BLUR_SIZE / 2 -1;

    int backgroundWidth = m_osdView->width() - BLUR_SIZE;
    int backgroundHeight = m_osdView->height() - BLUR_SIZE;

    //初始化路径
    QPainterPath linePath;
    linePath.addRoundedRect(QRect(blurStartX, blurStartY, backgroundWidth + 2, backgroundHeight + 2), 12, 12);
    m_windowHelper->setBlurEffect(QRegion(linePath.toFillPolygon().toPolygon()));
}


void OsdBasicWindow::disableBlurEffect()
{
    if (m_windowHelper) {
        m_windowHelper->setBlurEffect(QRegion(), 0, false);
    }
}