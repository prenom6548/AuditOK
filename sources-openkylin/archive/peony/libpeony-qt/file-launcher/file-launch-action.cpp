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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-launch-action.h"
#include <gio/gdesktopappinfo.h>

#include "file-info.h"
#include "file-info-job.h"
#include "file-utils.h"
#include "file-operation-utils.h"
#include "audio-play-manager.h"
#include "global-settings.h"
#include "file-lauch-dialog.h"

#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QProcess>
#include <recent-vfs-manager.h>
#include <QApplication>

#ifdef KYLIN_COMMON
#include <ukuisdk/kylin-com4cxx.h>
#endif

#include <QDebug>
#include <QtX11Extras/QX11Info>
#include <kstartupinfo.h>

using namespace Peony;

static void launchAsyncCallback(GAppInfo *source_object,
                                GAsyncResult *res,
                                gpointer user_data)
{
    GError *err = nullptr;
    bool ok = g_app_info_launch_uris_finish (source_object, res, &err);
    if (!ok) {
        g_warning ("failed to launch app: %s", err->message);
        qWarning() << "failed to launch app:" << err->message;
        g_error_free (err);
    }
    KStartupInfoId* startInfoId = static_cast<KStartupInfoId*>(user_data);
    if (!startInfoId)
        return;
    delete startInfoId;
}

static void contextLaunchedCallback(GAppLaunchContext *context,
                                    GAppInfo          *info,
                                    GVariant          *platform_data,
                                    gpointer           user_data)
{
    KStartupInfoId* startInfoId = static_cast<KStartupInfoId*>(user_data);
    if (!startInfoId)
        return;
    gint pid = -1;
    g_variant_lookup (platform_data, "pid", "i", &pid);
    KStartupInfoData data;
    data.addPid(pid);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
    data.setIconGeometry(QRect(0, 0, 1, 1));  // ugly
#endif
    KStartupInfo::sendChange(*startInfoId, data);
    KStartupInfo::resetStartupEnv();
}

static void setAppContextRect(GAppLaunchContext *context, const QRect &rect)
{
    const char *rectX = QString::number(rect.x()).toLocal8Bit().constData();
    const char *rectY = QString::number(rect.y()).toLocal8Bit().constData();
    const char *rectWidth = QString::number(rect.width()).toLocal8Bit().constData();
    const char *rectHeight = QString::number(rect.height()).toLocal8Bit().constData();
    auto object = G_OBJECT (context);
    g_object_set_data(object, "rectX", GINT_TO_POINTER(rect.x()));
    g_object_set_data(object, "rectY", GINT_TO_POINTER(rect.y()));
    g_object_set_data(object, "rectWidth", GINT_TO_POINTER(rect.width()));
    g_object_set_data(object, "rectHeight", GINT_TO_POINTER(rect.height()));
    g_app_launch_context_setenv(context, "LAUNCH_ICON_GEOMETRY_X", rectX);
    g_app_launch_context_setenv(context, "LAUNCH_ICON_GEOMETRY_Y", rectY);
    g_app_launch_context_setenv(context, "LAUNCH_ICON_GEOMETRY_WIDTH", rectWidth);
    g_app_launch_context_setenv(context, "LAUNCH_ICON_GEOMETRY_HEIGHT", rectHeight);
}

static GAppLaunchContext *getAppContext(FileLaunchAction *action = nullptr)
{
    auto context = g_app_launch_context_new();
    auto env_list = g_listenv();
    auto p = env_list;
    while (*p) {
        g_app_launch_context_setenv(context, *p, g_getenv(*p));
        p++;
    }
    g_strfreev(env_list);
    if (action) {
        auto app_info = action->gAppInfo();
        if (!G_IS_DESKTOP_APP_INFO (app_info))
            return context;
        auto desktop_app_info = G_DESKTOP_APP_INFO (app_info);
        auto desktop_filename = g_desktop_app_info_get_filename(desktop_app_info);
        g_app_launch_context_setenv(context, "APPLICATION_ID", desktop_filename);
        //修改 bug#205475 双屏扩展模式下，打开副屏上的.sh ，窗口显示在副屏上，注释 DESKTOP_STARTUP_ID 就可以显示在主屏
        //g_app_launch_context_setenv(context, "DESKTOP_STARTUP_ID", g_app_info_get_name(app_info));
        g_app_launch_context_setenv(context, "TIMESTAMP",  QString::number(QX11Info::getTimestamp()).toUtf8().constData());
    }
    return context;
}

