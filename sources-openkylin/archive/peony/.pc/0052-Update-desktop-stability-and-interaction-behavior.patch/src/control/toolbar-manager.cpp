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

#include "toolbar-manager.h"

#include "view-type-menu.h"
#include "sort-type-menu.h"
#include "tab-widget.h"
#include "action-constants.h"
#include "directory-view-menu.h"
#include "main-window.h"
#include "directory-view-container.h"
#include "icon-view.h"
#include "plugin-manager.h"
#include "menu-plugin-manager.h"
#include "file-info-job.h"
#include "file-info.h"
#include "manager-control.h"
#include "clipboard-utils.h"
#include "file-operation-utils.h"
#include "audio-play-manager.h"
#include "file-delete-operation.h"
#include "sound-effect.h"
#ifdef KY_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif
#include "file-utils.h"
#include "windows/format_dialog.h"
#include "file-launch-action.h"
#include "global-settings.h"
#include "directoryviewhelper.h"
#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"
#include "volume-manager.h"
#include "volumeManager.h"
#include "connect-to-server-dialog.h"
#include "format-dlg-create-delegate.h"
#include "file-enumerator.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-item.h"
#include "idm-dbus-manager.h"
#ifndef KY_UDF_BURN
#include "disccontrol.h"
#include "udfFormatDialog.h"
#else
#include <libkyudfburn/disccontrol.h>
#include "ky-udf-format-dialog.h"
#include "udfAppendBurnDataDialog.h"
using namespace UdfBurn;
#endif

#include <QUrl>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QApplication>
#include <QWidgetAction>
#include <QToolButton>
#include <QProcess>
#include <QShowEvent>
#include <QTimer>
#define DEBUG qDebug() << "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"

ToolBarWidget::ToolBarWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setMinimumWidth(100);  // 设置一个很小的最小宽度
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 6, 8, 6);
    m_layout->setSpacing(4);

    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(0);
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    line->setStyleSheet("background-color: #cccccc;");

    mainlayout->addLayout(m_layout);
    mainlayout->addWidget(line);
    setLayout(mainlayout);
    setFixedHeight(48);
}

void ToolBarWidget::addButton(QToolButton *button)
{
    if (!button) return;

    button->setProperty("setDisableBrush", QBrush(Qt::transparent));
    button->setProperty("setDefaultBrush", QBrush(Qt::transparent));
    button->setMinimumWidth(button->sizeHint().width());
    m_layout->addWidget(button);
    m_widgets.append(button);
    QString name = button->objectName();
    m_buttonMap[name] = button;
}

void ToolBarWidget::addSeparator()
{
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setFixedSize(QSize(2, 16));
   // line->setStyleSheet("background-color: #cccccc;");

    m_layout->addWidget(line);
    m_widgets.append(line);
}

void ToolBarWidget::addSpacer()
{
    // 添加弹簧
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addWidget(spacer);
}

void ToolBarWidget::toggleButtons(QList<QToolButton*> showButtons, QList<QToolButton*> hideButtons)
{
    bool layoutDirty = false;

    for (QToolButton *button : hideButtons) {
        if (!button)
            continue;
        if (button->isVisible()) {
            button->setVisible(false);
            layoutDirty = true;
        }
        const int hiddenCountBefore = m_hiddenButtons.size();
        m_hiddenButtons.insert(button);
        if (m_hiddenButtons.size() != hiddenCountBefore)
            layoutDirty = true;
    }

    for (QToolButton *button : showButtons) {
        if (!button)
            continue;
        if (m_hiddenButtons.remove(button))
            layoutDirty = true;
        if (!button->isVisible()) {
            button->setVisible(true);
            layoutDirty = true;
        }
    }

    if (layoutDirty)
        updateLayout(width() - 40);
}

void ToolBarWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QTimer::singleShot(0, this, [this]() {
        if (!isVisible())
            return;
        int w = width();
        if (w <= 1 && parentWidget())
            w = parentWidget()->width();
        updateLayout(qMax(0, w - 40));
    });
}

void ToolBarWidget::resizeEvent(QResizeEvent *e)
{
        updateLayout(e->size().width() - 40);
    QWidget::resizeEvent(e);
}

bool ToolBarWidget::allNonHiddenToolButtonsVisibleForLayout() const
{
    for (QWidget *widget : m_widgets) {
        auto *btn = qobject_cast<QToolButton *>(widget);
        if (!btn) {
            continue;
        }
        if (m_hiddenButtons.contains(btn)) {
            continue;
        }
        if (btn->objectName() == QLatin1String("more-action")) {
            continue;
        }
        if (btn->isVisible() && btn->isEnabled()) {
            return true;
        }
    }
    return false;
}

void ToolBarWidget::updateLayout(int availableWidth)
{
    if (m_widgets.count()<0) {
        return;
    }
    // 计算总宽度
    int totalWidth = 16;

    // 先计算所有控件的宽度
    QList<QPair<QWidget*, int>> widgetWidths;
    for (QWidget *widget : m_widgets) {
        int width = 0;
        if (qobject_cast<QFrame*>(widget)) { // 分隔符
            width = 8;
        } else if (QToolButton *btn = qobject_cast<QToolButton*>(widget)) {
            if (m_hiddenButtons.contains(btn)) {
                continue;
            }
            // 直接使用实际宽度或sizeHint
            width = btn->width();
            if (width <= 0) {
                width = btn->sizeHint().width() + 4;
            }
            // 确保有最小宽度
            width = btn->sizeHint().width();
        } else {
            width = widget->sizeHint().width();
        }

        widgetWidths.append(qMakePair(widget, width));
        totalWidth += width;
    }

    // 如果总宽度小于可用宽度，显示所有
    if (totalWidth <= availableWidth) {
        for (QWidget *widget : m_widgets) {
            if (QToolButton *btn = qobject_cast<QToolButton*>(widget)) {
                if (m_hiddenButtons.contains(btn)) {
                    continue;
                }
            }
            if (!widget->isVisible())
                widget->show();
        }
        Q_EMIT layoutChanged();
        return;
    }

    // 固定按钮必须显示（更多和预览）
    QSet<QWidget*> fixedWidgets;

    // 使用指针直接比较，更高效
        fixedWidgets.insert(m_buttonMap.value("preview-action"));
        fixedWidgets.insert(m_buttonMap.value("more-action"));

    // 计算固定按钮的总宽度
    int fixedWidth = 0;
    for (const auto &pair : widgetWidths) {
        if (fixedWidgets.contains(pair.first)) {
            fixedWidth += pair.second;
        }
    }


    // 开始布局计算
    int currentWidth = fixedWidth + 16 + 4;
    QSet<QWidget*> widgetsToShow = fixedWidgets;

    // 从前往后添加普通按钮
    for (int i = 0; i < widgetWidths.size(); ++i) {
        QWidget *widget = widgetWidths[i].first;
        int width = widgetWidths[i].second;

        // 跳过已处理的按钮
        if (widgetsToShow.contains(widget)) {
            continue;
        }
        if (QToolButton *btn = qobject_cast<QToolButton*>(widget)) {
            if (m_hiddenButtons.contains(btn)) {
                continue;
            }
        }
        // 处理分隔符 - 只有前后都有显示的控件时才显示
        if (qobject_cast<QFrame*>(widget)) {
            bool prevVisible = (i > 0 && widgetsToShow.contains(widgetWidths[i-1].first));
            bool nextVisible = (i < widgetWidths.size()-1 &&
                               !(m_hiddenButtons.contains(qobject_cast<QToolButton*>(widgetWidths[i+1].first))));

           // if (prevVisible && nextVisible && currentWidth + width <= availableWidth && currentWidth + width + widgetWidths[i+1].second <= availableWidth) {
                widgetsToShow.insert(widget);
                currentWidth += width;
            //}
            continue;
        }

        // 普通按钮：检查空间
        if (currentWidth + width <= availableWidth) {
            widgetsToShow.insert(widget);
            currentWidth += width + 4;
        } else {
            break; // 空间不足
        }
    }

    // 更新可见性
    for (QWidget *widget : m_widgets) {
        const bool shouldShow = widgetsToShow.contains(widget);
        if (widget->isVisible() != shouldShow)
            widget->setVisible(shouldShow);
    }

    Q_EMIT layoutChanged();
}

ToolBarManager::ToolBarManager(QObject *parent) : QObject(parent)
{

}
ToolBarManager::~ToolBarManager()
{
    // 先断开全局单例的连接，防止析构期间信号触发
    disconnect(Peony::GlobalSettings::getInstance(), nullptr, this, nullptr);

    // 清理动态创建的菜单
    if (m_labelMenu) {
        delete m_labelMenu;
        m_labelMenu = nullptr;
    }
    if (m_moreMenu) {
        delete m_moreMenu;
        m_moreMenu = nullptr;
    }
}

QWidget* ToolBarManager::createToolBar(MainWindow *parent)
{
    m_window = parent;

    // 创建自定义的工具栏
    m_toolBarWidget = new ToolBarWidget(parent);
    m_toolBarWidget->setObjectName("dynamic-toolbar");
    // 设置属性阻止菜单弹出
    m_toolBarWidget->setAttribute(Qt::WA_NoMousePropagation);
    m_toolBarWidget->setFocusPolicy(Qt::NoFocus);

    // 创建所有按钮
    createButtons();

    m_toolBarWidget->addButton(m_buttons.newButton);
    m_toolBarWidget->addSeparator();

    m_toolBarWidget->addButton(m_buttons.cutButton);
    m_toolBarWidget->addButton(m_buttons.copyButton);
    m_toolBarWidget->addButton(m_buttons.pasteButton);
    m_toolBarWidget->addButton(m_buttons.renameButton);
    m_toolBarWidget->addButton(m_buttons.compressButton);
    m_toolBarWidget->addButton(m_buttons.deleteButton);
    m_toolBarWidget->addSeparator();

    m_toolBarWidget->addButton(m_buttons.iconViewButton);
    m_toolBarWidget->addButton(m_buttons.listViewButton);
    m_toolBarWidget->addSeparator();

    m_toolBarWidget->addButton(m_buttons.sortButton);
    // 全选、多选（图标/列表视图下始终显示）
    m_toolBarWidget->addButton(m_buttons.selectAllButton);
    m_toolBarWidget->addButton(m_buttons.multiSelectButton);
    m_toolBarWidget->addSeparator();

    m_toolBarWidget->addButton(m_buttons.colorLabelButton);
    m_toolBarWidget->addButton(m_buttons.editSpaceButton);
    m_toolBarWidget->addButton(m_buttons.exitEditSpaceButton);
    m_toolBarWidget->addButton(m_buttons.AiQuestionButton);
    m_toolBarWidget->addButton(m_buttons.exitSearchButton);

    // 更多按钮
    m_toolBarWidget->addButton(m_buttons.moreButton);

    m_toolBarWidget->addSpacer();

    // 预览按钮（最后添加，总是在最右边）
    m_toolBarWidget->addButton(m_buttons.previewButton);

    connect(m_toolBarWidget, &ToolBarWidget::layoutChanged, this, &ToolBarManager::syncMoreButtonEnabledState);

    // 连接多选状态信号，当视图通过框选等方式触发多选时同步更新按钮状态
    connect(Peony::DirectoryViewHelper::globalInstance(), &Peony::DirectoryViewHelper::updateSelectStatus,
            this, [=](bool status) {
        m_isMultiSelect = status;
        updateMultiSelectButton();
        updateSelectAllButton(true);
    });

    return m_toolBarWidget;
}

