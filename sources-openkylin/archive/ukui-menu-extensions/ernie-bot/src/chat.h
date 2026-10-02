/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#ifndef UKUI_MENU_CHAT_H
#define UKUI_MENU_CHAT_H
#include <QObject>
#include <QDateTime>

class ChatPrivate;
class QNetworkAccessManager;
class Chat : public QObject
{
    Q_OBJECT
public:
    explicit Chat(QObject *parent = nullptr);
    Q_DISABLE_COPY(Chat)
    /**
     * 开启一个新会话
     * @param clientId
     * @param clientSecret
     * @return
     */
    void newChat(const QString &clientId = {}, const QString &clientSecret = {});

    /**
     * 清除对话历史
     * @return
     */
    void clear();

    /**
     * @brief 检查访问是否有效
     */
    bool isValid() const;

public Q_SLOTS:
    /**
     * @brief 发送用户消息
     * @param content 用户输入的对话内容
     */
    void talk(const QString& content);

Q_SIGNALS:
    void readyToChat();
    void response(const QString& content);
    void expired();
    void error(const QString &errorMsg);

#ifdef BUILD_TEST
public:
    void setNetworkAccessManagerForTest(QNetworkAccessManager *manager);
    void setTokenStateForTest(const QString &accessToken, const QDateTime &tokenGetTime, int expiresIn);
    int cacheSizeForTest() const;
    bool waitingForTokenForTest() const;
    bool waitingForResponseForTest() const;
#endif

private:
    ChatPrivate *d = nullptr;
};


#endif //UKUI_MENU_CHAT_H
