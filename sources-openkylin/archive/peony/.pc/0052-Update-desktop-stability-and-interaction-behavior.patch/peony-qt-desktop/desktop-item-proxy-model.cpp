/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "common.h"

#include "desktop-item-proxy-model.h"
#include "desktop-window-manager.h"
#include "file-info.h"
#include "file-meta-info.h"
#include "platformwindow.h"
#include "file-operation-utils.h"
#include "global-settings.h"
#include "advanced-desktop-item-model.h"
#include "screen-manager.h"
#include "file-utils.h"
#include "manager-control.h"

#include <QDir>
#include <QDebug>

#include <QLocale>
#include <QCollator>
#include <QStandardPaths>
#include <QUrl>
#include <QMimeData>
#include <QApplication>

using namespace Peony;

QLocale locale = QLocale(QLocale::system().name());
QCollator comparer = QCollator(locale);

bool startWithChinese(const QString &displayName)
{
    //NOTE: a newly created file might could not get display name soon.
    if (displayName.isEmpty()) {
        return false;
    }
    auto firstStrUnicode = displayName.at(0).unicode();
    return (firstStrUnicode <=0x9FA5 && firstStrUnicode >= 0x4E00);
}

DesktopItemProxyModel::DesktopItemProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    //enable number sort, like 100 is after 99
    comparer.setNumericMode(true);

    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(false);
    auto settings = GlobalSettings::getInstance();

    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        m_show_hidden = settings->isExist(SHOW_HIDDEN_PREFERENCE)? settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool() : false;
    } else {
        m_show_hidden = settings->isExist(SHOW_DESKTOP_HIDDEN_PREFERENCE)? settings->getValue(SHOW_DESKTOP_HIDDEN_PREFERENCE).toBool() : false;
    }
    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (SHOW_HIDDEN_PREFERENCE == key || SHOW_DESKTOP_HIDDEN_PREFERENCE == key) {
            handleHiddenPreferenceChange(key);
        } else if (USE_GLOBAL_DEFAULT_SORTING == key) {
            if (GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
                handleHiddenPreferenceChange(SHOW_HIDDEN_PREFERENCE);
            } else {
                handleHiddenPreferenceChange(SHOW_DESKTOP_HIDDEN_PREFERENCE);
            }
        } else if (key == DISPLAY_STANDARD_ICONS ||
                     key == HOME_ICON_VISIBLE ||
                     key == TRASH_ICON_VISIBLE ||
                     key == COMPUTER_ICON_VISIBLE) {
            invalidateFilter();
            Q_EMIT showStandardFile();
        } else if (ALLOW_FILE_OP_PARALLEL == key) {
            bool isAllowFileOpParallel = GlobalSettings::getInstance()->getValue(key).toBool();
            auto fileOpMgr = FileOperationManager::getInstance();
            fileOpMgr->setAllowParallel(isAllowFileOpParallel);
        }
    });
    connect(this, &DesktopItemProxyModel::idChanged, this, &DesktopItemProxyModel::invalidateFilter);
    connect(this, &DesktopItemProxyModel::instanceIdChanged, this, &DesktopItemProxyModel::invalidateFilter); //整理工具不会发生instanceId变更情况，但从框架层面来说不排除

    // 和desktop organization manager交互
    // void requestSort(int instanceId, int type);
    // void requestDisbandPartation(int instanceId);
    // void requestUpdateOrganizationView(int instanceId);
    auto desktopOrgMgr = DesktopOrganizationManager::instance();
    connect(desktopOrgMgr, &DesktopOrganizationManager::requestSort, this, &DesktopItemProxyModel::onRequestSort);
    connect(desktopOrgMgr, &DesktopOrganizationManager::requestDisbandPartation, this, &DesktopItemProxyModel::onRequestDisbandPartation);
    connect(desktopOrgMgr, &DesktopOrganizationManager::requestUpdateOrganizationView, this, &DesktopItemProxyModel::onRequestUpdateOrganizationView);
    connect(DesktopOrganizationManager::instance(),&DesktopOrganizationManager::mergeSameTypeWidgetToPrimaryScreenRequest,this,[=](int frominstanceid, int toinstanceid){
        if(m_instanceId == frominstanceid){
            for (int i = 0; i < rowCount(); i++) {
                auto modelIndex = index(i, 0);
                auto uri = modelIndex.data(Qt::UserRole).toString();
                auto metaInfo = FileMetaInfo::fromUri(uri);
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID,toinstanceid);
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_MERGEFROM,frominstanceid);
                //metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER,frominstanceid);
            }
            Q_EMIT desktopOrgMgr->requestUpdateOrganizationView(toinstanceid);
        }
    });

    connect(DesktopOrganizationManager::instance(),&DesktopOrganizationManager::restoreSameTypeWidgetToNewScreenRequest,this,[=](int frominstanceid,int toinstanceid){
        if(m_instanceId == frominstanceid){
            for (int i = 0; i < rowCount(); i++) {
                auto modelIndex = index(i, 0);
                auto uri = modelIndex.data(Qt::UserRole).toString();
                auto metaInfo = FileMetaInfo::fromUri(uri);
                if(metaInfo){
                    int filescreenid = metaInfo->getMetaInfoInt(SCREEN_ID);
                    int preinstanceid = metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_MERGEFROM);
                    auto partitonVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID_MERGEFROM);
                    bool isvaild = partitonVariant.isValid();
                    if(/*m_id != filescreenid &&*/ isvaild && preinstanceid!=-1 && toinstanceid == preinstanceid){
                         metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID,preinstanceid);
                         metaInfo->setMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID_MERGEFROM,-1);
                    }
                }
            }
        }
        Q_EMIT desktopOrgMgr->requestUpdateOrganizationView(m_instanceId);
    });

    qDebug() <<"DesktopItemProxyModel:" <<settings->isExist(SHOW_HIDDEN_PREFERENCE)<<m_show_hidden;

    m_sort_type = settings->getValue(LAST_DESKTOP_SORT_ORDER).toInt();
    m_sort_order = settings->getValue(DESKTOP_SORT_ORDER).toInt();

    m_bwListInfo = new BWListInfo();

    syncBlackAndWhiteData();

    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (key == APP_WHITE_LIST || key == APP_BLACK_LIST) {
            updateBlackAndWriteLists();
        }
    });

    //管控下发桌面快捷方式处理
    syncControlDeliverData();

    connect(GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=] (const QString& key) {
        if (key == CONTROL_DELIVER_DESKTOP_FILE) {
            qInfo() << "control deliver desktop file changed";
            //todo, update desktop file list
            syncControlDeliverData();
        }
    });
}

