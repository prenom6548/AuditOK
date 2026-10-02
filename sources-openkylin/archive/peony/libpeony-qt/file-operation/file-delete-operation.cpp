/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "file-delete-operation.h"
#include "file-operation-manager.h"
#include "file-node.h"
#include "file-node-reporter.h"
#include "sound-effect.h"
#ifdef KY_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif
#include <QApplication>
#include <QStandardPaths>
#include <QProcess>
#include <QString>
#include <sys/types.h>
#include <sys/stat.h>
using namespace Peony;
#ifdef KY_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

FileDeleteOperation::FileDeleteOperation(QStringList sourceUris, QObject *parent) : FileOperation(parent)
{
    QStringList srcUris;
    for(auto &uri : sourceUris){
        if(uri.startsWith("label://"))
        {
            QUrl url(uri);
            QString scheme = uri.section("?schema=",-1,-1);
            QString path = url.path().section("?schema=", 0, 0).section("/",2,-1);
            uri = QString(scheme).append(":///").append(path);
        }
        srcUris.append(uri);
    }
    m_src_uris = srcUris;
    m_reporter = new FileNodeReporter;
    m_info = std::make_shared<FileOperationInfo>(sourceUris, nullptr, FileOperationInfo::Delete);
    connect(m_reporter, &FileNodeReporter::nodeFound, this, &FileOperation::operationPreparedOne);
}

FileDeleteOperation::~FileDeleteOperation()
{
    delete m_reporter;
}

std::shared_ptr<FileOperationInfo> FileDeleteOperation::getOperationInfo()
{
    return m_info;
}

void FileDeleteOperation::deleteRecursively(FileNode *node)
{
    ++m_node_depth;
    if (isCancelled()) {
        --m_node_depth;
        return;
    }
    OperatorThreadPause();
    auto fileIconName = FileUtilsPrivate::getFileIconName(FileUtils::urlEncode(node->uri()));
    GFile *file = g_file_new_for_uri(node->uri().toUtf8().constData());
    if (node->isFolder()) {
        for (auto child : *(node->children())) {
            deleteRecursively(child);
        }
        GError *err = nullptr;
        g_file_delete(file,
                      getCancellable().get()->get(),
                      &err);
        if (err) {
            // 若是权限问题，尝试调整权限后重试一次
            //fix bug 430821 选中存在管道文件、套接字文件的目录拷贝到桌面，跳过所有异常文件保存后，使用快捷键shift+delete无法删除该文件夹
            if (err->code == G_IO_ERROR_PERMISSION_DENIED && node->parent() && m_node_depth > 2) {
                g_error_free(err);
                err = nullptr;
                g_autoptr(GFile) fileParent = g_file_new_for_uri(FileUtils::urlEncode(node->parent()->uri()).toUtf8().constData());
                g_autoptr(GFileInfo) info = g_file_query_info(fileParent,
                                                              G_FILE_ATTRIBUTE_UNIX_MODE,
                                                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                              nullptr,
                                                              &err);
                if (info && g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE)) {
                    guint32 mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);
                    mode |= S_IWUSR;
                    g_file_set_attribute_uint32(fileParent,
                                                G_FILE_ATTRIBUTE_UNIX_MODE,
                                                mode,
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                nullptr,
                                                nullptr);
                    qDebug() << "Adjusted parent directory permissions for" << node->parent()->uri();
                } else {
                    qDebug() << "Could not query or adjust parent directory permissions for" << node->parent()->uri();
                }
                if (err) {
                    g_error_free(err);
                    err = nullptr;
                }
                // 重试删除
                g_file_delete(file, getCancellable().get()->get(), &err);
            }

            if (err) {
                if (!m_prehandle_hash.isEmpty()) {
                    g_error_free(err);
                    --m_node_depth;
                    return;
                }
                FileOperationError except;
                except.errorType = ET_GIO;
                except.dlgType = ED_WARNING;
                except.srcUri = node->uri();
                except.op = FileOpDelete;
                except.title = tr("File delete error");
                except.errorStr = err->message;
                except.errorCode = err->code;
                Q_EMIT errored(except);
                auto response = except.respCode;
                auto responseType = response;
                if (responseType == Cancel) {
                    cancel();
                }
                m_prehandle_hash.insert(err->code, IgnoreAll);
            }
        }
    } else {
        GError *err = nullptr;
        g_file_delete(file,getCancellable().get()->get(),&err);
        if (err) {
            // 若是权限问题，尝试调整权限后重试一次
            if (err->code == G_IO_ERROR_PERMISSION_DENIED && node->parent() && m_node_depth > 2) {
                g_error_free(err);
                err = nullptr;
                g_autoptr(GFile) fileParent = g_file_new_for_uri(FileUtils::urlEncode(node->parent()->uri()).toUtf8().constData());
                g_autoptr(GFileInfo) info = g_file_query_info(fileParent,
                                                              G_FILE_ATTRIBUTE_UNIX_MODE,
                                                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                              nullptr,
                                                              &err);
                if (info && g_file_info_has_attribute(info, G_FILE_ATTRIBUTE_UNIX_MODE)) {
                    guint32 mode = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE);
                    mode |= S_IWUSR;
                    g_file_set_attribute_uint32(fileParent,
                                                G_FILE_ATTRIBUTE_UNIX_MODE,
                                                mode,
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                nullptr,
                                                nullptr);
                    qDebug() << "Adjusted parent directory permissions for" << node->parent()->uri();
                } else {
                    qDebug() << "Could not query or adjust parent directory permissions for" << node->parent()->uri();
                }
                if (err) {
                    g_error_free(err);
                    err = nullptr;
                }
                // 重试删除
                g_file_delete(file, getCancellable().get()->get(), &err);
            }

            if (err) {
                if (!m_prehandle_hash.isEmpty()) {
                    g_error_free(err);
                    --m_node_depth;
                    return;
                }
                FileOperationError except;
                except.errorType = ET_GIO;
                except.dlgType = ED_WARNING;
                except.srcUri = node->uri();
                except.op = FileOpDelete;
                except.title = tr("File delete error");
                except.errorCode = err->code;
                except.errorStr = err->message;
                Q_EMIT errored(except);
                auto response = except.respCode;
                auto responseType = response;
                if (responseType == Cancel) {
                    cancel();
                }
                // Similar errors only remind the user once
                m_prehandle_hash.insert(err->code, IgnoreAll);
            }
        }
    }
    g_object_unref(file);
    //qDebug()<<"deleted";
    //operationAfterProgressedOne(node->uri());
    m_current_offset += node->size();

    FileProgressCallback(node->uri(), node->uri(), fileIconName, m_current_offset, m_total_szie);

    --m_node_depth;
}

