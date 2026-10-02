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
#ifndef INTERACTION_H
#define INTERACTION_H

#include <QObject>
#include <QPixmap>
#include <QVariant>
#include <QFileInfo>
#include <QStandardItemModel>
#include "global/corevar.h"
#include <QPersistentModelIndex>
class coreinteraction;

class Interaction : public QObject
{
    Q_OBJECT
    friend class TestCore;
    friend class TestCoreInteraction;
    friend class TestViewControl;

signals:
    /*
     * 接口功能：后端初始化后，初始化前端
     * 接口场景：网络诊断完成后，初始化前端
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void toStartView();
    /*
     * 接口功能：网络错误，给ui层传递无网络信息
     * 接口场景：程序启动时，检测网络连接状况
     * 接口类型：信号，需要绑定
     * 参数类型：QString
     * 参数描述：返回网络错误信息
     */
    void netUiShow(QString);
    /*
     * 接口功能：请求服务器错误
     * 接口场景：访问服务器拿数据--目前返回的是code值
     * 接口类型：信号，需要绑定
     * 参数类型：int
     * 参数描述：返回访问服务器错误信息
     */

    void serverUiShow(QString);
    /*
     * 接口功能：数据解析
     * 接口场景：服务器返回数据为空或不全
     * 接口类型：信号，需要绑定
     * 参数类型：QString
     * 参数描述：返回数据解析错误信息
     */
    void dataUiShow(QString);
    /*
     * 接口功能：定位失败
     * 接口场景：定位时
     * 接口类型：信号，需要绑定
     * 参数类型：QString
     * 参数描述：返回定位失败
     */
    void geoUiShow(QString);
    /*
     * 接口功能：网络超时
     * 接口场景：各处需要检测超时的地方
     * 接口类型：信号，需要绑定
     * 参数类型：bool
     * 参数描述：网络超时
     */
    void netTimeout(QString);
    /*
     * 接口功能：显示生活指数信息
     * 接口场景：显示生活指数信息
     * 接口类型：信号，需要绑定
     * 参数类型：struct
     * 参数描述：生活指数信息
     */
    void showLifeStyle(LifeStyle m_lifeStyle); //发送信号，显示生活指数信息
    /*
     * 接口功能：显示当前城市信息
     * 接口场景：显示当前城市信息
     * 接口类型：信号，需要绑定
     * 参数类型：struct
     * 参数描述：当前城市信息
     */
    void showCurrent(CurrentWeather m_currentWeather); //发送信号，显示当前城市信息
    /*
     * 接口功能：创建托盘
     * 接口场景：创建托盘
     * 接口类型：信号，需要绑定
     * 参数类型：QString
     * 参数描述：创建托盘
     */
    void createTrayIcon(QString code); //发送信号，显示当前城市信息
    /*
     * 接口功能：显示城市预测信息
     * 接口场景：显示城市预测信息
     * 接口类型：信号，需要绑定
     * 参数类型：struct
     * 参数描述：当前城市信息
     */
    void showForcast(ForecastWeather forecastWeather); //发送信号，显示城市预测信息
    /*
     * 接口功能：显示逐小时信息
     * 接口场景：显示逐小时信息
     * 接口类型：信号，需要绑定
     * 参数类型：QVariant
     * 参数描述：逐小时信息
     */
    void showHourly(QVariant var); //发送信号，显示逐小时信息
    /*
     * 接口功能：搜索错误，给ui层传递无搜索结果信息
     * 接口场景：输入或不输入字符，无搜索内容
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：返回搜索错误信息
     */
    void searchNull();
    /*
     * 接口功能：搜索结果，给ui层传递搜索结果信息
     * 接口场景：输入或不输入字符，有/无搜索内容
     * 接口类型：信号，需要绑定
     * 参数类型：bool
     * 参数描述：返回有/无搜索结果
     */
    void searchUiShow(bool hasCity);
    /*
     * 接口功能：收藏城市为空
     * 接口场景：判断收藏城市是否为空
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void collectNull();
    /*
     * 接口功能：收藏城市不为空
     * 接口场景：判断收藏城市是否为空
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void collectShow();
    /*
     * 接口功能：关闭程序
     * 接口场景：点击关闭按钮和托盘退出选项
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void closePro();
    /*
     * 接口功能：在屏幕中间显示
     * 接口场景：显示
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void showInCenter();

    /*
     * 接口功能：隐藏/显示主界面底部收藏城市列表,收藏城市+定位城市<2时，隐藏列表
     * 接口场景：隐藏/显示
     * 接口类型：信号，需要绑定
     * 参数类型：int
     * 参数描述：item的个数
     */
    void sigShowBottomList(int itemSize);

