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

#include "tab-widget.h"
//#include "navigation-tab-bar.h"

#include "preview-page-factory-manager.h"
#include "preview-page-plugin-iface.h"
#include "directory-view-widget.h"

#include "file-info.h"
#include "file-utils.h"
#include "file-info-job.h"
#include "file-launch-manager.h"
#include "search-vfs-uri-parser.h"
//#include "properties-window.h"
#include "properties-window-factory-plugin-manager.h"
#include "file-enumerator.h"

#include <QStackedWidget>
#include <QToolButton>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QToolBar>
#include <QSplitter>
#include <QStringListModel>
#include <QFileDialog>
#include <QHeaderView>
#include <QScrollBar>
#include <QAction>

#include <QTimer>

#include "directory-view-container.h"
#include "file-utils.h"
#include "peony-main-window-style.h"

#include "directory-view-factory-manager.h"
#include "global-settings.h"
#include "manager-control.h"
#include "main-window.h"

#include "file-enumerator.h"
#include "file-info-job.h"
#include "file-info.h"

#include "volume-manager.h"
#include "volumeManager.h"
#include "directoryviewhelper.h"
#include "image-mount-manager.h"

#include "file-info-manager.h"
#include "file-info-job.h"
#include "file-meta-info.h"
#include "global-settings.h"
#include "space-save-dialog.h"
#include "idm-dbus-manager.h"
#include "file-utils.h"
#include "convenient-macros.h"

#include "directory-view-factory-manager.h"

#include <QApplication>
#include <QStandardPaths>

#include <QMessageBox>

#include <QDebug>
#include <QPainter>
#include <QPainterPath>

#include <QWidgetAction>

#include <QVariantAnimation>
#include <QParallelAnimationGroup>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define PUSH_BUTTON_TOTAL_PADDING 14

static PushButtonStyle *global_instance = nullptr;

PushButtonStyle *PushButtonStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new PushButtonStyle;
    }
    return global_instance;
}

void PushButtonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, option, painter, widget);
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
            return;
        }
        break;
    }
    default:
        break;
    }
    qApp->style()->drawControl(element, option, painter, widget);
}

int PushButtonStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ButtonMargin:
    {
        return 0;
    }
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QRect PushButtonStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_PushButtonContents:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            QRect rect = option->rect;
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                rect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
            } else if (!text && icon && !(button->features & QStyleOptionButton::HasMenu)) {

            } else {
                rect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
            }
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                rect.adjust(dbw, dbw, -dbw, -dbw);
            }
            return rect;
        }
        break;
    }
    default:
        break;
    }

    return QProxyStyle::subElementRect(element, option, widget);
}

TabWidget::TabWidget(QWidget *parent) : QMainWindow(parent)
{
    UI_TEST_INIT
    QString localeName = QLocale::system().name();
    if (localeName.contains("ug") || localeName.contains("kk") || localeName.contains("ky")) {
        setLayoutDirection(Qt::RightToLeft);
    }

    setStyle(PeonyMainWindowStyle::getStyle());

    setAttribute(Qt::WA_TranslucentBackground);

    m_parent = parent;
    m_tab_bar = new NavigationTabBar(this);
    m_tab_bar->setFocusPolicy(Qt::TabFocus);
    m_tab_bar->setLayoutDirection(layoutDirection());
    m_tab_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_stack = new QStackedWidget(this);
    m_stack->setContentsMargins(8, 0, 0, 0);
//    m_buttons = new PreviewPageButtonGroups(this);
    m_preview_page_container = new QStackedWidget(this);
    m_preview_page_container->setContentsMargins(0, 2, 0, 0);
    m_preview_page_container->setMinimumWidth(300);

    //status bar
    m_status_bar = new TabStatusBar(this, this);
    connect(this, &TabWidget::updateItemsNum, m_status_bar, &TabStatusBar::updateItemsNum);
    connect(this, &TabWidget::zoomRequest, m_status_bar, &TabStatusBar::onZoomRequest);
    connect(m_status_bar, &TabStatusBar::zoomLevelChangedRequest, this, &TabWidget::handleZoomLevel);
    //setStatusBar(m_status_bar);

    connect(m_tab_bar, &QTabBar::currentChanged, this, &TabWidget::changeCurrentIndex);
    connect(m_tab_bar, &QTabBar::tabMoved, this, &TabWidget::moveTab);
    connect(m_tab_bar, &QTabBar::tabCloseRequested, this, &TabWidget::removeTab);
    connect(m_tab_bar, &NavigationTabBar::pageRemoved, this, [this]{
        updateTabAllPages();
        updateTabBarGeometry();
    });
    connect(m_tab_bar->m_slide_animation, &QVariantAnimation::valueChanged, this, &TabWidget::updateTabBarGeometry);
    connect(m_tab_bar, &NavigationTabBar::addPageRequest, this, &TabWidget::addPage);
    connect(m_tab_bar, &NavigationTabBar::locationUpdated, this, &TabWidget::updateSearchPathButton);
    connect(m_tab_bar, &NavigationTabBar::locationUpdated, this, [this]{
        updateTabAllPages();
        updateTabBarGeometry();
    });

    connect(m_tab_bar, &NavigationTabBar::closeWindowRequest, this, &TabWidget::closeWindowRequest);
    connect(m_tab_bar, &QTabBar::currentChanged, [=](int index){
        Q_EMIT tabBarIndexUpdate(index);
    });

   // QActionGroup *group = new QActionGroup(this);
   // group->setExclusive(true);

    //bug#94981 修改添加控件的位置和形状
    m_add_page_button = new QToolButton(this);
    m_add_page_button->setFocusPolicy(Qt::TabFocus);
    m_add_page_button->setFixedSize(QSize(30, 30));
    m_add_page_button->setIconSize(QSize(16, 16));
    m_add_page_button->setIcon(QIcon::fromTheme("list-add-symbolic"));
    UI_TEST_SET_ALL(m_add_page_button, "peony_toolbutton_addPage_button");

    m_add_page_button->setPopupMode(QToolButton::DelayedPopup);
    m_add_page_button->setAutoRaise(true);
    m_add_page_button->setStyle(TabBarStyle::getStyle());
    // m_add_page_button->setStyleSheet("background-color: transparent; border: none;");

    connect(m_add_page_button, &QToolButton::clicked, this, [=](){
        QString str = m_tab_bar->tabData(m_tab_bar->currentIndex()).toString();
        m_tab_bar->addPageRequest(str, true);
    });

    m_show_page_button = new QToolButton(this);
    m_show_page_button->setPopupMode(QToolButton::DelayedPopup);
    m_show_page_button->setFixedSize(QSize(30, 30));
    m_show_page_button->setIconSize(QSize(16, 16));
    m_show_page_button->setIcon(QIcon::fromTheme("ukui-all-tabs-symbolic"));
    m_show_page_button->setAutoRaise(true);
    // m_show_page_button->setObjectName("toolButton");
    m_show_page_button->setStyle(TabBarStyle::getStyle());
    m_show_page_button->setToolTip(tr("Show all tabs"));

    m_model = new QStandardItemModel;
    m_treeView = new QTreeView(this);
    m_menu = new QMenu(this);

    connect(m_show_page_button, &QToolButton::clicked, this, [=](){
        updateTabAllPages();

        if (m_menu) {
            delete m_menu;
        }
        m_menu = new QMenu(this);
        m_menu->setAttribute(Qt::WA_TranslucentBackground);
        auto action = new QWidgetAction(this);
        action->setDefaultWidget(m_treeView);
        m_menu->addAction(action);
        connect(m_treeView, &QTreeView::clicked, this, [=](const QModelIndex &index) {
            switch (index.column()) {
            case 0: {
                changeCurrentIndex(index.row());
                m_menu->hide();
                break;
            }
            case 1: {
                removeTab(index.row());
                m_menu->hide();
                break;
            }
            default:
                break;
            }
        });
        auto pTabBar = m_tab_bar->mapToGlobal(m_tab_bar->pos());
        QPoint pos(pTabBar.x() + m_show_page_button->x(), pTabBar.y() + m_tab_bar->sizeHint().height() - 8);
        if (layoutDirection() == Qt::RightToLeft) {
            pos = m_show_page_button->mapToGlobal(QPoint(m_show_page_button->width() - m_menu->sizeHint().width(), m_show_page_button->height()));
        }

        m_menu->setContentsMargins(0,0,0,0);
        m_menu->exec(pos);
    });

    updateTabBarGeometry();

    //trash quick operate buttons
    QHBoxLayout *trash = new QHBoxLayout();
    m_trash_bar_layout = trash;
    QToolBar *trashButtons = new QToolBar(this);
    m_trash_bar = trashButtons;

    QLabel *Label = new QLabel(tr("Trash"), trashButtons);
//    Label->setFixedHeight(TRASH_BUTTON_HEIGHT);
//    Label->setFixedWidth(TRASH_BUTTON_WIDTH);
    m_trash_label = Label;
    QPushButton *clearAll = new QPushButton(tr("Clear"), trashButtons);
//    clearAll->setFixedWidth(TRASH_BUTTON_WIDTH);
//    clearAll->setFixedHeight(TRASH_BUTTON_HEIGHT);/* Fix the bug:62841,the font of the clear button is not displayed completely */
    clearAll->setStyle(PushButtonStyle::getStyle());
    m_clear_button = clearAll;
    QPushButton *recover = new QPushButton(tr("Recover"), trashButtons);
//    recover->setFixedWidth(TRASH_BUTTON_WIDTH);
//    recover->setFixedHeight(TRASH_BUTTON_HEIGHT);
    recover->setStyle(PushButtonStyle::getStyle());
    m_recover_button = recover;
    //hide trash button to fix bug 31322, according to designer advice
    m_recover_button->hide();

    //trash->addSpacing(10);
    trash->addWidget(Label, Qt::AlignLeft);
    trash->setContentsMargins(10, 0, 10, 0);
    trash->addWidget(trashButtons);
    trash->addStretch();
    trash->addWidget(recover);
    trash->addSpacing(10);
    trash->addWidget(clearAll);
    updateTrashBarVisible();

    connect(clearAll, &QPushButton::clicked, this, [=]()
    {
        Q_EMIT this->clearTrash();
    });

    connect(recover, &QPushButton::clicked, this, [=]()
    {
        Q_EMIT this->recoverFromTrash();
    });

    //advance search ui init
    initAdvanceSearch();

    QWidget *w = new QWidget();
    w->setAttribute(Qt::WA_TranslucentBackground);
    w->setStyleSheet(
                  "QWidget#w {background-color: transparent;"
                  "border: 0px solid transparent;}");

    auto vbox = new QVBoxLayout();
    m_top_layout = vbox;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addLayout(trash);
    vbox->addSpacing(6);
    vbox->addLayout(m_search_bar_layout);
    vbox->addSpacing(6);
    //vbox->addLayout(m_search_filter_layout);
    vbox->setAlignment(Qt::AlignTop);
    vbox->addWidget(m_filter_container,0, Qt::AlignTop);
    m_preview_splitter = new QSplitter(this);
    m_preview_splitter->setChildrenCollapsible(false);
    m_preview_splitter->setContentsMargins(0, 0, 0, 0);
    m_preview_splitter->setHandleWidth(1);
    m_preview_splitter->addWidget(m_stack);
    m_stack->installEventFilter(this);
    m_preview_splitter->setStretchFactor(0, 3);
    m_preview_splitter->setStretchFactor(1, 2);
    m_preview_splitter->addWidget(m_preview_page_container);
    m_preview_page_container->hide();

    vbox->addWidget(m_preview_splitter,1);
    w->setLayout(vbox);
    setCentralWidget(w);

    connect(FileLabelModel::getGlobalModel(), &FileLabelModel::labelColorChanged, this, [=](const QString& uri){
        for(int index = 0; index < m_stack->count(); index++){
            if(uri.startsWith("label:///") && uri == m_tab_bar->tabData(index).toString()){
                m_tab_bar->updateLocation(index, uri.toLocal8Bit());
            }
        }
    });
    //bind preview page
    connect(this, &TabWidget::activePageSelectionChanged, this, [=]() {
        updatePreviewPage();
        m_status_bar->update();
        Q_EMIT this->currentSelectionChanged();
    });

    connect(this, &TabWidget::activePageChanged, this, [=]() {
        QTimer::singleShot(100, this, [=]() {
            m_status_bar->update();
            this->updatePreviewPage();
        });
    });

    connect(this, &TabWidget::activePageLocationChanged, m_status_bar, [=]() {
        m_status_bar->update();
        updateTabBarGeometry();
    });

    connect(this, &TabWidget::activePageLocationChanged, this, [=]{
        auto settings = Peony::GlobalSettings::getInstance();
        if (!settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
            auto uri = getCurrentUri();
            auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
            if (!metaInfo) {
                qCritical()<<"can not get meta info"<<uri;
            } else {
                auto sortType = metaInfo->getMetaInfoVariant(SORT_COLUMN).isValid()? metaInfo->getMetaInfoInt(SORT_COLUMN): 0;
                auto sortOrder = metaInfo->getMetaInfoVariant(SORT_ORDER).isValid()? metaInfo->getMetaInfoInt(SORT_ORDER): 1;
                if (uri.startsWith("idm://")) {
                    QString spaceName = uri.remove("idm:///");
                    spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
                    sortType = Peony::IDMDBusManager::getInstance()->getSpacesSortType(spaceName);
                    sortOrder = Peony::IDMDBusManager::getInstance()->getSpacesSortOrder(spaceName);
                }
                currentPage()->setSortType(Peony::FileItemModel::ColumnType(sortType));
                currentPage()->setSortOrder(Qt::SortOrder(sortOrder));
            }
        }
    });

    //fix bug#166060, 将监听字体变化的处理放在构造函数中，避免重复调用或者闪退等问题
    //监听字体大小改变
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
        connect(fontSetting, &QGSettings::changed, this, [=](const QString &key) {
            double fontSize = fontSetting->get("systemFontSize").toDouble();
            for(int index=0;index<m_conditions_list.length();index++){
                setCondWidthWithFont(m_conditions_list[index], fontSize);
            }
            for(int index=0;index<m_classify_list.length();index++){
                setClassifyWidthWithFont(m_classify_list[index], fontSize);
            }
            // if (m_filter_container && m_filter_container->isVisible()) {
            //     m_file_type_box->updateWidth();
            //     m_file_mtime_box->updateWidth();
            //     m_file_size_box->updateWidth();
            //     m_file_label_box->updateWidth();
            // }
            //fix #185743
            auto realDisplayName = m_current_search->property("realDisplayName").toString();
            auto elidedDisplayName = fontMetrics().elidedText(realDisplayName, Qt::ElideMiddle, /*m_current_search->width()*/200 - m_search_bar->iconSize().width() - PUSH_BUTTON_TOTAL_PADDING);
            m_current_search->setText(elidedDisplayName);
            m_current_search->adjustSize();
            QFont font = m_current_search->font();
            //realDisplayName = Peony::FileUtils::wrapTextForTooltip(realDisplayName,QCursor::pos(),font);
            if (elidedDisplayName != realDisplayName) {
                m_current_search->setToolTip(realDisplayName);
            } else {
                m_current_search->setToolTip("");
            }
        });
    }
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::spaceFileLoadFinished, this, [=](const QString &spaceName){
        if (!currentPage()) {
            return;
        }
        QString updateUri = "idm:///" + QString(QUrl::toPercentEncoding(spaceName));
        if (currentPage()->getCurrentUri() == updateUri || currentPage()->getCurrentUri() == "idm:///New Intelligent Space") {
            handleSwitchModeAndShow(updateUri);
            goToUri(updateUri, true, true);
        }
    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::editSpaceConditionSignal, this, [=](const QString &uri){
        if (!currentPage()) {
            return;
        }
        if (currentPage()->getCurrentUri() != uri && !uri.isEmpty()) {
            goToUri(uri, true, true);
        }
        m_space_edit_button->click();
    });
