/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
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

#include "computer-view-container.h"
#include "computer-view.h"
#include "computer-proxy-model.h"
#include "abstract-computer-item.h"
#include "login-remote-filesystem.h"
#include "peony-drive-rename/drive-rename.h"
#include "FMWindowIface.h"
#include "mount-operation.h"
#include "manager-control.h"

#include <peony-qt/file-utils.h>
#include <peony-qt/format_dialog.h>
#include <peony-qt/file-info-job.h>
#include <peony-qt/file-item-model.h>
#include <peony-qt/file-item-proxy-filter-sort-model.h>
#include <peony-qt/format-dlg-create-delegate.h>
#include <peony-qt/plugin-manager.h>
#include <peony-qt/global-settings.h>
#ifndef KY_UDF_BURN
#include <peony-qt/disccontrol.h>
#include <peony-qt/udfFormatDialog.h>
#else
#include <libkyudfburn/disccontrol.h>
#include "volumeManager.h"
#include "udfAppendBurnDataDialog.h"
using namespace UdfBurn;
#endif

#include <QMenu>
#include <QProcess>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStyleOption>
#include <QInputDialog>
#include <QStylePainter>
#include <file-info.h>
#include <QApplication>

static void ask_question_cb(GMountOperation *op, char *message, char **choices, Peony::ComputerViewContainer *p_this);
static void ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags, Peony::ComputerViewContainer *p_this);

static GAsyncReadyCallback mount_enclosing_volume_callback(GFile *volume, GAsyncResult *res, Peony::ComputerViewContainer *p_this)
{
    GError *err = nullptr;
    g_file_mount_enclosing_volume_finish (volume, res, &err);
    if ((nullptr == err) || (g_error_matches (err, G_IO_ERROR, G_IO_ERROR_ALREADY_MOUNTED))) {
        qDebug() << "login successful!";
        // 尝试获取挂载点的default location uri
        g_autoptr (GMount) mount = g_file_find_enclosing_mount(volume, nullptr, nullptr);
        if (mount) {
            g_autoptr (GFile) default_location = g_mount_get_default_location(mount);
            g_autofree gchar* default_uri = g_file_get_uri(default_location);
            Q_EMIT p_this->updateWindowLocationRequest(default_uri);
        } else {
            Q_EMIT p_this->updateWindowLocationRequest(p_this->m_remote_uri);
        }
    } else if (err->message) {
        qDebug() << "login remote error: " <<err->code<<err->message<<err->domain;
        QString showMessage(err->message);
        if (showMessage.startsWith("Message recipient")) {
            showMessage = QObject::tr("Message recipient disconnected from message bus without replying!");
        }
        QMessageBox::warning(nullptr, QObject::tr("log remote error"), showMessage, QMessageBox::Ok);
    }

    if (nullptr != err) {
        g_error_free(err);
    }

    return nullptr;
}

void aborted_cb(GMountOperation *op, Peony::ComputerViewContainer *p_this)
{
    g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
    p_this->disconnect();
}

#include "peony-qt/file-enumerator.h"
static QString getComputerUriFromUri(const QString &uri){
    /* volume item,遍历方式获取uri */
    FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString computerUri = uri;
    for (auto fileInfo : e.getChildren()) {
        FileInfoJob infoJob(fileInfo);
        infoJob.querySync();
        /* 由item的uri获取computer uir */
        auto info = infoJob.getInfo();
        if(fileInfo.get()->targetUri() == uri && !uri.isEmpty()){
            computerUri = fileInfo.get()->uri();
            break;
        }
    }
    return computerUri;
}

static QString getComputerUriFromUnixDevice(const QString &unixDevice)
{
    /* volume item,遍历方式获取uri */
    Peony::FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString uri;
    for (auto fileInfo : e.getChildren()) {
        Peony::FileInfoJob infoJob(fileInfo);
        infoJob.querySync();
        /* 由volume的unixDevice获取computer uir */
        auto info = infoJob.getInfo();
        QString device = fileInfo.get()->unixDeviceFile();
        if(device==unixDevice){
            uri = fileInfo.get()->uri();
            break;
        }
    }
    return uri;
}

Peony::ComputerViewContainer::ComputerViewContainer(QWidget *parent) : DirectoryViewWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_op = g_mount_operation_new();
    g_signal_connect (m_op, "aborted", G_CALLBACK (aborted_cb), this);
    g_signal_connect (m_op, "ask-question", G_CALLBACK(ask_question_cb), this);
    g_signal_connect (m_op, "ask-password", G_CALLBACK (ask_password_cb), this);

