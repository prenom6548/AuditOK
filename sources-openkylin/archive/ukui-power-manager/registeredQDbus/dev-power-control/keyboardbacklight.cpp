/*
 * Copyright 2023 KylinSoft Co., Ltd.
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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <vector>

#include "keyboardbacklight.h"
//namespace fs = std::filesystem;

// 常见的键盘背光设备名关键字
const std::vector<std::string> keywords = {
    "kbd_backlight", "keyboard"
};

KeyboardBacklightDevice::KeyboardBacklightDevice(QObject *parent) : QObject(parent)
{
    m_powerConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_KEYBOARDBACKLIGHT);
//    for (const auto& pair : m_powerConfigMap) {
//        qDebug() << pair.first << ":" << pair.second;
//    }
    printConfigMap();
    m_controlFile = KEYBOARD_BACKLIGHT_FILE;
    getDevicePowerInfo();
}

void KeyboardBacklightDevice::getDevicePowerInfo()
{
    QDir deviceDir(KEYBOARD_BACKLIGHT_PATH);
    if(!deviceDir.exists()) {
        qDebug() << "KeyboardBacklightDevice::getKeyboardBacklightDevicePowerInfo(): " << KEYBOARD_BACKLIGHT_PATH << "not exists .";
        return;
    }

    //自动扫描 /sys/class/leds/ 目录寻找背光设备
    m_controlPath.clear(); // 确保路径为空
    // std::string base_path = KEYBOARD_BACKLIGHT_PATH;
    // for (const auto& entry : fs::directory_iterator(base_path)) {
    //     std::string name = entry.path().filename().string();
    //     // 检查路径名中是否包含关键字
    //     for (const auto& kw : keywords) {
    //         if (name.find(kw) != std::string::npos) {
    //             // 确认 brightness 和 max_brightness 文件存在且可写
    //             std::string bright = entry.path().string() + "/brightness";
    //             std::string max_bright = entry.path().string() + "/max_brightness";
    //             if (fs::exists(bright) && fs::exists(max_bright)) {
    //                 //return entry.path().string();
    //                 m_controlPath = QString::fromStdString(entry.path().string()) + "/";
    //                 break;
    //             }
    //         }
    //     }
    // DIR* dir = opendir(KEYBOARD_BACKLIGHT_PATH);
    // if (!dir) {
    //     qDebug() << "Failed to open directory:" << KEYBOARD_BACKLIGHT_PATH;
    //     return;
    // }

    // struct dirent* entry;
    // while ((entry = readdir(dir)) != nullptr) {
    //     // 跳过 "." 和 ".."
    //     if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
    //         continue;
    //     }

    //     std::string name = entry->d_name;
    //     // 检查路径名中是否包含关键字
    //     for (const auto& kw : keywords) {
    //         if (name.find(kw) != std::string::npos) {
    //             // 构造完整路径
    //             std::string fullPath = std::string(KEYBOARD_BACKLIGHT_PATH) + name + "/";
    //             std::string bright = fullPath + "brightness";
    //             std::string max_bright = fullPath + "max_brightness";

    //             // 使用 access 检查文件是否存在且可读（R_OK）
    //             if (access(bright.c_str(), R_OK) == 0 && access(max_bright.c_str(), R_OK) == 0) {
    //                 // 找到目标设备
    //                 m_controlPath = QString::fromStdString(fullPath);
    //                 break;
    //             }
    //         }
    //     }
    //     if (!m_controlPath.isEmpty()) {
    //         break;
    //     }
    // 获取 /sys/class/leds/ 下的所有子目录（不包括文件）
    QFileInfoList subDirs = deviceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    // 遍历每一个子目录
    for (const QFileInfo &dirInfo : subDirs) {
        QString dirName = dirInfo.fileName(); // 获取子目录的名称

        // 检查目录名中是否包含关键字
        bool keywordMatch = false;
        for (const auto& kw : keywords) {
            if (dirName.contains(QString::fromStdString(kw), Qt::CaseInsensitive)) {
                keywordMatch = true;
                break;
            }
        }

        if (!keywordMatch) {
            continue; // 如果不匹配关键字，跳过此目录
        }

        // 构造 brightness 和 max_brightness 文件的完整路径
        QString brightPath = dirInfo.absoluteFilePath() + "/" + KEYBOARD_BACKLIGHT_FILE;
        QString maxBrightPath = dirInfo.absoluteFilePath() + "/max_brightness";

        // 检查两个文件是否同时存在
        if (QFile::exists(brightPath) && QFile::exists(maxBrightPath)) {
            // 找到了符合条件的背光设备目录
            m_controlPath = dirInfo.absoluteFilePath() + "/"; // 末尾加上斜杠方便后续拼接
            break; // 跳出循环
        }
    }
    if (m_controlPath.isEmpty()) {
        qDebug() << "No suitable keyboard backlight device found.";
        return;
    }

    const QString tempDevicePath = m_controlPath + QString::fromStdString(KEYBOARD_BACKLIGHT_FILE);
    QFileInfo checkFile(tempDevicePath);
    if(checkFile.exists() && checkFile.isFile()) {
        qDebug() << "KeyboardBacklightDevice::getKeyboardBacklightDevicePowerInfo(): " << "pcie_aspm device control file is exists: " << tempDevicePath;
        m_devicePowerInfo.push_back(new DevicePowerInfo("keyboard_backlight", m_controlPath, "keyboard_backlight", m_controlFile, this));
    }
}

bool KeyboardBacklightDevice::setPowerStat(int deviceIndex, const QString &stat)
{

    // if (stat == "default" || stat == "performance" || stat == "powersave" || stat == "powersupersave") {
    //     if (deviceIndex < m_devicePowerInfo.count()) {
    //         return m_devicePowerInfo[deviceIndex]->setDevicePowerStat(stat);
    //     }
    // }
    // return false;
}

bool KeyboardBacklightDevice::setAllDevicePowerStat(const QString &stat)
{
    qDebug() << "KeyboardBacklightDevice::setAllDevicePowerStat: " << stat;
    if (stat == "default" || stat == "off") {
        for (int i = 0; i < m_devicePowerInfo.count(); ++i) {
            m_devicePowerInfo[i]->setDevicePowerStat(stat);
        }
        return true;
    }
    return false;
}
