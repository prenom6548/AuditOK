/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include "view/friendlist/friendlist.h"
#include "view/trayicon/trayiconwid.h"
#include "view/chatmsg/chatmsg.h"
#include "network/tcp_server.h"
#include "view/localinfo/localinfo.h"
#include "view/friendlist/homepagesearch/searchmsglist.h"
#include "view/friendlist/homepagesearch/search_msg_model.h"
#include "global/utils/globaldateformat.h"
#include "view/kyview.h"
#include "view/titlebar/titlebar.h"
#include "view/chatsearch/chat_search.h"
#include "view/chatsearch/chat_search_filter.h"

// FriendListView
FriendListView::FriendListView(QListView* parent) : QListView(parent) {}
FriendListView::~FriendListView() {}
FriendListView* FriendListView::getInstance(QListView* parent) {
    static FriendListView *instance = nullptr;
    if (!instance) {
        instance = new FriendListView(parent);
    }
    return instance;
}
ChatMsg* FriendListView::getMsgWidById(int) {
    static ChatMsg *msg = new ChatMsg(0, "");
    return msg;
}
void FriendListView::slotShowMsgWidById(int) {}
void FriendListView::slotDeleFriend() {}
void FriendListView::slotFriendState() {}

// TitleBar
TitleBar::TitleBar(QWidget *parent) : QWidget(parent) {
    m_pFuncLabel = new QLabel(this);
}
TitleBar::TitleBar(bool isMenu, bool isClose, bool isBlur, QWidget *parent) : QWidget(parent) {
    m_pFuncLabel = new QLabel(this);
}
TitleBar::~TitleBar() {}
void TitleBar::paintEvent(QPaintEvent *) {}
bool TitleBar::event(QEvent *e) { return QWidget::event(e); }
#ifdef __V10__
void TitleBar::mousePressEvent(QMouseEvent *) {}
void TitleBar::mouseMoveEvent(QMouseEvent *) {}
void TitleBar::mouseReleaseEvent(QMouseEvent *) {}
#endif

// ChatSearch
ChatSearch::ChatSearch(ChatMsg *m) : QWidget(m) {}
ChatSearch::~ChatSearch() {}
bool ChatSearch::eventFilter(QObject *o, QEvent *e) { return QWidget::eventFilter(o, e); }
void ChatSearch::keyPressEvent(QKeyEvent *) {}
void ChatSearch::closeEvent(QCloseEvent *) {}

// ChatMsgDelegate
ChatMsgDelegate::ChatMsgDelegate() : QAbstractItemDelegate(nullptr) {}
ChatMsgDelegate::~ChatMsgDelegate() {}
QSize ChatMsgDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const { return QSize(); }
void ChatMsgDelegate::paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const {}

// ChatSearchFilter
ChatSearchFilter::ChatSearchFilter() : QSortFilterProxyModel(nullptr) {}
ChatSearchFilter::~ChatSearchFilter() {}
bool ChatSearchFilter::filterAcceptsRow(int, const QModelIndex &) const { return true; }
bool ChatSearchFilter::lessThan(const QModelIndex &, const QModelIndex &) const { return true; }

// TrayIconWid
// Moved to stub_all.cpp

// LocalInfo
LocalInfo::LocalInfo(QWidget *parent) : QWidget(parent) {}
LocalInfo::~LocalInfo() {}
LocalInfo* LocalInfo::getInstance(QWidget* parent) {
    static LocalInfo *instance = nullptr;
    if (!instance) {
        instance = new LocalInfo(parent);
    }
    return instance;
}
void LocalInfo::changeFontSize() {}
void LocalInfo::setWidgetUi() {}
void LocalInfo::setWidgetStyle() {}
void LocalInfo::showList() {}
void LocalInfo::changeTheme() {}
void LocalInfo::displayInfo(QString, QString) {}
void LocalInfo::getIniText() {}
void LocalInfo::checkLocalName() {}
void LocalInfo::paintEvent(QPaintEvent *) {}
bool LocalInfo::event(QEvent *e) { return QWidget::event(e); }
bool LocalInfo::eventFilter(QObject *o, QEvent *e) { return QWidget::eventFilter(o, e); }

