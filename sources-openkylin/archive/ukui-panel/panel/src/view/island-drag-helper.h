//
// Created by iaom on 2024/9/24.
//

#ifndef ISLAND_DRAG_HELPER_H
#define ISLAND_DRAG_HELPER_H
#include <QObject>

class QQuickItem;

class IslandDragHelper : public QObject
{
    //从顶栏即将拖动到任务栏时，为了实时获取这个岛的布局尺寸，直接加载这个岛但是隐藏内容。Island内部用此属性判断是否是隐藏内容
    Q_PROPERTY(bool islandPendingAdd2Panel READ islandPendingAdd2Panel WRITE setIslandPendingAdd2Panel NOTIFY islandPendingAdd2PanelChanged)
    Q_PROPERTY(bool dragging READ dragging WRITE setDragging NOTIFY draggingChanged)
    Q_PROPERTY(QString draggingIsland READ draggingIsland WRITE setDraggingIsland NOTIFY draggingIslandChanged)
    Q_OBJECT
public:

    static IslandDragHelper* self();

    //drag
    //从顶栏拖出
    Q_INVOKABLE void IslandDragRemovingFromTopBar(bool removeWhenDrop);
    //从任务栏拖出
    Q_INVOKABLE void IslandDragRemovingFromPanel(bool removeWhenDrop);

    //drop
    //从顶栏拖动到任务栏
    Q_INVOKABLE void IslandDragRemoveFromTopBar(const QString &island);
    //从任务栏拖动到顶栏
    Q_INVOKABLE void IslandDragRemoveFromPanel(const QString &island);

    bool islandPendingAdd2Panel() const;
    void setIslandPendingAdd2Panel(bool islandPendingAdd2Panel);

    bool dragging() const;
    void setDragging(bool dragging);

    QString draggingIsland();
    void setDraggingIsland(const QString &draggingIsland);

Q_SIGNALS:
    //拖拽换位，松手
    void dragRemoveFromPanel(const QString &island);
    void dragRemoveFromTopBar(const QString &island);
    //拖拽换位，但还未松手
    void dragRemovingFromPanel(const QString &island, bool removeWhenDrop);
    void dragRemovingFromTopBar(const QString &island, bool removeWhenDrop);

    void islandPendingAdd2PanelChanged();
    void draggingChanged();
    void draggingIslandChanged();

private:
    IslandDragHelper(QObject *parent = nullptr);

    bool m_islandPendingAdd2Panel{false};
    bool m_dragging{false};
    QString m_draggingIsland;
};



#endif //ISLAND_DRAG_HELPER_H
