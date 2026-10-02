// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "magnifier.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator appTranslator;
    QTranslator qtTranslator;

    QString systemLanguage = QLocale::system().name();
    QString translationFile = "translations/" + systemLanguage;

    if (appTranslator.load(translationFile, "/usr/share/kylin-magnifier")) {
        app.installTranslator(&appTranslator);
    }

    if (qtTranslator.load(QString("qt_%1").arg(systemLanguage),
                          QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }

    int type = 1;
    if (argc > 1) {
        type = QString(argv[1]).toInt();
    }

    Magnifier magnifier(type);
    if (!magnifier.checkZoomIsStarted()) {
        return 0;
    }
    magnifier.show();
    return app.exec();
}
