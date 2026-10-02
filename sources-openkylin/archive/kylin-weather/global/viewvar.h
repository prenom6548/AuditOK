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
#ifndef VIEWVAR_H
#define VIEWVAR_H
#define FITTHEMESTYLE "org.ukui.style"
#include <QObject>
#include <QString>
#include <QGSettings>
#include <QDir>
#include <QSize>
#include <QRect>
#include <QPixmap>
#include <QStandardPaths>
#include <QDebug>
#include <QPen>
#include <QIcon>
#include <unistd.h>
#include <QTranslator>
#include <QMargins>
#include <gsettings.hpp>
#include <system_information.hpp>
#include "rotatechangeinfo.h"

enum CurrentMode {
    PCMode = 0,
    PadHMode,
    PadVMode,
};

class ViewVar : public QObject
{
    Q_OBJECT
public:
    static QMap<int, QStringList> HOTCITY_MAP; //热门城市列表
    static QColor s_colorItem;                 //收藏城市颜色
    static QColor s_colorWid;                  //收藏城市颜色
    static QColor s_colorSearch;               //收藏城市颜色
    static const QSize COLLECT_BTNSIZE;        //收藏城市按钮大小
    static const QSize COLLECT_BTNSIZE_WHITE;        //收藏城市按钮大小
    static QString icon_path;                  //主界面图标路径
    //按钮位置-收藏
    static const int COLLECT_BTNXL;
    static const int COLLECT_BTNYL;
    static const int COLLECT_BTNXR;
    static const int COLLECT_BTNYR;
    // pen
    static QPen m_collectPen;
    //文字位置-收藏
    static const int COLLECT_NAMEXL;
    static const int COLLECT_TMPXL;
    static const int COLLECT_TMPXL1;
    static const int COLLECT_CODEXL;
    static const int COLLECT_CODEXL1;
    //文字位置-same-收藏
    static const int COLLECT_SAMEYL;
    static const int COLLECT_SAMEXR;
    static const int COLLECT_SAMEYR;
    //图标位置-收藏
    static const int COLLECT_ICONXL;
    static const int COLLECT_ICONXL1;
    static const int COLLECT_ICONYL;
    static const int COLLECT_ICONXR;
    static const int COLLECT_ICONYR;

    //按钮位置-搜索
    static const int SEARCH_BTNXL;
    static const int SEARCH_BTNYL;
    static const int SEARCH_BTNXR;
    static const int SEARCH_BTNYR;
    //文字位置-搜索
    static const int SEARCH_NAMEXL;
    static const int SEARCH_NAMEYL;
    static const int SEARCH_NAMEXR;
    static const int SEARCH_NAMEYR;
    static const QString COLLECT_ICONPATH;   //图标路径
    static const QString COLLECT_ICONSUFFIX; //图标后缀
    static const QRect COLLECT_ITEMRECT;     //收藏城市item
    static const QRect SEARCH_ITEMRECT;      //搜索城市item
    static qreal SAME_ITEMRADIUS;      // item圆角
    static const qreal SAME_OPACITY_0;       //透明度-0
    static const qreal SAME_OPACITY_1;       //透明度-0.2
    static const qreal SAME_OPACITY_2;       //透明度-1