//    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::createSpaceSignal, this, [=](const QString &uri){
//        handleSwitchModeAndShow(uri);
//        goToUri(uri, true, true);
//    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::deleteSpaceSignal, this, [=](){
        if (!currentPage()) {
            return;
        }
        QString idmUri = "idm:///";
        auto p = this->topLevelWidget();
        auto widget = Peony::IDMDBusManager::getInstance()->getActiveWindow();
        if(widget == p){
            if (currentPage()->getCurrentUri() != idmUri) {
                goToUri(idmUri, true, true);
            }
        }
    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::spaceCreateFinished, this, [=](const QString &spaceName, const QStringList &files){
        if (!currentPage()) {
            return;
        }
        QString updateUri = "idm:///" + QString(QUrl::toPercentEncoding(spaceName));
        MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
        auto p = this->topLevelWidget();
        auto widget = Peony::IDMDBusManager::getInstance()->getActiveWindow();
        if(widget == p){
            if (currentPage()->getCurrentUri() != updateUri) {
                handleSwitchModeAndShow(updateUri);
                goToUri(updateUri, true, true);
            }
        }
    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::spaceAddFilesToSpaced, this, [=](const QString &spaceName){
        QString updateUri = "idm:///" + QString(QUrl::toPercentEncoding(spaceName));
        goToUri(updateUri, true, true);
    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::spacesFilesAutoInserted, this, [=](const QString &spaceName, const QStringList &files, const bool isGotoUri = false){
        if (!currentPage()) {
            return;
        }
        QString idmUri = "idm:///" + QString(QUrl::toPercentEncoding(spaceName));
        if (currentPage()->getCurrentUri() == idmUri && isGotoUri) {
            goToUri(idmUri, true, true);
        }
    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::spacesFilesAutoDeleted, this, [=](const QMap<QString, QStringList> &deleteMap){
        if (!currentPage()) {
            return;
        }
        QStringList changeSpaceNames = deleteMap.keys();
        for (auto spaceName : changeSpaceNames) {
            auto idmUri = "idm:///" + QString(QUrl::toPercentEncoding(spaceName));
            if (currentPage()->getCurrentUri() == idmUri) {
                goToUri(idmUri, true, true);
            }
        }
    });
    connect(Peony::IDMDBusManager::getInstance(), &Peony::IDMDBusManager::spaceRenamed, this, [=](const QString &oldName, const QString &newName){
        if (!currentPage()) {
            return;
        }
        auto idmUri = "idm:///" + QString(QUrl::toPercentEncoding(oldName));
        if (currentPage()->getCurrentUri() == idmUri) {
            auto newUri = "idm:///" + QString(QUrl::toPercentEncoding(newName));
            goToUri(newUri, true, true);
        }
    });

    connect(Experimental_Peony::VolumeManager::getInstance(), &Experimental_Peony::VolumeManager::signal_volumeRenameFinished, this, [=](){
        if (!currentPage()) {
            return;
        }
        QString currentUri = getCurrentUri();
        if (currentUri.startsWith("file:///run/media/") || currentUri.startsWith("file:///media/")) {
            qDebug() << "[TabWidget] Volume renamed, redirecting from" << currentUri << "to computer:///";
            goToUri("computer:///", true, true);
        }
    });
}

bool TabWidget::isMultFile(std::shared_ptr<Peony::FileInfo> info)
{
    if (!info) {
        qDebug() << "file info not valid";
        return false;
    }

    QString uri = info->uri();
    if (uri.startsWith("mult:///") && (!info->isDir())) {
        return true;
    }

    return false;
}

void TabWidget::setCondWidthWithFont(QComboBox *conditionCombox, int fontSize)
{
    QString language = QLocale::system().name();
    if(fontSize < 12){
        conditionCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2);
        //fix bug#166969, Tibetan language not show complete issue
        if ("bo_CN" == language){
            conditionCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2 + 20);
        }
    }else {
        if ("en_US" == language){
            conditionCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2 + 45);
        }else if ("bo_CN" == language){
            conditionCombox->setFixedWidth(TRASH_BUTTON_WIDTH *3);
        }else{
            conditionCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2);
        }
    }
}

void TabWidget::setClassifyWidthWithFont(QComboBox *classifyCombox, int fontSize)
{
    QString language = QLocale::system().name();
    if(fontSize < 12){
        classifyCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2);
        if ("en_US" == language)
            classifyCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2 + 45);
        if ("bo_CN" == language)
            classifyCombox->setFixedWidth(TRASH_BUTTON_WIDTH *2 + 20);
    }else{
        //最大字体最长字符串所需宽度
        classifyCombox->setFixedWidth(TRASH_BUTTON_WIDTH *3 + 10);
        if ("en_US" == language)
            classifyCombox->setFixedWidth(TRASH_BUTTON_WIDTH *3 + 50);
        if ("zh_HK" == language)
            classifyCombox->setFixedWidth(TRASH_BUTTON_WIDTH *3 + 25);
    }
}

void TabWidget::updateSearchFilterHeight()
{
//    m_filter_container->setMaximumHeight(m_search_filter_layout->getMaxinumHeight(m_filter_container->width()));
//    m_conditions_clear_btn->move(m_filter_container->geometry().right() - 46, m_filter_container->geometry().top() + 120);
//    m_conditions_clear_btn->raise();
}

void TabWidget::updateAdvanceShow(bool isVisible)
{
    m_filter_container->setVisible(isVisible);
//    m_file_type_box->setVisible(isVisible);
//    m_file_mtime_box->setVisible(isVisible);
//    m_file_size_box->setVisible(isVisible);
//    m_file_label_box->setVisible(isVisible);
    //m_input_edit->setVisible(isVisible);
    //m_conditions_clear_btn->setVisible(isVisible);
}

void TabWidget::removeCondition()
{
    int count = m_search_filter_layout->count();
    for (int i = count - 1; i >= 1; i--) {
        QLayoutItem *item = m_search_filter_layout->takeAt(i);
        if (item) {
            QWidget *widget = item->widget();
            if (widget) {
                delete widget;
            }
        }
        delete item;
    }
}

void TabWidget::handleSwitchModeAndShow(const QString &uri)
{
    //没有切换目录
    if (getCurrentUri() == uri) {
        //进入空间中
        if (uri.startsWith("idm:///") && uri != "idm:///") {
            m_space_edit_button->hide();
            //默认会updateSearchBar
            Q_EMIT editSpaceModeChanged(false);
        }
        return;
    }

    //默认创建空间
    if (uri == "idm:///New Intelligent Space") {
        m_mode = Create;
        return;
    }

    //进入普通模式或者空间
    if (!uri.startsWith("search:///") && uri != getCurrentUri() && !getCurrentUri().isEmpty()) {
        m_mode = General;

        m_search_title->hide();
        m_home_search->hide();
        m_current_search->hide();
        m_space_edit_button->hide();
        m_add_filter_button->hide();
        m_space_cancel_button->hide();
        m_space_ok_button->hide();
        m_filter_container->hide();
        m_saveas_space_button->hide();
        removeCondition();
        clearConditions();
        clearFileContentConditions();

        //进入空间中
        if (uri.startsWith("idm:///") && uri != "idm:///") {
            m_space_edit_button->hide();
            Q_EMIT editSpaceModeChanged(false);
            //默认会updateSearchBar
        }
    }


    //进入搜索模式
    if (uri.startsWith("search:///") && Create != m_mode) {
        m_mode = Search;
        //默认会updateSearchBar
    }
}

QString TabWidget::getSearchKey() const
{
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
    QString key = mainWindow->getLastSearchKey();
    return key;
}

void TabWidget::handleEditConditionCancel()
{
    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setText(tr("Need to save changes to current space conditions?"));
    QPushButton *notSave = messageBox.addButton(tr("Not Save"), QMessageBox::RejectRole);
    QPushButton *save = messageBox.addButton(tr("Save"), QMessageBox::AcceptRole);
    messageBox.setDefaultButton(save);
    messageBox.exec();
    if (messageBox.clickedButton() == save) {
        m_space_ok_button->click();
    } else if (messageBox.clickedButton() == notSave){
        clearFilterMode();
        Q_EMIT closeEditSpace();
    } else {
        m_mode = General;
    }
}

bool TabWidget::handleEditConditionSwitchPath()
{
    bool isSwitch = true;
    QString currentUri = getCurrentUri();
    if (m_mode != Edit || !currentUri.startsWith("idm://")) {
        return isSwitch;
    }

    QString spaceName = currentUri.remove("idm:///").split('/').first();
    spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
    if (!spaceName.isEmpty()) {
        QString conditions = getConditions();
        QString defaultConditions = Peony::IDMDBusManager::getInstance()->getConditionsBySpaces(spaceName);
        if (conditions != defaultConditions) {
            isSwitch = false;
            handleEditConditionCancel();
            if (m_mode == General && m_add_filter_button && m_add_filter_button->isVisible()) {
                clearFilterMode();
                Q_EMIT closeEditSpace();
            }
        }
        qDebug() << __func__ << m_mode << (conditions == defaultConditions);
    }

    return isSwitch;
}

void TabWidget::clearFilterMode()
{
    m_mode = General;
    m_search_title->hide();
    m_home_search->hide();
    m_current_search->hide();
    m_add_filter_button->hide();
    m_space_cancel_button->hide();
    m_space_ok_button->hide();
    m_space_edit_button->hide();
    removeCondition();
    clearConditions();
    updateAdvanceShow(false);
}

QString TabWidget::elideTextAndToolTip(QFont font, int width, QString strInfo, Qt::TextElideMode mode, QWidget *widgt)
{
    QFontMetrics fontMetrics(font);

    if (fontMetrics.width(strInfo) > width) {
        widgt->setToolTip(strInfo);
        strInfo = QFontMetrics(font).elidedText(strInfo, mode, width);
    }

    return strInfo;
}

bool TabWidget::isDefaultSpaceCondition(const QString &defaultCondition)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(defaultCondition.toLocal8Bit().data());
    QJsonObject rootObject = jsonDocument.object();
    int conditionCount = rootObject["conditions count"].toInt();
    QJsonArray labelsArray = rootObject["labels"].toArray();
    qDebug() << __func__ << conditionCount << labelsArray;
    if (conditionCount == 0 && labelsArray.isEmpty()) {
        return true;
    }
    return false;
}

void TabWidget::updateSearchBarByuri(QString uri)
{
    if (uri.startsWith("idm:///") && uri != "idm:///") {
        // if(m_mode != Create){
        if(m_saveas_space_button->isVisible() && uri == "idm:///New Intelligent Space"){
            m_space_edit_button->hide();
        }else{
            bool visiable  = uri == "idm:///New Intelligent Space";
            m_search_title->setVisible(visiable);
            m_home_search->setVisible(visiable);
            m_current_search->setVisible(visiable);
            m_add_filter_button->setVisible(visiable);
            m_saveas_space_button->setVisible(visiable);
            m_space_edit_button->setVisible(false);
            m_space_edit_button->setChecked(false);
            if (!visiable)
                Q_EMIT editSpaceModeChanged(false);
        }
        //}
    }else{
        m_search_title->hide();
        m_home_search->hide();
        m_current_search->hide();
        m_space_edit_button->hide();
        m_add_filter_button->hide();
        m_space_cancel_button->hide();
        m_space_ok_button->hide();
        m_filter_container->hide();
        m_saveas_space_button->hide();
        removeCondition();
        clearConditions();
        clearFileContentConditions();
        m_jumpToComputer = false;
        m_search_bar_layout->setContentsMargins(10, 0, 10, 0);
        updateAdvanceShow(false);
    }
}

