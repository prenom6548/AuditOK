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

    int widgetHorizontalEdgeInset() const;

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

    // 新建文件图标与插件相交检测及挤占重排
    Q_INVOKABLE void checkAndRelayoutIconIfIntersected(int screenId, const QPoint &gridPos = QPoint(0,0));

    Q_INVOKABLE bool adjustIntersectedItem(QQuickItem *dragItem, QQuickItem *item, int parentWidth, bool layoutGrid);

    Q_INVOKABLE bool adjustGridLayoutWidget(QQuickItem *item, UkuiQuick::WidgetItem *widgetItem, bool sizeChanged = false, QQuickItem *dragIndicatorItem = nullptr, WidgetsProxyModel *model = nullptr, int containerWidth = 1900);

    /// 小插件管理器：按 ManagerSizes 生成的标准尺寸变体添加实例
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
     * @param widgetItem 插件的 WidgetItem（root）；用于定位并从占用区域中排除自身
     * @param newWidth   目标像素宽度
     * @param newHeight  目标像素高度
     * @return 当前位置可容纳新尺寸时返回 true，否则返回 false
     */
    Q_INVOKABLE bool canResizeWidgetTo(UkuiQuick::WidgetItem *widgetItem,
                                       int newWidth, int newHeight);

    /**
     * @brief getScreenAvailableGeometry
     * 返回 item 所在屏幕的可用区域（已去掉任务栏占据的区域）。
     * 供插件 QML 在定位右键菜单时使用，确保菜单不被任务栏遮挡。
     * @return 包含 x/y/width/height 的 QVariantMap，坐标为虚拟桌面全局坐标系。
     */
    Q_INVOKABLE QVariantMap getScreenAvailableGeometry() const;

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
    Q_INVOKABLE bool startSystemDrag(QWindow *dragWindow, const QPoint &globalPos, int instanceId, int sourceScreenId,
                                     int widgetWidth, int widgetHeight, int indicatorX, int indicatorY,
                                     bool layoutWithGrid, int gridUnitSize);

    Q_INVOKABLE int widgetItemGridUnitSize(UkuiQuick::WidgetItem *widgetItem) const;

    Q_INVOKABLE void beginDesktopWidgetCrossScreenPreview();
    Q_INVOKABLE void endDesktopWidgetCrossScreenPreview();
    Q_INVOKABLE void previewDesktopWidgetDragPosition(int visualLeft, int layoutY);

    Q_INVOKABLE int handleDesktopWidgetDrop(int instanceId, int sourceScreenId, int targetScreenId, const QPoint &dropPos);

    /*!
     * \brief rejectDesktopWidgetCrossScreenDrop
     * 跨屏拖拽放置时预览未完全落在目标屏幕内，将源屏幕插件复位到 indicator 位置。
     */
    Q_INVOKABLE void rejectDesktopWidgetCrossScreenDrop(int instanceId, int sourceScreenId, int indicatorX, int indicatorY);
    //是否网格对齐
    bool isHeightFitWithGrid(int height);
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
    bool m_crossScreenDropMoved = false;
    bool m_crossScreenPreviewActive = false;
    int m_crossScreenPreviewGridUnit = 0;
    int m_crossScreenPreviewOldX = -1;
    int m_crossScreenPreviewOldY = -1;
    QRect m_crossScreenPreviewUsedRect;
    void restoreDesktopWidgetAfterCrossScreenDrag(int instanceId, int indicatorX, int indicatorY);
    void applyCrossScreenPreviewIconRelayout(QQuickItem *ghost, QQuickItem *dragIndicator);
#ifdef KY_SDK_WAYLANDHELPER
    UkuiWindowHelper *m_wmHelper = nullptr;
#endif
    Q_PROPERTY(int currentWidgetsLayoutWidth READ currentWidgetsLayoutWidth WRITE setCurrentWidgetsLayoutWidth NOTIFY currentWidgetsLayoutWidthChanged FINAL)
    Q_PROPERTY(int currentWidgetsLayoutHeight READ currentWidgetsLayoutHeight WRITE setCurrentWidgetsLayoutHeight NOTIFY currentWidgetsLayoutHeightChanged FINAL)
    Q_PROPERTY(int widgetHorizontalEdgeInset READ widgetHorizontalEdgeInset CONSTANT FINAL)
    Q_PROPERTY(bool isPrimary READ isPrimary NOTIFY isPrimaryChanged)
};

#endif // PLATFORMWINDOW_H