void ToolBarManager::createButtons()
{
    // 新建按钮
    m_buttons.newButton = new DropDownToolButton(m_toolBarWidget);
    m_buttons.newButton->setObjectName("create-action");
    m_buttons.newButton->setText(tr("New"));
    m_buttons.newButton->setIcon(QIcon::fromTheme("zoom-in-symbolic"));

    m_buttons.newButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_newMenu = new QMenu(m_toolBarWidget);
    m_buttons.newButton->setMenu(m_newMenu);
    m_buttons.newButton->setPopupMode(QToolButton::MenuButtonPopup);
    connect(m_newMenu, &QMenu::aboutToShow, this, &ToolBarManager::updateNewMenu, Qt::UniqueConnection);

    // 剪切
    m_buttons.cutButton = new QToolButton(m_toolBarWidget);
    m_buttons.cutButton->setObjectName("cut-action");
    m_buttons.cutButton->setIcon(QIcon::fromTheme("edit-cut-symbolic"));
    m_buttons.cutButton->setToolTip(tr("Cut"));
    m_buttons.cutButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(m_buttons.cutButton, &QToolButton::clicked, this, [this]() {
        if (m_selections.isEmpty())
            return;
        ClipboardUtils::setClipboardFiles(m_selections, true, m_isSearch);
        m_window->getCurrentPage()->getView()->repaintView();
    });

    // 复制
    m_buttons.copyButton = new QToolButton(m_toolBarWidget);
    m_buttons.copyButton->setObjectName("copy-action");
    m_buttons.copyButton->setIcon(QIcon::fromTheme("edit-copy-symbolic"));
    m_buttons.copyButton->setToolTip(tr("Copy"));
    m_buttons.copyButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(m_buttons.copyButton, &QToolButton::clicked, this, [this]() {
        if (m_selections.isEmpty())
            return;
        if (m_is_idm) {
            QStringList uris;
            if(m_selections.count() > 1)
            {
                for(auto path : m_selections)
                {
                    QString realPath = "";
                    if (!path.isEmpty()) {
                        QString tempUri = path.split('/', QString::SkipEmptyParts).last();
                        realPath = "file://" + QUrl::fromPercentEncoding(tempUri.toLatin1());
                        uris << realPath;
                    }
                }
            } else {
                QString uri = m_selections.first();
                QString tempUri = uri.split('/', QString::SkipEmptyParts).last();
                QString realUri = "file://" + QUrl::fromPercentEncoding(tempUri.toLatin1());
                if (!realUri.isEmpty()) {
                    uris << realUri;
                }
            }
            ClipboardUtils::getInstance()->setClipboardFiles(uris, false, false);
        } else if (m_is_recent) {
            QStringList selections;
            for (auto uri : m_selections) {
                uri = FileUtils::getTargetUri(uri);
                selections << uri;
            }
            ClipboardUtils::setClipboardFiles(selections, false);
        } else {
            ClipboardUtils::setClipboardFiles(m_selections, false);
        }
    });

    // 粘贴
    m_buttons.pasteButton = new QToolButton(m_toolBarWidget);
    m_buttons.pasteButton->setObjectName("paste-action");
    m_buttons.pasteButton->setIcon(QIcon::fromTheme("edit-paste-symbolic"));
    m_buttons.pasteButton->setToolTip(tr("Paste"));
    m_buttons.pasteButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    connect(ClipboardUtils::getInstance(), &ClipboardUtils::clipboardChanged, m_buttons.pasteButton, [=](){
        //粘贴按钮状态
        bool state = getPasteActionState();
        m_buttons.pasteButton->setEnabled(state);
    });

    connect(m_buttons.pasteButton, &QToolButton::clicked, this, [this]() {
        auto op = ClipboardUtils::pasteClipboardFiles(m_directory);
        if (op) {
            connect(op, &Peony::FileOperation::operationFinished, m_window, [=](){
                auto opInfo = op->getOperationInfo();
                auto targetUirs = opInfo->dests();
                QTimer::singleShot(300, m_window, [=](){
                    m_window->setCurrentSelectionUris(targetUirs);
                });
            }, Qt::BlockingQueuedConnection);
        }
        else{
            m_window->getCurrentPage()->getView()->repaintView();
        }
    });

    // 重命名
    m_buttons.renameButton = new QToolButton(m_toolBarWidget);
    m_buttons.renameButton->setObjectName("rename-action");
    m_buttons.renameButton->setIcon(QIcon::fromTheme("document-edit-symbolic"));
    m_buttons.renameButton->setToolTip(tr("Rename"));
    m_buttons.renameButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(m_buttons.renameButton, &QToolButton::clicked, this, [this]() {
        if (m_selections.count() == 1) {
            m_window->getCurrentPage()->getView()->editUri(m_selections.first());
        } else if (m_selections.count() > 1) {
            m_window->getCurrentPage()->getView()->editUris(m_selections);
        }
    });

    // 压缩
    QString engrampaPluginName = QString("libpeony-menu-plugin-kylin-engrampa.so");
    auto engrampaPlugin = PluginManager::getInstance()->getPluginByFileName(engrampaPluginName);
    QIcon compressIcon = QIcon(":/icons/media-zip-symbolic");
    m_buttons.compressButton = new QToolButton(m_toolBarWidget);
    m_buttons.compressButton->setObjectName(engrampaPlugin ? engrampaPlugin->name() : "compress-action");
    m_buttons.compressButton->setIcon(compressIcon);
    m_buttons.compressButton->setToolTip(tr("Compress"));
    m_buttons.compressButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttons.compressButton->setEnabled(false);
    if (engrampaPlugin) {
        auto engrampaPluginId = PluginManager::getInstance()->getPluginId(engrampaPlugin);
        connect(m_buttons.compressButton, &QToolButton::clicked, this, [this, engrampaPluginId]() {
            auto plugin = MenuPluginManager::getInstance()->getPlugin(engrampaPluginId);
            if (!plugin) {
                qDebug() << "压缩插件不存在";
                return;
            }
            QList<QAction *> actions = plugin->menuActions(MenuPluginInterface::DirectoryView, m_directory, m_selections);
            if (actions.isEmpty()) {
                qDebug() << "没有可用的压缩操作";
                return;
            }
            if (actions.count() == 1) {
                QAction *singleAction = actions.first();
                if (singleAction->isEnabled()) {
                    Q_EMIT singleAction->trigger();
                }
            } else {
                QMenu menu(m_toolBarWidget);
                menu.addActions(actions);
                if (m_buttons.compressButton) {
                    QPoint pos = m_buttons.compressButton->mapToGlobal(QPoint(0, m_buttons.compressButton->height()));
                    menu.exec(pos);
                } else {
                    menu.exec(QCursor::pos());
                }
            }
        });
    } else {
        // 插件不存在时，点击按钮提示用户
        connect(m_buttons.compressButton, &QToolButton::clicked, this, []() {
            qDebug() << "压缩插件未安装";
        });
    }

    // 删除
    m_buttons.deleteButton = new QToolButton(m_toolBarWidget);
    m_buttons.deleteButton->setObjectName("delete-action");
    m_buttons.deleteButton->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    m_buttons.deleteButton->setToolTip(tr("Delete"));
    m_buttons.deleteButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(m_buttons.deleteButton, &QToolButton::clicked, this, [this]() {
        switch(m_deleteActionType) {
        case DeleteActionType::CannotDelete:
            break;
        case DeleteActionType::MoveToTrash:
            FileOperationUtils::trash(m_selections, true, m_isSearch);
            break;
        case DeleteActionType::DeleteDirect:
            FileOperationUtils::remove(m_selections);
            break;
        case DeleteActionType::DeleteForever:
            FileOperationUtils::executeRemoveActionWithDialog(m_selections);
            break;
        }
    });

    // 图标视图
    m_buttons.iconViewButton = new QToolButton(m_toolBarWidget);
    m_buttons.iconViewButton->setObjectName("icon-view-action");
    m_buttons.iconViewButton->setProperty("viewId", "Icon View");
    m_buttons.iconViewButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));
    m_buttons.iconViewButton->setToolTip(tr("Icon View"));
    m_buttons.iconViewButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttons.iconViewButton->setCheckable(true);
    connect(m_buttons.iconViewButton, &QToolButton::clicked, this, [this]() {
        auto viewId = m_buttons.iconViewButton->property("viewId").toString();
        bool isSearchPage = m_directory.startsWith("search:///");
        if (isSearchPage) {
            return;
        }
        m_window->beginSwitchView(viewId);
    });

    // 列表视图
    m_buttons.listViewButton = new QToolButton(m_toolBarWidget);
    m_buttons.listViewButton->setObjectName("view-type-action");
    m_buttons.listViewButton->setProperty("viewId", "List View");
    m_buttons.listViewButton->setIcon(QIcon::fromTheme("view-list-symbolic"));
    m_buttons.listViewButton->setToolTip(tr("List View"));
    m_buttons.listViewButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttons.listViewButton->setCheckable(true);
    connect(m_buttons.listViewButton, &QToolButton::clicked, this, [this]() {
        auto viewId = m_buttons.listViewButton->property("viewId").toString();
        bool isSearchPage = m_directory.startsWith("search:///");
        if (isSearchPage) {
            return;
        }
        m_window->beginSwitchView(viewId);
    });

    m_viewButtonGroup = new QButtonGroup(m_toolBarWidget);
    m_viewButtonGroup->setExclusive(true);
    m_viewButtonGroup->addButton(m_buttons.iconViewButton);
    m_viewButtonGroup->addButton(m_buttons.listViewButton);

    // 排序按钮
    m_buttons.sortButton = new DropDownToolButton(m_toolBarWidget);
    m_buttons.sortButton->setObjectName("sort-type-action");
    m_buttons.sortButton->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
    m_buttons.sortButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.sortButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_sortTypeMenuWidget = new SortTypeMenu(m_buttons.sortButton);
    m_buttons.sortButton->setMenu(m_sortTypeMenuWidget);
    QString sortTypeName = m_sortTypeMenuWidget->getSortTypeName(m_window->getCurrentSortColumn());
    m_buttons.sortButton->setText(sortTypeName);

    // 连接排序信号
    connect(m_sortTypeMenuWidget, &SortTypeMenu::switchSortTypeRequest, m_window, &MainWindow::setCurrentSortColumn);
    connect(m_sortTypeMenuWidget, &SortTypeMenu::switchSortOrderRequest, m_window, [this](Qt::SortOrder order) {
        QToolButton *btn = m_buttons.sortButton;
        if (btn) {
            if (order == Qt::AscendingOrder) {
                btn->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
            } else {
                btn->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
            }
        }
        m_window->setCurrentSortOrder(order);
    });

    connect(m_sortTypeMenuWidget, &QMenu::aboutToShow, m_sortTypeMenuWidget, [this]() {
        bool originPathVisible = m_directory == "trash:///";
        m_sortTypeMenuWidget->setOriginPathVisible(originPathVisible);
        bool isSearchTab = m_directory.startsWith("search:///");
        m_sortTypeMenuWidget->setFilePathVisible(false);

        bool isIDMTab = m_directory.startsWith("idm:///");
        m_sortTypeMenuWidget->setUserAddedEnable(isIDMTab);

        if (!isIDMTab && m_sortTypeMenuWidget->isAutoModeType(m_window->getCurrentSortColumn())) {
            m_window->setCurrentSortColumn(0);
        }

        m_sortTypeMenuWidget->setSortType(m_window->getCurrentSortColumn());
        m_sortTypeMenuWidget->setSortOrder(m_window->getCurrentSortOrder());
    });

    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [=](const QString &key){
        // 安全检查：成员无效时直接返回
        if (!m_sortTypeMenuWidget || !m_window || !m_buttons.sortButton) {
            return;
        }
        if (SORT_COLUMN == key || key == USE_GLOBAL_DEFAULT_SORTING || SORT_UPDATE == key) {
            QString sortTypeName = m_sortTypeMenuWidget->getSortTypeName(m_window->getCurrentSortColumn());
            m_sortTypeMenuWidget->updateSortOrderName(m_window->getCurrentSortColumn());
            m_buttons.sortButton->setText(sortTypeName);
            if (m_window->getCurrentSortOrder() == Qt::AscendingOrder) {
                m_buttons.sortButton->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
            } else {
                m_buttons.sortButton->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
            }
        }
    });

    // 预览按钮
    auto manager = Peony::PreviewPageFactoryManager::getInstance();
    auto pluginNames = manager->getPluginNames();
    for (auto name : pluginNames) {
        auto factory = manager->getPlugin(name);
        m_buttons.previewButton = new QToolButton(m_toolBarWidget);
        m_buttons.previewButton->setObjectName("preview-action");
        m_buttons.previewButton->setIcon(factory->icon());
        m_buttons.previewButton->setToolTip(factory->name());
        m_buttons.previewButton->setAccessibleName(factory->name());
        m_buttons.previewButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_buttons.previewButton->setCheckable(true);
        m_buttons.previewButton->setVisible(true);
        connect(m_buttons.previewButton, &QToolButton::clicked, this, [this](bool checked) {
            if (!MANAGER_CONTROL->checkPreviewPanePermission()) {
                m_buttons.previewButton->setChecked(!checked);
                return;
            }
            Peony::GlobalSettings::getInstance()->setValue(DEFAULT_DETAIL, checked);
            m_window->m_tab->setTriggeredPreviewPage(checked);
            auto manager = Peony::PreviewPageFactoryManager::getInstance();
            auto pluginNames = manager->getPluginNames();
            for (auto name : pluginNames) {
                if (checked) {
                    auto plugin = manager->getPlugin(name);
                    m_window->m_tab->setPreviewPage(plugin->createPreviewPage());
                } else {
                    m_window->m_tab->setPreviewPage(nullptr);
                }
            }
        });
        break;
    }

    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [this](const QString &key) {
        if (key == ENABLE_FILE_PREVIEW_PANE) {
            updatePreviewPageVisible();
        }
    });

    // 颜色标识
    m_buttons.colorLabelButton = new QToolButton(m_toolBarWidget);
    m_buttons.colorLabelButton->setObjectName("file-label-action");
    m_buttons.colorLabelButton->setIcon(QIcon(":/icons/sign"));
    m_buttons.colorLabelButton->setToolTip(tr("Add File Label"));
    m_buttons.colorLabelButton->setAccessibleName(tr("Add File Label"));
    m_buttons.colorLabelButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttons.colorLabelButton->setPopupMode(QToolButton::InstantPopup);
    m_buttons.colorLabelButton->setCheckable(true);
    connect(m_buttons.colorLabelButton, &QToolButton::clicked, this, [this](bool checked) {
        if (m_labelMenu && m_labelMenu->isVisible()) {
            delete m_labelMenu;
            m_labelMenu = nullptr;
            m_buttons.colorLabelButton->setChecked(false);
            return;
        }
        m_buttons.colorLabelButton->setChecked(true);
        m_labelMenu = new FileLabelMenu(m_selections, m_toolBarWidget);
        connect(m_labelMenu, &QMenu::aboutToHide, this, [=](){
            m_buttons.colorLabelButton->setChecked(false);
        });
        if (m_buttons.colorLabelButton) {
            QPoint pos = m_buttons.colorLabelButton->mapToGlobal(QPoint(0, m_buttons.colorLabelButton->height()));
            m_labelMenu->exec(pos);
        } else {
            m_labelMenu->exec(QCursor::pos());
        }
    });

    // 编辑空间
    m_buttons.editSpaceButton = new DropDownToolButton(m_toolBarWidget);
    m_buttons.editSpaceButton->setObjectName("edit-space-action");
    m_buttons.editSpaceButton->setText(tr("Edit Conditions"));
    m_buttons.editSpaceButton->setIcon(QIcon(":/icons/folder-space-setting-symbolic"));
    m_buttons.editSpaceButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.editSpaceButton->setVisible(false);
    m_toolBarWidget->m_hiddenButtons.insert(m_buttons.editSpaceButton);
    connect(m_buttons.editSpaceButton, &QToolButton::clicked, this, [this]() {
        updateEditSpace(true);
        m_window->m_tab->switchToEditMode();
    });

    // 退出编辑空间
    m_buttons.exitEditSpaceButton = new DropDownToolButton(m_toolBarWidget);
    m_buttons.exitEditSpaceButton->setObjectName("exit-edit-space-action");
    m_buttons.exitEditSpaceButton->setText(tr("Exit Edit Conditions"));
    m_buttons.exitEditSpaceButton->setIcon(QIcon::fromTheme("edit-clear-symbolic"));
    m_buttons.exitEditSpaceButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.exitEditSpaceButton->setVisible(false);
    m_toolBarWidget->m_hiddenButtons.insert(m_buttons.exitEditSpaceButton);
    connect(m_buttons.exitEditSpaceButton, &QToolButton::clicked, this, [this]() {
        m_window->m_tab->quitEditMode();
        updateEditSpace(false);
    });


    m_buttons.AiQuestionButton =   new DropDownToolButton(m_toolBarWidget);
    m_buttons.AiQuestionButton->setObjectName("Ai-Question-Answering-action");
    m_buttons.AiQuestionButton->setText(tr("Ai Question Answer"));
    m_buttons.AiQuestionButton->setIcon(QIcon::fromTheme("kylin-ai-assistant"));
    m_buttons.AiQuestionButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.AiQuestionButton->setVisible(false);
    m_toolBarWidget->m_hiddenButtons.insert(m_buttons.AiQuestionButton);
    connect(m_buttons.AiQuestionButton, &QToolButton::clicked, this, [this]() {
        auto uri = m_window->m_tab->getCurrentUri();
        QString spaceName = uri.remove("idm:///");
        spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));

        // 获取空间文件列表
        QStringList files = IDMDBusManager::getInstance()->getFilesInSpaceByName(spaceName);
        int totalFiles = files.count();

        // 生成文件对应的docId列表
        QStringList docIds;
        for (const QString &filePath : files) {
            QString docId = IDMDBusManager::getInstance()->getFileLabels(filePath).isEmpty()
                ? filePath : IDMDBusManager::getInstance()->getFileLabels(filePath).first();
            // 使用文件路径作为docId（与generateDocId一致）
            docIds.append(filePath);
        }

        // 创建语义库并上传文件
        const QString kbId = IDMDBusManager::getInstance()->semanticKBCreate(spaceName);
        IDMDBusManager::getInstance()->semanticKBAddDocumentsForSpace(spaceName);

        if (!kbId.isEmpty()) {
            const QString dbPath = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                .absoluteFilePath("peony-intelligent-data-management-service/semantic-knoweldgebase/");
            QProcess::startDetached("kylin-aiassistant",
                                    QStringList() << "--appid"
                                                  << "peony-idm"
                                                  << "--dbpath"
                                                  << dbPath
                                                  << "--kb"
                                                  << kbId
                                                  <<"--kbname"
                                                  <<spaceName);
        }

        // 创建定时器定期查询解析进度
        if(m_progressTimer == nullptr){
            m_progressTimer = new QTimer(this);
        }
        connect(m_progressTimer, &QTimer::timeout, this, [this, spaceName, totalFiles, docIds]() {
            int completedCount = 0;
            for (const QString &docId : docIds) {
                int status = -1;
                int progress = IDMDBusManager::getInstance()->semanticKBGetDocumentStatus(spaceName, docId);
                if (progress < 0) {
                    continue;
                }
                // KB_DOC_STATUS_COMPLETED = 2 表示解析完成
                if (progress == 100) {
                    completedCount++;
                }
            }
            m_window->updateStatusBar(tr("AI has completed the analysis %1/%2 files").arg(completedCount).arg(totalFiles));

            // 当所有文件解析完成时停止定时器
            if (completedCount >= totalFiles && totalFiles > 0) {
                m_progressTimer->stop();
//                m_progressTimer->deleteLater();
//                m_progressTimer = nullptr;
            }
        });

        // 启动定时器，每500ms查询一次
        m_progressTimer->start(500);

        // 连接目录切换信号，当目录切换时停止定时器
        connect(m_window->m_tab, &TabWidget::directoryChanging, this, [this]() {
            if (m_progressTimer) {
                m_progressTimer->stop();
//                m_progressTimer->deleteLater();
//                m_progressTimer = nullptr;
            }
        });
    });


    // 退出搜索
    m_buttons.exitSearchButton = new DropDownToolButton(m_toolBarWidget);
    m_buttons.exitSearchButton->setObjectName("exit-search-action");
    m_buttons.exitSearchButton->setText(tr("Exit Search"));
    m_buttons.exitSearchButton->setIcon(QIcon::fromTheme("edit-clear-symbolic"));
    m_buttons.exitSearchButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.exitSearchButton->setVisible(false);
    m_toolBarWidget->m_hiddenButtons.insert(m_buttons.exitSearchButton);
    connect(m_buttons.exitSearchButton, &QToolButton::clicked, this, [this]() {
        if (!(m_window && m_window->m_tab)) {
            return;
        }
        auto container = m_window->getCurrentPage();
        QString targetUri = "";
        QStringList backList = container->getBackList();
        if (!backList.isEmpty()) {
             // 取最后一个就是进入搜索前的目录
             targetUri = backList.last();
             m_window->m_tab->goToUri(targetUri, true, true);
        }
        m_window->m_tab->closeSearch();

        // 退出搜索后，如果目标路径是 idm 空间，显示编辑空间按钮
        if (targetUri.startsWith("idm:///") && targetUri != "idm:///") {
            updateEditSpace(false);
        }
    });

    // 全选/取消全选（与多选按钮同样始终显示，逻辑与 header-bar 一致）
    m_buttons.selectAllButton = new QToolButton(m_toolBarWidget);
    m_buttons.selectAllButton->setObjectName("select-all-action");
    m_buttons.selectAllButton->setText(tr("Select All"));
    m_buttons.selectAllButton->setToolTip(tr("Select all"));
    m_buttons.selectAllButton->setIcon(QIcon::fromTheme("edit-select-all-symbolic"));
    m_buttons.selectAllButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.selectAllButton->setVisible(false);
    m_toolBarWidget->m_hiddenButtons.insert(m_buttons.selectAllButton);
    connect(m_buttons.selectAllButton, &QToolButton::clicked, this, [this]() {
        onSelectAllButtonClicked();
    });

    // 多选/退出多选按钮（在图标和列表视图中均显示）
    m_buttons.multiSelectButton = new QToolButton(m_toolBarWidget);
    m_buttons.multiSelectButton->setObjectName("multi-select-action");
    m_buttons.multiSelectButton->setText(tr("MultiSelect"));
    m_buttons.multiSelectButton->setToolTip(m_buttons.multiSelectButton->text());
    m_buttons.multiSelectButton->setIcon(QIcon::fromTheme("selection-symbolic"));
    m_buttons.multiSelectButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_buttons.multiSelectButton->setVisible(false);
    m_toolBarWidget->m_hiddenButtons.insert(m_buttons.multiSelectButton);
    connect(m_buttons.multiSelectButton, &QToolButton::clicked, this, [this]() {
        onMultiSelectButtonClicked();
    });

    // 更多按钮
    m_buttons.moreButton = new QToolButton(m_toolBarWidget);
    m_buttons.moreButton->setObjectName("more-action");
    m_buttons.moreButton->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    m_buttons.moreButton->setToolTip(tr("More"));
    m_buttons.moreButton->setAccessibleName(tr("More"));
    m_buttons.moreButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_buttons.moreButton->setCheckable(true);
    connect(m_buttons.moreButton, &QToolButton::clicked, this, [this](bool checked) {
        if (m_moreMenu && m_moreMenu->isVisible()) {
            delete m_moreMenu;
            m_moreMenu = nullptr;
            return;
        }
        m_buttons.moreButton->setChecked(true);
        // 计算机页面使用自定义菜单
        if (m_directory == "computer:///") {
            m_moreMenu = createComputerMenu();
        } else {
            m_moreMenu = createMoreMenu();
        }

        connect(m_moreMenu, &QMenu::aboutToHide, this, [=](){
            m_buttons.moreButton->setChecked(false);
        });

        if (m_buttons.moreButton) {
            QPoint pos = m_buttons.moreButton->mapToGlobal(QPoint(0, m_buttons.moreButton->height()));
            m_moreMenu->exec(pos);
        } else {
            m_moreMenu->exec(QCursor::pos());
        }

    });

}

