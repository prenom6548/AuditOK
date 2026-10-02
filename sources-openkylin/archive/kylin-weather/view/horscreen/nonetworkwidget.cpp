/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "nonetworkwidget.h"
#include <QDebug>

NoNetworkWidget::NoNetworkWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
}

void NoNetworkWidget::initUI()
{
    // 设置主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->setSpacing(20);

    // 创建图标标签
    m_iconLabel = new QLabel(this);
    m_iconLabel->setAccessibleName("kylin-weather_QLabel_NoNetworkWidget_iconLabel");
    QPixmap iconPixmap = QPixmap(":/res/contorl_icons/network_warn_dark.png");

    iconPixmap = iconPixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_iconLabel->setPixmap(iconPixmap);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    // 创建文本标签
    m_textLabel = new QLabel(tr("Network Error Code: 404. Please connect to the network and try again."), this);
    m_textLabel->setAccessibleName("kylin-weather_QLabel_NoNetworkWidget_textLabel");
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setWordWrap(true);
    m_textLabel->setStyleSheet("color: white; font-size: 14px;");

    // 创建重试按钮
    m_retryButton = new QPushButton(tr("Network Setting"), this);
    m_retryButton->setAccessibleName("kylin-weather_QPushButton_NoNetworkWidget_retryButton");
    m_retryButton->setFixedSize(116, 36);
    m_retryButton->setStyleSheet(
        "QPushButton {color: white; font-size: 14px;}"
        "QPushButton:!hover {background-color:rgba(255,255,255,19); border-radius:6px;}"
        "QPushButton:hover {background-color:rgba(255,255,255,25); border-radius:6px;}"
        "QPushButton:pressed {background-color:rgba(255,255,255,14); border-radius:6px;}"
    );

    // 创建水平布局用于居中按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_retryButton);
    buttonLayout->addStretch();

    // 添加到主布局
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_iconLabel);
    m_mainLayout->addWidget(m_textLabel);
    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addStretch();

    this->setLayout(m_mainLayout);
}

void NoNetworkWidget::initConnect()
{
    connect(m_retryButton, &QPushButton::clicked, this, &NoNetworkWidget::onSettingClicked);
}

void NoNetworkWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.fillRect(this->rect(), QColor("#1E1E1E"));
}

void NoNetworkWidget::onSettingClicked()
{
    if(m_process)
        m_process->deleteLater();

    m_process = new QProcess();
    QString cmd = "ukui-control-center";
    QStringList arg;
    arg.clear();
    arg << "-m";
    arg << "netconnect";
    m_process->startDetached(cmd, arg);
}
