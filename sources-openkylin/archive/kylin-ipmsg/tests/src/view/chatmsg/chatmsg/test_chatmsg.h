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

#ifndef TEST_CHATMSG_H
#define TEST_CHATMSG_H

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QDir>
#include "../../../../../../src/view/chatmsg/chatmsg.h"

Q_DECLARE_METATYPE(ChatMsgInfo*)

class TestChatMsg : public QObject
{
    Q_OBJECT

public:
    TestChatMsg(QObject *parent = nullptr);
    ~TestChatMsg();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testGetchatMsgModel();
    void testUpdateFriendNickname();
    void testSendMsgFailed();
    void testSendMsgSuccess();
    void testRecvMsgSuccess();
    void testIsScrollOnBottom();
    void testSetScrollToBottom();
    void testSignalEmission();
    void testPrivateMethods();
    void testSlotSendText();
    void testEvents();
    void testDragAndDrop();
    void testContextMenu();
    void testScreenModeChanges();
    void testFileOperations();
    void testActionSlots();
    void testMsgListClicked();
    void testPaintEvent();
    void testCoverageAdditions();

private:
    ChatMsg *m_chatMsg;
};

#endif // TEST_CHATMSG_H
