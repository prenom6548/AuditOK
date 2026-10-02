/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <QColor>
#include <QSize>
#include <QPen>
#include <QRect>
#include <QPoint>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QAbstractItemDelegate>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTableView>
#include <QListView>
#include <QStyledItemDelegate>
#include <QPair>

#include "global/viewvar.h"
#include "global/corevar.h"
#include "controller/Interaction.h"
#include "view/search/searchview.h"
#include "view/search/titlesearchcityview.h"
#include "view/search/bottomlistview.h"
#include "view/search/searchcitylist.h"
#include "view/search/collectcity.h"
#include "view/horscreen/horscreen.h"
#include "test_horscreen.h"
#include "controller/core/viewcontrol.h"
#include "global/rotatechangeinfo.h"
#include "controller/core/core.h"
#include "controller/core/errorreact.h"
#include "controller/core/coreinteraction.h"
#include "model/dataparser.h"
#include "view/search/titlesearchlistdelegate.h"
#include "view/search/bottomlistdelegate.h"
#include "view/search/collectitemdelegate.h"
#include "view/search/searchdelegate.h"

// Define missing static members of ViewVar
QMap<int, QStringList> ViewVar::HOTCITY_MAP = QMap<int, QStringList>();
QColor ViewVar::s_colorItem = QColor();
QColor ViewVar::s_colorWid = QColor();
QColor ViewVar::s_colorSearch = QColor();
const QSize ViewVar::COLLECT_BTNSIZE = QSize(100, 30);
const QSize ViewVar::COLLECT_BTNSIZE_WHITE = QSize(100, 30);
QString ViewVar::icon_path = "";
const int ViewVar::COLLECT_BTNXL = 0;
const int ViewVar::COLLECT_BTNYL = 0;
const int ViewVar::COLLECT_BTNXR = 0;
const int ViewVar::COLLECT_BTNYR = 0;
QPen ViewVar::m_collectPen = QPen();
const int ViewVar::COLLECT_NAMEXL = 0;
const int ViewVar::COLLECT_TMPXL = 0;
const int ViewVar::COLLECT_TMPXL1 = 0;
const int ViewVar::COLLECT_CODEXL = 0;
const int ViewVar::COLLECT_CODEXL1 = 0;
const int ViewVar::COLLECT_SAMEYL = 0;
const int ViewVar::COLLECT_SAMEXR = 0;
const int ViewVar::COLLECT_SAMEYR = 0;
const int ViewVar::COLLECT_ICONXL = 0;
const int ViewVar::COLLECT_ICONXL1 = 0;
const int ViewVar::COLLECT_ICONYL = 0;
const int ViewVar::COLLECT_ICONXR = 0;
const int ViewVar::COLLECT_ICONYR = 0;
const int ViewVar::SEARCH_BTNXL = 0;
const int ViewVar::SEARCH_BTNYL = 0;
const int ViewVar::SEARCH_BTNXR = 0;
const int ViewVar::SEARCH_BTNYR = 0;
const int ViewVar::SEARCH_NAMEXL = 0;
const int ViewVar::SEARCH_NAMEYL = 0;
const int ViewVar::SEARCH_NAMEXR = 0;
const int ViewVar::SEARCH_NAMEYR = 0;
const QString ViewVar::COLLECT_ICONPATH = "";
const QString ViewVar::COLLECT_ICONSUFFIX = "";
const QRect ViewVar::COLLECT_ITEMRECT = QRect();
const QRect ViewVar::SEARCH_ITEMRECT = QRect();
qreal ViewVar::SAME_ITEMRADIUS = 0;
const qreal ViewVar::SAME_OPACITY_0 = 0;
const qreal ViewVar::SAME_OPACITY_1 = 0;
const qreal ViewVar::SAME_OPACITY_2 = 0;
bool ViewVar::s_themeStyle = false;
const QSize ViewVar::COLLECT = QSize(300, 400);
const QSize ViewVar::COLLECT_PAD = QSize(300, 400);
const QSize ViewVar::COLLECT_LIST = QSize(300, 400);
const QSize ViewVar::COLLECT_LIST_PAD = QSize(300, 400);
const QPoint ViewVar::COLLECT_TITLE = QPoint();
const int ViewVar::NONECITYX = 0;
const int ViewVar::CITYLISTX = 0;
const int ViewVar::COLLECT_ITEMH = 0;
const int ViewVar::COLLECT_ITEMW = 0;
const int ViewVar::COLLECT_ITEMH_PAD = 0;
const int ViewVar::COLLECT_ITEMW_PAD = 0;
const QSize ViewVar::HOT = QSize();
const QSize ViewVar::HOT_LIST = QSize();
const QPoint ViewVar::HOT_TITLE = QPoint();
const int ViewVar::HOTLISTX = 0;
const int ViewVar::ITEM_LENGTHM = 0;
const int ViewVar::ITEM_NUM = 0;
const int ViewVar::ITEM_NUMM = 0;
const int ViewVar::ITEM_POS = 0;
const int ViewVar::ITEM_DIS0 = 0;
const int ViewVar::ITEM_DIS2 = 0;
const QSize ViewVar::HOT_BTN = QSize();
const QSize ViewVar::SEARCH_BOX = QSize(200, 30);
const QSize ViewVar::SEARCH_BOX_PAD = QSize(200, 30);
const QMargins ViewVar::SEARCH_MARGINS = QMargins();
const QSize ViewVar::SEARCH_ICON = QSize();
const int ViewVar::SEARCH_ICON_X0 = 0;
const int ViewVar::SEARCH_ICON_X1 = 0;
const QSize ViewVar::SEARCH_LIST = QSize();
const int ViewVar::SEARCH_ITEMH = 0;
const int ViewVar::SEARCH_ITEMW = 0;
const int ViewVar::SEARCH_NONEW = 0;
const QSize ViewVar::SEARCH = QSize(400, 500);
const QSize ViewVar::SEARCH_PAD = QSize(400, 500);
const int ViewVar::SEARCH_BOXY = 0;
const QPoint ViewVar::SEARCHLIST_POS = QPoint();
const int ViewVar::SEARCH_CHANGECITYX = 0;
const int ViewVar::DISTANCE0 = 0;
const int ViewVar::DISTANCE1 = 0;
bool ViewVar::m_editStyle = false;
bool ViewVar::m_existAddBtn = false;
const QString ViewVar::COLLENTTEXT = "Favorite";
QStringList ViewVar::platFormList = QStringList();
QString ViewVar::platForm = "";
bool ViewVar::widgetIsShow = false;
const QSize ViewVar::TITLESEARCHITEMSIZE = QSize();
bool ViewVar::m_isTitleSearch = false;
const QSize ViewVar::CITY_ICON = QSize();
const QSize ViewVar::CITY_ICON_MAXSIZE = QSize();
const QSize ViewVar::COLLECT_SIZE = QSize();
const QSize ViewVar::COLLECT_SIZE_PAD = QSize();
const QSize ViewVar::COLLECT_ITEM_SIZE = QSize();
const QSize ViewVar::COLLECT_ITEM_SIZE_PAD = QSize();
ViewVar* ViewVar::m_viewVar = nullptr;

