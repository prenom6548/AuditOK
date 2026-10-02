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
#ifndef PACKAGE_H
#define PACKAGE_H

#include <QString>
#include "PackageEnum.h"
#include <QDebug>
#include <QProcess>
#include "PackageUtils.h"
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
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
#include <QProgressBar>
#include <QMessageBox>
#include <QProcess>
#include <QCoreApplication>
#include <QApt/Backend>
#include <QFileInfo>
#include <QStorageInfo>
#include <QApt/DebFile>
#include <QToolButton>
#include "../src/common/utils/CustomBoxUtil.h"
#include <QRegExp>
#include <QRegularExpression>

enum packageType 
{
    TPYE_OTHER = 0,
    TPYE_DEBIAN,
    TPYE_KARE,
    TPYE_KAIMING
};

// 先写一个demo，让包的基本信息能够被获取到
class Package
{
public:
    Package(QString debPath, packageType pkgTypt = packageType::TPYE_OTHER);
    virtual void printInfo();
    bool packageArchIsMatch();

public:
    QString m_debPath;
    QString m_name;
    QString m_zhName;
    QString m_version;
    QString m_architecture;
    QString m_errorInfo;
    QString m_md5Sum;
    PackageEnum::Status m_status=PackageEnum::Status::OTHERS;
    QString m_installedVersion;
    QString m_pkgFileName;
    packageType m_pkgType;

private:
    virtual void initPackageInfo() = 0;
    virtual void initPackageInstallStatus() = 0;
    static const QString& detectArch();
};



#endif
