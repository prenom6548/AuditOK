#ifndef PLATFORMWINDOW_H
#define PLATFORMWINDOW_H

#include <island-view.h>
#include <ukui-quick/framework/widget-item.h>
#include <widget-item.h>
#include "peony-dbus-service.h"
#include <QHash>
#include <QVariantMap>

class QTimer;
class UkuiWindowHelper;
class WidgetsProxyModel;

namespace Peony
{
class  DesktopItemProxyModel;
class AdvancedDesktopItemModel;
}

class PlatformWindow : public UkuiQuick::IslandView
{

    Q_OBJECT
public:
    static QList<PlatformWindow *> allWindows();

    explicit PlatformWindow(int id, QScreen* screen, QWindow *parent = nullptr);
    ~PlatformWindow();
    void rowsInserted(const QModelIndex &parent, int first, int last);
    bool isFull();
    void ajustIconsOnScreenSize();

    bool isInvalidPoint(const QPoint &point);
    QPoint invalidPoint() const;
    void relayoutExsitingItems(const QStringList &uris);
    /*!
     * \brief relayoutExsitingItemsForDesktopWidgets
     * \param uris
     * 处理插件遮罩导致的图标重排
     */
    void relayoutExsitingItemsForDesktopWidgets(const QStringList &uris);
    void setFileMetaInfoPos(const QString &uri, const QPoint &pos);
    const QStringList getAllFileUris();
    /*!
     * \brief findNextEmptyGridPos
     * 获取下一个空网格所在的位置
     * \return
     *
     */

    Q_INVOKABLE QPoint findNextEmptyGridPos(const QPoint &startGridPos = QPoint());
    /*!
     * \brief getGridPosFromMousePos
     * 根据当前鼠标事件传入的位置返回网格的位置
     * \param mousePos
     * \return
     * 鼠标位置对应的网格位置
     */
    Q_INVOKABLE QPoint getGridPosFromMousePos(const QPoint &mousePos);

    bool checkIfDesktopWidgetsIntersected(const QPoint &gridPos, bool considerIconItem = false);

    /*!
     * \brief gridPosLesserThan
     * 从左上到右下比较两个网格位置的顺序
     * \param leftPos
     * \param rightPos
     * \return
     */
    bool gridPosLesserThan(const QPoint &leftPos, const QPoint &rightPos);

    void checkItemsOver();
    QPoint checkGridPos(const QPoint pos);
    void setSortType(int sortType);
    void setSortOrder(int sortOrder);
    void setId(int id);
    int id();
    int dataCount() const;
    bool checkEmptyPositon();
    QScreen *bindScreen() {
        return m_screen;
    }
    QScreen *destoryScreen() {
        return m_screen = nullptr;
    }
    void initDesktopOrgWidgets();
    UkuiQuick::WidgetItem *currentDragMoveItem() const;
    Q_INVOKABLE void setCurrentDragMoveItem(UkuiQuick::WidgetItem *newCurrentDragMoveItem);

    int currentWidgetsLayoutWidth() const;
    void setCurrentWidgetsLayoutWidth(int newCurrentWidgetsLayoutWidth);

    int currentWidgetsLayoutHeight() const;
    void setCurrentWidgetsLayoutHeight(int newCurrentWidgetsLayoutHeight);

    /*!
     * \brief getCurrentWidgetUsedRects
     * 获取屏幕上所有可见插件当前位置
     * \return
     */
    QHash<UkuiQuick::WidgetItem *, QRect> getCurrentWidgetUsedRects();

    /*!
     * \brief getWidgetExceptedRects
     * 获取屏幕上所有可见插件期望位置
     * \return
     */
    QHash<UkuiQuick::WidgetItem *, QRect> getCurrentWidgetExceptedRects();

    /*!
     * \brief relocateWidgetPosition
     * 对桌面插件进行重排，在屏幕尺寸或缩放改变时调用
     */
    void relocateWidgetPosition();
    //检查分区工具配置是否存在 新屏幕场景
    bool checkIsOrgconfigExist();
    void mergeSameTypeWidgetToPrimaryScreen(int screenid,int orgtype,int instanceid);
    bool isPrimary() const;

Q_SIGNALS:
    void viewUpdated();
    void isPrimaryChanged();

    void currentWidgetsLayoutWidthChanged();
    void currentWidgetsLayoutHeightChanged();
public Q_SLOTS:
    void updateWindowGeometry();
    void reloadData();
    void loadData();
    void invalidate();
    void updateView();
    /**
     * @brief layoutItems
     * 自动排列
     * @param closest
     * @return
     * 返回自动排列的uri列表
     */
    QStringList layoutItems(bool closest = false);

    /**
     * @brief relayoutExsitingItemsAndUpdate
     * 设置自动排列后，
     * @param uris
     */
    void relayoutExsitingItemsAndUpdate(const QStringList uris, bool forceUpdate = true);
    void showHiddenFile();

    void showStandardFile();

    bool fileRemoved(const QString &uri);
    bool filesRemoved(const QStringList &uris);
    bool idChanged(int id, int oldId, const QStringList &uris);

