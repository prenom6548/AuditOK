/*
 * Peony-Qt
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 */

#ifndef WIDGET_MANAGER_DIALOG_H
#define WIDGET_MANAGER_DIALOG_H

#include <QDialog>
#include <QFrame>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QPoint>
#include <QSize>
#include <widget-metadata.h>

class QScrollArea;
class QLabel;
class QVBoxLayout;
class QStackedWidget;
class QGridLayout;
class QListWidget;
class QTimer;
class QShowEvent;
class QEvent;
class QQmlEngine;
class QQuickWidget;
class QMouseEvent;
class QMoveEvent;
namespace kdk {
class KSearchLineEdit;
}

#if defined(KY_SDK_WAYLANDHELPER)
class UkuiWindowHelper;
#endif

class PlatformWindow;

/*!
 * \brief 桌面小插件管理器中的单张卡片控件。
 *
 * 在右侧网格中展示单个插件（或 ManagerVariants 中的一条变体）的标题、描述与 QML 预览，
 * 支持点击添加到桌面、拖拽到桌面（MIME：application/x-peony-widget）以及搜索/分类过滤。
 */
class WidgetCard : public QFrame
{
    Q_OBJECT
public:
    /*!
     * \brief 构造一张插件卡片。
     * \param metadata 插件包元数据（UkuiQuick::WidgetMetadata），决定根路径与默认 Preview 等。
     * \param variant 单条变体配置（来自 Contents.ManagerVariants 或空映射表示默认单实例）。
     * \param sharedPreviewEngine 可选的共享 QQmlEngine，为 nullptr 时卡片内自建引擎（不推荐）。
     * \param parent 父控件，通常为卡片容器。
     * \param previewSlotHeight 预览区统一高度（像素）；≤0 时按变体计算高度。
     */
    explicit WidgetCard(const UkuiQuick::WidgetMetadata &metadata,
                        const QVariantMap &variant,
                        QQmlEngine *sharedPreviewEngine,
                        QWidget *parent = nullptr,
                        int previewSlotHeight = -1);

    /*! \return 插件 Id（metadata.id()）。 */
    QString widgetId() const { return m_widgetId; }
    /*!
     * \brief 控制是否创建/销毁嵌入式 QML 预览，用于滚动视口内仅保留少量活动预览以节省资源。
     * \param active 为 true 时尝试加载预览；为 false 时销毁 QQuickWidget 并显示占位。
     */
    void setPreviewActive(bool active);
    /*!
     * \brief 判断当前卡片是否匹配搜索关键字（名称、描述、Id 的拼接小写子串匹配）。
     * \param keyword 用户输入，空字符串表示不过滤。
     * \return 匹配为 true。
     */
    bool matchesKeyword(const QString &keyword) const;
    /*!
     * \brief 判断当前卡片是否属于指定插件 Id（左侧列表选中某一插件时使用）。
     * \param pluginId 插件 Id；空字符串表示不过滤。
     * \return 匹配为 true。
     */
    bool matchesPlugin(const QString &pluginId) const;

Q_SIGNALS:
    /*!
     * \brief 用户短点击卡片（未触发拖拽）时发出，请求将插件添加到桌面。
     * \param widgetId 插件 Id。
     * \param variant 当前变体映射，供 PlatformWindow::addWidgetFromVariant 等使用。
     */
    void addRequested(const QString &widgetId, const QVariantMap &variant);

protected:
    /*! \param event 鼠标按下事件，用于记录拖拽起点。 */
    void mousePressEvent(QMouseEvent *event) override;
    /*! \param event 鼠标移动事件，超过系统拖拽阈值则启动拖拽。 */
    void mouseMoveEvent(QMouseEvent *event) override;
    /*! \param event 鼠标释放事件，短距离释放视为点击并发出 addRequested。 */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUi(const UkuiQuick::WidgetMetadata &metadata);
    void createPreviewWidget();
    void destroyPreviewWidget();
    /*!
     * \brief 判断给定坐标（卡片本地坐标系）是否落在预览方块 m_previewStack 的几何范围内。
     *
     * 仅当鼠标按下与释放都发生在该区域时，卡片才会发出 addRequested 或启动拖拽。
     * 用于防止点击文字标签或预览框两侧/上下空白处误触发"添加到桌面"。
     */
    bool isPointInPreview(const QPoint &posInCard) const;

    QString m_widgetId;
    QVariantMap m_variant;
    int m_previewSlotHeight = 0;
    QQmlEngine *m_sharedPreviewEngine = nullptr;
    QString m_searchText;
    QSize m_previewSize;

