/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "ukui-task-manager.h"
#include <QVector>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include <QDir>
#include <QFile>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QTranslator>
#include <QScreen>
#include <QJsonObject>
#include <QJsonArray>

#include <qglobal.h>
#include "task-manager-item.h"
#include "recent-files-service.h"
#include "taskmanageradaptor.h"
#include "utils.h"
#include <config-loader.h>
#include <function-control.h>
#include <window-manager.h>
#include "actions.h"
#include "config.h"
#include "taskmanageradaptor.h"
#include <kysdk/diagnosetest/libkydatacollect.h>

static const QString QUICK_LAUNCHERS_KEY("quickLaunchers");


static const QString NOTIFICATION_SERVICE = QStringLiteral("org.freedesktop.Notifications");
static const QString NOTIFICATION_PATH = QStringLiteral("/org/freedesktop/Notifications");
static const QString NOTIFICATION_INTERFACE = QStringLiteral("org.ukui.NotificationServer");
static const QStringList DEFAULT_QUICK_LAUNCHERS = {"peony",
                                                    "firefox",
                                                    "kylin-software-center",
                                                    "kylin-screenshot",
                                                    "ukui-control-center",
                                                    };
/**
 * 1.0 QUICK_LAUNCHERS_KEY 值从desktop文件全路径改为desktop文件名列表（不包含后缀）
 */
static const QString CONFIG_VERSION = QStringLiteral("1.0");

namespace TaskManager
{
using WindowManager = UkuiQuick::WindowManager;

bool compareListBySort(QStringList &list1, QStringList &list2) {
    if (list1.size() != list2.size())
        return false;

    list1.sort();
    list2.sort();
    return list1 == list2;
}

QString appIdFromDesktopFile(const QString &desktopFile)
{
    QFileInfo fileInfo(desktopFile);
    auto fileName = fileInfo.fileName();
    if (fileName.endsWith(QStringLiteral(".desktop"))) {
        return fileName.left(fileName.length() - 8);
    }
    return {};
}

class Q_DECL_HIDDEN UkuiTaskManager::Private
{
public:
    explicit Private(UkuiTaskManager *q = nullptr);
    ~Private();

    void windowAdded(const QString &windowId);
    void windowRemoved(const QString &windowId);
    void skipTaskbarChanged(const QString& windowId);
    void activeWindowChanged(const QString& windowId);
    void loadSettings();
    void addQuickLauncherItem(const QString &desktopFile);
    void save() const;
    void loadWindows();
    void newItem(const QString &wid, const QString &desktopFile);
    void onNotificationServiceChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void connectToNotificationServer();

    QVector<TaskManagerItem *> m_items;

    UkuiQuick::Config *m_config {nullptr};
    QStringList m_quickLaunchersOrder;

    uint m_groupID = 0; //当窗口的group为空时使用
    QTranslator *m_translator = nullptr;
    QDBusServiceWatcher *m_watcher = nullptr;
    bool m_connectedToNotificationServer = false;
    QString m_activeWindow;
    TaskManager::Actions m_windowActions;
    RecentFilesService *m_recentFilesService = nullptr;
    bool m_requestDataChange {false};

