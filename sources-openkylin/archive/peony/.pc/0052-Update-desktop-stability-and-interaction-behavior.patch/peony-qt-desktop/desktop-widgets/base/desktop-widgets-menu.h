#ifndef DESKTOP_WIDGETS_MENU_H
#define DESKTOP_WIDGETS_MENU_H
#include <QMenu>
#include <QActionGroup>
#include <QAction>

#include <QObject>

class QTimer;
class SpacesSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Qt::SortOrder sortOrder READ getSortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(int sortType READ getSortType WRITE setSortType NOTIFY sortTypeChanged)
    Q_PROPERTY(bool isSpaceListMode READ getIsSpaceListMode WRITE setIsSpaceListMode NOTIFY isSpaceListModeChanged)
public:
    static SpacesSettings *globalInstance();
    explicit SpacesSettings(QObject *parent = nullptr);

    Q_INVOKABLE Qt::SortOrder getSortOrder() const;
    Q_INVOKABLE void setSortOrder(Qt::SortOrder newSortOrder);
    Q_INVOKABLE int getSortType() const;
    Q_INVOKABLE void setSortType(int newSortType);
    Q_INVOKABLE bool getUseGlobalSorting() const;
    Q_INVOKABLE void setUseGlobalSorting(bool newUseGlobalSorting);


    void setSpacesSortConfig(int spaceId, int sortType, int sortOrder);
    QVariantHash getSpacesSortConfig(int spaceId);

    void setViewModel(int instanceId,bool islistModel);
    bool getIsPartitionListMode(int instanceId) const;
    bool getIsExpand(int instanceId);
    void setIsExpand(int instanceId,bool expand);
    bool getIsSpaceListMode() const;
    void setIsSpaceListMode(bool newIsSpaceListMode);
    int  getInitHeight(int instanceId);
    void setInitHeight(int instanceId,int height);
Q_SIGNALS:

    void sortOrderChanged();
    void sortTypeChanged();
    void sortConfigChanged();
    void isSpacesCollapsedChanged();
    void isSpaceListModeChanged();
    void useGlobalSortingChanged();
private:
    Qt::SortOrder sortOrder = Qt::DescendingOrder;
    int sortType = 0;
    bool useGlobalSorting = false;

    bool isSpacesCollapsed = false;

    bool isSpaceListMode = false;

    QTimer *delayer = nullptr;
};



class WidgetMenu : public QMenu {
    Q_OBJECT
public:
    explicit WidgetMenu(int instanceid,QWidget *parent = nullptr)
        : QMenu(tr("Widget Menu"), parent) {
        m_instanceId = instanceid;
        setupActions();
        connectSignals();
        updateMenuStates();
    }

Q_SIGNALS:
    void sortRequest(int type, int order, bool userAdded);

private:
    void setupActions();


    void connectSignals();
    void handleSortTypeChange(int type);
    void handleSortOrderChange(Qt::SortOrder order);

    // void handleGlobalSettingsChange(bool enabled);

    void updateMenuStates();

private:
    // 菜单项
    QAction *m_renamePartition;
    QAction *m_listView;
    QAction *m_iconView;
    QAction *m_actFileName;
    QAction *m_actDate;
    QAction *m_actFileType;
    QAction *m_actFileSize;
    QAction *m_sortAsc;
    QAction *m_sortDes;
    QAction *m_actdisbandPartition;
    // Action 分组
    QActionGroup *m_sortTypeGroup;
    QActionGroup *m_sortOrderGroup;
    QActionGroup *m_viewtypeGroup;

    // 状态变量
    Qt::SortOrder m_currentSortOrder;
    int m_instanceId;
    int m_SortType;
    int m_SortOrder;
    bool m_useGlobalSorting;
    bool m_islistmodel = false;
};
#endif // DESKTOP_WIDGETS_MENU_H
