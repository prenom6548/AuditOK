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
#include "Package.h"

QMap<packageType, QString> packageTypeStr = {
    { packageType::TPYE_OTHER,      "TPYE_OTHER"    },
    { packageType::TPYE_DEBIAN,     "TPYE_DEBIAN"   },
    { packageType::TPYE_KARE,       "TPYE_KARE"     },
    { packageType::TPYE_KAIMING,    "TPYE_KAIMING"  }
};

Package::Package(QString debPath, packageType pkgTypt):
    m_debPath(debPath),
    m_pkgType(pkgTypt)
{

}

void Package::printInfo()
{
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "debPath:"      << m_debPath;
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "name:"         << m_name;
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "zhName:"       << m_zhName;
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "version:"      << m_version;
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "architecture:" << m_architecture;
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "status:"       << m_status;
    qDebug() << "[" << packageTypeStr.value(m_pkgType) << "]" << "md5Sum:"       << m_md5Sum;
}

const QString& Package::detectArch()
{
    static const QString cachedArch = []() -> QString {
        QString output;
        QProcess process;
        process.start("dpkg", QStringList() << "--print-architecture");
        if (process.waitForFinished(2000)) {
            output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
        }

        if (output.isEmpty()) {
            process.start("uname", {"-m"});
            if (process.waitForFinished(2000)) {
                output = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
            }
        }

        return output.isEmpty() ? "unknown" : output;
    }();

    qDebug() << "Package detect system architecture:"<< cachedArch;

    return cachedArch;
}

bool Package::packageArchIsMatch()
{
    static const QString cachedArch = detectArch();

    if (m_pkgType == packageType::TPYE_KAIMING) {
        return true;
    }

    if (cachedArch.isEmpty() || cachedArch == "unknown") {
        qDebug() << "Failed to detect system architecture, using cached value:" << cachedArch;
        return false;
    }

    if (m_architecture.isEmpty() || m_architecture == "unknown") {
        qDebug() << "Package architecture is empty or unknown, using cached value:" << m_architecture;
        return false;
    }

    static const QStringList arch_alias = {
        "loongarch64",
        "loong64"
    };

    if (arch_alias.contains(m_architecture) && arch_alias.contains(cachedArch)) {
        return true;
    }

    if ( m_architecture.compare("all") && m_architecture.compare(cachedArch) ) {
        qDebug() << "The software package architecture does not match the native architecture";
        return false;
    }

    return true;
}