    //管控配置
    QJsonObject m_controlConfig;
    bool m_disableMouseLeftButton {false};
    bool m_disableMouseRightButton {false};
    bool m_disableMouseHover {false};
    int m_mergeIcons {0};
    QStringList m_appWhiteList;
    QStringList m_appBlackList;
    // 控制是否允许拖拽到应用图标，激活应用窗口功能（default: true）
    bool m_enableDragToApp {true};
    int m_taskbarIconsShowedOn {0};

private:
    UkuiTaskManager *q = nullptr;
};

UkuiTaskManager::Private::Private(UkuiTaskManager *q) : q(q)
{
}

UkuiTaskManager::Private::~Private()
{
    qDeleteAll(m_items);
    qDeleteAll(m_windowActions);
}

void UkuiTaskManager::Private::loadSettings()
{
    if (!m_config) {
        m_config = UkuiQuick::ConfigLoader::getConfig("org.ukui.panel.taskManager");
    }

    //版本升级处理
    if (m_config->getValue(QStringLiteral("version")).toString() != CONFIG_VERSION) {
        auto oldVersion = m_config->getValue(QStringLiteral("version")).toString();
        if (oldVersion.isEmpty() && m_config->contains(QUICK_LAUNCHERS_KEY)) {
            auto list = m_config->getValue(QUICK_LAUNCHERS_KEY).toStringList();
            QStringList newList;
            QFileInfo info;
            for (const auto&desktop: list) {
                info.setFile(desktop);
                QString filName = info.fileName();
                if (filName.endsWith(QStringLiteral(".desktop"))) {
                    newList.append(filName.left(filName.length() - 8));
                }
            }
            m_config->setValue(QUICK_LAUNCHERS_KEY, newList);
        }
        m_config->setValue(QStringLiteral("version"), CONFIG_VERSION);
    }

    QStringList quickLaunchers;
    if (m_config->contains(QUICK_LAUNCHERS_KEY)) {
        quickLaunchers = m_config->getValue(QUICK_LAUNCHERS_KEY).toStringList();
    } else {
        quickLaunchers = DEFAULT_QUICK_LAUNCHERS;
    }
    for (auto id : quickLaunchers) {
        auto fullPath = Utils::desktopFilePathFromName(id);
        if (!fullPath.isEmpty()) {
            m_quickLaunchersOrder.append(id);
            addQuickLauncherItem(fullPath);
        }
    }
    if (quickLaunchers != m_quickLaunchersOrder) {
        m_config->setValue(QUICK_LAUNCHERS_KEY, m_quickLaunchersOrder);
    }
    //mergeIcons
    if (m_config->data().contains("mergeIcons")) {
        m_mergeIcons = m_config->getValue("mergeIcons").value<MergeIcons>();
    } else {
        m_mergeIcons = MergeIcons::Always;
        m_config->setValue(QStringLiteral("mergeIcons"), MergeIcons::Always);
    }
    //taskBarIconsShowedOn
    if (m_config->data().contains("taskBarIconsShowedOn")) {
        m_taskbarIconsShowedOn = m_config->getValue("taskBarIconsShowedOn").value<TaskBarIconsShowedOn>();
    } else {
        m_taskbarIconsShowedOn = TaskBarIconsShowedOn::AllPanels;
        m_config->setValue(QStringLiteral("taskBarIconsShowedOn"), TaskBarIconsShowedOn::AllPanels);
    }
}

void UkuiTaskManager::Private::addQuickLauncherItem(const QString &desktopFile)
{
    qDebug() << "Init quickLauncher button from desktop:" << desktopFile;
    auto item = new TaskManagerItem(desktopFile, m_recentFilesService);
    connect(item, &TaskManagerItem::dataUpdated, q, [&, item](const QVector<int> &roles){
        QModelIndex changeIndex = q->index(m_items.indexOf(item), 0, QModelIndex());
        q->dataChanged(changeIndex, changeIndex, roles);
    });
    connect(q, &UkuiTaskManager::unReadMessagesNumberUpdate, item, &TaskManagerItem::updateUnreadMessagesNum);
    item->setHasLauncher(true);
    item->init();
    q->beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    q->endInsertRows();
}

void UkuiTaskManager::Private::windowAdded(const QString &windowId)
{
    //过滤不需要显示的窗口
    if(WindowManager::skipTaskBar(windowId)) {
        return;
    }
    QString windowGroup = WindowManager::windowGroup(windowId);
    if (windowGroup.isEmpty()) {
        windowGroup = QString::number(++m_groupID);
    }
    QString desktopFile =  Utils::desktopFileFromWid(windowId);
    auto appid = appIdFromDesktopFile(desktopFile);

    for (auto item : m_items) {
        //避免重复添加
        if(item->winIDs().contains(windowId)) {
            return;
        }

        //组已经存在，增加一个窗口（windowGroup就是appid）
        if (item->group() == windowGroup) {
            if (appid.isEmpty() || item->ID() == appid) {
                item->addWinID(windowId);
                return;
            }
        }

        //desktop匹配，增加一个窗口
        if (!item->ID().isEmpty() && !appid.isEmpty() && item->ID() == appid) {
            item->addWinID(windowId);
            return;
        }
    }
    newItem(windowId, desktopFile);
}

void UkuiTaskManager::Private::windowRemoved(const QString &windowId)
{
    for (int index = 0; index < m_items.size(); ++index) {
        auto item = m_items.at(index);
        if (item->winIDs().contains(windowId)) {
            item->removeWinID(windowId);
            if (item->winIDs().isEmpty() && !item->hasLauncher()) {
                q->beginRemoveRows(QModelIndex(), index, index);
                delete m_items.takeAt(index);
                q->endRemoveRows();
            }
            break;
        }
    }
}

void UkuiTaskManager::Private::skipTaskbarChanged(const QString &windowId)
{
    bool skipTaskBar = WindowManager::skipTaskBar(windowId);
    QString windowGroup = WindowManager::windowGroup(windowId);
    QString desktopFile =  Utils::desktopFileFromWid(windowId);
    auto appid = appIdFromDesktopFile(desktopFile);
    bool handled = false;
    for (int index = 0; index < m_items.size(); ++index) {
        auto item = m_items.at(index);
        if (item->winIDs().contains(windowId)) {
            if(skipTaskBar) {
                item->removeWinID(windowId);
                if (item->winIDs().isEmpty() && !item->hasLauncher()) {
                    q->beginRemoveRows(QModelIndex(), index, index);
                    delete m_items.takeAt(index);
                    q->endRemoveRows();
                }
            }
            handled = true;
            break;
        } else if(item->group() == windowGroup && !windowGroup.isEmpty() && !skipTaskBar) {
            if (appid.isEmpty() || item->ID() == appid) {
                item->addWinID(windowId);
                handled = true;
                break;
            }
        }
    }
    if(!handled) {
        if(!skipTaskBar) {
            newItem(windowId, desktopFile);
        }
    }
}

void UkuiTaskManager::Private::activeWindowChanged(const QString &windowId)
{
    if(windowId == m_activeWindow) {
        return;
    }
    for(auto item : m_items) {
        if(item->currentDesktopWinIDs().contains(m_activeWindow)) {
            item->setHasActiveWindow(false);
        }
        if(item->currentDesktopWinIDs().contains(windowId)) {
            item->setHasActiveWindow(true);
        }
    }
    m_activeWindow = windowId;
    Q_EMIT q->currentActiveWinIdChanged();
}

void UkuiTaskManager::Private::loadWindows()
{
    for (const QString &winID: WindowManager::windows()) {
        windowAdded(winID);
    }
}

void UkuiTaskManager::Private::newItem(const QString &wid, const QString &desktopFile)
{
    auto appid = appIdFromDesktopFile(desktopFile);
    //查看是否存在快速启动按钮
    if (m_quickLaunchersOrder.contains(appid)) {
        m_items.at(q->indexOf(appid).row())->addWinID(wid);
        return;
    }

    //新增一个item
    auto newItem = new TaskManagerItem(QStringList{wid}, m_recentFilesService);
    newItem->setID(appid);
    newItem->setDesktopFile(desktopFile);
    newItem->init();

    connect(q, &UkuiTaskManager::unReadMessagesNumberUpdate, newItem, &TaskManagerItem::updateUnreadMessagesNum);

    QObject::connect(newItem, &TaskManagerItem::dataUpdated, q, [&, newItem](const QVector<int> &roles){
        QModelIndex changeIndex = q->index(m_items.indexOf(newItem), 0, QModelIndex());
        q->dataChanged(changeIndex, changeIndex, roles);
    });
    q->beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(newItem);
    q->endInsertRows();
}

void UkuiTaskManager::Private::save() const
{
    m_config->setValue(QUICK_LAUNCHERS_KEY, m_quickLaunchersOrder);
    m_config->forceSync();
}

void UkuiTaskManager::Private::onNotificationServiceChanged(const QString &service, const QString &oldOwner,
                                                            const QString &newOwner)
{
    if(!oldOwner.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(NOTIFICATION_SERVICE,
                                                 NOTIFICATION_PATH,
                                                 NOTIFICATION_INTERFACE,
                                                 QStringLiteral("UnreadMessagesNumberUpdated"),
                                                 q,
                                                 SLOT(unReadMessagesNumberUpdate));
        m_connectedToNotificationServer = false;
        if(!newOwner.isEmpty()) {
            connectToNotificationServer();
        }
    } else if(!newOwner.isEmpty()) {
        if(!m_connectedToNotificationServer) {
            return;
        }
        connectToNotificationServer();
    }
}

void UkuiTaskManager::Private::connectToNotificationServer()
{
    m_connectedToNotificationServer = QDBusConnection::sessionBus().connect(NOTIFICATION_SERVICE,
                                                                            NOTIFICATION_PATH,
                                                                            NOTIFICATION_INTERFACE,
                                                                            QStringLiteral("UnreadMessagesNumberUpdated"),
                                                                            q,
                                                                            SLOT(unReadMessagesNumberUpdate(const QString &, uint)));
}

QModelIndex UkuiTaskManager::indexOf(const QString &appId)
{
    for(int row = 0; row < d->m_items.size(); ++row) {
        if(d->m_items.at(row)->ID() == appId) {
            return index(row, 0, {});
        }
    }
    return {};
}

QString UkuiTaskManager::currentActiveWinId() const
{
    return d->m_activeWindow;
}

UkuiTaskManager::UkuiTaskManager(QObject *parent) : QAbstractListModel(parent), d(new Private(this))
{
    d->m_recentFilesService = new RecentFilesService(this);
    d->m_translator = new QTranslator(this);
    if (!d->m_translator->load(QString(TRANSLATION_FILE_DIR) + "/libukui-task-manager_" + QLocale::system().name() + ".qm")) {
        qWarning() << "Load translations file" << QLocale::system().name() << "failed!";;
    }
    qApp->installTranslator(d->m_translator);
    //加载配置文件，初始化快速启动按钮，初始化mergeIcons,taskBarIconsShowedOn
    d->loadSettings();
    //加载当前打开的窗口
    d->loadWindows();
    //当taskBarIconsShowedOn变化时，更新筛选条件,并发送事件，更新数据
    connect(d->m_config, &UkuiQuick::Config::configChanged, this, [&](QString key) {
        if (key == "mergeIcons") {
            d->m_mergeIcons = d->m_config->getValue(key).value<MergeIcons>();
            Q_EMIT mergeIconsChanged();
        }
        if (key == "taskBarIconsShowedOn") {
            d->m_taskbarIconsShowedOn = d->m_config->getValue(key).value<TaskBarIconsShowedOn>();
            Q_EMIT taskbarIconsShowedOnChanged();
        }
    });
    connect(Utils::getApplicationInfo(), &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &UkuiTaskManager::onAppUninstalled);
    connect(WindowManager::self(), &WindowManager::windowAdded, this, [&](const QString &winId) {
        d->windowAdded(winId);
    });
    connect(WindowManager::self(), &WindowManager::windowRemoved, this, [&](const QString &winId) {
        d->windowRemoved(winId);
    });
    connect(WindowManager::self(), &WindowManager::skipTaskbarChanged, this, [&](const QString &winId) {
        d->skipTaskbarChanged(winId);
    });
    connect(WindowManager::self(), &WindowManager::activeWindowChanged, this, [&](const QString &winId){
        d->activeWindowChanged(winId);
    });

    //为了避免UI阻塞 bug#464235
    QTimer::singleShot(0, [this](){
        d->m_activeWindow = WindowManager::currentActiveWindow();
        Q_EMIT currentActiveWinIdChanged();
    });

    auto conn = QDBusConnection::sessionBus();
    d->m_watcher = new QDBusServiceWatcher(QStringLiteral("org.freedesktop.Notifications"),
                                           conn,
                                           QDBusServiceWatcher::WatchForOwnerChange,
                                           this);
    connect(d->m_watcher, &QDBusServiceWatcher::serviceOwnerChanged,this,[&](const QString &service, const QString &oldOwner, const QString &newOwner){
        d->onNotificationServiceChanged(service, oldOwner, newOwner);
            });
    d->connectToNotificationServer();
    new TaskManagerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/taskManager"), this);
    dbus.registerService(QStringLiteral("org.ukui.taskManager"));

    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-taskManager-v11"));
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [&](const QString &name) {
        onFunctionControlChanged(name);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [&]() {
       onFunctionControlChanged(QStringLiteral("ukui-panel-taskManager-v11"), true);
    });
    onFunctionControlChanged(QStringLiteral("ukui-panel-taskManager-v11"));
}

UkuiTaskManager::~UkuiTaskManager()
{
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-taskManager-v11"));
    if (d) {
        delete d;
        d = nullptr;
    }
}

QHash<int, QByteArray> UkuiTaskManager::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

QVariant UkuiTaskManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->m_items.count()) {
        return {};
    }
    TaskManagerItem *item = d->m_items.at(index.row());

    if (!item) {
        return {};
    }
    switch (role) {
        case Qt::DisplayRole:
            return item->name();
        case Icon: {
            if (item->icon().isNull()) {
                return "application-x-desktop";
            }
            return item->icon();
        }
        case IconName:
            return item->iconName();
        case Id:
            return item->ID();
        case DesktopFile:
            return item->desktopFile();
        case Name:
            return item->name();
        case GenericName:
            return item->genericName();
        case WinIdList:
            return item->winIDs();
        case CurrentDesktopWinIdList:
            return item->currentDesktopWinIDs();
        case WindowGeometries:
            return QVariant::fromValue(item->WindowGeometries());
        case WindowTitles:
            return QVariant::fromValue(item->windowTitles());
        case WindowIcons:
            return QVariant::fromValue(item->windowIcons());
        case WindowIconsName:
            return QVariant::fromValue(item->windowIconsName());
        case DemandsAttentionWinIdList:
            return item->demandsAttentionWinIDs();
        case ApplicationMenuServiceName:
        case ApplicationMenuObjectPath:
            return QString();
        case HasLauncher:
            return item->hasLauncher();
        case Actions: {
            auto actions = item->actions();
            for(auto action : actions) {
                if(action->type() == Action::Type::AddQuickLauncher) {
                    action->setParam(index.row());
                    break;
                }
            }
            return QVariant::fromValue(actions);
        }

        case UnreadMessagesNum:
            return item->unReadMessagesNum();
        case HasActiveWindow:
            return item->hasActiveWindow();
        case QuickWidgetId:
            return item->quickWidget();
        default:
            return {};
    }
    Q_UNREACHABLE();
}

