/*
 * Peony-Qt
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef WIDGET_MANAGER_DIALOG_H
#define WIDGET_MANAGER_DIALOG_H

#include <QDialog>
#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QLabel;
class QListWidget;
class QShowEvent;
class QEvent;
class QMoveEvent;
class QQuickItem;
class QSize;
class QWidget;
namespace kdk {
class KSearchLineEdit;
}

#if defined(KY_SDK_WAYLANDHELPER)
class UkuiWindowHelper;
#endif

class PlatformWindow;

namespace UkuiQuick {
class SharedEngineView;
class Widget;
class WidgetItem;
}

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
    ~WidgetManagerDialog() override;

    /*!
     * \brief 返回单例指针；首次调用时创建并关闭时不销毁（WA_DeleteOnClose 为 false）。
     * \return 全局 WidgetManagerDialog 实例，不会为 nullptr（在首次调用后）。
     */
    static WidgetManagerDialog *instance();

    Q_INVOKABLE void addWidgetFromPreview(int entryIndex);
    /*!
     * \brief 使用当前卡片中的真实 WidgetItem 创建实时原生拖拽，避免静态截图在高 DPI 下失真。
     */
    Q_INVOKABLE void startPreviewDrag(int entryIndex, QQuickItem *dragSource);

protected:
    /*! \brief 对话框正常结束时重置搜索状态，再交由 QDialog 完成关闭。 */
    void done(int result) override;
    /*!
     * \brief 显示时去标题栏并启用模糊（与属性窗口一致：X11 为 Motif + KWindowEffects；Wayland 为 UkuiWindowHelper）并刷新右侧底色与预览。
     * \param event 标准 QShowEvent。
     */
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void changeEvent(QEvent *event) override;

private Q_SLOTS:
    /*! \brief 左侧选中项变化时更新标题与卡片可见性。 */
    void onPluginChanged();
    /*! \brief 搜索词变化时切换到全局结果并更新过滤。 */
    void onSearchTextChanged(const QString &text);

private:
    void setupUi();
    void loadWidgets();
    QVariantMap previewEntryAt(int entryIndex) const;
    UkuiQuick::WidgetItem *loadPreviewWidget(const QString &widgetId,
                                             const QVariantMap &variant,
                                             const QSize &desktopSize);
    void clearPreviewWidgets();
    void beginPreviewDrag(const QVariantMap &entry, const QString &payload);
    void endPreviewDrag();
    /*!
     * \brief 按当前点击位置解析目标桌面窗口，与添加分区的选屏逻辑一致。
     * \return 点击位置所在屏幕的 PlatformWindow；无法解析时回退主屏或首个可用窗口。
     */
    PlatformWindow *getTargetWindow() const;
    void updateRightPaneTitle();
    void applyFilters();
    void updateLeftPaneBackground();
    /*! \brief 将应用调色板同步到右侧 Quick 预览场景。 */
    void updatePreviewPanePalette();
    /*! \brief 将应用字体同步到右侧 Quick 预览场景。 */
    void updatePreviewPaneFont();
    void repositionToBottomCenter();
    void schedulePaletteHideWhenDragActive();
    void finalizePaletteDragAfterExec();

    kdk::KSearchLineEdit *m_searchEdit = nullptr;
    QListWidget *m_pluginList = nullptr;
    QLabel *m_rightTitleLabel = nullptr;
    QWidget *m_leftPane = nullptr;
    UkuiQuick::SharedEngineView *m_previewView = nullptr;

    QList<UkuiQuick::Widget *> m_previewWidgets;
    QVariantList m_previewEntries;

#if defined(KY_SDK_WAYLANDHELPER)
    UkuiWindowHelper *m_wmHelper = nullptr;
#endif
    bool m_pendingShowAfterPaletteDrag = false;
    bool m_inProgrammaticMove = false;
    int m_paletteHideSequence = 0;
};

#endif // WIDGET_MANAGER_DIALOG_H
