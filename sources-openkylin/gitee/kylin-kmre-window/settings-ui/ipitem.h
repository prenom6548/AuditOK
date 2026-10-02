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

#ifndef IPITEM_H
#define IPITEM_H

#include "settingsitem.h"

class QLabel;
class QLineEdit;

class IpItem : public SettingsItem
{
    Q_OBJECT
    Q_PROPERTY(QString ip READ getIP WRITE setIP)

public:
    explicit IpItem(QFrame* parent = 0);
    void setTitle(const QString &title);
    QString getIP() const;
    void clear();

public slots:
    void setIP(const QString &ip);
    void textChanged(const QString &text);

signals:
    void BtnEnable();
protected:
//    bool eventFilter(QObject *watched, QEvent *event);

private:
    QLabel *m_titleLabel;
//    QLabel *m_dot1;
//    QLabel *m_dot2;
//    QLabel *m_dot3;
//    QLineEdit *m_ipEdit1;
//    QLineEdit *m_ipEdit2;
//    QLineEdit *m_ipEdit3;
//    QLineEdit *m_ipEdit4;
    QLineEdit *m_ipEdit;
    QString m_ip;
};

#endif // IPITEM_H