FileLaunchAction::FileLaunchAction(const QString &uri, GAppInfo *app_info, bool forceWithArg, QObject *parent) : QAction(parent)
{
    if(uri.startsWith("recent:///"))
        m_uri = FileUtils::getTargetUri(uri);
    else
        m_uri = uri;

    m_app_info = static_cast<GAppInfo*>(g_object_ref(app_info));
    m_force_with_arg = forceWithArg;

    if (!isValid())
        return;

    GIcon *icon = g_app_info_get_icon(m_app_info);
    auto iconName = FileUtils::getIconStringFromGIcon(icon);
    if (iconName.startsWith("/")) {
        m_icon.addFile(iconName);
    } else {
        m_icon = QIcon::fromTheme(iconName);
    }
    setIcon(m_icon);
    m_info_name = g_app_info_get_name(m_app_info);
    setText(m_info_name);
    m_info_display_name = g_app_info_get_display_name(m_app_info);

    connect(this, &QAction::triggered, [=]() {
        this->lauchFileAsync(m_force_with_arg, true);
    });
}

FileLaunchAction::~FileLaunchAction()
{
    if (m_app_info)
        g_object_unref(m_app_info);
}

const QString FileLaunchAction::getUri()
{
    return m_uri;
}

bool FileLaunchAction::isDesktopFileAction()
{
    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(m_uri);
    if (info->isEmptyInfo()) {
        FileInfoJob j(info);
        j.querySync();
    }
    return info->isDesktopFile();
}

const QString FileLaunchAction::getAppInfoName()
{
    return m_info_name;
}

const QString FileLaunchAction::getAppInfoDisplayName()
{
    return m_info_display_name;
}

bool FileLaunchAction::isExcuteableFile(QString fileType)
{
    if (m_executable_type.contains(fileType))
        return true;

    return false;
}

void FileLaunchAction::lauchFileSync(bool forceWithArg, bool skipDialog)
{
    m_force_with_arg = forceWithArg;

    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(m_uri);
    if (fileInfo->isEmptyInfo()) {
        FileInfoJob j(fileInfo);
        j.querySync();
    }

    bool readable = fileInfo->canRead();
    if (!readable) {
        QMessageBox::critical(0, tr("No Permission"), tr("File is not readable. Please check if file has read permisson."));
        return;
    }

    bool executable = fileInfo->canExecute();
    bool isAppImage = fileInfo->type() == "application/vnd.appimage";
    bool isExecutable = isExcuteableFile(fileInfo->type());
    if (isAppImage) {
        if (executable) {
            QUrl url = m_uri;
            auto path = url.path();

            QProcess p;
            p.setProgram(path);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.startDetached();
#else
            p.startDetached(path);
#endif
            return;
        }
    }

    if (executable && !isDesktopFileAction() && !skipDialog && isExecutable) {
        QMessageBox msg(FileUtils::getMainWindow());
        auto defaultAction = msg.addButton("By Default App", QMessageBox::ButtonRole::ActionRole);
        defaultAction->setDefault(true);
        auto exec = msg.addButton(tr("Execute Directly"), QMessageBox::ButtonRole::ActionRole);
        auto execTerm = msg.addButton(tr("Execute in Terminal"), QMessageBox::ButtonRole::ActionRole);
        msg.addButton(QMessageBox::Cancel);
        msg.setIcon(QMessageBox::Question);
        msg.setText(tr("Detected launching an executable file %1, you want?").arg(fileInfo->displayName()));
        msg.exec();
        auto button = msg.clickedButton();
        if (button == exec) {
            execFile();
            return;
        } else if (button == execTerm) {
            execFileInterm();
            return;
        } else if (button == defaultAction) {
            //skip
        } else {
            return;
        }
    }

    //fix bug#213466, remote file open not same with local issue
    //fix issue https://gitee.com/openkylin/peony/issues/I9QXWX, default open new file issue
    if (fileInfo->type() == "application/octet-stream" || fileInfo->type() == "application/x-zerosize") {
        GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(text_info, l,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
        g_object_unref(text_info);
        return;
    }

    if (!isValid()) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        QMessageBox::critical(nullptr, tr("Open Failed"), tr("Can not open %1, file not exist, is it deleted?").arg(m_uri));
        return;
    }

    if (isDesktopFileAction() && !forceWithArg) {
        g_app_info_launch(m_app_info, nullptr, nullptr, nullptr);
    } else {
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);
        g_app_info_launch_uris(m_app_info,
                               l,
                               nullptr,
                               nullptr);
        g_list_free_full(l, g_free);
    }

    return;
}

