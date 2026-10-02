/*
 * Copyright 2025 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hardwarereader.h"

#include <QDebug>
#include <QProcess>
#include <QFile>

HardwareReader* HardwareReader::m_instance = nullptr;

HardwareReader* HardwareReader::instance()
{
    if (!m_instance) {
        m_instance = new HardwareReader();
    }
    return m_instance;
}

HardwareReader::HardwareReader()
{
    // 初始化CPU信息
    getCpuType();
}

HardwareReader::~HardwareReader()
{
}

bool HardwareReader::isFlemingX()
{
    QString productName = getSystemProductName();
    bool result = productName.contains("L420x", Qt::CaseInsensitive);
    qDebug() << "Hardware detection - Product name:" << productName 
             << "Is FlemingX:" << result;
    return result;
}

QString HardwareReader::getSystemProductName()
{
    return executeCommand("dmidecode -s system-product-name").trimmed();
}

bool HardwareReader::isPgwDevice() const
{
    // 使用dmidecode读取OEM String 4
    QString output = executeCommand("dmidecode -t 11");

    // 检查输出是否为空
    if (output.isEmpty()) {
        qWarning() << "dmidecode command returned empty output";
        return false;
    }

    // 解析输出并查找PWC30标识
    QStringList lines = output.split('\n');
    bool foundString4 = false;
    
    for (const QString &line : lines) {
        if (line.contains("String 4:")) {
            foundString4 = true;
            qDebug() << "Found OEM String 4:" << line.trimmed();
            
            if (line.contains("PWC30", Qt::CaseInsensitive)) {
                qDebug() << "PGW device detected with PWC30 identifier";
                return true;
            }
        }
    }
    
    // 记录未找到String 4的情况
    if (!foundString4) {
        qDebug() << "OEM String 4 not found in dmidecode output";
    }
    
    return false;
}

QString HardwareReader::executeCommand(const QString &command) const
{
    QProcess process;

   // 步骤1: 启动进程并设置超时
   process.start("bash", QStringList() << "-c" << command);

   // 等待进程完成，添加超时处理(5秒)
   if (!process.waitForFinished(5000)) {
       qWarning() << "Command execution timed out:" << command;
       return QString();
   }
   
   // 步骤2: 处理结果
   // 检查是否执行成功
   if (process.exitCode() != 0) {
       qWarning() << "Command execution failed:" << command;
       qWarning() << "Error:" << process.readAllStandardError();
       return QString();
   }

   // 读取并返回结果
   return process.readAllStandardOutput();
}

void HardwareReader::getCpuType()
{
    if (false == readFile(UPM_CPU_INFO_FILE, m_cpuType)) {
        qDebug() << "get cpu info failed";
    }
}

bool HardwareReader::readFile(const QString &filePath, QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "read file error :" << filePath;
        return false;
    }

    content = file.readAll();
    file.close();

    return true;
}

bool HardwareReader::isPanguM900() const
{
    return m_cpuType.contains("PANGU M900", Qt::CaseInsensitive);
}

QString HardwareReader::getDMIInfo(const QString field) const
{
    QFile file(DMI_PATH + field);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "file " + DMI_PATH + field + " not exist!";
        return QString();
    }
    QString value;
    value = file.readAll();
    file.close();

    value.remove('\n');

    qInfo() << "get DMIInfo " << field << " is " << value;
    return value;
}