    void drop(const QStringList &selectedUris, Qt::DropAction action, int insertRow, const QPoint &pos);
    bool dragFromOtherScreen(QStringList selectedUris, int insertRow, const QPoint &offPos);

    Q_INVOKABLE void layoutAddedWidgetItem(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem, WidgetsProxyModel *proxyModel, bool isMerging = false);
    Q_INVOKABLE void updateDesktopWidgetsUsedRegion();

    Q_INVOKABLE void updateDesktopWidgetsUsedRegionInstantly(UkuiQuick::WidgetItem *excludedItem = nullptr, bool updateRegionOnly = false, WidgetsProxyModel *model = nullptr);
    Q_INVOKABLE void updateDragResizeUsedRegion(UkuiQuick::WidgetItem *excludedItem = nullptr, WidgetsProxyModel *model = nullptr);
    Q_INVOKABLE bool checkItemIntersected(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem = nullptr);

    Q_INVOKABLE bool adjustIntersectedItem(QQuickItem *dragItem, QQuickItem *item, int parentWidth, bool layoutGrid);

    Q_INVOKABLE void adjustGridLayoutWidget(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem, bool sizeChanged = false, QQuickItem *dragIndicatorItem = nullptr, WidgetsProxyModel *model = nullptr, int containerWidth = 1900);

    /// 小插件管理器：按 ManagerVariants 变体添加实例（含 sizeHint / desktopSize 等）
    Q_INVOKABLE bool addWidgetFromVariant(const QString &widgetId, const QVariantMap &variant = QVariantMap(),
                                          bool useDropPosition = false, int dropX = 0, int dropY = 0);
    /// 解析管理器拖放的 JSON（UTF-8），供 QML DropArea 调用
    Q_INVOKABLE bool addWidgetFromDropJson(const QString &jsonUtf8, bool useDropPosition, int dropX, int dropY);

    Q_INVOKABLE void updateLastRect(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem);

    Q_INVOKABLE void clearWidgetExceptedPos(UkuiQuick::WidgetItem *item);

    Q_INVOKABLE void updateWidgetSizetoForIntersected(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem);

    Q_INVOKABLE void recoredCurrentPositionAndSize(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem);

    Q_INVOKABLE bool isPlatformWindowHasRootItem();

    Q_INVOKABLE void removeWidgetInstance(int instanceId);

    /**
     * @brief canResizeWidgetTo
     * 检查将指定插件调整到新尺寸时，当前位置是否有足够空间（不与其他插件重叠）。
     * 镜像布局下变宽向左扩展，以当前右边缘为锚检测左侧可用空间。
     * @param item       插件的容器 QQuickItem（WidgetsLayout 里的 desktopWidget）
     * @param widgetItem 插件的 WidgetItem（root）；用于从占用区域中排除自身
     * @param newWidth   目标像素宽度
     * @param newHeight  目标像素高度
     * @return 当前位置可容纳新尺寸时返回 true，否则返回 false
     */
    Q_INVOKABLE bool canResizeWidgetTo(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem,
                                       int newWidth, int newHeight);

    /**
     * @brief constrainContextMenuPosition
     * 将插件请求的右键菜单位置约束到当前桌面窗口可见区域内，避免菜单被屏幕边缘裁切。
     * @param item       发起请求的插件项（用于局部坐标到桌面坐标映射）
     * @param localX     插件局部坐标系中的 x
     * @param localY     插件局部坐标系中的 y
     * @param menuWidth  菜单像素宽度
     * @param menuHeight 菜单像素高度
     * @param edgeMargin 菜单与边缘预留间距（像素）
     * @return 包含 x/y 的 QVariantMap（桌面 rootItem 坐标系）
     */
    Q_INVOKABLE QVariantMap constrainContextMenuPosition(QQuickItem *item, qreal localX, qreal localY,
                                                         qreal menuWidth, qreal menuHeight,
                                                         qreal edgeMargin = 6.0) const;

    /**
     * @brief getScreenAvailableGeometry
     * 返回 item 所在屏幕的可用区域（已去掉任务栏占据的区域）。
     * 供插件 QML 在定位右键菜单时使用，确保菜单不被任务栏遮挡。
     * @param item  插件的 WidgetItem（root）；为 nullptr 时回退到窗口所在屏幕或主屏。
     * @return 包含 x/y/width/height 的 QVariantMap，坐标为虚拟桌面全局坐标系。
     */
    Q_INVOKABLE QVariantMap getScreenAvailableGeometry(QQuickItem *item) const;

    /**
     * @brief clampWidgetPositionToScreen
     * 在插件切换到新尺寸后，将插件的容器（desktopWidget）位置钳夹到屏幕范围内，
     * 确保插件不会超出 rootItem 边界。同时持久化新位置。
     * @param item        插件的 WidgetItem（root），用于定位其父容器
     * @param newWidth    切换后的目标像素宽度
     * @param newHeight   切换后的目标像素高度
     */
    Q_INVOKABLE void clampWidgetPositionToScreen(QQuickItem *item, int newWidth, int newHeight);

