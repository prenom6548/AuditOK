#include <QtTest>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QQuickItem>
#include <QJsonObject>
#include <QJsonArray>
#include <config-loader.h>

#include "../../common/dbus-types.h"

#define private public
#define protected public
#include "plugin/tray-items-model.h"
#include "plugin/item-group-model.h"
#include "plugin/control-panel-model.h"
#include "plugin/items-filter-model.h"
#include "plugin/security-function-control.h"
#undef protected
#undef private

using namespace UkuiQuick;

namespace {

const QString WATCHER_SERVICE = QStringLiteral("org.kde.StatusNotifierWatcher");
const QString WATCHER_PATH = QStringLiteral("/StatusNotifierWatcher");

class ImitateStatusNotifierItem : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierItem")
    Q_PROPERTY(QString Category READ category)
    Q_PROPERTY(QString Id READ id)
    Q_PROPERTY(QString Title READ title)
    Q_PROPERTY(QString Status READ status)
    Q_PROPERTY(QString WindowId READ windowId)
    Q_PROPERTY(bool ItemIsMenu READ itemIsMenu)
    Q_PROPERTY(QString AttentionMovieName READ attentionMovieName)
    Q_PROPERTY(QString IconThemePath READ iconThemePath)
    Q_PROPERTY(QString OverlayIconName READ overlayIconName)
    Q_PROPERTY(DbusImageVector OverlayIconPixmap READ overlayIconPixmap)
    Q_PROPERTY(QString IconName READ iconName)
    Q_PROPERTY(DbusImageVector IconPixmap READ iconPixmap)
    Q_PROPERTY(QString AttentionIconName READ attentionIconName)
    Q_PROPERTY(DbusImageVector AttentionIconPixmap READ attentionIconPixmap)
    Q_PROPERTY(DbusToolTipStruct ToolTip READ toolTip)
    Q_PROPERTY(QDBusObjectPath Menu READ menu)

public:
    explicit ImitateStatusNotifierItem(const QString &service,
                                    const QString &itemId,
                                    const QString &itemTitle,
                                    const QString &itemIconName,
                                    const QString &itemStatus,
                                    const QString &subTitle,
                                    QObject *parent = nullptr)
        : QObject(parent),
          m_service(service),
          m_id(itemId),
          m_title(itemTitle),
          m_iconName(itemIconName),
          m_status(itemStatus),
          m_toolTip({itemIconName, {}, itemTitle, subTitle})
    {
    }

    QString category() const { return QStringLiteral("SystemServices"); }
    QString id() const { return m_id; }
    QString title() const { return m_title; }
    QString status() const { return m_status; }
    QString windowId() const { return QStringLiteral("window-%1").arg(m_id); }
    bool itemIsMenu() const { return m_itemIsMenu; }
    QString attentionMovieName() const { return QStringLiteral("movie-%1").arg(m_id); }
    QString iconThemePath() const { return QString(); }
    QString overlayIconName() const { return QStringLiteral("overlay-%1").arg(m_id); }
    DbusImageVector overlayIconPixmap() const { return {}; }
    QString iconName() const { return m_iconName; }
    DbusImageVector iconPixmap() const { return {}; }
    QString attentionIconName() const { return QStringLiteral("attention-%1").arg(m_id); }
    DbusImageVector attentionIconPixmap() const { return {}; }
    DbusToolTipStruct toolTip() const { return m_toolTip; }
    QDBusObjectPath menu() const { return QDBusObjectPath(QStringLiteral("/NO_DBUSMENU")); }

    int activateCount() const { return m_activateCount; }
    int contextMenuCount() const { return m_contextMenuCount; }
    int secondaryActivateCount() const { return m_secondaryActivateCount; }
    int scrollCount() const { return m_scrollCount; }
    QString lastToken() const { return m_lastToken; }

    void setTitle(const QString &title)
    {
        m_title = title;
        m_toolTip.title = title;
    }

    void emitNewTitle()
    {
        Q_EMIT NewTitle();
    }

public Q_SLOTS:
    void ContextMenu(int, int)
    {
        ++m_contextMenuCount;
    }

