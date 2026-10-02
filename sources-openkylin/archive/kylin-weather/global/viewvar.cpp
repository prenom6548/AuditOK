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
#include "viewvar.h"
#include <QDebug>
static QGSettings * m_gsettings = nullptr;
ViewVar *ViewVar::m_viewVar(nullptr);

ViewVar *ViewVar::getInstance()
{
    if (m_viewVar == nullptr) {
        m_viewVar = new ViewVar;
    }
    return m_viewVar;
}

//热门城市列表
QMap<int, QStringList> ViewVar::HOTCITY_MAP = ViewVar::saveHotCity(); //热门城市列表
//城市item--收藏和搜索
QColor ViewVar::s_colorItem = QColor(120, 120, 120);
QColor ViewVar::s_colorWid = QColor(246, 246, 246, 0);
QColor ViewVar::s_colorSearch = QColor(246, 246, 246, 1);
const QSize ViewVar::COLLECT_BTNSIZE = QSize(16, 16); //收藏城市按钮大小
const QSize ViewVar::COLLECT_BTNSIZE_WHITE = QSize(36, 36); //收藏城市按钮大小
//按钮位置
const int ViewVar::COLLECT_BTNXL = 148;
const int ViewVar::COLLECT_BTNYL = 9;
const int ViewVar::COLLECT_BTNXR = -10;
const int ViewVar::COLLECT_BTNYR = -8;
//文字位置
const int ViewVar::COLLECT_NAMEXL = 16;
const int ViewVar::COLLECT_TMPXL = 119;
const int ViewVar::COLLECT_TMPXL1 = 150;
const int ViewVar::COLLECT_CODEXL = 65;
const int ViewVar::COLLECT_CODEXL1 = 95;
//文字位置-same
const int ViewVar::COLLECT_SAMEYL = 0;
const int ViewVar::COLLECT_SAMEXR = -4;
const int ViewVar::COLLECT_SAMEYR = -4;
//图标位置
const int ViewVar::COLLECT_ICONXL = 93;
const int ViewVar::COLLECT_ICONXL1 = 93;
const int ViewVar::COLLECT_ICONYL = 16;
const int ViewVar::COLLECT_ICONXR = -116;
const int ViewVar::COLLECT_ICONYR = -20;
//按钮位置
const int ViewVar::SEARCH_BTNXL = 97;
const int ViewVar::SEARCH_BTNYL = 9;
const int ViewVar::SEARCH_BTNXR = -10;
const int ViewVar::SEARCH_BTNYR = -10;
//文字位置
const int ViewVar::SEARCH_NAMEXL = 16;
const int ViewVar::SEARCH_NAMEYL = 0;
const int ViewVar::SEARCH_NAMEXR = -4;
const int ViewVar::SEARCH_NAMEYR = -4;
const QString ViewVar::COLLECT_ICONPATH = QString(":/res/weather-icon/PNG16/2x/");
const QString ViewVar::COLLECT_ICONSUFFIX = QString(".png"); //图标后缀
QPen ViewVar::m_collectPen = QPen(Qt::black);

const QRect ViewVar::COLLECT_ITEMRECT = QRect(0, 0, 170, 170); //收藏城市item位置
const QRect ViewVar::SEARCH_ITEMRECT = QRect(0, 0, 170, 170);  //收藏城市item位置
qreal ViewVar::SAME_ITEMRADIUS = 0;                      //收藏和搜索列表城市item圆角
//透明度设置
const qreal ViewVar::SAME_OPACITY_0 = 0;
const qreal ViewVar::SAME_OPACITY_1 = 0.2;
const qreal ViewVar::SAME_OPACITY_2 = 1;

