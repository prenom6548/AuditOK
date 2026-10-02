/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "widget.h"
#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    QTranslator *trans_qt = new QTranslator();
    qDebug() << qtTransPath;
    if (trans_qt->load(QLocale(), "qt", "_", qtTransPath))
        qApp->installTranslator(trans_qt);
    Widget w;
    w.show();
    return a.exec();
}