    void Activate(int, int)
    {
        ++m_activateCount;
    }

    void SecondaryActivate(int, int)
    {
        ++m_secondaryActivateCount;
    }

    void Scroll(int, const QString &)
    {
        ++m_scrollCount;
    }

    void ProvideXdgActivationToken(const QString &token)
    {
        m_lastToken = token;
    }

Q_SIGNALS:
    void NewTitle();
    void NewIcon();
    void NewAttentionIcon();
    void NewOverlayIcon();
    void NewToolTip();
    void NewStatus(const QString &status);
    void NewMenu();

private:
    QString m_service;
    QString m_id;
    QString m_title;
    QString m_iconName;
    QString m_status;
    bool m_itemIsMenu = false;
    DbusToolTipStruct m_toolTip;
    int m_activateCount = 0;
    int m_contextMenuCount = 0;
    int m_secondaryActivateCount = 0;
    int m_scrollCount = 0;
    QString m_lastToken;
};

class ImitateStatusNotifierWatcher : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierWatcher")
    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ registeredStatusNotifierItems)

public:
    explicit ImitateStatusNotifierWatcher(QObject *parent = nullptr) : QObject(parent) {}

    QStringList registeredStatusNotifierItems() const
    {
        return m_registeredStatusNotifierItems;
    }

    void registerItem(const QString &service)
    {
        if (m_registeredStatusNotifierItems.contains(service)) {
            return;
        }
        m_registeredStatusNotifierItems.append(service);
        Q_EMIT StatusNotifierItemRegistered(service);
    }

    void unregisterItem(const QString &service)
    {
        if (!m_registeredStatusNotifierItems.removeOne(service)) {
            return;
        }
        Q_EMIT StatusNotifierItemUnregistered(service);
    }

public Q_SLOTS:
    void RegisterStatusNotifierHost(const QString &service)
    {
        m_registeredHosts.append(service);
    }

Q_SIGNALS:
    void StatusNotifierItemRegistered(const QString &service);
    void StatusNotifierItemUnregistered(const QString &service);

private:
    QStringList m_registeredStatusNotifierItems;
    QStringList m_registeredHosts;
};

struct ImitateItemSpec
{
    QString busService;
    QString objectPath;
    QString id;
    QString title;
    QString iconName;
    QString status;
    QString subTitle;
};

} // namespace

class trayItemsModel_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();

    void test_constructorMigration();
    void test_roleNamesAndHelpers();
    void test_addSourceAndData();
    void test_activateAndShowContextMenu();
    void test_setOrder();
    void test_setOrderInGroupAndBetweenGroups();
    void test_setSeparateIndexBoundaries();
    void test_configChangedHideAndShow();
    void test_removeSourceAndControlPanelState();
    void test_insertHiddenItemAndCanHideState();
    void test_setOrderRecordBranch();
    void test_dataUpdatedAndControlStateChanged();

private:
    ImitateStatusNotifierItem *registerItem(const ImitateItemSpec &spec);
    void unregisterItem(const QString &service);
    QModelIndex indexById(const QString &id) const;
    void waitForRowCount(int count) const;
    void ensureBaseItems();

    ImitateStatusNotifierWatcher *m_watcher = nullptr;
    TrayItemsModel *m_model = nullptr;
    QHash<QString, ImitateStatusNotifierItem *> m_imitateItems;
};

