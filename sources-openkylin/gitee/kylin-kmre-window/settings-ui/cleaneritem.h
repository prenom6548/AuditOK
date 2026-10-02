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

#ifndef CLEANER_ITME_H
#define CLEANER_ITME_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QStyleOption>
#include <QPainter>
#include <QListWidgetItem>

class QPushButton;

class CleanerItem : public QWidget
{
    Q_OBJECT
public:
    explicit CleanerItem(const QJsonObject &value, const QString &currentImage, const QString &container = "", QWidget *parent = 0);
    void setIcon(const QString &iconName);
    void setId(const QString &id);
    void setName(const QString &name);
    void setCreatedTime(const QString &time);
    void setSize(const QString &sz);
    void setContainerName(const QString &name);
    void setActiveStatus(bool b);
    void setCustomFont(QLabel *label, QFont referenceFont);
    void freeItem();
    QListWidgetItem *getItem();

signals:
    void requestRemove(const QString &id, const QString &containerName, const QString &imageName);
    void itemRemove(QListWidgetItem *item);

public slots:
    void onRemove();

protected:
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel *m_iconLabel;
//    QLabel *m_idLabel;
    QLabel *m_nameLabel;
    QLabel *m_timeLabel;
    QLabel *m_szLabel;
//    QLabel *m_containerLabel;
    QLabel *m_tipLabel;
    QPushButton *m_removeButton;
    QHBoxLayout *m_mainLayout;
    QPropertyAnimation *m_animation;
    QString m_imageid;
    QString m_imageName;
    QString referencedContainer;
    bool m_active;
    QListWidgetItem *m_item = nullptr;
};

#endif // CLEANER_ITME_H
