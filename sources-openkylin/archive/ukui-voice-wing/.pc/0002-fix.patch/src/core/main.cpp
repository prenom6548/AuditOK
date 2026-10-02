/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QString>
#include <QTranslator>

#include "logger.h"
#include "voiceInput.h"

namespace {

bool acquireProcessLock()
{
    if (qEnvironmentVariableIsSet("UKUI_VOICE_WING_SKIP_LOCK")) {
        return true;
    }

    const QString lockPath =
        QStringLiteral("/tmp/ukui-voice-wing-%1.lock").arg(getuid());
    const int fd = open(lockPath.toUtf8().constData(),
                        O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        VW_LOG(Logger::Level::Error) << "Failed to create lock file:" << lockPath;
        return false;
    }

    if (lockf(fd, F_TLOCK, 0) == -1) {
        VW_LOG(Logger::Level::Warning) << "Another instance is already running";
        close(fd);
        return false;
    }

    return true;
}

bool installAppTranslator(QGuiApplication &app)
{
    auto *translator = new QTranslator(&app);

    const QString locale = QLocale::system().name();
    const QString baseName = QStringLiteral("ukui-voice-wing_") + locale;

    const QStringList searchPaths = {
        QStringLiteral(UKUI_VOICE_WING_BUILD_TRANSLATIONS_DIR),
        QStringLiteral(UKUI_VOICE_WING_TRANSLATIONS_DIR),
        QLibraryInfo::location(QLibraryInfo::TranslationsPath),
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/translations"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../share/ukui-voice-wing/translations"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/translations"),
    };

    for (const QString &path : searchPaths) {
        if (path.isEmpty()) {
            continue;
        }
        if (translator->load(baseName, path)) {
            app.installTranslator(translator);
            return true;
        }
    }

    delete translator;
    return false;
}

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    installAppTranslator(app);
    Logger::instance().init();
    VW_LOG(Logger::Level::Info) << "ukui-voice-wing started";

    if (!acquireProcessLock()) {
        VW_LOG(Logger::Level::Info) << "ukui-voice-wing: another instance is already running, exiting";
        return 0;
    }

    VoiceInput::instance();

    return app.exec();
}