void trayItemsModel_test::initTestCase()
{
    qDBusRegisterMetaType<DbusImageStruct>();
    qDBusRegisterMetaType<DbusImageVector>();
    qDBusRegisterMetaType<DbusToolTipStruct>();

    QVERIFY(QDBusConnection::sessionBus().isConnected());

    m_watcher = new ImitateStatusNotifierWatcher(this);
    QVERIFY(QDBusConnection::sessionBus().registerService(WATCHER_SERVICE));
    QVERIFY(QDBusConnection::sessionBus().registerObject(WATCHER_PATH,
                                                         m_watcher,
                                                         QDBusConnection::ExportAllSlots
                                                         | QDBusConnection::ExportAllSignals
                                                         | QDBusConnection::ExportAllProperties));

    auto *config = ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"));
    config->setValue(QStringLiteral("separateIndex"), 9);
    config->setValue(QStringLiteral("fixedItems"), QStringList{QStringLiteral("ukui-sidebar"),
                                                               QStringLiteral("ukui-bluetooth")});
    config->setValue(QStringLiteral("orderedItems"), QStringList{QStringLiteral("ukui-bluetooth"),
                                                                 QStringLiteral("Fcitx"),
                                                                 QStringLiteral("ukui-power-manager-tray"),
                                                                 QStringLiteral("kylin-vpn")});
    config->setValue(QStringLiteral("trayIconsInhibited"), QStringList{QStringLiteral("ukui-bluetooth"),
                                                                       QStringLiteral("kylin-vpn")});
    config->setValue(QStringLiteral("controlPanelItems"), QStringList{QStringLiteral("legacy-panel")});
    config->setValue(QStringLiteral("version"), QStringLiteral("1.0"));
    config->setValue(QStringLiteral("trayIconsCanInhibited"), QStringList());
    config->forceSync();

    m_model = TrayItemsModel::instance();
    QVERIFY(m_model != nullptr);
}

void trayItemsModel_test::cleanup()
{
    const QStringList services = m_imitateItems.keys();
    for (const QString &service : services) {
        unregisterItem(service);
    }
    waitForRowCount(0);

    auto *config = ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"));
    config->setValue(QStringLiteral("trayIconsInhibited"), QStringList());
    config->setValue(QStringLiteral("trayIconsCanInhibited"), QStringList());
    config->setValue(QStringLiteral("orderedItems"), QStringList(m_model->m_orderList.mid(m_model->m_fixList.size())));
    config->forceSync();

    m_model->m_hideList.clear();
    m_model->m_canHideList.clear();
}

void trayItemsModel_test::cleanupTestCase()
{
    QDBusConnection::sessionBus().unregisterObject(WATCHER_PATH);
    QDBusConnection::sessionBus().unregisterService(WATCHER_SERVICE);
}

ImitateStatusNotifierItem *trayItemsModel_test::registerItem(const ImitateItemSpec &spec)
{
    const QString source = spec.busService + spec.objectPath;
    auto *item = new ImitateStatusNotifierItem(spec.busService,
                                            spec.id,
                                            spec.title,
                                            spec.iconName,
                                            spec.status,
                                            spec.subTitle,
                                            this);

    if (!QDBusConnection::sessionBus().registerService(spec.busService)) {
        delete item;
        return nullptr;
    }
    if (!QDBusConnection::sessionBus().registerObject(spec.objectPath,
                                                      item,
                                                      QDBusConnection::ExportAllSlots
                                                      | QDBusConnection::ExportAllSignals
                                                      | QDBusConnection::ExportAllProperties)) {
        QDBusConnection::sessionBus().unregisterService(spec.busService);
        delete item;
        return nullptr;
    }

    item->setProperty("busService", spec.busService);
    item->setProperty("objectPath", spec.objectPath);
    m_imitateItems.insert(source, item);
    m_watcher->registerItem(source);
    return item;
}

void trayItemsModel_test::unregisterItem(const QString &service)
{
    auto *item = m_imitateItems.take(service);
    if (!item) {
        return;
    }

    m_watcher->unregisterItem(service);
    QTRY_VERIFY(!m_model->m_itemInfo.contains(service));

    QDBusConnection::sessionBus().unregisterObject(item->property("objectPath").toString(), QDBusConnection::UnregisterTree);
    QDBusConnection::sessionBus().unregisterService(item->property("busService").toString());
    item->deleteLater();
}

QModelIndex trayItemsModel_test::indexById(const QString &id) const
{
    for (int row = 0; row < m_model->rowCount({}); ++row) {
        QModelIndex modelIndex = m_model->index(row, 0);
        if (modelIndex.data(TrayItemsModel::Id).toString() == id) {
            return modelIndex;
        }
    }
    return {};
}

void trayItemsModel_test::waitForRowCount(int count) const
{
    QTRY_COMPARE(m_model->rowCount({}), count);
}

