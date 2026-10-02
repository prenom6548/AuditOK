/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "shreddialog.h"
#include <QApplication>
#include <fcntl.h>
#include <syslog.h>
#include <QLocale>
#include <QStandardPaths>
#include <singleapplication.h>
#include "kom-utils.h"

int main(int argc, char *argv[]) {
    qInstallMessageHandler(kom::KomUtils::LogOutput);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    kdk::QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        app.sendMessage("top window");
        return 0;
    }

    /* 加载翻译文件 */
    QString kylinFileCrush = "/usr/share/kylin-os-manager/treasure-chest/kylin-file-crush/translations";
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator trans_global, trans_qt;
    if (!trans_qt.load(QLocale(), "qt", "_", qtTransPath)) {
        qWarning() << "main kylin-file-crush:Load qt translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_qt);
    }
    if (!trans_global.load(QLocale(), "kylin-file-crush", "_", kylinFileCrush)) {
        qWarning() << "main kylin-file-crush:Load global translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global);
    }

    //加载SDK的翻译文件
    QString locale = QLocale::system().name();
    QTranslator sdk_trans;
    if (!sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        qDebug() << "Load sdk translation file failed!";
    } else {
        app.installTranslator(&sdk_trans);
    }

    kom::KomUtils::SetWaylandActivationWindow(app);

    ShredDialog w;
    app.setActivationWindow(&w);
    w.show();

    return app.exec();
}
