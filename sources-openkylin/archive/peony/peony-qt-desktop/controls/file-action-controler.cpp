#include "file-action-controler.h"
#include "global-settings.h"
#include "clipboard-utils.h"
#include "file-enumerator.h"
#include "file-meta-info.h"
#include "thumbnail-manager.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-info-manager.h"
#include "file-watcher.h"
#include "file-operation-manager.h"
#include "file-move-operation.h"
#include "file-trash-operation.h"
#include "file-copy-operation.h"
#include "file-operation-utils.h"
#include "file-utils.h"
#include "desktop-menu.h"
#include "peony-desktop-application.h"
#include "audio-play-manager.h"
#include "file-launch-manager.h"
#include "file-rename-operation.h"
#include "file-batch-rename-operation.h"
#include "advanced-desktop-item-model.h"
#include "image-mount-manager.h"
#include "manager-control.h"

#include <QtX11Extras/QX11Info>
#include <kstartupinfo.h>
#ifdef KY_SDK_WAYLANDHELPER
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#include <kysdk/applications/ukuiwindowhelper/ukuistartup.h>
#endif

#include <QStandardPaths>
#include <QApplication>
#include <QDebug>
#include <QMimeData>
#include <QClipboard>
#include <QMessageBox>
#include <QTimer>
#include <QDir>
#include <QProcess>

#define MINGRIDSIZE 64
#define MAXGRIDSIZE 200
#define GRIDMARGIN 15
#define ICONMARGIN 5

using namespace Peony;

static bool meetSpecialConditions(const QStringList& selectedUris)
{
    /* The desktop home directory, computer, and trash do not allow operations such as copying, cutting,
     * deleting, renaming, moving, or using shortcut keys for corresponding operations.add by 2021/06/17 */
    static QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    if (selectedUris.contains("computer:///")
       ||selectedUris.contains("trash:///")
       ||selectedUris.contains(homeUri)){
        return true;
    }

    return false;
}

static FileActionController *global_instance = nullptr;

FileActionController *FileActionController::getInstance()
{
    if (!global_instance) {
        global_instance = new FileActionController;
    }
    return global_instance;
}

FileActionController::FileActionController(QObject *parent)
{
    m_directryUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    auto zoolLevel = getZoomLevel();
    setDefaultZoomLevel(zoolLevel);

    connect(qApp, &QApplication::fontChanged, this, [=](){
        //直接调用槽函数，获取font的高度是没改变之前的
        QTimer::singleShot(100, this, [this]() {
            QSize newGridSize = recalculateGridSize();
            setGridSize(newGridSize);
        });
    });

    m_initialized = true;
}

bool FileActionController::enableShortcutKeys()
{
    return GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool();
}

void FileActionController::fileCopy(const QStringList &selectedUris)
{
    if (! GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool()) {
        return ;
    }
    Q_EMIT changeOpacity(1);
    qDebug() << "copy:" << selectedUris;
    if (!selectedUris.isEmpty() && !meetSpecialConditions(selectedUris)){
        ClipboardUtils::setClipboardFiles(selectedUris, false);
    }
}

void FileActionController::fileCut(const QStringList &selectedUris)
{
    if (! GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool()) {
        return ;
    }

    if (!selectedUris.isEmpty() && !meetSpecialConditions(selectedUris) && 
        MANAGER_CONTROL->canDeleteItems(selectedUris, m_directryUri))
    {
        Q_EMIT changeOpacity(0.5);
        ClipboardUtils::setClipboardFiles(selectedUris, true);
    }
}

void FileActionController::filePaste(const QStringList &selectedUris)
{
    if (! GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool()) {
        return ;
    }

    if (qApp->clipboard()->mimeData()->hasFormat ("uos/remote-copy")) {
        Q_EMIT changeOpacity(1);
        auto op = ClipboardUtils::pasteClipboardFiles(m_directryUri);
        if (!op) {
           // viewport()->update();
        }
    } else {
        paste(selectedUris);
    }
}

