#include "platformwindow.h"
#include <QGuiApplication>
#include <QScreen>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <kwindowsystem.h>
#include "global-settings.h"
#include "common.h"

#include "manager-control.h"
#include "file-action-controler.h"
#include "global-settings-adapter.h"
#include "peony-desktop-application.h"
#include "desktop-item-proxy-model.h"
#include "advanced-desktop-item-model.h"
#include "file-info.h"
#include "file-utils.h"
#include "background-manager.h"
#include "windowmanager/windowmanager.h"
#include "screen-manager.h"
#include "theme-style.h"
#include "desktop-window-manager.h"
#include "widgets-proxy-model.h"
#include "desktop-widgets-manager.h"
#include "widget-manager-dialog.h"
#include <widget-loader.h>
#include "file-action-controler.h"
#ifdef KY_SDK_WAYLANDHELPER
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#endif
#include "file-meta-info.h"
#include <QFontMetrics>
#include <QFont>
#include <QApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQmlContext>
#include <QQmlEngine>
#include <QMimeData>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "textureviewer.h"
#include "texturegrabber.h"
#include <QDrag>
#include <QCursor>

#include <QMessageBox>
#include <QFileDialog>
#include <QListWidget>

static QList<PlatformWindow *> gWindows;

using namespace Peony;

#define WORKSTATION_ID 9999
#define HORIZON_MARGIN 10
#define VERTICAL_MARGIN 5
#define WIDGET_PADDING 8

static bool iconSizeLessThan (const QPair<QPoint, QString>& p1, const QPair<QPoint, QString>& p2)
{
    if (p1.first.x() > p2.first.x())
        return false;

    if (p1.first.x() < p2.first.x())
        return true;

    if ((p1.first.x() == p2.first.x()))
        return p1.first.y() < p2.first.y();

    return true;
}

// 在插件窗格挤占图标时使用
static bool iconSizeLessThanY (const QPair<QPoint, QString>& p1, const QPair<QPoint, QString>& p2)
{
    if (p1.first.y() > p2.first.y())
        return false;

    if (p1.first.y() < p2.first.y())
        return true;

    if ((p1.first.y() == p2.first.y()))
        return p1.first.x() < p2.first.x();

    return true;
}

// 在计算插件挤开位置时使用
static bool itemGridPosOffsetPosLesserThan(const QPoint &offsetPos1, const QPoint &offsetPos2)
{
    int dist1 = qAbs(offsetPos1.x()) + qAbs(offsetPos1.y());
    int dist2 = qAbs(offsetPos2.x()) + qAbs(offsetPos2.y());

    if (dist1 == dist2) {
        if (offsetPos1.y() == offsetPos2.y()) {
            return offsetPos1.x() > offsetPos2.x();
        }

        return offsetPos1.y() > offsetPos2.y();
    }

    return dist1 < dist2;
}

// 重排插件窗口时使用
static bool widgetRectPosLesserThan(const QPair<UkuiQuick::WidgetItem *, QRect>& p1, const QPair<UkuiQuick::WidgetItem *, QRect>& p2)
{
    auto rect1 = p1.second;
    auto rect2 = p2.second;

    if (rect1.right() > rect2.right()) {
        return true;
    } else if (rect1.right() < rect2.right()) {
        return false;
    }

    if (rect1.top() < rect2.top()) {
        return true;
    } else if (rect1.top() > rect2.top()) {
        return false;
    }

    auto widget1 = p1.first;
    auto widget2 = p2.first;

    return widget1->widget()->instanceId() < widget2->widget()->instanceId();
}

static QSize parseSizeHintValue(const QVariant &value)
{
    QVariantList sizeHintList;
    if (value.canConvert<QVariantList>()) {
        sizeHintList = value.toList();
    }
    if (sizeHintList.size() != 2) {
        return QSize();
    }
    return QSize(qMax(1, sizeHintList.at(0).toInt()), qMax(1, sizeHintList.at(1).toInt()));
}

static QSize widgetPixelSizeFromSizeHint(const QSize &sizeHint, bool layoutWithGrid, int gridUnitSize)
{
    if (!sizeHint.isValid() || sizeHint.isEmpty()) {
        return QSize();
    }
    if (!layoutWithGrid) {
        return sizeHint;
    }
    const int safeGridUnit = qMax(1, gridUnitSize);
    const int width = (safeGridUnit + WIDGET_SPACING) * sizeHint.width() - WIDGET_SPACING - WIDGET_MARGIN * 2;
    const int height = (safeGridUnit + WIDGET_SPACING) * sizeHint.height() - WIDGET_SPACING - WIDGET_MARGIN * 2;
    return QSize(qMax(1, width), qMax(1, height));
}

static QSize resolveWidgetPixelSize(const QString &widgetId, const QVariantMap &childData, const QVariantMap &variant)
{
    const int explicitWidth = childData.value(QStringLiteral("width")).toInt();
    const int explicitHeight = childData.value(QStringLiteral("height")).toInt();
    if (explicitWidth > 0 && explicitHeight > 0) {
        return QSize(explicitWidth, explicitHeight);
    }

    bool layoutWithGrid = false;
    int gridUnitSize = 60;
    QVariant metadataSizeHintValue;
    const QList<UkuiQuick::WidgetMetadata> allMetadata = UkuiQuick::WidgetLoader::globalLoader().widgetsMetadata();
    for (const auto &metadata : allMetadata) {
        if (metadata.id() != widgetId) {
            continue;
        }
        const QVariantMap contents = metadata.contents();
        layoutWithGrid = contents.value(QStringLiteral("SizePolicy")).toInt() != 0;
        if (contents.contains(QStringLiteral("GridUnitSize"))) {
            gridUnitSize = qMax(1, contents.value(QStringLiteral("GridUnitSize")).toInt());
        }
        metadataSizeHintValue = contents.value(QStringLiteral("SizeHint"));
        break;
    }

    QSize sizeHint = parseSizeHintValue(childData.value(QStringLiteral("SizeHint")));
    if (!sizeHint.isValid()) {
        sizeHint = parseSizeHintValue(variant.value(QStringLiteral("sizeHint")));
    }
    if (!sizeHint.isValid()) {
        sizeHint = parseSizeHintValue(metadataSizeHintValue);
    }
    if (sizeHint.isValid()) {
        return widgetPixelSizeFromSizeHint(sizeHint, layoutWithGrid, gridUnitSize);
    }

    if (layoutWithGrid) {
        return QSize(gridUnitSize, gridUnitSize);
    }
    return QSize(200, 150);
}

static bool widgetRectOverlapsExisting(const QRect &targetRect, PlatformWindow *window)
{
    if (!window || !window->mainView() || !targetRect.isValid() || targetRect.isEmpty()) {
        return false;
    }

    const QRect expandedTarget = targetRect.adjusted(-WIDGET_PADDING, -WIDGET_PADDING, WIDGET_PADDING, WIDGET_PADDING);
    const auto widgets = window->mainView()->widgets();
    for (auto *existingWidget : widgets) {
        if (!existingWidget || !existingWidget->config()) {
            continue;
        }
        if (existingWidget->metadata().contents().value(QStringLiteral("Layer")).toInt() < 0) {
            continue;
        }
        const int x = existingWidget->config()->getValue(QStringLiteral("x")).toInt();
        const int y = existingWidget->config()->getValue(QStringLiteral("y")).toInt();
        int width = existingWidget->config()->getValue(QStringLiteral("width")).toInt();
        int height = existingWidget->config()->getValue(QStringLiteral("height")).toInt();
        if (width <= 0 || height <= 0) {
            const QVariantMap tmpChildData {
                {QStringLiteral("width"), width},
                {QStringLiteral("height"), height},
                {QStringLiteral("SizeHint"), existingWidget->config()->getValue(QStringLiteral("SizeHint"))}
            };
            const QSize fallbackSize = resolveWidgetPixelSize(existingWidget->id(), tmpChildData, QVariantMap());
            width = fallbackSize.width();
            height = fallbackSize.height();
        }
        if (width <= 0 || height <= 0) {
            continue;
        }

        const QRect existingRect(x, y, width, height);
        const QRect expandedExisting = existingRect.adjusted(-WIDGET_PADDING, -WIDGET_PADDING, WIDGET_PADDING, WIDGET_PADDING);
        if (expandedExisting.intersects(expandedTarget)) {
            return true;
        }
    }
    return false;
}

static bool findNearestAvailableWidgetTopLeft(PlatformWindow *window,
                                              const QSize &widgetSize,
                                              const QPoint &preferredPos,
                                              QPoint &resultPos)
{
    if (!window || !widgetSize.isValid() || widgetSize.isEmpty()) {
        return false;
    }

    int lw = window->currentWidgetsLayoutWidth();
    int lh = window->currentWidgetsLayoutHeight();
    if (lw <= 0 || lh <= 0) {
        if (!window->mainView()) {
            return false;
        }
        const QRect g = window->mainView()->geometry();
        lw = qMax(1, g.width());
        lh = qMax(1, g.height());
    }

    const int margin = 8;
    const int minX = margin;
    const int minY = margin;
    const int maxX = qMax(margin, lw - widgetSize.width() - margin);
    const int maxY = qMax(margin, lh - widgetSize.height() - margin);
    auto clampPos = [minX, minY, maxX, maxY](const QPoint &p) {
        return QPoint(qBound(minX, p.x(), maxX), qBound(minY, p.y(), maxY));
    };

    auto canPlace = [window, widgetSize](const QPoint &p) {
        return !widgetRectOverlapsExisting(QRect(p, widgetSize), window);
    };

    const QPoint basePos = clampPos(preferredPos);
    if (canPlace(basePos)) {
        resultPos = basePos;
        return true;
    }

    const int step = qBound(8, qMin(widgetSize.width(), widgetSize.height()) / 4, 48);
    const int maxRadius = qMax(lw, lh);
    for (int radius = step; radius <= maxRadius; radius += step) {
        for (int offset = -radius; offset <= radius; offset += step) {
            const QPoint top = clampPos(basePos + QPoint(offset, -radius));
            if (canPlace(top)) {
                resultPos = top;
                return true;
            }
            const QPoint bottom = clampPos(basePos + QPoint(offset, radius));
            if (canPlace(bottom)) {
                resultPos = bottom;
                return true;
            }
            const QPoint left = clampPos(basePos + QPoint(-radius, offset));
            if (canPlace(left)) {
                resultPos = left;
                return true;
            }
            const QPoint right = clampPos(basePos + QPoint(radius, offset));
            if (canPlace(right)) {
                resultPos = right;
                return true;
            }
        }
    }

    return false;
}

QList<PlatformWindow *> PlatformWindow::allWindows()
{
    return gWindows;
}

bool PlatformWindow::isPrimary() const
{
    return m_screen && ScreenManager::getInstance()->isPrimaryScreen(m_screen);
}