void trayItemsModel_test::ensureBaseItems()
{
    if (!indexById(QStringLiteral("Fcitx")).isValid()) {
        QVERIFY(registerItem({QStringLiteral("org.test.TrayItemFcitx"),
                              QStringLiteral("/StatusNotifierItemFcitx"),
                              QStringLiteral("Fcitx"),
                              QStringLiteral("Fcitx Title"),
                              QStringLiteral("edit-find"),
                              QStringLiteral("Active"),
                              QStringLiteral("Fcitx Sub")}) != nullptr);
    }
    if (!indexById(QStringLiteral("ukui-sidebar")).isValid()) {
        QVERIFY(registerItem({QStringLiteral("org.test.TrayItemSidebar"),
                              QStringLiteral("/StatusNotifierItemSidebar"),
                              QStringLiteral("ukui-sidebar"),
                              QStringLiteral("Sidebar Title"),
                              QStringLiteral("applications-system"),
                              QStringLiteral("Active"),
                              QStringLiteral("Sidebar Sub")}) != nullptr);
    }
    if (!indexById(QStringLiteral("ukui-power-manager-tray")).isValid()) {
        QVERIFY(registerItem({QStringLiteral("org.test.TrayItemPower"),
                              QStringLiteral("/StatusNotifierItemPower"),
                              QStringLiteral("ukui-power-manager-tray"),
                              QStringLiteral("Power Title"),
                              QStringLiteral("battery-good"),
                              QStringLiteral("NeedsAttention"),
                              QStringLiteral("Power Sub")}) != nullptr);
    }
    if (!indexById(QStringLiteral("custom-item")).isValid()) {
        QVERIFY(registerItem({QStringLiteral("org.test.TrayItemCustom"),
                              QStringLiteral("/StatusNotifierItemCustom"),
                              QStringLiteral("custom-item"),
                              QStringLiteral("Custom Title"),
                              QStringLiteral("help-about"),
                              QStringLiteral("Passive"),
                              QStringLiteral("Custom Sub")}) != nullptr);
    }

    waitForRowCount(4);
}

void trayItemsModel_test::test_constructorMigration()
{
    QCOMPARE(m_model->getSeparateIndex(), 9);
    QCOMPARE(m_model->m_fixList, QStringList({QStringLiteral("ukui-sidebar")}));
    QCOMPARE(m_model->getControlPanelItems(),
             QStringList({QStringLiteral("ukui-volume-control-applet-qt"),
                          QStringLiteral("ukui-bluetooth"),
                          QStringLiteral("kylin-nm")}));
    QCOMPARE(m_model->m_hideList, QStringList());
    QCOMPARE(m_model->m_orderList,
             QStringList({QStringLiteral("ukui-sidebar"),
                          QStringLiteral("Fcitx"),
                          QStringLiteral("ukui-power-manager-tray"),
                          QStringLiteral("kylin-vpn")}));

    auto *config = ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"));
    QCOMPARE(config->getValue(QStringLiteral("version")).toString(), QStringLiteral("2.4"));
    QCOMPARE(config->getValue(QStringLiteral("controlPanelItems")).toStringList(), m_model->getControlPanelItems());
    QCOMPARE(config->getValue(QStringLiteral("trayIconsInhibited")).toStringList(), QStringList());
}

void trayItemsModel_test::test_roleNamesAndHelpers()
{
    QHash<int, QByteArray> roles = m_model->roleNames();
    QCOMPARE(roles.value(TrayItemsModel::Id), QByteArray("Id"));
    QCOMPARE(roles.value(TrayItemsModel::Desktop), QByteArray("Desktop"));

    QVERIFY(!m_model->extractIcon(QIcon::fromTheme(QStringLiteral("help-about"))).isNull());
    QCOMPARE(m_model->extractIcon(QIcon(), QVariant(QStringLiteral("fallback"))).toString(), QStringLiteral("fallback"));
    QVERIFY(!m_model->getDesktop(QCoreApplication::applicationPid()).isNull());
    QCOMPARE(m_model->indexOfSource(QStringLiteral("missing")), -1);
    QVERIFY(!m_model->data(m_model->index(-1, 0), TrayItemsModel::Id).isValid());
    QVERIFY(!m_model->data(m_model->index(0, 0), -1).isValid());

    m_model->activate({});
    m_model->showContextMenu({}, nullptr);
    m_model->setOrder({}, 0);
    m_model->removeSource(QStringLiteral("missing"));
    m_model->dataUpdated(QStringLiteral("missing"));
    m_model->controlStateChanged(false);
}

