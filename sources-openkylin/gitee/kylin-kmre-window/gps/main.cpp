/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QtSingleApplication>
#include <sys/file.h>
#include <sys/syslog.h>
#include <QDBusInterface>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStandardPaths>
#include "gpswindow.h"

#define LOG_IDENT "KMRE_kylin-kmre-gps"

void installTranslator()
{
    // 加载自定义翻译文件
    QString tranPath("/usr/share/kylin-kmre-gps/translations/");
    QTranslator *tran = new QTranslator;
    if (tran->load(QLocale(), QString("kylin-kmre-gps"), QString("_"), tranPath)) {
        QApplication::installTranslator(tran);
    } else {
        qWarning() << "Waring : load translation file fail";
    }

    // 加载qt翻译文件， 更改fileDialog英文
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QTranslator *tranQt = new QTranslator;
    if (tranQt->load(QLocale(), QString("qt"), QString("_"), qtTransPath)) {
        QApplication::installTranslator(tranQt);
    } else {
        qWarning() << "Waring : load qt translation file fail";
    }
}

int main(int argc, char *argv[])
{
    QString id = QString("kylin-kmre-gps" + QLatin1String(getenv("DISPLAY")));
    QtSingleApplication a(id, argc, argv);

    if (a.isRunning()) {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("kylin-kmre-gps is already running!");
        return EXIT_SUCCESS;
    }

    int nRet = 0;

    //加载翻译
    installTranslator();

    // 实现VNC单例
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString lockPath = QString(homePath.at(0) + "/.config/kylin-kmre-gps.lock");

    // 给文件锁加一个DISPLAY标识
    int fd = open(lockPath.toStdString().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        exit(1);
    }
    if (lockf(fd, F_TLOCK, 0)) {
        syslog(LOG_ERR, "Can't lock single file, kylin-kmre-gps is already running!");
        exit(0);
    }

    GPSWINDOW w;
    w.show();

    a.setActivationWindow(&w);
    QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(pullUpWindow(const QString&)));
    w.show();

    nRet = a.exec();
    return nRet;
}
