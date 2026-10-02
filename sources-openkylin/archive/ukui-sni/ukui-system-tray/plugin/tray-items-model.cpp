/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "tray-items-model.h"
#include "item-group-model.h"
#include "security-function-control.h"
#include "control-panel-model.h"

#include <config-loader.h>
#include <QMetaEnum>
#include <QDebug>
#include <mutex>
#include <QVersionNumber>

static std::once_flag onceFlag;
static TrayItemsModel* g_instance = nullptr;
static const QStringList DEFAULT_ORDERED_ITEMS = {"ukui-power-manager-tray",
                                                  "Fcitx",
                                                  "kylin-virtual-keyboard",
                                                  "kylin-vpn"};
static const QStringList DEFAULT_FIXED_ITEMS = {"ukui-sidebar"};
static const QStringList DEFAULT_CONTROL_PANEL_ITEMS = {"ukui-volume-control-applet-qt",
                                                        "ukui-bluetooth",
                                                        "kylin-nm"};
static const QString DEFAULT_CONTROL_PANEL_ID = {"ukui-sidebar"};
static const QStringList DEFAULT_CAN_HIDE_ITEMS = {"ukui-power-manager-tray",
                                                   "Fcitx"};
TrayItemsModel *TrayItemsModel::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_instance = new TrayItemsModel();
    });
    return g_instance;
}

TrayItemsModel::~TrayItemsModel()
{
    if (!m_applicationInfo) {
        delete m_applicationInfo;
        m_applicationInfo = nullptr;
    }
    g_instance = nullptr;
}