void trayItemsModel_test::test_addSourceAndData()
{
    auto *fcitx = registerItem({QStringLiteral("org.test.TrayItemFcitx"),
                                QStringLiteral("/StatusNotifierItemFcitx"),
                                QStringLiteral("Fcitx"),
                                QStringLiteral("Fcitx Title"),
                                QStringLiteral("edit-find"),
                                QStringLiteral("Active"),
                                QStringLiteral("Fcitx Sub")});
    auto *sidebar = registerItem({QStringLiteral("org.test.TrayItemSidebar"),
                                  QStringLiteral("/StatusNotifierItemSidebar"),
                                  QStringLiteral("ukui-sidebar"),
                                  QStringLiteral("Sidebar Title"),
                                  QStringLiteral("applications-system"),
                                  QStringLiteral("Active"),
                                  QStringLiteral("Sidebar Sub")});
    auto *power = registerItem({QStringLiteral("org.test.TrayItemPower"),
                                QStringLiteral("/StatusNotifierItemPower"),
                                QStringLiteral("ukui-power-manager-tray"),
                                QStringLiteral("Power Title"),
                                QStringLiteral("battery-good"),
                                QStringLiteral("NeedsAttention"),
                                QStringLiteral("Power Sub")});
    QVERIFY(registerItem({QStringLiteral("org.test.TrayItemCustom"),
                          QStringLiteral("/StatusNotifierItemCustom"),
                          QStringLiteral("custom-item"),
                          QStringLiteral("Custom Title"),
                          QStringLiteral("help-about"),
                          QStringLiteral("Passive"),
                          QStringLiteral("Custom Sub")}) != nullptr);
    QVERIFY(registerItem({QStringLiteral("org.test.TrayItemEmptyId"),
                          QStringLiteral("/StatusNotifierItemEmptyId"),
                          QString(),
                          QStringLiteral("Empty Id"),
                          QStringLiteral("help-about"),
                          QStringLiteral("Passive"),
                          QStringLiteral("Empty Sub")}) != nullptr);

    waitForRowCount(4);

    QCOMPARE(m_model->index(0, 0).data(TrayItemsModel::Id).toString(), QStringLiteral("ukui-sidebar"));
    QCOMPARE(m_model->index(1, 0).data(TrayItemsModel::Id).toString(), QStringLiteral("Fcitx"));
    QCOMPARE(m_model->index(2, 0).data(TrayItemsModel::Id).toString(), QStringLiteral("ukui-power-manager-tray"));
    QCOMPARE(m_model->index(3, 0).data(TrayItemsModel::Id).toString(), QStringLiteral("custom-item"));

    QModelIndex sidebarIndex = indexById(QStringLiteral("ukui-sidebar"));
    QModelIndex powerIndex = indexById(QStringLiteral("ukui-power-manager-tray"));
    QModelIndex fcitxIndex = indexById(QStringLiteral("Fcitx"));

    QVERIFY(sidebarIndex.data(TrayItemsModel::Fixed).toBool());
    QVERIFY(!sidebarIndex.data(TrayItemsModel::RecordOrder).toBool());
    QVERIFY(fcitxIndex.data(TrayItemsModel::RecordOrder).toBool());
    QVERIFY(powerIndex.data(TrayItemsModel::RecordOrder).toBool());

    QCOMPARE(powerIndex.data(TrayItemsModel::Service).toString(), QStringLiteral("org.test.TrayItemPower"));
    QCOMPARE(powerIndex.data(TrayItemsModel::Category).toString(), QStringLiteral("SystemServices"));
    QCOMPARE(powerIndex.data(TrayItemsModel::IconName).toString(), QStringLiteral("battery-good"));
    QCOMPARE(powerIndex.data(TrayItemsModel::IconThemePath).toString(), QString());
    QCOMPARE(powerIndex.data(TrayItemsModel::AttentionIconName).toString(),
             QStringLiteral("attention-ukui-power-manager-tray"));
    QCOMPARE(powerIndex.data(TrayItemsModel::AttentionMovieName).toString(),
             QStringLiteral("movie-ukui-power-manager-tray"));
    QCOMPARE(powerIndex.data(TrayItemsModel::ItemIsMenu).toBool(), false);
    QCOMPARE(powerIndex.data(TrayItemsModel::OverlayIconName).toString(),
             QStringLiteral("overlay-ukui-power-manager-tray"));
    QCOMPARE(powerIndex.data(TrayItemsModel::Desktop).toString(), QString());
    QCOMPARE(powerIndex.data(TrayItemsModel::Status).toString(), QStringLiteral("NeedsAttention"));
    QCOMPARE(powerIndex.data(TrayItemsModel::Title).toString(), QStringLiteral("Power Title"));
    QCOMPARE(powerIndex.data(TrayItemsModel::ToolTipTitle).toString(), QStringLiteral("Power Title"));
    QCOMPARE(powerIndex.data(TrayItemsModel::ToolTipSubTitle).toString(), QStringLiteral("Power Sub"));
    QCOMPARE(powerIndex.data(TrayItemsModel::WindowId).toString(), QStringLiteral("window-ukui-power-manager-tray"));
    QCOMPARE(powerIndex.data(TrayItemsModel::Row).toInt(), powerIndex.row());
    QVERIFY(powerIndex.data(TrayItemsModel::Icon).value<QIcon>().isNull() == false);
    QVERIFY(powerIndex.data(TrayItemsModel::AttentionIcon).value<QIcon>().isNull() == false);

    QVERIFY(m_model->m_canHideList.contains(QStringLiteral("Fcitx")));
    QVERIFY(m_model->m_canHideList.contains(QStringLiteral("ukui-power-manager-tray")));
    QCOMPARE(m_model->indexOfSource(QStringLiteral("org.test.TrayItemPower/StatusNotifierItemPower")), powerIndex.row());

    m_model->addSource(QStringLiteral("org.test.TrayItemPower/StatusNotifierItemPower"));
    waitForRowCount(4);

    QVERIFY(fcitx->activateCount() == 0);
    QVERIFY(sidebar != nullptr);
    QVERIFY(power != nullptr);
}

