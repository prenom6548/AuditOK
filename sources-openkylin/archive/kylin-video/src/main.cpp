/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include "mainwindow.h"
#include "global/translator.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>
#include "global/log.hpp"
int main(int argc, char *argv[])
{

    qInfo()<< QDateTime::currentMSecsSinceEpoch();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    qInfo()<< QDateTime::currentMSecsSinceEpoch();

   // 100ms
    QApplication a(argc, argv);
    qInfo()<< QDateTime::currentMSecsSinceEpoch();

    QStringList arg_list;
    for (int i=1; i<argc; i++)
    {
        if (QString(argv[i]).startsWith("http"))
        {
            arg_list << QString(argv[i]);
            continue;
        }
        QFileInfo f(QString(argv[i]));
        if(f.isFile())
            arg_list << f.absoluteFilePath();
    }
    qInfo()<< QDateTime::currentMSecsSinceEpoch();

    // 第一个操作：加载翻译
    Translator *translator = new Translator();
    translator->load("");
    QTranslator *tran = new QTranslator;
    QString localeName = QLocale::system().name();

    // 根据系统语言加载对应的翻译文件
    if(tran->load(":/translations/gui_" + localeName  + ".qm")){
        a.installTranslator(tran);
    }

    // 200ms
    MainWindow w(arg_list);
    w.show();

    // 使用singleShot延迟执行翻译加载（延迟0毫秒表示尽快执行但不阻塞当前流程）
    QTimer::singleShot(5, [&a, &w]() {
        qInstallMessageHandler(kdk::kabase::Log::logOutput);

        a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

        w.showList();


    });

    QScreen *primaryScreen = QGuiApplication::primaryScreen();

    if(primaryScreen){
        int x = primaryScreen->geometry().x() +
                (primaryScreen->geometry().width() - w.width()) / 2;
        int y = primaryScreen->geometry().y() +
                (primaryScreen->geometry().height() - w.height()) / 2;
        w.move(x,y);
    }
    return a.exec();
}
