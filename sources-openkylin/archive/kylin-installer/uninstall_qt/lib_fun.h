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
#ifndef __LIB_FUN_H__
#define __LIB_FUN_H__

#include <QString>
#include <QSettings>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QStandardPaths>
#include <QSqlError>

#include <unistd.h>

int snow_analysis_xml(const QString &p_desktop_path_in , QString &p_pkg_name_out , QString &p_runtime_out);
QString getAndroidPackageName(QString desktopFilePath);
QString snow_get_local_pkg_name(QString p_desktop_path_in);

QString get_zhname(QString p_desktop_path_in);
void remove_last_line_break(QString &src_in_out);
QString intercept_chinese_characters(QString src_in);
QString SelectPackgeName(QString p_desktop_in);
QString getKarePackageNameFromFile(const QString &filePath);
QString resolveRealFile(const QString &);
QString get_namezh_util(QString desktopPath, QString appName);
QString get_zhname_from_desktop(QString desktopPath);
QString get_zhname_from_uksc(const QString &appName);

#endif
