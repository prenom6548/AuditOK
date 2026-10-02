/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#ifndef DESKTOPWINDOWMANAGER_H
#define DESKTOPWINDOWMANAGER_H

#include <QObject>
#include <QList>
#include <QScreen>
#include <QMap>
#include <QTimeLine>
#include <QElapsedTimer>
#include <QDBusInterface>
#include <QQuickItem>
#include <QPoint>
#include <widget.h>
class DesktopBackgroundWindow;
class AdvancedDesktopIconView;
class FileActionController;
class GlobalSettingsAdapter;
class BackgroundManager;
class QPoint;
class QQuickView;
class QGSettings;
class PlatformWindow;
class TextureViewer;
class TextureGrabber;
namespace  Peony {
class AdvancedDesktopItemModel;
class DesktopItemProxyModel;
class PeonyDbusService;
}
namespace PeonyDesktop {
class DesktopPartitionDbusService;
}
class DesktopWindowManager : public QObject
{
   Q_OBJECT
   Q_PROPERTY(bool screenLocked READ screenLocked NOTIFY screenLockedChanged)
public:
    explicit DesktopWindowManager(QObject *parent = nullptr);
    ~DesktopWindowManager();

    static DesktopWindowManager *getInstance();
    QPoint postion(const QString &uri);
    QMap<QString, QPoint> getAllItemPos(const QStringList &uris);
    void createIdAndPos(int *id, QPoint *pos);
    void markCreateFilePos(const QPoint &pos);

    void initializeWithScreens();
    bool screenLocked() const { return m_screenLocked; }

public Q_SLOTS:
    void addQmlDesktop(QScreen *screen);
    void switchDisplayMode();
    void singleScreenMode();
    void multiscreenMode();

    void relocateIconView();

    void initModelFinish();
    void raiseWid();
    void layoutViewItems();
    void clearAllRestoreInfo();
    void fileCreated(int id, const QString &uri);
    void setInfoPos(const QString &uri);
    void resetWindowIds(QScreen* oldScreen, int oldId, QScreen* newScreen, int newId);

    Q_INVOKABLE void startInitAnima();
    Q_INVOKABLE void setOpacityToHideIcon();
    void handleLegacyScreenLock();
    void handlePrepareForLock();
    Q_INVOKABLE void requestInitialUnlockPreload();
    Q_INVOKABLE bool getLockState();
    Q_INVOKABLE bool getQuickStartAvailable();
    Q_INVOKABLE void setQAppProperty(const QString &propertyName, const QVariant &value);

    // Texture-based drag preview for cross-screen drag
    Q_INVOKABLE QWindow *createDragPreviewWithTexture(QQuickItem *textureSource, const QPoint &globalPos, int width, int height);
    Q_INVOKABLE void updateDragPreviewPosition(const QPoint &globalPos);
    Q_INVOKABLE void closeDragPreviewWithTexture();
    Q_INVOKABLE QWindow *dragPreviewWindow();
    Q_INVOKABLE bool isInExtendMode();
    Q_INVOKABLE QVariantMap desktopWidgetCrossScreenDragInfo() const;
    // 文件位置变化后检测图标与插件相交，如果相交则执行挤占重排
    Q_INVOKABLE void checkIconWidgetIntersection(int screenId, const QPoint &gridPos);
    void setDesktopWidgetCrossScreenDragInfo(int widgetWidth, int widgetHeight, int pressX, int pressY,
                                             bool layoutWithGrid, int gridUnitSize,
                                             const QString &widgetPayload = QString());
    void clearDesktopWidgetCrossScreenDragInfo();
    /*!
     * \brief initWorkStation
     * 初始化教育工作台的gsettings
     * \param
     * \deprecated
     */
    bool initWorkStation();

    /*!
     * \brief isEDUPlatform
     * 获取是否是教育版本
     * \param
     * \deprecated
     */
    bool isEDUPlatform();

    /*!
     * \brief changeStatusOfWorkStation
     * 根据 gsettings的ENABLE_KEY值，判断是否创建教育工作台
     * \param  gsettings的change 参数 key
     * \deprecated
     */
    bool changeStatusOfWorkStation(const QString &key);

    bool isWorkstationEnabled();
private:
    void registerCppTypeToQml();
    void initializeWindowView(QWindow *window);
    void setupWindowConnections(QWindow *window);
    void initPeonyDbusService();
    void handleScreenLockSignal(const char *source);
    void setScreenLocked(bool locked);
    void startUnlockAnimation(bool fromInitialRecovery);

Q_SIGNALS:
    void emitFinish();
    void setSortType(int sortType);
    void setSortOrder(int sortOrder);
    void modeChanged();
    void startInitAnimaRequest();
    void setOpacityToHideIconRequest();
    void unlockAnimationPreloadRequested(qulonglong generation);
    void unlockAnimationRequested(qulonglong generation);
    void unlockAnimationCancelled(qulonglong generation);
    void screenLockedChanged();
    void initAnima();
    void workstationEnabledChanged(bool enabled);
    void singleScreeModeRequest();
private:
    Peony::AdvancedDesktopItemModel *m_model = nullptr;
    QDBusInterface *m_dbusInterface = nullptr;
    QPoint m_markMousePressPos = QPoint(-1, -1);
    int m_markMousePressId = 0;
    bool m_initMetaInfo = false;
    bool m_desktopAutoLayout = false;
    Peony::PeonyDbusService *m_peonyDbusService = nullptr;
    PeonyDesktop::DesktopPartitionDbusService *m_partitionDbusService = nullptr;
    QTimeLine *m_layoutTimeLine = nullptr;
    QTimeLine *m_modeTimeLine = nullptr;
    bool hasUnlockAnima = false;
    qulonglong m_unlockGeneration = 0;
    bool m_screenLocked = false;
    bool m_initialLockQueryStarted = false;
    bool m_initialUnlockRecoveryTriggered = false;
    bool m_ignoreNextUnlockAfterRecovery = false;
    QElapsedTimer m_initialLockQueryTimer;
    bool isQuickStartAvailable = false;
    bool m_workStationEnable = true;
    bool m_isEDUPlat = false;
    QGSettings *m_workStationSetting = nullptr;

