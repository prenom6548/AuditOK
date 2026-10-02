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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QColor>
#include "shortcut-button.h"
#include "status-info.h"

class Shortcut : public QWidget
{
    Q_OBJECT
public:
    explicit Shortcut(QString displayName, QString iconName, QColor color, QWidget *parent = nullptr);
public Q_SLOTS:
    void statusChanged(UkuiShortcut::StatusInfo info);
Q_SIGNALS:
    void clicked();

private:
    QVBoxLayout *m_layout = nullptr;
    QLabel *m_nameLabel = nullptr;
    ShortcutButton * m_button = nullptr;

};

#endif // SHORTCUT_H