QModelIndex UkuiTaskManager::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return {};
    }
    return createIndex(row, column, nullptr);
}

int UkuiTaskManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->m_items.count();
}

TaskManager::Actions UkuiTaskManager::menuActions(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= d->m_items.count()) {
        return {};
    }

    auto *item = d->m_items.at(index.row());
    if (!item) {
        return {};
    }

    item->prepareActionsForMenu();

    auto actions = item->actions();
    for (auto action : actions) {
        if (action->type() == Action::Type::AddQuickLauncher) {
            action->setParam(index.row());
            break;
        }
    }

    return actions;
}

bool UkuiTaskManager::addQuickLauncher(const QString &desktopFile)
{
    //支持传入全路径，appId后带desktop后缀的appId
    QString fullpath = standardizePath(desktopFile);
    if (fullpath.isEmpty()) {
        return false;
    }

    const auto appId = appIdFromDesktopFile(fullpath);
    if (appId.isEmpty() || d->m_quickLaunchersOrder.contains(appId)) {
        return false;
    }

    QModelIndex index = indexOf(fullpath);
    if (index.isValid()) {
        addQuickLauncher(fullpath, index.row());
    } else {
        addQuickLauncher(fullpath, d->m_items.size());
    }

    return true;
}

bool UkuiTaskManager::checkQuickLauncher(const QString &desktopFile)
{
    if (std::any_of(d->m_quickLaunchersOrder.begin(), d->m_quickLaunchersOrder.end(), [&](const QString& id) {
        if (desktopFile.endsWith("/" + id + ".desktop")) {
            return true;
        }
        return false;
    })) {
        return true;
    }
    return false;
}