//     connect(this, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos){
//         auto selectedIndexes = m_view->selectionModel()->selectedIndexes();
//         auto index = m_view->indexAt(pos);
//         if (!selectedIndexes.contains(index))
//             m_view->clearSelection();

//         if (index.isValid()) {
//             m_view->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
//         }

//         QMenu menu(this);
//         auto model = static_cast<ComputerProxyModel *>(m_view->model());
//         QStringList uris;
//         QList<AbstractComputerItem *> items;
//         for (auto index : m_view->selectionModel()->selectedIndexes()) {
//             auto item = model->itemFromIndex(index);
//             uris<<item->uri();
//             if (item->uri() == "network:///")
//                 return;
//             items<<item;
//         }

//         if (items.count() == 0) {

//             menu.addAction(tr("Connect a server"), [=](){
//                 if (nullptr !=m_dlg) {
//                     delete m_dlg;
//                     m_dlg = nullptr;
//                 }
//                 m_dlg = new ConnectServerDialog();

//                 auto code = m_dlg->exec();
//                 if (code == QDialog::Rejected) {
//                     return;
//                 }
//                 GFile* m_volume = g_file_new_for_uri(m_dlg->uri().toUtf8().constData());
//                 m_remote_uri = m_dlg->uri();

//                 if (nullptr != m_serverLoginDlg) {
//                     delete m_serverLoginDlg;
//                     m_serverLoginDlg = nullptr;
//                 }
//                 m_serverLoginDlg = new ConnectServerLogin(m_remote_uri);
//                 g_file_mount_enclosing_volume(m_volume, G_MOUNT_MOUNT_NONE, m_op, nullptr, GAsyncReadyCallback(mount_enclosing_volume_callback), this);
//             });
//         } else if (items.count() == 1
// //                   &&  (items.first()->uri() != "" ||items.first()->m_unixDeviceName.contains("/dev/sr"))
//                    && items.first()->uri() != "network:///" || items.first()->m_unixDeviceName.contains("/dev/sr")) {
//             //bool isWayland = qApp->property("isWayland").toBool(); // related to #105070
//             auto item = items.first();
//             bool unmountable = item->canUnmount() && !item->property("isKydrive").toBool();
//             bool isReddisk = false;
//             //fix bug#212689, 212690, 213120, 213121, hide reddisk format and unmount option
//             if (item->m_unixDeviceName.startsWith("/dev/dm") && QFile::exists("/opt/AQTJ/Client/JC/MAIN/bin/jc_main_ui"))
//                 isReddisk = true;

//             if(!item->canEject() && ! isReddisk){
//                 menu.addAction(tr("Unmount"), [=](){
//                     item->unmount(G_MOUNT_UNMOUNT_NONE);
//                 });
//                 menu.actions().first()->setEnabled(unmountable);
//             }
//             //fix bug#175330, wayland should be the same with mainline version
//             /* else if (isWayland && ! isReddisk) {
//                 menu.addAction(tr("Unmount"), [=](){
//                     item->unmount(G_MOUNT_UNMOUNT_NONE);
//                 });
//                 menu.actions().first()->setEnabled(unmountable);
//             }*/
//             /*eject function for volume. fix #18216*/
//             if(item->canEject()){
//                 auto ejectAction = menu.addAction(tr("Eject"), [=](){
//                     item->eject(G_MOUNT_UNMOUNT_NONE);
//                 });
//                 QString unixDevice = item->m_unixDeviceName;
//                 if(unixDevice.contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，将弹出菜单置灰不可用。link to bug#143293. */
//                     menu.actions().first()->setDisabled(FileUtils::isBusyDevice(unixDevice));
//                 }
//             }

//             auto uri = item->uri();
//             QString unixDevice = item->m_unixDeviceName;
//             if (uri.isEmpty() && !unixDevice.isEmpty()) {
//                 uri = getComputerUriFromUnixDevice(unixDevice);
//             }

//             auto realUri = m_view->tryGetVolumeRealUriFromUri(uri);
//             if (!realUri.isEmpty()) {
//                 uri = realUri;
//             }

//             if(uri.startsWith("file://") || !unixDevice.isEmpty()) {
//                 uri = getComputerUriFromUri(uri);
//             }
//             auto info = FileInfo::fromUri(uri);
//             if (info->displayName().isEmpty() || info->targetUri().isEmpty()) {
//                 FileInfoJob j(info);
//                 j.querySync();
//             }

//             auto mount = VolumeManager::getMountFromUri(info->targetUri());
//             //fix bug#52491, CDROM and DVD can format issue