    /*
     * 接口功能：后端处理完成【取消收藏】，前端UI改变
     * 接口场景：收藏界面编辑状态删除收藏城市
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void sigDeleCollectCity();

    /*
     * 接口功能：后端处理完成【收藏】，前端UI改变
     * 接口场景：收藏界面收藏城市
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void sigAddCollectCity();
    
    /*
     * 接口功能：后端加载完底部model，前端UI改变
     * 接口场景：界面初始化
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void sigBottomModeGet();

    /*
     * 接口功能：显示右侧城市信息
     * 接口场景：城市滑动切换城市
     * 接口类型：信号，需要绑定
     * 参数类型：无
     * 参数描述：无
     */
    void sigShowLAndRCityInfo(QList<CurrentWeather> cityList);

public:
    /*
     * 接口功能：单例取指针，供UI层实例化接口对象
     * 接口场景：程序启动时，首次调用要放到主界面构造函数的第一行
     * 接口类型：函数，直接调用
     * 返回类型：Interaction
     * 返回描述：接口对象
     */
    static Interaction *getInstance();

    /*
     * 接口功能：初始化核心模块
     * 接口场景：程序启动时，放在主界面构造函数的第二行，只需调用这一次，多次调用无效果
     * 接口类型：函数，直接调用
     * 参数类型：QStringList
     * 参数描述：程序启动时传入的参数，一般来自QApplication::arguments()
     */
    virtual void creatCore(const QStringList &list) = 0;
    /*
     * 接口功能：切换城市
     * 接口场景：点击城市切换主界面城市
     * 接口类型：函数，直接调用
     */
    virtual void changeCity(QString cityCode) = 0; //切换到目标城市

    /*
     * 接口功能：搜索城市
     * 接口场景：在搜索框输入字符
     * 接口类型：函数，直接调用
     */
    virtual void searchResult(QString inputText) = 0; //切换到目标城市
    /*
     * 接口功能：获取model指针
     * 接口场景：获取城市列表
     * 接口类型：函数，直接调用
     * 备注：对应QTabelView
     */
    virtual QStandardItemModel *getCityModel() = 0; //获取搜索model指针
    
    /*
     * 接口功能：获取model指针
     * 接口场景：获取城市列表
     * 接口类型：函数，直接调用
     * 备注：对应QListView
     */
    virtual QStandardItemModel *getTitleModel() = 0; //获取搜索model指针
    
    /*
     * 接口功能：获取收藏model指针
     * 接口场景：获取城市列表
     * 接口类型：函数，直接调用
     * 备注：对应QTabelView
     */
    virtual QStandardItemModel *getCollectModel() = 0; //获取收藏model指针

    /*
     * 接口功能：获取主界面最下面列表收藏model指针
     * 接口场景：获取城市列表
     * 接口类型：函数，直接调用
     * 备注：对应QTabelView
     */
    virtual QStandardItemModel *getBottomCollectModel() = 0; //获取收藏model指针