DesktopItemProxyModel::~DesktopItemProxyModel()
{
    delete m_bwListInfo;
}

bool DesktopItemProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!sourceModel())
        return false;

    auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (sourceIndex.data().toString().isEmpty())
        return false;
    auto uri = sourceIndex.data(Qt::UserRole).toString();
    auto info = FileInfo::fromUri(uri);
    bool  useDesktopOriganization =  GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool();
    auto metaInfo = FileMetaInfo::fromUri(uri);
    bool isManualPartition = (!(m_origanizationType>=0 && m_origanizationType<=2) && m_instanceId !=-1);
    bool isindesktop = m_instanceId ==-1 ? true :false;

    if (!m_show_hidden) {
        auto metainfo = FileMetaInfo::fromUri(uri);
        if (metainfo->hasHiddenInfo()) {
            bool isHidden = metainfo->isHidden();
            if (isHidden) {
                return false;
            }
        } else {
            bool isHidden = info->isHiddenFile();
            if (isHidden) {
                return false;
            }
        }
    }

    if((useDesktopOriganization || (isManualPartition &&  m_instanceId!=-1))){
        auto partitonVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID);
        bool isvaild = partitonVariant.isValid();
        int instanceId = -1;
        int unorderinstanceId = -1;
        if(isvaild){
            instanceId = partitonVariant.toString().toInt();
        }else{
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
        }
        bool flag = instanceId == m_instanceId;
        if(!flag || m_instanceId !=-1){
            return flag;
        }
    }
    //task#74174 通过id筛选出需要在该view中显示项
    int id = sourceIndex.data(Qt::UserRole+3).toInt();

    if (m_id != id) {
        return false;
    }

    //fix desktop show Desktop folder issue, bug#20293
    if (QUrl(uri).path() == QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Desktop"
        || QUrl(uri).path() == QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Desktop" + "/Desktop")
    {
        if (! QFile::exists(QUrl(uri).path()))
           return false;
    }

//    if (!GlobalSettings::getInstance()->getValue(DISPLAY_STANDARD_ICONS).toBool()) {
//        if (uri == "computer:///"
//            || uri == "trash:///"
//            || uri == ("file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
//            || uri == QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
//        {
//            return false;
//        }
//    }

    //家目录图标显示隐藏，只要DISPLAY_STANDARD_ICONS 或者 HOME_ICON_VISIBLE 其中任意1个为false值即可
    //但是如果改为隐藏后，需要显示，必须保证两个值都为true, 回收站和计算机图标为同样逻辑
    bool display_standard_icons = GlobalSettings::getInstance()->getValue(DISPLAY_STANDARD_ICONS).toBool();
    if (! GlobalSettings::getInstance()->getValue(HOME_ICON_VISIBLE).toBool() || ! display_standard_icons) {
        if (uri == ("file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
            || uri == QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
        {
            return false;
        }
    }

    if (((! GlobalSettings::getInstance()->getValue(TRASH_ICON_VISIBLE).toBool() || ! display_standard_icons)
         && uri == "trash:///")) {
       return false;
    }

    if (((! GlobalSettings::getInstance()->getValue(COMPUTER_ICON_VISIBLE).toBool() || ! display_standard_icons)
         && uri == "computer:///")) {
       return false;
    }

    if (info->isDesktopFile() && nullptr != info->desktopName()){
        qDebug() << "desktopName"<<info->desktopName();
        if (m_bwListInfo->isBlackListMode()){
            return !m_bwListInfo->desktopNameExist(info->desktopName());
        } else if (m_bwListInfo->isWriteListMode()){
            return m_bwListInfo->desktopNameExist(info->desktopName());
        }
    }
    //else if(!useDesktopOriganization){
    auto partitonVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID);
    bool isvaild = partitonVariant.isValid();
    if(isvaild){
        int instanceId = partitonVariant.toString().toInt();
        if(instanceId !=m_instanceId){
            return false;
        }
    }
    //}
    return true;
}

bool DesktopItemProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    //qDebug()<<"less than";
    if (m_sort_type == Other) {
        return true;
    }

    auto leftUri = source_left.data(Qt::UserRole).toString();
    auto leftInfo = FileInfo::fromUri(leftUri);
    //auto leftMetaInfo = FileMetaInfo::fromUri(leftUri);

    auto rightUri = source_right.data(Qt::UserRole).toString();
    auto rightInfo = FileInfo::fromUri(rightUri);
    //auto rightMetaInfo = FileMetaInfo::fromUri(rightUri);

    //computer home and trash first
    if (source_left.row() < 3) {
        if (source_right.row() < source_left.row()) {
            return (sortOrder()==Qt::AscendingOrder)? false: true;
        }
        return (sortOrder()==Qt::AscendingOrder)? true: false;
    }
    if (source_right.row() < 3) {
        if (source_left.row() < source_right.row()) {
            return (sortOrder()==Qt::AscendingOrder)? true: false;
        }
        return (sortOrder()==Qt::AscendingOrder)? false: true;
    }

    //dir first
    if (leftInfo->isDir()) {
        if (rightInfo->isDir()) {
            //fix bug#89115, folders not sort by name
            if (m_sort_type != ModifiedDate){
                if (leftInfo->isDir() && rightInfo->isDir())
                {
                    goto default_sort;
                }
            }
        } else {
            return (sortOrder()==Qt::AscendingOrder)? true: false;
        }
    } else {
        if (rightInfo->isDir()) {
            return (sortOrder()==Qt::AscendingOrder)? false: true;
        }
    }

    //qDebug()<<"sort in desktop"<<SortType(m_sort_type)<<m_sort_type;
    switch (m_sort_type) {
    case FileName: {
        if (FileOperationUtils::leftNameIsDuplicatedFileOfRightName(leftInfo->displayName(), rightInfo->displayName())) {
            return FileOperationUtils::leftNameLesserThanRightName(leftInfo->displayName(), rightInfo->displayName());
        }
        goto default_sort;
    }
    case ModifiedDate: {
        if (leftInfo->modifiedTime() == rightInfo->modifiedTime())
            goto default_sort;
        return leftInfo->modifiedTime() > rightInfo->modifiedTime();
    }
    case FileType: {
        if (leftInfo->fileType() == rightInfo->fileType())
            goto default_sort;

        QString leftDisplayType = leftInfo->fileType();
        QString rightDisplayType = rightInfo->fileType();

        if (startWithChinese(leftDisplayType)) {
            if (!startWithChinese(rightDisplayType)) {
                return (sortOrder()==Qt::AscendingOrder)? true: false;
            } else {
                //chinese pinyin sort order is reversed compared with english.
                //return !QSortFilterProxyModel::lessThan(source_left, source_right);
                //fix bug#89115, chinese files not sort by name pinyin
                return comparer.compare(leftDisplayType, rightDisplayType) > 0;
            }
        } else {
            if (startWithChinese(rightDisplayType)) {
                return (sortOrder()==Qt::AscendingOrder)? false: true;
            }
            return comparer.compare(leftDisplayType, rightDisplayType) > 0;
        }
    }
    case FileSize: {
        if (leftInfo->size() == rightInfo->size())
            goto default_sort;
        return leftInfo->size() > rightInfo->size();
    }
    }

default_sort:
    //when sort value is same, use name to sort, fix refresh change order issue
    //fix bug#99928, desktop sort not same with folder issue, and releated to bug#92525
    QString leftDisplayName = leftInfo->displayName();
    QString rightDisplayName = rightInfo->displayName();

    if (startWithChinese(leftInfo->displayName())) {
        if (!startWithChinese(rightInfo->displayName())) {
            return (sortOrder()==Qt::AscendingOrder)? true: false;
        } else {
            //chinese pinyin sort order is reversed compared with english.
            //return !QSortFilterProxyModel::lessThan(source_left, source_right);
            //fix bug#89115, chinese files not sort by name pinyin
            return comparer.compare(leftInfo->displayName(), rightInfo->displayName()) > 0;
        }
    } else {
        if (startWithChinese(rightInfo->displayName())) {
            return (sortOrder()==Qt::AscendingOrder)? false: true;
        }
        return comparer.compare(leftDisplayName, rightDisplayName) > 0;
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

void DesktopItemProxyModel::setShowHidden(bool showHidden)
{
    if (!MANAGER_CONTROL->checkShowHiddenPermission()) {
        return;
    }

    GlobalSettings::getInstance()->setGSettingValue(SHOW_HIDDEN_PREFERENCE, showHidden);
    m_show_hidden = showHidden;
    invalidateFilter();
}

int DesktopItemProxyModel::updateBlackAndWriteLists()
{
    m_bwListInfo->clearBWlist();
    syncBlackAndWhiteData();
    //重新过滤显示
    invalidateFilter();
    return 0;
}

void DesktopItemProxyModel::invalidateModel()
{
    invalidateFilter();
    Q_EMIT showHiddenFile();
}

QVariantMap DesktopItemProxyModel::updateIconPositionsByOrgtool(const QStringList &mimeData, Qt::DropAction action, int row, bool onItem)
{
    // 从整理工具拖拽文件到桌面
    QPoint currentPos;
    int id;
    QPoint pos = QCursor::pos();
    QVariantMap result;
    auto model = qobject_cast<Peony::AdvancedDesktopItemModel*>(sourceModel());
    // DesktopWindowManager::getInstance()->createIdAndPos(&id, &currentPos);
    int screenId = SCREEN_MANAGER->getScreenIdAt(pos);
    QPoint localPos = SCREEN_MANAGER->mapFromGlobal(screenId, pos);
    PlatformWindow *window =qobject_cast<PlatformWindow*>(SCREEN_MANAGER->getWindowAt(pos));
    QPoint tmpPos = window->getGridPosFromMousePos(localPos);
    dropSucess = true;
    QPoint nextEmptyPos = window->findNextEmptyGridPos(tmpPos);
     bool desktopAutoLayout = GlobalSettings::getInstance()->getValue(DESKTOP_USE_AUTO_LAYOUT).toBool();
    if(nextEmptyPos != tmpPos && !desktopAutoLayout){
        return result;
    }
    if(desktopAutoLayout){
        qApp->setProperty("AutoLayoutByorg",true);
    }
    for (int var = 0; var < mimeData.size(); ++var) {
        auto uri = mimeData[var];
        auto index = model->indexFromUri(uri);
        QPoint metaPos = model->data(index,PositionRole).toPoint();
        QStringList tmpurilist;
        tmpurilist.append(uri);
        QPoint nextEmptyPos = window->findNextEmptyGridPos(tmpPos);
        tmpPos = nextEmptyPos;
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER,-1);
            metaInfo->setMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION,0);
        }
        updateIconPositions(tmpurilist,action,row,QPoint(tmpPos.x() - metaPos.x(),tmpPos.y() - metaPos.y()),onItem);
        result.insert(uri,tmpPos);
        window->setFileMetaInfoPos(uri,QPoint(tmpPos.x(),tmpPos.y()));
    }
    invalidateFilter();
    return result;
}