//             //fix bug#140543, remote redirection volume can be format
//             if (nullptr != mount) {
//                 auto volume = VolumeManager::getVolumeFromMount(mount);
//                 if (mount && volume && ! isReddisk) {
//                     QString unixDevice = FileUtils::getUnixDevice(info->uri());
//                     if (!unixDevice.isNull() && unixDevice.contains("/dev/sr")
//                             &&!unixDevice.startsWith("/dev/bus/usb")
//                             && info->isVolume()) {
//                         auto fdMenu = menu.addAction(tr("Format"));
//                         fdMenu->setEnabled(false);
// #ifndef KY_UDF_BURN
//                         DiscControl *discControl = new DiscControl(unixDevice);
//                         if(discControl->work()){
//                            connect(discControl, &DiscControl::workFinished, [=](DiscControl *discCtrl){
//                                connect(fdMenu, &QAction::triggered, [=](){
//                                    UdfFormatDialog *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlg(uri, discCtrl);
//                                    udfFormatDlg->show();
//                                });
//                                qDebug() << unixDevice << "supportUdf values:" << discCtrl->supportUdf();
//                                fdMenu->setEnabled(discCtrl->supportUdf());
//                            });
//                         }
// #else
//                         UdfBurn::DiscControl *discControl = new UdfBurn::DiscControl(unixDevice);
//                         if(discControl->work()){
//                            connect(discControl, &UdfBurn::DiscControl::workFinished, [=](UdfBurn::DiscControl *discCtrl){
//                                connect(fdMenu, &QAction::triggered, [=](){
//                                    UdfBurn::UdfFormatDialogWrapper *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlgWrapper(uri, discCtrl);
//                                    udfFormatDlg->show();
//                                });
//                                qDebug() << unixDevice << "KY_UDF_BURN supportUdf values:" << discCtrl->supportUdf();
//                                fdMenu->setEnabled(discControl->supportUdf());
//                            });
//                         }
// #endif
//                     } else if (! unixDevice.isNull() && ! unixDevice.contains("/dev/sr")
//                             &&!unixDevice.startsWith("/dev/bus/usb")
//                             && info->isVolume() && info->canUnmount()/* && info->targetUri() != "file:///data"*/) {
//                         auto fdMenu = menu.addAction(tr("Format"), [=] () {
//                             Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
//                             fd->show();
//                         });
//                         if (! mount) {
//                             fdMenu->setEnabled(false);
//                         }
//                     }
//                 }
//             } else {
//                 if (!unixDevice.isNull() && !unixDevice.contains("/dev/sr")
//                         && !unixDevice.startsWith("/dev/bus/usb")
//                         && info->isVolume() && !uri.isEmpty()) {
//                     auto fdMenu = menu.addAction(tr("Format"), [=](){
//                         Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
//                         fd->show();
//                     });
//                 }
//             }

//             QStringList fakeUrls;
//             fakeUrls<<uri;

//             auto menuPlugins = PluginManager::getInstance()->getComputerViewMenuPlugins();
//             for (auto plugin : menuPlugins) {
//                 menu.addActions(plugin->menuActions(Peony::MenuPluginInterface::SideBar, "computer:///", fakeUrls));
//             }

//             if (qApp->property("deviceRenamePluginLoaded").toBool()) {
//                 // add drive rename action, link to: #105070
//                 auto driveRenamePlugin = new DriveRename(this);

//                 menu.addActions(driveRenamePlugin->menuActions(Peony::MenuPluginInterface::Type::SideBar, "computer:///", fakeUrls));
//             }

//             if (!item->uri().startsWith("network://")) {
//                 auto a = menu.addAction(tr("Property"), [=]() {
//                     if (uri.isNull()) {
//                         QMessageBox::warning(0, 0, tr("You have to mount this volume first"));
//                     } else {
//                         QProcess p;
//                         p.setProgram("/usr/bin/peony");
//                         QStringList args;
//                         args << "-p" << uri;
//                         p.setArguments(args);
// //                    p.startDetached();
//                         p.startDetached(p.program(), p.arguments());
//                     }
//                 });
//                 auto targetUri = FileUtils::getTargetUri(info->uri());
//                 bool isMounted = FileUtils::isFileUnmountable(info->uri());
//                 if ("file:///" == uri || "file:///" == targetUri || "file:///data" == uri || "computer:///ukui-data-volume" == uri) {
//                     isMounted = true;
//                 }
//                 a->setEnabled(!uri.isNull() && isMounted);
//                 if(item->m_unixDeviceName.contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。link to bug#143293. */
//                     a->setDisabled(FileUtils::isBusyDevice(item->m_unixDeviceName));
//                 }
//             }