PlatformWindow::PlatformWindow(int id,  QScreen* screen, QWindow *parent)
    : UkuiQuick::IslandView("desktop-view", "peony-qt-desktop")
{
    gWindows.append(this);
    setParent(parent);
    setScreen(screen);

    setColor(Qt::transparent);
    setFlags(flags() | Qt::FramelessWindowHint);
    m_layoutDelayer = new QTimer;
    m_layoutDelayer->setInterval(30);
    m_layoutDelayer->setSingleShot(true);
    connect(m_layoutDelayer, &QTimer::timeout, this, [=]{
        updateDesktopWidgetsUsedRegionInstantly();
    });

#ifdef KY_SDK_WAYLANDHELPER
    m_wmHelper = new UkuiWindowHelper(this);
    m_wmHelper->setWindowRole(UkuiWindowHelper::WindowRole::Desktop);
    m_wmHelper->setSkipSwitcher(true);
    m_wmHelper->setSkipTaskBar(true);
#endif

    qInfo() << "create new window on "<<screen->name()<<id<<screen->geometry() <<id;
    m_screen = screen;
    setGeometry(screen->geometry());

    m_model = PeonyDesktopApplication::getModel();
    m_proxyModel = new DesktopItemProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setId(id);
    m_proxyModel->setInstanceId(-1);

    connect(m_proxyModel, &DesktopItemProxyModel::rowsInserted, this, &PlatformWindow::rowsInserted);
    connect(m_model, &AdvancedDesktopItemModel::sig_fileRemoved, this, &PlatformWindow::fileRemoved);
    connect(m_model, &AdvancedDesktopItemModel::refreshed, this, &PlatformWindow::reloadData);
    connect(m_model, &AdvancedDesktopItemModel::idChanged, this, &PlatformWindow::idChanged);
    connect(m_model, &AdvancedDesktopItemModel::invalidateFilterRequest, m_proxyModel, &DesktopItemProxyModel::invalidateModel);
     connect(m_model, &AdvancedDesktopItemModel::dragFromOtherViewAutoLayout, m_proxyModel, &DesktopItemProxyModel::updateIconPositionsByOtherView);

    connect(m_proxyModel, &DesktopItemProxyModel::datechange, m_model, &Peony::AdvancedDesktopItemModel::setdata);
    connect(m_proxyModel, &DesktopItemProxyModel::showHiddenFile, this, &PlatformWindow::showHiddenFile);
    connect(m_proxyModel, &DesktopItemProxyModel::showStandardFile, this, &PlatformWindow::showStandardFile);
    connect(m_proxyModel, &DesktopItemProxyModel::drop, this, &PlatformWindow::drop);

    connect(this, &PlatformWindow::viewUpdated, m_proxyModel, &DesktopItemProxyModel::viewUpdated);

    FileActionController *controller = FileActionController::getInstance();
    GlobalSettingsAdapter *settingsAdapter = GlobalSettingsAdapter::getInstance();
    DesktopWindowManager *windowManager = DesktopWindowManager::getInstance();

#ifdef TEST_PEONY_DESKTOP_WIDGETS
    connect(controller, &FileActionController::requestManageWidgets, this, [=]{
        auto button = QMessageBox::question(0, 0, "是否想移除插件？");
        if (button == QMessageBox::Yes || button == QMessageBox::Ok) {
            auto widgets = this->mainView()->widgets();
            QMessageBox::information(0, 0, "选择想要移除的插件");
            auto listWidget = new QListWidget;
            for (auto widget : widgets) {
                QString widgetInfo = QString("%1: %2").arg(widget->instanceId()).arg(widget->id());
                listWidget->addItem(widgetInfo);
                auto item = listWidget->item(listWidget->count() - 1);
                item->setData(Qt::UserRole, widget->instanceId());
            }
            listWidget->show();
            connect(listWidget, &QListWidget::doubleClicked, listWidget, [=](QModelIndex index){
                auto item = listWidget->item(index.row());
                int instanceId = item->data(Qt::UserRole).toInt();
                mainView()->removeWidget(mainView()->widget(instanceId));
                mainView()->config()->removeChild("widgets", instanceId);
                listWidget->close();
            });
        } else {
            button = QMessageBox::question(0, 0, "是否想要添加插件？");
            if (button == QMessageBox::Yes || button == QMessageBox::Ok) {
                QMessageBox::information(0, 0, "选择插件所在目录");
                auto pluginDirString = QFileDialog::getExistingDirectory(0, 0, "/usr/share/ukui/widgets");
                if (pluginDirString.isEmpty()) {
                    return;
                }
                QDir pluginDir(pluginDirString);
                qDebug() << pluginDir;
                int count = mainView()->widgets().count();
                mainView()->addWidget(pluginDir.dirName(), DesktopWidgetsManager::getInstance()->getNextInstanceId());
            }
        }
    });
#endif

    rootContext()->setContextProperty("platformWindow", this);
    rootContext()->setContextProperty("mymodel", m_model);
    rootContext()->setContextProperty("myProxyModel", m_proxyModel);
    rootContext()->setContextProperty("controller", controller);
    rootContext()->setContextProperty("globalSettings", settingsAdapter);
    rootContext()->setContextProperty("backgroundManager", BackgroundManager::globalInstance());
    rootContext()->setContextProperty("themeStyle", ThemeStyle::instance());
    rootContext()->setContextProperty("windowManager", windowManager);
    rootContext()->setContextProperty("ManagerControl", Peony::ManagerControl::getInstance());

    m_gridSize = controller->getGridSize();
    QMargins margins = GlobalSettingsAdapter::getInstance()->panelMargins();
    QRect rect = screen->geometry();
    m_availableColumnCount = (rect.width() - margins.left() - margins.right()) / m_gridSize.width();
    m_availableRowCount = (rect.height() - margins.top() - margins.bottom()) / m_gridSize.height();

    QObject::connect(controller, &FileActionController::gridSizeChanged,
                     this, [=](){
        m_gridSize = controller->getGridSize();

        ajustIconsOnScreenSize();
        checkItemsOver();
    });

    connect(m_model, &AdvancedDesktopItemModel::refreshed, controller, &FileActionController::finishRefresh);
    // connect(this, &QQuickView::statusChanged, this, [=](QQuickView::Status status) {
    //      if (status == QQuickView::Ready) {
    //          qDebug() << "QML loaded, setting final geometry"<<screen->geometry();
    //          setGeometry(screen->geometry());
    //          if (rootObject()) {
    //              rootObject()->setState("Show");
    //          }
    //      }
    //  });

    connect(GlobalSettingsAdapter::getInstance(), &GlobalSettingsAdapter::valueChanged, this, [=](const QString &key){
        if (key == DESKTOP_USE_AUTO_LAYOUT) {
            bool desktopAutoLayout = GlobalSettingsAdapter::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool();
            if (desktopAutoLayout) {
                m_itemPosHash.clear();
                m_autoArrange.clear();
                m_storageBox.clear();
                m_resolutionItemPosHash.clear();
                m_resolutionItemNewPosHash.clear();
                auto sortedUris = layoutItems();
            }
        }
    });

    connect(GlobalSettingsAdapter::getInstance(), &GlobalSettingsAdapter::panelMarginsChanged, this, [=](){
        ajustIconsOnScreenSize();
        checkItemsOver();
    });

    // const QUrl url(QStringLiteral("qrc:/qml-desktop/DesktopView.qml"));
    // setSource(url);

    // setResizeMode(QQuickView::SizeRootObjectToView);
    qDebug() << "gridSize:"<<m_gridSize;

    bool loaded = loadMainViewWithoutItem("qml-desktop", id);
    // 教育工作台加载
    qApp->setProperty("ReOriganization", false);
    if (m_screen == qApp->primaryScreen() && DESKTOP_MANAGER->isEDUPlatform()) {
        mainView()->addWidget("org.ukui.kylinworkstation", WORKSTATION_ID);
        if (!DESKTOP_MANAGER->isWorkstationEnabled()) {
            mainView()->addDisableInstance(WORKSTATION_ID);
        }
        // 监听工作台enable状态
        connect(DESKTOP_MANAGER, &DesktopWindowManager::workstationEnabledChanged, this, [=](bool enabled){
            if (enabled) {
                mainView()->removeDisableInstance(WORKSTATION_ID);
                mainView()->addWidget("org.ukui.kylinworkstation", WORKSTATION_ID);
            } else {
                mainView()->removeWidget(WORKSTATION_ID);
                mainView()->addDisableInstance(WORKSTATION_ID);
            }
        });
    }
    bool needOriganization = GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool();
    if(needOriganization){
        // 此情况仅发生在桌面未启动，通过其它途径设置gsettings时
        if (!DesktopOrganizationManager::instance()->isOriganizationToolConfigExist()) {
            DesktopOrganizationManager::instance()->initOrganizationWidgets();
        }
    }
    if (loaded) {
        // 分段加载，确保updateDesktopWidgetsUsedRegionInstantly调用时containerwidget相关的资源已经就绪
        loadMainViewItem();
    }

    connect(FileActionController::getInstance(),&FileActionController::desktopOriganization, this, [=](){
        // 如果不是首次创建，只需要恢复之前状态
        //DesktopOrganizationManager::instance()->setConfigStatus(false);
        if (DesktopOrganizationManager::instance()->isOriganizationToolConfigExist()) {
            //新屏幕需要添加初始化分区
            if(!checkIsOrgconfigExist()){
                initDesktopOrgWidgets();
            }
            DesktopOrganizationManager::instance()->setDesktopOrganizationWidgetEnabled(this, true);
            // todo: 对于在禁用自动整理时新增的文件，需要整理到对应分区，如果禁用之前没有对应分区，则应该新建对应分区后将这些文件放入
            m_proxyModel->updateFilter();
        } else {
            // initOrganizationWidgets会对当前所有的platformwindow创建自动分区widget，并且会将isOriganizationToolConfigExist设为true
            DesktopOrganizationManager::instance()->initOrganizationWidgets();
            bool ismirrorModel = SCREEN_MANAGER->displayMode() == ScreenManager::DisplayMode::Mirror;
            if(ismirrorModel){
                //镜像模式开启整理工具
                qApp->setProperty("initOrgToolmirror",true);
            }
        }
        m_itemPosHash.clear();
        m_autoArrange.clear();
        for (int row = 0; row < m_proxyModel->rowCount(); row++) {
            auto index = m_proxyModel->index(row, 0, QModelIndex());
            QString uri = index.data(UriRole).toString();
            m_autoArrange.append(uri);
        }
        //m_autoArrange.clear();
    });
    connect(DesktopOrganizationManager::instance(),&DesktopOrganizationManager::checkorganizationExistRequest,this,[=](){
        bool  useDesktopOriganization =  GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool();
        if (DesktopOrganizationManager::instance()->isOriganizationToolConfigExist() && useDesktopOriganization) {
            //新屏幕需要添加初始化分区
            if(!checkIsOrgconfigExist()){
                initDesktopOrgWidgets();
            }
        }
    });
    connect(DesktopOrganizationManager::instance(),&DesktopOrganizationManager::addinitWidgtRequest, this, [=](){
        bool  useDesktopOriganization =  GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool();
        bool isinitOrgToolmirror = qApp->property("initOrgToolmirror").toBool();
        if(id!=0 && isinitOrgToolmirror && useDesktopOriganization){
            if(isinitOrgToolmirror){
                qApp->setProperty("initOrgToolmirror",false);
            }
            auto widgets = mainView()->widgets();
            // 通过container config读取对应子widget的config，通过id和org type判断是否需要启用
            for (auto widget : widgets) {
                auto config = widget->config();
                if(config==nullptr){
                    continue;
                }
                if (config->getValue("id").toString() == "desktop-organization-tool") {
                    int orgType = config->getValue("organizationType").toInt();
                    if (orgType >= 0 && orgType <= 2) {
                        auto urilist = DesktopOrganizationManager::instance()->getUrisByOrganizationType(orgType,id);
                        if(!urilist.isEmpty()){
                            for(auto uri :urilist){
                                auto metaInfo = FileMetaInfo::fromUri(uri);
                                if (metaInfo) {
                                    metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, widget->instanceId());
                                    metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, widget->instanceId());
                                }
                            }
                        }
                        QTimer::singleShot(100, this, [=](){
                            Q_EMIT DesktopOrganizationManager::instance()->requestUpdateOrganizationView(widget->instanceId());
                        });
                    }
                }
            }
        }
        if(id ==0 && useDesktopOriganization){
            auto widgets = mainView()->widgets();
            for(auto widget:widgets){
                int instanceid = widget->instanceId();
                QTimer::singleShot(150, this, [=](){
                    Q_EMIT DesktopOrganizationManager::instance()->requestUpdateOrganizationView(instanceid);
                });
            }
        }
    });
    connect(FileActionController::getInstance(),&FileActionController::quitOriganization, this, [=](){
        if(loaded){
            DesktopOrganizationManager::instance()->setDesktopOrganizationWidgetEnabled(this, false);
        }
        DesktopOrganizationManager::instance()->RestoreDesktopInfo();
        m_proxyModel->updateFilter();
        reloadData();
    });

    connect(DesktopOrganizationManager::instance(),&DesktopOrganizationManager::updateDesktopView,this,[=](){
        m_proxyModel->updateFilter();
    });
    showFullScreen();

    ScreenManager::getInstance()->addBackgroundWindow(this, screen, id);

    connect(ScreenManager::getInstance(), &ScreenManager::primaryScreenChanged,
            this, &PlatformWindow::isPrimaryChanged);

    connect(ScreenManager::getInstance(),&ScreenManager::screenRemovedFromExtend,this,[=](QScreen* screen, int screenid){
          if(screenid == this->id()){
            auto widgets = mainView()->widgets();
            for (auto widget:widgets) {
                int instanceid = widget->instanceId();
                int orgtype = widget->config()->getValue("organizationType").toInt();
                if(orgtype>=0 && orgtype<=2){
                    mergeSameTypeWidgetToPrimaryScreen(this->id(),orgtype,instanceid);
                }
            }
        }
    });
    connect(DesktopWindowManager::getInstance(),&DesktopWindowManager::singleScreeModeRequest,this,[=](){
        if(0 != this->id()){
            auto widgets = mainView()->widgets();
            for (auto widget : widgets) {
                int instanceid = widget->instanceId();
                int orgtype = widget->config()->getValue("organizationType").toInt();
                if(orgtype>=0 && orgtype<=2){
                    mergeSameTypeWidgetToPrimaryScreen(this->id(),orgtype,instanceid);
                }
            }
        }
    });
    QTimer::singleShot(100, this, [this]() {
        int rowCount = m_proxyModel->rowCount();
        for (int row = 0; row < rowCount; row++) {
            auto index = m_proxyModel->index(row, 0, QModelIndex());
            QString uri = index.data(UriRole).toString();
            QPoint restorepoint;
            auto metaInfo = FileMetaInfo::fromUri(uri);
            // if (!m_resolutionItemNewPosHash.keys().contains(uri)) {
            QStringList restorepos;
            if (metaInfo) {
                restorepos = metaInfo->getMetaInfoStringList(RESTORE_DRAG_ITEM_GRID_POS_ATTRIBUTE);
            }
            if(restorepos.size() == 3){
                restorepoint = QPoint(restorepos[0].toInt(),restorepos[1].toInt());
                m_resolutionItemNewPosHash.insert(uri,restorepoint);
            }
        }
    });
}

PlatformWindow::~PlatformWindow()
{
    gWindows.removeOne(this);
}

void PlatformWindow::rowsInserted(const QModelIndex &parent, int first, int last)
{
    QString firstEditUri;
    QStringList& editUris = FileActionController::getInstance()->urisToEdit();
    if (editUris.isEmpty()) {
        return;
    }
    firstEditUri = Peony::FileUtils::urlDecode(editUris.first());
    for (int row = first; row <= last; ++row) {
        QModelIndex proxyIndex = m_proxyModel->index(row, 0, parent);
        QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

        QString uri = m_model->data(sourceIndex, UriRole).toString();
        qDebug() << "Inserted item (source):" << uri;
        QString infoUri = Peony::FileUtils::urlDecode(uri);
        qDebug() << "fileCreated editUri:"<<firstEditUri<<infoUri;
        if (firstEditUri == infoUri ) {
            QTimer::singleShot(100, this, [=]() {
                Q_EMIT FileActionController::getInstance()->edit(uri);
            });
            editUris.clear();
        }
    }
}

void PlatformWindow::updateWindowGeometry()
{
    if (!m_screen) {
        qWarning() << "PlatformaWindow Screen is null.";
        return;
    }

    qDebug() << "displayMode:"<<ScreenManager::getInstance()->displayMode();

    if (m_screen != this->screen()) {
        setScreen(m_screen);
    }

    if (ScreenManager::getInstance()->displayMode() == ScreenManager::Mirror && m_proxyModel->id() != 0) {
        qDebug() << "geometry of screen when Mirror "<<m_screen->geometry()<<this->geometry();
         //镜像下也需要更新位置，切换扩展或者单屏下可能不会有geometry信号变化
        this->setGeometry(m_screen->geometry());
        this->ajustIconsOnScreenSize();
        lower();
#ifndef KY_SDK_WAYLANDHELPER
        ScreenManager::getInstance()->getWindowById(0)->raise();
#else
        ScreenManager::getInstance()->getWindowById(0)->requestActivate();
#endif
        return;
    }
    if (m_proxyModel->id() == 0) {
#ifndef KY_SDK_WAYLANDHELPER
        raise();
#else
        this->requestActivate();
#endif
    }
    qDebug() << "geometry: "<<m_screen->geometry();

    this->setGeometry(m_screen->geometry());
    this->ajustIconsOnScreenSize();
    this->requestUpdate();
    relocateWidgetPosition();
    updateDesktopWidgetsUsedRegion();
    if (ScreenManager::getInstance()->displayMode() == ScreenManager::Extend) {
        this->requestActivate();
    }
}

void PlatformWindow::resizeEvent(QResizeEvent *ev)
{
    UkuiQuick::IslandView::resizeEvent(ev);

    if (!m_screen) {
        qWarning() << "PlatformaWindow Screen is null.";
        return;
    }

    qDebug() << "resize:" << this->screen()->geometry() << this->screen() << m_proxyModel->id()
             << geometry() << bindScreen()->geometry() << bindScreen();

    if (m_screen != this->screen())
        setScreen(m_screen);

    if (geometry() != m_screen->geometry())
        setGeometry(m_screen->geometry());
}

void PlatformWindow::updateView()
{
    m_itemPosHash.clear();
    m_autoArrange.clear();
    m_storageBox.clear();
    m_resolutionItemPosHash.clear();
    m_resolutionItemNewPosHash.clear();
    m_proxyModel->updateFilter();

    this->ajustIconsOnScreenSize();
    this->checkItemsOver();

    Q_EMIT viewUpdated();
}

void PlatformWindow::ajustIconsOnScreenSize()
{
    QMargins margins = GlobalSettingsAdapter::getInstance()->panelMargins();
    QRect rect = m_screen->geometry();
    m_availableColumnCount = (rect.width() - margins.left() - margins.right()) / m_gridSize.width();
    m_availableRowCount = (rect.height() - margins.top() - margins.bottom()) / m_gridSize.height();

    qInfo()<<"ajust Icons pos, current geometry is "<<this->geometry()<<" row, col"<<m_availableColumnCount<<m_availableRowCount;

    m_itemPosHash.clear();
    if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
        if (m_autoArrange.count() != m_proxyModel->rowCount()) {
            layoutItems();
        } else {
            relayoutExsitingItemsAndUpdate(m_autoArrange);
        }
        return;
    }
    QStringList needChanged;
    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        QString uri = index.data(UriRole).toString();
        QPoint pos = index.data(PositionRole).toPoint();
        m_itemPosHash.insert(uri, pos);
        if (isInvalidPoint(pos)) {
            needChanged.append(uri);
            if (!m_resolutionItemPosHash.contains(uri)) {
                // remember item position before resolution changed.
                m_resolutionItemPosHash.insert(uri, pos);
            }
        }
    }

    qInfo()<<"need changed item"<<needChanged << " resolution item" << m_resolutionItemPosHash << id();

    if (!needChanged.isEmpty()) {
        qInfo()<<"屏幕过小，有元素超过屏幕范围" ;
        relayoutExsitingItems(needChanged);
    } else {
        qInfo()<<"没有元素超过屏幕范围， 尝试恢复超过屏幕范围的元素" <<m_resolutionItemPosHash;
        QStringList itemNeedBeRelayout;

        QList<QPair<QPoint, QString>> newPosition;
        for (auto i = m_resolutionItemPosHash.constBegin(); i != m_resolutionItemPosHash.constEnd(); ++i) {
            newPosition << QPair<QPoint, QString>(i.value(), i.key());
        }

        std::stable_sort(newPosition.begin(), newPosition.end(), iconSizeLessThan);

        for (auto pair : newPosition) {
            QPoint pos = pair.first;
            QString uri = pair.second;
            m_storageBox.removeOne(uri);

            if (!m_itemPosHash.contains(uri)) {
                m_resolutionItemPosHash.remove(uri);
                continue;
            }

            if (isInvalidPoint(pos)) {
                itemNeedBeRelayout.append(uri);
            } else if (!m_itemPosHash.values().contains(pos)) {
                m_resolutionItemPosHash.remove(uri);
                m_itemPosHash.insert(uri, pos);
            }
        }
        itemNeedBeRelayout.append(m_storageBox);
        m_storageBox.clear();

        relayoutExsitingItems(itemNeedBeRelayout);
    }

    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        QString uri = index.data(UriRole).toString();
        QPoint pos = m_itemPosHash.value(uri);
        int id = index.data(ScreenIdRole).toInt();
        m_proxyModel->setData(index, pos, PositionRole);
        setFileMetaInfoPos(uri, pos);
        if (m_resolutionItemPosHash.contains(uri)) {
            QPoint reolutionItemPos = m_resolutionItemPosHash.value(uri);
            QStringList exceptedPos;
            exceptedPos<<QString::number(reolutionItemPos.x());
            exceptedPos<<QString::number(reolutionItemPos.y());
            exceptedPos<<QString::number(id);
            m_proxyModel->setData(index, exceptedPos, ExceptedPositionRole);
        }
    }
}

void PlatformWindow::relayoutExsitingItems(const QStringList &uris)
{
    // fixme: 如何确保uris的顺序是按位置信息排列的？可能需要在调用端做调整
    if (uris.isEmpty() || m_itemPosHash.isEmpty()) {
        return;
    }
    for (auto uri : uris) {
        m_itemPosHash.remove(uri);
    }
    auto allFileUris = getAllFileUris();

    QPoint startPos = QPoint();
    int i = 0;
    for (; i < uris.count(); i++) {
        QString uri = uris[i];
        if (!allFileUris.contains(uri))
            continue;
        startPos = findNextEmptyGridPos(startPos);
        if (isInvalidPoint(startPos)) {
            break;
        }
        setFileMetaInfoPos(uri, startPos);
        m_storageBox.removeOne(uri);
    }

    for(; i < uris.count(); i++) {
        QString uri = uris[i];
        setFileMetaInfoPos(uri, QPoint(0, 0));
        if (!m_storageBox.contains(uri)) {
            m_storageBox.append(uri);
        }
    }
}

