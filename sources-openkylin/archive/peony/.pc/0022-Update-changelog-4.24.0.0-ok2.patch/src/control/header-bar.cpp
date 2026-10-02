/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "header-bar.h"
#include "main-window.h"
#include <gio/gio.h>

#include "view-type-menu.h"
#include "sort-type-menu.h"
#include "operation-menu.h"

#include "directory-view-container.h"
#include "directory-view-widget.h"
#include "advanced-location-bar.h"

#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface2.h"
#include "search-vfs-uri-parser.h"
#include "file-info.h"
#include "file-info-job.h"
#include "file-utils.h"
#include "tab-widget.h"
#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"

#include "clipboard-utils.h"
#include "file-operation-utils.h"
#include "directoryviewhelper.h"

#include <QHBoxLayout>
#include <QUrl>
#include <QMessageBox>
#include <QProcess>
#include <QDir>

#include <QStyleOptionToolButton>

#include <QEvent>
#include <QApplication>
#include <QTimer>
#include <QStandardPaths>

#include <KWindowSystem>
#include "global-settings.h"
#ifdef KYLIN_COMMON
#include <ukuisdk/kylin-com4cxx.h>
#endif

#include <QtConcurrent>
#include <QAction>

#include <QX11Info>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QPainter>

#include <QDebug>
#include <QFileDialog>

#include "file-utils.h"
#include "tab-widget.h"

#include "search-widget.h"
#ifdef KY_SDK_WAYLANDHELPER
#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#endif
#include "convenient-macros.h"
#define DBUS_STATUS_MANAGER_IF "com.kylin.statusmanager.interface"

static HeaderBarStyle *global_instance = nullptr;
static QString terminal_cmd = nullptr;
static QDBusInterface *g_statusManagerDBus = nullptr;

HeaderBar::HeaderBar(MainWindow *parent) : QToolBar(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents);

    setMouseTracking(true);
    setStyle(HeaderBarStyle::getStyle());
    setFocusPolicy(Qt::TabFocus);

    m_window = parent;
    //disable default menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    //setAttribute(Qt::WA_OpaquePaintEvent);
    setStyleSheet(".HeaderBar{"
                  "background-color: transparent;"
                  "border: 0px solid transparent;"
                  "margin: 4px 5px 4px 5px;"
                  "}");

    setMovable(false);

//    auto a = addAction(QIcon::fromTheme("folder-new-symbolic"), tr("Create Folder"), [=]() {
//        //use the same function
//        m_window->createFolderOperation();
//    });
//    auto createFolder = qobject_cast<QToolButton *>(widgetForAction(a));
//    createFolder->setAutoRaise(false);
//    createFolder->setFixedSize(QSize(40, 40));
//    createFolder->setIconSize(QSize(16, 16));

//    addSpacing(2);

    //find a terminal when init
//    findDefaultTerminal();
//    a = addAction(QIcon::fromTheme("terminal-app-symbolic"), tr("Open Terminal"), [=]() {
//        //open the default terminal
//        openDefaultTerminal();
//    });
//    auto openTerminal = qobject_cast<QToolButton *>(widgetForAction(a));
//    openTerminal->setAutoRaise(false);
//    openTerminal->setFixedSize(QSize(40, 40));
//    openTerminal->setIconSize(QSize(16, 16));

    auto goBack = new QToolButton(this);
    m_go_back = goBack;
    goBack->setEnabled(false);
    goBack->setToolTip(tr("Go Back"));
    goBack->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    auto a = addWidget(goBack);
    m_actions.insert(HeaderBarAction::GoBack, a);

    auto goForward = new QToolButton(this);
    m_go_forward = goForward;
    goForward->setEnabled(false);
    goForward->setToolTip(tr("Go Forward"));
    goForward->setIcon(QIcon::fromTheme("go-next-symbolic"));
    a = addWidget(goForward);
    m_actions.insert(HeaderBarAction::GoForward, a);
    connect(goForward, &QPushButton::clicked, m_window, [=]() {
        m_window->getCurrentPage()->goForward();
    });
#ifdef KYLIN_COMMON
    m_is_intel = (QString::compare("V10SP1-edu", QString::fromStdString(KDKGetPrjCodeName()), Qt::CaseInsensitive) == 0);
#else
    m_is_intel = false;
#endif // KYLIN_COMMON

    if (! m_is_intel)
    {
        //non intel project, show go up button
        auto goUp = new QToolButton(this);
        m_go_up = goUp;
        goUp->setEnabled(true);
        goUp->setToolTip(tr("Go Up"));
        goUp->setIcon(QIcon::fromTheme("go-up-symbolic"));
        a = addWidget(goUp);
        m_actions.insert(HeaderBarAction::GoForward, a);
        connect(goUp, &QPushButton::clicked, m_window, [=]() {
            m_window->getCurrentPage()->cdUp();
        });
    }
    addSpacing(9);

    //task#10993 实现文档管理器路径模式与搜索模式切换动画
    m_searchWidget = new Peony::SearchWidget(this);
    a = addWidget(m_searchWidget);
    m_actions.insert(HeaderBarAction::LocationBar, a);

    connect(this, &HeaderBar::updateSearchProgress, m_searchWidget, &Peony::SearchWidget::updateSearchProgress);
    connect(goBack, &QPushButton::clicked, m_window, [=]() {
        m_window->getCurrentPage()->goBack();
        Q_EMIT m_searchWidget->clearSearchBox();

    });

    connect(m_searchWidget, &Peony::SearchWidget::refreshRequest, [=]() {
        m_window->updateTabPageTitle();
    });
    connect(m_searchWidget, &Peony::SearchWidget::updateFileTypeFilter, [=](const int &index) {
        m_window->getCurrentPage()->setSortFilter(index);
    });

    connect(this, &HeaderBar::setGlobalFlag, m_searchWidget, &Peony::SearchWidget::setGlobalFlag );
    connect(this, &HeaderBar::updateSearchRecursive, m_searchWidget, &Peony::SearchWidget::updateSearchRecursive);
    connect(this, &HeaderBar::setLocation, m_searchWidget, &Peony::SearchWidget::updateLocation);
    connect(this, &HeaderBar::cancelEdit, m_searchWidget, &Peony::SearchWidget::cancelEdit);
    connect(this, &HeaderBar::startEdit, m_searchWidget, &Peony::SearchWidget::startEdit);
    connect(this, &HeaderBar::finishEdit, m_searchWidget, &Peony::SearchWidget::finishEdit);
    //connect(m_searchWidget, &Peony::SearchWidget::updateSearchRequest, this, &HeaderBar::updateSearchRequest);
    connect(m_searchWidget, &Peony::SearchWidget::updateLocationRequest, this, &HeaderBar::updateLocationRequest);
    connect(this, &HeaderBar::setLocation, this, &HeaderBar::quitMultiSelect);
    connect(this, &HeaderBar::setSearchBarFocus, m_searchWidget,&Peony::SearchWidget::setSearchBarFocus);
    addSpacing(2);
    //task#106007 【文件管理器】文件管理器应用做平板UI适配，增加多选模式
    addTabletMenu();
    addSpacing(2);

