/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
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

#ifndef ADDGAMEWIDGET_H
#define ADDGAMEWIDGET_H

#include <QWidget>
#include <QPushButton>

class SettingsGroup;
class InputItem;
class QCheckBox;
class QSettings;

#include "settingscrollcontent.h"

class AddGameWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit AddGameWidget(QWidget* parent = 0);
    ~AddGameWidget();
    void initUi();
    void initConnection();
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
#ifdef UKUI_WAYLAND
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
#endif

private slots:
    void addItem();
    void removeItem(const QString &pkg);

private:
    SettingsGroup *m_group;
    QPushButton *m_resetButton;
    QPushButton *m_okButton;
    QPushButton *m_closeBtn;
    QList<InputItem *> m_itemList;
    QWidget *mainWid;
};

#endif // ADDGAMEWIDGET_H