void DesktopItemProxyModel::updateIconPositionsByOtherView(const QStringList &mimeData/*, Qt::DropAction action, int row, bool onItem*/)
{
    auto model = qobject_cast<Peony::AdvancedDesktopItemModel*>(sourceModel());
    // ("dragInsertIndex",index)
    int row = qApp->property("dragInsertIndex").toInt();
    bool onItem  = false;
    QPoint pos = QCursor::pos();
    int screenId = SCREEN_MANAGER->getScreenIdAt(pos);
    QPoint localPos = SCREEN_MANAGER->mapFromGlobal(screenId, pos);
    PlatformWindow *window =qobject_cast<PlatformWindow*>(SCREEN_MANAGER->getWindowAt(pos));
    QPoint tmpPos = window->getGridPosFromMousePos(localPos);
    auto action = Qt::DropAction::MoveAction;
    if(screenId!=id()){
        return;
    }
    for (int var = 0; var < mimeData.size(); ++var) {
        auto uri = mimeData[var];
        auto index = model->indexFromUri(uri);
        QPoint metaPos = model->data(index,PositionRole).toPoint();
        QStringList tmpurilist;
        tmpurilist.append(uri);
        QPoint nextEmptyPos = window->findNextEmptyGridPos(tmpPos);
        updateIconPositions(tmpurilist,action,row,QPoint(tmpPos.x() - metaPos.x(),tmpPos.y() - metaPos.y()),onItem);
    }
    //});
}