void PlatformWindow::relayoutExsitingItemsForDesktopWidgets(const QStringList &uris)
{
    // 如果出现无法放置最终放置到0，0的点，记得将这些点的信息加入storage box

    for (auto uri: uris) {
        // 先将原位置信息缓存到还原hash中
        m_resolutionItemNewPosHash.insert(uri, m_itemPosHash.value(uri));

        // 把当前需要重排的item位置信息清空
        m_itemPosHash.remove(uri);
    }

    // 当第出现图标无法找到时，后续的计算都需要跳过
    bool noValidPos = false;

    // 重新放置图标
    for (auto uri: uris) {
        // 上一个图标已经无法放置，只需要把图标位置设为（0，0）
        if (noValidPos) {
            m_itemPosHash.insert(uri, QPoint());
            m_storageBox.append(uri);
            continue;
        }

        // 在未计算出可放置位置前认为没有可放置位置，计算出可放置位置后设置为false
        noValidPos = true;
        // 获取图标位置
        QPoint pos = m_resolutionItemNewPosHash.value(uri);
        // 确认遍历网格边界条件
        int minGridStep = 1;
        int maxGridStep = qMax(qMax(m_availableColumnCount - 1 - pos.x(), pos.x()), qMax(m_availableRowCount - 1 - pos.y(), pos.y()));
        // 在边界条件范围内计算可放置的位置
        for (int gridStep = minGridStep; gridStep <= maxGridStep; gridStep++) {
            // 这里使用偏移量的方式计算，以便于后续比较放置优先级
            QList<QPoint> tmpValidOffsetPoint;
            // 按上左下右边缘计算可放置位置

            // 计算上方边缘位置
            if (pos.y() - gridStep >= 0) {
                int offsetY = gridStep;
                for (int offsetX = gridStep; offsetX >= -gridStep; offsetX--) {
                    // 判断当前位置是否可放置

                    // 当前位置是否超出屏幕
                    if (pos.x() - offsetX < 0 || pos.x() - offsetX > m_availableColumnCount - 1) {
                        continue;
                    }

                    // 当前位置是否和插件相交
                    QPoint tmpGridPos = pos - QPoint(offsetX, offsetY);
                    if (checkIfDesktopWidgetsIntersected(tmpGridPos, true)) {
                        continue;
                    }

                    // 将当前可以放置点的位置偏移记录在比较列表中
                    tmpValidOffsetPoint.append(QPoint(offsetX, offsetY));
                }
            }

            // 计算左侧边缘位置，由于左上角已经计算过，并且之后下边缘还会计算左下角，这里就不计算左上左下角
            if (pos.x() - gridStep >= 0) {
                int offsetX = gridStep;
                for (int offsetY = gridStep - 1; offsetY >= -gridStep + 1; offsetY--) {
                    // 判断当前位置是否超出屏幕
                    if (pos.y() - offsetY < 0 || pos.y() - offsetY > m_availableRowCount - 1) {
                        continue;
                    }

                    // 当前位置是否和插件相交
                    QPoint tmpGridPos = pos - QPoint(offsetX, offsetY);
                    if (checkIfDesktopWidgetsIntersected(tmpGridPos, true)) {
                        continue;
                    }

                    // 将当前可以放置点的位置偏移记录在比较列表中
                    tmpValidOffsetPoint.append(QPoint(offsetX, offsetY));
                }
            }

            // 计算右侧边缘位置，类似左侧边缘计算
            if (pos.x() + gridStep <= m_availableColumnCount - 1) {
                int offsetX = -gridStep;
                for (int offsetY = gridStep - 1; offsetY >= -gridStep + 1; offsetY--) {
                    // 判断当前位置是否超出屏幕
                    if (pos.y() - offsetY < 0 || pos.y() - offsetY > m_availableRowCount - 1) {
                        continue;
                    }

                    // 当前位置是否和插件相交
                    QPoint tmpGridPos = pos - QPoint(offsetX, offsetY);
                    if (checkIfDesktopWidgetsIntersected(tmpGridPos, true)) {
                        continue;
                    }

                    // 将当前可以放置点的位置偏移记录在比较列表中
                    tmpValidOffsetPoint.append(QPoint(offsetX, offsetY));
                }
            }

            // 计算下方边缘位置，类似上方边缘位置计算
            if(pos.y() + gridStep <= m_availableRowCount - 1) {
                int offsetY = -gridStep;
                for (int offsetX = gridStep; offsetX >= -gridStep; offsetX--) {
                    // 判断当前位置是否可放置

                    // 当前位置是否超出屏幕
                    if (pos.x() - offsetX < 0 || pos.x() - offsetX > m_availableColumnCount - 1) {
                        continue;
                    }

                    // 当前位置是否和插件相交
                    QPoint tmpGridPos = pos - QPoint(offsetX, offsetY);
                    if (checkIfDesktopWidgetsIntersected(tmpGridPos, true)) {
                        continue;
                    }

                    // 将当前可以放置点的位置偏移记录在比较列表中
                    tmpValidOffsetPoint.append(QPoint(offsetX, offsetY));
                }
            }

            // 如果存在合适放置的位置，则更新itemPosHash，进入下一个uri重排
            if (!tmpValidOffsetPoint.isEmpty()) {
                std::stable_sort(tmpValidOffsetPoint.begin(), tmpValidOffsetPoint.end(), itemGridPosOffsetPosLesserThan);
                QPoint firstOffset = tmpValidOffsetPoint.first();
                QPoint targetPos = pos - firstOffset;
                m_itemPosHash.insert(uri, targetPos);
                noValidPos = false; // 找到pos，不执行放置到（0，0）流程
                break;
            }
        }

        if (noValidPos) {
            // 如果未找到uri可放置的位置，则放置到（0，0），并且将uri加入storage box
            m_itemPosHash.insert(uri, QPoint());
            m_storageBox.append(uri);
            noValidPos = true; //后续的图标排列不再进行计算
            continue;
        }
    }
}

const QStringList PlatformWindow::getAllFileUris()
{
    QStringList uris;
    for (int i = 0; i < m_proxyModel->rowCount(); i++) {
        auto index = m_proxyModel->index(i, 0);
        uris<<index.data(Qt::UserRole).toString();
    }
    return uris;
}

void PlatformWindow::setFileMetaInfoPos(const QString &uri, const QPoint &pos)
{
    m_itemPosHash.insert(uri, pos);
    QPoint currentPos(pos.x()*m_gridSize.width(), pos.y()*m_gridSize.height());
    QRect rect(mapToGlobal(currentPos)/**this->devicePixelRatio()*/, m_gridSize/**this->devicePixelRatio()*/);
    FileInfo::fromUri(uri).get()->setProperty("iconGeometry", rect);
}

QPoint PlatformWindow::findNextEmptyGridPos(const QPoint &startGridPos)
{
    for (int column = 0; column < m_availableColumnCount; column++) {
        for (int row = 0; row < m_availableRowCount; row++) {
            QPoint pos = QPoint(column, row);
            if (gridPosLesserThan(pos, startGridPos))
                continue;
            if (m_itemPosHash.keys(pos).isEmpty() && !checkIfDesktopWidgetsIntersected(pos)){
                return pos;
            }else if(!m_itemPosHash.keys(pos).isEmpty() && !checkIfDesktopWidgetsIntersected(pos)){
                auto uri = m_itemPosHash.key(pos);
                //如果已经收敛进去整理分区
                auto metaInfo = FileMetaInfo::fromUri(uri);
                if (metaInfo) {
                    int instanceid = metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID);
                    auto instanceidvariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID);
                    if(instanceidvariant.isValid() && instanceid!=-1){
                        return pos;
                    }
                }
            }
        }
    }
    return invalidPoint();
}

bool PlatformWindow::isInvalidPoint(const QPoint &point)
{
    return point.x() < 0 || point.x() >= m_availableColumnCount || point.y() < 0 || point.y() >= m_availableRowCount;
}

QPoint PlatformWindow::invalidPoint() const
{
    return QPoint(-1, -1);
}

QPoint PlatformWindow::getGridPosFromMousePos(const QPoint &mousePos)
{
    int x = mousePos.x()/m_gridSize.width();
//    if (layoutDirection() == Qt::RightToLeft) {
//        int vewportX = viewport()->rect().topRight().x();
//        x = (vewportX - mousePos.x())/m_gridSize.width();
//    }
    return QPoint(x, mousePos.y()/m_gridSize.height());
}

bool PlatformWindow::checkIfDesktopWidgetsIntersected(const QPoint &gridPos, bool considerIconItem)
{
    if (considerIconItem) {
        if (m_itemPosHash.values().contains(gridPos)) {
            return true;
        }
    }

    QRect itemRect = QRect(gridPos.x() * m_gridSize.width(), gridPos.y() * m_gridSize.height(), m_gridSize.width(), m_gridSize.height());
    itemRect.adjust(0, 0, -4, -4);
    if (m_desktopWidgetsUsedRegion.isEmpty()) {
        return false;
    } else {
        return m_desktopWidgetsUsedRegion.intersects(itemRect);
    }
}

bool PlatformWindow::gridPosLesserThan(const QPoint &leftPos, const QPoint &rightPos)
{
    if (leftPos.x() < rightPos.x())
        return true;

    if (leftPos.x() > rightPos.x())
        return false;

    if (leftPos.y() < rightPos.y())
        return true;
    return false;
}

void PlatformWindow::checkItemsOver()
{
    QStringList primaryItem, extendItem;
    QStringList needRelayoutItems;
    for (int i = 0; i < m_proxyModel->rowCount(); i++) {
        QModelIndex index = m_proxyModel->index(i, 0);
        QString uri = index.data(Qt::UserRole).toString();
        QPoint pos = index.data(PositionRole).toPoint();
        auto overUris = m_itemPosHash.keys(pos);
        if (overUris.count() > 1) {
            auto extendPos = index.data(ExtendScreenPositionRole).toStringList();
            if (3 == extendPos.count() && 0 != extendPos[2].toInt()) {
                extendItem.append(uri);
            } else {
                primaryItem.append(uri);
            }
        }
    }
    primaryItem<<extendItem;
    for (auto uri : primaryItem) {
        if (needRelayoutItems.contains(uri)) {
            continue;
        }
        auto pos = m_itemPosHash.value(uri);
        auto list = m_itemPosHash.keys(pos);
        list.removeAll(uri);

        //按照model的index排序
        QStringList orderedList;
        for (auto& primaryUri : primaryItem) {
            if (list.contains(primaryUri)) {
                orderedList.append(primaryUri);
            }
        }
        needRelayoutItems << orderedList;
    }

    needRelayoutItems.removeDuplicates();
    qDebug() << "checkitemsover" <<needRelayoutItems;

    if (0 == needRelayoutItems.size()) {
        return;
    }

    relayoutExsitingItems(needRelayoutItems);

    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        auto uri = index.data(UriRole).toString();
        auto pos = m_itemPosHash.value(uri);
        m_proxyModel->setData(index, pos, PositionRole);
        setFileMetaInfoPos(uri, pos);
    }
}

bool PlatformWindow::isFull()
{
    //bug#123045 主屏桌面空间不足，新建的文件未放置到扩展屏桌面上
    qDebug() << "colNum:" <<m_availableColumnCount << "rowNum:"<< m_availableRowCount << "total:";// << m_proxy_model->rowCount();
    for (int column = 0; column < m_availableColumnCount; column++) {
        for (int row = 0; row < m_availableRowCount; row++) {
            QPoint pos = QPoint(column, row);
            if (m_itemPosHash.keys(pos).isEmpty())
                return false;
        }
    }
    return true;
}

QPoint PlatformWindow::checkGridPos(const QPoint pos)
{
    if (!isInvalidPoint(pos)) {
        return pos;
    }
    int col = pos.x() >= m_availableColumnCount ? m_availableColumnCount-1 : pos.x();
    int row = pos.y() >= m_availableRowCount ? m_availableRowCount-1 : pos.y();
    QPoint newPos = findNextEmptyGridPos(QPoint(col, row));
    return newPos;
}

void PlatformWindow::loadData()
{
    reloadData();
}

void PlatformWindow::setSortOrder(int sortOrder)
{
    m_itemPosHash.clear();
    m_autoArrange.clear();
    m_storageBox.clear();
    m_resolutionItemPosHash.clear();
    m_proxyModel->setSortOrder(sortOrder);
    m_proxyModel->sort(0, Qt::SortOrder(sortOrder));
    // 排序需要清空restore pos信息
    m_proxyModel->clearExpectedItemPosForCurrentView();
    for (int i = 0; i < m_proxyModel->rowCount(); i++) {
        auto index = m_proxyModel->index(i, 0);
        QString uri = index.data(Qt::UserRole).toString();

        int col = i/m_availableRowCount;
        int row = i%m_availableRowCount;
        if (col >= m_availableColumnCount) {
            row = 0;
            col = 0;
            m_storageBox.append(uri);
        }
        QPoint pos(col, row);
        m_proxyModel->setData(index,pos,PositionRole);
        m_autoArrange.append(uri);
        setFileMetaInfoPos(uri, pos);
    }

    Q_EMIT m_proxyModel->layoutUpdate();

    updateDesktopWidgetsUsedRegion();
}

void PlatformWindow::setSortType(int sortType)
{
    m_itemPosHash.clear();
    m_autoArrange.clear();
    m_storageBox.clear();
    m_resolutionItemPosHash.clear();
    m_resolutionItemNewPosHash.clear();
    m_proxyModel->setSortType(sortType);
    m_proxyModel->sort(1);
    m_proxyModel->sort(0, Qt::SortOrder(m_proxyModel->getSortOrder()));
    // 排序需要清空restore pos信息
    m_proxyModel->clearExpectedItemPosForCurrentView();
    for (int i = 0; i < m_proxyModel->rowCount(); i++) {
        auto index = m_proxyModel->index(i, 0);
        QString uri = index.data(Qt::UserRole).toString();
        int col = i/m_availableRowCount;
        int row = i%m_availableRowCount;
        if (col >= m_availableColumnCount) {
            row = 0;
            col = 0;
           m_storageBox.append(uri);
        }
        QPoint pos(col, row);
        m_proxyModel->setData(index,pos,PositionRole);
        m_autoArrange.append(uri);
        setFileMetaInfoPos(uri, pos);
    }
    Q_EMIT m_proxyModel->layoutUpdate();

    updateDesktopWidgetsUsedRegion();
}

void PlatformWindow::setId(int id)
{
    m_proxyModel->setId(id);
    m_proxyModel->invalidate();
}

int PlatformWindow::id()
{
    return m_proxyModel->id();
}

int PlatformWindow::dataCount() const
{
    return m_proxyModel->rowCount();
}

void PlatformWindow::invalidate()
{
    return m_proxyModel->invalidate();
}

QStringList PlatformWindow::layoutItems(bool closest)
{
    if (!closest) {
        QHash<QString, QPoint> extendMap;
        QHash<QString, QPoint> primaryItemMap;
        QList<QPair<QPoint, QString>> newPosition;
        QStringList overItem;
        for (int row = 0; row < m_proxyModel->rowCount(); row++) {
            auto index = m_proxyModel->index(row, 0, QModelIndex());
            auto uri = index.data(UriRole).toString();
            QPoint pos = index.data(PositionRole).toPoint();
            auto extendPos = index.data(ExtendScreenPositionRole).toStringList();
            if (3 == extendPos.count() && 0 != extendPos[2].toInt()) {
                extendMap.insert(uri, pos);
            } else {
                if (primaryItemMap.keys(pos).empty()) {
                    newPosition << QPair<QPoint, QString>(pos, uri);
                    primaryItemMap.insert(uri, pos);
                } else {
                    overItem.append(uri);
                }
            }
        }

        for (QString uri : extendMap.keys()) {
            QPoint pos = extendMap.value(uri);
            if (primaryItemMap.keys(pos).isEmpty()) {
                newPosition << QPair<QPoint, QString>(pos, uri);
            } else {
                overItem.append(uri);
            }
        }

        if (!newPosition.isEmpty()) {

            std::stable_sort(newPosition.begin(), newPosition.end(), iconSizeLessThan);

            QStringList sortedUris;
            for (auto pair : newPosition) {
                if (m_storageBox.contains(pair.second)) {
                    overItem.prepend(pair.second);
                } else {
                    sortedUris.append(pair.second);
                }
            }
            sortedUris << overItem;
            sortedUris.removeDuplicates();
            qDebug() <<"--------------------layoutItems" <<sortedUris;
            relayoutExsitingItemsAndUpdate(sortedUris, false);
            return sortedUris;
        }
    } else {
        qInfo()<<"not supported yet";
    }
    return QStringList();
}

void PlatformWindow::relayoutExsitingItemsAndUpdate(const QStringList uris, bool forceUpdate)
{
    if (m_availableColumnCount == 1 && m_availableRowCount == 1)
        return;
    m_itemPosHash.clear();
    m_autoArrange.clear();
    m_storageBox.clear();
    int index = 0;
    //按照行和列顺序放置uri
    for (int column = 0; column < m_availableColumnCount && index < uris.size(); column++) {
        for (int row = 0; row < m_availableRowCount && index < uris.size(); row++) {
            QPoint pos = QPoint(column, row);
            if (checkIfDesktopWidgetsIntersected(pos)) {
                continue;
            }
            auto uri = uris[index];
            setFileMetaInfoPos(uri, pos);
            m_autoArrange.append(uri);
            index++;
        }
    }

    for (; index < uris.size(); index++) {
        //满屏暂时都放到（0，0）位置
        auto uri = uris[index];
        QPoint pos = QPoint(0, 0);
        setFileMetaInfoPos(uri, pos);
        m_autoArrange.append(uri);
        m_storageBox.append(uri);
    }

    m_autoArrange.removeDuplicates();
    if (forceUpdate)
        m_proxyModel->invalidate();
    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        auto uri = index.data(UriRole).toString();
        if(!uris.contains(uri)) {
            qDebug() << "uri does exist:" <<uri;
            continue;
        }

        auto pos = m_itemPosHash.value(uri);
        m_proxyModel->setData(index, pos, PositionRole);
    }
    qDebug() << "PlatformWindow::relayoutExsitingItemsAndUpdat---------" <<m_itemPosHash;
}

