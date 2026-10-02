/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVENT_DATA_H_
#define EVENT_DATA_H_

#include <QObject>
#include <QString>
#include <QDebug>
#include <vector>

#define SCROLL_UP 1
#define SCROLL_DOWN -1
#define DEFAULT_SLOT 9

#define CONTROL_START_APP               0x0001
#define CONTROL_SET_FOCUS               0x0002
#define CONTROL_CLOSE_APP               0x0003
#define CONTROL_MOUSE_EVENT             0x0004
#define CONTROL_WAKEUP_EVENT            0x0005
#define CONTROL_KEYBOARD_EVENT          0x0006
#define CONTROL_EMUGL_UPDATE_WINDOW     0x0007
#define CONTROL_EMUGL_DELETE_WINDOW     0x0008
#define CONTROL_EMUGL_PREPARE_SHM_DATA  0x0009
#define CONTROL_EMUGL_SET_SUPPORT_DYNAMIC_SIZE 0x000a

enum {
    Button1_Press = 1,
    Button1_Release,
    Button2_Press,
    Button2_Release,
    Button3_Press,
    Button3_Release,
    Button4_Up,
    Button5_Down,
    Mouse_Motion,
    Focus_In,
};

typedef struct
{
    int type;
    int slot;
    int x;
    int y;
} MouseEventInfo;

class EventData {
 public:
    //  typedef std::vector<EventData> EventList;
    EventData() {};
    EventData(std::uint16_t type, std::uint16_t code, std::int32_t value, std::uint64_t sec, std::uint64_t usec) { m_type = type; m_code = code; m_value = value; m_sec = sec; m_usec = usec; };
    ~EventData() {};

    std::uint16_t m_type;
    std::uint16_t m_code;
    std::int32_t m_value;

    std::uint64_t m_sec;
    std::uint64_t m_usec;
};

class EventRecordData {
 public:
    EventRecordData() {};
    EventRecordData(int type, int x, int y, int t_slot, std::uint64_t sec, std::uint64_t usec, bool status) { m_type = type; m_x = x; m_y = y; m_slot = t_slot; m_sec = sec; m_usec = usec; m_status = status; };
    ~EventRecordData() {};

    void setType(int type) { m_type = type; };
    void setX(int x) { m_x = x; };
    void setY(int y) { m_y = y; };
    void setSlot( int slot ) { m_slot = slot; };
    void setStatus( int status ) { m_status = status; };
    void setSec( std::uint64_t sec ) { m_sec = sec; };
    void setUSec( std::uint64_t usec ) { m_usec = usec; };

    int getType() const { return m_type; };
    int getX() const { return m_x; };
    int getY() const { return m_y; };
    int getSlot() const { return m_slot; };
    int getStatus() const { return m_status; };
    std::uint64_t getSec() const { return m_sec; };
    std::uint64_t getUSec() const { return m_usec; };

//protected:
    int m_type;
    int m_x;
    int m_y;
    int m_slot;
    int m_status;//0: inValid   1: Press    2: Release   3: Up    4: Down     5: Motion

    std::uint64_t m_sec;
    std::uint64_t m_usec;
};

Q_DECLARE_METATYPE(EventData)
Q_DECLARE_METATYPE(EventRecordData)

#endif // EVENT_DATA_H_