//    auto iconView = addAction(QIcon::fromTheme("view-grid-symbolic"), tr("Icon View"));
//    iconView->setData("Icon View");
//    m_actions.insert(HeaderBarAction::IconView, iconView);
//    iconView->setCheckable(true);
//    auto iconViewButton = qobject_cast<QToolButton *>(widgetForAction(iconView));
//    iconViewButton->setIconSize(QSize(16, 16));
//    iconViewButton->setProperty("isWindowButton", 1);
//    iconViewButton->setProperty("fillIconSymbolicColor", true);
//    iconViewButton->setFocusPolicy(Qt::StrongFocus);

//    auto listView = addAction(QIcon::fromTheme("view-list-symbolic"), tr("List View"));
//    listView->setData("List View");
//    m_actions.insert(HeaderBarAction::ListView, listView);
//    listView->setCheckable(true);
//    auto listViewButton = qobject_cast<QToolButton *>(widgetForAction(listView));
//    listViewButton->setIconSize(QSize(16, 16));
//    listViewButton->setProperty("isWindowButton", 1);
//    listViewButton->setProperty("fillIconSymbolicColor", true);
//    listViewButton->setFocusPolicy(Qt::StrongFocus);

//    m_view_actions = new QActionGroup(this);
//    m_view_actions->setExclusive(true);
//    m_view_actions->addAction(iconView);
//    m_view_actions->addAction(listView);
//    connect(m_view_actions, &QActionGroup::triggered, this, [=](QAction *action) {
//        auto viewId = action->data().toString();
//        bool isSearchPage = m_window->getCurrentUri().startsWith("search:///");
//        if(isSearchPage){
//            return;
//        }
//        m_window->beginSwitchView(viewId);
//    });

//    a = addAction(QIcon::fromTheme("view-grid-symbolic"), tr("View Type"));
//    m_actions.insert(HeaderBarAction::ViewType, a);
//    auto viewType = qobject_cast<QToolButton *>(widgetForAction(a));
//    viewType->setAutoRaise(false);
//    viewType->setFixedWidth(57);
//    viewType->setIconSize(QSize(16, 16));
//    viewType->setPopupMode(QToolButton::InstantPopup);
//    viewType->setFocusPolicy(Qt::StrongFocus);

//    m_view_type_menu = new ViewTypeMenu(viewType);

//    //fix bug#128963, QToolButton not update status issue
//    connect(m_view_type_menu, &QMenu::aboutToHide, this, [=](){
//        viewType->setAttribute(Qt::WA_UnderMouse, false);
//        viewType->setDown(false);
//        viewType->releaseMouse();
//        viewType->update();
//    });

//    a->setMenu(m_view_type_menu);

//    connect(m_view_type_menu, &ViewTypeMenu::switchViewRequest, this, [=](const QString &id, const QIcon &icon, bool resetToZoomLevel) {
//        viewType->setText(id);
//        viewType->setIcon(icon);
//        this->viewTypeChangeRequest(id);
//        if (resetToZoomLevel) {
//            auto viewId = m_window->getCurrentPage()->getView()->viewId();
//            auto factoryManger = Peony::DirectoryViewFactoryManager2::getInstance();
//            auto factory = factoryManger->getFactory(viewId);
//            int zoomLevelHint = factory->zoom_level_hint();
//            m_window->getCurrentPage()->setZoomLevelRequest(zoomLevelHint);
//        }
//    });

//    connect(m_view_type_menu, &ViewTypeMenu::updateZoomLevelHintRequest, this, &HeaderBar::updateZoomLevelHintRequest);

//    addVerticalSeparator(9, HeaderBarAction::SortType);

//    a = addAction(QIcon::fromTheme("view-sort-ascending-symbolic"), tr("Sort Type"));
//    m_actions.insert(HeaderBarAction::SortType, a);
//    auto sortType = qobject_cast<QToolButton *>(widgetForAction(a));
//    sortType->setIconSize(QSize(16, 16));
//    sortType->setPopupMode(QToolButton::InstantPopup);
//    sortType->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    sortType->setFocusPolicy(Qt::StrongFocus);

//    m_sort_type_menu = new SortTypeMenu(this);
//    a->setMenu(m_sort_type_menu);

//    updateSortButton();

//    connect(m_sort_type_menu, &SortTypeMenu::switchSortTypeRequest, m_window, &MainWindow::setCurrentSortColumn);
//    connect(m_sort_type_menu, &SortTypeMenu::switchSortOrderRequest, m_window, [=](Qt::SortOrder order) {
//        if (order == Qt::AscendingOrder) {
//            sortType->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
//        } else {
//            sortType->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
//        }
//        m_window->setCurrentSortOrder(order);
//    });
//    connect(m_sort_type_menu, &QMenu::aboutToShow, m_sort_type_menu, [=]() {
//        bool originPathVisible = m_window->getCurrentUri() == "trash:///";
//        m_sort_type_menu->setOriginPathVisible(originPathVisible);
//        bool isSearchTab = m_window->getCurrentUri().startsWith("search:///");
//        m_sort_type_menu->setFilePathVisible(false);

//        bool isIDMTab = m_window->getCurrentUri().startsWith("idm:///");
//        m_sort_type_menu->setUserAddedEnable(isIDMTab);

//        if (!isIDMTab && m_sort_type_menu->isAutoModeType(m_window->getCurrentSortColumn())) {
//            m_window->setCurrentSortColumn(0);
//        }

//        m_sort_type_menu->setSortType(m_window->getCurrentSortColumn());
//        m_sort_type_menu->setSortOrder(m_window->getCurrentSortOrder());
//    });

//    //fix bug#128963, QToolButton not update status issue
//    connect(m_sort_type_menu, &QMenu::aboutToHide, this, [=](){
//        sortType->setAttribute(Qt::WA_UnderMouse, false);
//        sortType->setDown(false);
//        sortType->releaseMouse();
//        sortType->update();
//    });
//    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [=](const QString &key){
//        if (SORT_COLUMN == key || key == USE_GLOBAL_DEFAULT_SORTING) {
//            updateSortButton();
//        }
//    });
//    if (QGSettings::isSchemaInstalled("org.ukui.style"))
//    {
//        //font monitor
//        QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
//        connect(fontSetting, &QGSettings::changed, this, [=](const QString &key){
//            qDebug() << "fontSetting changed:" << key;
//            if (key == "systemFont" || key == "systemFontSize") {
//                updateSortButton();
//            }
//        });
//    }

//    addVerticalSeparator(9, HeaderBarAction::SortType);

//    auto manager = Peony::PreviewPageFactoryManager::getInstance();
//    auto pluginNames = manager->getPluginNames();
//    for (auto name : pluginNames) {
//        auto factory = manager->getPlugin(name);
//        m_preview_action = addAction(factory->icon(), factory->name());
//        break;
//    }