void ToolBarManager::syncMoreButtonEnabledState()
{
    if (!m_buttons.moreButton || !m_toolBarWidget) {
        return;
    }
    bool enable = true;
    if (m_is_idm && !m_isSearch && m_directory == QStringLiteral("idm:///")
        && m_selections.isEmpty()
        && m_toolBarWidget->allNonHiddenToolButtonsVisibleForLayout()) {
        QMenu *probe = createMoreMenu();
        const bool menuEmpty = !probe || probe->isEmpty();
        delete probe;
        if (menuEmpty) {
            enable = false;
        }
    }
    m_buttons.moreButton->setEnabled(enable);
}

void ToolBarManager::updateToolBar(const QString &currentUri, const QStringList &selections)
{
    bool directoryChanged = (m_directory != currentUri);
    m_selections = selections;
    m_directory = currentUri;

    // 设置具体按钮状态
    updateActionStates(currentUri, selections);

    // 切换tab时更新编辑空间按钮
    handleSwitchModeAndShow(currentUri);

    // 多选按钮始终显示，全选按钮仅在多选模式下显示
    {
        m_toolBarWidget->m_hiddenButtons.remove(m_buttons.multiSelectButton);

        if (directoryChanged) {
            enableIconViewMultiSelect(false);
            enableListViewMultiSelect(false);
            m_isMultiSelect = false;
            updateMultiSelectButton();
        } else {
            // 切换视图时保持工具栏多选状态，同步到新视图；路径变化已在上面重置
            auto page = m_window->getCurrentPage();
            if (page) {
                auto view = page->getView();
                if (view) {
                    auto *iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
                    if (iface2) {
                        if (iface2->isEnableMultiSelect() != m_isMultiSelect) {
                            iface2->doMultiSelect(m_isMultiSelect);
                        }
                        updateMultiSelectButton();
                    }
                }
            }
        }
    }

    // 更新全选按钮状态
    updateSelectAllButton(true);

    // 触发布局更新
    m_toolBarWidget->updateLayout(m_toolBarWidget->width() - 40);
}