void trayItemsModel_test::test_activateAndShowContextMenu()
{
    ensureBaseItems();

    auto *power = m_imitateItems.value(QStringLiteral("org.test.TrayItemPower/StatusNotifierItemPower"));
    QVERIFY(power != nullptr);

    QModelIndex powerIndex = indexById(QStringLiteral("ukui-power-manager-tray"));
    QVERIFY(powerIndex.isValid());

    m_model->activate(powerIndex);
    QTRY_COMPARE(power->activateCount(), 1);

    QQuickItem item;
    m_model->showContextMenu(powerIndex, &item);
    QTRY_COMPARE(power->contextMenuCount(), 1);
}

void trayItemsModel_test::test_setOrder()
{
    ensureBaseItems();

    QCOMPARE(indexById(QStringLiteral("ukui-sidebar")).row(), 0);
    QCOMPARE(indexById(QStringLiteral("Fcitx")).row(), 1);
    QCOMPARE(indexById(QStringLiteral("ukui-power-manager-tray")).row(), 2);

    m_model->setOrder(indexById(QStringLiteral("ukui-sidebar")), 2);
    QCOMPARE(indexById(QStringLiteral("ukui-sidebar")).row(), 0);

    m_model->setOrder(indexById(QStringLiteral("Fcitx")), 1);
    QCOMPARE(indexById(QStringLiteral("Fcitx")).row(), 1);

    m_model->setOrder(indexById(QStringLiteral("Fcitx")), 2);
    QCOMPARE(indexById(QStringLiteral("Fcitx")).row(), 2);
    QCOMPARE(indexById(QStringLiteral("ukui-power-manager-tray")).row(), 1);

    m_model->setOrder(indexById(QStringLiteral("Fcitx")), 1);
    QCOMPARE(indexById(QStringLiteral("Fcitx")).row(), 1);
    QCOMPARE(indexById(QStringLiteral("ukui-power-manager-tray")).row(), 2);

    QCOMPARE(ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"))
                 ->getValue(QStringLiteral("orderedItems")).toStringList(),
             QStringList({QStringLiteral("Fcitx"),
                          QStringLiteral("ukui-power-manager-tray"),
                          QStringLiteral("kylin-vpn")}));
}

