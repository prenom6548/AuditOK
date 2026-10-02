/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef USERCONFIGSETTINGS_H
#define USERCONFIGSETTINGS_H

#include <QSettings>
#include <QString>

/**
 * @brief UserConfigSettings 自定义 QSettings 类，自动处理用户配置目录和文件的权限
 *
 * 功能：
 * 1. 当公共目录不存在时，创建并赋予用户权限
 * 2. 配置文件在创建或修改后，仅恢复父目录和文件本身的权限
 * 3. 不调整公共目录部分的权限
 */
class UserConfigSettings : public QSettings
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数 - 通过用户名
     * @param fileName 配置文件路径
     * @param format 配置文件格式
     * @param userName 用户名（用于获取 UID/GID）
     * @param parent 父对象
     */
    explicit UserConfigSettings(
        const QString &fileName, Format format, const QString &userName, QObject *parent = nullptr);

    /**
     * @brief 构造函数 - 通过 UID/GID
     * @param fileName 配置文件路径
     * @param format 配置文件格式
     * @param uid 用户 ID
     * @param gid 组 ID
     * @param parent 父对象
     */
    explicit UserConfigSettings(
        const QString &fileName, Format format, uid_t uid, gid_t gid, QObject *parent = nullptr);

    /**
     * @brief 析构函数，确保权限被修正
     */
    virtual ~UserConfigSettings();

    /**
     * @brief 同步配置到磁盘并修正权限
     * @return 是否同步成功
     */
    bool sync();

private:
    /**
     * @brief 初始化权限信息
     */
    void initPermissions();

    /**
     * @brief 确保配置目录存在并设置权限
     * @return 是否成功
     */
    bool ensureConfigDir();

    /**
     * @brief 修正配置文件及其直接父目录的权限
     * @return 是否成功
     */
    bool fixPermissions();

    /**
     * @brief 递归修正目录下所有文件和目录的权限
     * @param dirPath 目录路径
     * @return 是否成功
     */
    bool fixDirPermissionsRecursive(const QString &dirPath);

    /**
     * @brief 识别公共目录边界（通过获取用户家目录，返回家目录的直接子目录）
     * @param filePath 配置文件完整路径
     * @return 公共目录路径（家目录的直接子目录），如果无法识别则返回空字符串
     */
    QString detectCommonDirBoundary(const QString &filePath);

private:
    QString m_fileName;          // 配置文件路径
    uid_t m_uid;                 // 用户 ID
    gid_t m_gid;                 // 组 ID
    QString m_commonDirBoundary; // 公共目录边界
    bool m_hasModifications;     // 是否有修改
};

#endif // USERCONFIGSETTINGS_H