TrayItemsModel::TrayItemsModel(QObject *parent) : QAbstractListModel(parent)
{
    if (!m_config) {
        m_config = UkuiQuick::ConfigLoader::getConfig("org.ukui.systemTray");
    }

    const auto &data = m_config->data();
    //get separateIndex
    if (!data.contains(QStringLiteral("separateIndex"))) {
        m_config->setValue(QStringLiteral("separateIndex"), m_separateIndex);
    } else {
        m_separateIndex = m_config->getValue("separateIndex").toInt();
    }
    //fixed items
    if (!data.contains(QStringLiteral("fixedItems"))) {
        m_fixList = DEFAULT_FIXED_ITEMS;
        m_config->setValue(QStringLiteral("fixedItems"), DEFAULT_FIXED_ITEMS);
    } else {
        m_fixList = m_config->getValue("fixedItems").toStringList();
    }
    //showed items
    if (!data.contains(QStringLiteral("version"))) {
        m_config->setValue(QStringLiteral("version"), QStringLiteral("1.0"));
        m_orderList = DEFAULT_ORDERED_ITEMS;
        m_config->setValue(QStringLiteral("orderedItems"), DEFAULT_ORDERED_ITEMS);
    } else {
        if (!data.contains(QStringLiteral("orderedItems"))) {
            m_orderList = DEFAULT_ORDERED_ITEMS;
            m_config->setValue(QStringLiteral("orderedItems"), DEFAULT_ORDERED_ITEMS);
        } else {
            m_orderList = m_config->getValue(QStringLiteral("orderedItems")).toStringList();
        }
    }

    //hide items
    m_hideList = m_config->getValue(QStringLiteral("trayIconsInhibited")).toStringList();

    //2.x版本号校验
    QString version = m_config->getValue("version").toString();
    if (QVersionNumber::fromString(version).majorVersion() < 2) {
        m_controlPanelList = DEFAULT_CONTROL_PANEL_ITEMS;
        m_config->setValue("controlPanelItems", DEFAULT_CONTROL_PANEL_ITEMS);

        for (const QString & item : DEFAULT_CONTROL_PANEL_ITEMS) {
            m_fixList.removeAll(item);
            m_orderList.removeAll(item);
            m_hideList.removeAll(item);
        }

        m_config->setValue("fixedItems", m_fixList);
        m_config->setValue("orderedItems", m_orderList);
        m_config->setValue("trayIconsInhibited", m_hideList);
        m_config->setValue("version", "2.0");
    } else {
        m_controlPanelList = m_config->getValue("controlPanelItems").toStringList();
    }

    //2.1版本号校验,新增trayIconsCanInhibited
    if (QVersionNumber::fromString(version).minorVersion() < 1) {
        m_config->setValue("version", "2.1");
    }

    //2.2版本号校验,从可隐藏列表中移除vpn
    if (QVersionNumber::fromString(version).minorVersion() < 2) {
        m_hideList.removeAll("kylin-vpn");
        m_config->setValue("trayIconsInhibited", m_hideList);
        m_config->setValue("version", "2.2");
    }

    //2.3版本号校验,合并按钮中将电源换为蓝牙
    if (QVersionNumber::fromString(version).minorVersion() < 3) {
        m_controlPanelList = DEFAULT_CONTROL_PANEL_ITEMS;
        m_orderList.removeAll("ukui-bluetooth");
        if (!m_orderList.contains("ukui-power-manager-tray")) {
            m_orderList.prepend("ukui-power-manager-tray");
        }
        m_hideList.removeAll("ukui-bluetooth");

        m_config->setValue("controlPanelItems", m_controlPanelList);
        m_config->setValue("orderedItems", m_orderList);
        m_config->setValue("trayIconsInhibited", m_hideList);
        m_config->setValue("version", "2.3");
    }

    //2.4版本号校验,合并按钮调整顺序为网络，蓝牙，声音
    if (QVersionNumber::fromString(version).minorVersion() < 4) {
        if (m_controlPanelList != DEFAULT_CONTROL_PANEL_ITEMS) {
            m_controlPanelList = DEFAULT_CONTROL_PANEL_ITEMS;
            m_config->setValue("controlPanelItems", m_controlPanelList);
        }
        m_config->setValue("version", "2.4");
    }

    //固定的items放在order items的前面
    m_orderList = m_fixList + m_orderList;

    m_sniHost = UkuiSni::StatusNotifierHost::self();
    m_sniHost->registerHost();

    m_applicationInfo = new UkuiSearch::ApplicationInfo;

    connect(m_config, &UkuiQuick::Config::configChanged, this, [=] {
        QStringList hidelist = m_config->getValue(QStringLiteral("trayIconsInhibited")).toStringList();
        for (QString &itemId : hidelist) {
            if (!m_hideList.contains(itemId)) { //处理新增的隐藏
                m_hideList.append(itemId);
                QStringList sourceList = m_itemInfo.keys(itemId);
                for (QString &itemSource : sourceList) {
                    removeItem(itemSource);
                }
            }
        }

        QStringList list = m_hideList;
        for (QString &itemId : list) {
            if (!hidelist.contains(itemId)) { //处理新增的显示
                m_hideList.removeOne(itemId);
                QStringList sourceList = m_itemInfo.keys(itemId);
                for (QString &itemSource : sourceList) {
                    addSource(itemSource);
                }
            }
        }
    });
    connect(m_sniHost, &UkuiSni::StatusNotifierHost::itemAdded, this, &TrayItemsModel::addSource);
    connect(m_sniHost, &UkuiSni::StatusNotifierHost::itemRemoved, this, &TrayItemsModel::removeSource);
    connect(SecurityFunctionControl::instance(),
            &SecurityFunctionControl::controlEnabledChanged, this, &TrayItemsModel::controlStateChanged);

    for (auto &service : m_sniHost->services()) {
        addSource(service);
    }

    m_config->forceSync();
}

int TrayItemsModel::rowCount(const QModelIndex &parent) const
{
    return m_item.length();
}

QVariant TrayItemsModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if ((i < 0) || (i >= m_item.length())) {
        return QVariant();
    }
    TrayItem* item = m_item[i];
    switch (static_cast<Role>(role)) {
        case Role::Service:
            return item->service();
        case Role::AttentionIcon:
            return item->attentionIcon();
        case Role::AttentionIconName:
            return item->attentionIconName();
        case Role::AttentionMovieName:
            return item->attentionMovieName();
        case Role::Category:
            return item->category();
        case Role::Icon:
            return item->icon();
        case Role::IconName:
            return item->iconName();
        case Role::IconThemePath:
            return item->iconThemePath();
        case Role::Id:
            return item->id();
        case Role::ItemIsMenu:
            return item->itemIsMenu();
        case Role::OverlayIcon:
            return item->overlayIcon();
        case Role::OverlayIconName:
            return item->overlayIconName();
        case Role::Status:
            return item->status();
        case Role::Title:
            return item->title();
        case Role::ToolTipSubTitle:
            return item->toolTipSubTitle();
        case Role::ToolTipTitle:
            return item->toolTipTitle();
        case Role::WindowId:
            return item->windowId();
        case Role::Fixed:
            return item->fixed();
        case Role::RecordOrder:
            return item->recordOrder();
        case Role::Row:
            return index.row();
        case Role::Desktop:
            return item->desktop();
        default:
            return {};
    }
}