void PlatformWindow::showHiddenFile()
{
    if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
        if (m_autoArrange.count() != m_proxyModel->rowCount()) {
            layoutItems();
        }
        return;
    }

    // 非自动布局模式下，优先保留现有位置信息
    QHash<QString, QPoint> oldItemPosHash = m_itemPosHash;
    m_itemPosHash.clear();
    m_autoArrange.clear();
    QStringList needChanged;
    // 首先处理现有的图标位置，保持它们的位置优先
    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        QString uri = index.data(UriRole).toString();
        QPoint metaPos = index.data(PositionRole).toPoint();

        if (oldItemPosHash.contains(uri)) {
            m_itemPosHash.insert(uri, metaPos);
        } else {
            //新增隐藏图标与现有图标冲突，重排隐藏图标
            if (isInvalidPoint(metaPos) || !oldItemPosHash.keys(metaPos).isEmpty() || !m_itemPosHash.keys(metaPos).isEmpty()) {
                needChanged.append(uri);
            } else {
                m_itemPosHash.remove(uri);
                setFileMetaInfoPos(uri, metaPos);
            }
        }
    }
    qInfo()<<"need changed item"<<needChanged;
    relayoutExsitingItems(needChanged);

    for (auto uri : needChanged) {
        if (uri.isEmpty())
            continue;
        QModelIndex index = m_model->indexFromUri(uri);
        QPoint pos = m_itemPosHash.value(uri);
        m_model->setData(index, pos, PositionRole);
    }
    return;
}

void PlatformWindow::showStandardFile()
{
    if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
        if (m_autoArrange.count() != m_proxyModel->rowCount()) {
            layoutItems();
            return;
        }
        return;
    }
    ajustIconsOnScreenSize();
    checkItemsOver();
    return;
}

bool PlatformWindow::fileRemoved(const QString &uri)
{
    qDebug() << "file removed"<<uri;
    bool isExist = m_autoArrange.contains(uri)? true : false;
    QPoint emptyPos = m_itemPosHash.value(uri);
    m_itemPosHash.remove(uri);
    m_resolutionItemPosHash.remove(uri);
    m_resolutionItemNewPosHash.remove(uri);
    m_autoArrange.removeOne(uri);
    bool inBox = m_storageBox.removeOne(uri);

    if (GlobalSettingsAdapter::getInstance()->useAutoLayout() && isExist) {
        relayoutExsitingItemsAndUpdate(m_autoArrange, false);
        return true;
    }
    //重叠的重新分位置
    if (m_storageBox.isEmpty())
        return false;
    if (inBox)
        return true;

    QString relayoutUri =  m_storageBox.takeLast();
    if (!m_itemPosHash.contains(relayoutUri)) {
        return false;
    }
    QModelIndex index = m_model->indexFromUri(relayoutUri);
    m_model->setData(index, emptyPos, PositionRole);
    setFileMetaInfoPos(relayoutUri, emptyPos);
    return true;
}

bool PlatformWindow::filesRemoved(const QStringList &uris)
{
    qDebug() << "files removed"<<uris;
    bool isExist = false;
    QList<QPoint> emptyPosList;
    for (QString uri : uris) {
        isExist = m_autoArrange.contains(uri)? true : isExist;
        bool inBox = m_storageBox.removeOne(uri);
        if (!inBox) {
            QPoint pos = m_itemPosHash.value(uri);
            emptyPosList.append(pos);
        }
        m_itemPosHash.remove(uri);
        m_resolutionItemPosHash.remove(uri);
        m_resolutionItemNewPosHash.remove(uri);
        m_autoArrange.removeOne(uri);
    }

    if (GlobalSettingsAdapter::getInstance()->useAutoLayout() && isExist) {
        relayoutExsitingItemsAndUpdate(m_autoArrange, false);
        return isExist;
    }

    for (QPoint emptyPos : emptyPosList) {
        //重叠的重新分位置
        if (m_storageBox.isEmpty())
            return true;

        QString relayoutUri =  m_storageBox.takeLast();
        if (!m_itemPosHash.contains(relayoutUri)) {
            return false;
        }
        QModelIndex index = m_model->indexFromUri(relayoutUri);
        m_model->setData(index, emptyPos, PositionRole);
        setFileMetaInfoPos(relayoutUri, emptyPos);
    }
    return isExist;
}

bool PlatformWindow::idChanged(int id, int oldId, const QStringList &uris)
{
    qDebug()<<"file created "<< id << " uri:" <<uris<<" current id" <<m_proxyModel->id()<<oldId;
    if (oldId != id && oldId == m_proxyModel->id()) {
        if (-1 == id ) {
            //移除oldId的屏幕，更新oldId的图标缓存
            m_proxyModel->invalidate();
            filesRemoved(uris);
            return true;
        }
    }
    if (id != m_proxyModel->id() || uris.isEmpty()) {
        return false;
    }
//    m_proxyModel->invalidate();
    bool forceUpdate = oldId != id ? true : false;
    for (auto uri : uris) {
        if (uri.isEmpty())
            continue;
    auto index = m_model->indexFromUri(uri);
    QPoint metaPos = m_model->data(index,PositionRole).toPoint();
    if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
        if (!m_autoArrange.contains(uri)) {
            //自动排序下重命名导致出现重叠现象
            if (isInvalidPoint(metaPos)) {
                m_autoArrange.append(uri);
            } else {
                int i = metaPos.x() * m_availableRowCount +  metaPos.y();
                m_autoArrange.insert(i, uri);
            }
            relayoutExsitingItemsAndUpdate(m_autoArrange, forceUpdate);
            return true;
        }
        return false;
    }
    bool  isinorg = false;
    if(!m_itemPosHash.keys(metaPos).isEmpty()){
        QString uri = m_itemPosHash.key(metaPos);
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            int instanceid = metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID);
            auto instanceidvariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID);
            if(instanceidvariant.isValid() && instanceid!=-1){
                isinorg = true;
            }
        }
    }
    if (isInvalidPoint(metaPos) || (!m_itemPosHash.keys(metaPos).isEmpty() && !isinorg)) {
        QPoint emptypos = findNextEmptyGridPos();
        if (emptypos.x() < 0) {
            qWarning()<<"no empty pos";
            //如果没有空位则后面的item不排序
            emptypos = QPoint(0, 0);
            if (!m_storageBox.contains(uri))
                m_storageBox.append(uri);
        }
        metaPos = emptypos;
        m_model->setData(index, metaPos, PositionRole);
        setFileMetaInfoPos(uri, metaPos);
    } else {
        m_itemPosHash.remove(uri);
        setFileMetaInfoPos(uri, metaPos);
    }
    m_model->setData(index, m_proxyModel->id(), ScreenIdRole);
    }
    if (forceUpdate)
        m_proxyModel->invalidate();
    return true;
}

void PlatformWindow::drop(const QStringList &selectedUris, Qt::DropAction action, int insertRow, const QPoint &offPos)
{
    bool isExist = getAllFileUris().contains(selectedUris.first());
    if (isExist) {
        if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
            if (m_autoArrange.empty()) {
                qCritical()<<"sort list is empty"<<m_autoArrange<<"selected uris:"<<selectedUris;
                layoutItems();
                return;
            }
            QStringList destSortedUris = m_autoArrange;
            QStringList draggedUris;
            for (int i = 0; i < destSortedUris.count(); i++) {
                draggedUris.append(QString());
            }
            qDebug()<<"current uris:"<<m_autoArrange<<"selected uris:"<<selectedUris;
            for (QString uri : selectedUris) {
                int pos = destSortedUris.indexOf(uri);
                if (-1 == pos) {
                    qCritical()<<"get position of uri is -1:"<<m_autoArrange<<"currrent selected uri:"<<uri;
                    continue;
                }
                destSortedUris.replace(pos, QString());
                draggedUris.replace(pos, uri);
            }

            draggedUris.removeAll(QString());

            if (-1 == insertRow) {
                destSortedUris += draggedUris;
            } else {
                auto index = m_proxyModel->index(insertRow, 0, QModelIndex());
                QString currentHoverUri = index.data(UriRole).toString();
                int insertedPos = destSortedUris.indexOf(currentHoverUri);
                qDebug()<<"insert pos: "<<insertedPos<<currentHoverUri;
                if (insertedPos >= 0 && insertedPos <= destSortedUris.size()) {
                    destSortedUris.reserve(destSortedUris.size() + draggedUris.size());
                    for (const QString &uri : draggedUris) {
                        destSortedUris.insert(insertedPos++, uri);
                    }
                } else if (insertedPos < 0){
                    destSortedUris = m_autoArrange;
                } else {
                    destSortedUris += draggedUris;
                }
            }

            destSortedUris.removeAll(QString());
            destSortedUris.removeDuplicates();
            qDebug()<<"relayout: "<<destSortedUris;

            relayoutExsitingItemsAndUpdate(destSortedUris, false);
        } else {
            qDebug() << "非自动排序 拖拽:" <<selectedUris<< " 行为："<<action<<"插入行:"<<insertRow<<"偏移量："<<offPos;
            for (QString uri : selectedUris) {
                m_itemPosHash.remove(uri);
            }
            QHash<QString, QModelIndex> relayoutIndexes;
            for (int row = 0; row < m_proxyModel->rowCount(); row++) {
                auto index = m_proxyModel->index(row, 0, QModelIndex());
                QString uri = index.data(UriRole).toString();
                if (selectedUris.contains(uri)) {
                    QPoint pos = index.data(PositionRole).toPoint();
                    if (!isInvalidPoint(pos)) {
                        qDebug() << " 拖拽:" <<uri<< " 拖拽前坐标："<<pos;
                        pos += offPos;
                        qDebug() << " 拖拽后坐标："<<pos;
                        if (isInvalidPoint(pos) || !m_itemPosHash.keys(pos).isEmpty()) {
                            relayoutIndexes.insert(uri, index);
                            continue;
                        }
                    } else {
                        relayoutIndexes.insert(uri, index);
                        continue;
                    }
                    setFileMetaInfoPos(uri, pos);
                    m_proxyModel->setData(index, pos, PositionRole);
                }
            }
            for (QString uri : relayoutIndexes.keys()) {
                auto index = relayoutIndexes.value(uri);
                QPoint dropedGridPos = index.data(PositionRole).toPoint();
                if (!isInvalidPoint(dropedGridPos) && m_itemPosHash.keys(dropedGridPos+offPos).isEmpty()) {
                    dropedGridPos += offPos;
                }
                QPoint pos = findNextEmptyGridPos(dropedGridPos);
                if (!isInvalidPoint(pos)) {
                    setFileMetaInfoPos(uri, pos);
                    m_proxyModel->setData(index, pos, PositionRole);
                }
            }
        }

    } else {
        dragFromOtherScreen(selectedUris, insertRow, offPos);
    }

}

bool PlatformWindow::dragFromOtherScreen(QStringList selectedUris, int insertRow, const QPoint &offPos)
{
    int oldId = 0;
    if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
        qDebug() << "跨屏 自动排序 拖拽:" <<selectedUris<<"插入行:"<<insertRow<<"偏移量："<<offPos;

        for (int row = 0; row < m_model->rowCount(); row++) {
            auto index = m_model->index(row, 0, QModelIndex());
            QString uri = index.data(UriRole).toString();
            if (selectedUris.contains(uri)) {
                oldId = m_model->data(index, ScreenIdRole).toInt();
                m_model->setData(index, m_proxyModel->id(), ScreenIdRole);
            }
        }
        QStringList destSortedUris = m_autoArrange;
        if (-1 == insertRow) {
            destSortedUris.append(selectedUris);
        } else {
            auto index = m_proxyModel->index(insertRow, 0, QModelIndex());
            QString currentHoverUri = index.data(UriRole).toString();
            int insertedPos = destSortedUris.indexOf(currentHoverUri);
            qDebug()<<"insert pos: "<<insertedPos<<currentHoverUri;
            if (insertedPos >= 0 && insertedPos <= destSortedUris.size()) {
                destSortedUris.reserve(destSortedUris.size() + selectedUris.size());
                for (const QString &uri : selectedUris) {
                    destSortedUris.insert(insertedPos++, uri);
                }
            }
        }
        destSortedUris.removeDuplicates();
        relayoutExsitingItemsAndUpdate(destSortedUris);
        if(!qApp->property("AutoLayoutByorg").toBool()){
            Q_EMIT m_model->idChanged(-1, oldId, selectedUris);
        }else{
            qApp->setProperty("AutoLayoutByorg",false);
        }
        return true;
    } else {
        qDebug() << "跨屏 非自动排序 拖拽:" <<selectedUris<<"插入行:"<<insertRow<<"偏移量："<<offPos;

        QHash<QString, QModelIndex> relayoutIndexes;
        for (int row = 0; row < m_model->rowCount(); row++) {
            auto index = m_model->index(row, 0, QModelIndex());
            QString uri = index.data(UriRole).toString();
            if (selectedUris.contains(uri)) {
                oldId = m_model->data(index, ScreenIdRole).toInt();
                QPoint pos = index.data(PositionRole).toPoint();
                qDebug() << " 拖拽:" <<uri<< " 拖拽前坐标："<<pos;
                pos += offPos;
                qDebug() << " 拖拽后坐标："<<pos;
                if (isInvalidPoint(pos) || !m_itemPosHash.keys(pos).isEmpty()) {
                    relayoutIndexes.insert(uri, index);
                    continue;
                }
                setFileMetaInfoPos(uri, pos);
                m_model->setData(index, pos, PositionRole);
                m_model->setData(index, m_proxyModel->id(), ScreenIdRole);
            }
        }
        for (QString uri : relayoutIndexes.keys()) {
            auto index = relayoutIndexes.value(uri);
            QPoint dropedGridPos = index.data(PositionRole).toPoint();
            qDebug() <<"重排当前文件："<<uri<< " 重排当前坐标："<<dropedGridPos;
            dropedGridPos += offPos;
            qDebug() <<" 重排后坐标："<<dropedGridPos;
            QPoint pos = findNextEmptyGridPos(dropedGridPos);
            if (!isInvalidPoint(pos)) {
                setFileMetaInfoPos(uri, pos);
                m_model->setData(index, pos, PositionRole);
                m_model->setData(index, m_proxyModel->id(), ScreenIdRole);
            }
        }
        m_proxyModel->invalidate();
        Q_EMIT m_model->idChanged(-1, oldId, selectedUris);
        return true;
    }
    return false;
}

// 根据当前桌面信息，处理首次插入的插件默认排样
void PlatformWindow::layoutAddedWidgetItem(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem, WidgetsProxyModel *proxyModel, bool isMerging)
{
    // 这里通过initWithConfig获取初始大小
    if (!isMerging) {
        proxyModel->initWithConfig(widgetItem);
    }

    // 仅对和桌面图标并存的场景进行排列处理
    if (item->z() != 0) {
        return;
    }

    // 先更新当前widget占用区域到最新（不包含此widget），但是不更新图标位置，避免延迟导致的影响
    updateDesktopWidgetsUsedRegionInstantly(widgetItem, true);

    QRegion usedRegion = m_desktopWidgetsUsedRegion;

    // 剔除超过屏幕边界的区域
    usedRegion.intersects(QRect(QPoint(), m_screen->size()));
    // 对于网格类型和非网格类型的item均需处理，需要直接操作model层更新数据避免破坏绑定
    QPoint pos;
    if (WidgetsProxyModel::isItemLayoutWithGrid(widgetItem)) {
        if (!calculateWidgetLayoutTargetGridPos(widgetItem, usedRegion, pos, !isMerging)) {
            qWarning() << "无法获取合适的网格位置，需要排到右上角";
        }
    } else {
        // 非网格类型item，不考虑桌面图标位置（挤开桌面图标），仅考虑已有 widget item
        if (!calculateWidgetLayoutTargetPos(widgetItem, usedRegion, pos)) {
            qWarning() << "无法获取合适的位置，需要排到右上角";
        }
    }
    item->setX(pos.x());
    item->setY(pos.y());
    // 记录当前插件位置信息，用于后续处理拖拽和改变大小场景，拖拽和改变位置大小处理完成后此配置应该更新
    // widgetItem->setProperty("lastRect", QRect(pos.x(), pos.y(), widgetItem->width(), widgetItem->height()));
    proxyModel->updateItemPositionConfig(widgetItem, pos);
}

QPoint PlatformWindow::applyWidgetItemConfigPosition(QQuickItem *delegate,
                                                     UkuiQuick::WidgetItem *widgetItem,
                                                     WidgetsProxyModel *proxyModel,
                                                     int configX,
                                                     int configY)
{
    if (!delegate || !widgetItem) {
        return QPoint(configX, configY);
    }

    delegate->setX(configX);
    delegate->setY(configY);

    const bool layoutGrid = WidgetsProxyModel::isItemLayoutWithGrid(widgetItem);
    const int layoutWidth = m_currentWidgetsLayoutWidth;

    setCurrentDragMoveItem(widgetItem);
    updateDragResizeUsedRegion(widgetItem);

    QQuickItem *dragIndicator = nullptr;
    if (rootItem()) {
        dragIndicator = rootItem()->findChild<QQuickItem *>(QStringLiteral("dragIndicatorRect"));
    }

    if (layoutGrid && proxyModel && dragIndicator && layoutWidth > 0) {
        adjustGridLayoutWidget(delegate, widgetItem, false, dragIndicator, proxyModel, layoutWidth);
        if (dragIndicator->isVisible()) {
            delegate->setX(dragIndicator->x());
            delegate->setY(dragIndicator->y());
            dragIndicator->setVisible(false);
        }
    }

    if (checkItemIntersected(delegate, widgetItem)) {
        bool resolved = false;
        if (dragIndicator && layoutWidth > 0) {
            dragIndicator->setX(delegate->x());
            dragIndicator->setY(delegate->y());
            dragIndicator->setWidth(delegate->width());
            dragIndicator->setHeight(delegate->height());
            resolved = adjustIntersectedItem(delegate, dragIndicator, layoutWidth, layoutGrid);
            if (resolved) {
                delegate->setX(dragIndicator->x());
                delegate->setY(dragIndicator->y());
            }
        }
        if (!resolved && proxyModel) {
            layoutAddedWidgetItem(delegate, widgetItem, proxyModel, true);
        }
    }

    setCurrentDragMoveItem(nullptr);

    const QPoint finalConfigPos(int(delegate->x()), int(delegate->y()));
    if (proxyModel) {
        proxyModel->updateItemPositionConfig(widgetItem, finalConfigPos);
    } else {
        widgetItem->widget()->config()->setValue(QStringLiteral("x"), finalConfigPos.x());
        widgetItem->widget()->config()->setValue(QStringLiteral("y"), finalConfigPos.y());
    }

    clearWidgetExceptedPos(widgetItem);
    updateDesktopWidgetsUsedRegion();

    return finalConfigPos;
}

