/*
 * Peony Intelligent Spaces Service
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: He JinQin <hejinqin@kylinos.cn>
 *
 */

#include "search-view-container.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include"QHeaderView"
#include<file-launch-manager.h>
#include<QUuid>
#include<QProcess>
#include<QStandardPaths>
#include <file-watcher.h>
#include <file-utils.h>
#include "search-vfs-uri-parser.h"
using namespace Peony;

SearchViewContainer::SearchViewContainer(QWidget *parent,QString uri):m_defaultUri(uri)
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setObjectName("_searchview_layout");
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    m_view = new SearchView(this);

    DirectoryViewHelper * viewHelper = DirectoryViewHelper::globalInstance();
    viewHelper->addListViewWithDirectoryViewWidget(m_view, this);
    //m_view->setModel(m_model);
    m_clientId = QUuid::createUuid().toString();//分配客户端ID
    layout->addWidget(m_view);
    layout->addSpacing(6);
    setLayout(layout);
    m_signalTransInterface = new QDBusInterface("com.peony.idm.service",
                                                     "/org/peony/idm/space/service",
                                                     "org.peony.idm.modelcontroller");

    QDBusReply<int> reply = m_signalTransInterface->call("getAinotAvailabletype");
    aiConditionType = AIConditionType(reply.value());
    isAiConditionAvialable = !(aiConditionType == aiIndexNotFit);
    connect(m_view, &QTreeView::doubleClicked, this, &SearchViewContainer::onItemDoubleClicked);
    connect(m_view, &QTreeView::clicked, this, [this](const QModelIndex &index){
        QModelIndex realindex =  m_proxyModel->mapToSource(index);
        quintptr id = realindex.internalId();
        if(!isAiConditionAvialable && m_model->isContentItem(realindex) && m_model->hasDefaultContent()){
            requestShowAISubsystemDialog();
        }
    });
    connect(m_view, &SearchView::menuRequest, this,[this](const QPoint &pos){
         Q_EMIT menuRequest(pos);
    });
    m_searchTrigger = new QTimer(this);
    m_searchTrigger->setSingleShot(true);
    m_searchTrigger->setInterval(500); // 500ms防抖

    connect(m_searchTrigger, &QTimer::timeout, this, &SearchViewContainer::performSearch);

}

SearchViewContainer::~SearchViewContainer()
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_view->model());
    if(model) {
        model->clear();
    }
    delete m_view;
    if (m_model) {
        Q_EMIT m_model->cancelBatchQuery();
        m_model->cancelFindChildren();
        m_model->clearResult();
    }
    delete m_model;
    m_searchTrigger->stop();
    m_searching = false;
    m_signalTransInterface->call("cancelSearch");
}

const QString SearchViewContainer::getDirectoryUri()
{
    return m_currentUri;
}

const QStringList SearchViewContainer::getSelections()
{
   QModelIndexList selectedIndexs =  m_view->getSelectedIndexes();
   QStringList selections;
   for (auto index:selectedIndexs) {
       if(!index.parent().isValid())
           continue;
       QModelIndex realindex =  m_proxyModel->mapToSource(index);
       QString uri = m_model->data(realindex,Qt::UserRole).toString();
       selections<<uri;
   }
   return selections;
}

