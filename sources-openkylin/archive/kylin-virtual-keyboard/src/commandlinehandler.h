/*
 * Copyright (c) KylinSoft Co., Ltd. 2022.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMMANDLINEHANDLER_H
#define COMMANDLINEHANDLER_H

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>
#include "qtsingleapplication/src/QtSingleApplication"

class MessageHandler;

class CommandLineHandler : public QCommandLineParser {
public:
    CommandLineHandler();

    // 检查是否应该继续执行主程序（处理单例检查和启动命令）
    bool shouldContinueExecution(QtSingleApplication &app) const;

    // 绑定消息接收处理
    void bindMessageHandler(QtSingleApplication &app,
                            MessageHandler &messageHandler);

private:
    QCommandLineOption m_loglevelOption;
};

#endif // COMMANDLINEHANDLER_H