void DesktopItemProxyModel::onRequestSort(int instanceId, int type,int order)
{
    if (instanceId == m_instanceId) {
        sort(type);
        setSortType(type);
        setSortOrder(order);
        sort(1);
        sort(0, Qt::SortOrder(order));
    }
}

void DesktopItemProxyModel::onRequestDisbandPartation(int instanceId)
{
    if (instanceId == m_instanceId) {
        // todo: 解散分区，分区内文件metainfo更新，此方法需要注意时序，不能在widget remove后再调用
        for (int i = 0; i < count(); ++i) {
            auto modelIndex = index(i, 0);
            auto uri = modelIndex.data(Qt::UserRole).toString();
            auto metaInfo = FileMetaInfo::fromUri(uri);
            if (metaInfo) {
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, -1);
                metaInfo->setMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION,0);
            }
        }
        //Q_EMIT updateDesktopView();
    }
}

void DesktopItemProxyModel::onRequestUpdateOrganizationView(int instanceId)
{
    if (instanceId == m_instanceId) {
        updateFilter();
    }
}

QString DesktopItemProxyModel::getBlackAndWhiteModel()
{
    return m_bwListInfo->getMode();
}

bool DesktopItemProxyModel::getBlackAndWhiteListExist(QString name)
{
    return m_bwListInfo->desktopNameExist(name);
}

