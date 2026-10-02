/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "upmcustomhardwareconfig.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

Q_GLOBAL_STATIC(UpmCustomHardwareConfig, s_upmCustomHardwareConfig)

UpmCustomHardwareConfig::UpmCustomHardwareConfig(QObject *parent) : QObject(parent)
{
    getHardwareInfo();
    getCpuType();

    m_globalConfig = new QSettings(UPM_GLOBAL_HARDWARE_CONFIG_FILE, QSettings::IniFormat, this);

    m_globalConfig->beginGroup(UPM_GROUP_THIS_PRODUCT);
    //是否是首次运行
    if (1 == m_globalConfig->value(UPM_KEY_FIRST_RUN).toInt()) {
        //是否有扩展配置，对应品牌
        if (true == getExtendConfigFile(m_extendConfigPath)) {
            //有扩展配置，将配置文件路径写入全局配置文件
            m_globalConfig->setValue(UPM_KEY_EXTEND_CONFIG_PATH, m_extendConfigPath);

            m_extendConfig = new QSettings(m_extendConfigPath, QSettings::IniFormat, this);
            m_extendConfig->beginGroup("product");

            //配置文件中group[product]为产品标识，标识与硬件信息对应做为group，标识下的key为配置内容
            QStringList productList = m_extendConfig->allKeys();
            Q_FOREACH (const QString &product, productList) {
                if (m_hardwareInfo.contains(product, Qt::CaseInsensitive)) {
                    //扩展配置中，有对应产品型号，将标识写入全局配置
                    if (1 == m_extendConfig->value(product).toInt()) {
                        m_productGroup = product;
                        m_globalConfig->setValue(UPM_KEY_DEVICE_IDENTIFICATION, m_productGroup);
                    }
                    break;
                }
            }
            m_extendConfig->endGroup();
        }
        m_globalConfig->setValue(UPM_KEY_FIRST_RUN, 0);
    } else {
        //不是首次运行，直接从全局配置中读取配置文件及产品标识
        m_extendConfigPath = m_globalConfig->value(UPM_KEY_EXTEND_CONFIG_PATH).toString();
        m_productGroup = m_globalConfig->value(UPM_KEY_DEVICE_IDENTIFICATION).toString();
        if (false == m_extendConfigPath.isEmpty()) {
            qDebug() << "extend config path:" << m_extendConfigPath;
            m_extendConfig = new QSettings(m_extendConfigPath, QSettings::IniFormat, this);
        }
    }

    m_globalConfig->endGroup();
    m_globalConfig->sync();
}

UpmCustomHardwareConfig::~UpmCustomHardwareConfig() {}

UpmCustomHardwareConfig *UpmCustomHardwareConfig::self()
{
    return s_upmCustomHardwareConfig;
}

