/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 *
 */

#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H
#include <QCommandLineParser>
#include <QObject>
#include <QRemoteObjectNode>

class RemoteConfigReplica;

class MessageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit MessageProcessor(QObject *parent = nullptr);
    ~MessageProcessor();
    bool preprocessMessage(const QStringList& message);
private:
    static QUrl nodeUrl();
    QRemoteObjectNode m_qroNode;
    RemoteConfigReplica *m_configReplica = nullptr;
    QCommandLineParser m_parser;
};



#endif //MESSAGE_PROCESSOR_H
