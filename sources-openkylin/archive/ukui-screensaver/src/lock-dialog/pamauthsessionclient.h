/*
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef PAMAUTHSESSIONCLIENT_H
#define PAMAUTHSESSIONCLIENT_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <functional>

class QSocketNotifier;

class PamAuthSessionClient : public QObject
{
public:
    explicit PamAuthSessionClient(QObject *parent = nullptr);
    ~PamAuthSessionClient() override;

    bool start(int fd);
    void stop();
    bool inAuthentication() const;
    bool isActive() const;
    bool sendPacket(int cmdId, const QString &content = QString());

public:
    std::function<void(const QString &, int)> onShowMessage;
    std::function<void(const QString &, int)> onShowPrompt;
    std::function<void(const QVariant &, int)> onShowBinaryPrompt;
    std::function<void()> onAuthCompleted;

private:
    void onPipeRead();

private:
    int m_fd = -1;
    QSocketNotifier *m_notifier = nullptr;
    bool m_inAuthentication = true;
};

#endif // PAMAUTHSESSIONCLIENT_H