bool UpmCustomHardwareConfig::isPanguM900()
{
    if (true == m_cpuType.contains("PANGU M900", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UpmCustomHardwareConfig::isZhaoxinCpu()
{
    QString vendorId;

    QStringList lines = m_cpuType.split("\n");
    for (const QString &line : lines) {
        if (line.startsWith("vendor_id")) {
            vendorId = line.split(":").last().trimmed();
            break;
        }
    }

    // 判断是否是兆芯平台
    if (vendorId == "CentaurHauls" || vendorId == "Shanghai") {
        return true;
    }
    return false;
}

bool UpmCustomHardwareConfig::isHygonCpu()
{
    if (true == m_cpuType.contains("Hygon", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UpmCustomHardwareConfig::isD3000M()
{
    if (true == m_cpuType.contains("D3000M", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UpmCustomHardwareConfig::isMipsCpu()
{
    if (true == m_cpuType.contains("3A4000", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UpmCustomHardwareConfig::brightnessNodeEffective()
{
    QVariant value;
    if (true == readConfig(m_globalConfig,
                           UPM_GROUP_IGNORE_BRIGHTNESS_NODE,
                           UPM_KEY_BOARD_NAME, value)) {
        QStringList boards = value.toString().split(",");
        Q_FOREACH (const QString &board, boards) {
            if (m_hardwareInfo.contains(board, Qt::CaseInsensitive) && !board.isEmpty()) {
                return false;
            }
        }
    }

    return true;
}

QString UpmCustomHardwareConfig::getBrightnessNode()
{
    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig, m_productGroup,
                               UPM_KEY_EXTEND_BRIGHTNESS_NODE, value)) {
            return value.toString();
        }
    }

    QStringList cpuAndNodes = {"ZHAOXIN:acpi_video0",
                               "loongson:loongson-gpu",
                               "Phytium:ec_bl"};

    Q_FOREACH (const QString &str, cpuAndNodes) {
        QStringList cpuAndNode = str.split(":");
        if (m_cpuType.contains(cpuAndNode[0], Qt::CaseInsensitive)) {
            return cpuAndNode[1];
        }
    }

    return QString();
}

bool UpmCustomHardwareConfig::firstRunDonotSuspend()
{
    if (nullptr != m_extendConfig) {
        QVariant value;
        if (true == readConfig(m_extendConfig,
                               UPM_GROUP_EXTEND_BRAND_GENERIC_CONFIG,
                               UPM_KEY_FIRST_RUN_DONOT_SUSPEND, value)) {
            if (1 == value.toInt()) {
                return true;
            }
        }
    }
    return false;
}

bool UpmCustomHardwareConfig::getExtendConfig(const QString &configKey, QString &config)
{
    QVariant value;
    if (true == readConfig(m_extendConfig, m_productGroup, configKey, value)) {
        config = value.toString();
        return true;
    }
    return false;
}

void UpmCustomHardwareConfig::getHardwareInfo()
{
    if (false == readAll(UPM_HARDWARE_INFO_FILE, m_hardwareInfo)) {
        qDebug() << "get hardware info failed";
    }
}

void UpmCustomHardwareConfig::getCpuType()
{
    if (false == readAll(UPM_CPU_INFO_FILE, m_cpuType)) {
        qDebug() << "get cpu info failed";
    }
}

bool UpmCustomHardwareConfig::readConfig(QSettings *settings,
        const QString &group, const QString &key, QVariant &value)
{
    if (nullptr == settings) {
        return false;
    }
    settings->beginGroup(group);
    value = settings->value(key, UPM_NO_THIS_KEY);
    settings->endGroup();
    qDebug() << "READ" << key << group << value;
    if (UPM_NO_THIS_KEY == value.toString()) {
        return false;
    }
    return true;
}

bool UpmCustomHardwareConfig::getExtendConfigFile(QString &configFilePath)
{
    QDir extendConfigDir(UPM_EXTEND_HARDWARE_CONFIG_PATH);

    if (true == extendConfigDir.exists()) {
        QStringList extendConfigs = extendConfigDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        qDebug() << "dirs :" << extendConfigDir;
        Q_FOREACH (const QString &extendConfig, extendConfigs) {
            qDebug() << "extend config:" << extendConfig;
            QStringList fileName = extendConfig.split(".");
            qDebug() << "fileName:" << fileName;
            if (m_hardwareInfo.contains(fileName[0], Qt::CaseInsensitive)) {
                configFilePath = QString(UPM_EXTEND_HARDWARE_CONFIG_PATH) + extendConfig;
                qDebug() << "configFilePath:" << configFilePath;
                return true;
            }
        }
    }
    qDebug() << "get extend config file failed";
    return false;
}

bool UpmCustomHardwareConfig::readAll(const QString &filePath, QString &content)
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

QMap<QString, QString> UpmCustomHardwareConfig::getConfig(const QString &group)
{
    QMap<QString, QString> iniMap;
    QString m_arch;

    QSettings *settings = new QSettings(UPM_GLOBAL_HARDWARE_CONFIG_FILE, QSettings::IniFormat, this);
    QSettings *archSettings = new QSettings(UPM_EXTEND_HARDWARE_ARCH_CONFIG_PATH, QSettings::IniFormat, this);

    settings->beginGroup(group);
    // 获取该section下的所有键
    QStringList keys = settings->allKeys();
    // 将所有键值对存入QMap
    foreach (const QString &key, keys) {
        iniMap.insert(key, settings->value(key).toString());
    }
    settings->endGroup();

    //步骤1、CPU架构配置替换全局配置
    if (true == UpmCustomHardwareConfig::self()->isHygonCpu()) {
        m_arch = "Hygon";
    } else if (true == UpmCustomHardwareConfig::self()->isZhaoxinCpu()) {
        m_arch = "Zhaoxin";
    } else if (true == UpmCustomHardwareConfig::self()->isMipsCpu()) {
        m_arch = "Mips";
    } else if (true == UpmCustomHardwareConfig::self()->isD3000M()) {
        m_arch = "D3000M";
    } else {
        m_arch = "None";
    }

    archSettings->beginGroup("Arch");
    int archEnable = 0;
    QStringList archKeys = archSettings->childKeys();
    foreach (const QString &key, archKeys) {
        if (key == m_arch) {
            archEnable = archSettings->value(key, 0).toInt();
            break;
        }
    }
    archSettings->endGroup();

    if (archEnable == 1) {
        archSettings->beginGroup(m_arch);
        QStringList subArchKeys = archSettings->childKeys();

        foreach (const QString &key, subArchKeys) {
            if (iniMap.contains(key)) {
                qDebug() << "cpu cover key:" << key;
                iniMap[key] = archSettings->value(key).toString();
            }
        }
        archSettings->endGroup();
    }

    //步骤2、整机机型配置覆盖全局配置
    settings->beginGroup(UPM_GROUP_THIS_PRODUCT);
    QString m_productGroup = settings->value(UPM_KEY_DEVICE_IDENTIFICATION).toString();
    QString m_extendConfigPath = settings->value(UPM_KEY_EXTEND_CONFIG_PATH).toString();
    QSettings *m_extendConfig = nullptr;
    if (false == m_extendConfigPath.isEmpty() &&  false == m_productGroup.isEmpty()) {
        m_extendConfig = new QSettings(m_extendConfigPath, QSettings::IniFormat, this);

        //如果定制配置存在且可读
        if (m_extendConfig->status() == QSettings::NoError) {
            m_extendConfig->beginGroup(m_productGroup);
            QStringList extendKeys = m_extendConfig->allKeys();
            foreach (const QString &key, extendKeys) {
                if (iniMap.contains(key)) {
                    // 如果全局配置中存在相同的键，则替换值
                    qDebug() << "cover key:" << key << "value:" << m_extendConfig->value(key).toString();
                    iniMap[key] = m_extendConfig->value(key).toString();
                }
            }
            settings->endGroup();
        }
    }
    settings->endGroup();

    return iniMap;
}

QString UpmCustomHardwareConfig::getScenekeyValue(int policy, QString scene, QString group, QMap<QString, QString> iniMap)
{
        //[usbRPM]
        //defaultPerformanceUsbRPM=on
        //defaultBalanceUsbRPM=default
        //defaultPowersaveUsbRPM=auto
        //配置分三段   default: 场景  Performance：模式  UsbRP：ini 的group名字

        QString keyGroup = group;
        // 处理硬件名称：首字母大写，其余保持不变

        if (!keyGroup.isEmpty()) {
                keyGroup[0] = keyGroup[0].toUpper();
        }

        QString policyStr;
        switch(policy) {
        case 0:
            policyStr = "Performance";
            break;
        case 1:
            policyStr = "Balance";
            break;
        case 2:
            policyStr = "Powersave";
            break;
        }
        qDebug() << "keyGroup:"<< keyGroup << "policy: " << policyStr;

        // 构建配置键名
        QString key = QString("%1%2%3").arg(scene).arg(policyStr).arg(keyGroup);
        qDebug() << "get config key: " << key << " value:" << iniMap.value(key);

        // 从iniMap中获取对应的值，如果键不存在则返回未配置
        return iniMap.value(key, "default");
}

