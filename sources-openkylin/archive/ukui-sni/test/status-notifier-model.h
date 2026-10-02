#ifndef STATUSNOTIFIERMODEL_H
#define STATUSNOTIFIERMODEL_H

#include <QAbstractListModel>
#include <QIcon>
#include <QMenu>

#include "status-notifier-host.h"
#include "status-notifier-item.h"

class statusNotifierModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        DataEngineSource = 0,
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
    };

    explicit statusNotifierModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void onActivate(int i);
    Q_INVOKABLE void onContextMenu(int i);

private Q_SLOTS:
    void addSource(const QString &source);
    void removeSource(const QString &source);
    void dataUpdated(const QString &sourceName);
    void showMenu(QMenu *menu);

private:
    UkuiSni::StatusNotifierHost *m_sniHost = nullptr;
    QVector<QString> m_itemSource;

    static QVariant extractIcon(const QIcon &icon, const QVariant &defaultValue = QVariant());
    int indexOfSource(const QString &source) const;
};

#endif // STATUSNOTIFIERMODEL_H