QPoint PlatformWindow::widgetItemScreenPosition(UkuiQuick::WidgetItem *widgetItem) const
{
    if (!widgetItem || !rootItem()) {
        return QPoint();
    }
    const QPointF pos = widgetItem->mapToItem(rootItem(), QPointF(-HORIZON_MARGIN, -VERTICAL_MARGIN));
    return QPoint(qRound(pos.x()), qRound(pos.y()));
}

QPoint PlatformWindow::delegateConfigPosForScreenPosition(QQuickItem *delegate,
                                                          UkuiQuick::WidgetItem *widgetItem,
                                                          int screenX,
                                                          int screenY) const
{
    if (!delegate || !widgetItem) {
        return QPoint(screenX, screenY);
    }

    QQuickItem *layoutParent = delegate->parentItem();
    const int layoutW = layoutParent && layoutParent->width() > 0
        ? int(layoutParent->width())
        : m_currentWidgetsLayoutWidth;
    const int itemW = delegate->width() > 0 ? int(delegate->width()) : int(widgetItem->width());
    if (layoutW <= 0 || itemW <= 0) {
        return QPoint(screenX, screenY);
    }

    int configX = layoutW - itemW - screenX;
    configX = qBound(0, configX, qMax(0, layoutW - itemW));

    // 用 mapToItem 校准，避免仅用镜像公式与 rootItem 坐标系不一致
    for (int i = 0; i < 12; ++i) {
        const_cast<QQuickItem *>(delegate)->setX(configX);
        const QPoint actual = widgetItemScreenPosition(widgetItem);
        const int deltaX = screenX - actual.x();
        const int deltaY = screenY - actual.y();
        if (qAbs(deltaX) <= 1 && qAbs(deltaY) <= 1) {
            break;
        }
        configX -= deltaX;
        configX = qBound(0, configX, qMax(0, layoutW - itemW));
        if (qAbs(deltaY) > 1) {
            const_cast<QQuickItem *>(delegate)->setY(screenY);
        }
    }

    return QPoint(configX, int(delegate->y()));
}

void PlatformWindow::updateDesktopWidgetsUsedRegion()
{
    if (!m_layoutDelayer->isActive()) {
        m_layoutDelayer->start();
    }
}

void PlatformWindow::updateDesktopWidgetsUsedRegionInstantly(UkuiQuick::WidgetItem *excludedItem, bool updateRegionOnly, WidgetsProxyModel *model)
{
    // qDebug() << "test updateDesktopWidgetsUsedRegion";
    m_desktopWidgetsUsedRegion = QRegion();

    if(this->rootItem() == nullptr){
        return;
    }
    auto containerItemModel = getCurrentWidgetItemModel(model);
    if(containerItemModel == nullptr){
        return;
    }
    int widgetsCount = containerItemModel->rowCount(QModelIndex());
    for (int i = 0; i < widgetsCount; i++) {
        auto widget = qvariant_cast<UkuiQuick::WidgetItem *>(containerItemModel->data(containerItemModel->index(i), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
        if (widget == excludedItem) {
            continue;
        }
        if (widget && widget->widget()->metadata().contents().value("Layer").toInt() >= 0 && widget->isVisible()) {
            QRectF widgetRect = QRectF(QPointF(widget->mapToItem(this->rootItem(), QPointF(-HORIZON_MARGIN, -VERTICAL_MARGIN))), widget->size());
            m_desktopWidgetsUsedRegion += widgetRect.toRect().adjusted(-WIDGET_PADDING, -WIDGET_PADDING, WIDGET_PADDING, WIDGET_PADDING); // 考虑边距
        }
    }

    // qDebug() << "desktop widgets used region" << m_desktopWidgetsUsedRegion;

    if (updateRegionOnly) {
        return;
    }

    // fixme: 完善相关重排和恢复流程，提升性能和交互效果
    if (GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool()) {
        // todo: 优化重排性能
        QStringList sortedUris = getAutoRangeSortedUris();
        // QStringList urisNeedRelayout;
        // bool needRelayout = false;
        // for (auto uri: sortedUris) {
        //     if (needRelayout) {
        //         urisNeedRelayout.append(uri);
        //     } else if (checkIfDesktopWidgetsIntersected(m_itemPosHash.value(uri))) {
        //         needRelayout = true;
        //         urisNeedRelayout.append(uri);
        //         continue;
        //     }
        // }

        // qDebug() << "relayout existing items:\n" << urisNeedRelayout.join('\n');
        relayoutExsitingItemsAndUpdate(sortedUris, false);

        for (int row = 0; row < m_proxyModel->rowCount(); row++) {
            auto index = m_proxyModel->index(row, 0, QModelIndex());
            auto uri = index.data(UriRole).toString();
            auto pos = m_itemPosHash.value(uri);
            m_proxyModel->setData(index, pos, PositionRole);
            setFileMetaInfoPos(uri, pos);
        }
        return;
    }

    // qDebug() << "获取上次记录的可能需要恢复的图标位置的信息" << m_resolutionItemPosHash;
    // 先尝试恢复图标位置
    int rowCount = m_proxyModel->rowCount();
    for (int row = 0; row < rowCount; row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        QString uri = index.data(UriRole).toString();
        QPoint restorepoint;
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (!m_resolutionItemNewPosHash.keys().contains(uri)) {
                continue;
        }
        QPoint pos =  m_resolutionItemNewPosHash.value(uri);
        m_proxyModel->setData(index, pos, PositionRole);
        m_itemPosHash.insert(uri, pos);
        setFileMetaInfoPos(uri, pos);
        m_resolutionItemNewPosHash.remove(uri);
        metaInfo->setMetaInfoStringList(RESTORE_DRAG_ITEM_GRID_POS_ATTRIBUTE, QStringList());
    }

    QHash<QString, QPoint> tmpHash;
    auto uris = m_itemPosHash.keys();
    // qDebug() << "check uris" << uris;
    QStringList urisNeedRelayout;
    QList<QPair<QPoint, QString>> tmpItemList;
    for (auto uri: uris) {
        if (checkIfDesktopWidgetsIntersected(m_itemPosHash.value(uri))) {
            tmpItemList.append(QPair<QPoint, QString>(m_itemPosHash.value(uri), uri));
            tmpHash.insert(uri, m_itemPosHash.value(uri));
        }
    }
    std::stable_sort(tmpItemList.begin(), tmpItemList.end(), iconSizeLessThanY);
    for (auto pair : tmpItemList) {
        urisNeedRelayout.append(pair.second);
    }
    // qDebug() << "relayout uris" << urisNeedRelayout;

    //relayoutExsitingItems(urisNeedRelayout);
    relayoutExsitingItemsForDesktopWidgets(urisNeedRelayout);

    // qDebug() << "new hash pos:" << m_itemPosHash;

    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        QString uri = index.data(UriRole).toString();
        if (!urisNeedRelayout.contains(uri)) {
            continue;
        }
        QPoint pos = m_itemPosHash.value(uri);
        // int id = index.data(ScreenIdRole).toInt();
        m_proxyModel->setData(index, pos, PositionRole);
        m_itemPosHash.insert(uri, pos);
        setFileMetaInfoPos(uri, pos);
        QPoint reolutionItemPos = tmpHash.value(uri);
        QStringList exceptedPos;
        exceptedPos<<QString::number(reolutionItemPos.x());
        exceptedPos<<QString::number(reolutionItemPos.y());
        exceptedPos<<QString::number(this->id());
        //m_proxyModel->setData(index, exceptedPos, ExceptedPositionRole);
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            metaInfo->setMetaInfoStringList(RESTORE_DRAG_ITEM_GRID_POS_ATTRIBUTE, exceptedPos);
        }
        m_resolutionItemNewPosHash.insert(uri, reolutionItemPos);
    }

    // qDebug() << "记录可能需要恢复的图标位置" << m_resolutionItemPosHash;
}