//             FMWindowIface *windowIface = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
//             menu.addAction(tr("Open In New Window"), [=](){
//                 auto targetUri = info->targetUri();
//                 if(targetUri.isEmpty() && "computer:///ukui-data-volume" == info->uri()){
//                     targetUri = "file:///data";
//                 }
//                 auto newWindow = windowIface->create(targetUri);
//                 dynamic_cast<QWidget *>(newWindow)->show();
//             });

//             if (!qApp->property("tabletMode").toBool()) {
//                 menu.addAction(tr("Open In New Tab"), [=](){
//                     auto targetUri = info->targetUri();
//                     if(targetUri.isEmpty() && "computer:///ukui-data-volume" == info->uri()){
//                         targetUri = "file:///data";
//                     }
//                     windowIface->addNewTabs(QStringList()<<targetUri);
//                 });
//             }
//         } else if(items.first()->uri() != "" && items.first()->uri() != "network:///"){
//             qDebug() << "unable Property uri:" <<items.first()->uri();
//             menu.addAction(tr("Property"));
//             menu.actions().first()->setEnabled(false);
//         }

//         if (!menu.isEmpty())
//             menu.exec(mapToGlobal(pos));
//     });
}

static void ask_question_cb(GMountOperation *op, char *message, char **choices, Peony::ComputerViewContainer *p_this)
{
    qDebug()<<"ask question cb:"<<message;
    Q_UNUSED(p_this);
    QMessageBox *msg_box = new QMessageBox;
    msg_box->setIcon(QMessageBox::Information);
    msg_box->setText(message);
    char **choice = choices;
    int i = 0;
    while (*choice) {
        qDebug()<<*choice;
        QPushButton *button = msg_box->addButton(QString(*choice), QMessageBox::ActionRole);
        p_this->m_dlg->connect(button, &QPushButton::clicked, [=]() {
            g_mount_operation_set_choice(op, i);
        });
        *choice++;
        i++;
    }
    //block ui
    msg_box->exec();
    msg_box->deleteLater();
    qDebug()<<"msg_box done";
    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

static void ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags, Peony::ComputerViewContainer *p_this)
{
    Q_UNUSED(message);
    Q_UNUSED(default_user);
    Q_UNUSED(default_domain);
    Q_UNUSED(flags);
    Q_UNUSED(p_this);

    if(!p_this->m_serverLoginDlg){
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    if (p_this->m_serverLoginDlg->property("IsRejected").toBool()) {
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    if (p_this->m_serverLoginDlg->property("IsFirstAnonymous").toBool()
            && g_mount_operation_get_anonymous(op)) {
        p_this->m_serverLoginDlg->setProperty("IsFirstAnonymous", false);
        g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
        return;
    }

    if ((flags & G_ASK_PASSWORD_NEED_PASSWORD)
            && (g_mount_operation_get_username(op) || g_mount_operation_get_password(op))) {
        QString str = "";
        p_this->m_serverLoginDlg->setCurrentPasswd(str);
        p_this->m_serverLoginDlg->setCurrentUserName(str);
    }

    int code = p_this->m_serverLoginDlg->exec();
    g_mount_operation_set_anonymous(op, p_this->m_serverLoginDlg->anonymous());
    if (code == QDialog::Rejected) {
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        p_this->m_serverLoginDlg->setProperty("IsRejected", true);
        return;
    }

    p_this->m_serverLoginDlg->syncRemoteServer(p_this->m_remote_uri);
    if (!p_this->m_serverLoginDlg->anonymous()) {
        g_mount_operation_set_username(p_this->m_op, p_this->m_serverLoginDlg->user().toUtf8().constData());
        g_mount_operation_set_password(p_this->m_op, p_this->m_serverLoginDlg->password().toUtf8().constData());
        g_mount_operation_set_domain(p_this->m_op, p_this->m_serverLoginDlg->domain().toUtf8().constData());
    } else {
        p_this->m_serverLoginDlg->setProperty("IsFirstAnonymous", true);
    }
    g_mount_operation_set_password_save(p_this->m_op,/* dlgLogin.savePassword()? */G_PASSWORD_SAVE_NEVER/*: G_PASSWORD_SAVE_FOR_SESSION*/);
    p_this->m_serverLoginDlg->setProperty("IsRejected", false);

    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}


Peony::ComputerViewContainer::~ComputerViewContainer()
{
    if (nullptr != m_op) {
        g_object_unref(m_op);
    }
}

const QStringList Peony::ComputerViewContainer::getSelections()
{
    QStringList uris;
    // fix #260661 访问ftp时文管闪退并且产生core文件
    // note: 从gdb查看core信息，发现m_view在挂载异常时为nullptr，增加非空判断避免空指针调用导致的崩溃
    if (!m_view) {
        return uris;
    }
    auto model = static_cast<ComputerProxyModel *>(m_view->model());
    for (auto index : m_view->selectionModel()->selectedIndexes()) {
        auto item = model->itemFromIndex(index);
        uris<<item->uri();
    }
    return uris;
}

void Peony::ComputerViewContainer::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(this->rect(), palette().base());

    DirectoryViewWidget::paintEvent(e);
}

void Peony::ComputerViewContainer::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
//        if (m_enterAction)
//            m_enterAction->triggered();
        e->accept();
        return;
    }

    QWidget::keyPressEvent(e);
}

