/*
 * Peony-Qt
 *
 * Copyright (C) 2026, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: yangyanwei <yangyanwei@kylinos.cn>
 *
 */

#ifndef TOOLBARMANAGER_H
#define TOOLBARMANAGER_H

#include <QObject>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QMouseEvent>
#include <QActionGroup>
#include "file-label-menu.h"

#include <QHBoxLayout>
#include <QButtonGroup>

class QShowEvent;
class ViewTypeMenu;
class SortTypeMenu;
class MainWindow;

namespace Peony {
class DirectoryViewMenu;
class FileInfo;
}

class ToolBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBarWidget(QWidget *parent = nullptr);

    // 模拟 QToolBar 的接口
    void addButton(QToolButton *button);
    void addSeparator();
    void addSpacer();
    QMap<QString, QToolButton*> buttonMap() {
        return m_buttonMap;
    }
    void toggleButtons(QList<QToolButton*> showButtons, QList<QToolButton*> hideButtons);
    QSet<QToolButton*> m_hiddenButtons;  // 需要隐藏的按钮列表
    void updateLayout(int availableWidth); // 自定义布局方法
    void setPreviewToolbarButtons(const QList<QToolButton *> &buttons);
    /*! 非 m_hiddenButtons 内的工具按钮是否均因宽度足够而显示（不含「更多」本身） */
    bool allNonHiddenToolButtonsVisibleForLayout() const;
Q_SIGNALS:
    void layoutChanged();
protected:
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *event) override;

private:
    QHBoxLayout *m_layout;
    QList<QWidget*> m_widgets; // 保存所有添加的控件
    QMap<QString, QToolButton*> m_buttonMap;
    QList<QToolButton *> m_previewToolbarButtons; // 窄工具栏时与「更多」一并保留在右侧
};


class DropDownToolButton : public QToolButton {
public:
    using QToolButton::QToolButton;
    DropDownToolButton(QWidget *parent = nullptr) : QToolButton(parent)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    QSize sizeHint() const override
    {
        // 计算完整显示所需大小（图标+文本）
        QSize size = QToolButton::sizeHint();

        if (!text().isEmpty()) {
            // 如果有文本，确保有足够宽度
            QFontMetrics fm(font());
            int textWidth = fm.horizontalAdvance(text());
            int iconWidth = iconSize().width();

            // 完整宽度 = 文本 + 图标 + 边距
            int fullWidth = textWidth + iconWidth*2 + 4*2 + 8*2; // 30像素边距

            // 确保不小于最小宽度
            size.setWidth(qMax(fullWidth, size.width()));
        }

        return size;
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint(); // 最小大小等于完整大小
    }
protected:
    void mousePressEvent(QMouseEvent *e) override {
        // 左键点击时直接显示菜单
        if (e->button() == Qt::LeftButton && menu()) {
            showMenu();
            e->accept();
        } else {
            QToolButton::mousePressEvent(e);
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton && menu()) {
            e->accept();  // 阻止默认的释放事件
        } else {
            QToolButton::mouseReleaseEvent(e);
        }
    }
};

class ToolBarManager : public QObject
{
    friend class MainWindow;
    Q_OBJECT
public:
    struct ToolBarButton {
        // 创建操作组
        QToolButton *newButton;

        // 文件操作组
        QToolButton *cutButton;
        QToolButton *copyButton;
        QToolButton *pasteButton;
        QToolButton *renameButton;
        QToolButton *compressButton;
        QToolButton *deleteButton;

        // 视图操作组
        QToolButton *iconViewButton;
        QToolButton *listViewButton;
        QToolButton *sortButton;
        QToolButton *colorLabelButton;

        // 特殊操作组
        QToolButton *exitEditSpaceButton;
        QToolButton *editSpaceButton;
        QToolButton *exitSearchButton;
        QToolButton *AiQuestionButton;
        QToolButton *moreButton;

        // 多选操作组
        QToolButton *selectAllButton;  // 全选/取消全选按钮
        QToolButton *multiSelectButton;  // 多选/退出多选按钮
    };

