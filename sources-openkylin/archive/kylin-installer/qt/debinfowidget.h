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
#include <QPixmap>
#include <QGSettings>
#include <QFileInfo>

#define ORG_UKUI_STYLE_FONT            "org.ukui.style"
#define GSETTING_FONT_KEY              "systemFontSize"

class debInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit debInfoWidget(QString appName,QString appVersion,QString appPath,QWidget *parent = nullptr);

    QString      m_appName;
    QString      m_appVersion;
    QString      m_appPath;
    QVBoxLayout  *debInfoLayout;
    /*app  icon name  layout*/
    QWidget      *IconAndAppNameWidget;
    QVBoxLayout  *IconAndAppNameLayout;
    QHBoxLayout  *deb_nameLayout;
    QLabel       *debIconLabel;

    QHBoxLayout  * Namelyout;
    QHBoxLayout  * Versionlyout;
    /*deb version name layout*/
    QLabel       *debAppNameLabel;
    QWidget      *debNameAndVersionWidget;
    QHBoxLayout  *debNameAndVersionLayout;
    QLabel       *debNameLabel;
    QLabel       *debNameLabel_pkg;
    QLabel       *debVersionLabel;
    QWidget      *nameVersionWidget;
    QVBoxLayout  *nameVersionVLayout;
    QGSettings   *m_pGsettingFontSize;
    QString      getIconPath(QString appName);
    QString      setLabelStringBody(QString text, QLabel *lable);
    QString      SpliteText(QFontMetrics font, QString text,int nlablesize);
    QString      getAppCnName(QString appName);
    void         resetAppInfo(QString appName, QString appVersion);
    void         fontGsettingInit();
    void         setLabelText();
    void         setdebnamelable();
    void         setdebNamelable();
    QFont        getSystemFont(int size);
};

#endif // DEBINFOWIDGET_H
