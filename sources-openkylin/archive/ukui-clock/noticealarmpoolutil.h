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
#ifndef NOTICEALARMPOOLUTIL_H
#define NOTICEALARMPOOLUTIL_H

#include <QObject>
#include "noticeAlarm.h"

class NoticeAlarmPoolUtil : public QObject
{
    Q_OBJECT
public:
    NoticeAlarmPoolUtil(const NoticeAlarmPoolUtil&)=delete;
    NoticeAlarmPoolUtil& operator=(const NoticeAlarmPoolUtil&)=delete;
    static NoticeAlarmPoolUtil* getInstance(){
        static NoticeAlarmPoolUtil instance;
        return &instance;
    }
    friend class TestClass;  // 核心修改：声明测试类为友元

    void showAgain(uint id);
    void inseartNoticesMap(uint id,Natice_alarm * notice);
    void deleteNoticesMap(uint id);


signals:
private:
    explicit NoticeAlarmPoolUtil(QObject *parent = nullptr);
    QMap<uint,Natice_alarm *> * noticesMap = nullptr;
};

#endif // NOTICEALARMPOOLUTIL_H