void SearchViewContainer::bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel)
{
    auto layout = findChild<QVBoxLayout *>("_searchview_layout");
    bool ok = false;
    if (parentWidget()) {
        int statusBarHeight = parentWidget()->property("statusBarHeight").toInt(&ok);
        if (ok) {
            layout->setContentsMargins(0, 0, 0, statusBarHeight);
        }
    }
    disconnect(m_model);
    disconnect(m_proxyModel);
    SearchFileItemModel* searchmodel = new SearchFileItemModel();
    m_proxyModel = new SearchFilterProxyModel(m_view);
    m_proxyModel->setSourceModel(searchmodel);
    m_view->setModel(m_proxyModel);
    m_model = searchmodel;
    m_model->setClientId(m_clientId);
    // 配置视图
    m_view->setAnimated(true);
    m_view->setSortingEnabled(true);
    m_view->expandAll();
    m_view->setColumnWidth(0, 400);
    //m_view->setIndentation(40);
    m_view->resizeColumnToContents(0);
    m_view->header()->setSectionResizeMode(0, QHeaderView::Interactive);  // 第一列可交互调整
    m_view->header()->setSectionResizeMode(1, QHeaderView::Interactive);  // 第二列可交互调整
    m_view->header()->setSectionResizeMode(2, QHeaderView::Interactive);  // 第三列可交互调整
    m_view->header()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_view->header()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_view->setTextElideMode(Qt::ElideRight);
    //m_view->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // 或者让所有列按比例拉伸
    m_view->header()->setStretchLastSection(false);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);//启用右键选中
    m_delegate = new TreeviewDelegate(m_view);
    m_view->setItemDelegate(m_delegate);

    connect(m_delegate, &TreeviewDelegate::isEditing, this, [=](const bool &editing){
        m_view->m_delegate_editing = editing;
    });

    //添加索引跳转 如果条件不满足
    m_model->setAiCondition(isAiConditionAvialable);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qDebug() << "条件更新信号创建失败:" << connection.lastError().message();
    }
    connection.connect("com.peony.idm.service", "/org/peony/idm/space/service", "org.peony.idm.modelcontroller",
                       "searchConditionChanged", m_proxyModel,SLOT(updateconditions(QString)));
    connect(searchmodel,&SearchFileItemModel::searchFinished,this,[=](){
         m_view->reUpdateScrollBar();
         Q_EMIT viewDirectoryChanged();
    });
    connect(m_proxyModel,&SearchFilterProxyModel::conditionsChange,this,[=](){
        Q_EMIT viewDirectoryChanged();
    });
    connect(searchmodel, &SearchFileItemModel::resizeColFitContent, [=](){
        QString defaultStr = searchmodel->getDefaultString();
        if(defaultStr!=""){
            QFontMetrics fm(m_view->font());
            int textWidth = fm.horizontalAdvance(defaultStr);
            m_view->setColumnWidth(0, textWidth*7/10);
        }
        else{
            m_view->resizeColumnToContents(0);
        }
    });
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    auto selectionModel = m_view->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &DirectoryViewWidget::viewSelectionChanged);
    connect(searchmodel, &SearchFileItemModel::expand, [=](){
        int count = m_proxyModel->rowCount();
        for (int var = 0; var < count; ++var) {
            auto index = m_proxyModel->index(var,0);
            if(m_model->hasChildren(m_proxyModel->mapToSource(index))){
                m_view->expand(index);
            }
        }
    });

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, m_view, [=](const QItemSelection &selected, const QItemSelection &deselected){
        auto currentSelections = selected.indexes();
        if (!currentSelections.isEmpty()) {
            auto tmpIndex = currentSelections.first();
            auto index = m_proxyModel->mapToSource(tmpIndex);
            if (m_model->isFileCategory(index) || m_model->isContentCategory(index)) {
                return;
            }
        }
        m_view->getSelectedIndex(selected, deselected);
    });
    connect(m_model, &SearchFileItemModel::updated, m_view->viewport(), QOverload<>::of(&QWidget::update));
    connect(m_delegate,&TreeviewDelegate::reNameFinish,[=](const QString &olduri,const QString &newuri,const QString &fileName){
        QStringList  fileuris =  getAllFileUris();
        auto parentPath = FileUtils::getParentUri(newuri);
        parentPath = FileUtils::urlDecode(parentPath);
        QString filePath = parentPath+"/"+fileName;
        filePath = filePath.remove("file://");
        if(fileuris.contains(filePath)){
            m_model->childRename(olduri,newuri);
        }
    });

//    auto m_thumbnail_watcher = std::make_shared<Peony::FileWatcher>("thumbnail://");
//    connect(m_thumbnail_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri){
//        auto index = m_model->indexFromUri(uri,m_item_type);
//        if (index.isValid()) {
//            auto item = m_model->itemFromIndex(index);
//            if (item) {
//                m_model->updated();
//                //m_model->thumbnailUpdated(uri);
//            }
//        }
//    });
//    connect(m_thumbnail_watcher.get(), &FileWatcher::thumbnailUpdated, this, [=](const QString &uri) {
//            m_model->updated();
//            //m_model->dataChanged(m_model->indexFromUri(uri), m_model->indexFromUri(uri));
//        }, Qt::UniqueConnection);
}

