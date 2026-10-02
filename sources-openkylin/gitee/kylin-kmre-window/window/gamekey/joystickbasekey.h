/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JOYSTICKBASEKEY_H
#define JOYSTICKBASEKEY_H

#include <QWidget>
#include <QKeyEvent>
#include <QLineEdit>

#include <QGraphicsOpacityEffect>
const int DEFAULT_DELETE_KEY = 20;  //所有的都用

class KmreWindow;

class JoystickBaseKey : public QWidget
{
    Q_OBJECT
public:
    JoystickBaseKey(KmreWindow* window, int type);
    ~JoystickBaseKey();

    virtual void enableEdit(bool enable) = 0;
    virtual void updateSize(bool update_panel = true) = 0;
    virtual void updatePos();
    enum KeyType {
        SINGLEKEY = 1,
        COMBOKEY,
        AUTOMETHOD,
        JOYLEFT,
        JOYRIGHT,
        DIRECTION,
        FIRE,
        CROSSHAIR,
        VIEW,
        ACCELATE
    };

    void showKey(bool show);
    void setOpacity(double opacity);
    void setIsBeEdti(bool isBeEdit){mIsBeEdit = isBeEdit;}
    bool getIsBeEdit(){return mIsBeEdit;}
    bool isInEditing(){return m_enableEdit;}
    int getKeyType(){return mKeyType;}
    void setGeometryPos(double x,double y);     //控件的起始坐标
    void getGeometryPos(double &x,double &y);
    void getRealEventPos(double &x,double &y);  //获取真实的有效坐标点,及定义的中心点坐标
    void setGeometryHWRatio(double w,double h);   //设置控件整体相对大小的比例
    void getGeometruHWRatio(double &w,double &h);
    virtual void setOperateHWRatio(double w,double h);    //设置实际控制区域的相对大小比例
    virtual void getOperateHW(double &w,double &h);

signals:
    void deleteGameKey(int type,int idx);

protected:
    virtual void getTranslateCenterPos(QPoint pos) = 0;

    void checkPosOutsideScreen(QPoint pos);
    QPoint checkPosOutsideApp(QPoint pos);
    int getRandom(int init,int range);
    void setShowKeyString(QLineEdit* edit,QString str);
    void sendEventToApp(QEvent *event);

    KmreWindow* mMainWindow = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    double geometry_x;    //起始点坐标
    double geometry_y;
    double event_coordinate_x; //中心点坐标
    double event_coordinate_y;
    double mWidthRatio;  //实际宽高的比例
    double mHeightRatio;
    double mRealOperWidthRatio = 0;  //实际控制区域的宽高比例
    double mRealOPerHeightRatio = 0;
    int mKeyType = 1;
    bool m_enableEdit = true;//使能编辑状态（可调整大小位置及绑定key）
    bool m_outsideScreen = false;//是否移出屏幕外状态,当前无效
    bool mIsBeEdit = true;   //控件是否被编辑过，默认是true，即添加就算编辑过
    bool mFontSizeChange = false;

    QPoint m_lastPos = QPoint(0,0);//鼠标按下处坐标
    bool m_mousePressed = false;//鼠标按下状态
    QString mKeyString = "";

};

#endif // JOYSTICKBASEKEY_H