bool UkuiTaskManager::removeQuickLauncher(const QString &desktopFile)
{
    QString appId;
    if (desktopFile.endsWith(QStringLiteral("desktop"))) {
        appId = appIdFromDesktopFile(desktopFile);
    } else {
        appId = desktopFile;
    }

    if (appId.isEmpty()) {
        return false;
    }
    if (d->m_quickLaunchersOrder.contains(appId)) {
        int modelIndex = indexOf(appId).row();
        auto targetItem = d->m_items.at(modelIndex);
        int oldOrder = d->m_quickLaunchersOrder.indexOf(appId);
        //取消固定
        targetItem->setHasLauncher(false);
        //上传埋点数据
        addQuickLauncherEvent(appId, "unpin from taskbar");
        if (targetItem->winIDs().isEmpty()) {
            beginRemoveRows(QModelIndex(), modelIndex, modelIndex);
            delete d->m_items.takeAt(modelIndex);
            endRemoveRows();
        }
        d->m_quickLaunchersOrder.removeAt(oldOrder);
        d->save();
    } else {
        return false;
    }
    return true;
}

QStringList UkuiTaskManager::quickLaunchers() const
{
    return d->m_quickLaunchersOrder;
}

QString UkuiTaskManager::standardizePath(const QString &path)
{
    QString fullpath = path;
    if (!QFile::exists(path)) {
        if (path.endsWith(QStringLiteral("desktop"))) {
            fullpath = Utils::desktopFilePathFromName(path.left(path.length() - 8));
        } else {
            fullpath = Utils::desktopFilePathFromName(path);
        }
        if (!QFile::exists(fullpath)) {
            return {};
        }
    } else if (!path.endsWith(QStringLiteral("desktop"))) {
        return {};
    }
    return fullpath;
}