//    m_preview_action->setCheckable(true);
//    auto previewBtnt = qobject_cast<QToolButton *>(widgetForAction(m_preview_action));
//    previewBtnt->setFocusPolicy(Qt::TabFocus);
//    connect(m_preview_action,&QAction::triggered,[=](bool checked){
//        m_window->m_tab->setTriggeredPreviewPage(checked);
//        for (auto name : pluginNames) {
//            if (checked) {
//                auto plugin = Peony::PreviewPageFactoryManager::getInstance()->getPlugin(name);
//                m_window->m_tab->setPreviewPage(plugin->createPreviewPage());
//            } else {
//                m_window->m_tab->setPreviewPage(nullptr);
//            }
//        }
//    });

//    auto check = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_DETAIL).toBool();
//    m_window->m_tab->setTriggeredPreviewPage(check);
//    m_preview_action->setChecked(check);

//    connect(m_sort_type_menu, &SortTypeMenu::switchSortTypeRequest, this, [=](int type){
//        QString fullText = m_sort_type_menu->getSortTypeName(type);
//        QFontMetrics fm(sortType->font());
//        int availableWidth = sortType->width() - sortType->iconSize().width() - 50;
//        QString elidedText = fm.elidedText(fullText, Qt::ElideRight, availableWidth);
//        a->setText(elidedText);
//        a->setToolTip(fullText);
//        m_sort_type_menu->updateSortOrderName(type);
//    });

//    addSpacing(3);

    a = addAction(QIcon::fromTheme("open-menu-symbolic"), tr("Option"));
    m_actions.insert(HeaderBarAction::Option, a);
    QToolButton *optionButton = qobject_cast<QToolButton *>(widgetForAction(a));
    optionButton->setAutoRaise(false);
    optionButton->setIconSize(QSize(16, 16));
    optionButton->setPopupMode(QToolButton::InstantPopup);
    optionButton->setProperty("isOptionButton", true);
    optionButton->setProperty("isWindowButton", 1);

    OperationMenu *operationMenu = new OperationMenu(m_window,m_window);
    a->setMenu(operationMenu);

    // 更新视图选择menu相关-----------<疑问>
    m_operation_menu = operationMenu;
    connect(m_operation_menu, &OperationMenu::globalViewPolicyChanged, this, [=](){
        if(!m_window)
            return;
        int zoomLevel = Peony::FileUtils::getViewZoomLevel(m_window->getCurrentUri());
        QString  viewType = Peony::DirectoryViewFactoryManager2::getInstance()->getDefaultViewId(m_window->getCurrentUri());
        if(m_window->getCurrentPage()){
            m_window->getCurrentPage()->switchViewType(viewType);
            Q_EMIT m_window->getCurrentPage()->updateStatusBarSliderStateRequest();//update status bar zoom level
        }
    });

    // Add by wnn, add tool button when select item
    a = addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("&Copy"));
    m_actions.insert(HeaderBarAction::Copy, a);
    a->setVisible(false);
    a->setToolTip(tr("Copy"));
    auto copy = qobject_cast<QToolButton *>(widgetForAction(a));
    connect(a, &QAction::triggered, [=]() {
        if (!m_window->getCurrentSelections().isEmpty()) {
            if (m_window->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
                return ;
            }

            Peony::ClipboardUtils::setClipboardFiles(m_window->getCurrentSelections(), false);
        }
    });

    a = addAction(QIcon::fromTheme("edit-cut-symbolic"), tr("&Cut"));
    m_actions.insert(HeaderBarAction::Cut, a);
    a->setVisible(false);
    a->setToolTip(tr("Cut"));
    auto cut = qobject_cast<QToolButton *>(widgetForAction(a));
    connect(a, &QAction::triggered, [=]() {
        Peony::ClipboardUtils::setClipboardFiles(m_window->getCurrentSelections(), true);
    });

    a = addAction(tr("&Select All"));
    a->setIcon(QIcon::fromTheme("edit-select-all-symbolic"));
    m_actions.insert(HeaderBarAction::SeletcAll, a);
    a->setVisible(false);
    a->setToolTip(tr("Select All"));
    auto select = qobject_cast<QToolButton *>(widgetForAction(a));
    connect(a, &QAction::triggered, [=]() {
        if (m_window->getCurrentPage()->getView())
        {
            /// note: 通过getAllFileUris设置的全选效率过低，如果增加接口则会导致二进制兼容性问题
            /// 所以这里使用现有的反选接口实现高效的全选，这个方法在mainwindow中也有用到
            //auto allFiles = this->getCurrentPage()->getView()->getAllFileUris();
            //this->getCurrentPage()->getView()->setSelections(allFiles);
            if (m_isSelectAll) {
                m_window->getCurrentPage()->getView()->invertSelections();
                m_isSelectAll = false;
                select->setText(tr("Select All"));
            } else {
                m_window->getCurrentPage()->getView()->setSelections(QStringList());
                m_window->getCurrentPage()->getView()->invertSelections();
                m_isSelectAll = true;
                select->setText(tr("Deselect All"));
            }
        }
    });

    a = addAction(QIcon::fromTheme("edit-delete-symbolic"), tr("&Delete to trash"));
    m_actions.insert(HeaderBarAction::Delete, a);
    a->setVisible(false);
    a->setToolTip(tr("Delete to trash"));
    auto trash = qobject_cast<QToolButton *>(widgetForAction(a));
    connect(a, &QAction::triggered, [=]() {
        if (m_window->getCurrentUri() == "trash:///") {
            Peony::FileOperationUtils::executeRemoveActionWithDialog(m_window->getCurrentSelections());
        } else {
            Peony::FileOperationUtils::trash(m_window->getCurrentSelections(), true);
        }
    });

    for (auto action : actions()) {
        auto w = widgetForAction(action);
        w->setProperty("iconHighlightEffectMode", 1);
        w->setProperty("useIconHighlightEffect", true);
    }
}

HeaderBar::~HeaderBar()
{
//    bool check = m_preview_action->isChecked();
//    Peony::GlobalSettings::getInstance()->setValue(DEFAULT_DETAIL, check);
}

void HeaderBar::findDefaultTerminal()
{
    QtConcurrent::run([](){
        GList *infos = g_app_info_get_all();
        GList *l = infos;
        while (l) {
            const char *cmd = g_app_info_get_executable(static_cast<GAppInfo*>(l->data));
            QString tmp = cmd;
            if (tmp.contains("terminal")) {
                terminal_cmd = tmp;
                if (tmp == "mate-terminal") {
                    terminal_cmd = "/usr/bin/mate-terminal";
                    break;
                }
            }
            l = l->next;
        }
        g_list_free_full(infos, g_object_unref);
    });
}

