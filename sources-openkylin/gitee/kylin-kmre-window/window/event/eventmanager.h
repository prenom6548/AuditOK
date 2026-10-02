/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#pragma once

#include <QObject>
#include <QVector>
#include <atomic>
#include "eventdata.h"
#include "mousekey/kmreplatform.h"
#include "keyboard/keycodes_common.h"
#include <boost/asio/deadline_timer.hpp>
#include "singleton.h"

class InputmethodChannel;

namespace kmre {

namespace input {
class Manager;
}

namespace network {
class PublishedSocketConnector;
class MessageSender;
}  // namespace network

namespace rpc {
class ConnectionCreator;
class PendingCallCache;
class Channel;
}  // namespace rpc

class Runtime;
}

class EventManager : public QObject, public kmre::SingletonP<EventManager>
{
    Q_OBJECT

private:
    std::atomic_int mFocusedDisplayId;
    InputmethodChannel *mInputmethodChannel = nullptr;
    QThread* mInputmethodThread = nullptr;

    std::atomic_int global_track_id;
    std::shared_ptr<kmre::Runtime> rt;
    std::shared_ptr<KmrePlatform> mPlatform;
    std::shared_ptr<kmre::input::Manager> mInputManager;
    boost::asio::deadline_timer appmgr_start_timer;

private:
    EventManager();
    ~EventManager();

    void mouseEventHandler(int type, int slot, int x, int y);
    void keyboardEventHandler(int eventType, QVector<int> keyCodes);
    
signals:
    void sendMouseKeyData(QList<EventData> data, bool isTouch, int winId, int eventType);
    void sendInputmethodData(QString text);

public:
    bool isDisplayFocused(int displayId) {
        return (mFocusedDisplayId == displayId);
    }
    void setCurrentFocusedDisplay(const int displayId) {
        mFocusedDisplayId = displayId;
    }

    void sendMouseEvent(const int displayId, MouseEventInfo mouseEvent);
    void sendKeyBoardEvent(const int displayId, KeycodeBufferData* keyCodeBuf);
    void sendInputMethodData(const int displayId, const QString &text);

    friend SingletonP<EventManager>;
};