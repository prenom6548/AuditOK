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
#ifndef BASEMODULE_H
#define BASEMODULE_H

#include <QObject>
#include <QJsonObject>

class BaseModule : public QObject
{
    Q_OBJECT
private:
public:
    /**
     * @brief Construct a new Base Module object
     * 
     * @param moduleName  模块名
     * @param parent 父类
     */
    explicit BaseModule(QString moduleName,QObject *parent = nullptr);

    /**
     * @brief Destroy the Base Module object
     * 
     */
    virtual ~BaseModule();
public slots:
    /**
     * @brief 更新配置纯虚函数 
     * 
     * @param conf 配置文件配置
     * @param defaultConf 默认配置
     */
    virtual void updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf) = 0;
signals:

private:
    Q_PROPERTY(QString moduleName READ moduleName WRITE setModuleName NOTIFY moduleNameChanged)
    QString m_moduleName;
public:
    QString moduleName() const
    {
        return m_moduleName;
    }
public slots:
    void setModuleName(QString moduleName)
    {
        if (m_moduleName == moduleName)
            return;

        m_moduleName = moduleName;
        emit moduleNameChanged(m_moduleName);
    }
signals:
    void moduleNameChanged(QString moduleName);
};

#endif // BASEMODULE_H