void UkuiTaskManager::addQuickLauncher(const QString &desktopFile, int order)
{
    if(order < 0 || !QFile::exists(desktopFile)) {
        return;
    }
    auto appId = appIdFromDesktopFile(desktopFile);

    // 1. 任务栏已存在图标
    // 1.1 改变HasLauncher属性,并同步配置文件
    for (int i = 0; i < d->m_items.size(); ++i) {
        if (d->m_items.at(i)->ID() == appId) {
            if (!d->m_items.at(i)->hasLauncher()) {
                d->m_items.at(i)->setHasLauncher(true);
                auto launcherOrder = qMin(quickLauncherCountBeforeRow(i), d->m_quickLaunchersOrder.size());
                d->m_quickLaunchersOrder.insert(launcherOrder, appId);
                d->save();
            }
            // 1.2 移动到指定位置
            setOrder(indexOf(appId), order);
            addQuickLauncherEvent(appId, "pin to taskbar");
            return;
        }
    }

    // 2. 任务栏不存在图标
    // 2.1. 新增图标
    auto item = new TaskManagerItem(desktopFile, d->m_recentFilesService);
    item->setHasLauncher(true);
    connect(this, &UkuiTaskManager::unReadMessagesNumberUpdate, item, &TaskManagerItem::updateUnreadMessagesNum);
    QObject::connect(item, &TaskManagerItem::dataUpdated, this, [&, item](const QVector<int>&roles) {
        QModelIndex changeIndex = index(d->m_items.indexOf(item), 0, QModelIndex());
        Q_EMIT dataChanged(changeIndex, changeIndex, roles);
    });
    item->init();

    order = qMin(order, d->m_items.size());
    beginInsertRows(QModelIndex(), order, order);
    d->m_items.insert(order, item);
    endInsertRows();

    // 2.2 记录快捷启动按钮顺序，并同步配置文件
    auto launcherOrder = quickLauncherCountBeforeRow(order);
    launcherOrder = qMin(launcherOrder, d->m_quickLaunchersOrder.size());
    d->m_quickLaunchersOrder.insert(launcherOrder, appId);
    d->save();
    // 2.3 上传埋点数据
    addQuickLauncherEvent(appId, "pin to taskbar");
}

