#include "desktop-widgets-manager.h"
#include "desktop-window-manager.h"
#include "platformwindow.h"
#include <island.h>
#include <widget-container-item.h>
#include "global-settings.h"
#include <QApplication>

static DesktopWidgetsManager *global_instance = nullptr;

DesktopWidgetsManager::DesktopWidgetsManager(QObject *parent)
    : QObject{parent}
{}

DesktopWidgetsManager::~DesktopWidgetsManager()
{

}

DesktopWidgetsManager *DesktopWidgetsManager::getInstance()
{
    if (!global_instance) {
        global_instance = new DesktopWidgetsManager;
    }
    return global_instance;
}

int DesktopWidgetsManager::getNextInstanceId()
{
    int nextId = 0;
    QList<int> usedInstanceIdList;
    UkuiQuick::Island tmpIsland("desktop-view", "peony-qt-desktop");
    auto confList = tmpIsland.config()->children(UkuiQuick::Island::viewGroupName());
    for (UkuiQuick::Config *config: qAsConst(confList)) {
        // 这里需要触发关于widgets配置的初始化，当前ukui-quick需要调用addGroupInfo，否则无法找到children
        config->addGroupInfo("widgets", "instanceId");
        auto subConfigList = config->children(QStringLiteral("widgets"));
        for (UkuiQuick::Config *subConf: qAsConst(subConfigList)) {
            usedInstanceIdList << subConf->id().toInt();
        }
    }

    while (usedInstanceIdList.contains(nextId)) {
        nextId++;
    }
    return nextId;
}

void DesktopWidgetsManager::removeWidget(int instanceId, PlatformWindow *window)
{
    if (window) {
        auto widget = window->mainView()->widget(instanceId);
        if (widget) {
            window->mainView()->removeWidget(widget);
            window->mainView()->config()->removeChild(QStringLiteral("widgets"), instanceId);
        } else {
            qWarning() << "当前window不存在id为" << instanceId << "的窗口";
        }
    } else {
        for (auto platformwindow : PlatformWindow::allWindows()) {
            auto widget = platformwindow->mainView()->widget(instanceId);
            if (widget) {
                platformwindow->mainView()->removeWidget(widget);
                platformwindow->mainView()->config()->removeChild(QStringLiteral("widgets"), instanceId);
                return;
            }
        }
        qWarning() << "未找到实例 id 为" << instanceId << "的桌面插件";
    }
}