//void TabWidget
void TabWidget::initAdvanceSearch()
{
    //advance search bar
    QHBoxLayout *search = new QHBoxLayout();
    m_search_bar_layout = search;
    // Maybe it is unused
    QToolBar *searchButtons = new QToolBar(this);
    m_search_bar = searchButtons;

    QLabel *title = new QLabel(tr("Location:"), this);
    m_search_title = title;
    title->setFixedWidth(TRASH_BUTTON_WIDTH);
    title->setFixedHeight(TRASH_BUTTON_HEIGHT);

    m_current_search = new QPushButton(this);
//    m_current_search->setFixedWidth(TRASH_BUTTON_WIDTH + 50);
    m_current_search->setFixedHeight(TRASH_BUTTON_HEIGHT);


//    QComboBox *searchTypeCommobox = new QComboBox(searchButtons);
//    m_search_type_box = searchTypeCommobox;
//    searchTypeCommobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
//    searchTypeCommobox->setFixedWidth(TRASH_BUTTON_WIDTH * 3 + 15);
//    auto searchTypeModel = new QStringListModel(searchButtons);
//    searchTypeModel->setStringList(m_search_type_list);
//    searchTypeCommobox->setModel(searchTypeModel);
//    connect(searchTypeCommobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
//       if (index > 0) {
//           //设置过滤关键字
//           MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
//           QString key = mainWindow->getLastSearchKey();
//           qDebug() << __func__ << "key: " << key;
//           this->updateFilterContent(key);
//       } else {
//           bool searchIndex = isSearchIndex();
//           if (!searchIndex) {
//               int ret = QMessageBox::question(nullptr, tr("Search Settings"), tr("After the creation of the index, "
//                                                                                  "the next search can get the results of the document content containing the search term, "
//                                                                                  "during which you can exit the page at any time, "
//                                                                                  "we will continue to complete the creation in the background."));
//               if (QMessageBox::Yes == ret) {
//                   const QByteArray id(UKUI_SEARCH_SCHEMAS);
//                   if (QGSettings::isSchemaInstalled(id)) {
//                       QGSettings *searchSettings = new QGSettings(id, QByteArray(), this);
//                       if (searchSettings && searchSettings->keys().contains(SEARCH_METHOD_KEY)) {
//                           searchSettings->set(SEARCH_METHOD_KEY, true);
//                           searchSettings->set("contentIndexEnable", true);
//                           searchSettings->set("contentIndexEnableOcr", true);
//                       }
//                   }
//               }
//               m_search_type_box->setCurrentIndex(1);
//           }

//           //清除过滤关键字
//           currentPage()->clearFileContentConditions();
//           updateFilter();
//       }
//    });

    m_home_search = new QPushButton(tr("Computer"), this);
//    m_home_search->setFixedWidth(TRASH_BUTTON_WIDTH + 50);
    m_home_search->setFixedHeight(TRASH_BUTTON_HEIGHT);
//    m_home_search->setStyleSheet("border: 1px solid transparent;");

    QString filterDisplay = tr("Filter");
    m_add_filter_button = new QPushButton(QIcon::fromTheme("filter-symbolic"), filterDisplay, this);
    m_add_filter_button->setCheckable(true);
    m_add_filter_button->setFixedHeight(TRASH_BUTTON_HEIGHT);
    m_add_filter_button->setMinimumWidth(TRASH_BUTTON_HEIGHT + 20);
    filterDisplay = this->elideTextAndToolTip(m_add_filter_button->font(), m_add_filter_button->width() - m_add_filter_button->iconSize().width() - PUSH_BUTTON_TOTAL_PADDING - 2, filterDisplay, Qt::ElideRight, m_add_filter_button);
    m_add_filter_button->setText(filterDisplay);
    connect(m_add_filter_button, &QPushButton::toggled, this, [=](bool checked){
        if (checked) {
            if (m_search_filter_layout->count() == 1) {
                addCondition();
            }
            updateAdvanceShow(true);
           //updateSearchFilterHeight();
        } else {
            updateAdvanceShow(false);
        }
    });

    m_saveas_space_button = new QPushButton(tr("Save as Space"), this);
    m_saveas_space_button->setProperty("isImportant", true);
    m_saveas_space_button->setChecked(true);
    m_saveas_space_button->setFixedHeight(TRASH_BUTTON_HEIGHT);
    connect(m_saveas_space_button, &QPushButton::clicked, this, [=](){
        Peony::IDMDBusManager::getInstance()->setActiveWindow(this->topLevelWidget());
        QStringList allUris = currentPage()->getAllFileUris();
        QStringList allPaths;
        for (auto uri : allUris) {
            QString decodeUri = Peony::FileUtils::urlDecode(uri);
            allPaths.append(decodeUri.remove("file://"));
        }
        allPaths.removeDuplicates();
        MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
        QString keyWord = mainWindow->getLastSearchKey();
        QString conditions = getConditions();
        qDebug() << "============" << conditions;
        QWidget *widget = QApplication::topLevelAt(QCursor::pos());
        SpaceSaveDialog *dialog = new SpaceSaveDialog(allPaths, conditions, keyWord,widget);
        dialog->show();
    });

    m_space_cancel_button = new QPushButton(tr("Cancel"), this);
    m_space_cancel_button->setChecked(true);
    m_space_cancel_button->setFixedHeight(TRASH_BUTTON_HEIGHT);

    m_space_ok_button = new QPushButton(tr("Save"), this);
    m_space_ok_button->setChecked(true);
    m_space_ok_button->setFixedHeight(TRASH_BUTTON_HEIGHT);

    m_space_edit_button = new QPushButton(tr("Edit Conditiones"), this);
    m_space_edit_button->setProperty("isImportant", true);
    m_space_edit_button->setFixedHeight(TRASH_BUTTON_HEIGHT);

    //QWidget *m_filter_widget = new QWidget(this);
    m_filter_container = new QWidget(this);
    m_filter_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_filter_container->setContentsMargins(0, 0, 0, 6);

    //m_filter_widget->setFixedHeight(1000);
//    QHBoxLayout *filter_layout = new QHBoxLayout(m_filter_widget);
//    filter_layout->addWidget(m_filter_container);
    //m_filter_widget->setStyleSheet("background-color: #FF0000;");
    m_file_label_model = FileLabelModel::getGlobalModel();
    QList<FileLabelItem *> allLabels = m_file_label_model->getAllFileLabelItems();

    m_search_filter_layout = new QVBoxLayout(this);
    m_search_filter_layout->setSpacing(2);
    m_search_filter_layout->setContentsMargins(0, 0, 0, 0);
    m_fitall_box =  new QComboBox();
    QStringList m_fit_all_list = {tr("Satisfy All Conditions")};
    m_fitall_box->addItems(m_fit_all_list);
    m_fitall_box->setCurrentIndex(0);
    QWidget *rowWidget = new QWidget(this);
    QGridLayout *rowLayout = new QGridLayout(rowWidget);
    rowLayout->addWidget(m_fitall_box, 0, 0);
    rowWidget->setFixedHeight(TRASH_BUTTON_HEIGHT + 8);
    rowLayout->setAlignment(Qt::AlignTop);
    m_fitall_box->setMaximumWidth(200);
    m_fitall_box->setFixedHeight(TRASH_BUTTON_HEIGHT);
//    m_condition_label = new QLabel(tr("Condition"));
//    m_file_type_box = new Peony::MultiSelectComboBox();
//    m_file_type_box->addItems(m_file_type_list);
//    m_file_type_box->setPlaceholderText(tr("File Type"));

//    m_file_mtime_box = new Peony::MultiSelectComboBox();
//    m_file_mtime_box->addItems(m_file_mtime_list);
//    m_file_mtime_box->setPlaceholderText(tr("Modify time"));

//    m_file_size_box = new Peony::MultiSelectComboBox();
//    m_file_size_box->addItems(m_file_size_list);
//    m_file_size_box->setPlaceholderText(tr("File Size"));

//    m_file_label_box = new Peony::MultiSelectComboBox();
//    m_file_label_box->setPlaceholderText(tr("File Label"));

//    for (int i = 0; i < allLabels.size(); ++i) {
//        m_file_label_box->addItem(allLabels.at(i)->name(), false, 0, allLabels.at(i)->color());
//    }

//    m_input_edit = new QLineEdit();
//    m_input_edit->setFixedHeight(TRASH_BUTTON_HEIGHT);
//    m_input_edit->setFixedWidth(TRASH_BUTTON_WIDTH *4);
//    m_input_edit->setPlaceholderText(tr("Please input key words..."));
//    m_input_edit->setText("");
    //m_input_edit->setTextMargins(0, 0, 20, 0);

//    m_conditions_clear_btn = new QPushButton(this);
//    m_conditions_clear_btn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
//    m_conditions_clear_btn->setFixedSize(TRASH_BUTTON_HEIGHT, TRASH_BUTTON_HEIGHT);

//    m_search_filter_layout->addWidget(m_condition_label);
    m_search_filter_layout->addWidget(rowWidget,0, Qt::AlignTop);
//    m_search_filter_layout->addWidget(m_file_type_box);
//    m_search_filter_layout->addWidget(m_file_mtime_box);
//    m_search_filter_layout->addWidget(m_file_size_box);
//    m_search_filter_layout->addWidget(m_file_label_box);
//    m_search_filter_layout->addWidget(m_input_edit);

    m_filter_container->setLayout(m_search_filter_layout);
//   connect(m_file_type_box, &Peony::MultiSelectComboBox::hidingPopup, this, &TabWidget::updateAdvanceConditions);
//    connect(m_file_mtime_box, &Peony::MultiSelectComboBox::hidingPopup, this, &TabWidget::updateAdvanceConditions);
//    connect(m_file_size_box, &Peony::MultiSelectComboBox::hidingPopup, this, &TabWidget::updateAdvanceConditions);
//    connect(m_file_label_box, &Peony::MultiSelectComboBox::hidingPopup, this, &TabWidget::updateAdvanceConditions);
//    connect(m_file_type_box, &Peony::MultiSelectComboBox::showingPopup, this, &TabWidget::updateMultiComboBoxCount);
//    connect(m_file_mtime_box, &Peony::MultiSelectComboBox::showingPopup, this, &TabWidget::updateMultiComboBoxCount);
//    connect(m_file_size_box, &Peony::MultiSelectComboBox::showingPopup, this, &TabWidget::updateMultiComboBoxCount);
//    connect(m_file_label_box, &Peony::MultiSelectComboBox::showingPopup, this, &TabWidget::updateMultiComboBoxCount);
   // connect(m_input_edit, &QLineEdit::textChanged, this, &TabWidget::updateAdvanceConditions);
//    connect(m_conditions_clear_btn, &QPushButton::clicked, this, [=](){
//        for (int i = 0; i < m_conditions_list.size(); ++i) {
//            m_conditions_list.at(i)->uncheckAllItem();
//        }
//        //m_file_label_box->uncheckAllItem();
//        //m_input_edit->setText("");
//        updateAdvanceConditions();
//    });

//    m_conditions_list.append(m_file_type_box);
//    m_conditions_list.append(m_file_size_box);
//    m_conditions_list.append(m_file_mtime_box);

    connect(m_home_search, &QPushButton::clicked, m_home_search, [=]() {
        qDebug() << "m_jumpToComputer  m_home_search" << m_jumpToComputer;
        m_jumpToComputer = true;
        switchSearchPath(false);
    });
    connect(m_current_search, &QPushButton::clicked, m_current_search, [=]() {
        m_jumpToComputer = false;
        switchSearchPath(true);
        qDebug() << "m_jumpToComputer" << m_jumpToComputer;
        if (!m_jumpToComputer) {
            browsePath();
        } else {
            m_jumpToComputer = false;
        }
    });

    connect(m_space_cancel_button, &QPushButton::clicked, this, [=](){
        quitEditMode();
        Q_EMIT closeEditSpace();
    });

    connect(m_space_ok_button, &QPushButton::clicked, this, [=](){
        QString currentUri = getCurrentUri();
        if (currentUri.startsWith("idm://") && currentUri != "idm:///") {
            QString spaceName = currentUri.remove("idm:///");
            spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
            Peony::IDMDBusManager::getInstance()->editSpaceCondition(spaceName, getConditions(false));
            clearFilterMode();
            Q_EMIT closeEditSpace();
        }
    });

    connect(m_space_edit_button, &QPushButton::clicked, this, &TabWidget::switchToEditMode);

    search->addWidget(title, 0, Qt::AlignLeft);
    search->addSpacing(2);
    search->addWidget(m_home_search, 0, Qt::AlignLeft);

    search->addSpacing(10);
    search->addWidget(m_current_search, 0, Qt::AlignLeft);
    search->addSpacing(10);
    search->addStretch(1);
    search->addWidget(searchButtons);
    search->addWidget(m_add_filter_button, 0, Qt::AlignRight);
    search->addSpacing(10);
    search->addWidget(m_space_edit_button, 0, Qt::AlignRight);
    search->addWidget(m_saveas_space_button, 0, Qt::AlignRight);
    search->addWidget(m_space_cancel_button, 0, Qt::AlignRight);
    search->addSpacing(10);
    search->addWidget(m_space_ok_button, 0, Qt::AlignRight);
    search->setContentsMargins(10, 0, 16, 0);
    searchButtons->setVisible(false);
    title->setVisible(false);
    m_current_search->setVisible(false);
    m_home_search->setVisible(false);
    m_add_filter_button->setVisible(false);
    m_saveas_space_button->setVisible(false);
    m_space_cancel_button->setVisible(false);
    m_space_ok_button->setVisible(false);
    m_space_edit_button->setVisible(false);
    addCondition();
    updateAdvanceShow(false);
}

//search conditions changed, update filter
void TabWidget::searchUpdate()
{
    qDebug() <<"searchUpdate:" <<m_search_child_flag;
    auto currentUri = getCurrentUri();
    if (! currentUri.startsWith("search:///"))
    {
        qDebug() << "searchUpdate is not in search path";
        return;
    }

    QString targetUri = currentUri;
    if (m_search_child_flag)
    {
        targetUri = currentUri.replace("&recursive=0", "&recursive=1");
    }
    else
        targetUri = currentUri.replace("&recursive=1", "&recursive=0");

    qDebug() <<"searchUpdate targetUri:" <<targetUri;
    goToUri(targetUri, false, true);
}

void TabWidget::searchChildUpdate()
{
    searchUpdate();

    Q_EMIT this->searchRecursiveChanged(m_search_child_flag);
}

void TabWidget::browsePath()
{
    // use window modal dialog, fix #56549
    QFileDialog f(this->topLevelWidget());
    f.setStyle(nullptr);
    f.setWindowTitle(tr("Select Path"));
    f.setDirectoryUrl(QUrl(""));
    f.setWindowModality(Qt::WindowModal);
    f.setAcceptMode(QFileDialog::AcceptOpen);
    f.setOption(QFileDialog::ShowDirsOnly);
    f.setFileMode(QFileDialog::DirectoryOnly);
    f.setFilter(QDir::System|QDir::AllDirs|QDir::Files|QDir::NoDotAndDotDot);

    auto result = f.exec();
    if (result != QDialog::Accepted) {
        return;
    }

    //Gets the URI of the selected directory. link bug#92521
    QList<QUrl> urls = f.selectedUrls();
    if(urls.isEmpty()){
        return;
    }
    QString target_path = urls.at(0).toString();
//    QString target_path = f.directoryUrl().toString();
//    QString target_path = QFileDialog::getExistingDirectory(this, tr("Select path"), getCurrentUri(), QFileDialog::ShowDirsOnly);
    qDebug()<<"browsePath Opened:"<<target_path;
    //add root prefix
    if (! target_path.contains("file://") && target_path != "")
        target_path = "file://" + target_path;

    if (target_path != "" && target_path != getCurrentUri())
    {
        updateSearchPathButton(target_path);
        /* get search key */
        MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
        QString key=mainWindow->getLastSearchKey();
        Q_EMIT this->updateSearch(target_path,key);
    }
}

