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

#ifndef GLESVERSIONWIDGET_H
#define GLESVERSIONWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QListWidget>
#include <QPushButton>

#include "settingscrollcontent.h"

class RadioButtonItem;

class GlesVersionWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit GlesVersionWidget(const QString &gpuVendor = QString(""), QWidget* parent = 0);
    ~GlesVersionWidget();

    void initRadioBtns();
    void saveGlesVersion(const QString &version);
    QString loadGlesVersion();
//    void enablePage();

signals:
    void restartEnv();

public slots:
    void onRadioButtonClicked(int index);

private:
    QSettings *m_qsettings = nullptr;
    QWidget *m_radiosWidget = nullptr;
    QString m_defaultGles;
    QString m_confName;
    QPushButton *m_rebootBtn;
    QList<RadioButtonItem *> m_itemList;
    QStringList m_descLists;
    QButtonGroup *m_btnGroup;
    QStringList m_glesLists;
};

#endif // GLESVERSIONWIDGET_H
