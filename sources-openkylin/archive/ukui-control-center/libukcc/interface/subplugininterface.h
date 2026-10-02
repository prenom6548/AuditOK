/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2026 KylinSoft  Co., Ltd.
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
#ifndef SUBPLUGININTERFACE_H
#define SUBPLUGININTERFACE_H

#include "libukcc_global.h"

#include <QObject>
#include <QtPlugin>
#include <QString>

class SubPluginInterfacePrivate;

/**
 * @brief 子插件基类
 *
 * 单继承 QObject，新增成员与逻辑放入 SubPluginInterfacePrivate，避免破坏已发布子插件.so 的二进制布局。
 *
 * 子插件直接继承 SubPluginInterface，不要再额外继承 QObject。
 */
class LIBUKCC_EXPORT SubPluginInterface : public QObject
{
    Q_OBJECT

public:
    explicit SubPluginInterface(QObject *parent = nullptr);
    ~SubPluginInterface() override;

    /** @brief 子插件名称 */
    virtual const QString name() const = 0;

    /** @brief 子插件是否可用 */
    virtual bool isEnable() const = 0;

    /** @brief 子插件主界面UI */
    virtual QWidget *pluginUi() = 0;

    /**
     * @brief 多语言 .ts 路径模板，预留搜索词条（%1 为 locale）
     *
     * 默认返回搜索词条路径。比如子插件名称：kylin-gesture-plugin，父插件名称：accessibility，对应翻译路径是：/usr/share/accessibility-plugin/i18n/kylin-gesture-plugin/%1.ts
     *
     * 另外代码中添加搜索词条时必须加上父插件路径，如代码中：
     *
     * //~ contents_path /Accessibility/Air Gesture
     * tr("Air Gesture");

     */
    virtual QString translationPath() const { return QStringLiteral("/usr/share/<parent>-plugin/i18n/<sub-plugin-id>/"); }

protected:
    SubPluginInterface(SubPluginInterfacePrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(SubPluginInterface)
    SubPluginInterfacePrivate *const d_ptr;
};


#define SubPluginInterface_iid "org.ukcc.SubPluginInterface"

Q_DECLARE_INTERFACE(SubPluginInterface, SubPluginInterface_iid)

#endif // SUBPLUGININTERFACE_H
