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


#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QTranslator>
#include <QLibraryInfo>
#include <QIcon>
#include "photo_printer.h"

int main(int argc, char **argv)
{
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
    //高清屏幕自适应,适配990高DPI
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    #endif

    QApplication a(argc, argv);
    QString locale = QLocale::system().name();
    QTranslator trans_global, trans_menu;
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);// /usr/share/qt5/translations
    QString kylinPhotoPrinterTranslationsPath = "/usr/share/peony-print-pictures/translations";

    if(!trans_global.load("peony-print-pictures_" + locale + ".qm", kylinPhotoPrinterTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        a.installTranslator(&trans_global);

    if(!trans_menu.load("qt_" + locale + ".qm", qtTranslationsPath))
        qDebug() << "Load translations file" <<QLocale() << "failed!";
    else
        a.installTranslator(&trans_menu);
    // QLabel *c = new QLabel(nullptr);
    // c->setText(a.arguments().join(" "));
    // c->show();
    a.setWindowIcon(QIcon::fromTheme("kylin-photo-viewer"));
    QStringList ls = a.arguments();
    ls.removeAt(0);
    if (ls.size() <= 0) {
        return 0;
    }
    PhotoPrinter p(ls);
    a.exec();
}