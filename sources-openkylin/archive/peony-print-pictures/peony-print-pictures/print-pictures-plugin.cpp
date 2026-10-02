/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */

#include "print-pictures-plugin.h"
#include <file-info.h>
#include <gio/gio.h>

#include <QAction>
#include <QFileInfo>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QProcess>
#include <QUrl>
#include <QDebug>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QImage>
#include <QLibraryInfo>

#define BACKGROUND_SETTINGS "org.mate.background"
#define PICTRUE "picture-filename"

using namespace Peony;

PrintPicturesPlugin::PrintPicturesPlugin(QObject *parent) : QObject (parent)
{
    static QTranslator trans_global, trans_menu;
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath); // /usr/share/qt5/translations
    QString locale = QLocale::system().name();
    if (!trans_global.load("qt_" + locale + ".qm", qtTranslationsPath))
        qDebug() << "aaa Load translations file" << QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_global);

    QString kylinPrinterTranslationsPath = "/usr/share/peony-print-pictures/translations";
    if (!trans_menu.load("peony-print-pictures_" + locale + ".qm", kylinPrinterTranslationsPath))
        qDebug() << "aaa Load translations file" << QLocale() << "failed!";
    else
        QApplication::installTranslator(&trans_menu);
}

QList<QAction*> PrintPicturesPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
    Q_UNUSED(uri);

    log.info(QString("peony uri %1").arg(uri));
    log.info(QString("peony selectionUris %1").arg(selectionUris.join(" ")));

    QList<QAction*> actions;
    QStringList target;
    for (auto str : selectionUris) {
        target << Peony::FileUtils::urlDecode(str);
    }
    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow)
    {
        if (checkAllUris(target) == true) {
            QAction *set_action = new QAction(tr("Print"), nullptr);
            actions<<set_action;
            m_selectionUris = target;
            connect(set_action, &QAction::triggered, this, [=](){
                doPrintAction();
            });
        }
    }
    return actions;
}

void PrintPicturesPlugin::setSelectionUris(const QStringList &uris)
{
    QStringList target;
    for (auto str : uris) {
        qDebug() << Q_FUNC_INFO << str << "   =   " << Peony::FileUtils::urlDecode(str);
        target << Peony::FileUtils::urlDecode(str);
    }

    if (checkAllUris(target) == true) {
        m_selectionUris = uris;
    }
}

void PrintPicturesPlugin::doPrintAction()
{
    if (m_selectionUris.isEmpty()) {
        return ;
    }
    QStringList selectedPicPath;
    for (auto uri : m_selectionUris) {
        selectedPicPath.append(uriPathConvert(uri));
    }
    log.info(QString("kylin-photo-printer %1").arg(selectedPicPath.join(" ")));
    QProcess p;
    p.setProgram("kylin-photo-printer");
    p.setArguments(QStringList()<<selectedPicPath);
    p.startDetached(p.program(), p.arguments());
    p.waitForFinished(-1);
}

bool PrintPicturesPlugin::fileSuffixCheck(const QString &file_name)
{
    QFileInfo file(file_name);

    auto fileSuffix = file.suffix();
    qDebug() << "file_name:" <<fileSuffix;

    for (auto type : m_picture_type_list) {
        if (fileSuffix.compare(type, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}
bool PrintPicturesPlugin::is_picture_file(QString file_name)
{
#ifdef MIMETYPE_CHECK
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(file_name);
    qDebug() << type.name();
    if (type.name().startsWith("image/")) {
        return true;
    }
#endif
    if (fileSuffixCheck(file_name))
        return true;

    return false;
}


bool PrintPicturesPlugin::checkUriPrintable(const QString &uri)
{
    if (uri.isEmpty()) {
        return false;
    }

    // 不可读
    QFileInfo info(uriPathConvert(uri));
    if (info.isReadable() == false) {
        qDebug() << info.absoluteFilePath() << "不可读取!";
        return false;
    }

    // 回收站里
    if (uri.contains("trash:///")) {
        return false;
    }

    // 识别到是图
    if (is_picture_file(info.absoluteFilePath()) == true) {
        return true;
    }

    return false;
}

bool PrintPicturesPlugin::checkAllUris(const QStringList &uris)
{
    if (uris.isEmpty()) {
        return false;
    }
    // 使用严格判断机制，只要有文件无法打印就返回false.
    for (auto uri : uris) {
        qDebug() << "PrintPicturesPlugin::checkAllUris" << uri;
        if (checkUriPrintable(uri) == false) {
            return false;
        }
    }
    return true;
}


QString PrintPicturesPlugin::uriPathConvert(const QString &uri)
{
    if (uri.contains("file://")) {
        QString str(uri);
        str.remove("file://");
        return str;
    }
    return uri;
}