QSet<QString> DesktopItemProxyModel::getBWListInfo()
{
    return m_bwListInfo->getBWListInfo();
}

void DesktopItemProxyModel::setDesktopUseAutoLayout(bool desktopUseAutoLayout)
{
    m_desktopUseAutoLayout = desktopUseAutoLayout;
}

bool DesktopItemProxyModel::getDesktopUseAutoLayout() const
{
    return m_desktopUseAutoLayout;
}

QStringList DesktopItemProxyModel::getSortedUris() const
{
    return m_sortedUris;
}

void DesktopItemProxyModel::setSortedUris(const QStringList &sortedUris)
{
    m_sortedUris = sortedUris;
//    Q_EMIT requestSyncSortedUrisWithDelay();
}

void DesktopItemProxyModel::clearExpectedItemPosForCurrentView()
{
    // note: 目前发现多次排序可能会导致behavior动效不生效，这里reset一下model保证排序动效正常
    beginResetModel();
    endResetModel();
    for (int i = 0; i < rowCount(); i++) {
        auto modelIndex = index(i, 0);
        auto uri = modelIndex.data(Qt::UserRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        metaInfo->setMetaInfoStringList(RESTORE_ITEM_GRID_POS_ATTRIBUTE, QStringList());
    }
}

void DesktopItemProxyModel::setdata(int i, const QVariant &value, int role)
{
    sourceModel()->setData(index(i, 0), value, role);
    Q_EMIT datechange(i, value, role);
}

void DesktopItemProxyModel::updateIconPositions(const QStringList &mimeData, Qt::DropAction action, int row, const QPoint &offPos, bool onItem)
{
    qDebug() << "drag to current position:"<<row<< " 偏移量:"<<offPos<<" 是否在有效项之上"<<onItem<<"拖拽文件："<<mimeData;
    if ((-1 != row && onItem) || action == Qt::CopyAction) {
        QModelIndex modelindex = index(row, 0);
        if (modelindex.isValid()) {
            QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            //fix can drag copy home folder issue, link to bug#64824
            if (mimeData.contains(homePath) || mimeData.contains("computer:///"))
                return;
        }
    }
    if ((-1 != row && onItem) && action != Qt::CopyAction) {
        QModelIndex modelindex = index(row, 0);
        if (modelindex.isValid()) {
            auto uri = modelindex.data(Qt::UserRole).toString();
            auto info = FileInfo::fromUri(uri);
            qDebug() << "在有效项之上："<<uri;
            if (!info->isDir()|| mimeData.contains(uri)) {
                return;
            }
            qDebug() << "DesktopIconView move file to folder";
            for (auto uuri : mimeData) {
                if ("trash:///" == uuri || "computer:///" == uuri) {
                    return;
                }
            }
            // 将 QVariant 转换为 QMimeData
            QMimeData *data = new QMimeData();
            QList<QUrl> urls;
            QStringList uris = mimeData;
            // 将 QVariantMap 转换为 QMimeData
            for (QString uri : mimeData) {
                QUrl url = uri;
                if (!urls.contains(url))
                    urls<<url;
                urls<<url;
            }

            data->setUrls(urls);
            auto string = uris.join(" ");
            data->setData("peony-qt/encoded-uris", string.toUtf8());
            data->setText(string);

            QModelIndex modelindex = index(row, 0);
            bool sucess = sourceModel()->dropMimeData(data, action, -1, -1, modelindex);
            dropSucess = sucess;
            if (!sucess) {
                return;
            }
        }
    }
    Q_EMIT drop(mimeData, action, row, offPos);
}

void DesktopItemProxyModel::addFile(const QStringList &mimeData, Qt::DropAction action, int row, bool isSearch)
{
    qDebug() << mimeData;
    // 将 QVariant 转换为 QMimeData
    QMimeData *data = new QMimeData();
    QList<QUrl> urls;
    QStringList uris = mimeData;
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();

    // 将 QVariantMap 转换为 QMimeData
    for (QString uri : mimeData) {
        // 适配ukui-menu拖拽
        if (uri.startsWith("app://")) {
            uri.replace("app://", "file://");
            auto destDirUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            FileOperationUtils::copyDesktopFile(QStringList() << uri, destDirUri, true);
            return;
        }
        QUrl url = uri;
        if (!urls.contains(url))
            urls<<url;
        urls<<url;
    }

    data->setUrls(urls);
    auto string = uris.join(" ");
    data->setData("peony-qt/encoded-uris", string.toUtf8());
    QVariant isSearchData = QVariant(isSearch);
    data->setData("peony-qt/is-search", isSearchData.toByteArray());
    data->setText(string);
    //如果是按了shift的情况，则选择使用targetmove，因为普通的move在跨文件系统时不生效，为copy操作
    if (mods & Qt::ShiftModifier) {
        action = Qt::TargetMoveAction;
    }

    qDebug() << "drag to current position:"<<row<<"拖拽文件："<<mimeData<<action;
    QModelIndex modelindex = index(row, 0);
    sourceModel()->dropMimeData(data, action, -1, -1, modelindex);
}

void DesktopItemProxyModel::syncBlackAndWhiteData()
{
    auto settings = GlobalSettings::getInstance();
    QStringList whiteList = settings->getValue(APP_WHITE_LIST).toStringList();
    QStringList blackList = settings->getValue(APP_BLACK_LIST).toStringList();

    const bool whiteEnabled = !whiteList.isEmpty();
    const bool blackEnabled = !blackList.isEmpty();

    if (whiteEnabled) {
        m_bwListInfo->setBWListModel(BW_LIST_WHITE);
        m_bwListInfo->setBWListInfo(whiteList);
    } else if (blackEnabled) {
        m_bwListInfo->setBWListModel(BW_LIST_BLACK);
        m_bwListInfo->setBWListInfo(blackList);
    } else {
        m_bwListInfo->setBWListModel(BW_LIST_NORMAL);
    }
    qDebug() << "syncBlackAndWhiteData:" << "1.mode: "<< m_bwListInfo->getMode() << "2.list" << m_bwListInfo->getBWListInfo();
}

void DesktopItemProxyModel::syncControlDeliverData()
{
    auto settings = GlobalSettings::getInstance();
    QMap<QString, QVariant> mapInfo = settings->getValue(CONTROL_DELIVER_DESKTOP_FILE).toMap();

    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    for(auto it=mapInfo.begin(); it != mapInfo.end(); ++it){
        QString soureceFile = it.key();
        QString md5Value = it.value().toString();
        QString FileName = FileUtils::getUriBaseName(soureceFile);
        QString destFile = desktopPath + "/" + FileName;
        qDebug() << "filePath:" <<soureceFile <<"md5 value:"<<md5Value <<"FileName:" <<FileName;

        if (! QFile::exists(QUrl(soureceFile).path())) {
            qWarning() << "soureceFile is not exist:"<<soureceFile;
            continue;
        }

        if (! FileUtils::isAppPathExist(soureceFile)) {
            qInfo() <<"not installed desktop file app:"<<soureceFile;
            continue;
        }

        if (QFile::exists(QUrl(destFile).path())) {
            qDebug() << "target file exist:" <<destFile;
            //delete dest file if file md5 is different
            if (FileUtils::calculateFileMd5(destFile) != md5Value) {
                qInfo() << "md5 is differrnt, delete file:"<<destFile;
                QFile::remove(destFile);
            }
        }

        FileUtils::cmdCopyFile(soureceFile, destFile);
    }
}

int DesktopItemProxyModel::id() const
{
    return m_id;
}

void DesktopItemProxyModel::setId(int newId)
{
    if (m_id == newId)
        return;
    m_id = newId;
    Q_EMIT idChanged();
}

int DesktopItemProxyModel::instanceId() const
{
    return m_instanceId;
}

int DesktopItemProxyModel::origanizationType() const
{
    return m_origanizationType;
}

void DesktopItemProxyModel::setInstanceId(int newInstanceId)
{
    if (m_instanceId == newInstanceId)
        return;
    m_instanceId = newInstanceId;
    Q_EMIT instanceIdChanged();
}

void DesktopItemProxyModel::setOriganizationType(int type)
{
    if (type == m_origanizationType)
        return;
    m_origanizationType = type;
    Q_EMIT origanizationTypeChanged();
}

bool DesktopItemProxyModel::checkoriganization(QString uri) const
{
    bool flag = false;
    auto metaInfo = FileMetaInfo::fromUri(uri);
    auto partitonVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID);
    bool isvaild = partitonVariant.isValid();
    int instanceId = -1;
    int unorderinstanceId = -1;
    if(isvaild){
        instanceId = partitonVariant.toString().toInt();
    }else{
        metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
    }
    bool needOriganization = qApp->property("ReOriganization").toBool();
    bool isConfigExist =  DesktopOrganizationManager::instance()->isOriganizationToolConfigExist();
    if(!isConfigExist){
        needOriganization = true;
    }
    auto unorderpartitonVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID_UNORDER);//文件自定义分区检测 存在则优先匹配
    if(unorderpartitonVariant.isValid()){
        unorderinstanceId = unorderpartitonVariant.toString().toInt();
        flag = unorderinstanceId == m_instanceId;
        if(flag && m_instanceId != None)
        {
            if (metaInfo) {
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, m_instanceId);
            }
        }
        if(unorderinstanceId != -1){ //自定义分区不在桌面
            return flag;
        }
    }
    if(needOriganization){
        auto info = FileInfo::fromUri(uri);
        QString path = info->filePath();
        QString contentType = info->type();
        bool isDesktopFile = path.endsWith(".desktop") || (contentType == "application/x-desktop") || uri=="trash:///" || uri=="computer:///";
        bool isDir = info->isDir() && uri!="trash:///" && uri!="computer:///";
        switch (m_origanizationType) {
        case None:
            flag = (instanceId == -1);
            break;
        case OrgByApplycation:
            flag = isDesktopFile;
            break;
        case OrgByDir:
            flag = isDir;
            break;
        case OrgByFile:
            flag = (!isDir) && (!isDesktopFile);
            break;
        default:         //others
            flag = (instanceId == m_instanceId);
            break;
        }
        if(flag && m_instanceId != None)
        {
            if (metaInfo) {
                metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, m_instanceId);
            }
        }
    }else{
        flag = (instanceId == m_instanceId);
    }
    return flag;
}

