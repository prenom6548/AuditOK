/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef GLOBALINSTANCE_H
#define GLOBALINSTANCE_H

#include <QObject>

class GlobalInstance : public QObject
{
    Q_OBJECT
public:
    static GlobalInstance* getInstance();
    explicit GlobalInstance(QObject *parent = nullptr);
    friend class TestClass;  // 核心修改：声明测试类为友元

    static GlobalInstance *instance;
    ~GlobalInstance();

    enum   XDG_SESSION_DESKTOP {
        UNKNOWN,
        KYLIN_WLCOM,
        UKUI_WAYLAND,
        UKUI
    };
    XDG_SESSION_DESKTOP session_desktop(){
        if(m_desktop_type == UNKNOWN){
            if(QString(qgetenv("XDG_SESSION_DESKTOP")) == "kylin-wlcom"){
                m_desktop_type = KYLIN_WLCOM;
            }else if(QString(qgetenv("XDG_SESSION_DESKTOP")) == "ukui-wayland"){
                m_desktop_type = UKUI_WAYLAND;
            }else if(QString(qgetenv("XDG_SESSION_DESKTOP")) == "ukui"){
                m_desktop_type = UKUI;
            }
        }
        return m_desktop_type;
    }
signals:

private:
    XDG_SESSION_DESKTOP m_desktop_type = UNKNOWN;
};




#endif // GLOBALINSTANCE_H
