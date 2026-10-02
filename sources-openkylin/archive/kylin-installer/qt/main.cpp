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
#include "kreinstall.h"

#include <QApplication>
#include <QDesktopWidget>
#include "xatom-helper.h"
//#include "qtsingleapplication.h"
#include "adaptscreeninfo.h"
#include <qlogging.h>
#include <stdio.h>
#include <stdlib.h>

#include <QLibraryInfo>
#include <QTranslator>
//#include <ukui-log4qt.h>
//#include <log.hpp>
#include <QSettings>
//#include <window_management.hpp>
//#include <single_application.hpp>
//#include <system_information.hpp>
// #include <kysdk/kysdk-system/libkysysinfo.h>
#include <qlogging.h>
#include <QDir>
#include <singleapplication.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <signal.h>

#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QJsonArray>
#include <iostream>
#include "widbox.h"
#include <QCryptographicHash>
#include <libkylog.h>

#include <libkylog.h>
#include "../src/common/utils/SystemInfoUtil.h"

#define MAX_PATH_LENGTH 512

static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    switch (type) {
    case QtDebugMsg:
        klog_debug("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtInfoMsg:
        klog_info("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtWarningMsg:
        klog_warning("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtCriticalMsg:
        klog_err("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    case QtFatalMsg:
        klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, function);
        break;
    }

    return;
}

QString getDebInfo(QString m_debPath, QString type)
{
    QProcess terminal;
    QStringList cmdList;
    m_debPath = QDir::toNativeSeparators(m_debPath);
    qWarning() << "m_debPath" <<m_debPath;


    QApt::DebFile debfile(m_debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }

    QRegExp rx("[^a-zA-Z0-9]");
    int pos = rx.indexIn(m_debPath);
    if (pos > -1)
        m_debPath.toHtmlEscaped();

    QFileInfo file(m_debPath);

    cmdList << "-c" << QString("dpkg -I " + m_debPath);
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();

    QString debinfoerro = terminal.readAllStandardError().data();
    if ((debInfo.contains(type) || debInfo.contains(type.toLower())) && debinfoerro=="" ){
        QString ret;
        QStringList list = debInfo.split("\n");
        foreach (QString str, list) {
            if(type == "Package"){
                if (str.left(9) == " Package:"){
                    ret = str.remove(" Package:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Version"){
                if (str.left(9) == " Version:"){
                    ret = str.remove(" Version:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Architecture"){
                if (str.left(14) == " Architecture:"){
                    ret = str.remove(" Architecture:").remove("\n").remove(" ");
                    break;
                }
            }
        }
        if ( debInfo!="" && debInfo.contains(type)){
            qInfo() << ret << "=======================";
            return ret;
        }
    }
    else if (debInfo!="" && debinfoerro.contains("压缩数据已损坏") || debinfoerro.contains("གནོན་སྐྱུང་གཞི་གྲངས་ལ་སྐྱོན་ཤོར་བ།"))
    {
        //deb包已损坏，无法解析
        return "The software package is damaged";//软件包已损坏
    }
    else if (file.exists() && debInfo=="")
    {
        //路径包含特殊字符，需要debfile解析
        //QApt::DebFile::isValid();
        QApt::DebFile debfile(m_debPath);
        if (debfile.isValid()){
            if (type == "Package"){
                return debfile.packageName();}
            else if (type == "Version"){
                return debfile.version();
            }
            else{
                return debfile.architecture();
            }
        }
        else{
            return "The software package is damaged";
        }
    }

}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    // 打印输入参数
    qDebug() << "输入参数数量:" << argc;
    for (int i = 0; i < argc; ++i) {
        qDebug() << "arg[" << i << "]:" << argv[i];
    }

    if (0 == getuid()) {
        printf("权限错误，请以普通用户权限运行程序！\n");
        exit(0);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    qDebug()<<"Kmre::libPath="<<SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::INSTALL_LIB_PATH,"libkmre.so");
    qDebug()<<"Kmre::confPath="<<SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::KMRE_CONF_PATH,"kmre.conf");
    QString id = QString("kylin-installer-" + QLatin1String(getenv("DISPLAY")));
    kdk::QtSingleApplication a(id,argc, argv);

    QFileInfo file("/usr/share/qt5/translations/qt_zh_CN.qm");
    QTranslator * translator = new QTranslator();
    if(file.exists()!=false){
        translator->load("/usr/share/qt5/translations/qt_zh_CN.qm");
        a.installTranslator(translator);
    }
    a.setStyle("ukui");
    if (translator->load(QLocale(), QLatin1String("kre-install"), QLatin1String("_"), QLatin1String("/usr/share/kre/kre-install/translations")))
    {
        a.installTranslator(translator);
    }
    else
        qDebug() << "cannot load translator kre-install_" << QLocale::system().name() << ".qm!";
    QTranslator trans1;
    QString local1 = QLocale::system().name();
    if(trans1.load(QString(":translations/gui_" + local1 +".qm")))
    {
        a.installTranslator(&trans1);
    }

    a.setApplicationName(QObject::tr("kylin-installer"));
    a.setStyle("ukui");

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            const char *path = argv[i];
            if (!path) continue;

            size_t len = strlen(path);
            if (len == 0) {
                qDebug() << "参数" << i << "为空字符串，已跳过";
                continue;
            }

            if (len >= MAX_PATH_LENGTH) {
                qDebug() << path << "文件路径过长，不符合规范！";
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("","文件路径过长不符合规范!","");
                msgBox->exec();

                return 0;
            }
        }
    }

    QStringList importFiles = QCoreApplication::arguments();
    importFiles.removeFirst();

    if (a.isRunning()) {
        if (importFiles.size() > 0) {
            QJsonObject obj;
            obj["importFiles"] = QJsonArray::fromStringList(importFiles);
            QString jsonStr = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

            a.sendMessage(jsonStr);
        }

        qDebug() << QObject::tr("kylin-installer is already running!");
        return EXIT_SUCCESS;
    } else {
        setenv("QT_QPA_PLATFORMTHEME", "ukui", true);

        if (argc == 2) {
            std::string strTmp = argv[1];
            QString str = QString::fromStdString(strTmp);
            QFileInfo fileinfo(str);
            QString lang = getenv("LANG");
            if (!fileinfo.isFile()) {
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",str+QObject::tr(" is not file"),"");
                qWarning() << str << " is not file";
                msgBox->exec();
                exit(0);
            }
            /*判断文件名是否有特殊字符*/
            QString debPathname = str;
            if (debPathname.contains("`",Qt::CaseSensitive) || debPathname.contains("$(",Qt::CaseSensitive) || debPathname.contains(";",Qt::CaseSensitive)
                    || debPathname.contains("&",Qt::CaseSensitive) || debPathname.contains("|",Qt::CaseSensitive) ){
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",QObject::tr("Open file error\n"
                                                                                       "Naming contains nonstandard characters '` $( & | '")
                                                                                     ,"");
                msgBox->exec();
                exit(0);
            }


            QMimeDatabase mimeDatabase;
            QMimeType mimeType = mimeDatabase.mimeTypeForFile(str);

            if (mimeType.name() == "application/vnd.debian.binary-package") {
                qInfo() << "This is a deb package file.";
                QString debPath = fileinfo.absoluteFilePath();
                kreInstall w(debPath);
                kdk::UkuiStyleHelper::self()->removeHeader(&w);
                w.show();
                a.setActivationWindow(&w);
                QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
                return a.exec();
            } else if (mimeType.name() == "application/vnd.android.package-archive") {
                QString debPath = fileinfo.absoluteFilePath();
                kreInstall w (debPath);
                kdk::UkuiStyleHelper::self()->removeHeader(&w);
                a.setActivationWindow(&w);
                w.show();
                QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
                return a.exec();
            } else if (fileinfo.suffix() == "ok" || fileinfo.suffix() == "oab") {
                //开明包逻辑
                QString debPath = fileinfo.absoluteFilePath();
                kreInstall w (debPath);
                kdk::UkuiStyleHelper::self()->removeHeader(&w);
                a.setActivationWindow(&w);
                w.show();
                QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
                return a.exec();
            } else {
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",QObject::tr("This file is not a valid installable package."),"");
                msgBox->exec();
                exit(0);
            }

        } else {
            kreInstall w ("Batch installation");
            kdk::UkuiStyleHelper::self()->removeHeader(&w);
            w.show();
            a.setActivationWindow(&w);
            QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
            QObject::connect(&w, &kreInstall::uiReady, [&](){
                w.batchInstallListInit(importFiles);
            });
            return a.exec();
        }
    }
}