void ToolBarManager::updateActionStates(const QString &directory, const QStringList &selections)
{
    m_deleteActionType = DeleteActionType::CannotDelete;

    m_directory = directory;
    m_selections = selections;

    m_is_cd = false;
    m_is_ftp = false;
    m_is_computer = false;
    m_is_network = false;
    m_is_trash = false;
    m_isSearch = false;
    m_is_recent = false;
    m_is_favorite = false;
    m_can_delete = true;           // if contains computer:/// trash:/// recent:/// and desktop path
    m_is_droid = false;
    m_is_filesafe = false;
    m_is_filebox_file = false;     //filesafe path files operation flag
    m_is_smb_file = false;
    m_is_mobile_file = false;
    m_is_boxpath = false;
    m_is_label_model = false;
    m_is_idm = false;
    m_isContainsHome = false;
    m_hasStandardPath = false;

    QString homeUri = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString musicUri = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString videoUri = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    QString docUri = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString pictureUri = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString downloadUri = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    QString actualDir =  directory;
    m_directory = directory;
    if(actualDir.startsWith("search://")){
        m_isSearch = true;
        actualDir = FileUtils::getActualDirFromSearchUri(m_directory);
    }

    QString tmpUri;
    g_autoptr (GFile) tmp_file = g_file_new_for_uri(actualDir.toUtf8().constData());
    g_autofree gchar* tmp_uri = g_file_get_uri(tmp_file);
    tmpUri = tmp_uri;

    if (actualDir == "computer:///") {
        m_is_computer = true;
    }

    if (actualDir == "network:///") {
        m_is_network = true;
    }

    if (actualDir == "trash:///") {
        m_is_trash = true;
    }

    if (actualDir.startsWith("burn://")) {
        m_is_cd = true;
    }

    if (actualDir.startsWith("recent://")) {
        m_is_recent = true;
    }

    if (actualDir.startsWith("favorite://")) {
        m_is_favorite = true;
    }

    if (actualDir.startsWith("kmre:///") || actualDir.startsWith("mult://")) {
        m_is_droid = true;
    }

    if (actualDir.startsWith("ftp://")
            || actualDir.startsWith("sftp://") || tmpUri.startsWith("ftp://") || tmpUri.startsWith("sftp://")) {
        m_is_ftp = true;
    }

    if (actualDir.startsWith("filesafe:///")){
        m_is_filebox_file = true;
    }

    if(actualDir == "filesafe:///" || m_directory.startsWith("search:///search_uris=filesafe:///&")) {
        m_is_filesafe = true;
    }

    if(actualDir.startsWith("smb://") || tmpUri.startsWith("smb://")){
        m_is_smb_file = true;
    }

    QString boxpath = "file://"+QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.box";
    if(actualDir == boxpath) {
        m_is_boxpath = true;
    }

    if (actualDir.startsWith("label://")){
        m_is_label_model = true;
    }

    if (actualDir.startsWith("idm://") && !m_isSearch) {
        m_is_idm = true;
    }


    m_is_mobile_file = Peony::FileUtils::isMobileFile(actualDir);/* 判断是否为移动文件或目录 */

    if (!m_is_mobile_file && !m_selections.isEmpty()) {
        m_is_mobile_file = Peony::FileUtils::isMobileFile(m_selections.first());
    }

    bool isRemovableDevice = m_is_mobile_file;

    //task#147972 【删除回收站】选项需求
    //如果是长城的机器并且带了9215控制器，不再区分移动设备，统一右键删除到回收站
    bool trashSettings = GlobalSettings::getInstance()->getValue(TRASH_MOBILE_FILES).toBool();
    if (trashSettings && !isRemovableDevice) {
       m_is_mobile_file = false;
    }

    m_isContainsHome = m_selections.isEmpty()? false : m_selections.contains("file://" +  homeUri);
            // 包含家目录不能剪切
    m_hasStandardPath = FileUtils::containsStandardPath(m_selections);

    for (auto uriIndex = 0; uriIndex < m_selections.count(); ++uriIndex) {
        if (m_selections.at(uriIndex) == "favorite:///?schema=trash"
                || m_selections.at(uriIndex) == "favorite:///?schema=kmre"
                || m_selections.at(uriIndex) == "favorite:///?schema=recent"
                || m_selections.at(uriIndex) == "favorite:///data/usershare?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + homeUri + "?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + desktop + "?schema=file") {
            m_can_delete = false;
            break;
        }
    }

    //netwotk items not show operation menu
    if (m_is_network || m_is_computer || m_is_idm) {
        bool enable = m_is_idm && (m_selections.count()>0) && (actualDir != "idm:///");
        m_buttons.newButton->setEnabled(false);
        m_buttons.cutButton->setEnabled(false);
        m_buttons.copyButton->setEnabled(enable);
        m_buttons.pasteButton->setEnabled(false);
        m_buttons.renameButton->setEnabled(false);
        m_buttons.compressButton->setEnabled(false);
        m_buttons.deleteButton->setEnabled(false);
        m_buttons.iconViewButton->setEnabled(m_is_idm);
        m_buttons.listViewButton->setEnabled(m_is_idm);
        m_buttons.sortButton->setEnabled(m_is_idm);
        m_buttons.selectAllButton->setEnabled(m_is_idm);
        m_buttons.multiSelectButton->setEnabled(m_is_idm);
        m_buttons.colorLabelButton->setEnabled(enable);
        QList<QToolButton*> showButtons, hideButtons;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons<<m_buttons.exitSearchButton);
        syncMoreButtonEnabledState();
        return;
    }

    if (m_is_droid) {
        m_buttons.newButton->setEnabled(false);
        m_buttons.cutButton->setEnabled(false);
        bool state = getCopyActionState(); //只允许复制
        m_buttons.copyButton->setEnabled(state);
        m_buttons.pasteButton->setEnabled(false);
        m_buttons.renameButton->setEnabled(false);
        m_buttons.compressButton->setEnabled(false);
        m_buttons.deleteButton->setEnabled(false);
        m_buttons.iconViewButton->setEnabled(false);
        m_buttons.listViewButton->setEnabled(false);
        m_buttons.sortButton->setEnabled(false);
        m_buttons.colorLabelButton->setEnabled(false);
        QList<QToolButton*> showButtons, hideButtons;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons<<m_buttons.exitSearchButton);
        syncMoreButtonEnabledState();
        return;
    }

    // 新建按钮状态
    bool state = getNewActionState();
    m_buttons.newButton->setEnabled(state);

    //剪切按钮状态
    state = getCutActionState();
    m_buttons.cutButton->setEnabled(state);

    //复制按钮状态
    state = getCopyActionState();
    m_buttons.copyButton->setEnabled(state);

    //粘贴按钮状态
    state = getPasteActionState();
    m_buttons.pasteButton->setEnabled(state);

    //重命名按钮状态
    state = getRenameActionState();
    m_buttons.renameButton->setEnabled(state);

    //压缩按钮状态
    state = getCompressActionState();
    m_buttons.compressButton->setEnabled(state);

    //删除按钮状态
    QString text = getDeleteActionText();
    state = !text.isEmpty();
    m_buttons.deleteButton->setToolTip(text);
    if (m_deleteActionType == DeleteActionType::MoveToTrash) {
        m_buttons.deleteButton->setObjectName("trash-action");
    } else {
        m_buttons.deleteButton->setObjectName("delete-action");
    }
    m_buttons.deleteButton->setEnabled(state);

    //颜色标识按钮状态
    state = getColorLabelActionState();
    m_buttons.colorLabelButton->setEnabled(state);
    // 更新 FileLabelMenu 的选中项
    if (m_labelMenu && state) {
        m_labelMenu->updateSelections(selections);
    }
    m_buttons.multiSelectButton->setEnabled(true);

    //更新搜索状态
    state = m_isSearch;
//    m_buttons.exitSearchButton->setVisible(state);
    QList<QToolButton*> lists;
    if (state) {
        QList<QToolButton*> showButtons, hideButtons;
        m_toolBarWidget->toggleButtons(showButtons<<m_buttons.exitSearchButton, hideButtons);
    } else {
        QList<QToolButton*> showButtons, hideButtons;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons<<m_buttons.exitSearchButton);
    }

    syncMoreButtonEnabledState();
}

bool ToolBarManager::getNewActionState()
{
    // 检查是否允许新建
    bool canCreate = true;

    // 特殊路径不允许新建
    if (m_isSearch || m_is_favorite ||
        m_is_filesafe || m_is_trash||
        m_is_label_model || m_directory.startsWith("droid://") ||
        m_directory.startsWith("recent://") || m_is_cd || m_is_boxpath ||
        m_is_network) {
        canCreate = false;
        return canCreate;
    }

    // 检查是否为光盘路径
    if (m_directory.startsWith("burn://") || m_directory.contains(".iso")) {
        canCreate = false;
        return canCreate;
    }

    // 检查是否为.box路径
    if (m_directory.contains(".box")) {
        canCreate = false;
        return canCreate;
    }

    // 检查目录是否可写（除了 FTP 和 SMB 路径，这两种路径即使不可写也允许新建）
    auto info = FileInfo::fromUri(m_directory);
    if (!m_is_ftp && !m_is_smb_file && info && !info->canWrite()) {
        canCreate = false;
        return canCreate;
    }

    // 检查是否开启筛选功能
    if(m_window->getFilterWorking()) {
        canCreate = false;
    }
    return canCreate;
}