void UkuiTaskManager::setOrder(const QModelIndex &index, int order)
{
    int oldOrder = index.row();
    order = qMin(order, d->m_items.size() -1);
    if(order == oldOrder || !index.isValid() || oldOrder >= d->m_items.size()) {
        return;
    }
    auto item = d->m_items.at(oldOrder);
    if(item->hasLauncher()) {
        int launcherOrder = d->m_quickLaunchersOrder.indexOf(item->ID());
        if(oldOrder < order) {
            for(int row = oldOrder + 1; row <= order; ++row) {
                if(d->m_items.at(row)->hasLauncher()) {
                    ++launcherOrder;
                }
            }
        } else {
            for(int row = oldOrder - 1; row >= order; --row) {
                if(d->m_items.at(row)->hasLauncher()) {
                    --launcherOrder;
                }
            }

            if (launcherOrder < 0) {
                launcherOrder = 0;
            }
        }
        launcherOrder = qMin(d->m_quickLaunchersOrder.size() - 1, launcherOrder);
        d->m_quickLaunchersOrder.move(d->m_quickLaunchersOrder.indexOf(item->ID()), launcherOrder);
        d->save();
    }

    if (oldOrder < order) {
        beginMoveRows(index.parent(), oldOrder, oldOrder, index.parent(), order + 1);
    } else {
        beginMoveRows(index.parent(), oldOrder, oldOrder, index.parent(), order);
    }
    d->m_items.insert(order, d->m_items.takeAt(oldOrder));
    endMoveRows();
}

void UkuiTaskManager::activateWindowView(const QStringList &winIds)
{
    WindowManager::self()->activateWindowView(winIds);
}

void UkuiTaskManager::activateWindow(const QString &winId)
{
    WindowManager::activateWindow(winId);
}

QString UkuiTaskManager::windowTitle(const QString &winId)
{
    return WindowManager::windowTitle(winId);
}

QIcon UkuiTaskManager::windowIcon(const QString &winId)
{
    return WindowManager::windowIcon(winId);
}

UkuiTaskManager &UkuiTaskManager::self()
{
    static UkuiTaskManager s_self;
    return s_self;
}

TaskManager::Actions UkuiTaskManager::windowActions(const QString &winId)
{
    if(d->m_windowActions.isEmpty()) {
        //关闭
        auto action = new Action("Close", tr("Close"), QIcon::fromTheme("window-close-symbolic"), Action::Type::Close, winId);
        connect(action, &Action::actionTriggered, this, &UkuiTaskManager::closeWindow);
        d->m_windowActions.append(action);
        //还原
        action = new Action("Restore", tr("Restore"), QIcon::fromTheme("window-restore-symbolic"), Action::Type::Restore, winId);
        connect(action, &Action::actionTriggered, this, &UkuiTaskManager::restoreWindow);
        d->m_windowActions.append(action);
        //最大化
        action = new Action("Maximize", tr("Maximize"), QIcon::fromTheme("window-maximize-symbolic"), Action::Type::Maximize, winId);
        connect(action, &Action::actionTriggered, this, &UkuiTaskManager::maximizeWindow);
        d->m_windowActions.append(action);
        //最小化
        action = new Action("Minimize", tr("Minimize"), QIcon::fromTheme("window-minimize-symbolic"), Action::Type::Minimize, winId);
        connect(action, &Action::actionTriggered, this, &UkuiTaskManager::minimizeWindow);
        d->m_windowActions.append(action);
        //置顶
        action = new Action("Keep above", tr("Keep above"), QIcon::fromTheme("ukui-fixed-symbolic"), Action::Type::KeepAbove, winId);
        connect(action, &Action::actionTriggered, this, &UkuiTaskManager::keepAbove);
        d->m_windowActions.append(action);
        //取消置顶
        action = new Action("Unset keep above", tr("Unset keep above"), QIcon::fromTheme("ukui-unfixed-symbolic"), Action::Type::UnsetKeepAbove, winId);
        connect(action, &Action::actionTriggered, this, &UkuiTaskManager::unsetKeepAbove);
        d->m_windowActions.append(action);
    }
    for (auto action : d->m_windowActions) {
        if (action->param() != winId) {
            action->setParam(winId);
        }

        switch (action->type()) {
            case Action::Close:
                break;
            case Action::Minimize:
                action->setEnabled(WindowManager::isMinimizable(winId) && !WindowManager::isMinimized(winId));
                break;
            case Action::Maximize:
                action->setEnabled(WindowManager::isMaximizable(winId) && (!WindowManager::isMaximized(winId) || WindowManager::isMinimized(winId)));
                break;
            case Action::Restore:
                action->setEnabled(WindowManager::isMaximized(winId) || WindowManager::isMinimized(winId));
                break;
            case Action::KeepAbove:
                action->setEnabled(!WindowManager::isKeepAbove(winId));
                break;
            case Action::UnsetKeepAbove:
                action->setEnabled(WindowManager::isKeepAbove(winId));
                break;
            default:
                break;
        }
    }
    return d->m_windowActions;
}