void DesktopWidgetsManager::mergeWidgetsToPrimaryScreen()
{
    // 将插件窗口合并到主屏，尽可能保留其位置信息，用于镜像模式
    // 读取其它屏幕的配置，这里从配置读取的原因是单屏情况下platformwindow资源已经释放，无法通过副屛的platformwindow获取插件相关信息，
    // 这里需要将副屛信息拷贝到主屏然后通过新的信息重新加载相关插件，并且处理位置重排
    UkuiQuick::Island island("desktop-view", "peony-qt-desktop");
    auto islandConfig = island.config();
    auto configs = islandConfig->children(island.viewGroupName());

    PlatformWindow *primaryWindow = nullptr;
    auto windows = PlatformWindow::allWindows();
    for (auto window : windows) {
        if (window->id() == 0) {
            primaryWindow = window;
            break;
        }
    }
    if (!primaryWindow) {
        qCritical() << "未找到主屏窗口，插件合并终止";
        return;
    }
    bool needOriganization = Peony::GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool();
    for (auto config : configs) {
        if (config->getValue("id") != "qml-desktop") {
            continue;
        }
        int screenId = config->getValue("instanceId").toInt();
        if (screenId != 0) {
            // 非主屏配置，添加到主屏上
            auto widgetConfigs = config->children("widgets");
            for (auto widgetConfig: widgetConfigs) {
                int widgetInstanceId = widgetConfig->getValue("instanceId").toInt();
                int orgtype = widgetConfig->getValue("organizationType").toInt();
                qDebug() << "找到扩展屛插件" << screenId << widgetInstanceId;
                if (primaryWindow->mainView()->widget(widgetInstanceId)) {
                    qDebug() << "配置已经同步";
                    continue;
                }

                auto containerWidgetConfig = primaryWindow->mainView()->config();
                containerWidgetConfig->addChild("widgets", widgetConfig->data());
                auto newConfig = containerWidgetConfig->child("widgets", widgetInstanceId);
                bool isexistSameTypePartition  = false;
                bool isautoorg = orgtype>=0 && orgtype<=2;
                if(isautoorg){
                    //存在自动归类的分区，则不需要新增widget
                    for (auto widget : primaryWindow->mainView()->widgets()) {
                        if(widget->config()->getValue("organizationType").toInt() == orgtype){
                            isexistSameTypePartition = true;
                            break;
                        }
                    }
                }
                // oldScreenId用来记录插件合并之前所在的屏幕id
                newConfig->setValue("oldScreenId", screenId);
                // 清空位置信息？todo:优先使用原位置重排
                // newConfig->removeKey("x");
                // newConfig->removeKey("y");
                // newConfig->removeKey("width");
                // newConfig->removeKey("height");
                auto widget = UkuiQuick::WidgetLoader::globalLoader().loadWidget(newConfig->getValue("id").toString());
                // note: 由于这里是直接将副屛widget的config指针赋值给主屏widget，主屏配置仍然使用副屛的配置记录，这样副屛的配置在恢复时能够保留，
                // 但是ukui-quick的配置机制会导致出现两份配置，此处可能需要优化
                widget->setConfig(newConfig);
                widget->setInstanceId(widgetInstanceId);
                // fixme: ukui quick addwidget()方法不会记录widget的配置，这可能导致添加时数据异常，需要修改ukui quick
                // 详见 https://gitee.com/openkylin/ukui-quick/pulls/339/commits
                primaryWindow->mainView()->addWidget(widget);
                if(isexistSameTypePartition || (isautoorg && !needOriganization)){
                    primaryWindow->mainView()->addDisableInstance(widgetInstanceId);
                }
                qDebug() << newConfig->data();
                config->removeChild("widgets", widget->instanceId());
            }

            // 这里将扩展屛item的配置从副屛配置中移除，恢复时基于主屏配置重新添加，这是为了确保多屏恢复时配置在item added信号处理中是最新的
            //primaryWindow->config()->removeChild(UkuiQuick::Island::viewGroupName(), screenId);
        }
    }
}