bool ViewVar::s_themeStyle = true; //默认浅色
//收藏
const QSize ViewVar::COLLECT = QSize(630, 450);
const QSize ViewVar::COLLECT_PAD = QSize(724, 161);
// const QSize ViewVar::COLLECT = QSize(608, 360);
const QSize ViewVar::COLLECT_LIST = QSize(630, 360);
const QSize ViewVar::COLLECT_LIST_PAD = QSize(666, 131);
// const QSize ViewVar::COLLECT_LIST = QSize(558, 270);
const QPoint ViewVar::COLLECT_TITLE = QPoint(49, 0);
const int ViewVar::NONECITYX = 49;
const int ViewVar::CITYLISTX = 33;
const int ViewVar::COLLECT_ITEMH = 36;
const int ViewVar::COLLECT_ITEMW = 180;
const int ViewVar::COLLECT_ITEMH_PAD = 48;
const int ViewVar::COLLECT_ITEMW_PAD = 216;
bool ViewVar::m_editStyle = false; // 默认非编辑状态
bool ViewVar::m_existAddBtn = false; // 默认不存在
const QString ViewVar::COLLENTTEXT = "add city";
//热门
const QSize ViewVar::HOT = QSize(630, 208);      //热门
const QSize ViewVar::HOT_LIST = QSize(630, 190); //热门城市列表
const QPoint ViewVar::HOT_TITLE = QPoint(49, 0); //热门-title
const int ViewVar::HOTLISTX = 0;                 //热门城市list x值
const int ViewVar::ITEM_LENGTHM = 3;             //热门城市文字长度最大值
const int ViewVar::ITEM_NUM = 5;                 //热门城市每行数量
const int ViewVar::ITEM_NUMM = 11;               //热门城市每行数量
const int ViewVar::ITEM_POS = 96;                //热门城市item的间隔值
const int ViewVar::ITEM_DIS0 = 6;                //热门城市个数值-行1
const int ViewVar::ITEM_DIS2 = 12;               //热门城市个数值-行2
//热门-按钮大小
const QSize ViewVar::HOT_BTN = QSize(90, 36);
//搜索框
const QSize ViewVar::SEARCH_BOX = QSize(244, 36);
const QSize ViewVar::SEARCH_BOX_PAD = QSize(300, 48);
//搜索框文本间距--左上右下
const QMargins ViewVar::SEARCH_MARGINS = QMargins(34, 0, 0, 0);
//搜索框-图标控件大小
const QSize ViewVar::SEARCH_ICON = QSize(16, 16);
//搜索框-图标位置-x
const int ViewVar::SEARCH_ICON_X0 = 97 - 8;
const int ViewVar::SEARCH_ICON_X1 = 9;

//搜索列表-个尺寸大小
const QSize ViewVar::SEARCH_LIST = QSize(596, 370);
const int ViewVar::SEARCH_ITEMH = 36;  //搜索城市item height值
const int ViewVar::SEARCH_ITEMW = 96; //搜索城市item weight值 180
const int ViewVar::SEARCH_NONEW = 500; //搜索城市无city的控件宽度值
//搜索-收藏-总界面
const QSize ViewVar::SEARCH = QSize(640, 460);
const QSize ViewVar::SEARCH_PAD = QSize(726, 422);
//搜索框位置-y值
const int ViewVar::SEARCH_BOXY = 8;
const QPoint ViewVar::SEARCHLIST_POS = QPoint(36, 54); //搜索列表位置
const int ViewVar::SEARCH_CHANGECITYX = 8;
const int ViewVar::DISTANCE0 = 10; //间隔0--热门和收藏的相互间距
const int ViewVar::DISTANCE1 = 16; //间隔1--热门和收藏与搜索框的相互间距
QString ViewVar::icon_path = QString(":/res/weather-icon/PNG48/1x/");
QStringList ViewVar::platFormList = {"V10SP1-edu"};
QString ViewVar::platForm = ViewVar::platFormType();
bool ViewVar::widgetIsShow = false;

// 主界面图标尺寸
const QSize ViewVar::CITY_ICON = QSize(48, 48);
const QSize ViewVar::CITY_ICON_MAXSIZE = QSize(64, 64);

// 标题栏搜索
const QSize ViewVar::TITLESEARCHITEMSIZE = QSize(292, 48);
bool ViewVar::m_isTitleSearch = false;
QStringList ViewVar::hotMap(QString city, QString cityId)
{
    QStringList hotcity;
    hotcity.append(city);
    hotcity.append(cityId);
    return hotcity;
}

QMap<int, QStringList> ViewVar::saveHotCity()
{
    QMap<int, QStringList> hotcity;

    hotcity.insert(0, hotMap(tr("Beijing"), "101010100"));
    hotcity.insert(1, hotMap(tr("Shanghai"), "101020100"));
    hotcity.insert(2, hotMap(tr("Guangzhou"), "101280101"));
    hotcity.insert(3, hotMap(tr("Shenzhen"), "101280601"));
    hotcity.insert(4, hotMap(tr("Chengdu"), "101270101"));
    hotcity.insert(5, hotMap(tr("Wuhan"), "101200101"));
    hotcity.insert(6, hotMap(tr("Chongqing"), "101040100"));
    hotcity.insert(7, hotMap(tr("Tianjin"), "101030100"));
    hotcity.insert(8, hotMap(tr("Suzhou"), "101190401"));
    hotcity.insert(9, hotMap(tr("Nanjing"), "101190101"));
    hotcity.insert(10, hotMap(tr("Xian"), "101110101"));
    hotcity.insert(11, hotMap(tr("Changsha"), "101250101"));
    hotcity.insert(12, hotMap(tr("Hangzhou"), "101210101"));
    hotcity.insert(13, hotMap(tr("Zhengzhou"), "101180101"));
    hotcity.insert(14, hotMap(tr("Taibei"), "101340101"));
    hotcity.insert(15, hotMap(tr("Hongkong"), "101320101"));
    hotcity.insert(16, hotMap(tr("Macao"), "101330101"));
    hotcity.insert(17, hotMap(tr("Lasa"), "101140101"));

    return hotcity;
}
void ViewVar::initGsetting()
{
    m_gsettings = new QGSettings("org.ukui.style");
    connect(m_gsettings,&QGSettings::changed,ViewVar::getInstance(), &ViewVar::updateButtons);
    updateButtons("widgetThemeName");
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, ViewVar::getInstance(),
            [=]() {
                monitorGsetting();
            });
    monitorGsetting();
    return;
}

