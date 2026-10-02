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

#ifndef INPUTITEM_H
#define INPUTITEM_H

#include "settingsitem.h"
#include <QLabel>
#include <QLineEdit>

class InputItem : public SettingsItem
{
    Q_OBJECT

public:
    explicit InputItem(QFrame* parent = 0);

    void setTitle(const QString &title1, const QString &title2);
    const QString package() const { return m_pkgEdit->text(); }
    QPair<QString, QString> getInformation();
    void clear();

private:
    QLabel *m_appTitle;
    QLabel *m_pkgTitle;
    QLineEdit *m_appEdit;
    QLineEdit *m_pkgEdit;
};

#endif // INPUTITEM_H
