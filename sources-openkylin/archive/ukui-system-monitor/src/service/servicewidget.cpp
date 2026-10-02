/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "servicewidget.h"

#include <QPainterPath>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QDebug>

#include "../macro.h"
#include "../util.h"

ServiceWidget::ServiceWidget(QSettings *proSettings, QWidget *parent)
    : QWidget(parent)
    , m_svcSettings(proSettings)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initConnections();
}

ServiceWidget::~ServiceWidget()
{
    if (m_qtSettings) {
        delete m_qtSettings;
        m_qtSettings = nullptr;
    }
}

void ServiceWidget::onWndClose()
{
    if (m_svcTabView) {
        m_svcTabView->onWndClose();
    }
}

void ServiceWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ServiceWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(12,0,0,0);
    m_mainLayout->setSpacing(0);
    m_svcTableLayout = new QVBoxLayout();
    m_svcTableLayout->setContentsMargins(0,0,0,0);
    m_svcTableLayout->setSpacing(0);

    // 初始化控件

    //初始化进程列表
    m_svcTabView = new ServiceTableView(m_svcSettings);
    m_svcTabView->installEventFilter(this);
    m_svcTableLayout->addWidget(m_svcTabView);

    // 初始化loading指示器
    m_loadingWidget = new LoadingWidget(this);
    m_loadingWidget->setVisible(false);
    m_svcTableLayout->addWidget(m_loadingWidget);

    m_mainLayout->addLayout(m_svcTableLayout);
    this->setLayout(m_mainLayout);
    initThemeMode();
}

void ServiceWidget::initConnections()
{
    connect(this, SIGNAL(searchSignal(QString)), m_svcTabView, SLOT(onSearch(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(switchOnWidget()), m_svcTabView, SLOT(onSwitchOnView()));
}

void ServiceWidget::initThemeMode()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_qtSettings = new QGSettings(idd);
    }
    if (m_qtSettings) {
        //监听主题改变
        connect(m_qtSettings, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == "styleName") {
            } else if ("iconThemeName" == key) {
            } else if("systemFont" == key || "systemFontSize" == key) {
                qreal fontSize = m_qtSettings->get(FONT_SIZE).toString().toFloat();
                onThemeFontChanged(fontSize);
            }
        });
        qreal fontSize = m_qtSettings->get(FONT_SIZE).toString().toFloat();
        onThemeFontChanged(fontSize);
    }
}

void ServiceWidget::onThemeFontChanged(float fFontSize)
{
    Q_UNUSED(fFontSize);
}

void ServiceWidget::showLoadingIndicator(bool show)
{
    if (show) {
        m_loadingWidget->setVisible(true);
        m_svcTabView->setVisible(false);
        m_loadingWidget->startLoading();
    } else {
        m_loadingWidget->stopLoading();
        m_loadingWidget->setVisible(false);
        m_svcTabView->setVisible(true);
    }
}