void FileActionController::paste(const QStringList &selectedUris)
{
    //auto clipUris = ClipboardUtils::getClipboardFilesUris();
    if (ClipboardUtils::getInstance()->isClipboardHasFiles() && !meetSpecialConditions(selectedUris)) {
        Q_EMIT changeOpacity(1);
        auto op = ClipboardUtils::pasteClipboardFiles(m_directryUri);
        if (op) {
            connect(op, &Peony::FileOperation::operationFinished, this, [=](){
                auto opInfo = op->getOperationInfo();
                auto targetUirs = opInfo->dests();
                QTimer::singleShot(300, this, [=](){
                    QStringList newFiles;
                    for (QString uri : targetUirs) {
                        auto origin = FileUtils::getOriginalUri(uri);
                        newFiles<<origin;
                    }
                    Q_EMIT setSelectedUris(newFiles);
                });
            }, Qt::BlockingQueuedConnection);
        }
    }
}

void FileActionController::triggerDesktopOrganization()
{
    Q_EMIT desktopOriganization();
}

void FileActionController::triggerQuitDesktopOrganization()
{
    Q_EMIT quitOriganization();
}

void FileActionController::deleteToTrash(const QStringList &selectedUris)
{
    //add CTRL+D for delete operation
    if (! GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool()) {
        return ;
    }

    if (!MANAGER_CONTROL->checkDeleteToTrashPermission()) {
        return;
    }

    if (!selectedUris.isEmpty() && !meetSpecialConditions(selectedUris) && 
        MANAGER_CONTROL->canDeleteItems(selectedUris, m_directryUri)){
       FileOperationUtils::trash(selectedUris, true);
    }
}

void FileActionController::undo()
{
    qApp->setProperty("rename", true);
    FileOperationManager::getInstance()->undo();
}

void FileActionController::redo()
{
    // do not relayout item with redo.
    qApp->setProperty("rename", true);
    FileOperationManager::getInstance()->redo();
}

void FileActionController::zoomIn()
{
    if (Huge == m_zoom_level) {
        return;
    }
    switch (getZoomLevel()) {
    case Small:
        m_zoom_level = Normal;
        break;
    case Normal:
        m_zoom_level = Large;
        break;
    case Large:
        m_zoom_level = Huge;
        break;
    default:
        break;
    }
    setDefaultZoomLevel(m_zoom_level);
}

void FileActionController::zoomOut()
{
    if (Small == m_zoom_level) {
        return;
    }
    switch (getZoomLevel()) {
    case Huge:
        m_zoom_level = Large;
        break;
    case Large:
        m_zoom_level = Normal;
        break;
    case Normal:
        m_zoom_level = Small;
        break;
    default:
        break;
    }
    setDefaultZoomLevel(m_zoom_level);
}

void FileActionController::setDefaultZoomLevel(ZoomLevel level)
{
    qDebug()<<"set default zoom level:"<<level;
    m_zoom_level = level;
    QSize newIconSize;
    int emblemSize = 16;
//    m_margin = QPoint(10, 5);
    switch (level) {
    case Small:{
//        m_margin *= 0.8;
        emblemSize = 8;
        newIconSize = QSize(24, 24);
        break;
    }
    case Large:
//        m_margin *= 1.2;
        emblemSize = 24;
        newIconSize = QSize(64, 64);
        break;
    case Huge:
//        m_margin *= 1.4;
        emblemSize = 32;
        newIconSize = QSize(96, 96);
        break;
    default:
        m_zoom_level = Normal;
        newIconSize = QSize(48, 48);
        break;
    }

    auto metaInfo = FileMetaInfo::fromUri("computer:///");
    if (metaInfo) {
        qDebug()<<"set zoom level"<<m_zoom_level;
        metaInfo->setMetaInfoInt("peony-qt-desktop-zoom-level", int(m_zoom_level));
    }

    GlobalSettings::getInstance()->setValue(DEFAULT_DESKTOP_ZOOM_LEVEL, m_zoom_level);

    setEmblemSize(emblemSize);
    setIconSize(newIconSize);

    QSize newGridSize = recalculateGridSize();
    setGridSize(newGridSize);
}

