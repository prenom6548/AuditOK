/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef DEBINFOWIDGET_H
#define DEBINFOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QStandardPaths>
#include <QDebug>
#include <QVariant>
#include <QFile>
#include <QIcon>
#include <QGSettings>

class debInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit debInfoWidget(QString m_icon , QString m_name ,QString appName,QString appVersion,QWidget *parent = nullptr);


    QString m_appName;
    QString m_appVersion;
    QString m_icon;
    QString m_name;

    QVBoxLayout *debInfoLayout;
    //app  icon name  layout
    QWidget *IconAndAppNameWidget;
    QHBoxLayout *IconAndAppNameLayout;
    QLabel *debIconLabel;
    QLabel *debAppNameLabel;
    //deb version name layout
    QLabel       *debNameLabel_pkg;
    QWidget *debNameAndVersionWidget;
    QGSettings   *m_pGsettingFontSize;
    QHBoxLayout *debNameAndVersionLayout;
    QLabel *debNameLabel;
    QLabel *debVersionLabel;
    QWidget *nameVersionWidget;
    QVBoxLayout *nameVersionVLayout;

    QString getIconPath(QString appName);
    QString getAppCnName(QString appName);
    QString setdebnamelable();
    QString SpliteText(QFontMetrics font, QString text,int nlablesize);
    QString setLabelStringBody(QString text, QLabel *lable);
    void    setLabelText();
    void    fontGsettingInit();
    void    resetAppInfo(QString appName,QString appVersion);

signals:

};

#endif // DEBINFOWIDGET_H