void TabWidget::updateStatusBarSliderState()
{
    if (currentPage() && currentPage()->getView()) {
        if (m_isTabletMode) {
            m_status_bar->m_slider->setVisible(false);
        } else {
            bool enable = currentPage()->getView()->supportZoom();
            m_status_bar->m_slider->setEnabled(enable);
            m_status_bar->m_slider->setVisible(enable);
        }
    }
}

void TabWidget::updateTrashBarVisible(const QString &uri)
{
    bool visible = false;
    m_trash_bar_layout->setContentsMargins(10, 0, 10, 0);
    if (uri.indexOf("trash:///") >= 0)
    {
        visible = true;
        m_trash_bar_layout->setContentsMargins(10, 5, 10, 5);
    }

    m_trash_bar->setVisible(visible);
    m_trash_label->setVisible(visible);
    m_clear_button->setVisible(visible);
    m_recover_button->setVisible(visible);
}

void TabWidget::handleZoomLevel(int zoomLevel)
{
    if (!currentPage()) {
        return;
    }

    if (!currentPage()->getView()) {
        return;
    }

    currentPage()->getView()->clearIndexWidget();

    int currentViewZoomLevel = currentPage()->getView()->currentZoomLevel();
    int currentViewMimZoomLevel = currentPage()->getView()->minimumZoomLevel();
    int currentViewMaxZoomLevel = currentPage()->getView()->maximumZoomLevel();
    if (zoomLevel == currentViewZoomLevel) {
        return;
    }

    // save default zoom level
    // Peony::GlobalSettings::getInstance()->setValue(DEFAULT_VIEW_ZOOM_LEVEL, zoomLevel);
    Peony::FileUtils::setViewZoomLevel(getCurrentUri(), zoomLevel);

    if (zoomLevel <= currentViewMaxZoomLevel && zoomLevel >= currentViewMimZoomLevel) {
        currentPage()->getView()->setCurrentZoomLevel(zoomLevel);
    } else {
        //check which view to switch.
        auto directoryViewManager = Peony::DirectoryViewFactoryManager2::getInstance();
        auto viewId = directoryViewManager->getDefaultViewId(zoomLevel, getCurrentUri());
        switchViewType(viewId);
        currentPage()->getView()->setCurrentZoomLevel(zoomLevel);
    }
}
#include"windows/FMWindowIface.h"

#include <KWindowSystem>
void TabWidget::slot_responseUnmounted(const QString &destUri, const QString &sourceUri)
{
    for(int index = 0; index < m_stack->count(); index++)
    {
        int  currentIndex = this->currentIndex();
        QString decodedSrcUri = Peony::FileUtils::urlDecode(sourceUri);
        QString uri = qobject_cast<Peony::DirectoryViewContainer *>(m_stack->widget(index))->getCurrentUri();
        uri = Peony::FileUtils::urlDecode(uri);
        qDebug()<<"decodedSrcUri:"<<decodedSrcUri<<" uri:"<<uri<<" total count: "<<m_stack->count()<<" index:"<<index<<" currentIndex:"<<currentIndex;
        /* 不属于该设备的tab页不处理；属于该设备：文件管理器的当前标签页跳转到计算机页，其余标签页均关闭 */
        if(!decodedSrcUri.isEmpty() && uri.contains(decodedSrcUri) && uri != "file:///" && uri!= "filesafe:///")
        {
            //all window accessed mount path should goto self top path，related to bug#104551
            if((Peony::GlobalSettings::getInstance()->getValue("LAST_FOCUS_PEONY_WINID") == dynamic_cast<MainWindow *>(this->topLevelWidget())->winId()
                ||KWindowSystem::hasWId(dynamic_cast<MainWindow *>(this->topLevelWidget())->winId())
                ||QApplication::topLevelWidgets().contains(this->topLevelWidget()))
                    && index == currentIndex){
                qDebug()<<"sourceUri:"<<sourceUri<<"change to self top path"<<" index:"<<currentIndex;
                if (uri.startsWith("filesafe:///"))
                    this->goToUri("filesafe:///", true, true);  /* 跳转到文件保护箱路径 */
                else
                    this->goToUri(destUri, true, true);/* 跳转到计算机页 */
            }
            else{/* 其余tab页关闭 */
                qDebug()<<"remove tab  uri:"<<uri<<", index:"<<index;
                removeTab(index);
                index--;
            }
        }
    }
}

void TabWidget::updateSearchTypeShow()
{
//    //判断是否建立索引更新默认选项
//    bool index = isSearchIndex();
//    if (index) {
//        m_search_type_box->setCurrentIndex(0);
//    } else {
//        m_search_type_box->setCurrentIndex(1);
//    }
}

void TabWidget::updateFilterContent(const QString &key)
{
    if(!currentPage())
        return;

    currentPage()->clearFileContentConditions();
    if (m_search_rules_currentIndex) {
        currentPage()->addFileContentFilter(key);
    }

    updateFilter();
}

bool TabWidget::isSearchIndex()
{
    bool isSearchIndex = false;
    const QByteArray id(UKUI_SEARCH_SCHEMAS);
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *searchSettings = new QGSettings(id, QByteArray(), this);
        if (searchSettings && searchSettings->keys().contains(SEARCH_METHOD_KEY)) {
            isSearchIndex = searchSettings->get(SEARCH_METHOD_KEY).toBool();
        }
    }

    return isSearchIndex;
}

void TabWidget::setMutipleLabelConditions(QStringList names, QList<QColor> colors)
{
    if(!currentPage())
        return;
    currentPage()->setMutipleLabelConditions(names, colors);
}

void TabWidget::updateMultiComboBoxCount()
{
    if (!currentPage())
        return;

    currentPage()->recalculateAllCounts();

//    QMap<int, int> fileTypeMap = currentPage()->getFileTypeCount();
//    for (int i = 0; i < m_file_type_box->count(); ++i) {
//        m_file_type_box->setCountFromItemIndex(i, fileTypeMap.value(i + 1));
//    }

//    QMap<int, int> fileModifyTimeMap = currentPage()->getFileModifyTimeCount();
//    for (int i = 0; i < m_file_mtime_box->count(); ++i) {
//        m_file_mtime_box->setCountFromItemIndex(i, fileModifyTimeMap.value(i + 1));
//    }

//    QMap<int, int> fileSizeMap = currentPage()->getFileSizeCount();
//    for (int i = 0; i < m_file_size_box->count(); ++i) {
//        m_file_size_box->setCountFromItemIndex(i, fileSizeMap.value(i + 1));
//    }

//    QMap<int, int> fileLabelMap = currentPage()->getFileLabelCount();
//    for (int i = 0; i < m_file_label_box->count(); ++i) {
//        m_file_label_box->setCountFromItemIndex(i, fileLabelMap.value(i));
//    }
}

void TabWidget::clearAllMapsCount()
{
    if(!currentPage())
        return;

    currentPage()->clearAllMapsCount();
    updateFilter();
}

void TabWidget::clearFileContentConditions()
{
    if(!currentPage())
        return;
    currentPage()->clearFileContentConditions();
}

void TabWidget::updateSearchBar(bool showSearch)
{
    qDebug() << "updateSearchBar:" <<showSearch << m_mode;
    if (m_isVirtualTab) {
        m_isVirtualTab = false;
        return;
    }

    if (Create == m_mode){
        return;
    }

    if (showSearch && !qApp->property("tabletMode").toBool())
    {
        m_search_title->show();
        m_search_bar->show();
        m_current_search->show();
        m_home_search->show();
        m_add_filter_button->show();
        m_saveas_space_button->show();
        //文件保护箱里面禁用保存空间按钮
        QString currentUri =  getCurrentUri();
        if(currentUri.startsWith("search:///")){
            currentUri.remove("search:///search_uris=");
        }
        m_saveas_space_button->setEnabled(!currentUri.startsWith("filesafe:///"));
        m_space_edit_button->hide();
        m_space_cancel_button->hide();
        m_space_ok_button->hide();
        m_search_bar_layout->setContentsMargins(10, 5, 10, 5);
        updateSearchPathButton(getCurrentUri());
        if (m_jumpToComputer) {
            switchSearchPath(false);
        } else {
            switchSearchPath(true);
        }
        //m_search_type_box->show();
        updateSearchTypeShow();
        // if (m_search_filter_layout->count() == 1) {
        //     addCondition();
        // }
    }
    else
    {
        m_search_title->hide();
        m_search_bar->hide();
        m_current_search->hide();
        m_home_search->hide();
        m_saveas_space_button->hide();
        m_space_edit_button->hide();
        m_space_cancel_button->hide();
        m_space_ok_button->hide();
        m_add_filter_button->setChecked(false);
        m_add_filter_button->hide();
        m_jumpToComputer = false;
        m_search_bar_layout->setContentsMargins(10, 0, 10, 0);
        updateAdvanceShow(false);
    }

    if (! showSearch)
    {
        //exit advance search, clear search conditions
        clearConditions();
        clearFileContentConditions();
        updateFilter();

        /**
         * @bug #242210: [File Manager] Enter the folder and click search, switch the search directory,
         *               repeat the second time you can not switch the search directory.
         *
         * Closing the search without resetting MainWindow's m_last_search_path
         * to null prevents the search from being performed
         *
         * @author: Renyg <renyangguang@kylinos.cn>
         * @date: %{CurrentDate::yyyy-MM-dd}
         */
        MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
        if(mainWindow)
        {
            mainWindow->clearLastSearchPath();
        }
    }

    //9X0 changes, set default as true, fix bug#70916
    //enableSearchBar(true);
}

void TabWidget::updateCurrentSearchPath()
{
    QString currentUri = getCurrentUri();
    if (!currentUri.endsWith("///")) {
        GFile* file = g_file_new_for_uri(currentUri.toStdString().c_str());
        currentUri = g_file_peek_path (file);
        QString displayName = currentUri.right(currentUri.count() - currentUri.lastIndexOf("/") - 1);
        m_current_search->setText(displayName);
        m_current_search->adjustSize();
        g_object_unref(file);
    }
    else {
        QString displayName = currentUri.left(currentUri.indexOf(":"));
        m_current_search->setText(displayName);
        m_current_search->adjustSize();
    }
}

void TabWidget::switchSearchPath(bool isCurrent)
{
    if (isCurrent) {
        m_home_search->setFlat(true);
        m_current_search->setFlat(false);
        // m_home_search->setStyleSheet("border: 1px solid transparent;");
        //        m_current_search->setStyleSheet("border: 1px solid transparent;"
        //                                        "border-bottom: 1px solid gray;");
        Q_EMIT this->globalSearch(false);
//        searchChildUpdate(false);
    }
    else {
        m_home_search->setFlat(false);
        m_current_search->setFlat(true);
        //m_current_search->setStyleSheet("border: 1px solid transparent;");
        //        m_home_search->setStyleSheet("border: 1px solid transparent;"
        //                                     "border-bottom: 1px solid gray;");
        Q_EMIT this->globalSearch(true);
//        searchChildUpdate(true);
    }
}

void TabWidget::updateSearchPathButton(const QString &uri)
{
    QString curUri = uri;
    if (uri.startsWith("search:///")) {
        // Extract the actual path from the search URI
        curUri = Peony::SearchVFSUriParser::getSearchUriPath(uri);
    } else if (uri.isEmpty()) {
        curUri = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        if (!getCurrentUri().isNull()) {
            curUri = getCurrentUri();
        }
    }

    // Update file information
    auto info = Peony::FileInfo::fromUri(curUri);
    //m_search_button_info = info;
    if (info.get()->isEmptyInfo()) {
        Peony::FileInfoJob j(info);
        j.querySync();
    }

    // Get the current display name
    auto displayName = Peony::FileUtils::getFileDisplayName(curUri);
    if (displayName.contains("&")) {
        displayName = Peony::FileUtils::handleSpecialSymbols(displayName);
    }

    // Check if updates are needed
    QString currentRealDisplayName = m_current_search->property("realDisplayName").toString();
    qDebug() << "curUri: " << curUri << " displayName: " << displayName << " currentRealDisplayName: " << currentRealDisplayName;
    if (currentRealDisplayName == displayName) {
        // If the display name has not changed, return directly
        return;
    }

    //elide text if it is too long, Use ElideMiddle mode to design
    //related bug#155126, #185743
    m_current_search->setProperty("realDisplayName", displayName);
    // if (curUri.startsWith("idm://") && curUri != "idm:///") {
    //     if (!m_jumpToComputer) {
    //         QString spaceName = curUri.remove("idm:///");
    //         displayName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
    //     }
    // }
    QString elidedDisplayName = fontMetrics().elidedText(displayName, Qt::ElideMiddle, /*m_current_search->width()*/200 - m_search_bar->iconSize().width() - PUSH_BUTTON_TOTAL_PADDING);
    m_current_search->setText(elidedDisplayName);
    m_current_search->adjustSize();
    if (elidedDisplayName != displayName) {
        // QFont font = m_current_search->font();
        // displayName = Peony::FileUtils::wrapTextForTooltip(displayName,QCursor::pos(),font);
         m_current_search->setToolTip(displayName);
     } else {
         m_current_search->setToolTip("");
    }
}

Peony::DirectoryViewContainer *TabWidget::currentPage()
{
    return qobject_cast<Peony::DirectoryViewContainer *>(m_stack->currentWidget());
}

const QString TabWidget::getCurrentUri()
{
    if (!currentPage())
        return nullptr;
    return currentPage()->getCurrentUri();
}

const QStringList TabWidget::getCurrentSelections()
{
    if (!currentPage())
        return QStringList();
    return currentPage()->getCurrentSelections();
}

const int TabWidget::getCurrentRowcount()
{
    return currentPage()->getCurrentRowcount();
}

const QStringList TabWidget::getAllFileUris()
{
    if (!currentPage())
        return QStringList();
    return currentPage()->getAllFileUris();
}

const int TabWidget::getAllDisplayFileCount()
{
    if (!currentPage() || !currentPage()->getView())
        return 0;

    int count = 0;
    auto iface = Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(currentPage()->getView());
    if(iface)
        count = iface->getAllDisplayFileCount();

    return count;
}

