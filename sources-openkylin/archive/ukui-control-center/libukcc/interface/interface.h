/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef INTERFACE_H
#define INTERFACE_H

#include <QPushButton>
#include <QFile>

#include "libukcc_global.h"

class QString;
class QWidget;
class QStringLiteral;

/**
 * @brief 各插件类型
 *
 */
enum FunType {
    ACCOUNT,
    SYSTEM,
    DEVICES,
    NETWORK,
    PERSONALIZED,
    DATETIME,
    UPDATE,
    SECURITY,
    APPLICATION,
    SEARCH_F,
    COMMONINFO,
    TOTALMODULES,
};

/**
 * @brief 插件类，各插件需继承该类
 *
 */
class LIBUKCC_EXPORT CommonInterface{
public:
    /**
     * @brief
     *
     */
    virtual ~CommonInterface(){}
    /**
     * @brief 插件名字
     *
     * @return QString
     */
    virtual QString plugini18nName() = 0;
    /**
     * @brief 插件类型
     *
     * @return int
     */
    virtual int pluginTypes() = 0;
    /**
     * @brief 插件主界面
     *
     * @return QWidget
     */
    virtual QWidget * pluginUi() = 0;
    /**
     * @brief 是否显示在首页
     *
     * @return bool
     */
    virtual bool isShowOnHomePage() const = 0;
    /**
     * @brief 图标
     *
     * @return QIcon
     */
    virtual QIcon icon() const = 0;
    /**
     * @brief 插件是否启用
     *
     * @return bool
     */
    virtual bool isEnable() const = 0;
    /**
     * @brief
     *
     * @return const QString
     */
    virtual const QString name() const = 0;       // 模块名称
    /**
     * @brief 获取多语言文件路径，用于搜索
     *
     * @return QString
     */
    virtual QString translationPath() const {
        return QStringLiteral("/usr/share/ukui-control-center/shell/res/i18n/%1.ts");
    }
    QPushButton *pluginBtn = nullptr; /**< TODO: describe */

    /**
     * @brief 是否莱卡 Intel 教育定制项目
     *
     * @return bool
     */
    bool isIntel(){
        QString sysVersion = "/etc/apt/ota_version";
        QFile file(sysVersion);
        return file.exists();
    }
    /**
     * @brief 离开插件界面时的操作 eg：离开蓝牙界面停止扫描
     *
     */
    virtual void plugin_leave() {
        return ;
    }
};

#define CommonInterface_iid "org.ukcc.CommonInterface"

Q_DECLARE_INTERFACE(CommonInterface, CommonInterface_iid)

#endif // INTERFACE_H