bool UkuiTaskManager::windowIsActivated(const QString &winId)
{
    return winId == WindowManager::currentActiveWindow();
}

void UkuiTaskManager::execSpecifiedAction(const TaskManager::Action::Type &type, const QString &wid)
{
    switch (type) {
        case Action::Close:
            WindowManager::closeWindow(wid);
            break;
        case Action::Minimize:
            WindowManager::minimizeWindow(wid);
            break;
        case Action::Maximize:
            WindowManager::maximizeWindow(wid);
            break;
        case Action::Restore:
            WindowManager::restoreWindow(wid);
            break;
        case Action::KeepAbove:
        case Action::UnsetKeepAbove:
            WindowManager::keepAboveWindow(wid);
            break;
        default:
            break;
    }
}

void UkuiTaskManager::launch(const QModelIndex& index, const QQuickItem* item)
{
    if (!index.isValid() || index.row() >= d->m_items.count()) {
        return;
    }
    d->m_items.at(index.row())->launch(item);
}

void UkuiTaskManager::closeWindow(const QVariant &wid)
{
    WindowManager::closeWindow(wid.toString());
}

void UkuiTaskManager::minimizeWindow(const QVariant &wid)
{
    WindowManager::minimizeWindow(wid.toString());
}

void UkuiTaskManager::maximizeWindow(const QVariant &wid)
{
    if (!WindowManager::isMaximized(wid.toString())) {
        WindowManager::maximizeWindow(wid.toString());
    }
    WindowManager::activateWindow(wid.toString());
}

void UkuiTaskManager::restoreWindow(const QVariant &wid)
{
    WindowManager::restoreWindow(wid.toString());
    WindowManager::activateWindow(wid.toString());
}

void UkuiTaskManager::keepAbove(const QVariant &wid)
{
    WindowManager::keepAboveWindow(wid.toString());
}

void UkuiTaskManager::unsetKeepAbove(const QVariant &wid)
{
    WindowManager::keepAboveWindow(wid.toString());
}

int UkuiTaskManager::quickLauncherCountBeforeRow(int row) const
{
    int launcherCount = 0;
    row = qMin(row, d->m_items.size());
    for(int i = 0; i < row; ++i) {
        if(d->m_items.at(i)->hasLauncher()) {
            launcherCount++;
        }
    }
    return launcherCount;
}

void UkuiTaskManager::setQuickLauncher(const QString &desktopFile, int order)
{
    if(order >= d->m_items.size()) {
        order = d->m_items.size();
    }
    addQuickLauncher(desktopFile, order);
}

void UkuiTaskManager::onAppUninstalled(const QStringList &desktopFiles)
{
    for(const QString &desktopFile : desktopFiles) {
        removeQuickLauncher(desktopFile);
    }
}

void UkuiTaskManager::addQuickLauncherEvent(const QString &applicationName, const QString &operate )
{
    KCustomProperty *property = new KCustomProperty[2];
    property[0].key = strdup(QStringLiteral("AppliciationName").toLocal8Bit().data());
    property[0].value = strdup(applicationName.toLocal8Bit().data());
    property[1].key = strdup(QStringLiteral("EventType").toLocal8Bit().data());
    property[1].value = strdup(operate.toLocal8Bit().data());
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_append_custom_property(node, property, 2);
    kdk_dia_upload_default(node, strdup(QStringLiteral("pin_to_taskbar_event").toLocal8Bit().data()),strdup(QStringLiteral("ukui-panel").toLocal8Bit().data()));
    kdk_dia_data_free(node);
    free(property[0].key);
    free(property[0].value);
    free(property[1].key);
    free(property[1].value);
    delete []property;
}

//获取管控配置的生效值
template <typename T>
T getConfigValue(const QJsonObject &cfg, const QString &key, T localVal)
{
    if (cfg.contains(key)) {
        return cfg.value(key).toVariant().value<T>();
    }
    return localVal;
}