ZoomLevel FileActionController::getZoomLevel() const
{
    //FIXME:
    if (m_zoom_level != ZoomLevel::Invalid)
        return m_zoom_level;

    auto metaInfo = FileMetaInfo::fromUri("computer:///");
    if (metaInfo) {
        auto i = metaInfo->getMetaInfoInt("peony-qt-desktop-zoom-level");
        return ZoomLevel(i);
    }

    GFile *computer = g_file_new_for_uri("computer:///");
    GFileInfo *info = g_file_query_info(computer,
                                        "metadata::peony-qt-desktop-zoom-level",
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        nullptr,
                                        nullptr);
    char* zoom_level = g_file_info_get_attribute_as_string(info, "metadata::peony-qt-desktop-zoom-level");
    if (!zoom_level) {
        //qDebug()<<"======================no zoom level meta info\n\n\n";
        g_object_unref(info);
        g_object_unref(computer);
        return Normal;
    }
    g_object_unref(info);
    g_object_unref(computer);
    QString zoomLevel = zoom_level;
    g_free(zoom_level);
    //qDebug()<<ZoomLevel(QString(zoomLevel).toInt())<<"\n\n\n\n\n\n\n\n";
    return ZoomLevel(zoomLevel.toInt()) == ZoomLevel::Invalid? Normal: ZoomLevel(QString(zoomLevel).toInt());
}

GAsyncReadyCallback FileActionController::queryZoomLevelAsyncCallback(GObject *obj, GAsyncResult *res, FileActionController *p_this)
{
    GError *err = nullptr;
    auto info = g_file_query_info_finish(G_FILE(obj), res, &err);

    if (info) {
        char* zoom_level = g_file_info_get_attribute_as_string(info, "metadata::peony-qt-desktop-zoom-level");
        if (!zoom_level) {
            //qDebug()<<"======================no zoom level meta info\n\n\n";
            g_object_unref(info);
            p_this->setDefaultZoomLevel(Normal);
            return nullptr;
        }
        g_object_unref(info);
        QString zoomLevel = zoom_level;
        g_free(zoom_level);
        int level =(zoomLevel.toInt()) == ZoomLevel::Invalid? Normal: ZoomLevel(QString(zoomLevel).toInt());
        p_this->setDefaultZoomLevel(ZoomLevel(level));
        return nullptr;
    } else {
        if (err) {
            qDebug()<<err->code<<err->message;
            g_error_free(err);
        }
        p_this->setDefaultZoomLevel(Normal);
        return nullptr;
    }
}

void FileActionController::editUri(const QString &selectedUri)
{
    if (selectedUri.isEmpty() || meetSpecialConditions(QStringList(selectedUri))) {
        return;
    }
    auto origin = FileUtils::getOriginalUri(selectedUri);
    QTimer::singleShot(100, this, [=]() {
        Q_EMIT edit(selectedUri);
    });
}

void FileActionController::remove(const QStringList &selectedUris)
{
    //Qt::SHIFT + Qt::Key_Delete
    qDebug() << "=====================" << selectedUris;
    if (! GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool()) {
        return ;
    }

    if (!meetSpecialConditions(selectedUris) && 
        MANAGER_CONTROL->canDeleteItems(selectedUris, m_directryUri)){
        qDebug() << "delete" << selectedUris;
        FileOperationUtils::executeRemoveActionWithDialog(selectedUris);
    }
}

void FileActionController::help()
{
    PeonyDesktopApplication::showGuide();
}

void FileActionController::showProperties(const QStringList &selectedUris)
{
    //Qt::ALT + Qt::Key_Return ,Qt::ALT + Qt::Key_Enter
    if (!m_menu) {
        delete m_menu;
        m_menu = nullptr;
    }
    m_menu = new DesktopMenu(m_directryUri, selectedUris);
    if (selectedUris.count() > 0) {
        m_menu->showProperties(selectedUris);
    } else {
        QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        m_menu->showProperties(desktopPath);
    }
}

void FileActionController::newFolder()
{
    //QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N)
    CreateTemplateOperation op(m_directryUri, CreateTemplateOperation::EmptyFolder, tr("New Folder"));
    op.run();
    auto targetUri = op.target();

    QTimer::singleShot(300, this, [=]() {
        QString origin = FileUtils::getOriginalUri(targetUri);
        Q_EMIT edit(origin);
   });
}

void FileActionController::refresh()
{
    //QKeySequence(Qt::CTRL + Qt::Key_R) | Qt::Key_F5
    //fix refresh clear copy files issue, link to bug#109247
    if (Peony::ClipboardUtils::isDesktopFilesBeCut())
        Peony::ClipboardUtils::clearClipboard();/* Refresh clear cut status */

    if (!m_freshing) {
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        m_freshing = true;
        PeonyDesktopApplication::getModel()->refresh();
    }
}

void FileActionController::finishRefresh()
{
    QGuiApplication::restoreOverrideCursor();
    m_freshing = false;
}

void FileActionController::invertSelections()
{
    //QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L)
    invertSelections();
}