QHash<int, QByteArray> TrayItemsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

void TrayItemsModel::activate(const QModelIndex &index)
{
    if (!index.isValid()) return;
    m_item.at(index.row())->activate(QCursor::pos().x(), QCursor::pos().y());
}

void TrayItemsModel::showContextMenu(const QModelIndex &index, QQuickItem *item)
{
    if (!index.isValid()) return;
    m_item.at(index.row())->setParentItem(item);
    m_item.at(index.row())->contextMenu(QCursor::pos().x(), QCursor::pos().y());
}

QVariant TrayItemsModel::extractIcon(const QIcon &icon, const QVariant &defaultValue)
{
    if (!icon.isNull()) {
        return icon;
    } else {
        return defaultValue;
    }
}

QString TrayItemsModel::getDesktop(const uint &pid)
{
    QString desktop;
    m_applicationInfo->tranPidToDesktopFp(pid, desktop);
    return desktop;
}

int TrayItemsModel::indexOfSource(const QString &source) const
{
    for (int i = 0; i < rowCount(QModelIndex()); i++) {
        if (m_item[i]->source() == source) {
            return i;
        }
    }
    return -1;
}

void TrayItemsModel::removeItem(const QString &source)
{
    int i = indexOfSource(source);
    if (i >= 0) {
        auto item = m_item.at(i);

        beginRemoveRows({}, i, i);
        m_item.removeAt(i);
        endRemoveRows();

        if (item) {
            if (item->id() == DEFAULT_CONTROL_PANEL_ID) {;
                ControlPanelModel::self()->setEnable(false);
            }
            item->disconnect();
            item->deleteLater();
        }
    }
    Q_EMIT dataChanged(index(i, 0, {}), index(m_item.size() - 1, 0, {}), {Role::Row});
}

void TrayItemsModel::setOrderInGroup(ItemGroupModel *group, const QModelIndex &groupIndex, int order)
{
    int end = group->groupEnd();
    int newOrder = group->groupBegin() + order;
    if(newOrder > end) {
        newOrder = end;
    }
    setOrder(group->mapToSource(groupIndex), newOrder);
}

void TrayItemsModel::setOrderBetweenGroups(ItemGroupModel *fromGroup, const QModelIndex &beginIndex, ItemGroupModel *toGroup, int order)
{
    int newOrder = toGroup->groupBegin() + order;
    if(newOrder > toGroup->groupEnd()) {
        newOrder = toGroup->groupEnd() + 1;
    }
    setOrder(fromGroup->mapToSource(beginIndex), newOrder);
}

void TrayItemsModel::setSeparateIndex(int index)
{
    m_separateIndex = index;
    if (m_separateIndex > m_item.length() - 1) {
        m_separateIndex = m_item.length() - 1;
    }
    Q_EMIT separateIndexChanged();
    m_config->setValue(QStringLiteral("separateIndex"), index);
    m_config->forceSync();
}

void TrayItemsModel::setOrder(const QModelIndex &index, int order)
{
    int oldRow = index.row();
    if ((oldRow < 0) || (oldRow >= m_item.length())) {
        return;
    }
    auto item = m_item.at(oldRow);
    if(item->fixed()) {
        return;
    }
    if(oldRow == order) {
        return;
    }

    QString id  = m_item.at(index.row())->id();
    if(order > oldRow) {
        if(item->recordOrder()) {
            QString idBefore;
            for(int row = order; row > oldRow; --row) {
                if(m_item.at(row)->recordOrder()) {
                    idBefore = m_item.at(row)->id();
                    break;
                }
            }
            if(!idBefore.isEmpty()) {
                int targetOrder = m_orderList.indexOf(idBefore);
                m_orderList.insert(targetOrder, m_orderList.takeAt(m_orderList.indexOf(id)));
                m_config->setValue(QStringLiteral("orderedItems"), QStringList(m_orderList.mid(m_fixList.size())));
            }
        }
        beginMoveRows(index.parent(), oldRow, oldRow, index.parent(), order + 1);
        m_item.insert(order, m_item.takeAt(oldRow));
        endMoveRows();
        Q_EMIT dataChanged(this->index(oldRow, 0), this->index(order, 0), {Role::Row});
    } else {
        if(item->recordOrder()) {
            QString idAfter;
            for(int row = order; row < oldRow; ++row) {
                if(m_item.at(row)->recordOrder()) {
                    idAfter = m_item.at(row)->id();
                    break;
                }
            }
            if(!idAfter.isEmpty()) {
                int targetOrder = m_orderList.indexOf(idAfter);
                m_orderList.insert(targetOrder, m_orderList.takeAt(m_orderList.indexOf(id)));
                m_config->setValue(QStringLiteral("orderedItems"), QStringList(m_orderList.mid(m_fixList.size())));
            }
        }
        beginMoveRows(index.parent(), oldRow, oldRow, index.parent(), order);
        m_item.insert(order, m_item.takeAt(oldRow));
        endMoveRows();
        Q_EMIT dataChanged(this->index(order, 0), this->index(oldRow, 0), {Role::Row});
    }

    m_config->forceSync();
}