    /**
     * @brief showWidgetManager
     * 显示桌面小插件管理界面。若已处于可见状态则不做任何操作。
     * 供插件 QML 通过 desktopShell.showWidgetManager() 调用。
     */
    Q_INVOKABLE void showWidgetManager();
    /*!
     * \brief applyWidgetItemConfigPosition
     * 设置插件 config 坐标并执行与拖拽相同的碰撞检测/避让（含网格吸附）。
     * \return 避让后的 config 坐标 (x, y)
     */
    Q_INVOKABLE QPoint applyWidgetItemConfigPosition(QQuickItem *delegate,
                                                     UkuiQuick::WidgetItem *widgetItem,
                                                     WidgetsProxyModel *proxyModel,
                                                     int configX,
                                                     int configY);

    /*!
     * \brief widgetItemScreenPosition
     * 分区/插件在桌面上的可见左上角坐标（与 getCurrentWidgetUsedRects 一致，左上角为原点）。
     */
    Q_INVOKABLE QPoint widgetItemScreenPosition(UkuiQuick::WidgetItem *widgetItem) const;

    /*!
     * \brief delegateConfigPosForScreenPosition
     * 将屏幕可见坐标转换为 desktopWidget 的 config x/y（供 setX 使用）。
     */
    Q_INVOKABLE QPoint delegateConfigPosForScreenPosition(QQuickItem *delegate,
                                                          UkuiQuick::WidgetItem *widgetItem,
                                                          int screenX,
                                                          int screenY) const;
    // Texture-based drag preview for cross-screen drag
  /*  Q_INVOKABLE QWindow *createDragPreviewWithTexture(QQuickItem *textureSource, const QPoint &globalPos, int width, int height);
    Q_INVOKABLE void updateDragPreviewPosition(const QPoint &globalPos);
    Q_INVOKABLE void closeDragPreviewWithTexture()*/;
    Q_INVOKABLE bool isDraggingToOtherScreen(const QPoint &globalPos, int widgetWidth, int widgetHeight);
    Q_INVOKABLE void startSystemDrag(QWindow *dragWindow, const QPoint &globalPos, int instanceId, int sourceScreenId);

    Q_INVOKABLE void handleDesktopWidgetDrop(int instanceId, int sourceScreenId, int targetScreenId, const QPoint &dropPos);

protected:
    bool calculateWidgetLayoutTargetPos(UkuiQuick::WidgetItem *widgetItem, const QRegion &usedRegion, QPoint &pos);
    bool calculateWidgetLayoutTargetGridPos(UkuiQuick::WidgetItem *widgetItem, const QRegion &usedRegion, QPoint &pos, bool isInit = true);
    /*!
     * \brief getAutoRangeSortedUris
     * 获取自动排列的图标，fixme:当前使用实时计算的方式获取，对于堆叠的场景无法准确获取正确的顺序
     * 此处需要完善，可能要将图标顺序记录以便于维护，或者对堆叠图标记录顺序
     * \return
     */
    QStringList getAutoRangeSortedUris();

    UkuiQuick::WidgetItemModel *getCurrentWidgetItemModel(WidgetsProxyModel *model = nullptr);

protected:
    void resizeEvent(QResizeEvent *ev) override;

private:
    bool ajustIconsByScreenSizeChange = false;
    bool m_initData = false;
    Peony::DesktopItemProxyModel *m_proxyModel = nullptr;
    Peony::AdvancedDesktopItemModel *m_model = nullptr;
    int m_availableColumnCount;
    int m_availableRowCount;
    QSize m_gridSize;
    int m_currentWidgetsLayoutWidth = -1;
    int m_currentWidgetsLayoutHeight = -1;
    QMargins m_margins;
    QTimer *m_layoutDelayer = nullptr;
    QRegion m_desktopWidgetsUsedRegion;
    QRegion m_dragResizeUsedRegion;
    QMap<int, QRectF> m_dragResizeUsedRegionRects;
    QHash<QString, QPoint> m_resolutionItemPosHash;
    QHash<QString, QPoint> m_resolutionItemNewPosHash;
    QHash<QString, QPoint> m_itemPosHash;
    QStringList m_autoArrange;
    QStringList m_storageBox;
    QScreen *m_screen = nullptr;
    UkuiQuick::WidgetItem *m_currentDragMoveItem = nullptr;
#ifdef KY_SDK_WAYLANDHELPER
    UkuiWindowHelper *m_wmHelper = nullptr;
#endif
    Q_PROPERTY(int currentWidgetsLayoutWidth READ currentWidgetsLayoutWidth WRITE setCurrentWidgetsLayoutWidth NOTIFY currentWidgetsLayoutWidthChanged FINAL)
    Q_PROPERTY(int currentWidgetsLayoutHeight READ currentWidgetsLayoutHeight WRITE setCurrentWidgetsLayoutHeight NOTIFY currentWidgetsLayoutHeightChanged FINAL)
    Q_PROPERTY(bool isPrimary READ isPrimary NOTIFY isPrimaryChanged)
};

#endif // PLATFORMWINDOW_H