void trayItemsModel_test::test_setOrderInGroupAndBetweenGroups()
{
    ensureBaseItems();
    m_model->setSeparateIndex(1);

    ItemGroupModel showGroup;
    showGroup.setModelType(ItemGroupModel::Show);
    showGroup.setCurrentSeparateIndex(1);

    ItemGroupModel foldGroup;
    foldGroup.setModelType(ItemGroupModel::Fold);
    foldGroup.setCurrentSeparateIndex(1);

    QVERIFY(showGroup.rowCount() >= 2);
    QVERIFY(foldGroup.rowCount() >= 2);

    QModelIndex firstShowIndex = showGroup.index(0, 0);
    QModelIndex firstFoldIndex = foldGroup.index(0, 0);
    QVERIFY(firstShowIndex.isValid());
    QVERIFY(firstFoldIndex.isValid());
    const QString foldFirstId = firstFoldIndex.data(TrayItemsModel::Id).toString();
    QVERIFY(!foldFirstId.isEmpty());

    m_model->setOrderInGroup(&showGroup, firstShowIndex, 5);
    QCOMPARE(indexById(QStringLiteral("Fcitx")).row(), 1);

    m_model->setOrderBetweenGroups(&foldGroup, firstFoldIndex, &showGroup, 5);
    QVERIFY(indexById(foldFirstId).row() <= 2);
}

void trayItemsModel_test::test_setSeparateIndexBoundaries()
{
    ensureBaseItems();

    QSignalSpy separateChangedSpy(m_model, &TrayItemsModel::separateIndexChanged);

    m_model->setSeparateIndex(1);
    QCOMPARE(m_model->getSeparateIndex(), 1);

    m_model->setSeparateIndex(99);
    QCOMPARE(m_model->getSeparateIndex(), m_model->rowCount({}) - 1);
    QVERIFY(separateChangedSpy.count() >= 2);

    auto *config = ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"));
    QCOMPARE(config->getValue(QStringLiteral("separateIndex")).toInt(), 99);
}

void trayItemsModel_test::test_configChangedHideAndShow()
{
    ensureBaseItems();

    auto *config = ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"));
    config->setValue(QStringLiteral("trayIconsInhibited"), QStringList{QStringLiteral("ukui-power-manager-tray")});
    config->forceSync();

    QTRY_VERIFY(!indexById(QStringLiteral("ukui-power-manager-tray")).isValid());
    QCOMPARE(m_model->rowCount({}), 3);

    config->setValue(QStringLiteral("trayIconsInhibited"), QStringList());
    config->forceSync();

    QTRY_VERIFY(indexById(QStringLiteral("ukui-power-manager-tray")).isValid());
    QCOMPARE(m_model->rowCount({}), 4);
}

void trayItemsModel_test::test_removeSourceAndControlPanelState()
{
    ensureBaseItems();

    ControlPanelModel *controlPanelModel = ControlPanelModel::self();
    QVERIFY(controlPanelModel != nullptr);
    QVERIFY(controlPanelModel->enable());

    unregisterItem(QStringLiteral("org.test.TrayItemSidebar/StatusNotifierItemSidebar"));
    waitForRowCount(3);
    QVERIFY(!controlPanelModel->enable());

    QVERIFY(!m_model->m_canHideList.isEmpty());
    unregisterItem(QStringLiteral("org.test.TrayItemPower/StatusNotifierItemPower"));
    waitForRowCount(2);
    QVERIFY(!m_model->m_canHideList.contains(QStringLiteral("ukui-power-manager-tray")));
}