void SearchViewContainer::setDirectoryUri(const QString &uri)
{
    if (m_currentUri != uri) {
        m_currentUri = uri;
        m_proxyModel->setCurrenturi(m_currentUri);
        m_watcher = std::make_shared<Peony::FileWatcher>(m_currentUri, nullptr, true);
        m_watcher->setMonitorChildrenChange(true);
        connect(m_watcher.get(), &FileWatcher::fileDeleted, this, [=](QString uri) {
            m_model->childRemove(uri);
        });

        connect(m_watcher.get(), &FileWatcher::fileRenamed, this, [=](const QString &oldUri, const QString &newUri) {
            m_model->childRename(oldUri, newUri);
            QString fileName = QUrl(newUri).fileName();
            if(!fileName.contains(m_keyword)){
                m_model->childRemove(newUri);
            }
        });

        m_watcher->startMonitor();
    }
}

void SearchViewContainer::beginLocationChange()
{
    Q_EMIT viewDirectoryChanged();

    m_searching = true;
    // 单次定时器已激活时重新 start，以实现防抖。
    m_searchTrigger->start();
    m_signalTransInterface->call("cancelSearch");
}

void SearchViewContainer::stopLocationChange()
{
    Q_EMIT viewDirectoryChanged();
}

void SearchViewContainer::setSelections(const QStringList &uris)
{
    m_view->setSelections(uris);
}

void SearchViewContainer::invertSelections()
{
    m_view->invertSelections();
}


void SearchViewContainer::setSortType(int sortType)
{
    if (m_sortType == sortType)
        return;

    m_sortType = sortType;
    applySorting();
}

void SearchViewContainer::setSortOrder(int sortOrder)
{
    if (m_sortOrder == sortOrder)
        return;

    m_sortOrder = sortOrder;
    applySorting();
}

void SearchViewContainer::editUri(const QString &uri)
{
    m_view->editUri(uri);
}

void SearchViewContainer::editUris(const QStringList uris)
{
    m_view->editUris(uris);
}

void SearchViewContainer::repaintView()
{
    m_view->update();
    m_view->viewport()->update();
}

void SearchViewContainer::applySorting()
{
    int sortColumn;
    switch (m_sortType) {
        case 0: // 按名称
            sortColumn = 0;
            break;
        case 1: // 按日期
            sortColumn = 1;
            break;
        case 2: // 按大小
            sortColumn = 2;
            break;
        case 3: // 按类型
            sortColumn = 3;
            break;
        default:
            sortColumn = 0;
    }
    Qt::SortOrder order = static_cast<Qt::SortOrder>(m_sortOrder);
    if (m_proxyModel) {
        QString uri = (!m_defaultUri.isEmpty()&&m_currentUri.isEmpty())?m_defaultUri:m_currentUri;
        m_proxyModel->setCurrenturi(uri);
        m_proxyModel->checkSortSettings();
        m_view->header()->setSortIndicator(sortColumn, order);
//        m_proxyModel->sort(m_sortType, order);
//        m_proxyModel->update();
    }
}

QStringList  const SearchViewContainer:: getAllFileUris(){
    return m_proxyModel->getFiltereUrilist();
}

int SearchViewContainer::getSortType()
{
    return m_sortType;
}

Qt::SortOrder SearchViewContainer::getSortOrder()
{
    Qt::SortOrder order = static_cast<Qt::SortOrder>(m_sortOrder);
    return order;
}

void SearchViewContainer::onItemDoubleClicked(const QModelIndex &index){
    if (!index.isValid()) return;
    // 判断是否为二级Item（有父节点且父节点没有父节点）
    QModelIndex realindex =  m_proxyModel->mapToSource(index);
    quintptr id = realindex.internalId();
    if(!isAiConditionAvialable && m_model->isContentItem(realindex) && m_model->hasDefaultContent()){
        //requestShowAISubsystemDialog();
        return;
    }
    else if (index.parent().isValid() && !index.parent().parent().isValid()) {
         QModelIndex realindex =  m_proxyModel->mapToSource(index);
         QString uri = m_model->data(realindex,Qt::UserRole).toString();
         if (m_view->getSelectedIndexes().count() == 1) {
             m_view->queryFileinfo(uri);
             Q_EMIT this->viewDoubleClicked(uri);
         }
    }
}

void SearchViewContainer::requestShowAISubsystemDialog()
{
    if(aiConditionType==aiIndexNotFit)
        QProcess::startDetached("ukui-control-center", {"-m", "search"});
    else
        QProcess::startDetached("ukui-control-center", {"-m", "aisubsystem"});
}

