/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "curtain.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineFullScreenRequest>
#include <QDesktopWidget>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QVBoxLayout>
#include <QAction>
#include <QCursor>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <generatetools.h>

Curtain::Curtain(QWidget *parent) : QWidget(parent), m_view(new QWebEngineView(this))
{
    setWindowFlags(Qt::FramelessWindowHint); //无边框
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_view);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_view->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &Curtain::fullScreenRequested);
    connect(m_view, &QWebEngineView::loadFinished, this, [=](bool bo) {
        qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
        QString common = "xdotool mousemove 5 5 click 1";
        system(common.toUtf8().constData());
        qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));
    });

    m_suspendTabBar = new SuspendTabBar(SuspendTabBar::Display::Whole, false, this);
    connect(m_suspendTabBar, &SuspendTabBar::sigBtnClicked, this, &Curtain::slotTabBtnClicked);
    m_view->stackUnder(m_suspendTabBar);

    auto exitAction = new QAction(this);
    exitAction->setShortcut(Qt::Key_Escape);
    addAction(exitAction);
    connect(exitAction, &QAction::triggered, [this]() {
        Q_EMIT sigStateChange(Curtain::States::ReturnDesktop);
        hideView();
    });
}

Curtain::~Curtain() {}

void Curtain::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    if (m_suspendTabBar) {
        m_suspendTabBar->setTheme(m_theme);
    }
}

void Curtain::showView()
{
    if (isHidden()) {
        show();
        showMaximized();
        showFullScreen();
    }
}

void Curtain::hideView()
{
    if (!isHidden()) {
        hide();
    }
}

void Curtain::hideTabBar()
{
    if (m_suspendTabBar != nullptr) {
        m_suspendTabBar->hideTabBar();
    }
}

void Curtain::showTabBar()
{
    if (m_suspendTabBar != nullptr) {
        m_suspendTabBar->showTabBar();
    }
}

void Curtain::resizeEvent(QResizeEvent *event)
{
    QRect viewGeometry(QPoint(0, 0), size());
    m_view->setGeometry(viewGeometry);
    m_suspendTabBar->showTabBar();

    QWidget::resizeEvent(event);
}

void Curtain::loadUrl(QString &url)
{
    qInfo() << "Sending request to projection service.";
    m_url = url;
    m_view->setUrl(QUrl(m_url));
    showMaximized();
    showFullScreen();
}

void Curtain::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    request.accept();
    showMaximized();
    showFullScreen();
}

void Curtain::slotTabBtnClicked(SuspendTabBar::BtnType type)
{
    switch (type) {
    case SuspendTabBar::BtnType::ControlBtn: {
        // 暂时先模拟键盘进入反控
        hideTabBar();
        Q_EMIT sigStateChange(Curtain::States::Control);
    } break;
    case SuspendTabBar::BtnType::ReturnDesktopBtn: {
        hideView();
        Q_EMIT sigStateChange(Curtain::States::ReturnDesktop);
    } break;
    case SuspendTabBar::BtnType::ExitBtn: {
        hideView();
        Q_EMIT sigStateChange(Curtain::States::Exit);
    } break;
    }
}