void FileActionController::recoverNormalLevel()
{
    //QKeySequence(Qt::CTRL + Qt::Key_0)
    if (m_zoom_level == Normal)
        return;
    this->setDefaultZoomLevel(Normal);
}

void FileActionController::editUris(const QStringList &selectedUris)
{
    //QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_E)<<Qt::Key_F2)
    if (selectedUris.isEmpty() || meetSpecialConditions(selectedUris)) {
        return;
    }
    auto origin = FileUtils::getOriginalUri(selectedUris.first());
    QTimer::singleShot(100, this, [=]() {
        Q_EMIT edit(selectedUris.first());
    });
}

void FileActionController::setShowHidden()
{
    //QKeySequence(Qt::CTRL + Qt::Key_H)
    GlobalSettings *settings = GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        bool showHidden = settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
        settings->setGSettingValue(SHOW_HIDDEN_PREFERENCE, !showHidden);
    } else {
        bool showHidden = settings->getValue(SHOW_DESKTOP_HIDDEN_PREFERENCE).toBool();
        settings->setGSettingValue(SHOW_DESKTOP_HIDDEN_PREFERENCE, !showHidden);
    }
}

void FileActionController::clearClipboard()
{
    //Qt::Key_Escape
    Peony::ClipboardUtils::clearClipboard();
//    Q_EMIT update();
}

void FileActionController::setSelectAll()
{
    //QKeySequence::SelectAll
    if (! GlobalSettings::getInstance()->getValue(ENABLE_SHORTCUT_KEYS).toBool()) {
        return ;
    }

    Q_EMIT selectAll();
}

void FileActionController::showMenu(const QStringList &selectedUris, const QPoint &globalPos)
{
    if (selectedUris.isEmpty()) {
        if (!MANAGER_CONTROL->checkBackgroundMenuPermission()) {
            qDebug() << "Desktop background menu disabled by management control";
            return;
        }
    } else {
        for (const QString &uri : selectedUris) {
            if (!MANAGER_CONTROL->checkRightMenuPermission(uri)) {
                qDebug() << "Right menu disabled for item:" << uri;
                return;
            }
        }
    }

    if (m_menu) {
        delete m_menu;
        m_menu = nullptr;
    }
    m_menu = new DesktopMenu(m_directryUri, selectedUris);
#ifdef TEST_PEONY_DESKTOP_WIDGETS
    m_menu->addAction(tr("管理插件"), this, &FileActionController::requestManageWidgets);
    m_menu->addAction(tr("调整插件位置"), this, [=]{
        setDragDesktopWidgets(true);
    });
#endif
#ifdef KY_SDK_WAYLANDHELPER
    auto wmHelper = new UkuiWindowHelper(m_menu);
    wmHelper->removeTitleBar();
#endif
    connect(m_menu, &DesktopMenu::editUri, this, &FileActionController::editUri);
    connect(m_menu, &DesktopMenu::editUris, this, &FileActionController::editUris);
    connect(m_menu, &DesktopMenu::invertSelections, this, &FileActionController::selectAll);
    connect(m_menu, &DesktopMenu::setDefaultZoomLevel, this, &FileActionController::setDefaultZoomLevel);
    connect(m_menu, &DesktopMenu::refresh, this, &FileActionController::refresh);
    connect(m_menu, &DesktopMenu::changeOpacity, this, &FileActionController::changeOpacity);
    connect(m_menu, &DesktopMenu::paste, this, &FileActionController::paste);
   QTimer::singleShot(130, m_menu, [=]() {
        m_menu->exec(globalPos);
        m_urisToEdit = m_menu->urisToEdit();
   });
}

void FileActionController::closeMenu()
{
    if (m_menu) {
        m_menu->hide();
    }
}