QString SearchViewContainer::extractWithRegex(extractType type){
    QString tmpUri = FileUtils::urlDecode(m_currentUri);
    QStringList list = tmpUri.split("&");
    if (Peony::SearchVFSUriParser::checkForSpecialInStringLists(list)) {
        list = Peony::SearchVFSUriParser::handleAmpersandSplitStringList(&tmpUri, list);
        for (QString tmp:list) {
            if(type == keyWord && tmp.startsWith("name_regexp")){
                return tmp.remove("name_regexp=");
            }else if(type == spaceName && tmp.startsWith("search_uris=idm:")){
                return tmp.remove("search_uris=idm:///");
            }else if(type == searchPath && tmp.startsWith("search_uris=file:")){
                return tmp.remove("search_uris=file://");
            }
        }
    }
    switch (type) {
    case keyWord:{
        QRegularExpression regex("name_regexp=([^&]+)");
        QRegularExpressionMatch match = regex.match(tmpUri);
        return match.hasMatch() ? match.captured(1) : QString();
        break;
    }
    case spaceName:{
        QRegularExpression regex(R"(search_uris=idm:///([^&]+))");
        QRegularExpressionMatch match = regex.match(tmpUri);
        return match.hasMatch() ? match.captured(1) : QString();
        break;
    }
    case searchPath:{
        QRegularExpression regex(R"(search_uris=file://([^&]+))");
        QRegularExpressionMatch match = regex.match(tmpUri);
        if(!match.hasMatch()){
            QRegularExpression regextrash(R"(search_uris=trash://([^&]+))");
            QRegularExpressionMatch matchtrash = regextrash.match(tmpUri);
            QRegularExpression regexfilesafe(R"(search_uris=filesafe://([^&]+))");
            QRegularExpressionMatch matchfilesafe = regexfilesafe.match(tmpUri);
            QRegularExpression regexrecent (R"(search_uris=recent://([^&]+))");
            QRegularExpressionMatch matchrecent = regexrecent.match(tmpUri);
            QRegularExpression regexfavorite(R"(search_uris=favorite://([^&]+))");
            QRegularExpressionMatch matchfavorite = regexfavorite.match(tmpUri);
            QRegularExpression regexFtp(R"(search_uris=ftp://([^&]+))");
            QRegularExpressionMatch matchFtp = regexFtp.match(tmpUri);
            QRegularExpression regexSftp(R"(search_uris=sftp://([^&]+))");
            QRegularExpressionMatch matchSftp = regexSftp.match(tmpUri);
            QRegularExpression regexSmb(R"(search_uris=smb://([^&]+))");
            QRegularExpressionMatch matchSmb = regexSmb.match(tmpUri);
            isSeachInFileArea = !(matchtrash.hasMatch() || matchfilesafe.hasMatch() || matchrecent.hasMatch() || matchfavorite.hasMatch() || matchFtp.hasMatch() || matchSftp.hasMatch() || matchSmb.hasMatch());
        }else{
            isSeachInFileArea = true;
        }
        return match.hasMatch() ? match.captured(1) : QString();
        break;
    }
    default:
        break;
    }
}

void SearchViewContainer::performSearch()
{
    m_searching = false;
    QString key = extractWithRegex(keyWord);
    QString Spacename = extractWithRegex(spaceName);
    QString path = extractWithRegex(searchPath);
    QString realpath = (path == QStandardPaths::writableLocation(QStandardPaths::HomeLocation)) ? "" : path;
    bool isupdatebyrefresh  = qApp->property("refresh").toBool();
    if(isupdatebyrefresh){
         qApp->setProperty("refresh",false);
    }
    if(key == m_keyword && realpath == m_currentSearchPath && m_spaceName == Spacename && !isupdatebyrefresh){
        // 只有确认本次位置变更仍对应当前搜索时，才使用当前轮次的完成状态。
        m_model->finishSearch();
        return;
    }
    m_keyword = key;
    m_spaceName = Spacename;
    m_currentSearchPath = realpath;
    m_model->updateSearchstatus(false);
    m_model->keywordchange(key);
    m_delegate->setHighlightKeyword(key);
    if(isSeachInFileArea){
        m_signalTransInterface->call("setCurrentSearchPath", realpath); // 设置当前搜索路径
        m_signalTransInterface->call("startsearch", key, Spacename, m_clientId);
    }else{
        m_model->setRootUri(m_currentUri);
    }
}