void ToolBarManager::updateNewMenu()
{
    if (!m_newMenu) {
        m_newMenu = new QMenu(m_toolBarWidget);
        if (m_buttons.newButton)
            m_buttons.newButton->setMenu(m_newMenu);
        connect(m_newMenu, &QMenu::aboutToShow, this, &ToolBarManager::updateNewMenu, Qt::UniqueConnection);
    }

    m_newMenu->clear();

    QList<QAction *> actions;
    auto createFolderActions = new QAction(QIcon::fromTheme("folder"), tr("Folder"), this);
    actions<<createFolderActions;
    connect(actions.last(), &QAction::triggered, this, [=]() {
        //FileOperationUtils::create(m_directory, nullptr, CreateTemplateOperation::EmptyFolder);
        CreateTemplateOperation op(m_directory, CreateTemplateOperation::EmptyFolder, tr("New Folder"));
        Peony::FileOperationErrorDialogWarning dlg;
        connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
        op.run();
        auto targetUri = op.target();
        qDebug()<<"target:"<<targetUri;
        QTimer::singleShot(300, this, [=](){
            m_window->getCurrentPage()->getView()->clearIndexWidget();
            m_window->getCurrentPage()->getView()->setSelections(QStringList());
            m_window->editUri(targetUri);
        });
    });
    auto createEmptyFileAction = new QAction(QIcon::fromTheme("text-plain"), tr("Text Document"), this);
    actions<<createEmptyFileAction;
    connect(actions.last(), &QAction::triggered, this, [=]() {
        //FileOperationUtils::create(m_directory);
        CreateTemplateOperation op(m_directory);
        Peony::FileOperationErrorDialogWarning dlg;
        connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
        op.run();
        auto targetUri = op.target();
        qDebug()<<"target:"<<targetUri;
        QTimer::singleShot(300, this, [=](){
            m_window->getCurrentPage()->getView()->clearIndexWidget();
            m_window->getCurrentPage()->getView()->setSelections(QStringList());
            m_window->editUri(targetUri);
        });
    });
    m_newMenu->addActions(actions);

    //enumerate template dir
    QDir templateDir(g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES));
    QString templatePath = templateDir.path();
    qWarning()<<"tempalte Path is"<<templatePath;
    QStringList fileNames;
    if(!templatePath.isEmpty())
    {
        QDir templateDir(templatePath);
        auto templates = templateDir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
        if (!templates.isEmpty()) {
            for (auto t : templates) {
                QFileInfo qinfo(templateDir, t);
                qWarning()<<"template entry is"<<qinfo.filePath();
                GFile *gtk_file = g_file_new_for_path(qinfo.filePath().toUtf8().constData());
                char *uri_str = g_file_get_uri(gtk_file);
                std::shared_ptr<FileInfo> info = FileInfo::fromUri(uri_str);
                QIcon tmpIcon;
                if (info->uri().endsWith(".desktop")) {
                    QUrl url = info->uri();
                    auto path = url.path();
                    auto key_file = g_key_file_new();
                    if (g_key_file_load_from_file(key_file, path.toUtf8().constData(), G_KEY_FILE_NONE, 0)) {
                        g_autofree gchar* icon_name = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, 0);
                        if (icon_name) {
                            QString iconName = icon_name;
                            tmpIcon = QIcon::fromTheme(iconName);
                        }
                        g_key_file_free(key_file);
                    }
                } else {
                    QString iconName = FileUtils::updateFileIconName(info->uri(), true);
                    tmpIcon = QIcon::fromTheme(iconName);
                }


                QAction *action = new QAction(tmpIcon, qinfo.baseName(), this);
                connect(action, &QAction::triggered, this, [=]() {
                    // automatically check for conficts
                    CreateTemplateOperation op(m_directory, CreateTemplateOperation::Template, t);
                    Peony::FileOperationErrorDialogWarning dlg;
                    connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
                    op.run();
                    auto target = op.target();
                    QTimer::singleShot(300, this, [=](){
                        m_window->getCurrentPage()->getView()->clearIndexWidget();
                        m_window->getCurrentPage()->getView()->setSelections(QStringList());
                        m_window->editUri(target);
                    });
                });
                m_newMenu->addAction(action);
                fileNames.append(qinfo.fileName());
                g_free(uri_str);
                g_object_unref(gtk_file);
            }
        } else {
            qWarning()<<"template entries is empty";
        }
    } else {
        qWarning()<<"template path is empty";
    }

    templatePath = "/opt/kingsoft/wps-office/templates/";
    if(!templatePath.isEmpty())
    {
        QDir templateDir(templatePath);
        auto templates = templateDir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
        if (!templates.isEmpty()) {
            for (auto t : templates) {
                QFileInfo qinfo(templateDir, t);
                auto abspath = qinfo.absoluteFilePath();
                if (fileNames.contains(qinfo.fileName())) {
                    continue;
                }

                qWarning()<<"template entry is"<<qinfo.filePath();
                GFile *gtk_file = g_file_new_for_path(qinfo.filePath().toUtf8().constData());
                char *uri_str = g_file_get_uri(gtk_file);
                std::shared_ptr<FileInfo> info = FileInfo::fromUri(uri_str);

                QString mimeType = info->mimeType();
                if (mimeType.isEmpty()) {
                    FileInfoJob job(info);
                    job.querySync();
                    mimeType = info->mimeType();
                }

                QIcon tmpIcon;
                GList *app_infos = g_app_info_get_recommended_for_type(mimeType.toUtf8().constData());
                GList *l = app_infos;
                QList<FileLaunchAction *> actions;
                bool isOnlyUnref = false;
                while (l) {
                    auto app_info = static_cast<GAppInfo*>(l->data);
                    if (!isOnlyUnref) {
                        GThemedIcon *icon = G_THEMED_ICON(g_app_info_get_icon(app_info));
                        const char * const * icon_names = g_themed_icon_get_names(icon);
                        if (icon_names)
                            tmpIcon = QIcon::fromTheme(*icon_names);
                        if(!tmpIcon.isNull())
                            isOnlyUnref = true;
                    }
                    l = l->next;
                }
                g_list_free_full(app_infos, g_object_unref);

                QAction *action = new QAction(tmpIcon, qinfo.baseName(), this);
                connect(action, &QAction::triggered, [=]() {
                    // automatically check for conficts
                    CreateTemplateOperation op(m_directory, CreateTemplateOperation::Template, abspath);
                    Peony::FileOperationErrorDialogWarning dlg;
                    connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
                    op.run();
                    auto target = op.target();
                    QTimer::singleShot(300, this, [=](){
                        m_window->getCurrentPage()->getView()->clearIndexWidget();
                        m_window->getCurrentPage()->getView()->setSelections(QStringList());
                        m_window->editUri(target);
                    });
                });
                m_newMenu->addAction(action);
                g_free(uri_str);
                g_object_unref(gtk_file);
            }
        } else {
            qWarning()<<"template entries is empty";
        }
    } else {
        qWarning()<<"template path is empty";
    }

    // Add separator before intelligent space
    m_newMenu->addSeparator();

    // Add "New Intelligent Space" action with same icon as sidebar
   // m_window->topLevelWidget()->objectName();
    Peony::IDMDBusManager::getInstance()->setActiveWindow(m_window->topLevelWidget());
    QAction *createIntelligentSpaceAction = new QAction(QIcon::fromTheme("folder-space-symbolic"), tr("Intelligent Space"), this);
    m_newMenu->addAction(createIntelligentSpaceAction);
    connect(createIntelligentSpaceAction, &QAction::triggered, this, [=]() {
        // Emit signal to request creating intelligent space dialog
        Q_EMIT requestCreateIntelligentSpace();
    });
}

bool ToolBarManager::getCutActionState()
{
    bool canCut = !m_selections.isEmpty();
    if (canCut) {
        // 特殊路径不显示剪切
        if (m_is_computer || m_is_favorite || m_is_label_model || m_is_recent || m_is_filesafe || m_is_droid || m_is_network) {
            return false;
        }

        if (m_isContainsHome || m_hasStandardPath) {
            return false;
        }

        // 检查当前目录可写（除了 FTP 和 SMB 路径，这两种路径即使不可写也允许剪切）
        QString actualDir =  m_directory;
        if(actualDir.startsWith("search://")){
            actualDir =  FileUtils::getActualDirFromSearchUri(m_directory);
        }

        auto info = FileInfo::fromUri(actualDir);
        if (!m_is_ftp && !m_is_smb_file && info && !info->canWrite()) {
            canCut = false;
            return false;
        }

        // 搜索路径需要特殊处理
        if (m_isSearch) {
            canCut = FileUtils::isSearchFilesParentWriteable(m_selections, m_isSearch);
        }
    }
    return canCut;
}

bool ToolBarManager::getCopyActionState()
{
    bool canCopy = !m_selections.isEmpty();
    if (canCopy) {
        // 特殊路径不显示复制
        if (m_is_favorite || m_is_computer || m_is_filesafe || m_is_network) {
            canCopy = false;
            return canCopy;
        }
        if (m_isContainsHome) {
            canCopy = false;
            return canCopy;
        }

        // 只允许复制文件
        if (m_is_droid) {
            bool hasDir = false;
            for (auto uri : m_selections) {
                auto info = FileInfo::fromUri(uri);
                if (info->isDir()) {
                    hasDir = true;
                    break;
                }
            }

            if (hasDir) {
                canCopy = false;
            }
        }
    }
    return canCopy;
}

bool ToolBarManager::getPasteActionState()
{
    bool canPaste = ClipboardUtils::isClipboardHasFiles();
    if (canPaste) {
        // 特殊路径不显示粘贴
        if (m_is_trash || m_is_computer || m_isSearch || m_is_recent || m_is_favorite || m_is_droid || m_is_filesafe || m_is_label_model || m_is_cd || m_is_network || m_is_idm) {
            canPaste = false;
            return canPaste;
        }

        // 检查当前目录可写（除了 FTP 和 SMB 路径，这两种路径即使不可写也允许粘贴）
        auto info = FileInfo::fromUri(m_directory);
        if (!m_is_ftp && !m_is_smb_file && info && !info->canWrite()) {
            canPaste = false;
            return canPaste;
        }

        // 检查是否为特殊文件系统
        bool isDirectoryCanWrite = true;
        if (!info->isEmptyInfo()) {
            isDirectoryCanWrite = info->canWrite();
            if (!isDirectoryCanWrite) {
                QString fileSystem = info.get()->fileSystemType();
                if (fileSystem.isEmpty()) {
                    fileSystem = FileUtils::getFsTypeFromFile(info.get()->uri());
                    qDebug() << "file system :" << fileSystem;
                }
                if (fileSystem.contains("udf")) {
                    qDebug() << "file system contains:" << fileSystem;
                    isDirectoryCanWrite = true;
                }
            }
        }
        canPaste = isDirectoryCanWrite;
    }
    return canPaste;
}

bool ToolBarManager::getRenameActionState()
{
    bool canRename = !m_selections.isEmpty();
    if (canRename) {
        // 特殊路径不显示重命名
        if (m_is_trash || m_is_computer || m_is_recent || m_is_favorite || m_is_filesafe || m_is_label_model || m_is_droid || m_is_network) {
            canRename = false;
            return canRename;
        }

        // 检查是否为Home目录
        if (m_isContainsHome) {
            canRename = false;
            return canRename;
        }

        if (m_isSearch && m_selections.count() > 1) {
            canRename = false;
            return canRename;
        }


        // 检查是否包含标准路径
        bool hasStandardPath = FileUtils::containsStandardPath(m_selections);
        if (hasStandardPath) {
            canRename = false;
        }
    }
    return canRename;
}

bool ToolBarManager::getCompressActionState()
{
    // 特殊路径不显示压缩操作
    if (m_is_network || m_is_computer || m_is_idm || m_is_droid || m_is_favorite || m_is_label_model) {
        return false;
    }

    auto info = FileInfo::fromUri(m_directory);
    FileInfoJob infoJob(info);
    infoJob.querySync();
    if (info) {
        QString fsType = info.get()->fileSystemType();
        qDebug() << "光盘文件系统类型:" << fsType;
        QStringList notAllowType = {"isofs" , "iso9660"};
        // iso9660 类型不允许压缩操作
        if (notAllowType.contains(fsType)) {
            return false;
        }
    }

    QString engrampaPluginName = QString("libpeony-menu-plugin-kylin-engrampa.so");
    auto engrampaPlugin = PluginManager::getInstance()->getPluginByFileName(engrampaPluginName);
    if (!engrampaPlugin) {
        return false;
    }
    auto engrampaPluginId = PluginManager::getInstance()->getPluginId(engrampaPlugin);

    auto plugin = MenuPluginManager::getInstance()->getPlugin(engrampaPluginId);
    if (!plugin) {
        return false;
    }
    QList<QAction *> actions = plugin->menuActions(MenuPluginInterface::DirectoryView, m_directory, m_selections);
    qDebug() << "压缩actions数量:" << actions.count() << m_directory << m_selections;
    const bool enabled = !actions.isEmpty();
    qDeleteAll(actions);
    return enabled;
}

bool ToolBarManager::getColorLabelActionState()
{
    bool canColorLabel = !m_selections.isEmpty();
    if (canColorLabel) {
        // 特殊路径不显示颜色标识
        if (m_is_trash || m_is_smb_file || m_is_recent || m_is_computer || m_is_droid || m_is_network) {
            canColorLabel = false;
            return canColorLabel;
        }
        auto info = FileInfo::fromUri(m_directory);
        if (info->isVirtual()) {
            canColorLabel = false;
            return canColorLabel;
        }
        if (canColorLabel) {
    //            updateColorLabelMenu(selections);
        }
    }
    return canColorLabel;
}

QString ToolBarManager::getDeleteActionText()
{
    if (m_selections.isEmpty()) {
        m_deleteActionType = DeleteActionType::CannotDelete;
        return "";
    }

    // 特殊路径不显示删除按钮：文件保险箱、droid环境、网络路径
    if (m_is_filesafe || m_is_droid || m_is_network) {
        m_deleteActionType = DeleteActionType::CannotDelete;
        return "";
    }

    // 包含家目录或标准路径时不显示删除按钮
    if (m_hasStandardPath && m_isContainsHome) {
        m_deleteActionType = DeleteActionType::CannotDelete;
        return "";
    }

    // 特殊收藏项检查
    QString homeUri = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    for (const QString &uri : m_selections) {
        if (uri == "favorite:///?schema=trash" ||
            uri == "favorite:///?schema=kmre" ||
            uri == "favorite:///?schema=recent" ||
            uri == "favorite:///data/usershare?schema=file" ||
            uri == "favorite://" + homeUri + "?schema=file" ||
            uri == "favorite://" + desktop + "?schema=file") {
            m_deleteActionType = DeleteActionType::CannotDelete;
            return "";
        }
    }

    // 白名单应用检查：桌面目录下的白名单应用不能删除
    bool containsWhitelist = MANAGER_CONTROL->containsWhitelistApp(m_selections);
    bool isDesktopDir = MANAGER_CONTROL->isDesktopDirectory(m_directory);
    if (isDesktopDir && containsWhitelist) {
        m_deleteActionType = DeleteActionType::CannotDelete;
        return "";
    }

    // recent 路径下直接删除
    if (m_is_recent && m_selections.count() > 0) {
        m_deleteActionType = DeleteActionType::DeleteDirect;
        return tr("Delete");
    }

    if (m_is_trash) {
        m_deleteActionType = DeleteActionType::DeleteForever;
        return tr("Delete");
    }

    // 其他路径的判断逻辑
    bool hasDeleteForever = false;
    if (!m_is_recent && !m_is_favorite && !m_hasStandardPath && !m_is_filesafe && !m_is_label_model) {
        bool canTrash = true;
        bool canDelete = true;
        // 检查文件权限
        auto info = FileInfo::fromUri(m_selections.first());
        if (info && !info->canTrash())
            canTrash = false;
        if (info && !info->canDelete() && !m_selections.first().startsWith("ftp://"))
            canDelete = false;

        // 长文件名检查：不能删除到回收站
        if (FileUtils::isLongNameFileOfNotDel2Trash(m_selections.first())) {
            canTrash = false;
        }

        // 可以删除到回收站
        if (canTrash && !m_is_filebox_file && !m_is_mobile_file && !(isDesktopDir && containsWhitelist)) {
            m_deleteActionType = DeleteActionType::MoveToTrash;
            return tr("Delete to trash");
        }

        // 永久删除
        if (m_can_delete && canDelete && !(isDesktopDir && containsWhitelist)) {
            hasDeleteForever = true;
            m_deleteActionType = DeleteActionType::DeleteForever;
            // 文件保险箱文件显示"Delete"，其他显示"Delete forever"
            if (m_is_filebox_file) {
                return tr("Delete");
            } else {
                return tr("Delete forever");
            }
        }
    }

    // 收藏夹、标签页、smb、ftp 文件的永久删除
    if ((m_is_favorite || m_is_label_model) && m_can_delete && !m_is_filesafe && !hasDeleteForever) {
        m_deleteActionType = DeleteActionType::DeleteForever;
        return tr("Delete forever");
    }

    if (m_is_smb_file && m_can_delete && !hasDeleteForever) {
        m_deleteActionType = DeleteActionType::DeleteForever;
        return tr("Delete forever");
    }

    if (m_is_ftp && m_can_delete && !hasDeleteForever) {
        m_deleteActionType = DeleteActionType::DeleteForever;
        return tr("Delete forever");
    }

    m_deleteActionType = DeleteActionType::CannotDelete;
    return "";
}

