/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "primarymanager.h"
#include "windowmanager/windowmanager.h"
#include <math.h>


PrimaryManager::PrimaryManager()
{
    this->m_on_wayland = checkWayland();
    if(m_on_wayland){
        //支持 wayland
        mDbusXrandInter = new QDBusInterface(DBUS_NAME,
                                             DBUS_PATH,
                                             DBUS_INTERFACE,
                                             QDBusConnection::sessionBus());
        this->m_nScreen_x = getScreenGeometry("x");
        this->m_nScreen_y = getScreenGeometry("y");
        this->m_screenWidth = getScreenGeometry("width");
        this->m_screenHeight = getScreenGeometry("height");
        qDebug("初始化, geometry is  x=%d, y=%d, width=%d, height=%d", m_nScreen_x, m_nScreen_y, m_screenWidth, m_screenHeight);
        /*监听dbus变化  更改主屏幕时，会进行信号发送 */
        connect(mDbusXrandInter, SIGNAL(screenPrimaryChanged(int,int,int,int)),
                this, SLOT(priScreenChanged(int,int,int,int)));
    }else{
        //不支持 wayland
        m_pDeskWgt = QApplication::desktop();
        m_adaptScreenInfo = new adaptScreenInfo();
        m_adaptScreenInfo->setObjectName("ukui-clock_adaptScreenInfo_m_adaptScreenInfo");
        updateInfoByAdapt();
        //当改变屏幕分辨率时 重新计算 主屏坐标 屏幕宽高
        connect(QApplication::primaryScreen(), &QScreen::geometryChanged, this, &PrimaryManager::updateInfoByAdapt);
        //主屏发生变化槽函数 重新计算 主屏坐标 屏幕宽高
        connect(m_pDeskWgt, &QDesktopWidget::primaryScreenChanged, this, &PrimaryManager::updateInfoByAdapt);
        //屏幕数量改变时 重新计算 主屏坐标 屏幕宽高
        connect(m_pDeskWgt, &QDesktopWidget::screenCountChanged, this, &PrimaryManager::updateInfoByAdapt);
    }

}
/**
 * @brief 主屏幕变化监听函数
 */
void PrimaryManager::priScreenChanged(int x, int y, int width, int height)
{
    QString priName;
    priName = getScreenName ("priScreenName");
    this->m_nScreen_x=x;
    this->m_nScreen_y=y;
    this->m_screenWidth=width;
    this->m_screenHeight=height;
    qDebug("primary screen  changed, geometry is  x=%d, y=%d, width=%d, height=%d", x, y, width, height);
}
/**
 * @brief 根据adaptScreenInfo更新数据
 */
void PrimaryManager::updateInfoByAdapt()
{
    this->m_nScreen_x = m_adaptScreenInfo->m_nScreen_x;
    this->m_nScreen_y = m_adaptScreenInfo->m_nScreen_y;
    this->m_screenWidth = m_adaptScreenInfo->m_screenWidth;
    this->m_screenHeight = m_adaptScreenInfo->m_screenHeight;
}
/**
 * @brief 启动获取主屏幕坐标 x/y/width/height；  以及基于Kscreen获取的主屏幕名字
 */
void PrimaryManager::start()
{
    int priX, priY, priWid, priHei;
       QString priName;
       priX = getScreenGeometry("x");
       priY = getScreenGeometry("y");
       priWid = getScreenGeometry("width");
       priHei = getScreenGeometry("height");
       priName = getScreenName ("priScreenName");

       qDebug("Start: Primary screen geometry is x=%d, y=%d, width=%d, height=%d,",
              priX, priY, priWid, priHei);
       qDebug()<<"Primary screen name = " << priName;
}
/**
 * @brief 根据dbus 提供method 获取X/Y/width/height 参数
 */
int PrimaryManager::getScreenGeometry(QString methodName)
{
    int res = 0;
        QDBusMessage message = QDBusMessage::createMethodCall(DBUS_NAME,
                                   DBUS_PATH,
                                   DBUS_INTERFACE,
                                   methodName);
        QDBusMessage response = QDBusConnection::sessionBus().call(message);
        if (response.type() == QDBusMessage::ReplyMessage)
        {
            if(response.arguments().isEmpty() == false) {
                int value = response.arguments().takeFirst().toInt();
                res = value;
                qDebug() << value;
            }
        } else {
            qDebug()<<methodName<<"called failed";
        }
        return res;
}
/**
 * @brief 根据 dbus 提供method 获取基于kscreen获得的屏幕名字
 */