ViewVar::ViewVar() {}
ViewVar* ViewVar::getInstance() {
    if (!m_viewVar) m_viewVar = new ViewVar();
    return m_viewVar;
}
CurrentMode ViewVar::getCurrentMode() { return m_currentMode; }

// Define missing static members of CoreVar
const QString CoreVar::PROGRAM_NAME = "Weather";
const QString CoreVar::WEATHER_DBUS_SERVICENAME = "";
const QString CoreVar::WEATHER_DBUS_PARH = "";
const QString CoreVar::WEATHER_DBUS_INTERFACE = "";
const QString CoreVar::WEATHER_GSETTINGS_SERVICENAME = "";
const QString CoreVar::PCONLINEURL = "";
const QString CoreVar::IPIP = "";
QString CoreVar::URLHEAD = "";
const QString CoreVar::SERVICEREQUEST = "";
const QString CoreVar::SERVICEREQUEST_COLLECT = "";
const QMap<QString, QString> CoreVar::SUPPORT_CMD = QMap<QString, QString>();
const int CoreVar::REFRESH_RATE = 0;
const QString CoreVar::API_TYPE = "";
const int CoreVar::SEARCHCITYNUM = 0;
const QSize CoreVar::CITYITEM = QSize();
const QStringList CoreVar::WEATHER_CODE_TMP = QStringList();
const QString CoreVar::DEFAULT_CITYID = "";
const int CoreVar::DEFAULT_TIMEOUT = 0;
int CoreVar::TITLE_SEARCH_LIST_NUM = 0;
bool CoreVar::GET_CITY_INFO = false;
CityType CoreVar::CURRENT_CITY_TYPE = CityType::None;
QString CoreVar::CURRENT_CITY_ID = "";
QString CoreVar::CURRENT_CITY_NAME = "";
QString CoreVar::LOCATION_CITY_ID = "";
QStringList CoreVar::CURRENT_COLLECT_LIST = QStringList();
bool CoreVar::CURRENT_TITLE_SEARCH = false;
bool CoreVar::HAS_NETWORK = true;
QGSettings* CoreVar::m_settings = nullptr;

