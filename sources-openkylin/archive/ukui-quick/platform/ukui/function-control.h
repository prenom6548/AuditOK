/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef FUNCTION_CONTROL_H
#define FUNCTION_CONTROL_H

#include <QObject>
namespace UkuiQuick {
class FunctionControlPrivate;

/**
 * FunctionControl是一个系统功能管控接口封装类,用于管理一些功能的开关
 * */
class FunctionControl : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取FunctionControl实例
     * @return FunctionControl* FunctionControl实例
     * @note 此类线程不安全,不要在多线程环境中使用
     */
    static FunctionControl* instance();
    ~FunctionControl() override;

    /**
     * @brief 添加管控模块@param name 配置监听,当模块配置发生变化时,会发送functionControlChanged信号
     * 使用此方法添加的监听，会在FunctionControl内部增加一个监听计数，当监听计数为0时，会自动移除监听。
     * 使用此方法添加的监听，可以调用removeWatch方法来移除监听。
     * @param name 功能名称
     */
    void addWatch(const QString& name);
    /**
     * @brief 将针对@param name 的配置监听引用计数减一
     * 当监听计数为0时，会自动移除监听。
     * @param name 功能名称
     */
    void removeWatch(const QString& name);
    /**
     * @brief 直接移除所有针对功能@param name 的配置监听
     * @param name 功能名称
     */
    void removeAllWatches(const QString& name);
    /**
     * @brief 获取功能@param name 的配置
     * @param name 功能名称
     * @return QJsonObject 功能配置
     */
    QJsonObject functionControlObject(const QString& name);

Q_SIGNALS:
    /**
     * @brief 功能配置发生变化时,会发送此信号
     * @param name 功能名称
     */
    void functionControlChanged(const QString& name);
    /**
     * @brief 当管控功能退出时,会发送此信号
     * */
    void functionControlExit();

private:
    explicit FunctionControl(QObject* parent = nullptr);
    FunctionControlPrivate *d = nullptr;
};
}


#endif //FUNCTION_CONTROL_H