void FileActionController::openFileByUri(QString uri)
{
    if (!MANAGER_CONTROL->checkMouseClickPermission(uri, true)) {
        qDebug() << "Double click disabled for item:" << uri;
        return;
    }

    auto info = FileInfo::fromUri(uri);
    auto job = new FileInfoJob(info);
    job->setAutoDelete();
    job->connect(job, &FileInfoJob::queryAsyncFinished, [=]() {
        if ((info->isDir() || info->isVolume() || info->isVirtual())) {
            QDir dir(info->filePath());
            if (! dir.exists())
            {
                Peony::AudioPlayManager::getInstance()->playWarningAudio();
                auto result = QMessageBox::question(nullptr, tr("Open Link failed"),
                                      tr("File not exist, do you want to delete the link file?"),
                                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
                    qDebug() << "Delete unused symbollink in desktop.";
                    QStringList selections;
                    selections.push_back(uri);
                    FileOperationUtils::trash(selections, true);
                }
                return;
            }

            if (! info->uri().startsWith("trash://")
                    && ! info->uri().startsWith("computer://")
                    &&  ! info->canExecute())
            {
                Peony::AudioPlayManager::getInstance()->playWarningAudio();
                QMessageBox::critical(nullptr, tr("Open failed"),
                                      tr("Open directory failed, you have no permission!"));
                return;
            }

#if 1
            FileLaunchManager::openAsync(uri, false, false);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            QProcess p;
            QUrl url = uri;
            p.setProgram("peony");
            p.setArguments(QStringList() << url.toEncoded() <<"%U&");
            qint64 pid;
            p.startDetached(&pid);
            // send startinfo to kwindowsystem
            quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
            KStartupInfoId startInfoId;
            startInfoId.initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
            startInfoId.setupStartupEnv();
            KStartupInfoData data;
            data.setHostname();
            data.addPid(pid);
            QRect rect = info.get()->property("iconGeometry").toRect();
#ifdef KY_SDK_WAYLANDHELPER
            ukuiStartupManager()->setStartupGeometryByPid(pid, rect.x(), rect.y(), rect.width(), rect.height());
#endif
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
            if (rect.isValid())
                data.setIconGeometry(rect);
#endif
            data.setLaunchedBy(getpid());
            KStartupInfo::sendStartup(startInfoId, data);
            KStartupInfo::resetStartupEnv();
#else
            QProcess p;
            QString strq;
            for (int i = 0;i < uri.length();++i) {
                if(uri[i] == ' '){
                    strq += "%20";
                }else{
                    strq += uri[i];
                }
            }

            p.startDetached("/usr/bin/peony", QStringList()<<strq<<"%U&");
#endif
#endif
        } else {
            if (IMAGE_MOUNT_MANAGER->isImageFile(uri)) {
                IMAGE_MOUNT_MANAGER->mountImage(uri);
                return;
            }
            if (!(info->isDesktopFile() && execSharedFileLink(uri))) {
                FileLaunchManager::openAsync(uri, false, false);
            }
        }
        //this->clearSelection();
    });
    job->queryAsync();
}

void FileActionController::openFileByUri(const QStringList &uris)
{
    QStringList dirs;
    QStringList files;
    QStringList isoFiles;
    for (auto uri : uris) {
        auto info = FileInfo::fromUri(uri);
        if (info->isDir() || info->isVolume()) {
            dirs<<uri;
        } else if (IMAGE_MOUNT_MANAGER->isImageFile(uri)) {
            isoFiles << uri;
        } else {
            files<<uri;
        }
    }
    if (!dirs.isEmpty())
        this->openWindow(dirs);

    if (!files.isEmpty()) {
        Peony::FileLaunchManager::openFilesByDefaultApplications(files);
    }

    for (auto iso_uri: isoFiles) {
        IMAGE_MOUNT_MANAGER->mountImage(iso_uri);
    }
}

void FileActionController::openWindow(const QStringList &uris)
{
    QStringList args;
    for (auto arg : uris) {
        QUrl url = arg;
        args<<QString(url.toEncoded());
    }
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/peony");
    p.setArguments(QStringList()<<"--show-folders"<<args);
    p.startDetached();
#else
    p.startDetached("/usr/bin/peony", QStringList()<<"--show-folders"<<args);
#endif
}

