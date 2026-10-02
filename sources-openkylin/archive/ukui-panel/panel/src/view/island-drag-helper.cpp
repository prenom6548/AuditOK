//
// Created by iaom on 2024/9/24.
//

#include "island-drag-helper.h"
#include <mutex>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
static IslandDragHelper *g_islandDragHelper = nullptr;
static std::once_flag flag_islandDragHelper;

IslandDragHelper* IslandDragHelper::self()
{
    std::call_once(flag_islandDragHelper, [ & ] {
        g_islandDragHelper = new IslandDragHelper();
    });
    return g_islandDragHelper;
}

void IslandDragHelper::IslandDragRemovingFromTopBar(bool removeWhenDrop)
{
    setDragging(true);
    setIslandPendingAdd2Panel(removeWhenDrop);
    Q_EMIT dragRemovingFromTopBar(m_draggingIsland, removeWhenDrop);
}

void IslandDragHelper::IslandDragRemovingFromPanel(bool removeWhenDrop)
{
    setDragging(true);
    setIslandPendingAdd2Panel(!removeWhenDrop);
    Q_EMIT dragRemovingFromPanel(m_draggingIsland, removeWhenDrop);
}

void IslandDragHelper::IslandDragRemoveFromTopBar(const QString& island)
{
    if(island != m_draggingIsland) {
        return;
    }
    setDraggingIsland("");
    setDragging(false);
    Q_EMIT dragRemoveFromTopBar(island);
}

void IslandDragHelper::IslandDragRemoveFromPanel(const QString& island)
{
    if(island != m_draggingIsland) {
        return;
    }
    setDraggingIsland("");
    setDragging(false);
    Q_EMIT dragRemoveFromPanel(island);
}

bool IslandDragHelper::islandPendingAdd2Panel() const
{
    return m_islandPendingAdd2Panel;
}

void IslandDragHelper::setIslandPendingAdd2Panel(bool islandPendingAdd2Panel)
{
    if(m_islandPendingAdd2Panel != islandPendingAdd2Panel) {
        m_islandPendingAdd2Panel = islandPendingAdd2Panel;
        Q_EMIT islandPendingAdd2PanelChanged();
    }
}

bool IslandDragHelper::dragging() const
{
    return m_dragging;
}

void IslandDragHelper::setDragging(bool dragging)
{
    if(m_dragging != dragging) {
        m_dragging = dragging;
        Q_EMIT draggingChanged();
    }
}

QString IslandDragHelper::draggingIsland()
{
    return m_draggingIsland;
}

void IslandDragHelper::setDraggingIsland(const QString& draggingIsland)
{
    if(m_draggingIsland != draggingIsland) {
        m_draggingIsland = draggingIsland;
        Q_EMIT draggingIslandChanged();
    }
}

IslandDragHelper::IslandDragHelper(QObject* parent) : QObject(parent)
{
}
