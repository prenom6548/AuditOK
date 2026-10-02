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

#ifndef RADIOBUTTONITEM_H
#define RADIOBUTTONITEM_H

#include "settingsitem.h"

#include <QAbstractButton>

class QLabel;
class QPushButton;
class QRadioButton;

class RadioButtonItem : public SettingsItem
{
    Q_OBJECT

public:
    explicit RadioButtonItem(QButtonGroup *btnGroup, int index, QFrame* parent = 0);

    void setTitle(const QString &name, const QString &desc);
    void setChecked();
    void setConfirmButtonVisible(bool b);
    QString getName() const;

signals:
    void radioBtnClicked(int index);
    void restartEnv();

private:
    QRadioButton *m_radioBtn = nullptr;
    QLabel *m_label = nullptr;
    QPushButton *m_confirmBtn = nullptr;
    QString m_name;
    int m_index = -1;
};

#endif // RADIOBUTTONITEM_H