void CoreVar::setSettings(const QString&, const QVariant&) {}
QVariant CoreVar::getSettings(const QString&) { return QVariant(); }

// Mock Interaction
Interaction::~Interaction() {}
Interaction* Interaction::m_interaction = nullptr;
Interaction* Interaction::getInstance() {
    return MockInteraction::getInstance();
}

// Mock CoreInteraction
CoreInteraction::CoreInteraction() : Interaction() {
}
void CoreInteraction::creatCore(const QStringList&) {}
void CoreInteraction::changeCity(QString) {}
void CoreInteraction::searchResult(QString) {}
QStandardItemModel* CoreInteraction::getCityModel() { return MockInteraction::getInstance()->getCityModel(); }
QStandardItemModel* CoreInteraction::getTitleModel() { return MockInteraction::getInstance()->getTitleModel(); }
QStandardItemModel* CoreInteraction::getCollectModel() { return MockInteraction::getInstance()->getCollectModel(); }
QStandardItemModel* CoreInteraction::getBottomCollectModel() { return MockInteraction::getInstance()->getBottomCollectModel(); }
void CoreInteraction::changeCityFromTitleSearch(QString) {}
void CoreInteraction::changeCityFromSearch(QModelIndex) {}
void CoreInteraction::addCollectCity(QModelIndex) {}
void CoreInteraction::changeCityFromCollect(QModelIndex) {}
void CoreInteraction::delCollectCity(QModelIndex) {}
void CoreInteraction::loadCollectCity() {}
void CoreInteraction::showCenterAfter() {}
void CoreInteraction::closePrograme() {}
void CoreInteraction::allowLocationYes() {}
void CoreInteraction::allowLocationNo() {}
void CoreInteraction::refresh() {}
void CoreInteraction::addCollectCityFromWidget(QString) {}
void CoreInteraction::delCollectCityFromWidget(QString) {}
void CoreInteraction::getLAndRCityInfo(QString) {}
void CoreInteraction::test() {}

// Mock SearchView
SearchView::SearchView(QWidget *parent) : QWidget(parent) {}
void SearchView::changeUiShow() {}
void SearchView::searchReset() {}
void SearchView::hideEvent(QHideEvent*) {}
void SearchView::keyPressEvent(QKeyEvent*) {}
bool SearchView::event(QEvent*) { return false; }
bool SearchView::eventFilter(QObject*, QEvent*) { return false; }
void SearchView::sendSearchInput() {}
void SearchView::addCollectCity(QModelIndex) {}
void SearchView::changeCityFromSearch(QModelIndex) {}
void SearchView::changeCityFromCollect(QModelIndex) {}
void SearchView::searchListUi(bool) {}
void SearchView::changeStackedWidget() {}
void SearchView::onBackLabelClicked() {}
void SearchView::paintEvent(QPaintEvent*) {}

// Mock TitleSearchCityView
TitleSearchCityView::TitleSearchCityView(QWidget *parent) : QListView(parent) {}
TitleSearchCityView::~TitleSearchCityView() {}
void TitleSearchCityView::searchUiChange(bool) {}
void TitleSearchCityView::setViewModel(bool) {}
void TitleSearchCityView::slotSearchNull() {}
void TitleSearchCityView::slotChangeCityFromTitleSearchList(QModelIndex) {}

// Mock SearchCityLIst
SearchCityLIst::SearchCityLIst(QWidget *parent) : QTableView(parent) {}
void SearchCityLIst::searchUiChange(bool) {}
void SearchCityLIst::setViewModel(bool) {}
void SearchCityLIst::searchNull() {}

// Mock CollectCity
CollectCity::CollectCity(QWidget *parent) : QWidget(parent) {}
void CollectCity::changeEditBtnStyle() {}
void CollectCity::slotClickButton() {}
void CollectCity::changeShow() {}
void CollectCity::changeCollectWidgetStyle() {}
void CollectCity::setModelforView() {}
void CollectCity::changeToHotCity(QString) {}
void CollectCity::changeItem() {}

// Mock BottomListView
BottomListView::BottomListView(QWidget *parent) : QListView(parent) {}
BottomListView::~BottomListView() {}
void BottomListView::slotShowBottomList(int) {}
void BottomListView::slotChangeCityFromTitleSearchList(QModelIndex) {}