    /*
     * 接口功能：从搜索列表切换城市
     * 接口场景：点击城市切换主界面城市
     * 接口类型：函数，直接调用
     */
    virtual void changeCityFromSearch(QModelIndex index) = 0; //切换到目标城市
    /*
     * 接口功能：从搜索列表添加城市到收藏城市
     * 接口场景：点击“+”号按钮添加城市
     * 接口类型：函数，直接调用
     */
    virtual void addCollectCity(QModelIndex index) = 0; //切换到目标城市
    /*
     * 接口功能：从收藏城市列表切换城市
     * 接口场景：点击城市切换主界面城市
     * 接口类型：函数，直接调用
     */
    virtual void changeCityFromCollect(QModelIndex index) = 0; //切换到目标城市
    /*
     * 接口功能：删除收藏城市列表的城市
     * 接口场景：点击删除按钮添加城市
     * 接口类型：函数，直接调用
     */
    virtual void delCollectCity(QModelIndex index) = 0; //切换到目标城市
    /*
     * 接口功能：初始化核心模块
     * 接口场景：程序启动时，放在主界面构造函数的第二行，只需调用这一次，多次调用无效果
     * 接口类型：函数，直接调用
     * 参数类型：QStringList
     * 参数描述：程序启动时传入的参数，一般来自QApplication::arguments()
     */
    virtual void test() = 0;
    /*
     * 接口功能：加载收藏城市列表
     * 接口场景：show收藏城市列表时，发送此信号
     * 接口类型：函数，直接调用
     * 参数类型：无
     * 参数描述：无
     */
    virtual void loadCollectCity() = 0;

    /*
     * 接口功能：加载收藏城市列表
     * 接口场景：show收藏城市列表时，发送此信号
     * 接口类型：函数，直接调用
     * 参数类型：无
     * 参数描述：无
     */
    virtual void showCenterAfter() = 0;
    /*
     * 接口功能：退出程序
     * 接口场景：点击关闭，退出等按键
     * 接口类型：函数，可直接调用
     * 参数类型：无
     * 参数描述：无
     */
    virtual void closePrograme() = 0;
    /*
     * 接口功能：允许定位
     * 接口场景：点击菜单栏定位选项的允许定位
     * 接口类型：
     * 参数类型：无
     * 参数描述：无
     */
    virtual void allowLocationYes() = 0;
    /*
     * 接口功能：不允许定位
     * 接口场景：点击菜单栏定位选项的不允许定位
     * 接口类型：
     * 参数类型：无
     * 参数描述：无
     */
    virtual void allowLocationNo() = 0;

    /*
     * 接口功能：重试刷新网络
     * 接口场景：点击网络异常界面重试按钮
     * 接口类型：
     * 参数类型：无
     * 参数描述：无
     */
    virtual void refresh() = 0;
    
    /*
     * 接口功能：收藏城市
     * 接口场景：点击主界面的收藏城市按钮
     * 接口类型：
     * 参数类型：无
     * 参数描述：无
     */
    virtual void addCollectCityFromWidget(QString cityId) = 0;
 
    /*
     * 接口功能：取消收藏城市
     * 接口场景：点击主界面的取消收藏按钮
     * 接口类型：函数，直接调用
     * 备注：对应QListView
     */
    virtual void delCollectCityFromWidget(QString cityId) = 0; 
 
    /*
     * 接口功能：向后端发出信号获取天气数据
     * 接口场景：点击标题栏的搜索列表某一行
     * 接口类型：函数，直接调用
     * 备注：对应QListView
     */
    virtual void changeCityFromTitleSearch(QString cityId) = 0; //获取搜索model指针

    /*
     * 接口功能：左右侧城市获取
     * 接口场景：初始化后获取左右城市信息
     * 接口类型：函数，直接调用
     * 备注：对应QListView
     */
    virtual void getLAndRCityInfo(QString cityId) = 0; 
public:
    virtual ~Interaction();
private:
    static Interaction *m_interaction; //单例指针
};


#endif // INTERACTION_H
