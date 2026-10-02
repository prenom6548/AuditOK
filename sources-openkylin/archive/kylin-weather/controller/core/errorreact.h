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
#ifndef ERRORREACT_H
#define ERRORREACT_H

#include <QObject>
#include <QDebug>
class ErrorReact : public QObject
{
    Q_OBJECT
public:
    void networkFeedBack(bool isOnline, QString errorString); //网络错误码
    void serverFeedBack(bool isGetData, QString stateCode);   //服务器错误码
    void dataFeedBack(bool isDataError, QString dataError);   //数据解析错误码
    void searchFeedBack(bool hasCity);                        //本地搜索错误
    void networkTimeout(QString Timeout);                     //超时
    void geoFail(QString geoError);                           //定位失败
    void collectAddError(QString collectAdd);                 //添加城市异常
    /*
     * 接口功能：单例取指针
     * 接口场景：程序启动时，首次调用要放到主界面构造函数的第一行
     * 接口类型：函数，直接调用
     * 返回类型：ErrorReact
     * 返回描述：接口对象
     */
    static ErrorReact *getInstance();

private:
    ErrorReact();
    static ErrorReact *m_errorReact;



signals:
    void netUiShow(QString errorString);  //网络错误详情
    void serverUiShow(QString stateCode); //服务器错误码
    void dataUiShow(QString dataError);   //数据解析错误码
    void geoUiShow(QString geoError);     //定位失败
                                          //    void searchUiShow(bool hasCity);//搜索结果
                                          //    void searchNull();//搜索为空或输入为空
    void netTimeout(QString isTimeout);   //超时
};

#endif // ERROR_H