void PlatformWindow::updateDragResizeUsedRegion(UkuiQuick::WidgetItem *excludedItem, WidgetsProxyModel *model)
{
    auto containerItemModel = getCurrentWidgetItemModel(model);
    if (!containerItemModel) {
        return;
    }
    // 针对拖拽改变插件窗口大小场景占用区域
    // note: QRegion会将大的rect分成若干小块存储，这会影响网格对齐处理，这里引入一个vector用来保存原始rect的信息
    m_dragResizeUsedRegion = QRegion();
    m_dragResizeUsedRegionRects.clear();
    int widgetsCount = containerItemModel->rowCount(QModelIndex());
    for (int i = 0; i < widgetsCount; i++) {
        auto widget = qvariant_cast<UkuiQuick::WidgetItem *>(containerItemModel->data(containerItemModel->index(i), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
        if (widget == excludedItem) {
            if (model) {
                // 在初始化阶段，部分widgetitem还未添加到widgetslayout，只添加excludewidget之前的item，
                // 避免出现m_dragResizeUsedRegion信息异常，进而导致item判断相交逻辑异常引发重排到0，0的连锁问题
                break;
            } else {
                continue;
            }
        }
        if (widget && widget->widget()->metadata().contents().value("Layer").toInt() >= 0 && widget->isVisible()) {
            QRectF widgetRect = QRectF(QPointF(widget->mapToItem(this->rootItem(), QPointF(-HORIZON_MARGIN, -VERTICAL_MARGIN))), widget->size());
            QRectF targetRect = widgetRect.adjusted(-WIDGET_PADDING, -WIDGET_PADDING, WIDGET_PADDING, WIDGET_PADDING); // 考虑边距
            m_dragResizeUsedRegion += targetRect.toRect();
            m_dragResizeUsedRegionRects.insert(i, targetRect);
        }
    }
}

bool PlatformWindow::checkItemIntersected(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem)
{
    // 处理镜像translate，如果直接使用item的x，得到的实际位置会不准确
    auto rect = item->boundingRect();
    QPointF pos = item->mapToItem(item->parentItem(), QPointF());
    auto parentItemRect = item->parentItem()->boundingRect();
    if (parentItemRect.width() > 0 && rect.isEmpty() && widgetItem!=nullptr) {
        // item未就绪，但是widgetsLayout已就绪，需要进行x轴translate补偿
        pos -= QPointF(widgetItem->width(), 0);
    }
    rect.moveTo(pos.x(), pos.y());
    if (widgetItem) {
        // 在初始化阶段（onItemAdded），item的大小还没有和widgetitem同步，
        // 这里需要使用widgetitem的大小确保计算正确
        rect.setWidth(widgetItem->width());
        rect.setHeight(widgetItem->height());
    }
    bool intersected = m_dragResizeUsedRegion.intersects(rect.toRect());
    if (intersected) {
        return true;
    }
    return m_dragResizeUsedRegion.intersects(rect.toRect());
}

bool PlatformWindow::adjustIntersectedItem(QQuickItem * dragItem, QQuickItem *item, int parentWidth, bool layoutGrid)
{
    int dragItemWidgetUnitSize = -1;
    if (layoutGrid && currentDragMoveItem()) {
        // 需要获取当前待放置widget item的网格大小，用于计算合适的网格
        dragItemWidgetUnitSize = WidgetsProxyModel::itemWidgetGridUnitSize(currentDragMoveItem());
        // qDebug() << "当前网格大小" << dragItemWidgetUnitSize;
    }
    // 处理镜像translate，如果直接使用item的x，得到的实际位置会不准确
    QRectF rect = dragItem->boundingRect();
    QPointF pos = dragItem->mapToItem(dragItem->parentItem(), QPointF());
    rect.moveTo(pos);

    QRectF widgetsLayoutRect(0, 0, m_currentWidgetsLayoutWidth, m_currentWidgetsLayoutHeight);
    if (!widgetsLayoutRect.contains(rect)) {
        // 拖拽item超出屏幕，需要按照边缘位置重新对齐
        if (rect.right() > widgetsLayoutRect.right()) {
            pos.setX(widgetsLayoutRect.right() - rect.width());
        } else if (rect.left() < 0) {
            pos.setX(0);
        }

        if (rect.y() > widgetsLayoutRect.bottom()) {
            pos.setY(widgetsLayoutRect.bottom() - rect.height());
        } else if (pos.y() < 0) {
            pos.setY(0);
        }

        rect.moveTo(pos);
    }

    QRegion intersectedRegion = m_dragResizeUsedRegion.intersected(rect.toRect());
    if (intersectedRegion.rectCount() == 0) {
        return true;
    }
    auto intersectedRect = intersectedRegion.rects().first();

    // 找到当前相交的一个矩形
    for (QRectF tmpRect : qAsConst(m_dragResizeUsedRegionRects)) {
        if (tmpRect.intersects(rect)) {
            // 计算偏移量最小的释放区域
            if (intersectedRect.width() > intersectedRect.height()) {
                // 纵向移动
                // 判断是顶部相交还是底部相交
                if (tmpRect.y() == intersectedRect.y()) {
                    // 顶部相交，将item上移
                    rect.translate(0, -intersectedRect.height());
                } else {
                    // 底部相交，将item下移
                    rect.translate(0, intersectedRect.height());
                }
            }  else {
                // 横向移动
                // 判断左侧相交还是右侧相交
                if (tmpRect.x() == intersectedRect.x()) {
                    // 左侧相交，将item左移
                    rect.translate(-intersectedRect.width(), 0);
                } else {
                    // 右侧相交，将item右移
                    rect.translate(intersectedRect.width(), 0);
                }
            }

            if (layoutGrid && dragItemWidgetUnitSize > 0) {
                // 取相交rect右上角的点作为基准点，计算出网格的基准点
                QPointF gridBasePoint = tmpRect.topRight();
                gridBasePoint = gridBasePoint + QPoint(WIDGET_MARGIN - WIDGET_PADDING, -WIDGET_MARGIN + WIDGET_PADDING);
                // 通过网格基准点，找到widgetitem最适合放置的网格基准点，这个基准点可以按左上角算
                QPointF targetPoint = rect.topLeft() + QPoint(-WIDGET_MARGIN, -WIDGET_MARGIN);
                int offsetGridX = qRound((targetPoint.x() - gridBasePoint.x()) * 1.0 / (dragItemWidgetUnitSize + WIDGET_SPACING)); //近似计算
                int offsetGridY = qRound((targetPoint.y() - gridBasePoint.y()) * 1.0 / (dragItemWidgetUnitSize + WIDGET_SPACING)); //近似计算
                // 计算目标位置
                targetPoint.setX(gridBasePoint.x() + offsetGridX * (dragItemWidgetUnitSize + WIDGET_SPACING) + WIDGET_SPACING);
                targetPoint.setY(gridBasePoint.y() + offsetGridY * (dragItemWidgetUnitSize + WIDGET_SPACING));
                targetPoint = targetPoint + QPoint(WIDGET_MARGIN, WIDGET_MARGIN);

                // qDebug() << "原位置:" << rect << "相交:" << m_dragResizeUsedRegion.contains(rect.toRect());
                // 移动到目标位置
                rect.moveTo(targetPoint);

                // qDebug() << "目标位置:" << rect << "相交:" << m_dragResizeUsedRegion.contains(rect.toRect());
            }

            break;
        }
    }

    // 判断调整后的矩形是否和其它item相交
    if (m_dragResizeUsedRegion.contains(rect.toRect())) {
        return false;
    }

    // 调整后超出屏幕
    if (!widgetsLayoutRect.contains(rect.toRect())) {
        return false;
    }

    item->setX(parentWidth - rect.width() - rect.x()); //处理镜像translate
    item->setY(rect.y());
    return true;
}

void PlatformWindow::adjustGridLayoutWidget(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem, bool sizeChanged, QQuickItem *dragIndicatorItem, WidgetsProxyModel *model, int containerWidth)
{
    // 进行参数校验
    if (!item) {
        return;
    }
    if (!widgetItem) {
        return;
    }

    // 移动场景对参数做一些校验
    if (!sizeChanged) {
        if (!dragIndicatorItem) {
            return;
        }
        if (!model) {
            return;
        }
        if (containerWidth > m_screen->size().width()) {
            return;
        }
    }

    // 更新插件位置占用信息
    updateDesktopWidgetsUsedRegionInstantly(widgetItem, true);

    if (sizeChanged) {
        QRectF lastRect = widgetItem->property("lastRect").toRectF();
        if (lastRect.isValid()) {
            int gridSizeUnit = WidgetsProxyModel::itemWidgetGridUnitSize(widgetItem);

            // 做网格对齐，注意这里的x轴是镜像版本，判断相交时需要进行rect映射
            QRectF currentRect = QRectF(item->mapToItem(item->parentItem(), QPointF()), QSizeF(widgetItem->width(), widgetItem->height()));
            QRectF exceptedRect = currentRect;

            // 对期望网格取近似值
            int roundedGridWidth = qRound((currentRect.width() + 2.0 * WIDGET_MARGIN + WIDGET_SPACING) / (gridSizeUnit + WIDGET_SPACING));
            int roundedGridHeight = qRound((currentRect.height() + 2.0 * WIDGET_MARGIN + WIDGET_SPACING) / (gridSizeUnit + WIDGET_SPACING));

            if (lastRect.width() != currentRect.width()) {
                if (false && qAbs(lastRect.width() - currentRect.width()) < WIDGET_MARGIN) {
                    // 偏移量过小，保持上次状态
                    item->setX(lastRect.x());
                    item->setWidth(lastRect.width());
                } else {
                    // x侧未改变，对齐左侧
                    if (qAbs(lastRect.x() - item->x()) < 2) {
                        do {
                            int exceptedWidth = roundedGridWidth * (gridSizeUnit + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
                            int tmpWidth = exceptedRect.width();
                            exceptedRect.adjust(tmpWidth - exceptedWidth, 0, 0, 0);
                            roundedGridWidth--;
                        } while (roundedGridWidth > 1 && m_desktopWidgetsUsedRegion.intersects(exceptedRect.toRect())); // 找到最小的不相交网格宽度
                        // 由于镜像布局，实际是调整item的宽度，x保持不变
                        widgetItem->setWidth(exceptedRect.width());
                    } else {
                        // 右侧未改变，对齐右侧
                        do {
                            int exceptedWidth = roundedGridWidth * (gridSizeUnit + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
                            exceptedRect.setWidth(exceptedWidth);
                            roundedGridWidth--;
                        } while (roundedGridWidth > 1 && m_desktopWidgetsUsedRegion.intersects(exceptedRect.toRect())); // 找到最小的不相交网格宽度
                        // 由于镜像布局，实际是调整item的x值，并且修改宽度
                        item->setX(item->x() - exceptedRect.width() + currentRect.width());
                        widgetItem->setWidth(exceptedRect.width());
                    }
                }
            }

            if (lastRect.height() != currentRect.height()) {
                if (false && qAbs(lastRect.height() - currentRect.height()) < WIDGET_MARGIN) {
                    // 偏移量过小，保持上次状态
                    item->setY(lastRect.y());
                    item->setHeight(lastRect.height());
                } else {
                    if (qAbs(lastRect.y() - currentRect.y()) < 2) {
                        // 上边未改变，对齐下边
                        do {
                            int exceptedHeight = roundedGridHeight * (gridSizeUnit + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
                            exceptedRect.setHeight(exceptedHeight);
                            roundedGridHeight--;
                        } while (roundedGridHeight > 1 && m_desktopWidgetsUsedRegion.intersects(exceptedRect.toRect()));
                        widgetItem->setHeight(exceptedRect.height());
                    } else {
                        // 下边未改变，对齐上边
                        do {
                            int exceptedHeight = roundedGridHeight * (gridSizeUnit + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
                            int tmpHeight = exceptedRect.height();
                            exceptedRect.adjust(0, tmpHeight - exceptedHeight, 0, 0);
                            roundedGridHeight--;
                        } while (roundedGridHeight > 1 && m_desktopWidgetsUsedRegion.intersects(exceptedRect.toRect()));
                        item->setY(exceptedRect.y());
                        widgetItem->setHeight(exceptedRect.height());
                    }
                }
            }
        }
    } else {
        // 位置变化，需要获取吸附位置放置
        QRectF currentRect = item->boundingRect();
        QPointF currentPos = item->mapToItem(item->parentItem(), QPointF());
        currentRect.moveTo(currentPos);
        // qDebug() << "current rect" << currentRect;

        bool outOfScreen = false;
        QRectF widgetsLayoutRect(0, 0, m_currentWidgetsLayoutWidth, m_currentWidgetsLayoutHeight);
        if (!widgetsLayoutRect.contains(currentRect)) {
            outOfScreen = true;

            // 拖拽item超出屏幕，需要按照边缘位置重新对齐
            if (currentRect.right() > widgetsLayoutRect.right()) {
                currentPos.setX(widgetsLayoutRect.right() - currentRect.width());
            } else if (currentRect.left() < 0) {
                currentPos.setX(0);
            }

            if (currentRect.y() > widgetsLayoutRect.bottom()) {
                currentPos.setY(widgetsLayoutRect.bottom() - currentRect.height());
            } else if (currentPos.y() < 0) {
                currentPos.setY(0);
            }

            currentRect.moveTo(currentPos);
        }

        // 由于网格吸附要求临近边缘时就触发，这里需要扩大currentRect的检测范围
        QRectF thisRect = currentRect;
        thisRect.adjust(-WIDGET_MARGIN, -WIDGET_MARGIN, WIDGET_MARGIN, WIDGET_MARGIN);
        int gridUnitSize = WidgetsProxyModel::itemWidgetGridUnitSize(widgetItem);
        thisRect.adjust(-gridUnitSize, -gridUnitSize, gridUnitSize, gridUnitSize);

        bool isRectAlone = true;
        for (QRectF tmpRect : qAsConst(m_dragResizeUsedRegionRects)) {
            if (tmpRect.intersects(thisRect)) {
                isRectAlone = false;
                // 检测到item相交，需要判断网格是否一致，如果不一致则不吸附
                int index = m_dragResizeUsedRegionRects.key(tmpRect);
                // qDebug() << "当前靠近吸附的item:" << index << tmpRect;

                // 通过model和index获取对应吸附rect的widgetitem
                UkuiQuick::WidgetItem *targetItem = qvariant_cast<UkuiQuick::WidgetItem *>(model->data(model->index(index, 0)));
                if (!targetItem) {
                    qWarning() << "未找到吸附widgetitem的句柄，当前index:" << index;
                    return;
                } else {
                    int targetItemGridUnitSize = WidgetsProxyModel::itemWidgetGridUnitSize(targetItem);
                    if (targetItemGridUnitSize != gridUnitSize) {
                        // qDebug() << "当前item不满足网格匹配，继续查找就近的item" << index;
                        continue;
                    } else {
                        // 网格满足就近吸附条件，计算需要吸附的位置
                        // 由于吸附是从当前拖拽item进入被吸附item边缘就开始的，直接以基准网格近似放置在距离较远时不能实现期望的效果
                        // 这里需要进行拖拽item的方位判断，总共有八个方位，四边四角，在判断了方位后，对网格先进行一次贴边，再做近似网格计算

                        bool adjustSuccessed = false;
                        QRectF baseRect = tmpRect;
                        baseRect.adjust(WIDGET_PADDING, WIDGET_PADDING, -WIDGET_PADDING, -WIDGET_PADDING);
                        baseRect.adjust(-WIDGET_MARGIN, -WIDGET_MARGIN, WIDGET_MARGIN, WIDGET_MARGIN);
                        if (currentRect.bottom() < tmpRect.top()) {
                            // 位于上方
                            // qDebug() << "位于上方";
                            if (currentRect.right() < tmpRect.left()) {
                                // 位于左上方，吸附左上角网格
                                QPointF targetPoint = baseRect.topLeft();
                                targetPoint = targetPoint - QPoint(WIDGET_SPACING, WIDGET_SPACING);
                                targetPoint = targetPoint - QPoint(currentRect.width() + WIDGET_MARGIN, currentRect.height() + WIDGET_MARGIN);
                                currentRect.moveTo(targetPoint);
                            } else if (currentRect.left() > tmpRect.right()) {
                                // 位于右上方，吸附右上角网格
                                QPointF targetPoint = baseRect.topRight();
                                targetPoint = targetPoint + QPoint(WIDGET_SPACING, -WIDGET_SPACING);
                                targetPoint = targetPoint + QPoint(WIDGET_MARGIN, -WIDGET_MARGIN - currentRect.height());
                                currentRect.moveTo(targetPoint);
                            } else {
                                // 位于正上方，调整currentRect，贴住targetItem上方后进行网格近似处理
                                currentRect.setY(baseRect.top() - WIDGET_MARGIN - WIDGET_SPACING - currentRect.height());
                                int gridOffsetX = qRound((currentRect.x() - baseRect.x()) * 1.0/(gridUnitSize + WIDGET_SPACING));
                                // qDebug() << "offsetX" << gridOffsetX;
                                int targetX = tmpRect.x() + WIDGET_PADDING + gridOffsetX * (gridUnitSize + WIDGET_SPACING);
                                currentRect.moveTo(targetX, currentRect.y());
                                // qDebug() << baseRect.x() << targetX << currentRect;
                            }
                            adjustSuccessed = true;
                        } else if (currentRect.top() > tmpRect.bottom()) {
                            // 位于下方
                            // qDebug() << "位于下方";
                            if (currentRect.right() < tmpRect.left()) {
                                // 位于左下方，吸附左下角网格
                                QPointF targetPoint = baseRect.bottomLeft();
                                targetPoint = targetPoint - QPoint(WIDGET_SPACING, -WIDGET_SPACING);
                                targetPoint = targetPoint - QPoint(currentRect.width() + WIDGET_MARGIN, -WIDGET_MARGIN);
                                currentRect.moveTo(targetPoint);
                            } else if (currentRect.left() > tmpRect.right()) {
                                // 位于右下方，吸附右上角网格
                                QPointF targetPoint = baseRect.bottomRight();
                                targetPoint = targetPoint + QPoint(WIDGET_SPACING, WIDGET_SPACING);
                                targetPoint = targetPoint + QPoint(WIDGET_MARGIN, WIDGET_MARGIN);
                                currentRect.moveTo(targetPoint);
                            } else {
                                // 位于正下方，调整currentRect，贴住targetItem上方后进行网格近似处理
                                currentRect.setY(baseRect.bottom() + WIDGET_MARGIN + WIDGET_SPACING);
                                int gridOffsetX = qRound((currentRect.x() - baseRect.x()) * 1.0/(gridUnitSize + WIDGET_SPACING));
                                // qDebug() << "offsetX" << gridOffsetX;
                                int targetX = tmpRect.x() + WIDGET_PADDING + gridOffsetX * (gridUnitSize + WIDGET_SPACING);
                                currentRect.moveTo(targetX, currentRect.y());
                                // qDebug() << baseRect.x() << targetX << currentRect;
                            }
                            adjustSuccessed = true;
                        } else if (currentRect.right() < tmpRect.left()) {
                            // 位于左方
                            // qDebug() << "位于左方";
                            currentRect.setRight(baseRect.left() - WIDGET_SPACING - WIDGET_MARGIN);
                            currentRect.setLeft(currentRect.right() - widgetItem->width());
                            int gridOffsetY = qRound((currentRect.y() - WIDGET_MARGIN - baseRect.y()) * 1.0 / (gridUnitSize + WIDGET_SPACING));
                            int targetY = tmpRect.y() + WIDGET_PADDING + gridOffsetY * (gridUnitSize + WIDGET_SPACING);
                            currentRect.setTop(targetY);
                            currentRect.setHeight(widgetItem->height());
                            adjustSuccessed = true;
                        } else if (currentRect.left() > tmpRect.right()) {
                            // 位于右方
                            // qDebug() << "位于右方";
                            currentRect.setLeft(baseRect.right() + WIDGET_SPACING + WIDGET_MARGIN);
                            currentRect.setWidth(widgetItem->width());
                            int gridOffsetY = qRound((currentRect.y() - WIDGET_MARGIN - baseRect.y()) * 1.0 / (gridUnitSize + WIDGET_SPACING));
                            int targetY = tmpRect.y() + WIDGET_PADDING + gridOffsetY * (gridUnitSize + WIDGET_SPACING);
                            currentRect.setTop(targetY);
                            currentRect.setHeight(widgetItem->height());
                            adjustSuccessed = true;
                        }

                        if (adjustSuccessed) {
                            // 如果吸附成功，则调整dragIndicator位置并显示dragIndicator
                            dragIndicatorItem->setX(containerWidth - currentRect.width() - currentRect.x());
                            dragIndicatorItem->setY(currentRect.y());
                            dragIndicatorItem->setWidth(widgetItem->width());
                            dragIndicatorItem->setHeight(widgetItem->height());
                            dragIndicatorItem->setVisible(true);
                        }
                    }
                }
                break;
            }
        }

        if (isRectAlone) {
            dragIndicatorItem->setX(containerWidth - currentRect.width() - currentRect.x());
            dragIndicatorItem->setY(currentRect.y());
            dragIndicatorItem->setWidth(widgetItem->width());
            dragIndicatorItem->setHeight(widgetItem->height());
            if (outOfScreen) {
                // 超出屏幕的item独立需要修正位置，并且显示dragIndicator
                dragIndicatorItem->setVisible(true);
            } else {
                dragIndicatorItem->setVisible(false);
            }
        }
    }
}

void PlatformWindow::updateLastRect(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem)
{
    widgetItem->setProperty("lastRect", QRectF(item->x(), item->y(), item->width(), item->height()));
}

void PlatformWindow::clearWidgetExceptedPos(UkuiQuick::WidgetItem *item)
{
    auto config = item->widget()->config();
    config->removeKey("exceptedX");
    config->removeKey("exceptedY");
}

void PlatformWindow::updateWidgetSizetoForIntersected(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem)
{
    QRectF currentRect = QRectF(item->mapToItem(item->parentItem(), QPointF()), QSizeF(widgetItem->width(), widgetItem->height()));
    int instanceid = widgetItem->widget()->instanceId();
    if(widgetItem->height() == 0){
        int initheight = DesktopOrganizationManager::instance()->getInitHeight(instanceid);
        currentRect.setHeight(initheight);
    }
    QRectF exceptedRect = currentRect;
    int gridSizeUnit = WidgetsProxyModel::itemWidgetGridUnitSize(widgetItem);
    // 对期望网格取近似值
    updateDesktopWidgetsUsedRegionInstantly(widgetItem, true);
    int roundedGridWidth = qRound((currentRect.width() + 2.0 * WIDGET_MARGIN + WIDGET_SPACING) / (gridSizeUnit + WIDGET_SPACING));
    int roundedGridHeight = qRound((currentRect.height() + 2.0 * WIDGET_MARGIN + WIDGET_SPACING) / (gridSizeUnit + WIDGET_SPACING));
    do {
        int exceptedHeight = roundedGridHeight * (gridSizeUnit + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
        exceptedRect.setHeight(exceptedHeight);
        roundedGridHeight--;
    } while (roundedGridHeight > 0 && m_desktopWidgetsUsedRegion.intersects(exceptedRect.toRect()));
    widgetItem->setHeight(exceptedRect.height());
    Q_EMIT DesktopOrganizationManager::instance()->updateInitHeight(instanceid,exceptedRect.height());
}

void PlatformWindow::recoredCurrentPositionAndSize(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem)
{
    int x = item->x();
    int y = item->y();
    int width = widgetItem->width();
    int height = widgetItem->height();
    auto config = widgetItem->widget()->config();
    QVariantMap map;
    map.insert("x", x);
    map.insert("y", y);
    map.insert("width", width);
    map.insert("height", height);
    config->setValue("exceptedRect", map);
}

bool PlatformWindow::isPlatformWindowHasRootItem()
{
    return this->rootItem();
}

bool PlatformWindow::calculateWidgetLayoutTargetPos(UkuiQuick::WidgetItem *widgetItem, const QRegion &usedRegion, QPoint &pos)
{
    bool posFound = false;

    int itemWidth = widgetItem->width()? widgetItem->width():  widgetItem->childrenRect().width();
    int itemHeight = widgetItem->height()? widgetItem->height(): widgetItem->childrenRect().height();
    auto containerSize = m_screen->size() - QSize(HORIZON_MARGIN * 2, VERTICAL_MARGIN * 2);

    // 检测插件窗口是否过大，如果过大则放置到0，0
    if (itemWidth >= containerSize.width() || itemHeight >= containerSize.height()) {
        qDebug() << "插件尺寸宽度太大，无法正常放置";
        pos = QPoint();
        return false;
    }

    // 如果当前没有插件item，则放置item到右上角
    if (usedRegion.isEmpty()) {
        //pos = QPoint(containerSize.width() - itemWidth, 0);
        pos = QPoint();
        return true;
    }

    // 从usedRegion分割出rects，进行‌类似最低水平线算法‌的排样，区别是当前已经有数据，并且当前水平线从零开始，一次仅进行一个矩形的排样
    auto rects = usedRegion.rects(); // 过时的方法，后续可能需要用迭代器实现获取已有矩形
    QMap<int, int> levels;
    int minLevel = containerSize.width();
    int maxLevel = itemWidth;
    // 先设置最高和最低水平线，由于是从右到左从上到下排列，最低水平线是屏幕右边缘，最高水平线是最左侧能放置插件窗口的区域
    levels.insert(minLevel, 0); //是否需要考虑视图的offset？
    levels.insert(maxLevel, 0);
    for (QRect rect: qAsConst(rects)) {
        // 所有rect的x值都可以作为水平线，但是范围要在minLevel和maxLevel之间，否则排列会超过屏幕
        if (rect.x() >= maxLevel && rect.x() <= minLevel) {
            levels.insert(rect.x(), 0);
        }
    }
    // 从值最大的（最低水平线）level开始，从上到下进行可放置区域的检测
    QList<int> levelsList;
    for (auto level: levels.keys()) {
        levelsList.prepend(level);
    }

    for (auto level: levelsList) {
        // 对每个level检查当前level可以放置的region
        QRegion tmpRegion = QRect(level - 1, 0, 1, containerSize.height());
        tmpRegion -= usedRegion;
        // 检查当前level上的剩余区域是否能够放置下此插件
        auto tmpRects = tmpRegion.rects();
        QList<QRect> rectsCanLayout;
        for (QRect rect: qAsConst(tmpRects)) {
            if (rect.height() >= itemHeight) {
                rectsCanLayout.append(rect);
            }
        }

        // 如果当前level无法放置矩形底部，则跳过后续判断
        if (rectsCanLayout.isEmpty()) {
            continue;
        }

        // 对可放置的区域找到y值最小的实际可放置位置，需要注意当前level在当前高度可放置不意味着整个矩形能放置，
        // 有可能存在和其它矩形相交的情况，如果相交，这里需要通过扫描的方式再次检查
        while(!rectsCanLayout.isEmpty()) {
            QRect minYRect = QRect(level - 1, 0, 1, containerSize.height());
            for (QRect rect: rectsCanLayout) {
                if (rect.bottom() <= minYRect.bottom()) {
                    minYRect = rect;
                }
            }

            QRect tmpItemRect = QRect(level - itemWidth, 0, itemWidth, itemHeight);
            for (int y = minYRect.y(); y <= minYRect.bottom() - itemHeight; y++) {
                QRect tmp = tmpItemRect.translated(0, y);
                if (!usedRegion.contains(tmp)) {
                    // 找到可以放置的矩形，返回当前的坐标
                    posFound = true;
                    //pos = QPoint(level - itemWidth, y);
                    pos = QPoint(containerSize.width() - level, y);
                    return true;
                }
            }

            // 当前范围不可放置，将当前矩形移除再次尝试
            rectsCanLayout.removeOne(minYRect);
        }
    }

    if (!posFound) {
        // 未找到可以放置的矩形，放置在右上角
        pos = QPoint(containerSize.width() - itemWidth, 0);
    }

    return posFound;
}

bool PlatformWindow::calculateWidgetLayoutTargetGridPos(UkuiQuick::WidgetItem *widgetItem, const QRegion &usedRegion, QPoint &pos, bool isInit)
{
    int gridUnitSize = WidgetsProxyModel::itemWidgetGridUnitSize(widgetItem);

    bool posFound = false;

    int gridWidth;
    int gridHeight;

    // 获取widgetitem的大小，转化成网格大小
    if (isInit) {
        gridWidth = widgetItem->widget()->config()->getValue("initGridWidth").toInt();
        gridHeight = widgetItem->widget()->config()->getValue("initGridHeight").toInt();
        widgetItem->widget()->config()->setValue("initGridWidth", QVariant());
        widgetItem->widget()->config()->setValue("initGridHeight", QVariant());
    } else {
        gridWidth = qRound((widgetItem->width() + 2 * WIDGET_MARGIN)/gridUnitSize);
        gridHeight = qRound((widgetItem->height() + 2 * WIDGET_MARGIN)/gridUnitSize);
    }

    // fixme: 优化可用网格判断逻辑，这里应该获取widgetsLayout的尺寸
    int availiableWidgetRowCount = m_currentWidgetsLayoutHeight/(gridUnitSize + WIDGET_SPACING) - gridHeight;
    int availableWidgetColumnCount = m_currentWidgetsLayoutWidth/(gridUnitSize + WIDGET_SPACING) - gridWidth;
    if (availableWidgetColumnCount < 0 || availiableWidgetRowCount < 0) {
        qDebug() << "插件尺寸过大，无法进行正常放置";
        pos = QPoint();
        return false;
    }

    // 计算当前item可以尝试的位置
    // 尝试从上到下，从右到左对插件窗口进行布局，当前使用效率比较低的枚举法，后续考虑优化
    for (int column = 0; column < availableWidgetColumnCount; column++) {
        for (int row = 0; row <= availiableWidgetRowCount; row++) {
            // 如果尝试的位置不和usedRegion相交，则成功，设置item位置后返回
            QRect tmpRect = QRect(m_currentWidgetsLayoutWidth - HORIZON_MARGIN * 2 - (column + gridWidth)*(gridUnitSize + WIDGET_SPACING), row*(gridUnitSize + WIDGET_SPACING), gridWidth * (gridUnitSize + WIDGET_SPACING), gridHeight * (gridUnitSize + WIDGET_SPACING));
            if (usedRegion.contains(tmpRect)) {
                // overlapped
                continue;
            } else {
                pos = QPoint(column * (gridUnitSize + WIDGET_SPACING) + WIDGET_MARGIN, row * (gridUnitSize + WIDGET_SPACING) + WIDGET_MARGIN);
                posFound = true;
                qDebug() << "检测到当前可排样网格位置" << pos;
                return true;
            }
        }
    }

    // 如果所有尝试的位置都不可用，则以当前屏幕右上角的位置进行初始化位置分配
    if (!posFound) {
        //pos = QPoint(m_availableWidgetColumnCount - gridWidth, 0);
        pos = QPoint();
    }
    return posFound;
}

QStringList PlatformWindow::getAutoRangeSortedUris()
{
    QStringList result;

    QList<QPair<QPoint, QString>> itemsList;
    for (QString uri: m_itemPosHash.keys()) {
        if (m_autoArrange.contains(uri)) {
            // 已经堆叠的图标不进行排序，后续排序完成直接添加到列表尾部
            continue;
        }
        QPair<QPoint, QString> pair;
        pair.first = m_itemPosHash.value(uri);
        pair.second = uri;
        itemsList.append(pair);
    }

    std::stable_sort(itemsList.begin(), itemsList.end(), iconSizeLessThan);

    for (auto pair: itemsList) {
        result.append(pair.second);
    }

    if (!m_autoArrange.isEmpty()) {
        result.append(m_autoArrange);
    }

    return result;
}

UkuiQuick::WidgetItemModel *PlatformWindow::getCurrentWidgetItemModel(WidgetsProxyModel *model)
{
    // note: 当前ukui-quick widget container初始化流程加载widgetitem较早，此时在platformwindow侧rootitem还未初始化，
    // 考虑调整ukui-quick插件初始化加载流程解决找不到rootItem的问题
    // 关联：https://gitee.com/openkylin/ukui-quick/pulls/340/commits
    if (rootItem()) {
        return rootItem()->widgetItemModel();
    }
    if (model) {
        UkuiQuick::WidgetItemModel *widgetItemModel = qobject_cast<UkuiQuick::WidgetItemModel *>(model->sourceModel());
        return widgetItemModel;
    }
    return nullptr;
}

int PlatformWindow::currentWidgetsLayoutHeight() const
{
    return m_currentWidgetsLayoutHeight;
}

void PlatformWindow::setCurrentWidgetsLayoutHeight(int newCurrentWidgetsLayoutHeight)
{
    if (m_currentWidgetsLayoutHeight == newCurrentWidgetsLayoutHeight)
        return;
    m_currentWidgetsLayoutHeight = newCurrentWidgetsLayoutHeight;
    Q_EMIT currentWidgetsLayoutHeightChanged();
}

QHash<UkuiQuick::WidgetItem *, QRect> PlatformWindow::getCurrentWidgetUsedRects()
{
    QHash<UkuiQuick::WidgetItem *, QRect> result;

    auto containerItemModel = getCurrentWidgetItemModel();
    if (!containerItemModel) {
        return result;
    }
    int widgetsCount = containerItemModel->rowCount(QModelIndex());
    for (int i = 0; i < widgetsCount; i++) {
        auto widget = qvariant_cast<UkuiQuick::WidgetItem *>(containerItemModel->data(containerItemModel->index(i), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
        if (widget && widget->widget()->metadata().contents().value("Layer").toInt() >= 0 && widget->isVisible()) {
            QRectF widgetRect = QRectF(QPointF(widget->mapToItem(this->rootItem(), QPointF(-HORIZON_MARGIN, -VERTICAL_MARGIN))), widget->size());
            result.insert(widget, widgetRect.toRect());
        }
    }

    return result;
}

QHash<UkuiQuick::WidgetItem *, QRect> PlatformWindow::getCurrentWidgetExceptedRects()
{
    QHash<UkuiQuick::WidgetItem *, QRect> result;

    auto containerItemModel = getCurrentWidgetItemModel();
    if (!containerItemModel) {
        return result;
    }
    int widgetsCount = containerItemModel->rowCount(QModelIndex());
    for (int i = 0; i < widgetsCount; i++) {
        auto widget = qvariant_cast<UkuiQuick::WidgetItem *>(containerItemModel->data(containerItemModel->index(i), UkuiQuick::WidgetItemModel::Key::WidgetItemRole));
        if (widget && widget->widget()->metadata().contents().value("Layer").toInt() >= 0 && widget->isVisible()) {
            auto config = widget->widget()->config();
            if (config->keys().contains("exceptedX")) {
                int exceptedX = config->getValue("exceptedX").toInt();
                int exceptedY = config->getValue("exceptedY").toInt();
                result.insert(widget, QRect(exceptedX, exceptedY, widget->width(), widget->height()));
            }
        }
    }

    return result;
}

int PlatformWindow::currentWidgetsLayoutWidth() const
{
    return m_currentWidgetsLayoutWidth;
}

void PlatformWindow::setCurrentWidgetsLayoutWidth(int newCurrentWidgetsLayoutWidth)
{
    if (m_currentWidgetsLayoutWidth == newCurrentWidgetsLayoutWidth)
        return;
    m_currentWidgetsLayoutWidth = newCurrentWidgetsLayoutWidth;
    Q_EMIT currentWidgetsLayoutWidthChanged();
}

UkuiQuick::WidgetItem *PlatformWindow::currentDragMoveItem() const
{
    return m_currentDragMoveItem;
}

void PlatformWindow::relocateWidgetPosition()
{
    if (m_currentWidgetsLayoutHeight <= 0 || m_currentWidgetsLayoutWidth <= 0) {
        qWarning() << "获取当前widgesLayout尺寸异常";
        return;
    }

    QRect widgetsLayoutRect(0, 0, m_currentWidgetsLayoutWidth, m_currentWidgetsLayoutHeight);
    // 判断当前是否存在超出屏幕的插件
    QRect currentBoundingRect = m_desktopWidgetsUsedRegion.boundingRect();
    currentBoundingRect.adjust(WIDGET_PADDING, WIDGET_PADDING, -WIDGET_PADDING, -WIDGET_PADDING);
    if (widgetsLayoutRect.contains(currentBoundingRect)) {
        // 不存在超出的元素，尝试恢复越界的越界的元素
        QHash<UkuiQuick::WidgetItem *, QRect> widgetRects = getCurrentWidgetExceptedRects();
        for (auto widget: widgetRects.keys()) {
            auto exceptedRect = widgetRects.value(widget);
            if (widgetsLayoutRect.contains(exceptedRect)) {
                // 恢复位置，也许需要判断此处是否有插件窗口？
                widget->parentItem()->setX(exceptedRect.x());
                widget->parentItem()->setY(exceptedRect.y());
                auto config = widget->widget()->config();
                config->setValue("x", exceptedRect.x());
                config->setValue("y", exceptedRect.y());
                config->removeKey("exceptedX");
                config->removeKey("exceptedY");
            }
        }
    } else {
        // 存在超出的元素，将超出的元素按照位置顺序进行重排
        QHash<UkuiQuick::WidgetItem *, QRect> widgetRects = getCurrentWidgetUsedRects();
        m_desktopWidgetsUsedRegion = QRegion();
        QList<QPair<UkuiQuick::WidgetItem *, QRect>> widgetsNeedRelayout;
        for (auto widget: widgetRects.keys()) {
            // 如果当前屏幕可以放置插件，则保持当前位置不变
            QRect rect = widgetRects.value(widget);
            if (widgetsLayoutRect.contains(rect)) {
                rect.adjust(-WIDGET_PADDING, -WIDGET_PADDING, WIDGET_PADDING, WIDGET_PADDING);
                m_desktopWidgetsUsedRegion += rect;
            } else {
                widgetsNeedRelayout.append(QPair<UkuiQuick::WidgetItem *, QRect>(widget, rect));
            }
        }

        // 按照右上到左下的顺序对待重排插件进行排序，按排序后的顺序进行重排
        std::stable_sort(widgetsNeedRelayout.begin(), widgetsNeedRelayout.end(), widgetRectPosLesserThan);

        for (auto pair: widgetsNeedRelayout) {
            auto widget = pair.first;
            bool ok = false;
            QPoint pos;
            if (WidgetsProxyModel::isItemLayoutWithGrid(widget)) {
                ok = calculateWidgetLayoutTargetGridPos(widget, m_desktopWidgetsUsedRegion, pos, false);
            } else {
                ok = calculateWidgetLayoutTargetPos(widget, m_desktopWidgetsUsedRegion, pos);
            }

            if (ok) {
                widget->parentItem()->setX(pos.x());
                widget->parentItem()->setY(pos.y());
                //proxyModel->updateItemPositionConfig(widgetItem, pos);
                auto config = widget->widget()->config();
                config->setValue("x", pos.x());
                config->setValue("y", pos.y());
                if (!config->keys().contains("exceptedX")) {
                    config->setValue("exceptedX", m_currentWidgetsLayoutWidth - pair.second.right());
                    config->setValue("exceptedY", pair.second.y());
                }

                QRect rect = QRect(pos, QSize(widget->width(), widget->height()));
                rect.adjust(-WIDGET_PADDING, -WIDGET_PADDING, WIDGET_PADDING, WIDGET_PADDING);
                m_desktopWidgetsUsedRegion += rect;
            } else {
                // todo: 超出的item不显示，等待下次重排
                // 也许需要适配model层？
                widget->parentItem()->setX(0);
                widget->parentItem()->setY(0);
            }
        }
    }
}

bool PlatformWindow::checkIsOrgconfigExist()
{
    //检查屏幕是否存在整理工具配置
    auto containerConfig = mainView()->config();
    auto widgets = mainView()->widgets();
    for (auto widget : widgets) {
        auto config = widget->config();
        if(config!=nullptr){
            if (config->getValue("id").toString() == "desktop-organization-tool") {
                return true;
            }
        }
    }

    auto instances = mainView()->disableInstances();
    for (int id : instances) {
        auto config = containerConfig->child("widgets", id);
        if(config!=nullptr){
            if (config->getValue("id").toString() == "desktop-organization-tool") {
                return true;
            }
        }
    }

    UkuiQuick::Island island("desktop-view", "peony-qt-desktop");
    auto islandConfig = island.config();
    auto configs = islandConfig->children(island.viewGroupName());
    for (auto config: configs) {
        if (config->getValue("id") != "qml-desktop") {
            continue;
        }
        int screenId = config->getValue("instanceId").toInt();
        //if (screenId != id()) {
            // 非主屏配置，添加到主屏上
        auto widgetConfigs = config->children("widgets");
        for (auto widgetConfig: widgetConfigs) {
            //int widgetInstanceId = widgetConfig->getValue("instanceId").toInt();
            //int orgtype = widgetConfig->getValue("organizationType").toInt();
            if(widgetConfig->contains("oldScreenId")){
                int oldsreenid = widgetConfig->getValue("oldScreenId").toInt();
                if(oldsreenid == id()){
                    return true;
                }
            }
        }
    }

    return false;
}

void PlatformWindow::mergeSameTypeWidgetToPrimaryScreen(int screenid, int orgtype ,int instanceid)
{
    auto windows = PlatformWindow::allWindows();
    PlatformWindow *primarywindow = nullptr;
    for (auto window : windows) {
        if(window->id() == SCREEN_MANAGER->getPrimaryScreenId()){
            primarywindow = window;
        }
    }
    int id = primarywindow->id();
    auto widgets = primarywindow->mainView()->widgets();
    for (auto widget : widgets) {
        int type = widget->config()->getValue("organizationType").toInt();
        if(orgtype == type){
            int toinstanceid = widget->instanceId();
            Q_EMIT DesktopOrganizationManager::instance()->mergeSameTypeWidgetToPrimaryScreenRequest(instanceid,toinstanceid);
        }
    }
}

void PlatformWindow::setCurrentDragMoveItem(UkuiQuick::WidgetItem *newCurrentDragMoveItem)
{
    m_currentDragMoveItem = newCurrentDragMoveItem;
}

void PlatformWindow::handleDesktopWidgetDrop(int instanceId, int sourceScreenId, int targetScreenId, const QPoint &dropPos)
{
    // if (sourceScreenId == targetScreenId) {
    //     qDebug() << "[handleDesktopWidgetDrop] Same screen, no need to move";
    //     return;
    // }

    // 获取源屏幕和目标屏幕的窗口
    auto allWindows = PlatformWindow::allWindows();
    PlatformWindow *sourceWindow = nullptr;
    PlatformWindow *targetWindow = nullptr;

    for (auto window : allWindows) {
        if (window->id() == sourceScreenId) {
            sourceWindow = window;
        }
        if (window->id() == targetScreenId) {
            targetWindow = window;
        }
    }

    if (!sourceWindow || !targetWindow) {
        qWarning() << "[handleDesktopWidgetDrop] Source or target window not found"
                   << "source:" << sourceScreenId << "target:" << targetScreenId;
        return;
    }

    // 从源屏幕获取widget
    auto sourceWidget = sourceWindow->mainView()->widget(instanceId);
    if (!sourceWidget) {
        qWarning() << "[handleDesktopWidgetDrop] Widget not found on source screen, instanceId:" << instanceId;
        return;
    }

    // 获取widget的配置
    auto widgetConfig = sourceWidget->config();
    QString widgetId = widgetConfig->getValue("id").toString();

    // 从源屏幕移除widget
    sourceWindow->mainView()->removeWidget(sourceWidget);
    sourceWindow->mainView()->config()->removeChild("widgets", instanceId);

    // 更新配置中的位置为drop位置（镜像计算）
    QVariantMap childData = widgetConfig->data();
    int widgetWidth = childData.value("width").toInt();
    int targetLayoutWidth = targetWindow->currentWidgetsLayoutWidth();
    // 镜像计算: x = targetLayoutWidth - widgetWidth - dropPos.x()
    childData["x"] = targetLayoutWidth - widgetWidth - dropPos.x();
    childData["y"] = dropPos.y();
    // 分配新的instanceId
    int newInstanceId = DesktopWidgetsManager::getInstance()->getNextInstanceId();
    childData["instanceId"] = newInstanceId;

    // 添加配置到目标屏幕
    targetWindow->mainView()->config()->addChild("widgets", childData);
    auto newConfig = targetWindow->mainView()->config()->child("widgets", newInstanceId);

    // 在目标屏幕创建新widget
    auto newWidget = UkuiQuick::WidgetLoader::globalLoader().loadWidget(widgetId);
    newWidget->setConfig(newConfig);
    newWidget->setInstanceId(newInstanceId);
    targetWindow->mainView()->addWidget(newWidget);

    qDebug() << "[handleDesktopWidgetDrop] Widget moved from screen" << sourceScreenId << "to screen" << targetScreenId
             << "widgetId:" << widgetId << "newInstanceId:" << newInstanceId;
}
template<typename T>
QByteArray serialize(const T& value)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << value;
    return data;
}
void PlatformWindow::startSystemDrag(QWindow *dragWindow, const QPoint &globalPos, int instanceId, int sourceScreenId)
{
    if (!dragWindow) {
        return;
    }
    // auto serializeWindow = [](QWindow *window) -> QByteArray {
    //     QByteArray data;
    //     QDataStream stream(&data, QIODevice::WriteOnly);
    //     stream << reinterpret_cast<qintptr>(window);
    //     return data;
    // };
    // auto serializePoint = [](const QPoint &point) -> QByteArray {
    //     QByteArray data;
    //     QDataStream stream(&data, QIODevice::WriteOnly);
    //     stream << point;
    //     return data;
    // };
    auto serializeInt = [](int val) -> QByteArray {
        return QByteArray::number(val);
    };

    // pressPosition: 光标在 TextureViewer 窗口内的相对位置
    QPoint cursorPos = QCursor::pos();
    QPoint pressPosition = cursorPos - globalPos;

    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/x-qt-mainwindowdrag-window",
                      serialize(reinterpret_cast<qintptr>(dragWindow)));
    mimeData->setData("application/x-qt-mainwindowdrag-position", serialize(pressPosition));
    mimeData->setData("DesktopWidgets", "true");
    mimeData->setData("DesktopWidgets_instanceId", serializeInt(instanceId));
    mimeData->setData("DesktopWidgets_sourceScreenId", serializeInt(sourceScreenId));
    mimeData->setData("DesktopWidgets_pressPosition",
                      QString("%1,%2").arg(pressPosition.x()).arg(pressPosition.y()).toUtf8());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    Qt::DropAction action = drag->exec(Qt::MoveAction);
    if (action == Qt::MoveAction) {
        qDebug() << "[PlatformWindow] Drag completed with MoveAction";
    }

    // 拖拽结束后清理
    DesktopWindowManager::getInstance()->closeDragPreviewWithTexture();
}

void PlatformWindow::reloadData()
{
    m_itemPosHash.clear();
    m_autoArrange.clear();
    m_storageBox.clear();
    if (GlobalSettingsAdapter::getInstance()->useAutoLayout()) {
        layoutItems();
        return ;
    }
    for (int row = 0; row < m_proxyModel->rowCount(); row++) {
        auto index = m_proxyModel->index(row, 0, QModelIndex());
        QString uri = index.data(UriRole).toString();
        QPoint pos = index.data(PositionRole).toPoint();
        setFileMetaInfoPos(uri, pos);
    }
    checkItemsOver();
}

bool PlatformWindow::checkEmptyPositon()
{
    if (m_autoArrange.isEmpty())
        return false;
    for (int col = 0; col < m_availableColumnCount; col++) {
        for (int row = 0; row < m_availableRowCount; row++) {
            if (col * m_availableRowCount + row + 1 > m_autoArrange.count()) {
                return false;
            }
            if (m_itemPosHash.keys(QPoint(col, row)).isEmpty()) {
                return true;
            }
        }
    }

    return false;
}

void PlatformWindow::initDesktopOrgWidgets()
{
    // 4. 将模型设置到小部件的上下文
    int widgetsCount = 3;
    for (int i = 0; i < widgetsCount; i++) {
        QString name ;
        QStringList uris;
        // todo: 根据当前数据分类
        Peony::DesktopItemProxyModel::OriganizationType origanizationType = Peony::DesktopItemProxyModel::OriganizationType::None;
        if(i == 0){
            name = tr("Application");
            origanizationType = Peony::DesktopItemProxyModel::OriganizationType::OrgByApplycation;
        }else if(i == 1){
            name = tr("Folder");
            origanizationType = Peony::DesktopItemProxyModel::OriganizationType::OrgByDir;
        }else if(i == 2){
            name = tr("Files");
            origanizationType = Peony::DesktopItemProxyModel::OriganizationType::OrgByFile;
        }

        // todo: 自动整理的分区，需要根据uris是否为空决定是否创建对应分区
        uris  = DesktopOrganizationManager::instance()->getUrisByOrganizationType(origanizationType,id());
        DesktopOrganizationManager::instance()->addDesktopOrganizationWidget(this, origanizationType, name, uris); //这里的uri需要根据类型获取
    }
    m_proxyModel->updateFilter();
}

bool PlatformWindow::addWidgetFromVariant(const QString &widgetId, const QVariantMap &variant,
                                          bool useDropPosition, int dropX, int dropY)
{
    if (!mainView()) {
        return false;
    }
    const int instanceId = DesktopWidgetsManager::getInstance()->getNextInstanceId();
    QVariantMap childData;
    childData.insert(QStringLiteral("id"), widgetId);
    childData.insert(QStringLiteral("instanceId"), instanceId);
    if (variant.contains(QStringLiteral("id"))) {
        childData.insert(QStringLiteral("variantId"), variant.value(QStringLiteral("id")).toString());
    }
    if (variant.contains(QStringLiteral("fixedResizable"))) {
        childData.insert(QStringLiteral("Resizable"), variant.value(QStringLiteral("fixedResizable")).toBool());
    }
    if (variant.contains(QStringLiteral("sizeHint"))) {
        childData.insert(QStringLiteral("SizeHint"), variant.value(QStringLiteral("sizeHint")));
    }
    const QVariant desktopSizeVariant = variant.value(QStringLiteral("desktopSize"));
    if (desktopSizeVariant.canConvert<QVariantList>()) {
        const QVariantList ds = desktopSizeVariant.toList();
        if (ds.size() == 2) {
            const int dw = qMax(1, ds.at(0).toInt());
            const int dh = qMax(1, ds.at(1).toInt());
            childData.insert(QStringLiteral("width"), dw);
            childData.insert(QStringLiteral("height"), dh);
        }
    }
    {
        int lw = currentWidgetsLayoutWidth();
        int lh = currentWidgetsLayoutHeight();
        if (lw <= 0 || lh <= 0) {
            const QRect g = mainView()->geometry();
            lw = qMax(1, g.width());
            lh = qMax(1, g.height());
        }
        const int margin = 8;
        QSize widgetPixelSize = resolveWidgetPixelSize(widgetId, childData, variant);
        int widgetW = widgetPixelSize.width();
        int widgetH = widgetPixelSize.height();
        if (widgetW <= 0 || widgetH <= 0) {
            widgetW = qMax(1, childData.value(QStringLiteral("width"), 1).toInt());
            widgetH = qMax(1, childData.value(QStringLiteral("height"), 1).toInt());
        }
        QPoint preferredPos(qMax(margin, lw - widgetW - margin), margin);
        // QML 拖放传入的是 widgetsLayout 内“视觉坐标”，而桌面插件配置中的 x 使用镜像前的逻辑坐标
        // （WidgetsLayout 通过 Translate 在 X 轴做镜像摆放）。新增实例时先把视觉中心点换算回逻辑左上角，
        // 这样从管理器拖到桌面的落点就会与鼠标位置一致，同时不会影响文件/文件夹拖放路径。
        if (useDropPosition) {
            const int visualLeft = dropX - widgetW / 2;
            const int visualTop = dropY - widgetH / 2;
            const int logicalLeft = lw - widgetW - visualLeft;
            const int x = qBound(margin, logicalLeft, qMax(margin, lw - widgetW - margin));
            const int y = qBound(margin, visualTop, qMax(margin, lh - widgetH - margin));
            preferredPos = QPoint(x, y);
        }

        QPoint finalPos;
        if (!findNearestAvailableWidgetTopLeft(this, QSize(widgetW, widgetH), preferredPos, finalPos)) {
            qWarning() << "addWidgetFromVariant: no free area for widget" << widgetId;
            return false;
        }
        childData.insert(QStringLiteral("x"), finalPos.x());
        childData.insert(QStringLiteral("y"), finalPos.y());
    }
    mainView()->config()->addChild(QStringLiteral("widgets"), childData);
    auto config = mainView()->config()->child(QStringLiteral("widgets"), instanceId);
    auto widget = UkuiQuick::WidgetLoader::globalLoader().loadWidget(widgetId);
    if (!widget || !config) {
        return false;
    }
    widget->setConfig(config);
    widget->setInstanceId(instanceId);
    mainView()->addWidget(widget);
    return true;
}

bool PlatformWindow::addWidgetFromDropJson(const QString &jsonUtf8, bool useDropPosition, int dropX, int dropY)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonUtf8.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "addWidgetFromDropJson: parse failed:" << parseError.errorString();
        return false;
    }
    const QJsonObject obj = doc.object();
    const QString widgetId = obj.value(QStringLiteral("widgetId")).toString();
    if (widgetId.isEmpty()) {
        qWarning() << "addWidgetFromDropJson: missing widgetId";
        return false;
    }
    QJsonObject variantObj = obj;
    variantObj.remove(QStringLiteral("widgetId"));
    const QVariantMap variant = variantObj.toVariantMap();
    return addWidgetFromVariant(widgetId, variant, useDropPosition, dropX, dropY);
}

void PlatformWindow::removeWidgetInstance(int instanceId)
{
    if (instanceId < 0) {
        return;
    }
    DesktopWidgetsManager::getInstance()->removeWidget(instanceId, this);
    for (PlatformWindow *w : PlatformWindow::allWindows()) {
        w->updateDesktopWidgetsUsedRegion();
    }
}

void PlatformWindow::showWidgetManager()
{
    auto dlg = WidgetManagerDialog::instance();
    if (dlg->isVisible()) {
        return;
    }
    dlg->refreshCards();
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

bool PlatformWindow::canResizeWidgetTo(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem,
                                       int newWidth, int newHeight)
{
    if (!item || newWidth <= 0 || newHeight <= 0) {
        return false;
    }
    if (!rootItem()) {
        return false;
    }

    // 排除自身后重建占用区域，与 checkItemIntersected 保持一致的坐标系
    updateDragResizeUsedRegion(widgetItem);

    // 使用 widgetItem（插件 root）在 rootItem 坐标系的位置，与 updateDragResizeUsedRegion 建区域时相同的映射方式。
    // mapToItem 会自动考虑 WidgetsLayout 的镜像 Translate，返回实际渲染位置。
    // 桌面插件在 WidgetsLayout 中通过 X 轴镜像摆放，变宽时视觉右边缘固定、向左扩展（与 ResizeHandler.mirrored 一致），
    // 因此检测占用区域时应以当前右边缘为锚向左试探，而非固定左上角向右扩展。
    QQuickItem *posSource = widgetItem ? static_cast<QQuickItem *>(widgetItem) : item;
    const qreal currentWidth = posSource->width();
    QPointF marginTopLeft = posSource->mapToItem(rootItem(), QPointF(-HORIZON_MARGIN, -VERTICAL_MARGIN));
    QPointF actualTopLeft = posSource->mapToItem(rootItem(), QPointF(0, 0));
    const qreal testLeft = marginTopLeft.x() + currentWidth - qreal(newWidth);
    const qreal testTop = marginTopLeft.y();
    QRectF testRect(testLeft, testTop, qreal(newWidth), qreal(newHeight));

    // 检查与其他插件不重叠
    if (m_dragResizeUsedRegion.intersects(testRect.toRect())) {
        return false;
    }

    // 检查新尺寸在当前位置是否完全在屏幕（rootItem）范围内，
    // 避免边缘插件切换到大尺寸后超出屏幕。
    QRectF screenBounds(0, 0, rootItem()->width(), rootItem()->height());
    const qreal screenTestLeft = actualTopLeft.x() + currentWidth - qreal(newWidth);
    QRectF screenTestRect(screenTestLeft, actualTopLeft.y(), qreal(newWidth), qreal(newHeight));
    if (!screenBounds.contains(screenTestRect)) {
        return false;
    }

    return true;
}

QVariantMap PlatformWindow::constrainContextMenuPosition(QQuickItem *item, qreal localX, qreal localY,
                                                         qreal menuWidth, qreal menuHeight,
                                                         qreal edgeMargin) const
{
    QVariantMap result;
    if (!rootItem()) {
        result.insert(QStringLiteral("x"), localX);
        result.insert(QStringLiteral("y"), localY);
        return result;
    }

    const qreal margin = qMax<qreal>(0.0, edgeMargin);
    const qreal w = qMax<qreal>(0.0, menuWidth);
    const qreal h = qMax<qreal>(0.0, menuHeight);

    // 与插件 Popup/Overlay 的 parent（window.contentItem）保持同一坐标系，避免全局偏移。
    QQuickItem *targetItem = rootItem();
    if (item && item->window() && item->window()->contentItem()) {
        targetItem = item->window()->contentItem();
    }

    QPointF anchor(localX, localY);
    if (item && targetItem) {
        anchor = item->mapToItem(targetItem, QPointF(localX, localY));
    }

    const qreal maxX = qMax<qreal>(margin, targetItem->width() - w - margin);
    const qreal maxY = qMax<qreal>(margin, targetItem->height() - h - margin);
    const qreal clampedX = qBound<qreal>(margin, anchor.x(), maxX);
    const qreal clampedY = qBound<qreal>(margin, anchor.y(), maxY);

    result.insert(QStringLiteral("x"), clampedX);
    result.insert(QStringLiteral("y"), clampedY);
    return result;
}

QVariantMap PlatformWindow::getScreenAvailableGeometry(QQuickItem *item) const
{
    QScreen *screen = nullptr;
    // 优先使用 item 所在窗口绑定的屏幕
    if (item && item->window()) {
        screen = item->window()->screen();
    }
    // 回退到 PlatformWindow 自身绑定的屏幕
    if (!screen) {
        screen = m_screen;
    }
    // 最后兜底：主屏幕
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    QRect avail;
    if (screen) {
        // availableGeometry() 返回去掉任务栏、Dock 等系统保留区域后的可用矩形，
        // 坐标在虚拟桌面全局坐标系（与 mapToGlobal 一致）
        avail = screen->availableGeometry();
    }

    QVariantMap result;
    result.insert(QStringLiteral("x"),      avail.x());
    result.insert(QStringLiteral("y"),      avail.y());
    result.insert(QStringLiteral("width"),  avail.width());
    result.insert(QStringLiteral("height"), avail.height());
    return result;
}

void PlatformWindow::clampWidgetPositionToScreen(QQuickItem *item, int newWidth, int newHeight)
{
    if (!item || !rootItem() || newWidth <= 0 || newHeight <= 0) {
        return;
    }

    // 找到 desktopWidget（widgetItem 的父容器，由 WidgetsLayout delegate 设置）
    QQuickItem *container = item->parentItem();
    if (!container) {
        return;
    }

    // container 的位置已经考虑了 Translate 变换，这里直接用 rootItem 坐标系下的渲染位置判断。
    // mapToItem 会自动折算 WidgetsLayout 的镜像 Translate，得到实际渲染左上角。
    QPointF rendered = item->mapToItem(rootItem(), QPointF(0, 0));

    const qreal screenW = rootItem()->width();
    const qreal screenH = rootItem()->height();

    // 计算在屏幕坐标系下需要的偏移量
    qreal offsetX = 0.0;
    qreal offsetY = 0.0;

    if (rendered.x() < 0) {
        offsetX = -rendered.x();
    } else if (rendered.x() + newWidth > screenW) {
        offsetX = screenW - newWidth - rendered.x();
    }

    if (rendered.y() < 0) {
        offsetY = -rendered.y();
    } else if (rendered.y() + newHeight > screenH) {
        offsetY = screenH - newHeight - rendered.y();
    }

    if (qFuzzyIsNull(offsetX) && qFuzzyIsNull(offsetY)) {
        return;
    }

    // 将屏幕坐标偏移量转换为 container 的父坐标系偏移量
    // container 上有 Translate 变换，但 x/y 属性仍在父坐标系下，
    // Translate 会导致视觉 x 与属性 x 方向相反（镜像），因此 x 偏移取负
    container->setX(container->x() - offsetX);
    container->setY(container->y() + offsetY);
}

bool PlatformWindow::isDraggingToOtherScreen(const QPoint &globalPos, int widgetWidth, int widgetHeight)
{
    QScreen *currentScreen = m_screen;
    if (!currentScreen) {
        return false;
    }

    QRect currentScreenRect = currentScreen->geometry();
    // globalPos 是 widget 左上角坐标（由 QML 的 mapToItem(window, Qt.point(0, 0)) 计算得出）
    QRect widgetRect(globalPos.x(), globalPos.y(), widgetWidth + 60, widgetHeight);

    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen != currentScreen && !currentScreenRect.contains(widgetRect)) {
            return true;
        }
    }
    return false;
}