int TrayItemsModel::getSeparateIndex()
{
    return m_separateIndex;
}

QStringList TrayItemsModel::getControlPanelItems()
{
    return m_controlPanelList;
}

void TrayItemsModel::addSource(const QString &source)
{
    for (TrayItem* &trayItem : m_item) {
        if (trayItem->source() == source) {
            return;
        }
    }

    auto item = new TrayItem(source);
    if (SecurityFunctionControl::instance()->controlEnabled()) {
        item->setDesktop(getDesktop(item->pid()));
    }

    connect(item, &TrayItem::itemReady, this, &TrayItemsModel::insertItem);
    if (item->id() != "") {
        insertItem(item);
    }
}

void TrayItemsModel::removeSource(const QString &source)
{
    QString id = m_itemInfo.value(source);
    if (DEFAULT_CAN_HIDE_ITEMS.contains(id)) {
        m_canHideList.removeAll(id);
        m_config->setValue("trayIconsCanInhibited", m_canHideList);
    }
    m_itemInfo.remove(source);
    removeItem(source);
}

void TrayItemsModel::insertItem(TrayItem* item)
{
    if (m_item.contains(item)) return;
    m_itemInfo.insert(item->source(), item->id());
    if (DEFAULT_CAN_HIDE_ITEMS.contains(item->id())) {
        if (!m_canHideList.contains(item->id())) {
            m_canHideList.append(item->id());
            m_config->setValue("trayIconsCanInhibited", m_canHideList);
        }
    }
    if (m_hideList.contains(item->id())) {
        removeItem(item->source());
        return;
    }

    int order = m_item.size();
    for(const QString &id : m_orderList) {
        if(id == item->id()) {
            int index = m_orderList.indexOf(id);

            if(index <= m_fixList.size() - 1) {
                item->setFixed(true);
            } else {
                item->setRecordOrder(true);
            }
            //index为0表示第一个
            if(index == 0) {
                order = 0;
            } else if(index > 0){
                bool find = false;
                //寻找前一个item所在的位置，前一个item未注册时寻找再前一个的位置
                for(int preIndex = index - 1; preIndex >= 0 && !find; preIndex--) {
                    QString preId = m_orderList.at(preIndex);
                    for(int i = 0; i < m_item.size(); i++) {
                        if(m_item.at(i)->id() == preId) {
                            order = i + 1;
                            find = true;
                            break;
                        }
                    }
                }
                //找不到放到第一个
                if(!find) {
                    order = 0;
                }
            }
            break;
        }
    }
    beginInsertRows(QModelIndex(), order, order);
    m_item.insert(order, item);
    endInsertRows();
    Q_EMIT dataChanged(index(order, 0, {}), index(m_item.size() - 1, 0, {}), {Role::Row});

    connect(item, &TrayItem::itemDataChanged, this, &TrayItemsModel::dataUpdated);
    connect(item, &TrayItem::menuStateChanged, this, &TrayItemsModel::menuStateChanged);

    if (item->id() == DEFAULT_CONTROL_PANEL_ID) {
        ControlPanelModel::self()->setEnable(true);
    }
}

void TrayItemsModel::dataUpdated(const QString &sourceName)
{
    for (TrayItem* &trayItem : m_item) {
        if (trayItem->source() == sourceName) {
            int i = indexOfSource(sourceName);

            if (i >= 0) {
                Q_EMIT dataChanged(index(i, 0, {}), index(i, 0, {}));
            }
            return;
        }
    }
}

void TrayItemsModel::controlStateChanged(bool controlState)
{
    if (controlState) {
        for (int i = 0; i < rowCount(QModelIndex()); i++) {
            if (m_item[i]->desktop() != "") continue;
            m_item[i]->setDesktop(getDesktop(m_item[i]->pid()));
            Q_EMIT dataChanged(index(i, 0, {}), index(i, 0, {}), {Role::Desktop});
        }
    }
}
