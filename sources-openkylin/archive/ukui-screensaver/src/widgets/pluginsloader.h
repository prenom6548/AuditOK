/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef PLUGINSLOADER_H
#define PLUGINSLOADER_H

#include <QThread>
#include <QMap>
#include <QSharedPointer>

class LoginPluginInterface;

/**
 * @brief 插件加载器
 *
 */
class PluginsLoader : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief 实例
     *
     * @return PluginsLoader 插件加载器
     */
    static PluginsLoader &instance();

    /**
     * @brief 获取插件实例列表
     *
     * @return QMap<QString, QSharedPointer<LoginPluginInterface> > 插件实例列表
     */
    inline QMap<QString, QSharedPointer<LoginPluginInterface>> moduleList()
    {
        return m_plugins;
    }
    /**
     * @brief 通过名称查找插件实例
     *
     * @param name 插件名
     * @return LoginPluginInterface 插件实例
     */
    LoginPluginInterface *findModuleByName(const QString &name) const;
    /**
     * @brief 通过类型查找插件
     *
     * @param type 类型
     * @return QMap<QString, LoginPluginInterface *> 插件列表
     */
    QMap<QString, LoginPluginInterface *> findModulesByType(const int type) const;
    /**
     * @brief 移除插件
     *
     * @param moduleName 插件名
     */
    void removeModule(const QString &moduleName);

Q_SIGNALS:
    /**
     * @brief 插件被加载信号
     *
     * @param 插件实例
     */
    void moduleFound(LoginPluginInterface *);

protected:
    /**
     * @brief 加载器运行过程
     *
     */
    void run() override;

private:
    /**
     * @brief 加载器构造
     *
     * @param parent 父对象指针
     */
    explicit PluginsLoader(QObject *parent = nullptr);
    /**
     * @brief 构造器析构
     *
     */
    virtual ~PluginsLoader();

    /**
     * @brief 通过路径查找插件
     *
     * @param path 插件路径
     */
    void findModule(const QString &path);

private:
    QMap<QString, QSharedPointer<LoginPluginInterface>> m_plugins; /** 插件实例列表 */
};

#endif // PLUGINSLOADER_H
