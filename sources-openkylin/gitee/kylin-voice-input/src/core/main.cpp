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

#include <QApplication>
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

bool installAppTranslator(QApplication &app)
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    // TipDialog（QMessageBox）需要 QApplication；QML 语音窗仍可正常使用
    QApplication app(argc, argv);
    // 后台常驻服务：关闭提示窗等顶层窗口时不应退出进程
    app.setQuitOnLastWindowClosed(false);
    Logger::instance().init();
    installAppTranslator(app);
    VW_LOG(Logger::Level::Info) << "ukui-voice-wing started";

    if (!acquireProcessLock()) {
        VW_LOG(Logger::Level::Info) << "ukui-voice-wing: another instance is already running, exiting";
        return 0;
    }

    VoiceInput::instance();

    return app.exec();
}
