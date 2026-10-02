/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QKeyEvent>
#include <functional>
#include <QDBusInterface>
#include "button.h"
#include <ukuiwindowhelper/ukuiwindowhelper.h>

typedef std::function<void()> Action;


class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    Button* createButton(const QString& text, const QString& img);
    void initTitleUi();
    void initButtons();
    void initLine();
    void initBottom();
    void initUi();

    void initStyleSettings();

    bool initNetworkDisplayButton();
    void showWidget();
protected:
    void keyPressEvent(QKeyEvent* event);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);
private:
    void selectedLast();
    void selectedNext();
    void confirmCurrent();
    void setWindowCenter();
    void addShortcutAction(const QKeySequence& keys, Action action);
    void addShortcutActions();

    void initWindowHelper();
public Q_SLOTS:
    void msgReceiveAnotherOne();
private:
    QWidget* m_titleWidget = nullptr;
    QWidget* m_btnsWidget = nullptr;
    QWidget* m_bottomWidget = nullptr;

    QVBoxLayout* m_mainLayout = nullptr;
    QLabel* m_title = nullptr;
    QHBoxLayout* m_titleLayout = nullptr;
    QVBoxLayout* m_btnsLayout = nullptr;
    QHBoxLayout* m_bottomLayout = nullptr;
    QPushButton* m_networkDisplay = nullptr;

    QButtonGroup* m_buttonGroup = nullptr;
    QDBusInterface* m_networkDisplayInterface = nullptr;

    UkuiWindowHelper* m_windowHelper = nullptr;
    int m_screenCount = 0;
};
#endif // WIDGET_H