const QStringList TabWidget::getBackList()
{
    if (!currentPage())
        return QStringList();
    return currentPage()->getBackList();
}

const QStringList TabWidget::getForwardList()
{
    if (!currentPage())
        return QStringList();
    return currentPage()->getForwardList();
}

bool TabWidget::canGoBack()
{
    if (!currentPage())
        return false;
    return currentPage()->canGoBack();
}

bool TabWidget::canGoForward()
{
    if (!currentPage())
        return false;
    return currentPage()->canGoForward();
}

bool TabWidget::canCdUp()
{
    if (!currentPage())
        return false;
    return currentPage()->canCdUp();
}

int TabWidget::getSortType()
{
    //fix switch to computer view and back change to default sort issue, link to bug#92261
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        auto sortType = settings->isExist(SORT_COLUMN)? settings->getValue(SORT_COLUMN).toInt() : 0;
        if (getCurrentUri() != "trash:///" && sortType == 4) {
            sortType = 0;
        }
        if(!getCurrentUri().startsWith("idm:///") && sortType >= 7){
            sortType = 0;
        }
        return sortType;
    } else {
        auto metaInfo = Peony::FileMetaInfo::fromUri(getCurrentUri());
        if (!metaInfo) {
            qWarning()<<"no meta info"<<getCurrentUri();
            Peony::FileInfoJob j(getCurrentUri());
            j.querySync();
            metaInfo = Peony::FileMetaInfo::fromUri(getCurrentUri());
        }
        auto sortType = metaInfo->getMetaInfoVariant(SORT_COLUMN).isValid()? metaInfo->getMetaInfoInt(SORT_COLUMN): 0;
        QString currentUri = getCurrentUri();
        if (currentUri.startsWith("idm://")) {
            QString spaceName = currentUri.remove("idm:///");
            spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
            QVariantHash config = Peony::IDMDBusManager::getInstance()->getSpacesSortConfig(spaceName);
            sortType = config.value("sortType").toInt();
            qDebug() << __func__ << currentUri << sortType;
        }
        return sortType;
    }

//    if (!currentPage())
//        return 0;
//    return currentPage()->getSortType();
}

Qt::SortOrder TabWidget::getSortOrder()
{
    //fix switch to computer view and back change to default sort issue, link to bug#92261
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        auto sortOrder = settings->isExist(SORT_ORDER)? settings->getValue(SORT_ORDER).toInt() : 1;

        return Qt::SortOrder(sortOrder);
    } else {
        auto metaInfo = Peony::FileMetaInfo::fromUri(getCurrentUri());
        if (!metaInfo) {
            qWarning()<<"no meta info"<<getCurrentUri();
            Peony::FileInfoJob j(getCurrentUri());
            j.querySync();
            metaInfo = Peony::FileMetaInfo::fromUri(getCurrentUri());
        }
        auto sortOrder = metaInfo->getMetaInfoVariant(SORT_ORDER).isValid()? metaInfo->getMetaInfoInt(SORT_ORDER): 1;
        QString currentUri = getCurrentUri();
        if (currentUri.startsWith("idm://")) {
            QString spaceName = currentUri.remove("idm:///");
            spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
            QVariantHash config = Peony::IDMDBusManager::getInstance()->getSpacesSortConfig(spaceName);
            sortOrder = config.value("sortOrder").toInt();
        }
        return Qt::SortOrder(sortOrder);
    }

//    if (!currentPage())
//        return Qt::AscendingOrder;
//    return currentPage()->getSortOrder();
}

bool TabWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Resize) {
        updateStatusBarGeometry();
    }
    return false;
}

void TabWidget::setCurrentIndex(int index)
{
    m_tab_bar->setCurrentIndex(index);
    m_stack->setCurrentIndex(index);
}

void TabWidget::setPreviewPage(Peony::PreviewPageIface *previewPage)
{
    bool visible = false;
    auto previewPageWidget = dynamic_cast<QWidget *>(previewPage);
    if (previewPageWidget)
        visible = true;

    if (m_preview_page) {
        m_preview_page_container->removeWidget(m_preview_page_container->widget(0));
        m_preview_page->closePreviewPage();
    }

    m_preview_page = previewPage;

    if (m_preview_page) {
        previewPageWidget->setParent(m_preview_page_container);
        m_preview_page_container->addWidget(previewPageWidget);
        updatePreviewPage();
    }

    m_preview_page_container->blockSignals(!visible);
    m_preview_page_container->setVisible(visible);

    this->topLevelWidget()->setProperty("isPreviewMode", visible);
}

void TabWidget::addPage(const QString &uri, bool jumpTo)
{
    //story 28077, control start of peony
    if (!MANAGER_CONTROL->checkCanCreateNewView()) {
        return;
    }

    setCursor(QCursor(Qt::WaitCursor));

    auto info = Peony::FileInfo::fromUri(uri);
    auto infoJob = new Peony::FileInfoJob(info);
    //infoJob->setAutoDelete();

    connect(infoJob, &Peony::FileInfoJob::queryAsyncFinished, this, [=](){
        infoJob->deleteLater();
        QString rootDir = info.get()->uri();
        if (info.get()->uri().startsWith("file:///") && !info.get()->isDir()) {
            rootDir = Peony::FileUtils::getParentUri(rootDir);
        }

        auto enumerator = new Peony::FileEnumerator;
        enumerator->setEnumerateDirectory(rootDir);
        //enumerator->setAutoDelete();
        connect(enumerator, &Peony::FileEnumerator::cancelled, this, [=](){
            if (!currentPage()) {
                QTimer::singleShot(100, topLevelWidget(), &QWidget::close);
            }else{
                this->refresh();
            }
            enumerator->deleteLater();
        });
        connect(enumerator, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
            if (critical) {
                QWidget *parent = Peony::FileUtils::getMainWindow();
                QMessageBox msgBox(parent);
                msgBox.setWindowModality(Qt::WindowModal);
                if (G_IO_ERROR_NOT_FOUND == err->code() || G_IO_ERROR_EXISTS == err->code()) {
                    msgBox.setIcon(QMessageBox::Warning);
                    msgBox.setWindowTitle(tr("Warning"));
                } else {
                    msgBox.setIcon(QMessageBox::Critical);
                    msgBox.setWindowTitle(tr("Error"));
                }
                msgBox.setText(err->message());
                msgBox.exec();
                //QMessageBox::critical(0, 0, err.get()->message());
                setCursor(QCursor(Qt::ArrowCursor));
                // if there is no active page, window should be closed to avoid crash. link to: #48031
                if (!currentPage()) {
                    this->topLevelWidget()->close();
                }
                enumerator->deleteLater();
                return;
            }
            auto viewContainer = new Peony::DirectoryViewContainer(m_stack);
            viewContainer->setProperty("statusBarHeight", qApp->fontMetrics().height() + 10);
            bool hasCurrentPage = currentPage();
            bool hasView = false;
            if (hasCurrentPage)
                hasView = currentPage()->getView();
            int zoomLevel = -1;

            if (hasCurrentPage) {
                // perfer to use current page view type
                auto internalViews = Peony::DirectoryViewFactoryManager2::getInstance()->internalViews();
                //fix continuously click add button quickly crash issue, bug #41425
                if (hasView && internalViews.contains(currentPage()->getView()->viewId()))
                    viewContainer->switchViewType(currentPage()->getView()->viewId());

                if (hasView && hasCurrentPage) {
                    hasCurrentPage = true;
                    zoomLevel = currentPage()->getView()->currentZoomLevel();
                }
            } else {
                // viewContainer->switchViewType(Peony::GlobalSettings::getInstance()->getValue(DEFAULT_VIEW_ID).toString());
                QString viewId = Peony::DirectoryViewFactoryManager2::getInstance()->getDefaultViewId(uri);
                viewContainer->switchViewType(viewId);
            }
            viewContainer->setMinimumWidth(520);
            m_stack->addWidget(viewContainer);
            if (jumpTo) {
                m_stack->setCurrentWidget(viewContainer);
            }

            //process open symbolic link
            auto realUri = uri;
            if (info->isSymbolLink() && info->symlinkTarget().length() >0 && uri.startsWith("file://")) {
                realUri = "file://" + info->symlinkTarget();
            } else if (!info->isDir() && !realUri.startsWith("smb://") && !realUri.startsWith("label://") ) {
                realUri = Peony::FileUtils::getParentUri(uri);
            }

            //Fix bug#132638, special # character use in symbolic link open fail issue
            if (realUri.contains("\#") && ! realUri.startsWith("filesafe:///") && !realUri.startsWith("search:///"))
                realUri = Peony::FileUtils::urlEncode(realUri);

            // fix #174653
            if (realUri.isEmpty()) {
                realUri = "file:///";
            }

            if (!t.isEmpty()) {
                realUri = t;
            }

            //m_stack->addWidget(viewContainer);
            viewContainer->goToUri(realUri, false, true);

            if (!info->isDir() && Peony::FileUtils::isFileExsit(uri)) {
                QTimer::singleShot(500, [=] {
                    viewContainer->getView()->setSelections(QStringList() << uri);
                });
            }

            bindContainerSignal(viewContainer);
            updateTrashBarVisible(uri);


            if (zoomLevel > 0)
                viewContainer->getView()->setCurrentZoomLevel(zoomLevel);
            else{
                //viewContainer->getView()->setCurrentZoomLevel(Peony::GlobalSettings::getInstance()->getValue(DEFAULT_VIEW_ZOOM_LEVEL).toInt());
                int defaultZoomLevel = Peony::FileUtils::getViewZoomLevel(realUri);
                viewContainer->getView()->setCurrentZoomLevel(defaultZoomLevel);
            }
            //auto viewContainer = new Peony::DirectoryViewContainer(m_stack);
            auto settings = Peony::GlobalSettings::getInstance();
            if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
                auto sortType = settings->isExist(SORT_COLUMN)? settings->getValue(SORT_COLUMN).toInt(): 0;
                auto sortOrder = settings->isExist(SORT_ORDER)? settings->getValue(SORT_ORDER).toInt(): 1;
                viewContainer->setSortType(Peony::FileItemModel::ColumnType(sortType));
                viewContainer->setSortOrder(Qt::SortOrder(sortOrder));
            } else {
                auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
                auto sortType = metaInfo->getMetaInfoVariant(SORT_COLUMN).isValid()? metaInfo->getMetaInfoInt(SORT_COLUMN): 0;
                auto sortOrder = metaInfo->getMetaInfoVariant(SORT_ORDER).isValid()? metaInfo->getMetaInfoInt(SORT_ORDER): 1;
                if (uri.startsWith("idm:///")) {
                    QString spaceName = uri.mid(7);
                    spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
                    sortType = Peony::IDMDBusManager::getInstance()->getSpacesSortType(spaceName);
                    sortOrder = Peony::IDMDBusManager::getInstance()->getSpacesSortOrder(spaceName);
                }
                viewContainer->setSortType(Peony::FileItemModel::ColumnType(sortType));
                viewContainer->setSortOrder(Qt::SortOrder(sortOrder));
            }

            m_tab_bar->addPage(realUri, jumpTo);
            updateTabAllPages();
            updateTabBarGeometry();
            enumerator->deleteLater();

            if (realUri != uri) {
                m_tab_bar->updateLocation(m_tab_bar->count() - 1, realUri);
            }
        });
        enumerator->prepare();
    });

    infoJob->queryAsync();
}

void TabWidget::goToUri(const QString &uri, bool addHistory, bool forceUpdate)
{
    // 发射目录切换信号，用于取消AI轮询等操作
    Q_EMIT directoryChanging();

    if (!currentPage()) {
        // do not trigger go to uri if there is no active page, avoid crash. link to: #45684
        return;
    }
    qDebug() << "goToUri:" << uri;
    currentPage()->goToUri(uri, addHistory, forceUpdate);
    m_tab_bar->updateLocation(m_tab_bar->currentIndex(), uri);
    updateTrashBarVisible(uri);
    updatePreviewPage();
}

void TabWidget::updateTabPageTitle()
{
    qDebug() << "updateTabPageTitle:" <<getCurrentUri();

/* hotfix bug#169196 【文件管理器】插入/弹出U盘，文件管理器当前界面显示异常 */
//    //fix error for glib2 signal: G_FILE_MONITOR_EVENT_DELETED
//    if("trash:///" == getCurrentUri()){
//        Peony::VolumeManager* vm = Peony::VolumeManager::getInstance();
//        connect(vm,&Peony::VolumeManager::volumeRemoved,this,[=](const std::shared_ptr<Peony::Volume> &volume){
//            refresh();
//        });
//        connect(vm,&Peony::VolumeManager::volumeAdded,this,[=](const std::shared_ptr<Peony::Volume> &volume){
//            refresh();
//        });
//    }

    m_tab_bar->updateLocation(m_tab_bar->currentIndex(), getCurrentUri().toLocal8Bit());
    //m_tab_bar->updateLocation(m_tab_bar->currentIndex(), QUrl::fromPercentEncoding(getCurrentUri().toLocal8Bit()));
    updateTrashBarVisible(getCurrentUri());
    updateStatusBarSliderState();
}

void TabWidget::switchViewType(const QString &viewId)
{
    if(!currentPage()||!(currentPage()->getView()))
        return;

    if (currentPage()->getView()->viewId() == viewId)
        return;

    currentPage()->switchViewType(viewId);

    // change default view id
    auto factoryManager = Peony::DirectoryViewFactoryManager2::getInstance();
    auto internalViews = factoryManager->internalViews();
    if (internalViews.contains(viewId)){
        //Peony::GlobalSettings::getInstance()->setValue(DEFAULT_VIEW_ID, viewId);
        if (Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_VIEW).toBool()) {
            Peony::GlobalSettings::getInstance()->setValue(DEFAULT_VIEW_ID, viewId);
        } else {
            QString currUri = getCurrentUri();
            auto metaInfo = Peony::FileMetaInfo::fromUri(currUri);
            if (!metaInfo) {
                qWarning()<<"no meta info"<<currUri;
                Peony::FileInfoJob j(currUri);
                j.querySync();
                metaInfo = Peony::FileMetaInfo::fromUri(currUri);
            }
            metaInfo->setMetaInfoString(DEFAULT_VIEW_ID, viewId);
        }
    }

    this->m_status_bar->m_slider->setEnabled(this->currentPage()->getView()->supportZoom());
}

void TabWidget::goBack()
{
    if(!currentPage())
        return;
    currentPage()->goBack();
}

