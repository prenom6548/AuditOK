#ifndef ITEMGROUPMODEL_H
#define ITEMGROUPMODEL_H

#include <QSortFilterProxyModel>
#include <QPersistentModelIndex>
#include <QHash>
#include <QObject>
#include <QStringList>
#include <QQuickItem>

#include "tray-item.h"

class ItemsFilterModel;

class ItemGroupModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(GroupType modelType READ getModelType WRITE setModelType)
    Q_PROPERTY(int currentSeparateIndex READ currentSeparateIndex WRITE setCurrentSeparateIndex NOTIFY currentSeparateIndexChanged)
    Q_PROPERTY(int showCount READ showCount WRITE setShowCount NOTIFY showCountChanged)
public:
    enum GroupType {Show, Fold};
    Q_ENUM(GroupType)

    explicit ItemGroupModel(QObject *parent = nullptr);

    void setModelType(GroupType type);
    GroupType getModelType();

    int currentSeparateIndex();
    void setCurrentSeparateIndex(int index);

    int showCount();
    void setShowCount(int index);

    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void onActivate(const QModelIndex &index);
    Q_INVOKABLE void onContextMenu(const QModelIndex &index, QQuickItem *item);

    Q_INVOKABLE QVariant foldIcon();

    Q_INVOKABLE int groupBegin() const;
    Q_INVOKABLE int groupEnd() const;

Q_SIGNALS:
    void currentSeparateIndexChanged();
    void showCountChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void onShowCountChanged();
    void onSeparateIndexChanged();

    QHash<QPersistentModelIndex, int> m_order;
    ItemsFilterModel *m_sourceModel = nullptr;
    int m_groupBegin = 0;
    int m_groupEnd = 0;
    GroupType m_groupType = GroupType::Show;

    int m_currentSeparateIndex = 0;
    int m_showCount = 0;
};

#endif // ITEMGROUPMODEL_H