    QFrame *m_previewFrame = nullptr;
    QVBoxLayout *m_previewLayout = nullptr;
    QStackedWidget *m_previewStack = nullptr;
    QLabel *m_previewPlaceholder = nullptr;
    QList<QUrl> m_previewUrls;
    // m_previewWidget：仅作 QML 离屏渲染源（WA_DontShowOnScreen），不进入 m_previewStack。
    // m_previewDisplay：真正显示给用户的 QLabel，每帧由 m_previewWidget->grabFramebuffer() 设置 pixmap。
    QQuickWidget *m_previewWidget = nullptr;
    QLabel *m_previewDisplay = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_descLabel = nullptr;
    QPoint m_dragStartPos;
    bool m_pressInPreview = false;
};

/*!
 * \brief 桌面小插件管理对话框。
 *
 * 左侧为搜索框与插件列表（含「全部」与已安装插件），右侧为可滚动预览网格。
 * 仅展示 ShowIn 含 Desktop 且 Contents 中 ShowInManager（或 ManagerVisible）为真的插件。
 */
class WidgetManagerDialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     * \brief 构造管理对话框。
     * \param parent 父窗口，可为 nullptr。
     */
    explicit WidgetManagerDialog(QWidget *parent = nullptr);

    /*!
     * \brief 返回单例指针；首次调用时创建并关闭时不销毁（WA_DeleteOnClose 为 false）。
     * \return 全局 WidgetManagerDialog 实例，不会为 nullptr（在首次调用后）。
     */
    static WidgetManagerDialog *instance();

    /*!
     * \brief 外部触发刷新：重新调度可见预览更新（防抖后执行 updateVisiblePreviewsNow）。
     */
    void refreshCards();

    /*!
     * \brief 从管理界面卡片开始拖拽时调用：在拖拽已交给系统后的下一事件循环再隐藏窗口，避免 hide 取消 DnD。
     */
    static void beginPaletteDragFromCard();
    /*!
     * \brief 卡片拖拽结束后调用：取消尚未执行的延迟隐藏；若拖拽过程中已隐藏则恢复并贴底居中。
     */
    static void endPaletteDragFromCard();

protected:
    /*!
     * \brief 显示时去标题栏并启用模糊（与属性窗口一致：X11 为 Motif + KWindowEffects；Wayland 为 UkuiWindowHelper）并刷新右侧底色与预览。
     * \param event 标准 QShowEvent。
     */
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    /*!
     * \brief 监听滚动视口 Resize/Show，以调度预览刷新。
     * \param watched 通常为 m_scrollArea->viewport()。
     * \param event 事件对象。
     * \return 基类 eventFilter 返回值。
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    /*!
     * \brief 将插件添加到主桌面 PlatformWindow（优先 id==0）。
     * \param widgetId 插件 Id。
     * \param variant 若含 sizeHint、fixedResizable 或 desktopSize 则走 addWidgetFromVariant，否则走默认 addWidget。
     */
    void onAddWidget(const QString &widgetId, const QVariantMap &variant);
    /*! \param text 搜索框当前文本（内部通过 applyFilters 过滤）。 */
    void onSearchTextChanged(const QString &text);
    /*! \brief 左侧选中项变化时更新标题与卡片可见性。 */
    void onPluginChanged();

private:
    void setupUi();
    void loadWidgets();
    /*!
     * \brief 解析主桌面窗口。
     * \return id 为 0 的 PlatformWindow；若无则返回列表首项；无窗口时返回 nullptr。
     */
    PlatformWindow *getPrimaryWindow() const;
    void ensureSharedPreviewEngine();
    void updateRightPaneTitle();
    void applyFilters();
    void schedulePreviewUpdate();
    void updateVisiblePreviewsNow();
    /*!
     * \brief 恢复右侧滚动区与卡片容器的不透明底色，避免 QQuickWidget 反复显隐后出现透底。
     */
    void repairRightScrollAreaBackground();
    void repositionToBottomCenter();
    void schedulePaletteHideWhenDragActive();
    void finalizePaletteDragAfterExec();

    kdk::KSearchLineEdit *m_searchEdit = nullptr;
    QListWidget *m_pluginList = nullptr;
    QLabel *m_rightTitleLabel = nullptr;
    QWidget *m_cardContainer = nullptr;
    QGridLayout *m_cardLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QTimer *m_previewDebounceTimer = nullptr;
    QQmlEngine *m_sharedPreviewEngine = nullptr;

    QList<UkuiQuick::WidgetMetadata> m_allMetadata;
    QList<WidgetCard *> m_cards;

#if defined(KY_SDK_WAYLANDHELPER)
    UkuiWindowHelper *m_wmHelper = nullptr;
#endif
    bool m_pendingShowAfterPaletteDrag = false;
    bool m_inProgrammaticMove = false;
    int m_paletteHideSequence = 0;
};

#endif // WIDGET_MANAGER_DIALOG_H
