#include"desktop-widgets-menu.h"
#include"desktop-window-manager.h"
#include"directory-view-menu.h"
void WidgetMenu::setupActions()
{
    /* 创建 Action Groups */
    m_sortTypeGroup = new QActionGroup(this);
    m_viewtypeGroup = new QActionGroup(this);
    m_sortOrderGroup = new QActionGroup(this);
    /* 排序类型菜单项 */
    // Most Used
    m_renamePartition = addAction(tr("Rename Partition"));
    m_renamePartition->setCheckable(false);
    addSeparator();
    // Add Mode 子菜单
    QMenu *viewMenu = new QMenu(tr("View Type"));
    m_iconView = viewMenu->addAction(tr("Icon View"));
    m_listView = viewMenu->addAction(tr("List View"));
    m_iconView->setCheckable(true);
    m_listView->setCheckable(true);
    m_viewtypeGroup->addAction(m_iconView);
    m_viewtypeGroup->addAction(m_listView);
    addMenu(viewMenu);

    // 其他排序类型
    QMenu *SortMenu = new QMenu(tr("Sort Type"));
    SortMenu->setStyle(new MenuStyle(SortMenu->style()));
    m_actFileName = SortMenu->addAction(tr("File Name"));
    m_actFileName->setCheckable(true);
    m_sortTypeGroup->addAction(m_actFileName);

    m_actDate = SortMenu->addAction(tr("Time Modified"));
    m_actDate->setCheckable(true);
    m_sortTypeGroup->addAction(m_actDate);

    m_actFileType = SortMenu->addAction(tr("File Type"));
    m_actFileType->setCheckable(true);
    m_sortTypeGroup->addAction(m_actFileType);

    m_actFileSize = SortMenu->addAction(tr("File Size"));
    m_actFileSize->setCheckable(true);
    m_sortTypeGroup->addAction(m_actFileSize);


    SortMenu->addSeparator();
    QStringList sortNames;
    sortNames.append(tr("Descending Order"));
    sortNames.append(tr("Ascending Order"));
    auto sortMap = SpacesSettings::globalInstance()->getSpacesSortConfig(m_instanceId);
    m_SortType = sortMap.value("partitionSortType").toInt();
    m_useGlobalSorting = SpacesSettings::globalInstance()->getSortType();
    int sortType = m_useGlobalSorting ? SpacesSettings::globalInstance()->getSortType() : m_SortType;
    if (sortType == 1) {
        sortNames.clear();
        sortNames.append(tr("Newest to oldest"));
        sortNames.append(tr("Oldest to newest"));
    } else if (sortType == 3) {
        sortNames.clear();
        sortNames.append(tr("Files from large to small"));
        sortNames.append(tr("Files from small to large"));
    }
    SortMenu->addSeparator();
    m_sortDes = SortMenu->addAction(QIcon::fromTheme("view-sort-descending-symbolic"),sortNames.at(0));
    m_sortAsc =SortMenu->addAction(QIcon::fromTheme("view-sort-ascending-symbolic"),sortNames.at(1));
    m_sortDes->setCheckable(true);
    m_sortAsc->setCheckable(true);
    m_sortOrderGroup->addAction(m_sortDes);
    m_sortOrderGroup->addAction(m_sortAsc);
    // int sortOrder = GlobalSettings::getInstance()->getValue(DESKTOP_SORT_ORDER).toInt();
    // tmp.at(sortOrder)->setCheckable(true);
    // tmp.at(sortOrder)->setChecked(true);
    // for (int i = 0; i < tmp.count(); i++) {
    //     connect(tmp.at(i), &QAction::triggered, this, [=](){
    //         Q_EMIT setSortOrder(i);
    //         GlobalSettings::getInstance()->setValue(DESKTOP_SORT_ORDER, i);
    //     });
    // }

    addMenu(SortMenu);
    m_actdisbandPartition = addAction(tr("Disband Partition"));
}

void WidgetMenu::updateMenuStates() {
    // // 从当前设置源获取值
    auto sortMap = SpacesSettings::globalInstance()->getSpacesSortConfig(m_instanceId);
    m_SortType = sortMap.value("partitionSortType").toInt();
    m_SortOrder =  sortMap.value("partitionSortAsc").toInt();
    int sortType = m_useGlobalSorting ? SpacesSettings::globalInstance()->getSortType() : m_SortType;
    int sortorder = m_useGlobalSorting ? SpacesSettings::globalInstance()->getSortOrder() : m_SortOrder;
    m_islistmodel = SpacesSettings::globalInstance()->getIsPartitionListMode(m_instanceId);
    //int sortOrder = m_useGlobalSorting ? SpacesSettings::globalInstance()->getSortOrder() : m_currentSortOrder;
    if(m_islistmodel){
        m_listView->setChecked(true);
    }else{
        m_iconView->setChecked(true);
    }
    // 更新单选状态
    QAction* typeActions[] = {m_actFileName,
                               m_actDate, m_actFileType, m_actFileSize, m_sortAsc,m_sortDes};
    if (sortType >= 0 && sortType < 4) {
        if (typeActions[sortType]) {
            typeActions[sortType]->setChecked(true);
        }
    }
    if(sortorder==0){
        if (typeActions[5]) {
            typeActions[5]->setChecked(true);
        }
    }else if(sortorder==1){
        if (typeActions[5]) {
            typeActions[4]->setChecked(true);
        }
    }
}

