#include "manager-control.h"
#include "file-info.h"
#include "global-settings.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>

using namespace Peony;

ManagerControl* ManagerControl::m_instance = nullptr;

ManagerControl* ManagerControl::getInstance()
{
    if (!m_instance) {
        m_instance = new ManagerControl();
    }
    return m_instance;
}

ManagerControl::ManagerControl(QObject *parent) : QObject(parent)
{
}

bool ManagerControl::checkSmbSharePermission() const
{
    return GlobalSettings::getInstance()->getValue(ENABLE_SMB_SHARE).toBool();
}

bool ManagerControl::checkCanCreateNewView() const
{
    bool allowed = GlobalSettings::getInstance()->getValue(ENABLE_START_PEONY).toBool();
    
    if (!allowed) {
        qWarning() << "Peony is disabled to start new windows/tabs";
        GlobalSettings::getInstance()->sendNotifyMessage(
            QObject::tr("Peony is disabled to start!")
        );
    }
    
    return allowed;
}

ManagerControl::ItemType ManagerControl::getItemType(const QString &uri) const
{
    if (isAppItem(uri)) {
        return AppItem;
    } else if (isFolderItem(uri)) {
        return FolderItem;
    } else {
        return FileItem;
    }
}

bool ManagerControl::isFileItem(const QString &uri) const
{
    auto info = FileInfo::fromUri(uri);
    if (!info)
        return false;

    static QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (uri == "computer:///" || uri == "trash:///" || uri == homeUri) {
        return false;
    }

    return !info->isDir() && !info->isVolume() && !info->isVirtual() && !info->isDesktopFile();
}

bool ManagerControl::isFolderItem(const QString &uri) const
{
    auto info = FileInfo::fromUri(uri);
    if (!info)
        return false;

    static QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (uri == "computer:///" || uri == "trash:///" || uri == homeUri) {
        return true;
    }

    return info->isDir() && !info->isVolume() && !info->isVirtual();
}

bool ManagerControl::isAppItem(const QString &uri) const
{
    static QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (uri == "computer:///" || uri == "trash:///" || uri == homeUri) {
        return false;
    }

    auto info = FileInfo::fromUri(uri);
    if (!info)
        return false;

    return info->isDesktopFile();
}

bool ManagerControl::checkMouseClickPermission(const QString &uri, bool isDoubleClick) const
{
    auto settings = GlobalSettings::getInstance();

    switch (getItemType(uri)) {
    case FileItem:
        if (isDoubleClick) {
            return settings->getValue(ENABLE_MOUSE_DOUBLE_CLICK_FILE).toBool();
        } else {
            return settings->getValue(ENABLE_MOUSE_CLICK_FILE).toBool();
        }
    case FolderItem:
        if (isDoubleClick) {
            return settings->getValue(ENABLE_MOUSE_DOUBLE_CLICK_FOLDER).toBool();
        } else {
            return settings->getValue(ENABLE_MOUSE_CLICK_FOLDER).toBool();
        }
    case AppItem:
        if (isDoubleClick) {
            bool newConfig = settings->getValue(ENABLE_MOUSE_DOUBLE_CLICK_APP_START).toBool();
            bool oldConfig = settings->getValue(ENABLE_DOUBLE_CLICK_DESKTOP).toBool();
            return newConfig && oldConfig;
        } else {
            return settings->getValue(ENABLE_MOUSE_CLICK_APP_START).toBool();
        }
    }

    return true;
}

bool ManagerControl::checkRightMenuPermission(const QString &uri) const
{
    auto settings = GlobalSettings::getInstance();

    switch (getItemType(uri)) {
    case FileItem:
        return settings->getValue(ENABLE_DESKTOP_FILE_MENU).toBool();
    case FolderItem:
        return settings->getValue(ENABLE_PEONY_RIGHT_MENU).toBool();
    case AppItem:
        return settings->getValue(ENABLE_DESKTOP_APP_START_MENU).toBool();
    }

    return true;
}

bool ManagerControl::checkBackgroundMenuPermission() const
{
    auto settings = GlobalSettings::getInstance();
    return settings->getValue(ENABLE_DESKTOP_BACKGROUND_MENU).toBool();
}

bool ManagerControl::isWhitelistAppProtected(const QString &uri) const
{
    auto settings = GlobalSettings::getInstance();
    QStringList whitelist = settings->getValue(APP_WHITE_LIST).toStringList();

    if (whitelist.isEmpty() || !uri.endsWith(".desktop")) {
        return false;
    }

    auto info = FileInfo::fromUri(uri);
    if (!info || !info->isDesktopFile()) {
        return false;
    }

    QFileInfo fileInfo(uri);
    QString desktopName = fileInfo.fileName();
    return whitelist.contains(desktopName);
}

bool ManagerControl::containsWhitelistApp(const QStringList &uris) const
{
    for (const QString &uri : uris) {
        if (isWhitelistAppProtected(uri)) {
            return true;
        }
    }
    return false;
}

bool ManagerControl::isDesktopDirectory(const QString &directoryUri) const
{
    static QString desktopPath = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    return directoryUri == desktopPath;
}

bool ManagerControl::canDeleteItems(const QStringList &uris, const QString &directoryUri) const
{
    return !(isDesktopDirectory(directoryUri) && containsWhitelistApp(uris));
}

bool ManagerControl::checkShowHiddenPermission() const
{
    return !GlobalSettings::getInstance()->isJsonCacheKey(SHOW_HIDDEN_PREFERENCE);
}

bool ManagerControl::checkThumbnailPermission() const
{
    return !GlobalSettings::getInstance()->isJsonCacheKey(FORBID_THUMBNAIL_IN_VIEW);
}

bool ManagerControl::checkPreviewPanePermission() const
{
    return !GlobalSettings::getInstance()->isJsonCacheKey(ENABLE_FILE_PREVIEW_PANE);
}

bool ManagerControl::checkDeleteToTrashPermission() const
{
    auto settings = GlobalSettings::getInstance();
    if (!settings->isJsonCacheKey(ENABLE_PERMANENT_DELETE)) {
        return true;
    }
    return !settings->getValue(ENABLE_PERMANENT_DELETE).toBool();
}

bool ManagerControl::checkLocalDiskUnmountPermission() const
{
    auto settings = GlobalSettings::getInstance();
    if (!settings->isJsonCacheKey(ENABLE_LOCAL_DISK_UNMOUNT)) {
        return true;
    }
    return settings->getValue(ENABLE_LOCAL_DISK_UNMOUNT).toBool();
}

bool ManagerControl::checkFormatRemovableDevicePermission() const
{
    auto settings = GlobalSettings::getInstance();
    if (!settings->isJsonCacheKey(ENABLE_FORMAT_REMOVABLE_DEVICE)) {
        return true;
    }
    return settings->getValue(ENABLE_FORMAT_REMOVABLE_DEVICE).toBool();
}

bool ManagerControl::checkOpticalDrivePermission() const
{
    auto settings = GlobalSettings::getInstance();
    if (!settings->isJsonCacheKey(ENABLE_OPTICAL_DRIVE)) {
        return true;
    }
    return settings->getValue(ENABLE_OPTICAL_DRIVE).toBool();
}

bool ManagerControl::checkFileSearchPermission() const
{
    auto settings = GlobalSettings::getInstance();
    if (!settings->isJsonCacheKey(ENABLE_FILE_SEARCH)) {
        return true;
    }
    return settings->getValue(ENABLE_FILE_SEARCH).toBool();
}
