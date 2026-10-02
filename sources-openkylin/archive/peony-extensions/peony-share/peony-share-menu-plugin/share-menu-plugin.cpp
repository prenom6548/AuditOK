/*
 * Peony-Qt's Library
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "share-menu-plugin.h"
#include "file-info.h"
#include "usershare-manager.h"
#include "properties-window.h"
#include "peony-share/app/share-emblem-provider.h"
#include "global-settings.h"

#include <QTranslator>
#include <QApplication>
#include <QDir>
#include <QProcess>
#include <QUrl>

using namespace Peony;

const QString SHARECONFIGPATH = "/var/lib/samba/usershares/";

ShareMenuPlugin::ShareMenuPlugin(QObject *parent)
    : QObject(parent)
{
    QTranslator *t = new QTranslator(this);
    qDebug()<<"system().name:"<<QLocale::system().name();
    qDebug()<<"\n\n\n\n\n\n\ntranslate:"<<t->load(":/translations/peony-share-menu-plugin_"+QLocale::system().name());
    QApplication::installTranslator(t);
}

QList<QAction *> ShareMenuPlugin::menuActions(Peony::MenuPluginInterface::Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction *> l;
    if (types == MenuPluginInterface::DesktopWindow || types == MenuPluginInterface::DirectoryView) {
        if (selectionUris.count() == 1) {
            qDebug() << __func__ << uri << selectionUris;

            if (!isSupportUri(selectionUris[0]))
                return l;

            QStringList disExtensions = GlobalSettings::getInstance()->getValue(DISABLED_EXTENSIONS).toStringList();
            if (disExtensions.contains("libpeony-share.so"))
                return l;

            auto selectFileInfo = FileInfo::fromUri(selectionUris[0]);
            if (selectFileInfo->isVirtual()) {
                return l;
            }

            if (selectFileInfo->isDir()) {
                auto manager = UserShareInfoManager::getInstance();
                QStringList userShareLists = manager->getUsershareLists();
                bool isShared = false;
                for (auto &list : userShareLists) {
                    if (QString::compare(list, selectFileInfo->displayName(), Qt::CaseInsensitive) == 0) {
                        isShared = true;
                        break;
                    }
                }

                QFileInfo shareInfo(SHARECONFIGPATH + selectFileInfo->displayName().toLower());
                QFileInfo fileInfo(selectFileInfo->filePath());
                if (shareInfo.owner() != fileInfo.owner()) {
                    isShared = false;
                }

                if (isShared) {
                    isShared = checkOriginPath(selectFileInfo->filePath(), shareInfo.filePath());
                }

                QAction* shareOperationAction = nullptr;
                if (isShared) {
                    shareOperationAction = new QAction(tr("Cancel sharing"), nullptr);
                } else {
                    shareOperationAction = new QAction(tr("Share folder"), nullptr);
                }
                shareOperationAction->setIcon(QIcon::fromTheme("send-to-symbolic"));
                connect(shareOperationAction, &QAction::triggered, [=](){
                    if (isShared) {
                        auto fileInfo = FileXattrInfo::fromUri(selectFileInfo->uri());
                        if(fileInfo && !fileInfo->getXattrInfoString(SHARE_EMBLEMS).isEmpty()){
                            fileInfo->removeXattrInfo(SHARE_EMBLEMS);
                            EmblemProviderManager::getInstance()->queryAsync(selectFileInfo->uri());
                        }
                        QString name = selectFileInfo->displayName();
                        UserShareInfoManager::getInstance()->removeShareInfoAcl(name);
                        bool ret;
                        QStringList args;
                        args << "/usr/bin/setfacl" << "-b" << QString("\"%1\"").arg(selectFileInfo->filePath());
                        UserShareInfoManager::exectueSetAclCommand(args, &ret);
                    } else {
                        if (GlobalSettings::getInstance()->isExist(SHOW_SHARE_PROPERTIES)) {
                            GlobalSettings::getInstance()->setValue(SHOW_SHARE_PROPERTIES, true);
                        }

                        QUrl url = selectFileInfo->uri();
                        QProcess p;
                    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                        p.setProgram("/usr/bin/peony");
                        p.setArguments(QStringList()<<"--show-properties"<<url.toEncoded());
                        p.startDetached();
                    #else
                        p.startDetached("/usr/bin/peony", QStringList()<<"--show-properties"<<url.toEncoded());
                    #endif
                    }
                });

                l << shareOperationAction;
                qDebug() << __func__ << __LINE__ << userShareLists << selectFileInfo->displayName();
            }
        }
    }
    return l;
}

bool ShareMenuPlugin::checkOriginPath(const QString &selectFilePath, const QString &configFilePath)
{
    bool ret = true;
    QFile file(configFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "open share config failed:" << configFilePath;
        ret = false;
        return ret;
    }

    QTextStream stream(&file);
    QString result = stream.readAll();
    QStringList lines = result.split('\n');
    QString originPath;

    for (QString line : lines) {
        if (line.startsWith("path")) {
            originPath = line.split('=').last();
            break;
        }
    }

    if (0 != originPath.compare(selectFilePath)) {
        ret = false;
    }
    return ret;
}

bool ShareMenuPlugin::isSupportUri(const QString &uri)
{
    if (uri.startsWith("file:///box"))
    {
        return false;
    }
    auto info = FileInfo::fromUri(uri);
    if (!info->isDir() || info->isVirtual() || !info->uri().startsWith("file:///")) {
        return false;
    }
    // don't share user's home directory file:///home/xxx/
    QStringList file = info->uri().split('/', QString::SkipEmptyParts);
    if ((3 == file.size ()) && (file.at(1) == "home")) {
        return false;
    }

    // don't share directory that has no permission
    if (!info->canRead() || !info->canWrite() || !info->canExecute()) {
        return false;
    }

    g_autoptr(GFile) gFile = g_file_new_for_uri(info->uri().toUtf8().constData());
    if(gFile) {
        g_autoptr(GFileInfo) gInfo = g_file_query_info(gFile,
                                            "owner::*",
                                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                            nullptr,
                                            nullptr);

        if (gInfo) {
            QString ownerUser = g_file_info_get_attribute_string(gInfo, G_FILE_ATTRIBUTE_OWNER_USER);
            QString userName = qgetenv("USER");
            if (0 != QString::compare(ownerUser, userName)) {
                return false;
            }
        }
    }

    if (info->isSymbolLink()) {
        return false;
    }

    return true;
}