void trayItemsModel_test::test_insertHiddenItemAndCanHideState()
{
    ensureBaseItems();

    QVERIFY(m_model->m_itemInfo.contains(QStringLiteral("org.test.TrayItemFcitx/StatusNotifierItemFcitx")));
    QVERIFY(m_model->m_canHideList.contains(QStringLiteral("Fcitx")));

    auto *config = ConfigLoader::getConfig(QStringLiteral("org.ukui.systemTray"));
    config->setValue(QStringLiteral("trayIconsInhibited"), QStringList{QStringLiteral("Fcitx")});
    config->forceSync();
    QTRY_VERIFY(!indexById(QStringLiteral("Fcitx")).isValid());

    unregisterItem(QStringLiteral("org.test.TrayItemFcitx/StatusNotifierItemFcitx"));
    QVERIFY(!m_model->m_canHideList.contains(QStringLiteral("Fcitx")));
    QCOMPARE(config->getValue(QStringLiteral("trayIconsCanInhibited")).toStringList().contains(QStringLiteral("Fcitx")), false);

    QVERIFY(registerItem({QStringLiteral("org.test.TrayItemFcitx2"),
                          QStringLiteral("/StatusNotifierItemFcitx2"),
                          QStringLiteral("Fcitx"),
                          QStringLiteral("Fcitx Title 2"),
                          QStringLiteral("edit-find"),
                          QStringLiteral("Active"),
                          QStringLiteral("Fcitx Sub 2")}) != nullptr);
    QTest::qWait(100);
    QVERIFY(!indexById(QStringLiteral("Fcitx")).isValid());

    config->setValue(QStringLiteral("trayIconsInhibited"), QStringList());
    config->forceSync();
    QTRY_VERIFY(indexById(QStringLiteral("Fcitx")).isValid());
}

void trayItemsModel_test::test_setOrderRecordBranch()
{
    ensureBaseItems();

    for (TrayItem *item : std::as_const(m_model->m_item)) {
        item->setRecordOrder(false);
    }
    QModelIndex customIndex = indexById(QStringLiteral("custom-item"));
    QVERIFY(customIndex.isValid());
    m_model->m_item[customIndex.row()]->setRecordOrder(true);

    const QStringList oldOrderList = m_model->m_orderList;
    m_model->setOrder(customIndex, 2);
    QCOMPARE(indexById(QStringLiteral("custom-item")).row(), 2);
    m_model->setOrder(indexById(QStringLiteral("custom-item")), 3);
    QCOMPARE(indexById(QStringLiteral("custom-item")).row(), 3);
    QCOMPARE(m_model->m_orderList, oldOrderList);
}

void trayItemsModel_test::test_dataUpdatedAndControlStateChanged()
{
    ensureBaseItems();

    auto *customItem = m_imitateItems.value(QStringLiteral("org.test.TrayItemCustom/StatusNotifierItemCustom"));
    QVERIFY(customItem != nullptr);

    QModelIndex customIndex = indexById(QStringLiteral("custom-item"));
    QVERIFY(customIndex.isValid());

    QSignalSpy dataChangedSpy(m_model, &QAbstractItemModel::dataChanged);
    customItem->setTitle(QStringLiteral("Custom Title Updated"));
    customItem->emitNewTitle();
    QTRY_VERIFY(!dataChangedSpy.isEmpty());
    QTRY_COMPARE(indexById(QStringLiteral("custom-item")).data(TrayItemsModel::Title).toString(),
                 QStringLiteral("Custom Title Updated"));

    for (TrayItem *item : std::as_const(m_model->m_item)) {
        item->setDesktop(QString());
    }

    const int beforeCount = dataChangedSpy.count();
    m_model->controlStateChanged(true);
    QTRY_VERIFY(dataChangedSpy.count() > beforeCount);

    const int afterFirstEnableCount = dataChangedSpy.count();
    m_model->controlStateChanged(true);
    QTest::qWait(50);
    QCOMPARE(dataChangedSpy.count(), afterFirstEnableCount);
}

QTEST_MAIN(trayItemsModel_test)

#include "trayItemsModelTest.moc"