    enum DeleteActionType
    {
        CannotDelete,    // 不能删除
        MoveToTrash,     // 移动到回收站
        DeleteDirect,    // 直接删除（无警告）
        DeleteForever    // 永久删除（有警告对话框）
    };
    Q_ENUM(DeleteActionType)
    explicit ToolBarManager(QObject *parent = nullptr);
    ~ToolBarManager();
    QWidget* createToolBar(MainWindow *parent);
    void updateToolBar(const QString &currentUri, const QStringList &selections);
    void updateActionStates(const QString &currentUri, const QStringList &selections);
    bool getNewActionState();
    bool getCutActionState();
    bool getCopyActionState();
    bool getPasteActionState();
    bool getRenameActionState();
    bool getCompressActionState();
    bool getColorLabelActionState();
    QString getDeleteActionText();
    void updateIcons();
    void handleSwitchModeAndShow(const QString &uri);
    void updatePreviewPageVisible();
    void updatePreviewStatus(bool checked);
    void updateEditSpace(bool mode);
    void syncMoreButtonEnabledState();

Q_SIGNALS:
    void requestCreateIntelligentSpace();

private:
    QString checkedPreviewPluginId() const;
    void syncPreviewToolbarChecks(bool previewOn, const QString &pluginId);
    void createButtons();
    void updateNewMenu();
    QMenu* createComputerMenu();
    QMenu* createMoreMenu();
    bool menuHasActionObjectName(QMenu *menu, const QString &objectName) const;
    void insertMenuActionAtBeginning(QMenu *menu, QAction *action);
    QAction *createOverflowSortTypeAction(QMenu *parentMenu);

    QString getComputerUriFromUnixDevice(const QString& unixDevice);
    QString getComputerUriFromUri(const QString& uri);
    QString getSelectionUnixDevices();
    QString getDeviceInfoFromUri(const QString& uri);

    ToolBarButton m_buttons;
    ToolBarWidget *m_toolBarWidget = nullptr;
    QButtonGroup *m_viewButtonGroup = nullptr;
    QTimer *m_progressTimer = nullptr;
    MainWindow* m_window = nullptr;
    QMenu *m_newMenu = nullptr;
    QMenu *m_moreMenu = nullptr;
    SortTypeMenu *m_sortTypeMenuWidget = nullptr;      // 排序菜单
    FileLabelMenu *m_labelMenu = nullptr;
    QList<QToolButton *> m_previewPluginButtons;

    QString m_directory;
    QStringList m_selections;

    DeleteActionType m_deleteActionType = CannotDelete; // 初始化为不能删除

    bool m_is_cd = false;
    bool m_is_ftp = false;
    bool m_is_computer = false;
    bool m_is_network = false;
    bool m_is_trash = false;
    bool m_isSearch = false;
    bool m_is_recent = false;
    bool m_is_favorite = false;
    bool m_can_delete = true;           // if contains computer:/// trash:/// recent:/// and desktop path
    bool m_is_droid = false;
    bool m_is_filesafe = false;
    bool m_is_filebox_file = false;     //filesafe path files operation flag
    bool m_is_smb_file = false;
    bool m_is_mobile_file = false;
    bool m_is_boxpath = false;
    bool m_is_label_model = false;
    bool m_is_idm = false;
    bool m_isContainsHome = false;
    bool m_hasStandardPath = false;

    // 多选功能状态
    bool m_isSelectAll = false;  // 记录当前是否为全选状态
    bool m_isMultiSelect = false;  // 记录当前是否处于多选模式

    // 全选按钮相关方法
    void updateSelectAllButton(bool autoUpdate);
    void onSelectAllButtonClicked();
    void onMultiSelectButtonClicked();
    void updateMultiSelectButton();
    void enableIconViewMultiSelect(bool enable);
    void enableListViewMultiSelect(bool enable);
};

#endif // TOOLBARMANAGER_H