QString PrimaryManager::getScreenName(QString methodName)
{
    QString res = 0;
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_NAME,
                               DBUS_PATH,
                               DBUS_INTERFACE,
                               methodName);
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        if(response.arguments().isEmpty() == false) {
            QString value = response.arguments().takeFirst().toString();
            res = value;
            qDebug() << value;
        }
    } else {
        qDebug()<<methodName<<"called failed";
    }
    return res;
}
/**
 * @brief 如果应用必须要用QScreen 可进行参数判断，代码如下：
 */
void PrimaryManager::init()
{
    int dX, dY, dWidth, dHeight;
        QScreen *mPriScreen;

        dX = getScreenGeometry("x");
        dY = getScreenGeometry("y");
        dWidth = getScreenGeometry("width");
        dHeight = getScreenGeometry("height");
        //根据真实值判断哪一个是主屏
        for (auto screen : QApplication::screens()){
            if (screen->geometry().x() == dX && screen->geometry().y() == dY) {
                 mPriScreen = screen;
            }
        }
}

PrimaryManager::~PrimaryManager()
{
    delete mDbusXrandInter;
    delete m_adaptScreenInfo;
}

int PrimaryManager::getScreenWidth() const
{
    return m_screenWidth;
}

int PrimaryManager::getScreenHeight() const
{
    return m_screenHeight;
}

int PrimaryManager::getNScreen_x() const
{
    return m_nScreen_x;
}

int PrimaryManager::getNScreen_y() const
{
    return m_nScreen_y;
}
/**
 * @brief 多屏幕下，Natice_alarm移动
 * @param spostion SP_LEFT 左下角 SP_RIGHT 右下角 SP_CENTER 居中
 * @param
 *
 * @return 返回说明
 */
void PrimaryManager::moveUnderMultiScreen(PrimaryManager::ScreenPosition spostion,QWidget * tempDialog,int hiddenFlag)
{
    QScreen *screen=QGuiApplication::primaryScreen ();
    int screen_width = screen->geometry().width();
    int screen_height = screen->geometry().height();
    int x = getNScreen_x();
    int y = getNScreen_y();
    qWarning()<<"dbq-screen_width"<<screen_width<<"screen_height"<<screen_height<<"x"<<x<<"y"<<y;
    int widthRadio = 20;
    int heightRadio = 14;

    int moveWidth = 0;
    int moveHeight =0;
    int diaWidth = tempDialog->width();
    int diaHeight = tempDialog->height();

    switch (spostion) {
    case SP_LEFT:
    {
        moveWidth = x+round(tempDialog->width()+round(1.0/widthRadio*screen_width));
        moveHeight = y+round(screen_height-tempDialog->height()-round(1.0/heightRadio*screen_height));
    }break;
    case SP_RIGHT:
    {
        moveWidth = x+round(screen_width-tempDialog->width());
        moveHeight = y+round(screen_height-tempDialog->height()-50);
    }break;
    case SP_CENTER:
    {
        moveWidth = x+round((screen_width-tempDialog->width())*1.0/2);
        moveHeight = y+round((screen_height-tempDialog->height())*1.0/2);
    }break;
    case UP_LEFT:
    {
        moveWidth = x+round(tempDialog->width()+round(1.0/widthRadio*screen_width));
        moveHeight = y+round(tempDialog->height()+round(1.0/heightRadio*screen_height));
    }break;
    case UP_RIGHT:
    {
        moveWidth = x+round(screen_width-tempDialog->width()-round(1.0/widthRadio*screen_width));
        moveHeight = y+round(1.0/20*screen_height);
    }break;
    case UP_CENTER:
    {
        moveWidth = x+round((screen_width-tempDialog->width())*1.0/2);
        moveHeight = y+round((screen_height-tempDialog->height())*1.0/2);
    }break;
    default:
    {}
    }
    QRect finalRect = QRect(moveWidth,moveHeight,diaWidth,diaHeight);
    if (hiddenFlag == 0){
        finalRect = QRect(moveWidth,moveHeight-tempDialog->height(),diaWidth,diaHeight);
    }
    qWarning()<<"dbq-finalRect"<<finalRect;
    kdk::WindowManager::setGeometry(tempDialog->windowHandle(),finalRect);

}
/**
 * @brief 根据环境变量判断是否支持wayland
 */
bool PrimaryManager::checkWayland()
{
    QByteArray byte = qgetenv("DESKTOP_SESSION");
    QString info =QString::fromLocal8Bit(byte);
    qWarning()<<"dbq-info"<<info<<info.contains("wayland");
    if(info.contains("wayland") || info.contains("kylin-wlcom")){
        return true;
    }else{
        return false;
    }
}
