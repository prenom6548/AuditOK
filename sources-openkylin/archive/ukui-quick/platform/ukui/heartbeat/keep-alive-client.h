/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
#ifndef UKUI_QUICK_KEEPALIVECLIENT_H
#define UKUI_QUICK_KEEPALIVECLIENT_H

#include <QObject>
#include <QScopedPointer>

namespace UkuiQuick::HeartBeat
{
class KeepAliveClient : public QObject
{
    Q_OBJECT

public:
    explicit KeepAliveClient(const QString &appName, QObject *parent = nullptr);
    ~KeepAliveClient() override;
    bool start();
    void stop();
    [[nodiscard]] bool isStarted() const;
    [[nodiscard]] bool isRegistered() const;
    void sendStatusResponse() const;
Q_SIGNALS:
    void statusResponseReceived();
    void registrationSucceeded();
private:
    class Private;
    QScopedPointer<Private> d;
};
}


#endif //UKUI_QUICK_KEEPALIVECLIENT_H
