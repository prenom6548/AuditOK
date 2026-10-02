/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "custom_refresh_button.h"
#include <QLabel>
#include <QHBoxLayout>

CustomRefreshButton::CustomRefreshButton(QWidget *parent)
    : CustomPushButton(parent)
{
    m_label = new QLabel(this);
    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->addWidget(m_label);
    this->setLayout(layout);

    QMovie *movieLoading = new QMovie(this);
    movieLoading->setFileName(":/main_win_ui/loading.gif");
    movieLoading->setScaledSize(QSize(16,16));
    m_label->setMovie(movieLoading); //初始默认图片0
    m_label->setAlignment(Qt::AlignCenter);
    movieLoading->start();
    refreshFinish();
}

CustomRefreshButton::~CustomRefreshButton() 
{
}

void CustomRefreshButton::refreshStart()
{
    if (m_showText.size()) {
        CustomPushButton::setText(QString());
    } else {
        setIcon(QIcon());
    }

    m_label->show();
    setEnabled(false);
}

void CustomRefreshButton::refreshFinish()
{
    m_label->hide();

    if (m_showText.size()) {
        CustomPushButton::setText(m_showText);
        setIcon(QIcon());
    } else {
        setIcon(QIcon::fromTheme("view-refresh-symbolic"));
        CustomPushButton::setText(QString());
    }

    setEnabled(true);
}

void CustomRefreshButton::setText(const QString &text)
{
    CustomPushButton::setText(text);
    m_showText = text;
    refreshFinish();
}