void WidgetMenu::connectSignals() {
    // 排序类型变更
    connect(m_actFileName, &QAction::triggered, [this] { handleSortTypeChange(0); });
    connect(m_actDate, &QAction::triggered, [this] { handleSortTypeChange(1); });
    connect(m_actFileType, &QAction::triggered, [this] { handleSortTypeChange(2); });
    connect(m_actFileSize, &QAction::triggered, [this] { handleSortTypeChange(3); });
    connect(m_sortDes, &QAction::triggered, [this] { handleSortOrderChange(Qt::SortOrder(0)); });
    connect(m_sortAsc, &QAction::triggered, [this] { handleSortOrderChange(Qt::SortOrder(1)); });
    connect(m_renamePartition, &QAction::triggered, [this] {
        Q_EMIT DesktopOrganizationManager::instance()->renameFocusRequest(m_instanceId,9999);
    });
    connect(m_listView,&QAction::triggered, [this]{
        SpacesSettings::globalInstance()->setViewModel(m_instanceId,true);
    });
    connect(m_iconView,&QAction::triggered, [this]{
        SpacesSettings::globalInstance()->setViewModel(m_instanceId,false);
    });
    connect(m_actdisbandPartition, &QAction::triggered, [this] {
       DesktopOrganizationManager::instance()->disbandPartition(m_instanceId);
    });
    // // 全局设置切换
    // connect(m_actGlobalSettings, &QAction::toggled, this, &SortMenu::handleGlobalSettingsChange);
}

void WidgetMenu::handleSortTypeChange(int type) {
    m_SortType = type;
    if (m_useGlobalSorting) {
        // 更新全局设置
        SpacesSettings::globalInstance()->setSortType(type);
    } else {
        // 发送当前视图排序请求
        //m_FileItemModel->doSort(m_SortType, m_currentSortOrder,m_userAddedFirst);
        SpacesSettings::globalInstance()->setSpacesSortConfig(m_instanceId,m_SortType,m_SortOrder);
        DesktopOrganizationManager::instance()->doSort(m_instanceId,m_SortType,m_SortOrder);
    }
    updateMenuStates();
}

void WidgetMenu::handleSortOrderChange(Qt::SortOrder order)
{
    m_SortOrder = order;
    if (m_useGlobalSorting) {
        // 更新全局设置
        SpacesSettings::globalInstance()->setSortOrder(order);
    } else {
        // 发送当前视图排序请求
        //m_FileItemModel->doSort(m_SortType, m_currentSortOrder,m_userAddedFirst);
        SpacesSettings::globalInstance()->setSpacesSortConfig(m_instanceId,m_SortType,order);
        DesktopOrganizationManager::instance()->doSort(m_instanceId,m_SortType,m_SortOrder);
    }
}

#include <QSettings>
#include <QTimer>
#include <QDebug>

static SpacesSettings *instance = nullptr;

SpacesSettings *SpacesSettings::globalInstance()
{
    if (!instance) {
        instance = new SpacesSettings;
    }
    return instance;
}

SpacesSettings::SpacesSettings(QObject *parent)
    : QObject{parent}
{
    delayer = new QTimer(this);
    delayer->setInterval(500);
    delayer->setSingleShot(true);

    QSettings settings;
    settings.beginGroup("Common");
    sortType = settings.value("partitionSortType", 0).toInt();
    sortOrder = settings.value("partitionSortAsc", false).toBool()? Qt::AscendingOrder: Qt::DescendingOrder;
    useGlobalSorting = settings.value("partitionUseGlobalSorting", true).toBool();
    isSpacesCollapsed = settings.value("ispartitionCollapsed").toBool();
    isSpaceListMode = settings.value("ispartitionListMode").toBool();
    settings.endGroup();

    auto startTimer = [=]{delayer->start();};
    connect(this, &SpacesSettings::sortOrderChanged, delayer, startTimer);
    connect(this, &SpacesSettings::sortTypeChanged, delayer, startTimer);
    connect(delayer, &QTimer::timeout, this, [=]{
        QSettings settingsToSync;
        settingsToSync.beginGroup("Common");
        settingsToSync.setValue("partitionSortType", sortType);
        bool sortAsc = (sortOrder == Qt::AscendingOrder);
        settingsToSync.setValue("partitionSortAsc", sortAsc);
        settingsToSync.setValue("partitionUseGlobalSorting", useGlobalSorting);
        settingsToSync.endGroup();
        settingsToSync.sync();
        qDebug() << settingsToSync.fileName();

        Q_EMIT sortConfigChanged();
    });
}

