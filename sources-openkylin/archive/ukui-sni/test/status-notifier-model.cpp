#include "status-notifier-model.h"
#include <QDebug>

statusNotifierModel::statusNotifierModel(QObject *parent) : QAbstractListModel(parent)
{
    m_sniHost = UkuiSni::StatusNotifierHost::self();

    connect(m_sniHost, &UkuiSni::StatusNotifierHost::itemAdded, this, &statusNotifierModel::addSource);
    connect(m_sniHost, &UkuiSni::StatusNotifierHost::itemRemoved, this, &statusNotifierModel::removeSource);

    for (auto service : m_sniHost->services()) {
        addSource(service);
    }
}

int statusNotifierModel::rowCount(const QModelIndex &parent) const
{
    return m_itemSource.length();
}

QVariant statusNotifierModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if ((i < 0) || (i >= m_itemSource.length())) {
        return QVariant();
    }

    UkuiSni::StatusNotifierItem *item = m_sniHost->itemForService(m_itemSource[i]);

    switch (static_cast<Role>(role)) {
    case Role::DataEngineSource:
        return m_itemSource[i];
    case Role::Service:
        return item->service();
    case Role::AttentionIcon:
        return extractIcon(item->attentionIcon(), item->icon());
    case Role::AttentionIconName:
        return ("image://itemIcon/" + item->attentionIconName());
    case Role::AttentionMovieName:
        return item->attentionMovieName();
    case Role::Category:
        return item->category();
    case Role::Icon:
        return extractIcon(item->icon());
    case Role::IconName:
        return  ("image://itemIcon/" + item->iconName());
    case Role::IconThemePath:
        return "file://" + item->iconThemePath() + "/" +item->iconName();
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
    default:
        return QVariant();
    }

}

QHash<int, QByteArray> statusNotifierModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles.insert(static_cast<int>(Role::DataEngineSource), QByteArrayLiteral("DataEngineSource"));
    roles.insert(static_cast<int>(Role::Service), QByteArrayLiteral("Service"));
    roles.insert(static_cast<int>(Role::AttentionIcon), QByteArrayLiteral("AttentionIcon"));
    roles.insert(static_cast<int>(Role::AttentionIconName), QByteArrayLiteral("AttentionIconName"));
    roles.insert(static_cast<int>(Role::AttentionMovieName), QByteArrayLiteral("AttentionMovieName"));
    roles.insert(static_cast<int>(Role::Category), QByteArrayLiteral("Category"));
    roles.insert(static_cast<int>(Role::Icon), QByteArrayLiteral("Icon"));
    roles.insert(static_cast<int>(Role::IconName), QByteArrayLiteral("IconName"));
    roles.insert(static_cast<int>(Role::IconThemePath), QByteArrayLiteral("IconThemePath"));
    roles.insert(static_cast<int>(Role::Id), QByteArrayLiteral("Id"));
    roles.insert(static_cast<int>(Role::ItemIsMenu), QByteArrayLiteral("ItemIsMenu"));
    roles.insert(static_cast<int>(Role::OverlayIcon), QByteArrayLiteral("OverlayIcon"));
    roles.insert(static_cast<int>(Role::OverlayIconName), QByteArrayLiteral("OverlayIconName"));
    roles.insert(static_cast<int>(Role::Status), QByteArrayLiteral("Status"));
    roles.insert(static_cast<int>(Role::Title), QByteArrayLiteral("Title"));
    roles.insert(static_cast<int>(Role::ToolTipSubTitle), QByteArrayLiteral("ToolTipSubTitle"));
    roles.insert(static_cast<int>(Role::ToolTipTitle), QByteArrayLiteral("ToolTipTitle"));
    roles.insert(static_cast<int>(Role::WindowId), QByteArrayLiteral("WindowId"));

    return roles;
}

QVariant statusNotifierModel::extractIcon(const QIcon &icon, const QVariant &defaultValue)
{
    if (!icon.isNull()) {
        return icon;
    } else {
        return defaultValue;
    }
}

int statusNotifierModel::indexOfSource(const QString &source) const
{
    for (int i = 0; i < rowCount(); i++) {
        if (m_itemSource[i] == source) {
            return i;
        }
    }
    return -1;
}

void statusNotifierModel::onActivate(int i)
{
    m_sniHost->itemForService(m_itemSource[i])->activate(0,0);
}

void statusNotifierModel::onContextMenu(int i)
{
    m_sniHost->itemForService(m_itemSource[i])->contextMenu(0,0);
}

void statusNotifierModel::addSource(const QString &source)
{
    int i = rowCount(QModelIndex());
    beginInsertRows(QModelIndex(), i, i);

    connect(m_sniHost->itemForService(source), &UkuiSni::StatusNotifierItem::dataUpdated, this, [=, this]() {
        dataUpdated(source);
    });
    connect(m_sniHost->itemForService(source), &UkuiSni::StatusNotifierItem::contextMenuReady, this, &statusNotifierModel::showMenu);

    m_itemSource.append(source);
    endInsertRows();
}

void statusNotifierModel::removeSource(const QString &source)
{
    int i = indexOfSource(source);
    if (i >= 0) {
        beginRemoveRows(QModelIndex(), i, i);
        m_itemSource.removeAt(i);
        endRemoveRows();
    }
}

void statusNotifierModel::dataUpdated(const QString &sourceName)
{
    int i = indexOfSource(sourceName);

    if (i >= 0) {
        Q_EMIT dataChanged(index(i, 0), index(i, 0));
    }
}

void statusNotifierModel::showMenu(QMenu *menu)
{
    menu->exec(QCursor::pos());
}