    static bool s_themeStyle; //主题变化
    //各窗口大小
    //收藏
    static const QSize COLLECT;        //收藏
    static const QSize COLLECT_PAD;    //收藏(PAD)
    static const QSize COLLECT_LIST;   //收藏
    static const QSize COLLECT_LIST_PAD;//收藏(Pad)
    static const QPoint COLLECT_TITLE; //收藏-title
    static const int NONECITYX;        //无收藏城市x值
    static const int CITYLISTX;        //收藏城市list x值
    static const int COLLECT_ITEMH;    //收藏城市item height值
    static const int COLLECT_ITEMW;    //收藏城市item weight值
    static const int COLLECT_ITEMH_PAD;//收藏城市item height值(pad)
    static const int COLLECT_ITEMW_PAD;//收藏城市item weight值(pad)
    //热门
    static const QSize HOT;        //热门
    static const QSize HOT_LIST;   //热门城市列表
    static const QPoint HOT_TITLE; //热门-title
    static const int HOTLISTX;     //热门城市list x值
    static const int ITEM_LENGTHM; //热门城市文字长度最大值
    static const int ITEM_NUM;     //热门城市每行数量
    static const int ITEM_NUMM;    //热门城市2行数量
    static const int ITEM_POS;     //热门城市item的间隔值
    static const int ITEM_DIS0;    //热门城市个数值-行1
    static const int ITEM_DIS2;    //热门城市个数值-行2
    //热门-按钮
    static const QSize HOT_BTN; //热门-按钮
    //搜索框
    static const QSize SEARCH_BOX;
    static const QSize SEARCH_BOX_PAD;

    static const QMargins SEARCH_MARGINS; //搜索框文本间距--左上右下
    static const QSize SEARCH_ICON;       //搜索框-图标控件大小
    static const int SEARCH_ICON_X0;      //搜索框-图标位置-x
    static const int SEARCH_ICON_X1;
    //搜索列表-个尺寸大小
    static const QSize SEARCH_LIST;
    static const int SEARCH_ITEMH; //搜索城市item height值
    static const int SEARCH_ITEMW; //搜索城市item weight值
    static const int SEARCH_NONEW; //搜索城市无city的控件宽度值


    //搜索-收藏-总界面
    static const QSize SEARCH;
    static const QSize SEARCH_PAD;       // 收藏界面尺寸（PAD）
    static const int SEARCH_BOXY;        //搜索框位置-y值
    static const QPoint SEARCHLIST_POS;  //搜索列表位置
    static const int SEARCH_CHANGECITYX; //交换位置的x值
    static const int DISTANCE0;          //间隔0--热门和收藏的相互间距
    static const int DISTANCE1;          //间隔1--热门和收藏与搜索框的相互间距
    static bool m_editStyle;             // 收藏城市界面是否在编辑状态
    static bool m_existAddBtn;             // 收藏城市界面是否存在添加按钮
    static const QString COLLENTTEXT;  // 收藏城市按钮

    static void initGsetting();
    static void initFontSize();
    static QMap<int, QStringList> saveHotCity(); //存储热门城市
    static ViewVar *getInstance();               //单例
    static QStringList platFormList;             //平台
    static QString platForm;                     //平台
    static bool widgetIsShow;                    //默认没有显示窗口
    
    // 标题栏搜索
    static const QSize TITLESEARCHITEMSIZE;
    static bool m_isTitleSearch;

    // 主界面图标尺寸
    static const QSize CITY_ICON;          // 主界面的城市图标
    static const QSize CITY_ICON_MAXSIZE;  // 主界面的城市图标（平板模式）

    // 搜藏城市尺寸
    static const QSize COLLECT_SIZE;
    static const QSize COLLECT_SIZE_PAD;
    static const QSize COLLECT_ITEM_SIZE;
    static const QSize COLLECT_ITEM_SIZE_PAD;

    // 获取当前模式
    CurrentMode getCurrentMode();
    CurrentMode m_currentMode = CurrentMode::PCMode;
    QPair<int, int> m_currentSize;

private:
    ViewVar();
    static QStringList hotMap(QString city, QString cityId);

    static void monitorGsetting(); //主题gsettings监听
    static void updateButtons(QString key);
    static ViewVar *m_viewVar;     //单例指针
    static QString platFormType(); //获得平台名字

    RotateChangeInfo *m_rotateChange = nullptr; // 平板切换
    void slotChangeModel(bool mode,int width,int height);
signals:
    void monitorThemeChanged(QString themeType);
    void setFontSizeSignal(int);
    void sigChangeModeToPadH(); // 切换平板横屏
    void sigChangeModeToPadV(); // 切换平板竖屏
    void sigChangeModeToPc(); // 切换PC模式
};

#endif // VIEWVAR_H