void FileLaunchAction::lauchFileAsync(bool forceWithArg, bool skipDialog)
{
    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(m_uri);
    if (fileInfo->isEmptyInfo()) {
        FileInfoJob j(fileInfo);
        j.querySync();
    }

    bool executable = fileInfo->canExecute();
    bool isAppImage = fileInfo->type() == "application/vnd.appimage";
    bool isExecutable = isExcuteableFile(fileInfo->type());
    qDebug() <<"executable:" <<executable <<isAppImage <<fileInfo->type();

    QUrl url = m_uri;
    if (isAppImage) {
        if (executable) {
            QUrl url = m_uri;
            auto path = url.path();

            QProcess p;
            p.setProgram(path);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.startDetached();
#else
            p.startDetached(path);
#endif
            return;
        }
    }

    if (executable && !isDesktopFileAction() && !skipDialog && isExecutable) {
        QMessageBox msg(FileUtils::getMainWindow());
        auto defaultAction = msg.addButton(tr("By Default App"), QMessageBox::ButtonRole::ActionRole);
        defaultAction->setDefault(true);
        msg.setIcon(QMessageBox::Question);
        auto exec = msg.addButton(tr("Execute Directly"), QMessageBox::ButtonRole::ActionRole);
        auto execTerm = msg.addButton(tr("Execute in Terminal"), QMessageBox::ButtonRole::ActionRole);
        msg.addButton(QMessageBox::Cancel);

        msg.setWindowTitle(tr("Launch Options"));
        msg.setText(tr("Detected launching an executable file %1, you want?").arg(fileInfo->displayName()));
        msg.exec();
        auto button = msg.clickedButton();
        if (button == exec) {
            execFile();
            return;
        } else if (button == execTerm) {
            execFileInterm();
            return;
        } else if (button == defaultAction) {
            //skip
        } else {
            return;
        }
    }

    //fix bug#213466, remote file open not same with local issue
    //fix issue https://gitee.com/openkylin/peony/issues/I9QXWX, default open new file issue
    if (fileInfo->type() == "application/octet-stream" || fileInfo->type() == "application/x-zerosize") {
        GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(text_info, l,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
        g_object_unref(text_info);
        return;
    }

    if (!isValid()) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        bool isReadable = fileInfo->canRead();
        if (!isReadable)
        {
            if (fileInfo->isSymbolLink())
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Open Link failed"));
                msgBox.setText(tr("File not exist, do you want to delete the link file?"));
                msgBox.setIcon(QMessageBox::Question);

                QPushButton *deleteButton = msgBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
                QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

                msgBox.setDefaultButton(deleteButton);

                int ret = msgBox.exec();
                if (msgBox.clickedButton() == deleteButton) {
                    qDebug() << "Delete unused symbollink.";
                    QStringList selections;
                    selections.push_back(m_uri);
                    FileOperationUtils::trash(selections, true);
                }
            }
            else
                QMessageBox::critical(nullptr, tr("Open Failed"),
                                  tr("Can not open %1, Please confirm you have the right authority.").arg(url.toDisplayString()));
        }
        else if (fileInfo->isDesktopFile() && GlobalSettings::getInstance()->getProjectName() != V10_SP1_EDU)
        {
            auto result = QMessageBox::question(nullptr, tr("Open App failed"),
                                  tr("The linked app is changed or uninstalled, so it can not work correctly. \n"
                                     "Do you want to delete the link file?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                qDebug() << "Delete unused desktop file";
                QStringList selections;
                selections.push_back(m_uri);
                FileOperationUtils::trash(selections, true);
            }
        } else if (fileInfo->isExeFile()) {
            notifyUnsupportedFileType("exe", "Microsoft Windows");
        } else if (fileInfo->isAndroidFile()) {
            notifyUnsupportedFileType("apk", "Android");
        } else {
            QUrl url = m_uri;
            if(!QFile(url.path()).exists())
            {
                QMessageBox::warning(nullptr,
                                     tr("Error"),
                                     tr("File original path not exist, are you deleted or moved it?"));
                return;
            }
            QFileInfo file(fileInfo->displayName());
            QString filesuffix = file.suffix().isEmpty()? fileInfo->displayName(): "."+file.suffix();
            notifyUnsupportedFileType(filesuffix, "");
        }
        return;
    }

    if (isDesktopFileAction() && !forceWithArg) {
#if GLIB_CHECK_VERSION(2, 60, 0)
        // send startup info to kwindowsystem
        quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
        KStartupInfoId* startInfoId = new KStartupInfoId();
        startInfoId->initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
        startInfoId->setupStartupEnv();
        KStartupInfoData data;
        data.setHostname();
        float scale = qApp->devicePixelRatio();
        QRect rect = fileInfo.get()->property("iconGeometry").toRect();
        rect.moveTo(rect.x()/* * scale*/, rect.y()/* * scale*/);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
        if (rect.isValid()) {
            data.setIconGeometry(rect);
            qDebug() << "KStartupInfoData iconGeometry:" << m_uri <<rect ;
        }
#endif
        data.setLaunchedBy(getpid());

        KStartupInfo::sendStartup(*startInfoId, data);

        auto context = getAppContext(this);
        setAppContextRect(context, rect);
        g_signal_connect(context, "launched", G_CALLBACK(contextLaunchedCallback), startInfoId);
        g_app_info_launch_uris_async(m_app_info, nullptr,
                                     context, nullptr,
                                     GAsyncReadyCallback(launchAsyncCallback), startInfoId);
        g_object_unref(context);
#else
        g_app_info_launch_uris(m_app_info, nullptr, nullptr, nullptr);
#endif
    } else {
        GList *l = nullptr;
        char *uri = g_strdup(m_uri.toUtf8().constData());
        l = g_list_prepend(l, uri);

        QString name = fileInfo.get()->displayName();
        QString group = g_app_info_get_executable(m_app_info);
        if (G_IS_DESKTOP_APP_INFO (m_app_info)) {
            if (g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO (m_app_info))) {
                group = g_desktop_app_info_get_startup_wm_class(G_DESKTOP_APP_INFO (m_app_info));
            }
            name = g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO (m_app_info));
        }
        QString exec = g_app_info_get_commandline(m_app_info);
        RecentVFSManager::getInstance()->insert(fileInfo.get()->uri(), fileInfo.get()->mimeType(), name, exec, group);