void Peony::ComputerViewContainer::bindModel(Peony::FileItemModel *model, Peony::FileItemProxyFilterSortModel *proxyModel)
{
    //try fo fix model item update abnormal issue
    //related to task#224468
    disconnect(m_model);
    disconnect(m_proxyModel);
    m_model = model;
    m_proxyModel = proxyModel;
    model->setRootUri("computer:///");
    connect(model, &Peony::FileItemModel::findChildrenFinished, this, &Peony::DirectoryViewWidget::viewDirectoryChanged);

    if (m_view)
        m_view->deleteLater();

    m_view = new ComputerView(this);
    auto layout = new QHBoxLayout;
    layout->addWidget(m_view);
    setLayout(layout);
    Q_EMIT viewDirectoryChanged();

    auto selectionModel = m_view->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &DirectoryViewWidget::viewSelectionChanged);;

    connect(this, &ComputerViewContainer::containerDoubleClicked, this, [=](const QModelIndex &index){
        if (!index.parent().isValid() || m_view->getRightDoubleClickState()){
            //qDebug() << "doubleClicked state" << m_view->getRightDoubleClickState();
            m_view->setRightDoubleClickState(false);
            return;
        }

        auto model = static_cast<ComputerProxyModel *>(m_view->model());
        auto item = model->itemFromIndex(index);
        if (!item->uri().isEmpty()) {
            item->check();
            qDebug() << "doubleClicked updateWindowLocationRequest:"<<item->uri();
            this->updateWindowLocationRequest(item->uri());
        } else {
            item->mount();
        }
    });

    connect(m_view, &QAbstractItemView::activated, this, [=](const QModelIndex &index) {
        //平板模式下，长按打开文件处理
        if (m_view->m_touch_active_timer->isActive()) {
            auto costTime = m_view->m_touch_active_timer->interval() - m_view->m_touch_active_timer->remainingTime();
            if (costTime > qApp->doubleClickInterval()) {
                m_view->m_touch_active_timer->stop();
                return;
            }
        }
        const auto selected = m_view->selectionModel()->selectedIndexes();
        // 多选时交给主窗口处理
        if (selected.count() > 1)
            return;

        QModelIndex target;
        if (selected.count() == 1) {
            target = selected.first();
        } else if (index.isValid()) {
            // 偶现双击时 selection 尚未写入模型，activated 仍带有正确 index
            target = index;
        } else {
            return;
        }

        Q_EMIT containerDoubleClicked(target);
        m_view->m_touch_active_timer->stop();
    });

    connect(this, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos){
        auto selectedIndexes = m_view->selectionModel()->selectedIndexes();
        auto index = m_view->indexAt(pos);
        if (!selectedIndexes.contains(index))
            m_view->clearSelection();

        if (index.isValid()) {
            m_view->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
        }

        QMenu menu(this);
        auto model = static_cast<ComputerProxyModel *>(m_view->model());
        QStringList uris;
        QList<AbstractComputerItem *> items;
        for (auto index : m_view->selectionModel()->selectedIndexes()) {
            auto item = model->itemFromIndex(index);
            uris<<item->uri();
            if (item->uri() == "network:///")
                return;
            items<<item;
        }

        if (items.count() == 0) {
            bool allowSmbShare = Peony::GlobalSettings::getInstance()->isJsonCacheKey(ENABLE_SMB_SHARE)
            ? Peony::GlobalSettings::getInstance()->getValue(ENABLE_SMB_SHARE).toBool() : true;
            if (!allowSmbShare) {
                qDebug() << "ENABLE_SMB_SHARE" << allowSmbShare;
                return;
            }
            menu.addAction(tr("Connect a server"), [=](){
                if (m_dlg) {
                    m_dlg->close();
                    m_dlg->deleteLater();
                    m_dlg = nullptr;
                }

                m_dlg = new ConnectServerDialog();

                auto code = m_dlg->exec();
                if (code == QDialog::Rejected) {
                    return;
                }
                //GFile* m_volume = g_file_new_for_uri(m_dlg->uri().toUtf8().constData());
                m_remote_uri = m_dlg->uri();

                // deprecated, use updateWindowLocationRequest() instead.
                if (m_serverLoginDlg) {
                    m_serverLoginDlg->close();
                    m_serverLoginDlg->deleteLater();
                    m_serverLoginDlg = nullptr;
                }
                m_serverLoginDlg = new ConnectServerLogin(m_remote_uri);

                updateWindowLocationRequest(m_remote_uri);
                //g_file_mount_enclosing_volume(m_volume, G_MOUNT_MOUNT_NONE, m_op, nullptr, GAsyncReadyCallback(mount_enclosing_volume_callback), this);
            });
        } else if (items.count() == 1
                       //                   &&  (items.first()->uri() != "" ||items.first()->m_unixDeviceName.contains("/dev/sr"))
                       && items.first()->uri() != "network:///" || items.first()->m_unixDeviceName.contains("/dev/sr")) {
            //bool isWayland = qApp->property("isWayland").toBool(); // related to #105070
            auto item = items.first();
            bool unmountable = item->canUnmount() && !item->property("isKydrive").toBool();
            bool isReddisk = false;
            //fix bug#212689, 212690, 213120, 213121, hide reddisk format and unmount option
            //fix bug#333377, jc encrpt software change install path issue
            if (item->m_unixDeviceName.startsWith("/dev/dm") &&
                    (QFile::exists("/opt/AQTJ/Client/JC/MAIN/bin/jc_main_ui") ||
                    QFile::exists("/opt/apps/com.jc.bmt/files/MAIN/bin/jc_main_ui")) )
                isReddisk = true;

            bool isCloud = QFile::exists("/etc/ecloud") || QFile::exists("/usr/local/share/Ecloud");
            bool hideUnmount = false;
            bool hideFormat = false;
            //fix task#185121, sudo mount remove volume unmount has no effect issue
            //除samba, ftp, sftp 外的远程目录，云桌面环境下，不显示卸载和格式化选项
            /* 云桌面重定向的盘，不显示卸载选项和格式化选项 bug#255725, task#185121 */
            if (isCloud && AbstractComputerItem::RemoteVolume == item->itemType()) {
                hideFormat = true;
                if ( ! item->uri().startsWith("smb:///") &&
                     ! item->uri().startsWith("ftp:///") &&
                     ! item->uri().startsWith("sftp:///")) {
                     hideUnmount = true;
                }
            }

            bool localDiskShowUnmountOpt = true;
            if(QUrl(item->uri()).isLocalFile() && !item->canEject()){
                localDiskShowUnmountOpt = MANAGER_CONTROL->checkLocalDiskUnmountPermission();
            }

            //fix bug#175330, wayland should be the same with mainline version
            /* else if (isWayland && ! isReddisk) {
                    menu.addAction(tr("Unmount"), [=](){
                        item->unmount(G_MOUNT_UNMOUNT_NONE);
                    });
                    menu.actions().first()->setEnabled(unmountable);
                }*/
            /*eject function for volume. fix #18216*/

            auto uri = item->uri();
            QString unixDevice = item->m_unixDeviceName;
            if (uri.isEmpty() && !unixDevice.isEmpty()) {
                uri = getComputerUriFromUnixDevice(unixDevice);
            }

            auto realUri = m_view->tryGetVolumeRealUriFromUri(uri);
            if (!realUri.isEmpty()) {
                uri = realUri;
            }

            if(uri.startsWith("file://") || !unixDevice.isEmpty()) {
                uri = getComputerUriFromUri(uri);
            }
            auto info = FileInfo::fromUri(uri);
            if (info->displayName().isEmpty() || info->targetUri().isEmpty()) {
                FileInfoJob j(info);
                j.querySync();
            }

            auto targetUri = FileUtils::getTargetUri(info->uri());
            bool isMounted = FileUtils::isFileUnmountable(info->uri());
            if ("file:///" == uri || "file:///" == targetUri || "file:///data" == uri || "computer:///ukui-data-volume" == uri) {
                isMounted = true;
            }
            FMWindowIface *windowIface = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
            if (!qApp->property("tabletMode").toBool()) {
                menu.addAction(QIcon::fromTheme("tab-new-symbolic"),tr("Open In New Tab"), [=](){
                    auto targetUri = info->targetUri();
                    if(targetUri.isEmpty() && "computer:///ukui-data-volume" == info->uri()){
                        targetUri = "file:///data";
                        if (Peony::FileUtils::isFileExsit("file:///data/usershare"))
                            targetUri = "file:///data/usershare";
                    }
                    windowIface->addNewTabs(QStringList()<<targetUri);
                });
            }
            menu.actions().last()->setEnabled(!uri.isNull() && isMounted);

            menu.addAction(QIcon::fromTheme("window-new-symbolic"),tr("Open In New Window"), [=](){
                auto targetUri = info->targetUri();
                if(targetUri.isEmpty() && "computer:///ukui-data-volume" == info->uri()){
                    targetUri = "file:///data";
                    if (Peony::FileUtils::isFileExsit("file:///data/usershare"))
                        targetUri = "file:///data/usershare";
                }
                auto newWindow = windowIface->create(targetUri);
                dynamic_cast<QWidget *>(newWindow)->show();
            });
            menu.actions().last()->setEnabled(!uri.isNull() && isMounted);
            menu.addSeparator();
            if(!item->canEject() && ! isReddisk && ! hideUnmount && localDiskShowUnmountOpt){
                menu.addAction(QIcon::fromTheme("user-trash-symbolic"),tr("Unmount"), [=](){
                    item->unmount(G_MOUNT_UNMOUNT_NONE);
                });
                menu.actions().last()->setEnabled(unmountable);
            }

            if(item->canEject()){
                auto ejectAction = menu.addAction(QIcon::fromTheme("media-eject-symbolic"),tr("Eject"), [=](){
                    item->eject(G_MOUNT_UNMOUNT_NONE);
                });
                QString unixDevice = item->m_unixDeviceName;
                if(unixDevice.contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，将弹出菜单置灰不可用。link to bug#143293. */
                    menu.actions().last()->setDisabled(FileUtils::isBusyDevice(unixDevice));
                }
            }

            auto mount = VolumeManager::getMountFromUri(info->targetUri());
            const bool isBurnOptical = info->targetUri().startsWith("burn:///") && unixDevice.contains("/dev/sr");
            //fix bug#52491, CDROM and DVD can format issue

            //fix bug#140543, remote redirection volume can be format
            if (nullptr != mount || isBurnOptical) {
                auto volume = mount ? VolumeManager::getVolumeFromMount(mount) : nullptr;
                if (((mount && volume) || isBurnOptical) && ! isReddisk && ! hideFormat) {
                    QString unixDevice = FileUtils::getUnixDevice(info->uri());
                    if (unixDevice.isEmpty()) {
                        unixDevice = item->m_unixDeviceName;
                    }
                    if (!unixDevice.isNull() && unixDevice.contains("/dev/sr")
                        &&!unixDevice.startsWith("/dev/bus/usb")
                        && info->isVolume()) {
                        qDebug() << "format-menu add optical format item, device:" << unixDevice;
                        auto fdMenu = menu.addAction(QIcon::fromTheme("format-symbolic"),tr("Format"));
                        fdMenu->setEnabled(false);
#ifndef KY_UDF_BURN
                        DiscControl *discControl = new DiscControl(unixDevice);
                        if(discControl->work()){
                            connect(discControl, &DiscControl::workFinished, [=](DiscControl *discCtrl){
                                connect(fdMenu, &QAction::triggered, [=](){
                                    UdfFormatDialog *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlg(uri, discCtrl);
                                    udfFormatDlg->show();
                                });
                                qDebug() << unixDevice << "supportUdf values:" << discCtrl->supportUdf();
                                fdMenu->setEnabled(discCtrl->supportUdf());
                            });
                        }
#else
                            UdfBurn::DiscControl *discControl = new UdfBurn::DiscControl(unixDevice);
                            if(discControl->work()){
                                connect(discControl, &UdfBurn::DiscControl::workFinished, [=](UdfBurn::DiscControl *discCtrl){
                                    connect(fdMenu, &QAction::triggered, [=](){
                                        UdfBurn::UdfFormatDialogWrapper *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlgWrapper(uri, discCtrl);
                                        udfFormatDlg->show();
                                    });
                                    qDebug() << unixDevice << "KY_UDF_BURN supportUdf values:" << discCtrl->supportUdf();
                                    fdMenu->setEnabled(discControl->supportUdf());
                                });
                            }
#endif
                    } else if (! unixDevice.isNull() && ! unixDevice.contains("/dev/sr")
                               &&!unixDevice.startsWith("/dev/bus/usb")
                               &&!unixDevice.startsWith("/dev/loop")
                               && info->isVolume() && info->canUnmount()/* && info->targetUri() != "file:///data"*/) {
                        bool allowFormat = true;
                        if (item->canEject()) {
                            allowFormat = MANAGER_CONTROL->checkFormatRemovableDevicePermission();
                        }
                        if (allowFormat) {
                            auto fdMenu = menu.addAction(QIcon::fromTheme("format-symbolic"),tr("Format"), [=] () {
                                Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
                                fd->show();
                            });
                            if (! mount) {
                                fdMenu->setEnabled(false);
                            }
                        }
                    }
                }
            } else {
                if (!unixDevice.isNull() && !unixDevice.contains("/dev/sr")
                    && !unixDevice.startsWith("/dev/bus/usb")
                    && !unixDevice.startsWith("/dev/loop")
                    && !unixDevice.isEmpty()
                    && info->isVolume() && !uri.isEmpty()) {
                    bool allowFormat = true;
                    if (item->canEject()) {
                        allowFormat = MANAGER_CONTROL->checkFormatRemovableDevicePermission();
                    }
                    if (allowFormat) {
                        auto fdMenu = menu.addAction(QIcon::fromTheme("format-symbolic"),tr("Format"), [=](){
                            Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
                            fd->show();
                        });
                    }
                }
            }

            QStringList fakeUrls;
            fakeUrls<<uri;

            auto menuPlugins = PluginManager::getInstance()->getComputerViewMenuPlugins();
            for (auto plugin : menuPlugins) {
                auto tmp = plugin->menuActions(Peony::MenuPluginInterface::SideBar, "computer:///", fakeUrls);
                menu.addActions(tmp);
                for (auto action : tmp) {
                    action->setParent(this);
                    if(action->icon().isNull()){
                        auto pluginicon = plugin->icon();
                        if(!pluginicon.isNull()){
                            action->setIcon(pluginicon);
                        }
                    }
                }
            }

            if (qApp->property("deviceRenamePluginLoaded").toBool()) {
                // add drive rename action, link to: #105070
                auto driveRenamePlugin = new DriveRename(this);

                menu.addActions(driveRenamePlugin->menuActions(Peony::MenuPluginInterface::Type::SideBar, "computer:///", fakeUrls));
            }
            menu.addSeparator();
            if (!item->uri().startsWith("network://")) {
                auto a = menu.addAction(QIcon::fromTheme("properties-symbolic"),tr("Property"), [=]() {
                    if (uri.isNull()) {
                        QMessageBox::warning(0, 0, tr("You have to mount this volume first"));
                    } else {
                        QProcess p;
                        p.setProgram("/usr/bin/peony");
                        QStringList args;
                        args << "-p" << uri;
                        p.setArguments(args);
                        //                    p.startDetached();
                        p.startDetached(p.program(), p.arguments());
                    }
                });
                menu.actions().last()->setShortcut(QKeySequence("Alt+Enter"));
                // auto targetUri = FileUtils::getTargetUri(info->uri());
                // bool isMounted = FileUtils::isFileUnmountable(info->uri());
                // if ("file:///" == uri || "file:///" == targetUri || "file:///data" == uri || "computer:///ukui-data-volume" == uri) {
                //     isMounted = true;
                // }
                a->setEnabled(!uri.isNull() && isMounted);
                if(item->m_unixDeviceName.contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。link to bug#143293. */
                    a->setDisabled(FileUtils::isBusyDevice(item->m_unixDeviceName));
                }
            }


        } else if(items.first()->uri() != "" && items.first()->uri() != "network:///"){
            qDebug() << "unable Property uri:" <<items.first()->uri();
            menu.addAction(QIcon::fromTheme("properties-symbolic"),tr("Property"));
            menu.actions().last()->setShortcut(QKeySequence("Alt+Enter"));
            menu.actions().first()->setEnabled(false);
        }

        if (!menu.isEmpty())
            menu.exec(mapToGlobal(pos));
    });

//    m_enterAction = new QAction(this);
//    m_enterAction->setShortcut(Qt::Key_Enter);
//    addAction(m_enterAction);

//    connect(m_enterAction, &QAction::triggered, this, [=](){
//        if (m_view->selectionModel()->selectedIndexes().count() == 1) {
//            Q_EMIT m_view->doubleClicked(m_view->selectionModel()->selectedIndexes().first());
//        }
//    });

    connect(m_view, &ComputerView::updateLocationRequest, this, &ComputerViewContainer::updateWindowLocationRequest);
}

void Peony::ComputerViewContainer::beginLocationChange()
{
    Q_EMIT viewDirectoryChanged();

    m_view->refresh();
}

void Peony::ComputerViewContainer::stopLocationChange()
{
    Q_EMIT viewDirectoryChanged();
}