void FileDeleteOperation::run()
{
    if (isCancelled())
        return;

    Q_EMIT operationStarted();
    for (auto src : m_src_uris) {
        // pre-check for delete special directory
        if (src == "file:///data/home" || src == "file:///data/usershare" ||
                src == "file:///data/root" || src == "file:///home" ||
                FileUtils::isStandardPath(src) || src == "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation)) {
            FileOperationError except;
            except.srcUri = src;
            except.destDirUri = nullptr;
            except.isCritical = true;
            except.op = FileOpTrash;
            except.title = tr("Delete file error");
            except.errorType = ET_GIO;
            except.dlgType = ED_WARNING;
            except.errorStr = tr("Invalid Operation! Can not delete \"%1\".").arg(FileUtils::urlDecode(src));
            errored(except);
            cancel();
        }

        if (isCancelled()) {
            Q_EMIT operationFinished();
            return;
        }
    }

    Q_EMIT operationRequestShowWizard();

    goffset *total_size = new goffset(0);
    bool isMobileDevice = FileUtils::isMobileDeviceFile(m_src_uris.first());

    QList<FileNode*> nodes;
    for (auto uri : m_src_uris) {
        FileNode *node = new FileNode(uri, nullptr, m_reporter);
        node->findChildrenRecursively();
        node->computeTotalSize(total_size);
        nodes<<node;
    }
    operationPrepared();

    m_total_szie = *total_size;
    m_current_offset = 0;
    delete total_size;

    //jump to the clearing stage.
    //operationProgressed();

    for (auto node : nodes) {
        deleteRecursively(node);
    }

    for (auto node : nodes) {
        delete node;
    }

    //fix delete file not sync issue,link to bug#113826
    if (isMobileDevice) {
        QString parentUri = FileUtils::getParentUri(FileUtils::urlEncode(m_src_uris.first()));
        auto file = wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(parentUri).toUtf8().constData()));
        char *path = g_file_get_path(file.get()->get());
        if (path) {
            operationStartSnyc();
            QProcess p;
            // p.start(QString("/usr/bin/sync -f %1").arg(path));
            p.start("/usr/bin/sync", {"-f", QString::fromUtf8(path)});
            p.waitForFinished(-1);
            if (p.exitCode() == 0) {
                qDebug() << "sync completed successfully";
            } else {
                qDebug() << "sync failed with exit code:" << p.exitCode();
            }
        }
    }

#ifdef KY_UDF_BURN
    std::shared_ptr<FileOperationHelper> mHelper = std::make_shared<FileOperationHelper>(m_src_uris.first());
    if (mHelper->isUnixCDDevice()) {
        mHelper->judgeSpecialDiscOperation();
        mHelper->discDeleteOperation(m_src_uris);
    }
#endif

    Q_EMIT operationFinished();

    qApp->property("clearTrash");
    if(true == qApp->property("clearTrash").toBool()){
        //Peony::SoundEffect::getInstance()->recycleBinClearMusic();
        //Task#152997, use sdk play sound
#ifdef KY_SDK_SOUND_EFFECTS
        kdk::KSoundEffects::playSound(SoundType::TRASH_EMPTY);
#endif
    }
}

void FileDeleteOperation::cancel()
{
    if (m_reporter)
        m_reporter->cancel();
    FileOperation::cancel();
}
