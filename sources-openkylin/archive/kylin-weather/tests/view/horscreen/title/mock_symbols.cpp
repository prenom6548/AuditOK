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
#include "global/corevar.h"
#include "global/viewvar.h"
#include "controller/Interaction.h"
#include "view/horscreen/horscreen.h"
#include "view/horscreen/menumodule.h"
#include "view/search/searchview.h"
#include <QStringList>
#include <QStandardItemModel>
#include <QToolButton>

// ViewVar static members
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
const QSize ViewVar::COLLECT = QSize();
const QSize ViewVar::COLLECT_PAD = QSize();
const QSize ViewVar::COLLECT_LIST = QSize();
const QSize ViewVar::COLLECT_LIST_PAD = QSize();
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
const QSize ViewVar::SEARCH_BOX = QSize();
const QSize ViewVar::SEARCH_BOX_PAD = QSize();
const QMargins ViewVar::SEARCH_MARGINS = QMargins();
const QSize ViewVar::SEARCH_ICON = QSize();
const int ViewVar::SEARCH_ICON_X0 = 0;
const int ViewVar::SEARCH_ICON_X1 = 0;
const QSize ViewVar::SEARCH_LIST = QSize();
const int ViewVar::SEARCH_ITEMH = 0;
const int ViewVar::SEARCH_ITEMW = 0;
const int ViewVar::SEARCH_NONEW = 0;
const QSize ViewVar::SEARCH = QSize();
const QSize ViewVar::SEARCH_PAD = QSize();
const int ViewVar::SEARCH_BOXY = 0;
const QPoint ViewVar::SEARCHLIST_POS = QPoint();
const int ViewVar::SEARCH_CHANGECITYX = 0;
const int ViewVar::DISTANCE0 = 0;
const int ViewVar::DISTANCE1 = 0;
bool ViewVar::m_editStyle = false;
bool ViewVar::m_existAddBtn = false;
const QString ViewVar::COLLENTTEXT = "";
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

ViewVar* ViewVar::getInstance() {
    if (!m_viewVar) m_viewVar = new ViewVar();
    return m_viewVar;
}
ViewVar::ViewVar() {}
CurrentMode ViewVar::getCurrentMode() { return PCMode; }

// CoreVar static members
bool CoreVar::CURRENT_TITLE_SEARCH = false;
const QString CoreVar::PROGRAM_NAME = "";
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
CityType CoreVar::CURRENT_CITY_TYPE = None;
QString CoreVar::CURRENT_CITY_ID = "";
QString CoreVar::CURRENT_CITY_NAME = "";
QString CoreVar::LOCATION_CITY_ID = "";
QStringList CoreVar::CURRENT_COLLECT_LIST = QStringList();
bool CoreVar::HAS_NETWORK = true;
QGSettings* CoreVar::m_settings = nullptr;
void CoreVar::setSettings(const QString&, const QVariant&) {}
QVariant CoreVar::getSettings(const QString&) { return QVariant(); }

// Interaction Mock
Interaction::~Interaction() {}
static Interaction* g_interaction = nullptr;
Interaction* Interaction::getInstance() { return g_interaction; }

class MockInteraction : public Interaction {
public:
    MockInteraction() { g_interaction = this; }
    void creatCore(const QStringList&) override {}
    void changeCity(QString) override {}
    void searchResult(QString) override {}
    
    static QStandardItemModel* getDummyModel() {
        static QStandardItemModel model;
        return &model;
    }
    
    QStandardItemModel* getCityModel() override { return getDummyModel(); }
    QStandardItemModel* getTitleModel() override { return getDummyModel(); }
    QStandardItemModel* getCollectModel() override { return getDummyModel(); }
    QStandardItemModel* getBottomCollectModel() override { return getDummyModel(); }
    
    void changeCityFromSearch(QModelIndex) override {}
    void addCollectCity(QModelIndex) override {}
    void changeCityFromCollect(QModelIndex) override {}
    void delCollectCity(QModelIndex) override {}
    void test() override {}
    void loadCollectCity() override {}
    void showCenterAfter() override {}
    void closePrograme() override {}
    void allowLocationYes() override {}
    void allowLocationNo() override {}
    void refresh() override {}
    void addCollectCityFromWidget(QString) override {}
    void delCollectCityFromWidget(QString) override {}
    void changeCityFromTitleSearch(QString) override {}
    void getLAndRCityInfo(QString) override {}
};
static MockInteraction dummy;

// horscreen Mock
horscreen* horscreen::mutual = nullptr;
CurrentWeather* horscreen::m_showCurrentWeather = nullptr;
horscreen::horscreen(QWidget *parent) : QWidget(parent) {}
void horscreen::changeBackgroundImage(CurrentWeather) {}
void horscreen::showLocateWidget() {}
void horscreen::showAllowLocationWidget() {}
bool horscreen::eventFilter(QObject*, QEvent*) { return false; }
void horscreen::resizeEvent(QResizeEvent*) {}
bool horscreen::event(QEvent*) { return false; }
void horscreen::keyPressEvent(QKeyEvent*) {}
void horscreen::hideSearchView() {}
void horscreen::showSearchView() {}
void horscreen::recieveNetSignal(QString) {}
void horscreen::showCurrent(CurrentWeather) {}
void horscreen::showForcast(ForecastWeather) {}
void horscreen::showLifeStyle(LifeStyle) {}
void horscreen::showHourly(QVariant) {}
void horscreen::createTrayIcon(QString) {}
void horscreen::updateTrayIcon(QString) {}
void horscreen::handleIconClickedSub() {}
void horscreen::iconActivated(QSystemTrayIcon::ActivationReason) {}
void horscreen::closeActivated() {}
void horscreen::slotShowTitleSearchCityView(bool) {}
void horscreen::slotShowTitleSearchCityViewUi() {}
void horscreen::slotShowNoNetworkWidget() {}
void horscreen::slotHideNoNetworkWidget() {}
void horscreen::slotChangePCMode() {}
void horscreen::slotChangePadHMode() {}
void horscreen::slotChangePadVMode() {}

// menuModule Mock
menuModule::menuModule(QWidget *parent) : QWidget(parent) {
    menuButton = new QToolButton(this);
}
void menuModule::monitorThemeChanged(QString) {}
void menuModule::setMenuModuleAllowLocationYes() {}

// SearchView Mock
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