bool FileActionController::execSharedFileLink(const QString uri)
{
    auto info = FileInfo::fromUri(uri);
    if (info->isEmptyInfo()) {
        FileInfoJob j(info);
        j.querySync();
    }
    if (uri.endsWith(".desktop")) {
        GKeyFile* key_file = g_key_file_new();
        QUrl url = uri;
        QString desktopfp = url.path();
        g_key_file_load_from_file(key_file, desktopfp.toUtf8().constData(), G_KEY_FILE_KEEP_COMMENTS, nullptr);
        GError* error = NULL;
        if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, "X-Peony-CMD", nullptr)) {
            if (g_key_file_has_key(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, nullptr)) {
                g_autofree char* val = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, &error);
                if (error) {
                    qWarning() << "get desktop file:" << uri << " name error:" << error->code << " -- " << error->message;
                    g_error_free(error);
                    error = nullptr;
                } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                    QProcess p;
                    p.setProgram("peony");
                    QString str = val;
                    str = str.replace("peony ","");
                    p.setArguments(QStringList() << str);
                    qint64 pid;
                    p.startDetached(&pid);
                    // send startinfo to kwindowsystem
                    quint32 timeStamp = QX11Info::isPlatformX11() ? QX11Info::appUserTime() : 0;
                    KStartupInfoId startInfoId;
                    startInfoId.initId(KStartupInfo::createNewStartupIdForTimestamp(timeStamp));
                    startInfoId.setupStartupEnv();
                    KStartupInfoData data;
                    data.setHostname();
                    data.addPid(pid);
#ifdef KSTARTUPINFO_HAS_SET_ICON_GEOMETRY
                    QRect rect = info.get()->property("iconGeometry").toRect();
                    if (rect.isValid()) {
                        data.setIconGeometry(rect);
                    }
#endif
                    data.setLaunchedBy(getpid());
                    KStartupInfo::sendStartup(startInfoId, data);
#else
                    QProcess p;
                    QString strq;
                    for (int i = 0;i < uri.length();++i) {
                        if(uri[i] == ' '){
                            strq += "%20";
                        }else{
                            strq += uri[i];
                        }
                    }
                    p.startDetached("/usr/bin/peony", QStringList()<<strq<<"%U&");
#endif
                    return true;
                }
            }
        }
    }
    return false;
}

bool FileActionController::dragDesktopWidgets() const
{
    return m_dragDesktopWidgets;
}

void FileActionController::setDragDesktopWidgets(bool newDragDesktopWidgets)
{
    if (m_dragDesktopWidgets == newDragDesktopWidgets)
        return;
    m_dragDesktopWidgets = newDragDesktopWidgets;
#ifdef TEST_PEONY_DESKTOP_WIDGETS
    Q_EMIT dragDesktopWidgetsChanged();
#endif
}

void FileActionController::rename(const QStringList& selections, const QString newName)
{
    qDebug() << "rename operation "<<selections<<newName;
    if (selections.count() > 1) {
        auto fileOpMgr = FileOperationManager::getInstance();
        QStringList lists = selections;
        auto renameOp = new FileBatchRenameOperation(lists, newName);
        qApp->setProperty("rename", true);
        //select file when rename finished
        connect(renameOp, &FileBatchRenameOperation::operationFinished, this, [=](){
            auto info = renameOp->getOperationInfo().get();
            auto uri = info->target();
            QTimer::singleShot(100, this, [=](){
                Q_EMIT setSelectedUris(QStringList()<<uri);
            });
        }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    } else if (selections.count() == 1) {
        QStringList strPos;
        QString uri = selections.first();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            strPos = metaInfo->getMetaInfoStringListV1(ITEM_GRID_POS_ATTRIBUTE);
        }
        auto fileOpMgr = FileOperationManager::getInstance();
        auto renameOp = new FileRenameOperation(uri, newName);
        qApp->setProperty("rename", true);
        qApp->setProperty("renamepoint", strPos);
        //select file when rename finished
        connect(renameOp, &FileRenameOperation::operationFinished, this, [=](){
            auto info = renameOp->getOperationInfo().get();
            auto uri = info->target();
            qApp->setProperty("renamepoint", "");
            QTimer::singleShot(100, this, [=](){
                Q_EMIT setSelectedUris(QStringList()<<uri);
            });
        }, Qt::BlockingQueuedConnection);

        fileOpMgr->startOperation(renameOp, true);
    }
}

QIcon FileActionController::icon(const QString &uri)
{
    FileInfo* info = FileInfo::fromUri(uri).get();
    return info->getIcon();
}

void FileActionController::updateFileElideStatus(const QString &uri, bool isElided)
{
    auto info = FileInfo::fromUri(uri);
    if (info) {
        bool _isElided = info->property("isElided").toBool();
        if (_isElided != isElided) {
            info->setProperty("isElided", isElided);
            PeonyDesktopApplication::getModel()->updateTooltips(uri);
        }
    }
}

void FileActionController::setClickedScreenId(const int screenId)
{
    qDebug() << "setClickedScreenId"<<screenId<<m_clickedScreenId;
    if (m_clickedScreenId != screenId) {
        m_clickedScreenId = screenId;
        Q_EMIT clickedScreenIdChanged(m_clickedScreenId);
    }
}