Qt::SortOrder SpacesSettings::getSortOrder() const
{
    return sortOrder;
}

void SpacesSettings::setSortOrder(Qt::SortOrder newSortOrder)
{
    if (sortOrder == newSortOrder)
        return;
    sortOrder = newSortOrder;
    Q_EMIT sortOrderChanged();
}

int SpacesSettings::getSortType() const
{
    return sortType;
}

void SpacesSettings::setSortType(int newSortType)
{
    if (sortType == newSortType)
        return;
    sortType = newSortType;
    Q_EMIT sortTypeChanged();
}

bool SpacesSettings::getUseGlobalSorting() const
{
    return useGlobalSorting;
}

void SpacesSettings::setUseGlobalSorting(bool newUseGlobalSorting)
{
    if (useGlobalSorting == newUseGlobalSorting)
        return;
    useGlobalSorting = newUseGlobalSorting;
    if (useGlobalSorting) {
        QSettings settings;
        settings.beginGroup("Partition");
        settings.clear();
        settings.endGroup();
        settings.sync();
    }
    Q_EMIT useGlobalSortingChanged();
}

void SpacesSettings::setSpacesSortConfig(int instanceId, int sortType, int sortOrder)
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginWriteArray(QString::number(instanceId), 2);
    settings.setValue("partitionSortType", sortType);
    settings.setValue("partitionSortAsc", sortOrder);
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

QVariantHash SpacesSettings::getSpacesSortConfig(int instanceId)
{
    QVariantHash result;
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginReadArray(QString::number(instanceId));

    QVariant value = settings.value("partitionSortType");
    if (value.isNull()) {
        result.insert("partitionSortType", this->getSortType());
    } else {
        result.insert("partitionSortType", value);
    }

    value = settings.value("partitionSortAsc");
    if (value.isNull()) {
        result.insert("partitionSortAsc", this->getSortOrder() == Qt::AscendingOrder? 0: 1);
    } else {
        result.insert("partitionSortAsc", value);
    }

    settings.endArray();
    settings.endGroup();
    return result;
}

void SpacesSettings::setViewModel(int instanceId, bool islistModel)
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginWriteArray(QString::number(instanceId), 1);
    settings.setValue("ispartitionListMode", islistModel);
    settings.endArray();
    settings.endGroup();
    settings.sync();
    Q_EMIT DesktopOrganizationManager::instance()->viewModelChanged(instanceId,islistModel);
}

bool SpacesSettings::getIsPartitionListMode(int instanceId) const
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginReadArray(QString::number(instanceId));
    QVariant value = settings.value("ispartitionListMode");
    return value.toBool();
}

bool SpacesSettings::getIsExpand(int instanceId)
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginReadArray(QString::number(instanceId));
    QVariant value = settings.value("ispartitionExpand");
    if(!value.isValid()){
        return true;
    }
    return value.toBool();
}

void SpacesSettings::setIsExpand(int instanceId, bool expand)
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginWriteArray(QString::number(instanceId), 1);
    settings.setValue("ispartitionExpand", expand);
    settings.endArray();
    settings.endGroup();
    settings.sync();
}

// bool SpacesSettings::getIsSpacesCollapsed() const
// {
//     return isSpacesCollapsed;
// }

// void SpacesSettings::setIsSpacesCollapsed(bool newIsSpacesCollapsed)
// {
//     if (isSpacesCollapsed == newIsSpacesCollapsed)
//         return;
//     isSpacesCollapsed = newIsSpacesCollapsed;
//     QSettings settings;
//     settings.beginGroup("Common");
//     settings.setValue("isSpacesCollapsed", newIsSpacesCollapsed);
//     settings.endGroup();
//     settings.sync();
//     Q_EMIT isSpacesCollapsedChanged();
// }


bool SpacesSettings::getIsSpaceListMode() const
{
    return isSpaceListMode;
}

void SpacesSettings::setIsSpaceListMode(bool newIsSpaceListMode)
{
    if (isSpaceListMode == newIsSpaceListMode)
        return;
    isSpaceListMode = newIsSpaceListMode;
    QSettings settings;
    settings.beginGroup("Common");
    settings.setValue("islistModel", newIsSpaceListMode);
    settings.endGroup();
    settings.sync();
    Q_EMIT isSpaceListModeChanged();
}

int SpacesSettings::getInitHeight(int instanceId)
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginReadArray(QString::number(instanceId));
    QVariant value = settings.value("initHeight");
    return value.toInt();
}

void SpacesSettings::setInitHeight(int instanceId, int height)
{
    QSettings settings;
    settings.beginGroup("Partition");
    settings.beginWriteArray(QString::number(instanceId), 1);
    settings.setValue("initHeight", height);
    settings.endArray();
    settings.endGroup();
    settings.sync();
}