void ToolBarManager::updateIcons()
{
    if(!m_window)
        return;

    if (!m_window->getCurrentPage())
        return;

    if (!m_window->getCurrentPage()->getView())
        return;

    qDebug()<<"updateIcons:" <<m_window->getCurrentUri();
    qDebug()<<"updateIcons:" <<m_window->getCurrentSortColumn();
    qDebug()<<"updateIcons:" <<m_window->getCurrentSortOrder();

    int count = 0;
    if(m_window->getCurrentPage() && m_window->getCurrentPage()->getView()){
        auto iface = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(m_window->getCurrentPage()->getView());
        if(iface)
            count = iface->getAllDisplayFileCount();
    }
    if(!(m_window->getCurrentUri().startsWith("search:///") && count>30000)){/* 取消搜索时数量超过阈值不排序 */
        m_sortTypeMenuWidget->switchSortTypeRequest(m_window->getCurrentSortColumn());
        m_sortTypeMenuWidget->switchSortOrderRequest(m_window->getCurrentSortOrder());
    }

    QString viewId = m_window->getCurrentPage()->getView()->viewId();
    for (auto button : m_viewButtonGroup->buttons()) {
        if (button->property("viewId").toString() == viewId) {
            button->setChecked(true);
        }
    }

    auto url = m_window->getCurrentUri();
    if (url == "computer:///") {
        m_buttons.iconViewButton->setEnabled(false);
        m_buttons.listViewButton->setEnabled(false);
        m_buttons.sortButton->setEnabled(false);
    } else {
        m_buttons.iconViewButton->setEnabled(true);
        m_buttons.listViewButton->setEnabled(true);
        m_buttons.sortButton->setEnabled(true);
    }
    if(url.startsWith("search:///")){
        m_buttons.iconViewButton->setEnabled(false);
    }else{
        m_buttons.iconViewButton->setEnabled(true);
    }

    updatePreviewPageVisible();
}

void ToolBarManager::handleSwitchModeAndShow(const QString &uri)
{
    qDebug()<<"switch space mode"<<uri<<m_window->getCurrentPage()->getCurrentUri();

    // 搜索时隐藏编辑空间和退出编辑空间按钮
    if (uri.startsWith("search:///")) {
        QList<QToolButton*> showButtons, hideButtons;
        hideButtons<<m_buttons.editSpaceButton<<m_buttons.exitEditSpaceButton<<m_buttons.AiQuestionButton;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons);
        return;
    }

    // 进入 idm 空间中（排除 idm:/// 根目录和新建空间）
    if (uri.startsWith("idm:///") && uri != "idm:///" && uri != "idm:///New Intelligent Space") {
        if (m_window->m_tab->isEditMode()) {
            updateEditSpace(true);
        } else {
            updateEditSpace(false);
        }
    } else {
        // 非 idm 空间，隐藏编辑空间按钮
        QList<QToolButton*> showButtons, hideButtons;
        hideButtons<<m_buttons.editSpaceButton<<m_buttons.exitEditSpaceButton<<m_buttons.AiQuestionButton;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons);
    }
}

void ToolBarManager::updatePreviewPageVisible()
{
    if (!MANAGER_CONTROL->checkPreviewPanePermission()) {
        m_buttons.previewButton->setEnabled(false);
        bool shouldEnablePreview = Peony::GlobalSettings::getInstance()->getValue(ENABLE_FILE_PREVIEW_PANE).toBool();
        bool currentlyTriggered = m_window->m_tab->getTriggeredPreviewPage();

        if (shouldEnablePreview && !currentlyTriggered) {
            auto manager = Peony::PreviewPageFactoryManager::getInstance();
            auto pluginNames = manager->getPluginNames();
            for (auto name : pluginNames) {
                auto plugin = manager->getPlugin(name);
                m_window->m_tab->setPreviewPage(plugin->createPreviewPage());
                m_window->m_tab->setTriggeredPreviewPage(true);
                break;
            }
        } else if (!shouldEnablePreview && currentlyTriggered) {
            m_window->m_tab->setPreviewPage(nullptr);
            m_window->m_tab->setTriggeredPreviewPage(false);
        }
        m_buttons.previewButton->setChecked(shouldEnablePreview);
        return;
    }

    m_buttons.previewButton->setEnabled(true);
    auto check = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_DETAIL).toBool();
    m_window->m_tab->setTriggeredPreviewPage(check);
    m_buttons.previewButton->setChecked(check);
    m_buttons.previewButton->setCheckable(true);
    m_buttons.previewButton->setVisible(true);

    auto currentUri = m_window->getCurrentUri();
    if (currentUri == "computer:///") {
        m_buttons.previewButton->setChecked(false);
    }

    auto manager = Peony::PreviewPageFactoryManager::getInstance();
    auto pluginNames = manager->getPluginNames();
    for (auto name : pluginNames) {
        if (m_buttons.previewButton->isChecked() && m_buttons.previewButton->isVisible()) {
            auto plugin = Peony::PreviewPageFactoryManager::getInstance()->getPlugin(name);
            m_window->m_tab->setPreviewPage(plugin->createPreviewPage());
        } else {
            m_window->m_tab->setPreviewPage(nullptr);
        }
    }
}

void ToolBarManager::updatePreviewStatus(bool checked)
{
    if (m_buttons.previewButton && m_buttons.previewButton->isVisible()) {
        m_buttons.previewButton->setChecked(checked);
        m_buttons.previewButton->clicked(checked);
    }
}

void ToolBarManager::updateEditSpace(bool mode)
{
    if (mode) {
        QList<QToolButton*> showButtons, hideButtons;
        showButtons<<m_buttons.exitEditSpaceButton<<m_buttons.AiQuestionButton;
        hideButtons<<m_buttons.editSpaceButton;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons);
    } else {
        QList<QToolButton*> showButtons, hideButtons;
        showButtons<<m_buttons.editSpaceButton<<m_buttons.AiQuestionButton;
        hideButtons<<m_buttons.exitEditSpaceButton;
        m_toolBarWidget->toggleButtons(showButtons, hideButtons);
    }

}

QString ToolBarManager::getComputerUriFromUnixDevice(const QString &unixDevice)
{
    /* volume item,遍历方式获取uri - 参考 side-bar-menu.cpp */
    Peony::FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString uri;
    for (auto fileInfo : e.getChildren()) {
        Peony::FileInfoJob infoJob(fileInfo);
        infoJob.querySync();
        /* 由volume的unixDevice获取computer uri */
        auto info = infoJob.getInfo();
        QString device = fileInfo.get()->unixDeviceFile();
        if (device == unixDevice) {
            uri = fileInfo.get()->uri();
            break;
        }
    }
    return uri;
}

QString ToolBarManager::getComputerUriFromUri(const QString &uri)
{
    Peony::FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString computerUri = uri;
    for (auto fileInfo : e.getChildren()) {
        Peony::FileInfoJob infoJob(fileInfo);
        infoJob.querySync();
        if ((fileInfo.get()->targetUri() == uri
             || Peony::FileUtils::urlDecode(fileInfo.get()->targetUri()) == uri) && !uri.isEmpty()) {
            computerUri = fileInfo.get()->uri();
            break;
        }
    }
    return computerUri;
}

QString ToolBarManager::getSelectionUnixDevices()
{
    QString device = "";

    if (!m_window)
        return device;

    auto page = m_window->getCurrentPage();
    if (!page)
        return device;

    auto view = page->getView();
    if (!view)
        return device;

    // 通过 DirectoryViewIface2 获取实际 View
    auto iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);

    QAbstractItemView *abstractView = nullptr;

    if (iface2) {
        // 常规视图：IconView/ListView
        abstractView = dynamic_cast<QAbstractItemView*>(iface2);
    } else {
        // 计算机页面等插件视图，尝试获取内部的 QAbstractItemView
        abstractView = view->findChild<QAbstractItemView*>();
    }

    if (!abstractView)
        return device;

    // 获取选中项
    auto selectionModel = abstractView->selectionModel();
    if (!selectionModel)
        return device;

    QModelIndexList indexes = selectionModel->selectedIndexes();
    if (indexes.isEmpty())
        return device;

    const int UnixDeviceRole = Qt::UserRole + 100;
    for (const auto &index : indexes) {
        QVariant deviceVar = abstractView->model()->data(index, UnixDeviceRole);
        qDebug()<<"UnixDeviceRole data:"<<deviceVar.toString();
        if (deviceVar.isValid() && !deviceVar.toString().isEmpty()) {
            return deviceVar.toString();
        }
    }

    // 常规路径：通过 FileItemModel 获取
    auto proxyModel = qobject_cast<Peony::FileItemProxyFilterSortModel*>(abstractView->model());
    if (!proxyModel)
        return device;

    auto sourceModel = qobject_cast<Peony::FileItemModel*>(proxyModel->sourceModel());
    if (!sourceModel)
        return device;

    for (const auto &proxyIndex : indexes) {
        auto sourceIndex = proxyModel->mapToSource(proxyIndex);
        auto item = sourceModel->itemFromIndex(sourceIndex);
        if (item && item->info()) {
            device = item->info()->unixDeviceFile();
            if (!device.isEmpty())
                return device;
        }
    }

    return device;
}

