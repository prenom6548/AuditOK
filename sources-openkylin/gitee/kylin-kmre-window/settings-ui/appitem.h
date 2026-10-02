/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef _APP_ITME_H
#define _APP_ITME_H

#include <QWidget>
#include <QLabel>
#include <QListWidgetItem>
#include <QHBoxLayout>

class SwitchButton;

class AppItem : public QWidget
{
    Q_OBJECT

public:
    AppItem(QWidget *parent=0);

    QListWidgetItem* getItem();
    QString getPkgName();
    void setInfo(const QString &appName, const QString &pkgName, bool checked);
    void setChecked(const bool checked = true);
    void highlight();
    void unhighlight();

signals:
    void enter();
    void checkedChanged(const QString &pkgName, const bool checked) const;

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    QString m_pkgName;
    QLabel *m_appNameLabel = nullptr;
    SwitchButton *m_switchBtn = nullptr;
    QHBoxLayout *m_btnLayout = nullptr;
    QHBoxLayout *m_infoLayout = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QListWidgetItem *m_item = nullptr;
    QLabel *m_iconLabel = nullptr;
    bool m_isEntered;
};

#endif // _APP_ITME_H
