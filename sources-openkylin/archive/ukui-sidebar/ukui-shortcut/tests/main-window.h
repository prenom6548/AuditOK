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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>
#include "short-cut-manager.h"
#include "shortcut.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    UkuiShortcut::ShortcutManager *m_shortcutManager = nullptr;
    QList<UkuiShortcut::UkuiShortcutPlugin*> m_shortcutPlugins;
    QWidget *m_centralWidget = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QList<Shortcut *> m_shortcuts;

};
#endif // MAINWINDOW_H