void TabWidget::goForward()
{
    if(!currentPage())
        return;
    currentPage()->goForward();
}

void TabWidget::cdUp()
{
    if(!currentPage())
        return;
    currentPage()->cdUp();
}

void TabWidget::refresh()
{
    if(!currentPage())
        return;
    currentPage()->refresh();
}

void TabWidget::stopLoading()
{
    if(!currentPage())
        return;
    currentPage()->stopLoading();
}

void TabWidget::tryJump(int index)
{
    if(!currentPage())
        return;
    currentPage()->tryJump(index);
}

void TabWidget::clearHistory()
{
    if(!currentPage())
        return;
    currentPage()->clearHistory();
}

void TabWidget::setSortType(int type)
{
    if(!currentPage())
        return;
    currentPage()->setSortType(Peony::FileItemModel::ColumnType(type));
}

void TabWidget::setSortOrder(Qt::SortOrder order)
{
    if(!currentPage())
        return;
    currentPage()->setSortOrder(order);
}

void TabWidget::setSortFilter(int FileTypeIndex, int FileMTimeIndex, int FileSizeIndex)
{
    if(!currentPage())
        return;
    currentPage()->setSortFilter(FileTypeIndex, FileMTimeIndex, FileSizeIndex);
}

void TabWidget::setShowHidden(bool showHidden)
{
    if(!currentPage())
        return;
    currentPage()->setShowHidden(showHidden);
}

void TabWidget::setUseDefaultNameSortOrder(bool use)
{
    if(!currentPage())
        return;
    currentPage()->setUseDefaultNameSortOrder(use);
}

void TabWidget::setSortFolderFirst(bool folderFirst)
{
    if(!currentPage())
        return;
    currentPage()->setSortFolderFirst(folderFirst);
}

void TabWidget::addFilterCondition(int option, int classify, bool updateNow)
{
    if(!currentPage())
        return;
    currentPage()->addFilterCondition(option, classify, updateNow);
}

void TabWidget::removeFilterCondition(int option, int classify, bool updateNow)
{
    if(!currentPage())
        return;
    currentPage()->removeFilterCondition(option, classify, updateNow);
}

void TabWidget::clearConditions()
{
    if(!currentPage())
        return;
    currentPage()->clearConditions();
}

void TabWidget::updateTabAllPages()
{
    m_model->clear();
    auto uris = m_tab_bar->getCurrentUris();
    for(int i = 0; i < uris.size(); ++i) {
        auto uri = uris.at(i);
        QList<QStandardItem*> items1;
        QStandardItem *item1 = new QStandardItem;
        QStandardItem *item2 = new QStandardItem(QIcon::fromTheme("window-close-symbolic"),"");
        item1->setIcon(QIcon::fromTheme(Peony::FileUtils::getFileIconName(uri)));
        QString displayName = Peony::FileUtils::getFileDisplayName(uri);
        item1->setText(displayName);
        item1->setData(displayName, Qt::ToolTipRole);
        items1.append(item1);
        items1.append(item2);
        m_model->appendRow(items1);
    }
    if (m_treeView) {
        delete m_treeView;
    }
    m_treeView = new QTreeView(this);
    m_treeView->setWindowFlag(Qt::Popup);
    m_treeView->move(m_show_page_button->x(), m_tab_bar->sizeHint().height());
    m_treeView->setHeaderHidden(true);
    m_treeView->header()->setStretchLastSection(true);
    m_treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    m_treeView->header()->setMinimumSectionSize(2);
    m_treeView->setSelectionBehavior(QTreeView::SelectRows);
    m_treeView->setEditTriggers(QTreeView::NoEditTriggers);
    m_treeView->setAttribute(Qt::WA_TranslucentBackground);
    m_treeView->setFrameStyle(QFrame::Sunken);
    m_treeView->setIndentation(0);
    m_treeView->setModel(m_model);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // m_treeView->setViewportMargins(0,0,0,0);
    m_treeView->setContentsMargins(0,0,0,0);
    m_treeView->verticalScrollBar()->move(m_treeView->width() - m_treeView->verticalScrollBar()->width(), 0);
    int rowCount = m_model->rowCount();
    int rowHeight = m_treeView->model()->rowCount() > 0 ? m_treeView->sizeHintForRow(0) : 36;
    if (rowHeight > 0) {
        int headerHeight = m_treeView->header()->isVisible() ? m_treeView->header()->height() : 0;
        int padding = 4;
        int totalHeight = rowCount * rowHeight + headerHeight + padding;
        int maxVisibleRows = 20;
        int maxHeight = maxVisibleRows * rowHeight + headerHeight + padding;
        // m_treeView->resize(m_treeView->width(), (qMin(totalHeight,maxHeight)));
        m_treeView->setFixedWidth(192);
        m_treeView->setFixedHeight(qMin(totalHeight,maxHeight));
    } else {
        m_treeView->setFixedHeight(360);
    }
    m_treeView->header()->resizeSection(0, 152);
    m_treeView->header()->resizeSection(1, 20);
}

void TabWidget::updateFilter()
{
    if(!currentPage())
        return;
    currentPage()->updateFilter();
    m_status_bar->update();
}

void TabWidget::updateAdvanceConditions()
{
    clearConditions();
    int count = m_search_filter_layout->count();
    if(count <= 1)
        return;
    for (int i = 1; i < count; ++i) {
          QLayoutItem *item = m_search_filter_layout->itemAt(i);
          auto widget = item->widget();
          QGridLayout *rowLayout = static_cast<QGridLayout *>(widget->layout());
          auto typecombobox = rowLayout->itemAtPosition(0, 0)->widget();
          int type = static_cast<QComboBox *>(typecombobox)->currentIndex();
          if(type + 1 == FileName)
              continue;
          auto selectcombobox = rowLayout->itemAtPosition(0, 2)->widget();
          int select = static_cast<QComboBox *>(selectcombobox)->currentIndex();
          addFilterCondition(type, select);
//        QList<int> selectIndexs = m_conditions_list.at(i)->getSelectItemsIndex();
//        if (!selectIndexs.isEmpty()) {
//            for (int j = 0; j < selectIndexs.size(); j++) {
//                addFilterCondition(i, selectIndexs.at(j) + 1);
//            }
//        }
    }

//    QStringList selectTexts = m_file_label_box->getSelectItemsText();
//    if (!selectTexts.isEmpty()) {
//        QList<QColor> colors;
//        for (auto text : selectTexts) {
//            QColor tmpColor = FileLabelModel::getGlobalModel()->getLableColorFromLabelName(text);
//            if (tmpColor != Qt::transparent) {
//                colors.append(tmpColor);
//            }
//        }
//        setMutipleLabelConditions(selectTexts, colors);
//    }

    for(auto key : m_input_edit_keyList)
    {
        if(!currentPage())
            continue;
        if (key != "")
            currentPage()->addFileNameFilter(key);
    }

    updateFilter();
}

void TabWidget::setCurrentSelections(const QStringList &uris)
{
    if (!currentPage() || !currentPage()->getView()) {
        qWarning()<<"can not set current selection, current page is invalid. maybe not ready?";
        return;
    }
    currentPage()->getView()->setSelections(uris);
    if (uris.count() >0)
        currentPage()->getView()->scrollToSelection(uris.first());
}

void TabWidget::editUri(const QString &uri)
{
    currentPage()->getView()->editUri(uri);
}

void TabWidget::editUris(const QStringList &uris)
{
    currentPage()->getView()->editUris(uris);
}

void TabWidget::onViewDoubleClicked(const QString &uri)
{
    qDebug()<<"tab widget double clicked"<<uri;
    //auto info = Peony::FileInfo::fromUri(uri);
    // fix #206224
    auto info = Peony::FileInfoManager::getInstance()->findFileInfoByUri(uri);
    if (!info) {
        info = Peony::FileInfo::fromUri(uri);
    }

#ifdef MULTI_DISABLE
    if (isMultFile(info)) {
        qDebug() << "Mult video or audio file, do not open";
        QMessageBox::warning(nullptr, "", tr("Opening such files is not currently supported"));//暂时不支持打开此类文件
        return;
    }
#endif

    if (info->uri().startsWith("trash://")) {
        QMainWindow *w = Peony::PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<uri);
        //auto w = new Peony::PropertiesWindow(QStringList()<<uri);
        //w->show();
        Peony::PropertiesWindowFactoryPluginManager::getInstance()->show();
        return;
    }
    if (info->isDir() || info->isVolume() || info->isVirtual()) {
        if(info->uri().startsWith("file://")
                && !info->canExecute()){
            QMessageBox::critical(m_parent, tr("Open failed"),
                                  tr("Open directory failed, you have no permission!"));
            return;
        }

        bool check = Peony::GlobalSettings::getInstance()->getValue(SHOW_IN_NEW_WINDOW).toBool();
        if (check && info->isDir()) {
            //task#147390  新建窗口来打开文件夹
            auto window = dynamic_cast<Peony::FMWindowIface *>(this->topLevelWidget());
            auto newWindow = dynamic_cast<QWidget *>(window->create(uri));
            newWindow->show();
        } else {
            Q_EMIT this->updateWindowLocationRequest(uri, true);
        }
    } else if (IMAGE_MOUNT_MANAGER->isImageFile(uri)) {
        IMAGE_MOUNT_MANAGER->mountImage(uri);
    }
    else {
        Peony::FileLaunchManager::openAsync(uri, false, false);
    }
}

void TabWidget::changeCurrentIndex(int index)
{
    //fix bug#291259, when index is -1 crash issue
    if (index < 0 || index >= m_tab_bar->getCurrentUris().count()) {
        return;
    }

    m_tab_bar->setCurrentIndex(index);
    m_stack->setCurrentIndex(index);

    QString uri = m_tab_bar->getCurrentUris().at(index);
    /**
     * @bug #292259: Various abnormal phenomena that occur when switching tabs in search mode
     *
     * separate handling of tab switching scenarios that deal with search states
     *
     * @author: Renyg <renyangguang@kylinos.cn>
     * @date:   2025-01-06
     */
    bool isSearching = uri.startsWith("search:///");
    QString searchKey;
    if (isSearching) {
        searchKey = Peony::SearchVFSUriParser::getSearchUriNameRegexp(uri);
        // emit a signal to update the search status of HeaderBar
        Q_EMIT searchStateChanged(isSearching, searchKey);
    } else {
        closeSearch();
    }
    updateSearchBarByuri(uri);
    Q_EMIT currentIndexChanged(index);
    Q_EMIT activePageChanged();
}

int TabWidget::count()
{
    return m_stack->count();
}

int TabWidget::currentIndex()
{
    return m_stack->currentIndex();
}

void TabWidget::moveTab(int from, int to)
{
    auto w = m_stack->widget(from);
    if (!w)
        return;
    m_stack->removeWidget(w);
    m_stack->insertWidget(to, w);
    Q_EMIT tabMoved(from, to);
}

void TabWidget::removeTab(int index)
{
    auto widget = m_stack->widget(index);
    m_stack->removeWidget(widget);
    widget->deleteLater();
    m_tab_bar->removeTab(index);
//    m_tab_bar->removeTabUris(index);
    if (m_model) {
        m_model->removeRow(index);
    }
    if (m_stack->count() > 0)
        Q_EMIT activePageChanged();
}

#include <KWindowSystem>
void TabWidget::bindContainerSignal(Peony::DirectoryViewContainer *container)
{
    connect(container, &Peony::DirectoryViewContainer::signal_responseUnmounted, this,&TabWidget::slot_responseUnmounted);
    connect(container, &Peony::DirectoryViewContainer::updateWindowLocationRequest, this, &TabWidget::updateWindowLocationRequest);
    connect(container, &Peony::DirectoryViewContainer::directoryChanged, this, &TabWidget::activePageLocationChanged);
    connect(container, &Peony::DirectoryViewContainer::selectionChanged, this, &TabWidget::activePageSelectionChanged);
    connect(container, &Peony::DirectoryViewContainer::viewTypeChanged, this, &TabWidget::activePageViewTypeChanged);
    connect(container, &Peony::DirectoryViewContainer::viewDoubleClicked, this, &TabWidget::onViewDoubleClicked);
    connect(container, &Peony::DirectoryViewContainer::menuRequest, this, &TabWidget::menuRequest);
    connect(container, &Peony::DirectoryViewContainer::zoomRequest, this, &TabWidget::zoomRequest);
    connect(container, &Peony::DirectoryViewContainer::setZoomLevelRequest, m_status_bar, &TabStatusBar::updateZoomLevelState);
    connect(container, &Peony::DirectoryViewContainer::updateStatusBarSliderStateRequest, this, [=]() {
        this->updateStatusBarSliderState();
    });

    connect(container, &Peony::DirectoryViewContainer::updateWindowSelectionRequest, this, [=](const QStringList &uris){
        if (container == currentPage()) {
            Q_EMIT this->updateWindowSelectionRequest(uris);
        }
    });
    connect(container, &Peony::DirectoryViewContainer::signal_itemAdded, this, [=](const QString& uri){
        if (container == currentPage())
            Q_EMIT this->signal_itemAdded(uri);
    });

    connect(container, &Peony::DirectoryViewContainer::statusBarChanged, this, [=](){
        m_status_bar->update();
    });

    connect(container, &Peony::DirectoryViewContainer::signal_updateTabPageTitle, this, [=](const QString& uri){
        for(int index = 0; index < m_stack->count(); index++){
            if(uri.startsWith("label:///") && uri == m_tab_bar->tabData(index).toString()){
                m_tab_bar->updateLocation(index, uri.toLocal8Bit());
            }
        }
    });

    connect(container, &Peony::DirectoryViewContainer::signal_updateLocationBar, this, [=](const QString& uri){
        auto mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
        for(int index = 0; index < m_stack->count(); index++){
            if(uri.startsWith("label:///") && uri == m_tab_bar->tabData(index).toString()){
                mainWindow->updateHeaderBar();
            }
        }
    });
}

void TabWidget::updatePreviewPage()
{
    if (!m_preview_page)
        return;
    auto selection = getCurrentSelections();
    m_preview_page->cancel();
    if (selection.isEmpty())
        return ;
    m_preview_page->prepare(selection.first());
    m_preview_page->startPreview();
}

void TabWidget::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    updateTabBarGeometry();
    updateStatusBarGeometry();
    //updateSearchFilterHeight();
}

