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

#ifndef DISPLAYMODEWIDGET_H
#define DISPLAYMODEWIDGET_H

#include <QWidget>
#include <QSettings>

#include "settingscrollcontent.h"

class QButtonGroup;
class QPushButton;
class RadioButtonItem;

class DisplayModeWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit DisplayModeWidget(const QString &displayType = QString(""), const QString &gpuVendor = QString(""), QWidget * parent = 0);
    ~DisplayModeWidget();

    void initInfo();
    void initRadioBtns();
    void initCurrentValue();
    void saveMode(const QString &mode);
    QString loadMode();
    bool isAMDGraphicCard();
//    void enablePage();

signals:
    void restartEnv();
public slots:
    void onRadioButtonClicked(int index);

private:
    QSettings *m_qsettings = nullptr;
    QPushButton *m_rebootBtn;
    QString m_currentDisplayType;
    QString m_confName;
    QList<RadioButtonItem *> m_itemList;
    QWidget *m_radiosWidget = nullptr;
    QButtonGroup *m_btnGroup;
//    QMap<QString,QString> m_modeMap;
    QStringList m_nameList;
    QStringList m_descList;
    QString m_gpuVendor;
    QString m_gpuModel;
    QString m_cpuType;
    QString m_displayType;
    QString m_displayTypes;
};


#endif // DISPLAYMODEWIDGET_H