void DesktopWidgetsManager::restoreWidgetsFromPrimaryScreen()
{
    // 恢复插件到各个窗口，用于从镜像模式恢复到扩展模式
    PlatformWindow *primaryWindow = nullptr;
    QMap<int, PlatformWindow *> windowsMap;
    auto windows = PlatformWindow::allWindows();
    for (auto window : windows) {
        if (window->id() == 0) {
            primaryWindow = window;
        }
        windowsMap.insert(window->id(), window);
    }
    if (!primaryWindow) {
        qCritical() << "未找到主屏窗口，插件还原终止";
        return;
    }
    QMap<int,int> orgTypeMap;
    auto widgets = primaryWindow->mainView()->widgets();
    for (UkuiQuick::Widget *widget: qAsConst(widgets)) {
        auto config = widget->config();
        if (config->getValue("id").toString() == "desktop-organization-tool") {
            int orgType = config->getValue("organizationType").toInt();
            if (orgType >= 0 && orgType <= 2) {
                int oldScreenId = config->getValue("oldScreenId").toInt();
                // 判断是否可以恢复到副屛
                auto window = windowsMap.value(oldScreenId);
                if (!window) {
                    continue;
                }
                orgTypeMap.insert(orgType,widget->instanceId());
            }
        }
        if (config->contains("oldScreenId")) {
            int oldScreenId = config->getValue("oldScreenId").toInt();
            // 判断是否可以恢复到副屛
            auto window = windowsMap.value(oldScreenId);
            if (!window) {
                continue;
            }

            // 将主屏插件最新的配置同步到副屛，并添加widget
            auto containerWidgetConfig = window->mainView()->config();
            containerWidgetConfig->addChild("widgets", widget->config()->data());
            auto newConfig = containerWidgetConfig->child("widgets", widget->instanceId());
            auto newWidget = UkuiQuick::WidgetLoader::globalLoader().loadWidget(widget->id());

            // 恢复副屛插件的位置
            if (newConfig->contains("exceptedRect")) {
                QVariantMap map = config->getValue("exceptedRect").toMap();
                newConfig->setValue("x", map.value("x"));
                newConfig->setValue("y", map.value("y"));
                newConfig->setValue("width", map.value("width"));
                newConfig->setValue("height", map.value("height"));
            }

            newWidget->setConfig(newConfig);
            newWidget->setInstanceId(widget->instanceId());

            // 将带有配置的widget添加到副屛
            window->mainView()->addWidget(newWidget);

            // 移除主屏插件和对应插件配置、副屛插件多余配置
            newWidget->config()->removeKey("exceptedRect");
            newWidget->config()->removeKey("oldScreenId");

            primaryWindow->mainView()->removeWidget(widget);
            primaryWindow->mainView()->config()->removeChild("widgets", widget->instanceId());
        }
    }

    bool needOriganization = Peony::GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool();
    auto containerConfig = primaryWindow->mainView()->config();
    auto disabledInstances = primaryWindow->mainView()->disableInstances();

    // 通过container config读取对应子widget的config，通过id和org type判断是否需要启用
    for (auto instanceId : disabledInstances) {
        auto config = containerConfig->child("widgets", instanceId);
        if (config && config->getValue("id").toString() == "desktop-organization-tool" && config->contains("oldScreenId")) {
            int orgType = config->getValue("organizationType").toInt();
            if (orgType >= 0 && orgType <= 2) {
                int oldScreenId = config->getValue("oldScreenId").toInt();
                // 判断是否可以恢复到副屛
                auto window = windowsMap.value(oldScreenId);
                if (!window) {
                    continue;
                }
                if(orgTypeMap.contains(orgType)){
                    int frominstanceid =  orgTypeMap.value(orgType);
                    Q_EMIT  DesktopOrganizationManager::instance()->restoreSameTypeWidgetToNewScreenRequest(frominstanceid,instanceId);
                }
                primaryWindow->mainView()->removeDisableInstance(instanceId);
                auto newWidget = UkuiQuick::WidgetLoader::globalLoader().loadWidget("desktop-organization-tool");
                if (config->contains("exceptedRect")) {
                    QVariantMap map = config->getValue("exceptedRect").toMap();
                    config->setValue("x", map.value("x"));
                    config->setValue("y", map.value("y"));
                    config->setValue("width", map.value("width"));
                    config->setValue("height", map.value("height"));
                }

                // 将带有配置的widget添加到副屛
                newWidget->setConfig(config);
                newWidget->setInstanceId(instanceId);
                window->mainView()->addWidget(newWidget);
                if(!needOriganization){
                    window->mainView()->addDisableInstance(instanceId);
                }
                // 移除主屏插件和对应插件配置、副屛插件多余配置
                newWidget->config()->removeKey("exceptedRect");
                newWidget->config()->removeKey("oldScreenId");
                primaryWindow->mainView()->config()->removeChild("widgets", instanceId);
            }
        }
    }
    Q_EMIT DesktopOrganizationManager::instance()->checkorganizationExistRequest();
}

void DesktopWidgetsManager::swapWidgets(PlatformWindow *window1, PlatformWindow *window2)
{
    // 将两个窗口的插件进行交换，用于主副屛切换场景
    // fixme: 是否需要处理图标平移动效？
    int window1Id = window1->mainView()->instanceId();
    int window2Id = window2->mainView()->instanceId();
    bool ok1 = window1->loadMainView("qml-desktop", window2Id);
    bool ok2 = window2->loadMainView("qml-desktop", window1Id);
    return;
}

int DesktopWidgetsManager::addWidget(PlatformWindow *window, const QString &pluginId)
{
    // 添加插件
    int instanceId = getNextInstanceId();
    window->mainView()->addWidget(pluginId, instanceId);
    return instanceId;
}
