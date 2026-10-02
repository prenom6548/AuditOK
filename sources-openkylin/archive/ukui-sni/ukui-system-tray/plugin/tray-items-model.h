#ifndef TRAYITEMSMODEL_H
#define TRAYITEMSMODEL_H

#include <QAbstractListModel>
#include <QQuickItem>
#include <QIcon>
#include <QMenu>
#include <QSettings>

#include <config.h>

#include "status-notifier-host.h"
#include "status-notifier-item.h"
#include "tray-item.h"
#include "application-info.h"

class ItemGroupModel;

class TrayItemsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        Service,
        AttentionIcon,
        AttentionIconName,
        AttentionMovieName,
        Category,
        Icon,
        IconName,
        IconThemePath,
        Id,
        ItemIsMenu,
        OverlayIcon,
        OverlayIconName,
        Status,
        Title,
        ToolTipSubTitle,
        ToolTipTitle,
        WindowId,
        Fixed,
        RecordOrder,
        Row,
        Desktop
    };
    Q_ENUM(Role)

    static TrayItemsModel *instance();
    ~TrayItemsModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void activate(const QModelIndex &index);
    void showContextMenu(const QModelIndex &index, QQuickItem *item);

    void setOrderInGroup(ItemGroupModel *group, const QModelIndex &groupIndex, int order);
    void setOrderBetweenGroups(ItemGroupModel *fromGroup, const QModelIndex &beginIndex, ItemGroupModel *toGroup, int order);
    void setSeparateIndex(int index);
    int getSeparateIndex();
    QStringList getControlPanelItems();

    void setOrder(const QModelIndex &index, int order);

Q_SIGNALS:
    void separateIndexChanged();
    void menuStateChanged(bool state);

private Q_SLOTS:
    void addSource(const QString &source);
    void removeSource(const QString &source);
    void insertItem(TrayItem* item);
    void dataUpdated(const QString &sourceName);
    void controlStateChanged(bool controlState);

private:
    TrayItemsModel(QObject *parent = nullptr);

    UkuiSni::StatusNotifierHost *m_sniHost = nullptr;
    UkuiSearch::ApplicationInfo *m_applicationInfo = nullptr;

    UkuiQuick::Config *m_config {nullptr};
    QVector<TrayItem*> m_item;
    QStringList m_fixList;
    QStringList m_orderList;
    QStringList m_hideList;
    QStringList m_controlPanelList;
    QStringList m_canHideList;
    int m_currentSeparateIndex = 7;
    int m_separateIndex = 7;
    QMap<QString, QString> m_itemInfo;

    static QVariant extractIcon(const QIcon &icon, const QVariant &defaultValue = QVariant());
    QString getDesktop(const uint & pid);
    int indexOfSource(const QString &source) const;
    void removeItem(const QString &source);
};

#endif // TRAYITEMSMODEL_H