void TabWidget::updateTabBarGeometry()
{
    //204 = 48 * 4 + 12   4个按钮每个48px，相互间隔4px
    //168 + 12 = 180
    quint32 windowButtonsWidth = 180;
    if (Peony::GlobalSettings::getInstance()->getProjectName() == V10_SP1_EDU) {
        //windowButtonsWidth -= 52;
        //cherry-pick commit:339dbaf18b9555d274e69c0589a755457e3f555b, 为解决冲突加入的下一行
        windowButtonsWidth = 148;
    }
    //更新添加控件的位置
    int addPageX = 0;
    int tabBarWidth = 0;
    if (m_tab_bar->sizeHint().width() + 2 > this->width() - m_add_page_button->width() - windowButtonsWidth) {
        tabBarWidth = this->width() - m_add_page_button->width() - windowButtonsWidth;
        addPageX = this->width() - m_add_page_button->width() - windowButtonsWidth;
    } else {
        tabBarWidth = this->width() - windowButtonsWidth;
        addPageX = m_tab_bar->sizeHint().width() + 2;

        if (m_tab_bar->count() > 1 && m_tab_bar->m_slide_animation->state() != QAbstractAnimation::Stopped) {
            auto tabWidth = m_tab_bar->tabSizeHint(m_tab_bar->count() - 1).width();
            addPageX -= tabWidth * (1 - m_tab_bar->m_slide_animation->currentValue().toReal());
        }
    }

    if (layoutDirection() == Qt::LeftToRight)
        m_tab_bar->setGeometry(0, 1, tabBarWidth, m_tab_bar->sizeHint().height());
    else
        m_tab_bar->setGeometry(this->width() - addPageX + 2, 1, addPageX, m_tab_bar->sizeHint().height());
    m_tab_bar->raise();

    if (Peony::GlobalSettings::getInstance()->getProjectName() == V10_SP1_EDU) {
        if (layoutDirection() == Qt::LeftToRight)
            m_add_page_button->move(addPageX, 0);
        else
            m_add_page_button->move(this->width() - addPageX - m_add_page_button->width(), 0);
        if (tabBarWidth == addPageX) {
            m_show_page_button->show();
            if (layoutDirection() == Qt::LeftToRight)
                m_show_page_button->move(addPageX + 40, 0);
            else
                m_show_page_button->move(this->width() - addPageX - 40 - m_show_page_button->width(), 0);
        } else {
            m_show_page_button->hide();
        }
    } else {
        auto lastTabRect =  m_tab_bar->rect();
        int fixedY = lastTabRect.center().y() - m_add_page_button->height()/2 + 1;
        if (layoutDirection() == Qt::LeftToRight)
            m_add_page_button->move(addPageX + 6, fixedY);
        else
            m_add_page_button->move(this->width() - addPageX - m_add_page_button->width() + 6, fixedY);
        if (tabBarWidth == addPageX) {
            m_show_page_button->show();
            if (layoutDirection() == Qt::LeftToRight)
                m_show_page_button->move(addPageX + 36, fixedY);
            else
                m_show_page_button->move(this->width() - addPageX - 36 - m_show_page_button->width(), fixedY);
        } else {
            m_show_page_button->hide();
        }
    }

    m_add_page_button->raise();
    m_show_page_button->raise();
}

void TabWidget::updateStatusBarGeometry()
{
    auto font = qApp->font();
    QFontMetrics fm(font);
    int x = width() - m_stack->width();

    auto sideBarDock = findChild<QDockWidget *>("peony_NavigationSideBarContainer");
    if (sideBarDock) {
        x = sideBarDock->width() + 8;
    }

    if (layoutDirection() == Qt::RightToLeft) {
        x = 0;
    }

    m_status_bar->setGeometry(x, this->height() - fm.height() - 16 , m_stack->width() + 4, fm.height() + 10);
    m_status_bar->raise();
    if (Peony::GlobalSettings::getInstance()->getValue(ZOOM_SLIDER_VISIBLE).toBool() && !m_isTabletMode) {
        if (currentPage()) {
            if (auto view = currentPage()->getView()) {
                bool supportZoom = view->supportZoom();
                m_status_bar->m_slider->setEnabled(supportZoom);
                m_status_bar->m_slider->setVisible(supportZoom);
            }
        }
    } else {
        m_status_bar->m_slider->hide();
    }
}

void TabWidget::paintEvent(QPaintEvent *e)
{
    //bug#95007 打开预览窗口，有分割线
    QPainter painter(this);
    auto handle = m_preview_splitter->handle(1);
    auto handlePoint = handle->mapTo(this, QPoint());
    QPainterPath path;
    path.addRect(handlePoint.x(),handlePoint.y(), handle->size().width(),handle->size().height());
    path.setFillRule(Qt::FillRule::WindingFill);
    painter.fillPath(path, this->palette().window().color());

    QMainWindow::paintEvent(e);
}

const QList<std::shared_ptr<Peony::FileInfo>> TabWidget::getCurrentSelectionFileInfos()
{
    const QStringList uris = getCurrentSelections();
    QList<std::shared_ptr<Peony::FileInfo>> infos;
    for(auto uri : uris) {
        auto info = Peony::FileInfo::fromUri(uri);
        infos<<info;
    }
    return infos;
}

const QList<std::shared_ptr<Peony::FileInfo>> TabWidget::getCurrentAllFileInfos()
{
    const QStringList uris = getAllFileUris();
    QList<std::shared_ptr<Peony::FileInfo>> infos;
    for(auto uri : uris) {
        auto info = Peony::FileInfo::fromUri(uri);
        infos<<info;
    }
    return infos;
}

void TabWidget::updateTabletModeValue(bool isTabletMode)
{
    //task#106007 【文件管理器】文件管理器应用做平板UI适配，去掉页签
    m_isTabletMode = isTabletMode;
    m_tab_bar->setVisible(!isTabletMode);
    m_add_page_button->setVisible(!isTabletMode);
    m_status_bar->m_slider->hide();
}

void TabWidget::addCondition(const QJsonObject &rootObject, const QJsonObject &object)
{
    int defaultType = m_search_filter_layout->count();
    if(defaultType < 1)
        return;

    if (!object.isEmpty()) {
        defaultType = Type(object["condition type"].toInt());
    }

    if (!rootObject.isEmpty() && object.isEmpty()) {
        defaultType = Type::FileLabel;
    }

    auto typeCombobox = new FilterComboBox(this);
    typeCombobox->addItems(m_option_types);
    typeCombobox->setCurrentIndex(defaultType - 1);
    if(defaultType > 6)
        typeCombobox->setCurrentIndex(0);

    typeCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
    QWidget *rowWidget = new QWidget(this);
    QGridLayout *rowLayout = new QGridLayout(rowWidget);
    rowLayout->addWidget(typeCombobox, 0, 0);
    rowWidget->setFixedHeight(TRASH_BUTTON_HEIGHT + 8);
    rowLayout->setAlignment(Qt::AlignTop);
    rowWidget->setLayout(rowLayout);
    connect(typeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        qDebug() << "defaultType" << defaultType << index;
        switchType(Type(index + 1), rootObject, object, rowWidget);
    });
    switchType(Type(defaultType), rootObject, object, rowWidget);
    QPushButton *addconditonbtn = new QPushButton(QIcon::fromTheme("list-add-symbolic"), "");
    addconditonbtn->setFixedSize(QSize(TRASH_BUTTON_HEIGHT, TRASH_BUTTON_HEIGHT));
    addconditonbtn->setIconSize(QSize(16, 16));
    addconditonbtn->setFlat(true);
    rowLayout->addWidget(addconditonbtn, 0, 4, Qt::AlignTop | Qt::AlignRight); // 第0行，第1列
    //rowLayout->setAlignment(addconditonbtn, Qt::AlignTop |Qt::AlignRight);
    QPushButton *delconditonbtn = new QPushButton(QIcon::fromTheme("list-remove-symbolic"), "");
    delconditonbtn->setFixedSize(QSize(TRASH_BUTTON_HEIGHT, TRASH_BUTTON_HEIGHT));
    delconditonbtn->setIconSize(QSize(16, 16));
    delconditonbtn->setFlat(true);

    int count = m_search_filter_layout->count();
    if(count == 1){
        delconditonbtn->setDisabled(true);
    }
    rowLayout->addWidget(delconditonbtn, 0, 5, Qt::AlignTop | Qt::AlignRight); // 第0行，第1列
    rowLayout->setColumnStretch(3, 1); // 第2列可拉伸
    rowLayout->setColumnStretch(2, 0); // 第2列可拉伸
    rowLayout->setColumnStretch(1, 0); // 第2列可拉伸
    rowLayout->setColumnStretch(0, 0); // 第2列可拉伸
    m_search_filter_layout->setAlignment(Qt::AlignTop);
    m_search_filter_layout->addWidget(rowWidget, 0, Qt::AlignTop);
    connect(addconditonbtn, &QPushButton::clicked, this, [=](){
        //int count = m_search_filter_layout->count();
        m_filter_container->hide();
        addCondition();
        m_filter_container->show();
        updatebtnstatus();
    });
    connect(delconditonbtn, &QPushButton::clicked, this, [=](){
        m_filter_container->hide();
        m_search_filter_layout->removeWidget(rowWidget);
        m_filter_container->show();
        rowWidget->deleteLater();
        //updateAdvanceConditions();
        updatebtnstatus();
    });
}

void TabWidget::switchType(const Type conditionType, const QJsonObject &rootObject, const QJsonObject &object, QWidget *widget)
{
    int type = 0;
    bool contains = true;
    int containIndex = 0;
    QString keyWords = "";
    QStringList labels;
    if (!object.isEmpty()) {
        type = object["type"].toInt();
        contains = object["contains"].toBool();
        containIndex = contains ? 0 : 1;
        if (Type::FileName == conditionType) {
            QStringList keyWordLists;
            QJsonArray keyWordsArray = object["keywords"].toArray();
            for (auto array : keyWordsArray) {
                keyWordLists.append(array.toString());
            }
            keyWords = keyWordLists.join(",");
        }
    }
    if (!rootObject.isEmpty()) {
        if (Type::FileLabel == conditionType) {
            QJsonArray labelsArray = rootObject["labels"].toArray();
            for (auto arrry : labelsArray) {
                labels.append(arrry.toString());
            }
        }
    }
    QGridLayout *rowLayout = static_cast<QGridLayout *>(widget->layout());
    if (rowLayout->columnCount() > 1) {
        auto widget1 = rowLayout->itemAtPosition(0, 1)->widget();
        auto widget2 = rowLayout->itemAtPosition(0, 2)->widget();
        rowLayout->removeItem(rowLayout->itemAtPosition(0, 1));
        rowLayout->removeItem(rowLayout->itemAtPosition(0, 2));
        widget1->deleteLater();
        widget2->deleteLater();
    }
    QStringList selections = {tr("Contains"), tr("Not Contains")};
    QStringList matchrules = {tr("FileName and FileContent"), tr("Only FileName")};

    auto includeExcludeCombobox = new FilterComboBox(this);
    includeExcludeCombobox->addItems(selections);
    if (Type::ConditionMatchRule == conditionType || Type::FileLabel == conditionType) {
        includeExcludeCombobox->setEnabled(false);
    }
    includeExcludeCombobox->setCurrentIndex(containIndex);
    includeExcludeCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
    rowLayout->addWidget(includeExcludeCombobox, 0, 1);
    connect(includeExcludeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
        Peony::IDMDBusManager::getInstance()->updateSearchCondition(getConditions(false));
    });

    switch (conditionType) {
    case Type::FileName:{
        auto nameEdit = new QLineEdit;
        rowLayout->addWidget(nameEdit, 0, 2);
        nameEdit->setText(keyWords);
        nameEdit->setFixedHeight(TRASH_BUTTON_HEIGHT);
        connect(nameEdit, &QLineEdit::textChanged, this, [=](){
            m_input_edit_keyList = nameEdit->text().split(",");
            //updateAdvanceConditions();
            Peony::IDMDBusManager::getInstance()->updateSearchCondition(getConditions(false));
        });
        break;
    }
    case Type::FileSize:{
        auto fileSizeCombobox = new FilterComboBox(this);
        rowLayout->addWidget(fileSizeCombobox, 0, 2);
        fileSizeCombobox->addItems(m_file_size_list);
        fileSizeCombobox->setCurrentIndex(type);
        fileSizeCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
        connect(fileSizeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
            //updateAdvanceConditions();
            Peony::IDMDBusManager::getInstance()->updateSearchCondition(getConditions(false));
        });
        break;
    }
    case Type::DateRange:{
        auto fileDateRangeCombobox = new FilterComboBox(this);
        rowLayout->addWidget(fileDateRangeCombobox, 0, 2);
        fileDateRangeCombobox->addItems(m_file_mtime_list);
        fileDateRangeCombobox->setCurrentIndex(type);
        fileDateRangeCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
        connect(fileDateRangeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
            //updateAdvanceConditions();
            Peony::IDMDBusManager::getInstance()->updateSearchCondition(getConditions(false));
        });
        break;
    }
    case Type::FileLabel:{
        auto fileLabelCombobox = new Peony::MultiSelectComboBox(this, false);
        fileLabelCombobox->setPlaceholderText(tr("File smart tags"));
        rowLayout->addWidget(fileLabelCombobox, 0, 2);
        //编辑时设置默认选项
        QStringList allLabels = Peony::IDMDBusManager::getInstance()->getIntelligentLabel();
        bool hasLabel = !allLabels.isEmpty();
        if (Edit == m_mode) {
            if(allLabels.isEmpty() && !labels.isEmpty()){
                fileLabelCombobox->addItems(labels);
            }else{
                fileLabelCombobox->addItems(allLabels);
            }
            fileLabelCombobox->setDefaultItems(labels);
        }else{
            fileLabelCombobox->addItems(allLabels);
        }
        fileLabelCombobox->setDisabled(!hasLabel);
        fileLabelCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
        connect(fileLabelCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
            QString key = getSearchKey();
            QString uri = getCurrentUri();
            QString spaceName = uri.remove("idm:///");
            spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
            if (!key.isEmpty()) {
                Peony::IDMDBusManager::getInstance()->reSearchWithLabels(fileLabelCombobox->getSelectItemsText(), spaceName, key);
            }
        });
        break;
