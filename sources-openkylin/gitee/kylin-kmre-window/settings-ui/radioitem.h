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

#ifndef RADIOITEM_H
#define RADIOITEM_H

#include "settingsitem.h"

#include <QAbstractButton>

class QLabel;
class QButtonGroup;
class QRadioButton;

class RadioItem : public SettingsItem
{
    Q_OBJECT

public:
    explicit RadioItem(QFrame* parent = 0);

    void setTitle(const QString &title, const QString &name1, bool support1, const QString &name2, bool support2);
    void setTip(const QString &tip1, const QString &tip2);
    void setDefaultValue(const QString& value);
    const QString currentText() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void onButtonClicked(QAbstractButton *button);

signals:
    void sendSelectedMode(const QString &mode);

private:
    QLabel *m_titleLabel = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
    QRadioButton *m_btn1 = nullptr;
    QRadioButton *m_btn2 = nullptr;
    QString m_name1;
    QString m_name2;
    QString m_tip1;
    QString m_tip2;
};

#endif // RADIOITEM_H