void FileActionController::setFocusedDesktopWidgetInstanceId(int instanceId)
{
    if (m_focusedDesktopWidgetInstanceId == instanceId) {
        return;
    }
    m_focusedDesktopWidgetInstanceId = instanceId;
    Q_EMIT focusedDesktopWidgetInstanceIdChanged();
}

QString FileActionController::getAppId(const QString uri)
{
    auto info = FileInfo::fromUri(uri);
    if(info->isDesktopFile() && nullptr != info->desktopName()) {
        return info->desktopName().remove(".desktop");
    }
    return "";
}

QString FileActionController::getLanguageSpecificSample()
{
    // 获取系统语言环境
    QLocale locale = QLocale::system();
    QLocale::Language language = locale.language();

    // 根据语言和文字系统返回对应的测试文本
    switch (language) {
    case QLocale::Tibetan:
        // 藏文
        return QStringLiteral("ཀཁགངཅཆཇཉཏཐདནཔཕབམཙཚཛཝཞཟའཡརལཤསཧཨ");
    case QLocale::Mongolian:
        // 蒙文
        return QStringLiteral("ᠠᠡᠢᠣᠤᠥᠦᠧᠨᠩᠪᠫᠬᠭᠮᠯᠰᠱᠲᠳᠴᠵᠶᠷᠸᠹᠺᠻᠼᠽᠾᠿᡁᡂᡃᡄᡅᡆᡇᡈᡉᡊᡋᡌᡍᡎᡏᡐᡑᡒᡓᡔᡕᡖᡗᡘᡙᡚᡛᡜᡝᡞᡟᡠᡡᡢᡣᡤᡥᡦᡧᡨᡩᡪᡫᡬᡭᡮᡯᡰᡱᡲᡳ");
    case QLocale::Chinese:
        // 中文
        return QStringLiteral("简体中文示例文字");
    default:
        // 语言样本，包含各种字符
        return QStringLiteral("中ཀᠠTextÀÁРусالعربيةहिन्दी");
    }
}

QSize FileActionController::recalculateGridSize()
{
    QSize newGridSize;
    Peony::GlobalSettings *settings = Peony::GlobalSettings::getInstance();
    if (!m_initialized && settings) {
        QSize sizeFromConfig = settings->getValue(DEFAULT_GRID_SIZE).toSize();
        if (!sizeFromConfig.isEmpty() && sizeFromConfig.width() > MINGRIDSIZE && sizeFromConfig.width() < MAXGRIDSIZE && sizeFromConfig.height() > MINGRIDSIZE && sizeFromConfig.height() < MAXGRIDSIZE) {
            newGridSize = sizeFromConfig;
            return newGridSize;
        }
    }

    newGridSize = QSize(GRIDMARGIN, GRIDMARGIN + ICONMARGIN); // padding: 5
    newGridSize += m_iconSize; // icon size
    newGridSize += QSize(0, ICONMARGIN); // text-icon-padding: 5
    QFontMetrics fm = qApp->fontMetrics();
    //The current height obtained is in english, The Chinese text is approximately 1.5 to 2 times as long
//    newGridSize += QSize(0, fm.height() > 30? fm.height()*2 : fm.height()*2*1.5); // 2 line text height, todo: fix bo_CN,

    /*
     *  在某些字体下中文通过QFontMetrics::height得到的字体高度不准确，
     *  因此对不同语言进行QFontMetrics::boundingRect特殊处理得到其高度,取
     *  QFontMetrics::height与boundingRect.height的最大值
     */
    QString str = getLanguageSpecificSample();
    QRect rect = fm.boundingRect(str);

    qDebug()<<"QFontMetrics::height"<<fm.height()<<"string font boundingRect height"<<rect.height();
    newGridSize += QSize(0, qMax(fm.height() * 2, rect.height() * 2));

    newGridSize.setWidth(newGridSize.width() + 41);

    return newGridSize;
}

void FileActionController::setGridSize(const QSize &newGridSize)
{
    if (m_gridSize != newGridSize) {
        m_gridSize = newGridSize;
        Q_EMIT gridSizeChanged();

        Peony::GlobalSettings *settings = Peony::GlobalSettings::getInstance();
        if (settings) {
            settings->setValue(DEFAULT_GRID_SIZE, m_gridSize);
        }
    }
}
