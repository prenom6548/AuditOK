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

#include "commandlinehandler.h"
#include <iostream>

#include "log.h"
#include "messagehandler.h"

CommandLineHandler::CommandLineHandler()
    : m_loglevelOption(QStringList() << "loglevel",
                       "Set the log level \n"
                       "(available: debug, info, warn, error).\n"
                       "e.g. --loglevel=debug",
                       "loglevel") {
    addHelpOption();
    addVersionOption();
    addOption(m_loglevelOption);
}

bool CommandLineHandler::shouldContinueExecution(
    QtSingleApplication &app) const {
    // 单实例判断
    if (!app.isRunning()) {
        return true;
    }

    if (isSet(m_loglevelOption)) {
        QString level = value(m_loglevelOption);
        QString command = QString("loglevel %1").arg(level);
        app.sendMessage(command);
    }

    std::cout << "kylin-virtual-keyboard is already running!" << std::endl;
    return false;
}

void CommandLineHandler::bindMessageHandler(QtSingleApplication &app,
                                            MessageHandler &messageHandler) {
    QObject::connect(
        &app, &QtSingleApplication::messageReceived,
        [&](const QString &message) {
            messageHandler.processMessage(message, [&](const QString &result) {
                KVKBD_INFO("Command result: {}", result.toStdString());
            });
        });
}