    // Texture-based drag preview members
    class TextureViewer *m_dragTextureViewer = nullptr;
    class TextureGrabber *m_dragTextureGrabber = nullptr;
    bool m_dragTexturePreviewCreated = false;

    int m_crossScreenWidgetWidth = 0;
    int m_crossScreenWidgetHeight = 0;
    int m_crossScreenPressX = 0;
    int m_crossScreenPressY = 0;
    bool m_crossScreenLayoutWithGrid = false;
    int m_crossScreenGridUnitSize = 60;
    QString m_crossScreenWidgetPayload;
    bool m_crossScreenDragInfoActive = false;
};

#include <QObject>
#include <QQmlEngine>
#include "desktop-item-proxy-model.h"
#include <gio/gio.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
class DesktopOrganizationManager : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged) // 示例属性
public:
    static DesktopOrganizationManager* instance();
    bool isOriganizationToolConfigExist();
    void setConfigStatus(bool isExits);
    void RestoreDesktopInfo();
    void CreatePartition(QStringList uris);
    void doSort(int instanceId,int type,int order);
    void disbandPartition(int instanceId);
    void initOrganizationWidgets();
    void addDesktopOrganizationWidget(PlatformWindow *targetWindow, Peony::DesktopItemProxyModel::OriganizationType type, const QString &widgetName, const QStringList &fileUris);
    void setDesktopOrganizationWidgetEnabled(PlatformWindow *targetWindow, bool enabled);
    QStringList getUrisByOrganizationType(int type,int screenId);
public Q_SLOTS:
    Q_INVOKABLE void renameSpace(int instanceId,QString name);
    Q_INVOKABLE void cancelEdit();
    Q_INVOKABLE void showMenuRequest(const QPoint &globalPos,int Instanceid,bool isShowByMenu = true);
    Q_INVOKABLE void addFileFromDesktopToOrgTool(QStringList urilist, int instanceId);
    Q_INVOKABLE bool getIsListMode(int instanceid);
    Q_INVOKABLE void expandStatusChanged(int instanceId,bool expand,int initheight);
    Q_INVOKABLE bool isExpand(int instanceId);
    Q_INVOKABLE int  getInitHeight(int instanceId);
    Q_INVOKABLE void setInitHeight(int instanceId,int height);
    Q_INVOKABLE void updateOrganizationView(int instanceId);
    Q_INVOKABLE void renameByorgTool();
    Q_INVOKABLE void createNewFile(int instanceId, const QString &fileType = QString());
    Q_INVOKABLE void createNewFileFromTemplate(int instanceId, const QString &templateName);
    Q_INVOKABLE void pasteFiles(int instanceId);
    Q_INVOKABLE void setGirdSizeByModel(QAbstractItemModel *model, int instanceId);
    bool isManulCreatePartition(int instanceid);
Q_SIGNALS:
    void renameFocusRequest(int instanceId,int clickposx);
    void renameRequest(int instanceId,QString name,bool needfocus);
    void cancelEditRequest();
    void viewModelChanged(int instanceId,bool flag);
    void updateDesktopView();
    void expandStatusChangedRequest(int instanceId,bool expand);
    void selectAndJumptoUriRequest(int instanceId,QString uri);
    void expandChanged();
    void updateInitHeight(int instanceid,int height);
    void createFileAndEditRequest(int instanceId, const QString &uri);

    // 和model通信
    void requestSort(int instanceId, int type,int order);
    void requestDisbandPartation(int instanceId);
    void requestUpdateOrganizationView(int instanceId);
    void mergeSameTypeWidgetToPrimaryScreenRequest(int frominstanceid,int toinstanceid);
    void restoreSameTypeWidgetToNewScreenRequest(int frominstanceid,int toinstanceid);
    void addinitWidgtRequest();
    void checkorganizationExistRequest();
private:
    explicit DesktopOrganizationManager(QObject* parent = nullptr) : QObject(parent) {}
    void initOrganizationWidgetsForWindow(PlatformWindow *window);
    // bool m_isConfigExist = false;
    // QString m_organizationConfigPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/org.ukui/peony-qt-desktop/desktop-view.json";
    QString  m_directryUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    Q_DISABLE_COPY(DesktopOrganizationManager)
};

#define DESKTOP_MANAGER DesktopWindowManager::getInstance()

#include <widget-container-item.h>
#include "widgets-proxy-model.h"
class DesktopUKUIpluginnManager : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged) // 示例属性
public:
    static DesktopUKUIpluginnManager* instance();

public Q_SLOTS:
    Q_INVOKABLE void setGirdSize(int col, int row, int instanceId);
    Q_INVOKABLE bool isNeedInit(int instanceid);
    int minSize(UkuiQuick::WidgetItem *widgetItem);
Q_SIGNALS:

private:

};


#endif // DESKTOPWINDOWMANAGER_H