#if GLIB_CHECK_VERSION(2, 60, 0)
        // send startup info to kwindowsystem
        quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
        KStartupInfoId* startInfoId = new KStartupInfoId();
        startInfoId->initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
        startInfoId->setupStartupEnv();
        KStartupInfoData data;
        data.setHostname();
        float scale = qApp->devicePixelRatio();
        QRect rect = fileInfo.get()->property("iconGeometry").toRect();
        rect.moveTo(rect.x()/* * scale*/, rect.y()/* * scale*/);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
        if (rect.isValid()) {
            data.setIconGeometry(rect);
            qDebug() << "KStartupInfoData iconGeometry:" << m_uri <<rect ;
        }
#endif
        data.setLaunchedBy(getpid());

        KStartupInfo::sendStartup(*startInfoId, data);

        auto context = getAppContext(this);
        setAppContextRect(context, rect);
        g_signal_connect(context, "launched", G_CALLBACK(contextLaunchedCallback), startInfoId);
        g_app_info_launch_uris_async(m_app_info, l,
                                     context, nullptr,
                                     GAsyncReadyCallback(launchAsyncCallback), startInfoId);

        g_object_unref(context);
#else
        g_app_info_launch_uris(m_app_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
    }
}

void FileLaunchAction::lauchFilesAsync(const QStringList files, bool forceWithArg, bool skipDialog)
{
    m_force_with_arg = forceWithArg;
    if(files.isEmpty())
        return;

    //FIXME: replace BLOCKING api in ui thread.
    auto fileInfo = FileInfo::fromUri(m_uri);
    if (fileInfo->isEmptyInfo()) {
        FileInfoJob j(fileInfo);
        j.querySync();
    }

    bool executable = fileInfo->canExecute();
    bool isAppImage = fileInfo->type() == "application/vnd.appimage";
    bool isExecutable = isExcuteableFile(fileInfo->type());
    if (isAppImage) {
        if (executable) {
            QProcess p;
            for (auto uri:files) {
                auto path = ((QUrl) uri).path();
                QProcess p;
                p.setProgram(path);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                p.startDetached();
#else
                p.startDetached(path);
#endif
            }
            return;
        }
    }

    if (executable && !isDesktopFileAction() && !skipDialog && isExecutable) {
        QMessageBox msg(FileUtils::getMainWindow());
        auto defaultAction = msg.addButton(tr("By Default App"), QMessageBox::ButtonRole::ActionRole);
        defaultAction->setDefault(true);
        msg.setIcon(QMessageBox::Question);
        auto exec = msg.addButton(tr("Execute Directly"), QMessageBox::ButtonRole::ActionRole);
        auto execTerm = msg.addButton(tr("Execute in Terminal"), QMessageBox::ButtonRole::ActionRole);
        msg.addButton(QMessageBox::Cancel);

        msg.setWindowTitle(tr("Launch Options"));
        msg.setText(tr("Detected launching an executable file %1, you want?").arg(fileInfo->displayName()));
        msg.exec();
        auto button = msg.clickedButton();
        if (button == exec) {
            execFile();
            return;
        } else if (button == execTerm) {
            execFileInterm();
            return;
        } else if (button == defaultAction) {
            //skip
        } else {
            return;
        }
    }

    if (!isValid()) {
        Peony::AudioPlayManager::getInstance()->playWarningAudio();
        bool isReadable = fileInfo->canRead();
        if (!isReadable)
        {
            if (fileInfo->isSymbolLink())
            {
                auto result = QMessageBox::question(nullptr, tr("Open Link failed"),
                                      tr("File not exist, do you want to delete the link file?"),
                                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
                    qDebug() << "Delete unused symbollink.";
                    QStringList selections;
                    selections.push_back(m_uri);
                    FileOperationUtils::trash(selections, true);
                }
            }
            else
                QMessageBox::critical(nullptr, tr("Open Failed"),
                                  tr("Can not open %1, Please confirm you have the right authority.").arg(m_uri),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        }
        else if (fileInfo->isDesktopFile() && GlobalSettings::getInstance()->getProjectName() != V10_SP1_EDU)
        {
            auto result = QMessageBox::question(nullptr, tr("Open App failed"),
                                  tr("The linked app is changed or uninstalled, so it can not work correctly. \n"
                                     "Do you want to delete the link file?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                qDebug() << "Delete unused desktop file";
                QStringList selections;
                selections.push_back(m_uri);
                FileOperationUtils::trash(selections, true);
            }
        }
        else {
            auto result = QMessageBox::question(nullptr, tr("Error"), tr("Can not get a default application for opening %1, do you want open it with text format?").arg(m_uri),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                GAppInfo *text_info = g_app_info_get_default_for_type("text/plain", false);
                GList *l = nullptr;
                for (auto uri : files) {
                    l = g_list_prepend(l, g_strdup(uri.toUtf8().constData()));
                }
#if GLIB_CHECK_VERSION(2, 60, 0)
                g_app_info_launch_uris_async(text_info, l,
                                             nullptr, nullptr,
                                             nullptr, nullptr);
#else
                g_app_info_launch_uris(text_info, l, nullptr, nullptr);
#endif
                g_list_free_full(l, g_free);
                g_object_unref(text_info);
            }
        }
        return;
    }

    if (isDesktopFileAction() && !forceWithArg) {
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(m_app_info, nullptr,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(m_app_info, nullptr, nullptr, nullptr);
#endif
    } else {
        GList *l = nullptr;
        for (auto uri : files) {
            l = g_list_prepend(l, g_strdup(uri.toUtf8().constData()));
        }
#if GLIB_CHECK_VERSION(2, 60, 0)
        g_app_info_launch_uris_async(m_app_info, l,
                                     nullptr, nullptr,
                                     nullptr, nullptr);
#else
        g_app_info_launch_uris(m_app_info, l, nullptr, nullptr);
#endif
        g_list_free_full(l, g_free);
    }

    return;
}

bool FileLaunchAction::isValid()
{
    return G_IS_APP_INFO(m_app_info);
}

void FileLaunchAction::execFile()
{
    QUrl url = m_uri;
    char *quote = g_shell_quote(url.path().toUtf8());
    QString newDir = m_uri.section('/',0,m_uri.count('/')-1);
    GAppInfo *app_info = g_app_info_create_from_commandline(quote, nullptr, G_APP_INFO_CREATE_NONE, nullptr);
    QDir::setCurrent(QUrl(newDir).path());
    g_app_info_launch(app_info, nullptr, nullptr, nullptr);
    QDir::setCurrent(QDir::homePath());
    g_object_unref(app_info);
    g_free(quote);
}

void FileLaunchAction::execFileInterm()
{
    QUrl url = m_uri;
    char *quote = g_shell_quote(url.path().toUtf8());
    QString newDir = m_uri.section('/',0,m_uri.count('/')-1);
    GAppInfo *app_info = g_app_info_create_from_commandline(quote, nullptr, G_APP_INFO_CREATE_NEEDS_TERMINAL, nullptr);
    QDir::setCurrent(QUrl(newDir).path());
    g_app_info_launch(app_info, nullptr, nullptr, nullptr);
    QDir::setCurrent(QDir::homePath());
    g_object_unref(app_info);
    g_free(quote);
}

void FileLaunchAction::notifyUnsupportedFileType(const QString &fileExtension, const QString &fileType)
{
    QWidget *widget = QApplication::topLevelAt(QCursor::pos());
    QMessageBox msgBox(widget);
    msgBox.setWindowTitle(tr("Operate Tips"));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowModality(Qt::WindowModal);

    // 根据文件类型设置不同的提示信息
    QString message;
    QStringList parameters;
    if (fileExtension == "exe") {
        message = QString(
            "<html>"
            "<body>"
            "<p style='margin-bottom: 8px;'><b>%1</b></p>"
            "<p style='color: #666; margin-top: 0;'>%2</p>"
            "</body>"
            "</html>"
        ).arg(
            tr("Sorry, the system currently does not have an application that supports .%1 (%2) files.").arg(fileExtension, fileType),
            tr("Please search the software store for programs that can run %1 applications, or select an existing application on your computer to open it.").arg("Windows")
        );

        parameters<<"-find"<<"kylin-wine-assistant";

    } else if (fileExtension == "apk") {
        message = QString(
            "<html>"
            "<body>"
            "<p style='margin-bottom: 8px;'><b>%1</b></p>"
            "<p style='color: #666; margin-top: 0;'>%2</p>"
            "</body>"
            "</html>"
        ).arg(
            tr("Sorry, the system currently does not have an application that supports .%1 (%2) files.").arg(fileExtension, fileType),
            tr("Please search the software store for programs that can run %1 applications, or select an existing application on your computer to open it.").arg(fileType)
        );

        parameters<<"-menuid=3"<<"-tagid=1";

    } else {
        message = QString(
            "<html>"
            "<body>"
            "<p style='margin-bottom: 8px;'><b>%1</b></p>"
            "<p style='color: #666; margin-top: 0;'>%2</p>"
            "</body>"
            "</html>"
        ).arg(
            tr("Sorry, the system currently does not have a default application set for opening %1 file format.").arg(fileExtension),
            tr("Please search the software store for applications that can open this file, or select an existing application in the system.")
        );
    }

    msgBox.setText(message);

    // 添加自定义按钮
    QPushButton *searchStoreBtn = msgBox.addButton(tr("Search Software Store"), QMessageBox::YesRole);
    searchStoreBtn->setDefault(true);
    searchStoreBtn->setProperty("isImportant", true);
    QPushButton *selectAppBtn = msgBox.addButton(tr("Select Application"), QMessageBox::NoRole);
    selectAppBtn->setProperty("useButtonPalette", true);

    // 显示消息框
    msgBox.exec();

    // 处理按钮点击
    if (msgBox.clickedButton() == selectAppBtn) {
        FileLauchDialog d(m_uri);
        d.exec();
    } else if (msgBox.clickedButton() == searchStoreBtn) {
        QProcess p;
        p.setProgram("/usr/bin/kylin-software-center");
        p.setArguments(parameters);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        p.startDetached();
#else
        p.startDetached("/usr/bin/kylin-software-center", parameters);
#endif
    }
}