void HeaderBar::openDefaultTerminal()
{
    //don't find any terminal
    if (terminal_cmd == nullptr)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle(tr("Operate Tips"));
        msgBox->setText(tr("Don't find any terminal, please install at least one terminal!"));
        msgBox->exec();
        return;
    }

    QUrl url = m_window->getCurrentUri();
    auto directory = url.path().toUtf8().constData();
    gchar **argv = nullptr;
    g_shell_parse_argv (terminal_cmd.toUtf8().constData(), nullptr, &argv, nullptr);
    GError *err = nullptr;
    g_spawn_async (directory,
                   argv,
                   nullptr,
                   G_SPAWN_SEARCH_PATH,
                   nullptr,
                   nullptr,
                   nullptr,
                   &err);
    if (err) {
        qDebug()<<err->message;
        g_error_free(err);
        err = nullptr;
        tryOpenAgain();
    }
    g_strfreev (argv);
}

void HeaderBar::tryOpenAgain()
{
    QUrl url = m_window->getCurrentUri();
    auto absPath = url.path();
    qDebug() << "tryOpenAgain url" <<url<<absPath<<terminal_cmd;
    QProcess p;
    p.setProgram(terminal_cmd);
    p.setArguments(QStringList()<<"--working-directory"<<absPath);
    p.startDetached(p.program(), p.arguments());
    p.waitForFinished(-1);
}

void HeaderBar::switchSelectStatus(bool select)
{
    if (m_tablet_mode) {
        // fixme: 没有实现directoryviewiface2接口的view不应该显示全选之类的选项
        //task#106007 【文件管理器】文件管理器应用做平板UI适配，增加多选模式
        updateSelectStatus(true);
        return;
    }

    if (m_is_intel && select) {
//        m_actions.find(HeaderBarAction::SortType).value()->setVisible(false);
//        m_actions.find(HeaderBarAction::ViewType).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Copy).value()->setVisible(true);
        m_actions.find(HeaderBarAction::Cut).value()->setVisible(true);
        m_actions.find(HeaderBarAction::SeletcAll).value()->setVisible(true);
        m_actions.find(HeaderBarAction::Delete).value()->setVisible(true);
    }
    else {
//        m_actions.find(HeaderBarAction::SortType).value()->setVisible(true);
//        m_actions.find(HeaderBarAction::ViewType).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Copy).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Cut).value()->setVisible(false);
        m_actions.find(HeaderBarAction::SeletcAll).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Delete).value()->setVisible(false);
    }
    //fix bug#100105 After the selected status changes, the view type is grayed out.
    updateViewTypeEnable();
    updateSortTypeEnable();
}

void HeaderBar::addSpacing(int pixel)
{
    for (int i = 0; i < pixel; i++) {
        addSeparator();
    }
}

void HeaderBar::addVerticalSeparator(int width, HeaderBarAction followAction)
{
    VerticalSeparator *spacer = new VerticalSeparator(this);
    spacer->setFixedWidth(width);
    spacer->setLineVisible(true);
    spacer->setProperty("followAction", static_cast<int>(followAction));
    this->addWidget(spacer);
}

void HeaderBar::updateSeparatorVisibility()
{
    // 遍历所有子控件，找到 VerticalSeparator
    for (QWidget *child : findChildren<QWidget*>()) {
        if (VerticalSeparator *separator = qobject_cast<VerticalSeparator*>(child)) {
            int followAction = separator->property("followAction").toInt();
            if (followAction > 0) {
                HeaderBarAction action = static_cast<HeaderBarAction>(followAction);
                auto it = m_actions.find(action);
                if (it != m_actions.end() && it.value()) {
                    // 如果关联的动作有效且可见，则显示分隔符
                    bool actionVisible = it.value()->isVisible();
                    separator->setLineVisible(actionVisible);
                    separator->setVisible(!actionVisible);
                } else {
                    // 如果关联的动作无效，则隐藏分隔符
                    separator->hide();
                }
            }
        }
    }
}

void HeaderBar::mouseMoveEvent(QMouseEvent *e)
{
    QToolBar::mouseMoveEvent(e);
    QCursor c;
    c.setShape(Qt::ArrowCursor);
    this->topLevelWidget()->setCursor(c);
}

//fix Right click in the blank space on the right side of the tab to quickly maximize and restore the window. link bug102455
void HeaderBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    QToolBar::mouseDoubleClickEvent(e);
    //fix bug#196512, RightButton double click in buttons can also maximizeOrRestore window issue
    // if(e->button() == Qt::LeftButton/* || e->button() == Qt::RightButton*/){
    //     m_window->maximizeOrRestore();
    // }
}

void HeaderBar::updatePreviewPageVisible()
{
//    auto currentUri = m_window->getCurrentUri();
//    if (currentUri == "computer:///") {
//        m_preview_action->setVisible(false);
//    } else {
//        m_preview_action->setVisible(true);
//    }

//    auto manager = Peony::PreviewPageFactoryManager::getInstance();
//    auto pluginNames = manager->getPluginNames();
//    for (auto name : pluginNames) {
//        if (!m_tablet_mode && m_preview_action->isChecked() && m_preview_action->isVisible()) {
//            auto plugin = Peony::PreviewPageFactoryManager::getInstance()->getPlugin(name);
//            m_window->m_tab->setPreviewPage(plugin->createPreviewPage());
//        } else {
//            m_window->m_tab->setPreviewPage(nullptr);
//        }
//    }
}

void HeaderBar::finishEdit()
{
    m_searchWidget->finishEdit();
}

void HeaderBar::quitSerachMode()
{
    if (m_searchWidget->isSearchMode())
       m_searchWidget->clearSearchBox();
}

void HeaderBar::updateIcons()
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
//    m_view_type_menu->setCurrentDirectory(m_window->getCurrentUri());
//    m_view_type_menu->setCurrentView(m_window->getCurrentPage()->getView()->viewId(), true);

//    int count = 0;
//    if(m_window->getCurrentPage() && m_window->getCurrentPage()->getView()){
//        auto iface = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(m_window->getCurrentPage()->getView());
//        if(iface)
//            count = iface->getAllDisplayFileCount();
//    }
//    if(!(m_window->getCurrentUri().startsWith("search:///") && count>30000)){/* 取消搜索时数量超过阈值不排序 */
//        m_sort_type_menu->switchSortTypeRequest(m_window->getCurrentSortColumn());
//        m_sort_type_menu->switchSortOrderRequest(m_window->getCurrentSortOrder());
//    }
    //use fixed icon mark-location-symbolic in close search button
    //m_searchWidget->updateCloseSearch(Peony::FileUtils::getFileIconName(m_window->getCurrentUri()));
    //go back & go forward
    if (m_window->getCurrentPage()) {
        m_go_back->setEnabled(m_window->getCurrentPage()->canGoBack());
        m_go_forward->setEnabled(m_window->getCurrentPage()->canGoForward());
        if (! m_is_intel)
        {
            m_go_up->setEnabled(m_window->getCurrentPage()->canCdUp());
            m_go_up->setProperty("useIconHighlightEffect", 0x2);
            m_go_up->setProperty("isWindowButton", 1);
        }
    }

    //fix create folder fail issue in special path
