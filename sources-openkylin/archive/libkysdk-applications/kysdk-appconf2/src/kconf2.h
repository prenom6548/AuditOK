/*
 * libkysdk-appconf2's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KCONF2_H
#define KCONF2_H

#include <QObject>
#include <QVariant>

namespace kdk {

class KConf2Private;

class KConf2 : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief 创建KConf2对象
     * @param schema_id 从应用名开始到目标组的路径，每个组名以"."分割
     * @param version 指定配置版本号
     * @param parent 若指定父对象，则自动释放对象内存
     */
    KConf2(const QByteArray &schema_id, const QString version = QString() ,QObject*parent = nullptr);

    ~KConf2();

    /**
     * @brief 获取配置中指定键所对应的值
     * @param key 键名
     * @return 返回值类型为QVariant，需自行转换具体值类型
     */
    QVariant get(const QString &key) const;

    /**
     * @brief 设置配置中指定键所对应的值
     * @param key 键名
     * @param value 对应值
     */
    void set(const QString &key, const QVariant &value);

    /**
     * @brief 设置配置中指定键所对应的值
     * @param key 键名
     * @param value 对应值
     * @return 设置成功返回true, 设置失败返回false
     */
    bool trySet(const QString &key, const QVariant &value);

    /**
     * @brief 列举配置中的所有键
     * @return 键列表
     */
    QStringList keys() const;

    /**
     * @brief 将指定键的值重置为默认值
     * @param key 键名
     */
    void reset(const QString &key);

    /**
     * @brief 判断该配置是否存在
     * @param schema_id 配置名
     * @return
     */
    static bool isSettingsAvailable(const QString &schema_id);

    /**
     * @brief 递归列举指定应用名下的所有配置项
     * @param app 应用名
     * @param version 配置版本
     * @return
     */
    QStringList schemas(const QString &app, const QString &version) const;

    /**
     * @brief 获取指定键的详细描述
     * @param key 键名
     * @return
     */
    QString getDescrition(QString &key);

    /**
     * @brief 获取指定键的简要描述
     * @param key 键名
     * @return
     */
    QString getSummary(QString &key);

    /**
     * @brief 获取值类型
     * @param key 键名
     * @return
     */
    QString getType(QString &key);

    /**
     * @brief 检查键值是否可写
     * @param key 键名
     * @return
     */
    bool isWritable(QString &key);

    /**
     * @brief 获取句柄的配置id
     * @return
     */
    QString getId();

    /**
     * @brief 获取配置版本
     * @return
     */
    QString getVersion();

    /**
     * @brief 列举配置组的子组
     * @return
     */
    QStringList children() const;

    /**
     * @brief 获取指定的子组
     * @param name 子组名称
     * @return 子对象
     */
    KConf2* getChild(QString &name);

Q_SIGNALS:
    void changed(const QString &key);

private:
    Q_DECLARE_PRIVATE(KConf2)
    KConf2Private* const d_ptr;
};

}
#endif // KCONF2_H
