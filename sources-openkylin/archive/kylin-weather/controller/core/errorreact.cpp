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
#include "errorreact.h"
ErrorReact *ErrorReact::m_errorReact = nullptr;
ErrorReact *ErrorReact::getInstance()
{
    if (m_errorReact == nullptr) {
        m_errorReact = new ErrorReact;
    }
    return m_errorReact;
}

ErrorReact::ErrorReact() {}
//网络错误
void ErrorReact::networkFeedBack(bool isOnline, QString errorString)
{
    if ("无网络" == errorString) {
        emit netUiShow(tr("No internet connection. Please check your connection and try again."));
        return;
    }
    emit netUiShow(tr("Network connection error. Please check your network or the website address and try again."));
}
//请求服务器模块
void ErrorReact::serverFeedBack(bool isGetData, QString stateCode)
{
    qDebug() << "请求服务器返回错误：异常等状况" << isGetData << stateCode;
    emit serverUiShow(tr("Abnormal access address!"));
}
//数据解析模块
void ErrorReact::dataFeedBack(bool isDataError, QString dataError)
{
    qDebug() << "数据解析错误" << isDataError << dataError;
    emit dataUiShow(tr("Data parsing error!"));
}
//搜索有关---暂时不用，在core里处理
void ErrorReact::searchFeedBack(bool hasCity)
{
    qDebug() << "搜索问题" << hasCity;
    //    if (hasCity == false) {
    //        //给搜索列表显示两个控件的显隐
    //        emit searchNull();
    //    }
    //    //结果如何-传递给前端搜索-收藏界面进行界面的调整和显示
    //    emit searchUiShow(hasCity);
}
//网络超时部分
void ErrorReact::networkTimeout(QString Timeout)
{
    qDebug() << "超时:";
    // Timeout--true:为主界面或者切换主界面超时，--false:为搜索部分超时
    emit netTimeout(tr("Network Timeout!"));
}
//定位失败
void ErrorReact::geoFail(QString geoError)
{
    qDebug() << "定位失败";
    emit geoUiShow(tr("Seek failed!"));
}
//添加收藏城市等暂时无需界面显示的错误
void ErrorReact::collectAddError(QString collectAdd)
{
    qDebug() << collectAdd;
}