QModelIndexList DesktopItemProxyModel::getAllFileIndexes()
{
    QModelIndexList l;
    int i = 0;
    while (this->index(i, 0, QModelIndex()).isValid()) {
        auto index = this->index(i, 0, QModelIndex());
        if (m_show_hidden) {
            l<<index;
        } else {
            auto disyplayName = index.data(Qt::DisplayRole).toString();
            if (disyplayName.isEmpty()) {
                auto uri = this->index(i, 0, QModelIndex()).data(Qt::UserRole).toString();
                disyplayName = FileUtils::getFileDisplayName(uri);
            }
            if (!disyplayName.startsWith(".")) {
                l<<index;
            }
        }

        i++;
    }
    return l;
}

void DesktopItemProxyModel::updateFilter()
{
    invalidateFilter();  // 内部可以调用protected方法
}

void DesktopItemProxyModel::handleHiddenPreferenceChange(const QString& key)
{
    bool useGlobalDefaultSorting = GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool();

    // 检查设置键与排序方式是否匹配
    bool shouldProcess = (key == SHOW_HIDDEN_PREFERENCE) ? useGlobalDefaultSorting : !useGlobalDefaultSorting;
    if (!shouldProcess) return;

    bool showHidden = GlobalSettings::getInstance()->getValue(key).toBool();
    if (m_show_hidden != showHidden) {
        m_show_hidden = showHidden;
        invalidateFilter();
        Q_EMIT showHiddenFile();
    }
}