QMenu* ToolBarManager::createComputerMenu()
{
    qDebug()<<"current selections"<<m_selections<<m_selections.count()<<m_selections.isEmpty();
    QMenu* computerMenu = new QMenu(m_toolBarWidget);
    // 无选中项时显示"连接远程服务器"
    if (m_selections.isEmpty()) {
        bool allowSmbShare = Peony::GlobalSettings::getInstance()->isJsonCacheKey(ENABLE_SMB_SHARE)
            ? Peony::GlobalSettings::getInstance()->getValue(ENABLE_SMB_SHARE).toBool() : true;
        if (allowSmbShare) {
            computerMenu->addAction(QIcon::fromTheme("network-server-symbolic"), tr("Connect a server"), this, [this](){
            Peony::ConnectServerDialog dlg;
            auto code = dlg.exec();
            if (code == QDialog::Rejected) {
                return;
            }
            QString remoteUri = dlg.uri();
            if (!remoteUri.isEmpty()) {
                m_window->m_tab->goToUri(remoteUri, true, true);
            }
            });
        }
    } else if (m_selections.count() >= 1) {
        // 有选中项时显示设备相关菜单
        QString uri = m_selections.first();
        if (uri == "file:///data" && Peony::FileUtils::isFileExsit("file:///data/usershare")) {
            uri = "computer:///ukui-data-volume";
        }
        /* 计算机页选中手机等：列表选中项常为 mtp:// / gphoto2://，与侧边栏 computer:///…volume 统一 */
        if (uri.startsWith("mtp://") || uri.startsWith("gphoto2://")) {
            QString computerUri = getComputerUriFromUri(uri);
            if (!computerUri.isEmpty()) {
                uri = computerUri;
            }
        }

        // 获取文件信息
        auto info = FileInfo::fromUri(uri);
        QString unixDevice = info->unixDeviceFile();

        if (unixDevice.isEmpty()) {
            unixDevice = getSelectionUnixDevices();
        }
        if (uri == "file:///") {
            /* 文件系统特殊处理 */
            uri = "computer:///root.link";
        } else if (!unixDevice.isEmpty()) {
            /* 通过 unixDevice 获取 computer uri */
            QString computerUri = getComputerUriFromUnixDevice(unixDevice);
            if (!computerUri.isEmpty()) {
                uri = computerUri;
            } else {
                /* unixDevice不为空但computerUri为空的特殊情况 */
                if ("kyfs" == unixDevice) {
                    uri = getComputerUriFromUri(m_selections.first());
                } else {
                    unixDevice = getDeviceInfoFromUri(info->uri());
                    if (!unixDevice.isEmpty())
                        uri = getComputerUriFromUnixDevice(unixDevice);
                }
            }
        }
        qDebug()<<"computer unixDevice:"<<unixDevice<<" uri:"<<uri<<info->uri()<<getComputerUriFromUri(m_selections.first());

        info = FileInfo::fromUri(uri);
        if (info->displayName().isEmpty() || info->targetUri().isEmpty()) {
            FileInfoJob infoJob(info);
            infoJob.querySync();
        }

        QString targetUri = FileUtils::getTargetUri(info->uri());
        bool isMounted = FileUtils::isFileUnmountable(info->uri());
        if ("file:///" == uri || "file:///" == targetUri || "file:///data" == uri || "computer:///ukui-data-volume" == uri) {
            isMounted = true;
        }

        bool isReddisk = false;
        if (unixDevice.startsWith("/dev/dm") &&
            (QFile::exists("/opt/AQTJ/Client/JC/MAIN/bin/jc_main_ui") ||
             QFile::exists("/opt/apps/com.jc.bmt/files/MAIN/bin/jc_main_ui"))) {
            isReddisk = true;
        }

        bool isCloud = QFile::exists("/etc/ecloud") || QFile::exists("/usr/local/share/Ecloud");
        bool hideUnmount = false;
        bool hideFormat = false;
        if (isCloud) {
            hideFormat = true;
            if ( ! uri.startsWith("smb:///") &&
                 ! uri.startsWith("ftp:///") &&
                 ! uri.startsWith("sftp:///")) {
                 hideUnmount = true;
            }
        }

        // 新标签页打开
        if (!qApp->property("tabletMode").toBool()) {
            QAction *newTabAction = computerMenu->addAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open In New Tab"), this, [this, info](){
                QString targetUri = info->targetUri();
                if (targetUri.isEmpty() && "computer:///ukui-data-volume" == info->uri()) {
                    targetUri = "file:///data";
                    if (Peony::FileUtils::isFileExsit("file:///data/usershare"))
                        targetUri = "file:///data/usershare";
                }
                m_window->addNewTabs(QStringList() << targetUri);
            });
            newTabAction->setObjectName(OPEN_IN_NEW_TAB_ACTION);
            newTabAction->setShortcut(QKeySequence("Ctrl+t"));
            newTabAction->setEnabled(!uri.isNull() && isMounted);
        }

        // 新窗口打开
        QAction *newWindowAction = computerMenu->addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open In New Window"), this, [this, info](){
            QString targetUri = info->targetUri();
            if (targetUri.isEmpty() && "computer:///ukui-data-volume" == info->uri()) {
                targetUri = "file:///data";
                if (Peony::FileUtils::isFileExsit("file:///data/usershare"))
                    targetUri = "file:///data/usershare";
            }
            QProcess p;
            p.setProgram("/usr/bin/peony");
            QStringList args;
            args << targetUri;
            p.setArguments(args);
            p.startDetached(p.program(), p.arguments());
        });
        newWindowAction->setObjectName(OPEN_IN_NEW_WINDOW_ACTION);
        newWindowAction->setShortcut(QKeySequence("Ctrl+n"));
        newWindowAction->setEnabled(!uri.isNull() && isMounted);

        computerMenu->addSeparator();

        // 卸载
        bool canUnmount = info->canUnmount() && !info->property("isKydrive").toBool();
        if (!hideFormat && !isReddisk && !(info->canEject() || info->canStop())) {
            QAction *unmountAction = computerMenu->addAction(QIcon::fromTheme("user-trash-symbolic"), tr("Unmount"), this, [=](){
                std::shared_ptr<FileInfo> uinfo = FileInfo::fromUri(uri);
                if (uinfo && uinfo->targetUri().isEmpty()) {
                    FileInfoJob job(uri, this);
                    job.querySync();
                }

                QString dev = uinfo ? uinfo->unixDeviceFile() : QString();
                if (dev.isEmpty()) {
                    dev = unixDevice;
                }

                auto unmountByMountFromUri = [](const QString &u) -> bool {
                    if (u.isEmpty()) {
                        return false;
                    }
                    auto pmount = Peony::VolumeManager::getMountFromUri(u);
                    if (!pmount) {
                        return false;
                    }
                    GMount *gm = pmount->getGMount();
                    if (!gm || !g_mount_can_unmount(gm)) {
                        return false;
                    }
                    Experimental_Peony::Mount expMount(gm);
                    expMount.unmount();
                    return true;
                };

                auto tryUnmountByUri = [&]() {
                    if (uinfo && !uinfo->targetUri().isEmpty()) {
                        if (unmountByMountFromUri(uinfo->targetUri())) {
                            return;
                        }
                    }
                    unmountByMountFromUri(uri);
                };

                if (dev.isEmpty()) {
                    tryUnmountByUri();
                    return;
                }

                auto volumes = Experimental_Peony::VolumeManager::getInstance()->allVaildVolumes();
                bool matched = false;
                for (auto &volume : *volumes) {
                    if (volume.device() == dev) {
                        volume.unmount();
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    tryUnmountByUri();
                }
            });
            unmountAction->setEnabled(canUnmount);
        }

        // 弹出
        if (info->canEject() || info->canStop()) {
            QAction *ejectAction = computerMenu->addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Eject"), this, [unixDevice](){
                // 通过设备路径获取 Volume 并弹出
                auto volumes = Experimental_Peony::VolumeManager::getInstance()->allVaildVolumes();
                for (auto &volume : *volumes) {
                    if (volume.device() == unixDevice) {
                        volume.eject(G_MOUNT_UNMOUNT_NONE);
                        break;
                    }
                }
            });
            if (unixDevice.contains("/dev/sr")) {
                ejectAction->setDisabled(FileUtils::isBusyDevice(unixDevice));
            }
        }

        // fix bug#66471，66479 - not allow format data block
        auto mount = VolumeManager::getMountFromUri(info->targetUri());
        if (mount) {
            auto volume = VolumeManager::getVolumeFromMount(mount);
            if (mount && volume && !isReddisk && !hideFormat) {
                QString unixDevice = FileUtils::getUnixDevice(info->uri());
                if (!unixDevice.isNull() && unixDevice.contains("/dev/sr")
                    && !unixDevice.startsWith("/dev/bus/usb")
                    && info->isVolume()) {
                    // 光盘格式化 (UDF)
                    QAction *formatAction = computerMenu->addAction(QIcon::fromTheme("format-symbolic"), tr("Format"));
                    formatAction->setEnabled(false);
#ifndef KY_UDF_BURN
                    if (!FileUtils::isBusyDevice(unixDevice)) {
                        DiscControl *discControl = new DiscControl(unixDevice);
                        if (discControl->work()) {
                            connect(discControl, &DiscControl::workFinished, this, [=](DiscControl *discCtrl){
                                connect(formatAction, &QAction::triggered, this, [=](){
                                    UdfFormatDialog *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlg(uri, discCtrl);
                                    udfFormatDlg->show();
                                });
                                qDebug() << unixDevice << "supported Udf values:" << discCtrl->supportUdf();
                                formatAction->setEnabled(discCtrl->supportUdf());
                            });
                        }
                    }
#else
                    auto isUdfBusy = Peony::FileOperationManager::getInstance()->isUdfBurnRunning();
                    if (!FileUtils::isBusyDevice(unixDevice) && !isUdfBusy) {
                        UdfBurn::DiscControl *discControl = new UdfBurn::DiscControl(unixDevice);
                        if (discControl->work()) {
                            connect(discControl, &UdfBurn::DiscControl::workFinished, this, [=](UdfBurn::DiscControl *discCtrl){
                                connect(formatAction, &QAction::triggered, this, [=](){
                                    UdfBurn::UdfFormatDialogWrapper *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlgWrapper(uri, discCtrl);
                                    udfFormatDlg->show();
                                });
                                qDebug() << unixDevice << "supported Udf values:" << discCtrl->supportUdf();
                                formatAction->setEnabled(discCtrl->supportUdf());
                            });
                        }
                    }
#endif
                } else if (!unixDevice.isNull() && !unixDevice.contains("/dev/sr")
                           && !unixDevice.startsWith("/dev/bus/usb")
                           && !unixDevice.startsWith("/dev/loop")
                           && info->isVolume() && info->canUnmount()) {
                    // 其它格式化
                    bool allowFormat = true;
                    if (info->canEject() || info->canStop()) {
                        allowFormat = MANAGER_CONTROL->checkFormatRemovableDevicePermission();
                    }
                    if (allowFormat) {
                        QAction *formatAction = computerMenu->addAction(QIcon::fromTheme("format-symbolic"), tr("Format"), this, [=]() {
                            auto formatInfo = FileInfo::fromUri(uri);
                            if (formatInfo->targetUri().isEmpty()) {
                                FileInfoJob job(uri, this);
                                job.querySync();
                            }
                            Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
                            fd->show();
                        });
                        if (!mount) {
                            formatAction->setEnabled(false);
                        }
                    }
                }
            }
        } else {
            if (!unixDevice.isNull() && !unixDevice.contains("/dev/sr")
                && !unixDevice.startsWith("/dev/bus/usb")
                && !unixDevice.startsWith("/dev/loop")
                && info->isVolume() && !uri.isEmpty()
                && !isReddisk && !hideFormat && uri != "computer:///root.link") {
                bool allowFormat = true;
                if (info->canEject() || info->canStop()) {
                    allowFormat = MANAGER_CONTROL->checkFormatRemovableDevicePermission();
                }
                if (allowFormat) {
                    QAction *formatAction = computerMenu->addAction(QIcon::fromTheme("format-symbolic"), tr("Format"), this, [=]() {
                        auto formatInfo = FileInfo::fromUri(uri);
                        if (formatInfo->targetUri().isEmpty()) {
                            FileInfoJob job(uri, this);
                            job.querySync();
                        }
                        Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
                        fd->show();
                    });
                }
            }
        }

        QStringList fakeUrls;
        fakeUrls << uri;
        auto mgr = Peony::MenuPluginManager::getInstance();
        auto ids = mgr->getPluginIds();
        for (auto id : ids) {
            auto factory = mgr->getPlugin(id);
            if (!factory) continue;

            auto tmp = factory->menuActions(Peony::MenuPluginInterface::SideBar, uri, fakeUrls);
            for (auto action : tmp) {
                qDebug()<<"meun action"<<uri<<action->text();
                action->setParent(computerMenu);
                computerMenu->addAction(action);
                if (action->icon().isNull()) {
                    auto pluginicon = factory->icon();
                    if (!pluginicon.isNull()) {
                        action->setIcon(pluginicon);
                    }
                }
            }
        }

        computerMenu->addSeparator();

        // 属性
        if (!uri.startsWith("network://")) {
            QAction *propertyAction = computerMenu->addAction(QIcon::fromTheme("properties-symbolic"), tr("Property"), this, [uri](){
                if (uri.isNull()) {
                    QMessageBox::warning(0, 0, tr("You have to mount this volume first"));
                } else {
                    QProcess p;
                    p.setProgram("/usr/bin/peony");
                    QStringList args;
                    args << "-p" << uri;
                    p.setArguments(args);
                    p.startDetached(p.program(), p.arguments());
                }
            });
            propertyAction->setShortcut(QKeySequence("Alt+Enter"));
            propertyAction->setEnabled(!uri.isNull() && isMounted);
            if (unixDevice.contains("/dev/sr")) {
                propertyAction->setDisabled(FileUtils::isBusyDevice(unixDevice));
            }
        }
    }
    return computerMenu;
}