// SearchMsgList
SearchMsgList::SearchMsgList(QListView* parent) : QListView(parent) {}
SearchMsgList::~SearchMsgList() {}
SearchMsgList* SearchMsgList::getInstance(QListView* parent) {
    static SearchMsgList *instance = nullptr;
    if (!instance) {
        instance = new SearchMsgList(parent);
    }
    return instance;
}

// SearchMsgModel
SearchMsgModel::SearchMsgModel() : QStandardItemModel(nullptr) {}
SearchMsgModel::~SearchMsgModel() {}
SearchMsgModel* SearchMsgModel::getInstance() {
    static SearchMsgModel *instance = nullptr;
    if (!instance) {
        instance = new SearchMsgModel();
    }
    return instance;
}
void SearchMsgModel::slotSelect(QString) {}

// GlobalDateFormat
GlobalDateFormat* GlobalDateFormat::getInstance() {
    static GlobalDateFormat *instance = nullptr;
    if (!instance) {
        instance = new GlobalDateFormat();
    }
    return instance;
}
GlobalDateFormat::GlobalDateFormat() : QObject(nullptr) {}
GlobalDateFormat::~GlobalDateFormat() {}
void GlobalDateFormat::slotDateFormatChange(QString) {}
void GlobalDateFormat::slotShortDateFormatChange(QString) {}
void GlobalDateFormat::slotLongDataFormatChange(QString) {}
void GlobalDateFormat::slotTimeFormatChange(QString) {}

// KyView
KyView::KyView(QWidget *parent) : QWidget(parent) {
    m_titleBar = new TitleBar();
}
KyView::~KyView() {}
KyView* KyView::getInstance() {
    static KyView *instance = nullptr;
    if (!instance) {
        instance = new KyView();
    }
    return instance;
}
void KyView::showList() {}
void KyView::setWidgetUi() {}
void KyView::setWidgetStyle() {}
void KyView::initGsetting() {}
void KyView::checkLocalName() {}
void KyView::closeEvent(QCloseEvent *) {}
void KyView::paintEvent(QPaintEvent *) {}
void KyView::keyPressEvent(QKeyEvent *) {}
bool KyView::event(QEvent *e) { return QWidget::event(e); }
bool KyView::eventFilter(QObject *o, QEvent *e) { return QWidget::eventFilter(o, e); }
void KyView::slotSearchInterface() {}
void KyView::slotShowBlankPage() {}
void KyView::slotFriendInterface() {}
void KyView::slotChangeStatck() {}
void KyView::slotShowSearchBlankPage() {}
void KyView::slotChangeHScreenMode() {}
void KyView::slotChangeVScreenMode() {}
void KyView::slotChangePCMode() {}

// TitleBar private slots
void TitleBar::onClicked() {}
void TitleBar::onMenuButtonClicked() {}

// ChatSearch slots required by moc
void ChatSearch::onSliderMoved(int) {}
void ChatSearch::showAllInfo() {}
void ChatSearch::showFileInfo() {}
void ChatSearch::showImageInfo() {}
void ChatSearch::showLinkInfo() {}
void ChatSearch::filterFriendByReg(QString) {}
void ChatSearch::showContextMenu(const QPoint &) {}
bool ChatSearch::checkFileExist(QString) { return true; }
void ChatSearch::slotOpenFile() {}
void ChatSearch::slotOpenDir() {}
void ChatSearch::showDeleMenu() {}
void ChatSearch::showChooseMsg() {}
void ChatSearch::slotChooseMsg(const QModelIndex &) {}
void ChatSearch::slotDeleteMsg() {}
void ChatSearch::slotClearMsg() {}
void ChatSearch::slotCancelClick() {}
void ChatSearch::slotSureClick() {}
void ChatSearch::changeSizeHScreenMode() {}
void ChatSearch::changeSizeVScreenMode() {}
void ChatSearch::changeSizePCMode() {}