void UkuiTaskManager::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-taskManager-v11")) {
        return;
    }
    //管控中有值时，设置为管控值，否则设置为配置文件中的值
    QJsonObject oldControlConfig = d->m_controlConfig;
    d->m_controlConfig = exit ? QJsonObject() : UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();
    qDebug() << "TaskManager" << "function control config:" << d->m_controlConfig;
    //应用白名单
    if (d->m_controlConfig.contains(QStringLiteral("appWhiteList"))) {
        QStringList appWhiteList;
        for (QVariant &app : d->m_controlConfig.value(QStringLiteral("appWhiteList")).toArray().toVariantList()) {
            auto id = app.toString();
            if (id.endsWith(".desktop")) {
                id = id.left(id.size() - 8);
            }
            appWhiteList.append(id);
        }
        if (!compareListBySort(d->m_appWhiteList, appWhiteList)) {
            d->m_appWhiteList = appWhiteList;
            Q_EMIT appWhiteListChanged();
        }
    } else if (!d->m_appWhiteList.isEmpty()) {
        d->m_appWhiteList.clear();
        Q_EMIT appWhiteListChanged();
    }
    //应用黑名单
    if (d->m_controlConfig.contains(QStringLiteral("appBlackList"))) {
        QStringList appBlackList;
        for (QVariant &app : d->m_controlConfig.value(QStringLiteral("appBlackList")).toArray().toVariantList()) {
            auto id = app.toString();
            if (id.endsWith(".desktop")) {
                id = id.left(id.size() - 8);
            }
            appBlackList.append(id);
        }
        if (!compareListBySort(d->m_appBlackList, appBlackList)) {
            d->m_appBlackList= appBlackList;
            Q_EMIT appBlackListChanged();
        }
    } else if (!d->m_appBlackList.isEmpty()) {
        d->m_appBlackList.clear();
        Q_EMIT appBlackListChanged();
    }

    auto checkConfigChanged = [this, &oldControlConfig](const QString& key, auto& defaultValue, auto signal) {
        auto oldValue = getConfigValue(oldControlConfig, key, d->m_config->getValue(key).toInt());
        auto newValue = getConfigValue(d->m_controlConfig, key, d->m_config->getValue(key).toInt());
        if (oldValue != newValue) {
            defaultValue = newValue;
            Q_EMIT (this->*signal)();
        }
    };
    //mergeIcons
    checkConfigChanged(QStringLiteral("mergeIcons"), d->m_mergeIcons, &UkuiTaskManager::mergeIconsChanged);
    //taskbarIconsShowedOn
    checkConfigChanged(QStringLiteral("taskbarIconsShowedOn"), d->m_taskbarIconsShowedOn, &UkuiTaskManager::taskbarIconsShowedOnChanged);
    //是否禁用左键点击（任务栏整体禁用）
    if (d->m_controlConfig.contains(QStringLiteral("disableMouseLeftButton"))) {
        bool disable = d->m_controlConfig.value(QStringLiteral("disableMouseLeftButton")).toBool();
        if (disable != d->m_disableMouseLeftButton) {
            d->m_disableMouseLeftButton = disable;
            Q_EMIT disableMouseLeftButtonChanged();
        }
    } else {
        d->m_disableMouseLeftButton = false;
        Q_EMIT disableMouseLeftButtonChanged();
    }
    //是否禁用右键点击（任务栏整体禁用）
    if (d->m_controlConfig.contains(QStringLiteral("disableMouseRightButton"))) {
        bool disable = d->m_controlConfig.value(QStringLiteral("disableMouseRightButton")).toBool();
        if (disable != d->m_disableMouseRightButton) {
            d->m_disableMouseRightButton = disable;
            Q_EMIT disableMouseRightButtonChanged();
        }
    } else {
        d->m_disableMouseRightButton = false;
        Q_EMIT disableMouseRightButtonChanged();
    }
    //是否禁用任务栏鼠标悬浮效果（任务栏整体）
    if (d->m_controlConfig.contains(QStringLiteral("disableMouseHover"))) {
        bool disable = d->m_controlConfig.value(QStringLiteral("disableMouseHover")).toBool();
        if (disable != d->m_disableMouseHover) {
            d->m_disableMouseHover = disable;
            Q_EMIT disableMouseHoverChanged();
        }
    } else {
        d->m_disableMouseHover = false;
        Q_EMIT disableMouseHoverChanged();
    }
}

bool UkuiTaskManager::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        for(auto item : d->m_items) {
            item->updateIconStatus();
        }
        d->m_requestDataChange = false;
    }
    return QObject::event(event);
}

void UkuiTaskManager::requestDataChange()
{
    if (!d->m_requestDataChange) {
        d->m_requestDataChange = true;
        auto ev = new QEvent(QEvent::UpdateRequest);
        QCoreApplication::postEvent(this, ev);
    }
}

bool UkuiTaskManager::disableMouseLeftButton() const
{
    return d->m_disableMouseLeftButton;
}

bool UkuiTaskManager::disableMouseRightButton() const
{
    return d->m_disableMouseRightButton;
}

bool UkuiTaskManager::disableMouseHover() const
{
    return d->m_disableMouseHover;
}

int UkuiTaskManager::mergeIcons() const
{
    return d->m_mergeIcons;
}

QStringList UkuiTaskManager::appWhiteList() const
{
    return d->m_appWhiteList;
}

QStringList UkuiTaskManager::appBlackList() const
{
    return d->m_appBlackList;
}

bool UkuiTaskManager::enableDragToApp() const
{
    return d->m_enableDragToApp;
}

void UkuiTaskManager::setEnableDragToApp(bool enable)
{
    if (enable != d->m_enableDragToApp) {
        d->m_enableDragToApp = enable;
        Q_EMIT enableDragToAppChanged();
    }
}
int UkuiTaskManager::taskbarIconsShowedOnByControl() const
{
    return d->m_taskbarIconsShowedOn;
}
}
#include "moc_ukui-task-manager.cpp"
