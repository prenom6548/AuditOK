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
#ifndef NONETWORKWIDGET_H
#define NONETWORKWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QProcess>
#include "controller/Interaction.h"
#include "global/viewvar.h"

class NoNetworkWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NoNetworkWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();
    void initConnect();

    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_textLabel = nullptr;
    QPushButton *m_retryButton = nullptr;
    QProcess *m_process = nullptr;

private slots:
    void onSettingClicked(); // 重试按钮点击事件

signals:
    void settingClicked(); // 重试信号
};

#endif // NONETWORKWIDGET_H