//    auto curUri = m_window->getCurrentUri();
//    auto info = Peony::FileInfo::fromUri(curUri, false);
//    Peony::FileInfoJob job(info);
//    job.querySync();
//    if (info->canWrite())
//        m_create_folder->setEnabled(true);
//    else
//        m_create_folder->setEnabled(false);

    m_go_back->setProperty("useIconHighlightEffect", 0x2);
    m_go_back->setProperty("isWindowButton", 1);
    m_go_forward->setProperty("useIconHighlightEffect", 0x2);
    m_go_forward->setProperty("isWindowButton", 1);

    //maximize & restore
    //updateMaximizeState();

//    QString viewId = m_window->getCurrentPage()->getView()->viewId();
//    for (auto action : m_view_actions->actions()) {
//        if (action->data().toString() == viewId) {
//            action->setChecked(true);
//        }
//    }
}

void HeaderBar::updateViewTypeEnable()
{
//    auto url = m_window->getCurrentUri();
//    if (url == "computer:///") {
//        m_actions.find(HeaderBarAction::IconView).value()->setVisible(false);
//        m_actions.find(HeaderBarAction::ListView).value()->setVisible(false);
//    } else {
//        m_actions.find(HeaderBarAction::IconView).value()->setVisible(true);
//        m_actions.find(HeaderBarAction::ListView).value()->setVisible(true);
//    }
//    if(url.startsWith("search:///")){
//        m_actions.find(HeaderBarAction::IconView).value()->setEnabled(false);
//    }else{
//        m_actions.find(HeaderBarAction::IconView).value()->setEnabled(true);
//    }
//    m_view_type_menu->setEnabled(false);
//    m_view_type_menu->menuAction()->setVisible(false);
//    auto url = m_window->getCurrentUri();
//    //qDebug() << "updateViewTypeEnable url:" << url;
//    if(url == "computer:///"){
//        m_view_type_menu->setEnabled(false);
//        m_view_type_menu->menuAction()->setVisible(false);
//    }else{
//        m_view_type_menu->setEnabled(true);
//        m_view_type_menu->menuAction()->setVisible(true);
//        //bug#118439 修改切换成列表视图后，图标仍然是图标视图
//        m_actions.find(HeaderBarAction::ViewType).value()->setIcon(m_view_type_menu->getCurrentIconFromViewId());
//    }
}

void HeaderBar::updateSortTypeEnable()
{
//    auto url = m_window->getCurrentUri();
//    qDebug() << "url:" << url;
//    if(url == "computer:///"){
//        m_sort_type_menu->setEnabled(false);
//        m_sort_type_menu->menuAction()->setVisible(false);
//    }else{
//        m_sort_type_menu->setEnabled(true);
//        m_sort_type_menu->menuAction()->setVisible(true);
//    }
//    // 更新分隔符显示状态
//    updateSeparatorVisibility();
}

void HeaderBar::updateMaximizeState()
{
    //maximize & restore
    bool maximized = m_window->isMaximized();
    if (maximized) {
        m_maximize_restore_button->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        m_maximize_restore_button->setToolTip(tr("Restore"));
    } else {
        m_maximize_restore_button->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_maximize_restore_button->setToolTip(tr("Maximize"));
    }
}

void HeaderBar::cancleSelect()
{
    switchSelectStatus(false);
}

void HeaderBar::addTabletMenu()
{
    //task#106007 【文件管理器】文件管理器应用做平板UI适配，增加多选模式
    QAction *tabletAction = addAction(tr("Select All Item"));
    m_actions.insert(HeaderBarAction::TabletSelectAll, tabletAction);
    auto select = qobject_cast<QToolButton *>(widgetForAction(tabletAction));
    select->setProperty("textColor", 0x01);

    tabletAction->setVisible(false);
    connect(tabletAction, &QAction::triggered, [=]() {
        updateSelectStatus(false);
    });

    tabletAction = addAction(tr("Select"));
    m_actions.insert(HeaderBarAction::TabletSelectDone, tabletAction);
    auto selectDone = qobject_cast<QToolButton *>(widgetForAction(tabletAction));
    selectDone->setProperty("textColor", 0x01);

    connect(tabletAction, &QAction::triggered, this, [=]() {
        auto view = m_window->getCurrentPage()->getView();
        auto iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
        if (iface2) {
            if (iface2->isEnableMultiSelect()) {
                quitMultiSelect();
            } else {
                iface2->doMultiSelect(true);
                updateSelectStatus(true);
            }
         }
        m_window->getCurrentPage()->getView()->repaintView();
    });

    connect(Peony::DirectoryViewHelper::globalInstance(), &Peony::DirectoryViewHelper::updateSelectStatus, this, [=](bool status){
        if (m_tablet_mode) {
            updateSelectStatus(true);
        }
    });

    addSpacing(2);

    tabletAction = addAction(tr("MoveTo"));
    m_actions.insert(HeaderBarAction::TabletMoveTo, tabletAction);
    tabletAction->setVisible(false);
    connect(tabletAction, &QAction::triggered, [=]() {
        if (CopyOrMoveTo(true)) {
            quitMultiSelect();
        }
    });

    tabletAction = addAction(tr("CopyTo"));
    m_actions.insert(HeaderBarAction::TabletCopyTo, tabletAction);
    tabletAction->setVisible(false);
    connect(tabletAction, &QAction::triggered, [=]() {
        if (CopyOrMoveTo(false)) {
            quitMultiSelect();
        }
    });

    tabletAction = addAction(tr("Delete"));
    m_actions.insert(HeaderBarAction::TabletDelete, tabletAction);
    tabletAction->setVisible(false);
    auto trash = qobject_cast<QToolButton *>(widgetForAction(tabletAction));
    trash->setProperty("textColor", 0x02);

    connect(tabletAction, &QAction::triggered, [=]() {
        if (m_window->getCurrentUri() == "trash:///") {
            Peony::FileOperationUtils::executeRemoveActionWithDialog(m_window->getCurrentSelections());
        } else {
            Peony::FileOperationUtils::trash(m_window->getCurrentSelections(), true);
        }
        updateSelectStatus(true);
    });
}

void HeaderBar::updateTabletModeValue(bool isTabletMode)
{
    setStyle(HeaderBarStyle::getStyle());
    m_searchWidget->closeSearch();
    m_searchWidget->updateSearchRequest(false);
    m_searchWidget->updateTabletModeValue(isTabletMode);
    m_tablet_mode = isTabletMode;
    bool noComputer = false;
    auto url = m_window->getCurrentUri();
    if(url != "computer:///"){
        noComputer = true;
    }
//    m_view_type_menu->menuAction()->setVisible(false);
//    m_actions.find(HeaderBarAction::SortType).value()->setVisible(noComputer);
//    m_actions.find(HeaderBarAction::ViewType).value()->setVisible(noComputer);
    if (isTabletMode) {
        m_actions.find(HeaderBarAction::TabletSelectDone).value()->setVisible(noComputer);
        m_actions.find(HeaderBarAction::Copy).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Cut).value()->setVisible(false);
        m_actions.find(HeaderBarAction::SeletcAll).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Delete).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Option).value()->setVisible(true);
        if (! m_is_intel) {
            m_actions.find(HeaderBarAction::GoForward).value()->setVisible(false);
        }
        updateSelectStatus(true);

    } else {
        m_actions.find(HeaderBarAction::TabletSelectDone).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletSelectAll).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletMoveTo).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletCopyTo).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletDelete).value()->setVisible(false);
        m_actions.find(HeaderBarAction::Option).value()->setVisible(false);
        if (! m_is_intel) {
            m_actions.find(HeaderBarAction::GoForward).value()->setVisible(true);
        }
    }
