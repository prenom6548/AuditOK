/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#include "loadingwidget.h"

#include <QBoxLayout>
#include <QMovie>
#include <QIcon>
#include <syslog.h>

#define ICON_WIDTH 128
#define ICON_HEIGHT 128

LoadingWidget::LoadingWidget(QWidget *parent)
    : QWidget(parent)
    , mUseUkuiThemeIcon(true)
    , mCounter(0)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_label = new QLabel(this);
    m_label->setFixedSize(ICON_WIDTH, ICON_HEIGHT);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_label, 0, Qt::AlignCenter);
    layout->addStretch();

    this->setLayout(layout);

    QIcon icon1 = QIcon::fromTheme("ukui-loading-0-symbolic");
    QIcon icon2 = QIcon::fromTheme("ukui-loading-1-symbolic");
    QIcon icon3 = QIcon::fromTheme("ukui-loading-2-symbolic");
    QIcon icon4 = QIcon::fromTheme("ukui-loading-3-symbolic");
    QIcon icon5 = QIcon::fromTheme("ukui-loading-4-symbolic");
    QIcon icon6 = QIcon::fromTheme("ukui-loading-5-symbolic");
    QIcon icon7 = QIcon::fromTheme("ukui-loading-6-symbolic");
    QIcon icon8 = QIcon::fromTheme("ukui-loading-7-symbolic");

    if ((!icon1.isNull()) && (!icon2.isNull()) && (!icon3.isNull()) && (!icon4.isNull())
            && (!icon5.isNull()) && (!icon6.isNull()) && (!icon7.isNull()) && (!icon8.isNull())) {
        mIconList.clear();
        mIconList.append(icon1.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon2.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon3.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon4.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon5.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon6.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon7.pixmap(ICON_WIDTH, ICON_HEIGHT));
        mIconList.append(icon8.pixmap(ICON_WIDTH, ICON_HEIGHT));

        m_label->setPixmap(mIconList.at(0));

        mTimer = new QTimer(this);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(onUpdateIcon()));
        mUseUkuiThemeIcon = true;
    }
    else {
        m_movie = new QMovie(":/res/loadgif.gif");
        m_label->setMovie(m_movie);
        mUseUkuiThemeIcon = false;
    }
}

LoadingWidget::~LoadingWidget()
{
    if (mTimer) {
        delete mTimer;
        mTimer = nullptr;
    }

    if (m_movie) {
        delete m_movie;
        m_movie = nullptr;
    }
}

void LoadingWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (mUseUkuiThemeIcon) {
        if (mTimer) {
            mTimer->start(100);
        }
    }
    else {
        if(m_movie && m_movie->state() != QMovie::Running) {
            m_movie->start();
        }
    }
}

void LoadingWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    if (mUseUkuiThemeIcon) {
        if (mTimer) {
            mTimer->stop();
        }
    }
    else {
        if (m_movie) {
            m_movie->stop();
        }
    }
}

void LoadingWidget::onUpdateIcon()
{
    if (mUseUkuiThemeIcon) {
        mCounter++;
        int idx = mCounter % mIconList.size();
        m_label->setPixmap(mIconList.at(idx));
    }
}