void ViewVar::initFontSize()
{
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemFontSizeChange, ViewVar::getInstance(),
            [=]() {
                //获取字号的值
                int fontSizeKey = kdk::kabase::Gsettings::getSystemFontSize().toInt();
                //发送改变信号
                if (fontSizeKey > 0) {
                    if (fontSizeKey > 15) {
                        fontSizeKey = 15;
                    }
                    emit ViewVar::getInstance()->setFontSizeSignal(fontSizeKey);
                }
            });
    //启动时设置字号
    int fontSizeKey = 11; //系统默认字号
    int fontSizeKeyTmp = kdk::kabase::Gsettings::getSystemFontSize().toInt();
    if (fontSizeKeyTmp > 0) {
        fontSizeKey = fontSizeKeyTmp;
    }
    if (fontSizeKey > 15) {
        fontSizeKey = 15;
    }
    emit ViewVar::getInstance()->setFontSizeSignal(fontSizeKey);
}

//监听主题配置文件
void ViewVar::monitorGsetting()
{
    QString themeStyle = kdk::kabase::Gsettings::getSystemTheme().toString();
    if ("ukui-dark" == themeStyle || "ukui-black" == themeStyle) {
        ViewVar::s_colorItem = QColor(255, 255, 255, 255 * 0.6);
        ViewVar::s_colorWid = QColor(0, 0, 0, 0);
        ViewVar::s_colorSearch = QColor(0, 0, 0, 1);
        ViewVar::m_collectPen = QPen(Qt::white);
        s_themeStyle = false;
    } else {
        ViewVar::s_colorItem = QColor(120, 120, 120);
        //        ViewVar::s_colorWid = QColor(246,246,246,0);
        //        ViewVar::s_colorSearch = QColor(246,246,246,1);
        ViewVar::s_colorWid = QColor(255, 255, 255, 0);
        ViewVar::s_colorSearch = QColor(255, 255, 255, 1);
        ViewVar::m_collectPen = QPen(Qt::black);
        s_themeStyle = true;
    }
    emit ViewVar::getInstance()->monitorThemeChanged(themeStyle);
}

void ViewVar::updateButtons(QString key)
{
    if (key == "widgetThemeName"){
        QString themeData = m_gsettings->get("widgetThemeName").toString();
        if (themeData == "classical"){
            ViewVar::SAME_ITEMRADIUS = 0;
        } else {
            ViewVar::SAME_ITEMRADIUS = 6;
        }
    }
}

QString ViewVar::platFormType()
{
    QString platformName;
    kdk::kabase::SystemInformation systemInformation;
    platformName = systemInformation.getProjectCodeName();
    return platformName;
}

ViewVar::ViewVar()
{
    m_rotateChange = new RotateChangeInfo(this);
    connect(m_rotateChange, &RotateChangeInfo::sigRotationChanged, this, &ViewVar::slotChangeModel);
    getCurrentMode();
}

void ViewVar::slotChangeModel(bool isPCMode, int width, int height)
{
    m_currentSize.first = width;
    m_currentSize.second = height;
    if (isPCMode) {
        m_currentMode = CurrentMode::PCMode;
        emit sigChangeModeToPc();
    } else {
        if (width > height) {
            m_currentMode = CurrentMode::PadHMode;
            emit sigChangeModeToPadH();
        } else {
            m_currentMode = CurrentMode::PadVMode;
            emit sigChangeModeToPadV();
        }
    }
    return;
}

CurrentMode ViewVar::getCurrentMode()
{
    bool isPCMode = m_rotateChange->getCurrentMode();
    QPair<int, int> currentSize = m_rotateChange->getCurrentScale();
    if (!m_rotateChange->m_existDbus) {
        m_currentMode = CurrentMode::PCMode;
        return m_currentMode;
    }
    m_currentSize = currentSize;
    if (isPCMode) {
        m_currentMode = CurrentMode::PCMode;
    } else {
        if (currentSize.first > currentSize.second) {
            m_currentMode = CurrentMode::PadHMode;
        } else {
            m_currentMode = CurrentMode::PadVMode;
        }
    }
    return m_currentMode;
}