//    m_preview_action->setVisible(!isTabletMode);
}

bool HeaderBar::CopyOrMoveTo(bool isCut)
{
    auto currentUri = m_window->getCurrentSelections();
    if (currentUri.startsWith("trash://") || currentUri.startsWith("recent://")
        || currentUri.startsWith("computer://") || currentUri.startsWith("favorite://")
        || currentUri.startsWith("search://"))
    {
        return false;
    }

    QString targetPath = QFileDialog::getExistingDirectory(this, tr("Select path"), "computer://", QFileDialog::ShowDirsOnly);
    if (targetPath.isEmpty()) {
        return false;
    }
    Peony::ClipboardUtils::setClipboardFiles(currentUri, isCut);
    if (! targetPath.contains("file://") && targetPath != "") {
        targetPath = "file://" + targetPath;
    }
    if (Peony::ClipboardUtils::isClipboardHasFiles()) {
        Peony::ClipboardUtils::pasteClipboardFiles(targetPath);
    }
    return true;
}

void HeaderBar::quitMultiSelect()
{
    if (m_tablet_mode) {
        m_isSelectAll = false;
        auto view = m_window->getCurrentPage()->getView();
        view->setSelections(QStringList());

        QAction *action = m_actions.find(HeaderBarAction::TabletSelectDone).value();
        auto select = qobject_cast<QToolButton *>(widgetForAction(action));
        select->setText(tr("Select"));

        bool status = view->getAllFileUris().count() > 0 ? true : false;
        action->setVisible(status);

        auto iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
        if (iface2 && iface2->isEnableMultiSelect()) {
            iface2->doMultiSelect(false);
        }
        m_actions.find(HeaderBarAction::TabletSelectAll).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletMoveTo).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletCopyTo).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletDelete).value()->setVisible(false);
    }
}

void HeaderBar::setSearchMode(bool isSearching)
{
    m_searchWidget->setSearchMode(isSearching);
}

void HeaderBar::updateSelectStatus(bool autoUpdate)
{
    QAction *action = m_actions.find(HeaderBarAction::TabletSelectDone).value();
    auto selectDone = qobject_cast<QToolButton *>(widgetForAction(action));
    if(!m_window->getCurrentPage())
        return;
    auto view = m_window->getCurrentPage()->getView();
    auto iface2 = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(view);
    if (!iface2)
        return ;

    if (!iface2->isEnableMultiSelect()) {
        m_actions.find(HeaderBarAction::TabletSelectAll).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletMoveTo).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletCopyTo).value()->setVisible(false);
        m_actions.find(HeaderBarAction::TabletDelete).value()->setVisible(false);
        bool status = view->getAllFileUris().count() > 0? true : false;
        m_actions.find(HeaderBarAction::TabletSelectDone).value()->setVisible(status);
        selectDone->setText(tr("Select"));
        return;
    }
    selectDone->setText(tr("Select Done"));

    action = m_actions.find(HeaderBarAction::TabletSelectAll).value();
    action->setVisible(true);
    auto selectAll = qobject_cast<QToolButton *>(widgetForAction(action));

    if (autoUpdate) {
        int num = view->getSelections().count();
        int totalNum = view->getAllFileUris().count();
        if (num == totalNum) {
            m_isSelectAll = true;
            selectAll->setText(tr("Deselect All"));
        } else {
            m_isSelectAll = false;
            selectAll->setText(tr("Select All Item"));
        }
        bool status = num > 0? true : false;
        m_actions.find(HeaderBarAction::TabletMoveTo).value()->setVisible(status);
        m_actions.find(HeaderBarAction::TabletCopyTo).value()->setVisible(status);
        m_actions.find(HeaderBarAction::TabletDelete).value()->setVisible(status);
    } else {
        if (m_isSelectAll) {
            view->invertSelections();
            m_isSelectAll = false;
            selectAll->setText(tr("Select All Item"));
        } else {
            view->setSelections(QStringList());
            view->invertSelections();
            m_isSelectAll = true;
            selectAll->setText(tr("Deselect All"));
        }
    }
}

void HeaderBar::updateSortButton()
{
//    QAction *a = m_actions.find(HeaderBarAction::SortType).value();
//    QToolButton *sortType = qobject_cast<QToolButton *>(widgetForAction(a));
//    QString sortTypeName = m_sort_type_menu->getSortTypeName(m_window->getCurrentSortColumn());
//    m_sort_type_menu->updateSortOrderName(m_window->getCurrentSortColumn());
//    QFontMetrics fm(sortType->font());
//    int textWidth = fm.boundingRect(sortTypeName).width(); // 文本宽度
//    int iconWidth = sortType->iconSize().width();    // 图标宽度
//    int totalWidth = textWidth + (iconWidth * 2) + 36;// 加上一些内边距36
//    totalWidth = totalWidth < 167 ? totalWidth : 167;
//    sortType->setFixedWidth(totalWidth);
//    int availableWidth = totalWidth - sortType->iconSize().width() - 50;
//    QString elidedText = fm.elidedText(sortTypeName, Qt::ElideRight, availableWidth);
//    a->setText(elidedText);
//    a->setToolTip(sortTypeName);

//    if (m_window->getCurrentSortOrder() == Qt::AscendingOrder) {
//        a->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
//    } else {
//        a->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
//    }
}

void HeaderBar::updatePreviewStatus(bool check)
{
//    m_preview_action->setChecked(check);
//    m_preview_action->triggered(check);
}

//HeaderBarToolButton
HeaderBarToolButton::HeaderBarToolButton(QWidget *parent) : QToolButton(parent)
{
    setAutoRaise(false);
    setIconSize(QSize(16, 16));
}

//HeadBarPushButton
HeadBarPushButton::HeadBarPushButton(QWidget *parent) : QPushButton(parent)
{
    setIconSize(QSize(16, 16));
}

//HeaderBarStyle
HeaderBarStyle *HeaderBarStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new HeaderBarStyle;
    }
    return global_instance;
}

int HeaderBarStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    if (qobject_cast<const HeaderBarContainer *>(widget))
        return 0;

    switch (metric) {
    case PM_ToolBarIconSize:
        return 16;
    case PM_ToolBarSeparatorExtent:
        return 1;
    case PM_ToolBarItemSpacing: {
        return 1;
    }
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void HeaderBarStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    //This is a "lie". We want to use instant popup menu for tool button, and we aslo
    //want use popup menu style with this tool button, so we change the related flags
    //to draw in our expected.
    if (control == CC_ToolButton) {
        QStyleOptionToolButton button = *qstyleoption_cast<const QStyleOptionToolButton *>(option);
        if (button.features.testFlag(QStyleOptionToolButton::HasMenu)) {
            button.features = QStyleOptionToolButton::None;
            if (!widget->property("isOptionButton").toBool()) {
                button.features |= QStyleOptionToolButton::HasMenu;
                button.features |= QStyleOptionToolButton::MenuButtonPopup;
                button.subControls |= QStyle::SC_ToolButtonMenu;
            }
            button.palette.setColor(QPalette::Button,Qt::transparent);
            return qApp->style()->drawComplexControl(control, &button, painter, widget);
        } else {
            if (0x01 == widget->property("textColor").toInt()) {
                button.palette.setColor(QPalette::Text, QColor(55, 144, 250));
            } else if (0x02 == widget->property("textColor").toInt()) {
                button.palette.setColor(QPalette::Text, QColor(243, 34, 45));
            }
            return qApp->style()->drawComplexControl(control, &button, painter, widget);
        }
    }
    return qApp->style()->drawComplexControl(control, option, painter, widget);
}

void HeaderBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_IndicatorToolBarSeparator) {
        return;

    }
    return qApp->style()->drawPrimitive(element, option, painter, widget);
}

TopMenuBar::TopMenuBar(HeaderBar *headerBar, MainWindow *parent) : QMenuBar(parent)
{
    UI_TEST_INIT
    m_window = parent;
    m_header_bar = headerBar;
    setContextMenuPolicy(Qt::CustomContextMenu);

    setStyleSheet(".TopMenuBar"
                  "{"
                  "background-color: transparent;"
                  "border: 0px solid transparent"
                  "}");

    setFixedHeight(40);

    m_top_menu_layout = new QHBoxLayout(this);
    m_top_menu_layout->setSpacing(0);
    m_top_menu_layout->setContentsMargins(0, 0, 0, 0);

    m_top_menu_internal_widget = new QWidget(this);
    m_top_menu_internal_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    addWindowButtons();
    m_window->installEventFilter(this);
    UI_TEST_SET_ALL(this, "peony_TopMenubar_QMenuBar");
}

bool TopMenuBar::eventFilter(QObject *obj, QEvent *e)
{

    Q_UNUSED(obj)
    if (m_window) {
#ifdef KY_SDK_WAYLANDHELPER
        if (obj == m_max_or_restore) {
            if (e->type() == QEvent::ToolTip) {
                e->setAccepted(true);
                e->accept();
                //auto helpevent = static_cast<QHelpEvent *>(e);
                QRect rect = m_max_or_restore->geometry();
                auto pos = m_max_or_restore->mapTo(m_window, QPoint(rect.width()/2, 0));
                rect.translate(pos.x() - 84, 0);
                m_window->m_window_helper->showTileFlyout(rect);
                return true;
            }
        }
#endif
        //use updateMaximizeState function, comment those code
        if (e->type() == QEvent::Resize || QEvent::WindowStateChange == e->type()) {
            //fix bug#95419, drag change window to restore not change state issue
            m_header_bar->updateMaximizeState();
        }
        //fix double click space window has no change issue, bug#38499
//        if (e->type() == QEvent::MouseButtonDblClick)
//        {
//            m_header_bar->m_window->maximizeOrRestore();
//        }
        return false;
    } else {
        if (e->type() == QEvent::MouseMove) {
            //auto w = qobject_cast<QWidget *>(obj);
            QCursor c;
            c.setShape(Qt::ArrowCursor);
            //this->setCursor(c);
            //w->setCursor(c);
            this->topLevelWidget()->setCursor(c);
        }

    }

    return false;

}
void TopMenuBar::addWindowButtons()
{
    UI_TEST_INIT
    m_window->installEventFilter(this);
    auto layout = new QHBoxLayout;

    layout->setContentsMargins(0, 0, 6, 0);
    layout->setSpacing(4);

    auto optionButton = new QToolButton(m_top_menu_internal_widget);
    optionButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    optionButton->setToolTip(tr("Option"));
    optionButton->setAutoRaise(true);
    optionButton->setPopupMode(QToolButton::InstantPopup);
    optionButton->setProperty("isOptionButton", true);
    optionButton->setProperty("isWindowButton", 1);
    optionButton->setFixedSize(30, 30);
    OperationMenu *operationMenu = new OperationMenu(m_window, optionButton);
    optionButton->setMenu(operationMenu);

    //minimize, maximize and close
    //  最小化，最大化，关闭
    auto minimize = new QToolButton(m_top_menu_internal_widget);
    minimize->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minimize->setToolTip(tr("Minimize"));
    minimize->setAutoRaise(true);
    minimize->setFixedSize(30, 30);
    connect(minimize, &QToolButton::clicked, this, [=]() {
        KWindowSystem::minimizeWindow(m_window->winId());
        m_window->showMinimized();
    });

    //window-maximize-symbolic
    //window-restore-symbolic
    auto maximizeAndRestore = new QToolButton(m_top_menu_internal_widget);
    m_header_bar->m_maximize_restore_button = maximizeAndRestore;
    //switch tips with button status, fix bug#77604
    m_header_bar->updateMaximizeState();
    maximizeAndRestore->setAutoRaise(true);
    maximizeAndRestore->setFixedSize(30, 30);

    connect(maximizeAndRestore, &QToolButton::clicked, this, [=]() {
        m_window->maximizeOrRestore();
    });
    m_max_or_restore = maximizeAndRestore;

    auto close = new QToolButton(m_top_menu_internal_widget);
    close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    close->setToolTip(tr("Close"));
    close->setAutoRaise(true);
    close->setFixedSize(30, 30);

    connect(close, &QToolButton::clicked, this, [=]() {
        m_window->close();
    });

    auto palette = qApp->palette();
    palette.setColor(QPalette::Highlight, QColor("#E54A50"));
    close->setPalette(palette);

    m_minimize = minimize;
    m_close = close;

    if (QDBusConnection::connectToBus(0,QString("com.kylin.statusmanager.interface")).isConnected())
    {
        if (!g_statusManagerDBus) {
            g_statusManagerDBus = new QDBusInterface(DBUS_STATUS_MANAGER_IF, "/" ,DBUS_STATUS_MANAGER_IF,QDBusConnection::sessionBus(),this);
        }
        QDBusReply<bool> message_a = g_statusManagerDBus->call("get_current_tabletmode");
        if (message_a.isValid()) {
            m_tablet_mode = message_a.value();
        }
        updateTabletMode(m_tablet_mode);

        connect(g_statusManagerDBus, SIGNAL(mode_change_signal(bool)), this, SLOT(updateTabletMode(bool)));
    }


    layout->addWidget(optionButton);
    layout->addWidget(minimize);
    layout->addWidget(maximizeAndRestore);
    layout->addWidget(close);

    m_top_menu_internal_widget->setLayout(layout);
    QSpacerItem *spacer = new QSpacerItem(4000, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_top_menu_layout->addSpacerItem(spacer);
    m_top_menu_layout->addWidget(m_top_menu_internal_widget);

    optionButton->setMouseTracking(true);
    optionButton->installEventFilter(this);
    minimize->setMouseTracking(true);
    minimize->installEventFilter(this);
    maximizeAndRestore->setMouseTracking(true);
    maximizeAndRestore->installEventFilter(this);
    close->setMouseTracking(true);
    close->installEventFilter(this);

    optionButton->setFocusPolicy(Qt::FocusPolicy(optionButton->focusPolicy() /*&  ~Qt::TabFocus*/));
    minimize->setFocusPolicy(Qt::FocusPolicy(minimize->focusPolicy() /*& ~Qt::TabFocus*/));
    maximizeAndRestore->setFocusPolicy(Qt::FocusPolicy(maximizeAndRestore->focusPolicy() /*& ~Qt::TabFocus*/));
    close->setFocusPolicy(Qt::FocusPolicy(close->focusPolicy() /*& ~Qt::TabFocus*/));

    for (int i = 0; i < layout->count(); i++) {
        auto w = layout->itemAt(i)->widget();
        w->setProperty("isWindowButton", 1);
        w->setProperty("useIconHighlightEffect", 0x2);
    }
    close->setProperty("isWindowButton", 2);
    close->setProperty("useIconHighlightEffect", 0x8);

    optionButton->setVisible((Peony::GlobalSettings::getInstance()->getProjectName() != V10_SP1_EDU));

    UI_TEST_SET_ALL(optionButton, "peony_toolButton_option");
    UI_TEST_SET_ALL(m_minimize, "peony_toolButton_minimuze");
    UI_TEST_SET_ALL(m_max_or_restore, "peony_toolButton_maxrestore");
    UI_TEST_SET_ALL(m_close, "peony_toolButton_close");
}

void TopMenuBar::updateTabletMode(bool isTabletMode)
{
    m_tablet_mode = isTabletMode;
    if(m_tablet_mode)
    {
        m_minimize->hide();
        m_max_or_restore->hide();
        m_close->hide();
    }
    else
    {
        m_minimize->setVisible(true);
        m_max_or_restore->setVisible(true);
        m_close->setVisible(true);
    }
    QTimer::singleShot(100, this, [=](){
        auto palette = qApp->palette();
        palette.setColor(QPalette::Highlight, QColor("#E54A50"));
        m_close->setPalette(palette);
    });
    qApp->setProperty("tabletMode", isTabletMode);
}

HeaderBarContainer::HeaderBarContainer(QWidget *parent) : QToolBar(parent)
{
    setStyle(HeaderBarStyle::getStyle());
    setContextMenuPolicy(Qt::CustomContextMenu);

//    setStyleSheet(".HeaderBarContainer"
//                  "{"
//                  "background-color: transparent;"
//                  "border: 0px solid transparent"
//                  "}");

    setMovable(false);

    m_layout = new QHBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_internal_widget = new QWidget(this);
    m_internal_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

bool HeaderBarContainer::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj)
    auto window = qobject_cast<MainWindow *>(obj);
    if (window) {
        //use updateMaximizeState function, comment those code
        if (e->type() == QEvent::Resize || QEvent::WindowStateChange == e->type()) {
            //fix bug#95419, drag change window to restore not change state issue
            m_header_bar->updateMaximizeState();
        }
        //fix double click space window has no change issue, bug#38499
        //fix Right click in the blank space on the right side of the tab to quickly maximize and restore the window. link bug102455
//        if (e->type() == QEvent::MouseButtonDblClick)
//        {
//            m_header_bar->m_window->maximizeOrRestore();
//        }
        return false;
    } else {
        if (e->type() == QEvent::MouseMove) {
            //auto w = qobject_cast<QWidget *>(obj);
            QCursor c;
            c.setShape(Qt::ArrowCursor);
            //this->setCursor(c);
            //w->setCursor(c);
            this->topLevelWidget()->setCursor(c);
        }
    }

    return false;
}

