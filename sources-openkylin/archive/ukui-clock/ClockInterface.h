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
#ifndef CLOCKINTERFACE_H
#define CLOCKINTERFACE_H
#include <string>
#include <QString>

#define CLOCK_DBUS_SERVICE_NAME "org.kylin.dbus.ukuiclock"
#define CLOCK_DBUS_SERVICE_PATH "/org/kylin/dbus/ukuiclock"
#define CLOCK_DBUS_UPDATE_CLOCK_BY_ID_PATH "/org/kylin/dbus/ukuiclock/updateClockById"
#define CLOCK_DBUS_SELECT_CLOCK_BY_ID_PATH "/org/kylin/dbus/ukuiclock/selectClockById"
#define CLOCK_DBUS_DELETE_CLOCK_BY_ID_PATH "/org/kylin/dbus/ukuiclock/deleteClockById"
#define CLOCK_DBUS_NOTICE_SHOW_AGAIN_PATH "/org/kylin/dbus/ukuiclock/noticeShowAgain"
namespace clockInterface {

struct ReturnMsg{
    unsigned status = 0;
    QString msg;
};
struct RequestParams{
    QString clockName;
    unsigned hour = 0;
    unsigned minute = 0;
};
enum STATUS_CODE{
    SUCCESS = 0,
    FAIL = 1,
};
}
#endif // CLOCKINTERFACE_H