QMenu* ToolBarManager::createMoreMenu()
{
    QMenu* moreMenu = new Peony::DirectoryViewMenu(m_window, m_window);

    if (m_buttons.sortButton && m_buttons.sortButton->isEnabled() && !menuHasActionObjectName(moreMenu, QStringLiteral("sort-type-action"))) {
        insertMenuActionAtBeginning(moreMenu, createOverflowSortTypeAction(moreMenu));
    }

    QStringList basicActions;
    QStringList specicalActions = {"exit-edit-space-action", "exit-search-action","Ai-Question-Answering-action"};
    QMap<QString, QToolButton*> buttons = m_toolBarWidget->buttonMap();
    QMap<QString, QToolButton*>::iterator it;
    for (it = buttons.begin(); it != buttons.end(); ++it) {
        QToolButton* button = it.value();
        QString key = it.key();

        if (button) {
            if (button->isVisible()) {
                basicActions.append(button->objectName());
                qDebug() << "Button" << key << "is visible";
            } else if (specicalActions.contains(button->objectName()) && !m_toolBarWidget->m_hiddenButtons.contains(button)) {
                // 创建新的 QAction，而不是重用现有按钮
                QAction *menuAction = new QAction(button->icon(), button->text(), moreMenu);
                menuAction->setObjectName(button->objectName());
                menuAction->setEnabled(button->isEnabled());
                menuAction->setToolTip(button->toolTip());

                // 连接点击事件
                connect(menuAction, &QAction::triggered, this, [button]() {
                    if (button && button->isEnabled()) {
                        button->click();
                    }
                });

                // 插入到菜单开始位置
                if (!moreMenu->actions().isEmpty()) {
                    moreMenu->insertAction(moreMenu->actions().first(), menuAction);
                } else {
                    moreMenu->addAction(menuAction);
                }
            }
        }
    }

    QList<QAction*> allActions = moreMenu->actions();
    for (QAction *action : allActions) {
        if (basicActions.contains(action->objectName())) {
            moreMenu->removeAction(action);
        }
    }

    // 将因空间不足而隐藏的按钮添加到更多菜单（如果菜单中没有对应objectName的action）
    QMap<QString, QToolButton*> buttons2 = m_toolBarWidget->buttonMap();
    for (auto it2 = buttons2.begin(); it2 != buttons2.end(); ++it2) {
        QToolButton *button = it2.value();
        if (!button)
            continue;
        // 跳过可见的按钮（已在工具栏上显示）
        if (button->isVisible())
            continue;
        // 跳过被主动隐藏的按钮（如编辑空间等通过toggleButtons隐藏的）
        if (m_toolBarWidget->m_hiddenButtons.contains(button))
            continue;
        // 跳过更多按钮本身和预览按钮
        if (button->objectName() == "more-action" || button->objectName() == "preview-action")
            continue;
        // 跳过已禁用的按钮
        if (!button->isEnabled())
            continue;
        // 如果菜单中已有相同objectName的action，则不添加
        if (menuHasActionObjectName(moreMenu, button->objectName()))
            continue;

        // 创建对应的菜单action
        QAction *overflowAction = new QAction(button->icon(), button->text().isEmpty() ? button->toolTip() : button->text(), moreMenu);
        overflowAction->setObjectName(button->objectName());
        overflowAction->setEnabled(button->isEnabled());
        overflowAction->setToolTip(button->toolTip());
        connect(overflowAction, &QAction::triggered, this, [button]() {
            if (button && button->isEnabled()) {
                button->click();
            }
        });
        moreMenu->addAction(overflowAction);
    }

    // 如果退出编辑空间存在，则移除编辑空间
    bool hasExitEditSpace = false;
    // 检查工具栏中退出编辑空间是否可见
    if (m_buttons.exitEditSpaceButton && m_buttons.exitEditSpaceButton->isVisible()) {
        hasExitEditSpace = true;
    }
    // 检查退出编辑空间是否应该显示在menu中
    if (!hasExitEditSpace && m_buttons.exitEditSpaceButton && !m_toolBarWidget->m_hiddenButtons.contains(m_buttons.exitEditSpaceButton)) {
        hasExitEditSpace = true;
    }
    if (hasExitEditSpace) {
        for (QAction *action : moreMenu->actions()) {
            if (action->objectName() == "edit-space-action") {
                moreMenu->removeAction(action);
                break;
            }
        }
    }

    return moreMenu;
}

bool ToolBarManager::menuHasActionObjectName(QMenu *menu, const QString &objectName) const
{
    if (!menu || objectName.isEmpty()) {
        return false;
    }

    for (QAction *action : menu->actions()) {
        if (action && action->objectName() == objectName) {
            return true;
        }
    }

    return false;
}

void ToolBarManager::insertMenuActionAtBeginning(QMenu *menu, QAction *action)
{
    if (!menu || !action) {
        return;
    }

    if (!menu->actions().isEmpty()) {
        menu->insertAction(menu->actions().first(), action);
    } else {
        menu->addAction(action);
    }
}

QAction *ToolBarManager::createOverflowSortTypeAction(QMenu *parentMenu)
{
    if (!parentMenu || !m_sortTypeMenuWidget || !m_window) {
        return nullptr;
    }

    auto sortAction = new QAction(QIcon::fromTheme("ukui-view-sort-letter-symbolic"), tr("Sort By"), parentMenu);
    sortAction->setObjectName("sort-type-action");

    QMenu *sortSubMenu = new QMenu(parentMenu);
    QList<QAction *> sortActions;
    QPixmap emptyPixmap(0, 0);
    sortActions << sortSubMenu->addAction(QIcon(emptyPixmap), tr("Name"));
    sortActions << sortSubMenu->addAction(QIcon(emptyPixmap), tr("Time Modified"));
    sortActions << sortSubMenu->addAction(tr("File Type"));
    sortActions << sortSubMenu->addAction(tr("File Size"));
    sortActions << sortSubMenu->addAction(tr("Original Path"));
    sortSubMenu->setStyle(new MenuStyle(sortSubMenu->style()));
    if (m_directory != QStringLiteral("trash:///")) {
        sortActions.last()->setVisible(false);
    }

    int currentSortType = m_window->getCurrentSortColumn();
    if (currentSortType >= 0 && currentSortType < sortActions.count()) {
        sortActions.at(currentSortType)->setCheckable(true);
        sortActions.at(currentSortType)->setChecked(true);
    }

    for (int i = 0; i < sortActions.count(); ++i) {
        connect(sortActions.at(i), &QAction::triggered, this, [this, i]() {
            if (m_window) {
                m_window->setCurrentSortColumn(i);
            }
        });
    }

    QStringList sortNames;
    sortNames << tr("Descending Order") << tr("Ascending Order");
    if (currentSortType == 1) {
        sortNames.clear();
        sortNames << tr("Newest to oldest") << tr("Oldest to newest");
    } else if (currentSortType == 3) {
        sortNames.clear();
        sortNames << tr("Files from large to small") << tr("Files from small to large");
    }

    sortSubMenu->addSeparator();
    QList<QAction *> orderActions;
    orderActions << sortSubMenu->addAction(QIcon::fromTheme("view-sort-descending-symbolic"), sortNames.at(0));
    orderActions << sortSubMenu->addAction(QIcon::fromTheme("view-sort-ascending-symbolic"), sortNames.at(1));
    int currentSortOrder = m_window->getCurrentSortOrder();
    if (currentSortOrder >= 0 && currentSortOrder < orderActions.count()) {
        orderActions.at(currentSortOrder)->setCheckable(true);
        orderActions.at(currentSortOrder)->setChecked(true);
    }
    for (int i = 0; i < orderActions.count(); ++i) {
        connect(orderActions.at(i), &QAction::triggered, this, [this, i]() {
            if (m_window) {
                m_window->setCurrentSortOrder(Qt::SortOrder(i));
            }
        });
    }

    sortAction->setMenu(sortSubMenu);
    sortAction->setEnabled(!m_is_computer);
    return sortAction;
}
/**
 * @brief 从文件信息获取Unix设备标识和计算机URI
 * @param info 文件信息对象
 * @param unixDevice 输出参数：返回的Unix设备标识
 * @return 计算机URI，失败返回空字符串
 */
QString ToolBarManager::getDeviceInfoFromUri(const QString& uri)
{
    // 获取目标URI
    QString targetUri = FileUtils::getTargetUri(uri);
    qDebug() << "Target URI:" << targetUri;

    if (targetUri.isEmpty()) {
        qWarning() << "Target URI is empty";
        return QString();
    }

    // 获取Volume
    auto volume = Peony::VolumeManager::getVolumeFromUri(targetUri.toUtf8().constData());
    if (!volume) {
        qWarning() << "Failed to get volume from URI:" << targetUri;
        return QString();
    }

    // 获取GVolume
    auto gvolume = volume->getGVolume();
    if (!gvolume) {
        qWarning() << "Failed to get GVolume";
        return QString();
    }

    // 获取Unix设备标识
    g_autofree gchar *unix_device = g_volume_get_identifier(gvolume,
                                                             G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if (!unix_device) {
        qWarning() << "Failed to get Unix device identifier";
        return QString();
    }

    QString unixDevice = QString::fromUtf8(unix_device);

    return unixDevice;
}

// 更新全选按钮状态
void ToolBarManager::updateSelectAllButton(bool autoUpdate)
{
    if (autoUpdate) {
        // 通过MainWindow获取当前视图
        auto page = m_window->getCurrentPage();
        if (!page)
            return;
        auto view = page->getView();
        if (view) {
            int num = view->getSelections().count();
            int totalNum = view->getAllFileUris().count();
            m_isSelectAll = (num == totalNum && num > 0);
        }
    }

    if (m_isSelectAll) {
        m_buttons.selectAllButton->setText(tr("Deselect All"));  // 取消全选
    } else {
        m_buttons.selectAllButton->setText(tr("Select All"));  // 全选
    }
}

// 全选：仅操作选中项，与多选模式无关；表头复选框随选中状态联动
void ToolBarManager::onSelectAllButtonClicked()
{
    auto page = m_window->getCurrentPage();
    if (!page)
        return;
    auto view = page->getView();
    if (!view)
        return;

    if (m_isSelectAll) {
        view->invertSelections();
        m_isSelectAll = false;
        m_buttons.selectAllButton->setText(tr("Select All"));

    } else {
        view->setSelections(QStringList());
        view->invertSelections();
        m_isSelectAll = true;
        m_buttons.selectAllButton->setText(tr("Deselect All"));
        //m_toolBarWidget->m_hiddenButtons.remove(m_buttons.selectAllButton);
    }
    m_buttons.selectAllButton->setToolTip(m_buttons.selectAllButton->text());
}

// 启用/禁用图标视图多选模式
void ToolBarManager::enableIconViewMultiSelect(bool enable)
{
    m_isMultiSelect = enable;
    auto page = m_window->getCurrentPage();
    if (!page)
        return;
    auto view = page->getView();
    if (!view)
        return;

    // 使用 DirectoryViewHelper 获取接口并调用多选方法
    auto *iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
    if (iface2) {
        iface2->doMultiSelect(enable);
    }
}

// 启用/禁用列表视图多选模式
void ToolBarManager::enableListViewMultiSelect(bool enable)
{
    m_isMultiSelect = enable;
    auto page = m_window->getCurrentPage();
    if (!page)
        return;
    auto view = page->getView();
    if (!view)
        return;

    // 使用 DirectoryViewHelper 获取接口并调用多选方法
    auto *iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
    if (iface2) {
        iface2->doMultiSelect(enable);
    }
}

// 多选按钮点击处理
void ToolBarManager::onMultiSelectButtonClicked()
{
    auto page = m_window->getCurrentPage();
    if (!page)
        return;
    auto view = page->getView();
    if (!view)
        return;

    // 切换多选状态
    m_isMultiSelect = !m_isMultiSelect;

    // 使用 DirectoryViewHelper 获取接口并调用多选方法
    auto *iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
    if (iface2) {
        iface2->doMultiSelect(m_isMultiSelect);
    }

    updateMultiSelectButton();
}

// 更新多选按钮状态
void ToolBarManager::updateMultiSelectButton()
{
    if (m_isMultiSelect) {
        m_buttons.multiSelectButton->setText(tr("Exit MultiSelect"));
        // 进入多选模式时显示全选按钮
        m_toolBarWidget->m_hiddenButtons.remove(m_buttons.selectAllButton);
        m_buttons.selectAllButton->setVisible(true);
    } else {
        m_buttons.multiSelectButton->setText(tr("MultiSelect"));
        // 退出多选模式时隐藏全选按钮
        m_buttons.selectAllButton->setVisible(false);
        m_toolBarWidget->m_hiddenButtons.insert(m_buttons.selectAllButton);
    }
    m_buttons.multiSelectButton->setToolTip(m_buttons.multiSelectButton->text());
}