// Mock viewControl
viewControl* viewControl::m_viewControl = nullptr;
static viewControl* g_viewControl = nullptr;
viewControl::viewControl() : QObject(nullptr) {}
viewControl* viewControl::getInstance() { 
    if (!g_viewControl) g_viewControl = new viewControl();
    return g_viewControl; 
}
QWidget* viewControl::activeWindow() { 
    static QWidget dummy;
    return &dummy; 
}
void viewControl::autoStart() {}
void viewControl::updateIsStart(bool) {}
void viewControl::toShowView() {}
void viewControl::toShowViewAndRefresh() {}

// Missing method in horscreen
QString horscreen::getBackgroundImageName(QString code) {
    if (code == "100") return "sunny";
    return "default";
}

// More missing symbols definitions
TitleSearchModel::TitleSearchModel() : QStandardItemModel() {}
TitleSearchModel::~TitleSearchModel() {}

TitleSearchListDelegate::TitleSearchListDelegate(QObject* parent) : QStyledItemDelegate(parent) {}
TitleSearchListDelegate::~TitleSearchListDelegate() {}
void TitleSearchListDelegate::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const {}
QSize TitleSearchListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const { return QSize(); }
bool TitleSearchListDelegate::editorEvent(QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex&) { return false; }

BottomListDelegate::BottomListDelegate(QObject* parent) : QStyledItemDelegate(parent) {}
BottomListDelegate::~BottomListDelegate() {}
void BottomListDelegate::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const {}
QSize BottomListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const { return QSize(); }

CollectItemDelegate::CollectItemDelegate(QWidget* parent) : QStyledItemDelegate(parent) {}
void CollectItemDelegate::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const {}
QSize CollectItemDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const { return QSize(); }
bool CollectItemDelegate::editorEvent(QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex&) { return false; }

SearchDelegate::SearchDelegate(QWidget* parent) : QStyledItemDelegate(parent) {}
void SearchDelegate::paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const {}
bool SearchDelegate::editorEvent(QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex&) { return false; }

RotateChangeInfo::RotateChangeInfo(QObject* parent) : QObject(parent) {}
bool RotateChangeInfo::getCurrentMode() { return false; }
QPair<int, int> RotateChangeInfo::getCurrentScale() { return qMakePair(1, 1); }
void RotateChangeInfo::slotModeChanged(bool) {}
void RotateChangeInfo::slotRotateChanged() {}

ErrorReact* ErrorReact::m_errorReact = nullptr;
static ErrorReact* g_errorReact = nullptr;
ErrorReact::ErrorReact() : QObject(nullptr) {}
ErrorReact* ErrorReact::getInstance() { 
    if (!g_errorReact) g_errorReact = new ErrorReact();
    return g_errorReact; 
}
void ErrorReact::networkFeedBack(bool, QString) {}
void ErrorReact::serverFeedBack(bool, QString) {}
void ErrorReact::dataFeedBack(bool, QString) {}
void ErrorReact::searchFeedBack(bool) {}
void ErrorReact::networkTimeout(QString) {}
void ErrorReact::geoFail(QString) {}
void ErrorReact::collectAddError(QString) {}

Core::Core() : QObject(nullptr) {}
void Core::initCore(const QStringList&) {}
void Core::changeCity(QString) {}
void Core::searchCity(QString) {}
void Core::addCollectCity(QModelIndex) {}
void Core::delCollectCity(QModelIndex) {}
void Core::loadCollectCity() {}
void Core::progremExit() {}
void Core::allowLocationYes() {}
void Core::allowLocationNo() {}
void Core::refreshCity() {}
void Core::addCollectCityFromWidget(QString) {}
void Core::delCollectCityFromWidget(QString) {}
QStandardItemModel* Core::getCityModel() { return nullptr; }
QStandardItemModel* Core::getCityTitleModel() { return nullptr; }
QStandardItemModel* Core::getCollectModel() { return nullptr; }
QStandardItemModel* Core::getBottomCollectModel() { return nullptr; }
void Core::changeCityFromCollect(QModelIndex) {}
void Core::changeCityFromSearch(QModelIndex) {}
void Core::networkState(bool) {}
void Core::updateCollect(QList<CollectItemData>) {}

DataParser::DataParser() : QObject(nullptr) {}
void DataParser::dataReplyL(QByteArray) {}
void DataParser::dataReply(QByteArray) {}
void DataParser::dataReplyR(QByteArray) {}
void DataParser::dataReplyColl(QByteArray) {}