void HeaderBarContainer::addHeaderBar(HeaderBar *headerBar)
{
    if (m_header_bar)
        return;
    m_header_bar = headerBar;

    headerBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_layout->addWidget(headerBar);
    m_internal_widget->setLayout(m_layout);
    addWidget(m_internal_widget);

//    m_header_bar->m_window->installEventFilter(this);
}
void HeaderBarContainer::addMenu(MainWindow *m_window)
{
    m_topMenu = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(m_topMenu);

    QToolButton *minimize = new QToolButton(this);
    minimize->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minimize->setToolTip(tr("Minimize"));
    minimize->setAutoRaise(true);
    minimize->setFixedSize(QSize(48, 48));
    minimize->setIconSize(QSize(16, 16));
    minimize->setProperty("isWindowButton", 1);
    connect(minimize, &QToolButton::clicked, this, [=]() {
        KWindowSystem::minimizeWindow(m_window->winId());
        m_window->showMinimized();
    });

    QToolButton *close = new QToolButton(this);
    close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    close->setToolTip(tr("Close"));
    close->setAutoRaise(true);
    close->setFixedSize(QSize(48, 48));
    close->setIconSize(QSize(16, 16));

    //fix bug#143507, button color is not red issue
    close->setProperty("isWindowButton", 2);
    close->setProperty("useIconHighlightEffect", 0x8);
    connect(close, &QToolButton::clicked, this, [=]() {
        m_window->close();
    });

    layout->addWidget(minimize);
    layout->addWidget(close);
    m_topMenu->setLayout(layout);
    m_topMenu->hide();
    m_layout->addWidget(m_topMenu);

}
void HeaderBarContainer::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QStyleOptionToolBar opt;
    initStyleOption(&opt);
    bool isEnable = opt.state & QStyle::State_Enabled;
    bool isActive = opt.state & QStyle::State_Active;
    auto color = qApp->palette().color(isEnable? isActive? QPalette::Active: QPalette::Inactive: QPalette::Disabled, QPalette::Base);
    p.fillRect(this->rect(), color);
    auto brush = qvariant_cast<QBrush>(qApp->property("button-active"));
    p.setPen(brush.color());
    p.drawLine(this->rect().bottomLeft(), this->rect().bottomRight());
}

void VerticalSeparator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    if (m_lineVisible) {
        // 计算竖线的位置：水平居中，垂直居中
        int lineX = width() / 2;
        int lineTop = (height() - 16) / 2;
        int lineBottom = lineTop + 16;

        // 设置画笔
        QPen pen(QColor(0x87, 0x87, 0x87, 64), 1); // 1像素宽
        painter.setPen(pen);

        // 绘制竖线
        painter.drawLine(lineX, lineTop, lineX, lineBottom);
    }
}
