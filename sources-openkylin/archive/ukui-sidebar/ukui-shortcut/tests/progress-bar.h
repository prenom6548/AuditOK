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
 *
 */

//
// Created by hxf on 22-7-21.
//

#ifndef UKUI_SIDEBAR_PROGRESS_BAR_H
#define UKUI_SIDEBAR_PROGRESS_BAR_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>

class ProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = nullptr);

    void setIcon(const QString &icon);

public Q_SLOTS:
    void setValue(int size);

Q_SIGNALS:
    void add(int);

private:
    QProgressBar *m_progressBar = nullptr;
    QPushButton *m_button = nullptr;
    QLabel *m_label = nullptr;
};


#endif //UKUI_SIDEBAR_PROGRESS_BAR_H