//        auto fileLabelCombobox = new Peony::MultiSelectComboBox(this);
//        rowLayout->addWidget(fileLabelCombobox, 0, 2);
//        QList<FileLabelItem *> allLabels = m_file_label_model->getAllFileLabelItems();
//        for (int i = 0; i < allLabels.size(); ++i) {
//            fileLabelCombobox->addItem(allLabels.at(i)->name(), false, 0, allLabels.at(i)->color());
//        }
//        //fileLabelCombobox->addItems(m_file_mtime_list);
//        fileLabelCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
//        connect(fileLabelCombobox, &Peony::MultiSelectComboBox::hidingPopup, this, [=]() {
//            QStringList selectTexts = fileLabelCombobox->getSelectItemsText();
//            if (!selectTexts.isEmpty()) {
//                QList<QColor> colors;
//                for (auto text : selectTexts) {
//                    QColor tmpColor = FileLabelModel::getGlobalModel()->getLableColorFromLabelName(text);
//                    if (tmpColor != Qt::transparent) {
//                        colors.append(tmpColor);
//                    }
//                }
//                setMutipleLabelConditions(selectTexts, colors);
//            }
//        });
//        break;
    }
    case Type::ConditionMatchRule:{
        auto conditionMatchRuleCombobox = new FilterComboBox(this);
        rowLayout->addWidget(conditionMatchRuleCombobox, 0, 2);
        conditionMatchRuleCombobox->addItems(matchrules);
        conditionMatchRuleCombobox->setItemData(0, matchrules[0], Qt::ToolTipRole);
        conditionMatchRuleCombobox->setItemData(1, matchrules[1], Qt::ToolTipRole);
        // 可选：确保 ComboBox 宽度适配内容
        conditionMatchRuleCombobox->setCurrentIndex(type);
        conditionMatchRuleCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
        conditionMatchRuleCombobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        QFontMetrics fm(conditionMatchRuleCombobox->font());
        int maxwidth = 0;
        for(const QString& text:matchrules){
            int textwidth = fm.horizontalAdvance(text);
            maxwidth = qMax(maxwidth,textwidth);
        }
        conditionMatchRuleCombobox->setMinimumWidth(maxwidth + 80);
        connect(conditionMatchRuleCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            m_search_rules_currentIndex = index;
            if (index > 0) {
                //设置过滤关键字
                MainWindow *mainWindow = dynamic_cast<MainWindow *>(this->topLevelWidget());
                QString key = mainWindow->getLastSearchKey();
                qDebug() << __func__ << "key: " << key;
                this->updateFilterContent(key);
            } else {
                bool searchIndex = isSearchIndex();
                if (!searchIndex) {
                    int ret = QMessageBox::question(nullptr, tr("Search Settings"), tr("After the creation of the index, "
                                                                                       "the next search can get the results of the document content containing the search term, "
                                                                                       "during which you can exit the page at any time, "
                                                                                       "we will continue to complete the creation in the background."));
                    if (QMessageBox::Yes == ret) {
                        const QByteArray id(UKUI_SEARCH_SCHEMAS);
                        if (QGSettings::isSchemaInstalled(id)) {
                            QGSettings *searchSettings = new QGSettings(id, QByteArray(), this);
                            if (searchSettings && searchSettings->keys().contains(SEARCH_METHOD_KEY)) {
                                searchSettings->set(SEARCH_METHOD_KEY, true);
                                searchSettings->set("contentIndexEnable", true);
                                searchSettings->set("contentIndexEnableOcr", true);
                            }
                        }
                    }
                    conditionMatchRuleCombobox->setCurrentIndex(1);
                }

                //清除过滤关键字
                currentPage()->clearFileContentConditions();
                updateFilter();
            }
            Peony::IDMDBusManager::getInstance()->updateSearchCondition(getConditions(false));
            //updateAdvanceConditions();
        });
        break;
    }
    default:{
        auto fileTypeCombobox = new FilterComboBox(this);
        rowLayout->addWidget(fileTypeCombobox, 0, 2);
        if(getCurrentUri().startsWith("idm")){
            fileTypeCombobox->addItems(m_file_type_list_nofolder);
        }else{
            fileTypeCombobox->addItems(m_file_type_list);
        }
        fileTypeCombobox->setCurrentIndex(type);
        fileTypeCombobox->setFixedHeight(TRASH_BUTTON_HEIGHT);
        connect(fileTypeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
            //updateAdvanceConditions();
            qDebug() << "===========" << getConditions();
            Peony::IDMDBusManager::getInstance()->updateSearchCondition(getConditions(false));
        });
        break;
    }
    }
    //updateAdvanceConditions();
}

QString TabWidget::getConditions(const bool isDefaultCondition) const {
    QJsonObject configObj;
    // 获取标记配置
    int count = m_search_filter_layout->count() - 1;
    //增加默认关键字配置
    auto window = dynamic_cast<MainWindow *>(this->topLevelWidget());
    QString keyWords = window->getLastSearchKey();
    int defaultCount = 0;
    if (!keyWords.isEmpty() && isDefaultCondition) {
        defaultCount = 1;
        QStringList words = keyWords.split(',');
        QJsonObject fileNameObj;
        fileNameObj["condition type"] = int(Type::FileName);
        fileNameObj["contains"] = true;
        fileNameObj["keywords"] = QJsonArray::fromStringList(words);
        fileNameObj["orCondition"] = false;
        configObj[QString::number(0)] = fileNameObj;
    }

    // 获取条件配置
    configObj["conditions count"] = count + defaultCount;
    int j = 1;
    for (int i = 1; i <= count; ++i) {
        QLayoutItem *item = m_search_filter_layout->itemAt(i);
        auto widget = item->widget();
        QGridLayout *gridLayout = static_cast<QGridLayout *>(widget->layout());
        if (gridLayout->columnCount() < 3) {
            return "";
        }
        //auto condition = QSharedPointer<Condition>(new Condition(m_type));
        QWidget *widget1 = gridLayout->itemAtPosition(0, 1)->widget();
        QWidget *widget2 = gridLayout->itemAtPosition(0, 2)->widget();
        int type = static_cast<QComboBox *>(gridLayout->itemAtPosition(0, 0)->widget())->currentIndex();
        bool contains = qobject_cast<QComboBox *>(widget1)->currentIndex() == 0 ? true : false;

        switch (Type(type + 1)) {
        case Type::FileName: {
            auto edit = qobject_cast<QLineEdit *>(widget2);
            QString text = edit->text();
            QStringList keywords = text.split(',');
            if (!keywords.isEmpty()) {
                QJsonObject fileNameObj;
                fileNameObj["condition type"] = int(Type::FileName);
                fileNameObj["contains"] = contains;
                fileNameObj["keywords"] = QJsonArray::fromStringList(keywords);
                fileNameObj["orCondition"] = false;
                configObj[QString::number(j - 1 + defaultCount)] = fileNameObj;
                j++;
            }
            break;
        }
        case Type::FileType: {
            auto typeComboBox = qobject_cast<QComboBox *>(widget2);
            int type = typeComboBox->currentIndex();
            QJsonObject fileTypeObj;
            fileTypeObj["condition type"] = int(Type::FileType);
            fileTypeObj["contains"] = contains;
            fileTypeObj["type"] = type;
            fileTypeObj["orCondition"] = false;
            configObj[QString::number(j - 1 + defaultCount)] = fileTypeObj;
            j++;
            break;
        }
        case Type::FileSize: {
            auto typeComboBox = qobject_cast<QComboBox *>(widget2);
            int type = typeComboBox->currentIndex();
            QJsonObject fileSizeObj;
            fileSizeObj["condition type"] = int(Type::FileSize);
            fileSizeObj["contains"] = contains;
            fileSizeObj["type"] = type;
            fileSizeObj["orCondition"] = false;
            configObj[QString::number(j - 1 + defaultCount)] = fileSizeObj;
            j++;
            break;
        }
            //            case Condition::FileContent: {
            //                    auto edit = qobject_cast<QLineEdit *>(widget2);
            //                    QString text = edit->text();
            //                    if(text.isEmpty())
            //                        m_isContentNull = true;
            //                    condition->setFileContentCondition(text);
            //                    break;
            //                }
            //            case Condition::SearchLocation: {
            //                    auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
            //                    bool contains = includeExculdeCombobox->currentIndex() == 0;
            //                    auto edit = qobject_cast<QLineEdit *>(widget2);
            //                    QString path = edit->text();
            //                    if(path.isEmpty())
            //                        m_isContentNull = true;
            //                    condition->setSearchLocationCondition(contains, path);
            //                    break;
            //                }
        case Type::DateRange: {
            QJsonObject dateRangObj;
            dateRangObj["condition type"] = int(Type::DateRange);
            auto typeComboBox = qobject_cast<QComboBox *>(widget2);
            int type = typeComboBox->currentIndex();
            dateRangObj["contains"] = contains;
            dateRangObj["type"] = type;
            dateRangObj["orCondition"] = false;
            configObj[QString::number(j - 1 + defaultCount)] = dateRangObj;
            j++;
            break;
        }
        case Type::FileLabel: {
            auto typeComboBox = qobject_cast<Peony::MultiSelectComboBox *>(widget2);
            QStringList labels = typeComboBox->getSelectItemsText();
            labels.sort();
            configObj["conditions count"] = count + defaultCount - 1;
            configObj["labels"] = QJsonArray::fromStringList(labels);
            break;
        }
        case Type::ConditionMatchRule: {
            QJsonObject conditionMatchRuleObj;
            conditionMatchRuleObj["condition type"] = int(Type::ConditionMatchRule);
            auto typeComboBox = qobject_cast<QComboBox *>(widget2);
            int type = typeComboBox->currentIndex();
            conditionMatchRuleObj["contains"] = contains;
            conditionMatchRuleObj["type"] = int(type);
            conditionMatchRuleObj["orCondition"] = false;
            configObj[QString::number(j - 1 + defaultCount)] = conditionMatchRuleObj;
            j++;
            break;
        }
        default:
            break;
        }
    }
    QJsonDocument doc(configObj);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);
    return jsonStr;
}

void TabWidget::updateConditionFormJson()
{
    removeCondition();
    QString currentUri = getCurrentUri();
    if (currentUri.isEmpty())
        return;

    QString spaceName = currentUri.remove("idm:///");
    spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
    QString jsonString = Peony::IDMDBusManager::getInstance()->getConditionsBySpaces(spaceName);
    qDebug() << "==========" << currentUri << jsonString;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
    if (jsonDocument.isNull()) {
        qDebug() << "==========Json Null";
    }
    QJsonObject rootObject = jsonDocument.object();
    int conditionCount = rootObject["conditions count"].toInt();
    QJsonArray labelsArray = rootObject["labels"].toArray();
    qDebug() << __func__ << conditionCount;
    if (conditionCount == 0 && labelsArray.isEmpty()) {
        addCondition();
    } else {
        for (int i = 0; i < conditionCount; ++i) {
            auto conditionObj = rootObject[QString::number(i)].toObject();
            addCondition(rootObject, conditionObj);
        }
    }
    if (!labelsArray.isEmpty()) {
        addCondition(rootObject);
    }
    updatebtnstatus();
}

void TabWidget::updateCreateVirtualTabShow()
{
    m_mode = Create;
    m_isVirtualTab = true;
    m_search_title->show();
    m_search_bar->show();
    m_current_search->show();
    m_home_search->show();
    removeCondition();
    m_add_filter_button->show();
    m_add_filter_button->setChecked(false);
    m_saveas_space_button->show();
    m_space_edit_button->hide();
    m_space_cancel_button->hide();
    m_space_ok_button->hide();
    m_search_bar_layout->setContentsMargins(10, 5, 10, 5);
    updateSearchPathButton(getCurrentUri());
    switchSearchPath(false);
    updateSearchTypeShow();
}

void TabWidget::switchToEditMode()
{
    m_mode = Edit;
    m_search_title->show();
    m_home_search->setChecked(true);
    m_current_search->show();
    m_space_edit_button->hide();
    m_add_filter_button->setChecked(true);
    m_add_filter_button->show();
    m_space_cancel_button->show();
    m_space_ok_button->show();
    m_filter_container->show();
    updateConditionFormJson();
    Q_EMIT editSpaceModeChanged(true);
}

void TabWidget::quitEditMode()
{
    QString currentUri = getCurrentUri();
    if (currentUri.startsWith("idm://") && currentUri != "idm:///") {
        QString spaceName = currentUri.remove("idm:///").split('/').first();
        spaceName = QString(QUrl::fromPercentEncoding(spaceName.toUtf8()));
        if (!spaceName.isEmpty()) {
            QString conditions = getConditions();
            QString defaultConditions = Peony::IDMDBusManager::getInstance()->getConditionsBySpaces(spaceName);
            if (conditions != defaultConditions && !isDefaultSpaceCondition(defaultConditions)) {
                handleEditConditionCancel();
            }
            qDebug() << __func__ << m_mode << (conditions == defaultConditions);
        }
        clearFilterMode();
    }
}

void TabWidget::updatebtnstatus()
{
    int count = m_search_filter_layout->count();
    if(count == 2)
    {
        for (int i = 1; i < count; ++i) {
            QLayoutItem *item = m_search_filter_layout->itemAt(i);
            auto widget = item->widget();
            QGridLayout *rowLayout = static_cast<QGridLayout *>(widget->layout());
            auto delbtn = rowLayout->itemAtPosition(0, 5)->widget();
            delbtn->setDisabled(true);
        }
    }
    else if(count >= 10) {
        for (int i = 1; i < count; ++i) {
            QLayoutItem *item = m_search_filter_layout->itemAt(i);
            auto widget = item->widget();
            QGridLayout *rowLayout = static_cast<QGridLayout *>(widget->layout());
            auto addbtn = rowLayout->itemAtPosition(0, 4)->widget();
            addbtn->setDisabled(true);
        }
    }
    else{
        for (int i = 1; i < count; ++i) {
            QLayoutItem *item = m_search_filter_layout->itemAt(i);
            auto widget = item->widget();
            QGridLayout *rowLayout = static_cast<QGridLayout *>(widget->layout());
            auto addbtn = rowLayout->itemAtPosition(0, 4)->widget();
            auto delbtn = rowLayout->itemAtPosition(0, 5)->widget();
            addbtn->setDisabled(false);
            delbtn->setDisabled(false);
        }
    }
}

PreviewPageContainer::PreviewPageContainer(QWidget *parent) : QStackedWidget(parent)
{

}

PreviewPageButtonGroups::PreviewPageButtonGroups(QWidget *parent) : QButtonGroup(parent)
{
    setExclusive(true);
}
