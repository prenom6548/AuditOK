#ifndef MANAGERCONTROL_H
#define MANAGERCONTROL_H

#include <QObject>
#include <QString>
#include "peony-core_global.h"

namespace Peony {

class PEONYCORESHARED_EXPORT ManagerControl : public QObject
{
    Q_OBJECT

public:
    enum ItemType {
        FileItem,      // 普通文件（除文件夹、应用程序图标外的其他文件）
        FolderItem,    // 文件夹（包括普通目录、computer、trash、home 等特殊图标）
        AppItem        // 应用程序图标（*.desktop应用）
    };

    static ManagerControl* getInstance();

    ItemType getItemType(const QString &uri) const;
    bool isFileItem(const QString &uri) const;
    bool isFolderItem(const QString &uri) const;
    bool isAppItem(const QString &uri) const;

    Q_INVOKABLE bool checkMouseClickPermission(const QString &uri, bool isDoubleClick = false) const;
    Q_INVOKABLE bool checkRightMenuPermission(const QString &uri) const;
    Q_INVOKABLE bool checkBackgroundMenuPermission() const;
    Q_INVOKABLE bool checkSmbSharePermission() const;
    Q_INVOKABLE bool checkCanCreateNewView() const;
    Q_INVOKABLE bool checkShowHiddenPermission() const;
    Q_INVOKABLE bool checkThumbnailPermission() const;
    Q_INVOKABLE bool checkPreviewPanePermission() const;
    Q_INVOKABLE bool checkDeleteToTrashPermission() const;
    Q_INVOKABLE bool checkLocalDiskUnmountPermission() const;
    Q_INVOKABLE bool checkFormatRemovableDevicePermission() const;
    Q_INVOKABLE bool checkOpticalDrivePermission() const;
    Q_INVOKABLE bool checkFileSearchPermission() const;

    // Whitelist protection functions
    Q_INVOKABLE bool isWhitelistAppProtected(const QString &uri) const;
    Q_INVOKABLE bool containsWhitelistApp(const QStringList &uris) const;
    Q_INVOKABLE bool isDesktopDirectory(const QString &directoryUri) const;
    Q_INVOKABLE bool canDeleteItems(const QStringList &uris, const QString &directoryUri) const;

private:
    ManagerControl(QObject *parent = nullptr);
    static ManagerControl* m_instance;
};

}

#define MANAGER_CONTROL Peony::ManagerControl::getInstance()

#endif // MANAGERCONTROL_H
