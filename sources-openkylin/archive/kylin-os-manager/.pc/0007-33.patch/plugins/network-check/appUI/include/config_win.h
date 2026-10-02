/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef CONFIG_WIN_H
#define CONFIG_WIN_H

#include <QWidget>
#include <QDialog>
#include <QGSettings>
#include "libBase.h"
#include "mainwindow.h"

namespace Ui
{
class ConfigWin;
}

class ConfigWin : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWin(QWidget *parent = nullptr);
    ~ConfigWin();

public slots:
    void showWin();
    void resizeWinSize();
    void recordChange();
private slots:
    void setInnerCheckShow(bool isCheck);
    void cancelPress();
    void savePress();
signals:
    void refreshUI();
    void sigUpdateConfigFile();

private:
    Ui::ConfigWin *ui;
    void setWin();
    bool saveSettings();
    InnerNetCheck m_innerCheckArgSettings;
    QGSettings *m_checkSettings = nullptr;
    bool m_isShow = false;
    bool m_isON = true;
};

#endif
