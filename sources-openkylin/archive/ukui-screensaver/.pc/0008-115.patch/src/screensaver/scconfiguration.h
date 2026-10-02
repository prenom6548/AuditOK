/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef SCCONFIGURATION_H
#define SCCONFIGURATION_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QObject>
#include <QString>
#include <QVariant>

class QGSettings;
class BackendDbusHelper;

class SCConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit SCConfiguration(QObject *parent = nullptr);
    static SCConfiguration *instance(QObject *parent = nullptr);

    QString getDefaultBackground(); //获取屏保默认背景
    int getTimeType();              //获取显示时间格式
    QString getDateType();          //获取日期格式
    int getFontSize();              //获取当前字体大小
    double getPtToPx();
    bool getAutoSwitch();        //获取是否自动切换
    bool getCShowRestTime();     //自定义是否显示休息时间
    bool getUShowRestTime();     // UKUI是否显示休息时间
    bool getIsCustom();          //屏保是否自定义
    int getCycleTime();          //背景图片切换间隔时间
    QString getBackgroundPath(); //背景图片来源
    bool getTextIsCenter();      //文字是否居中
    QString getMyText();         //获取文字
    bool getMessageShowEnable(); //获取是否显示消息
    int getMessageNumber();      //获取消息数量
    int getBlurNumber();
    QString getcurStyle();
    QString getVideoPath();
    QString getVideoFormat();
    int getVideoSize();
    int getVideoWidth();
    int getVideoHeight();

public:
public Q_SLOTS:
    void onConfigurationChanged(QString key, QVariant value);

Q_SIGNALS:
    void cycleTimeChanged(int cycleTime);
    bool autoSwitchChanged(bool changed);
    QString backgroundPathChanged(QString path);
    QString backgroundChanged(QString path);
    QString myTextChanged(QString text);
    bool showCRestTimeChanged(bool isShow);
    bool showURestTimeChanged(bool isShow);
    bool textIsCenterChanged(bool isCenter);
    int messageNumberChanged(int number);
    bool messageShowEnableChanged(bool enabled);
    int timeTypeChanged(int time);
    QString dateTypeChanged(QString type);
    int blurChanged(int num);
    QString styleChanged(QString type);
    int fontSizeChanged(int fontSize);

private:
    BackendDbusHelper *m_helperBackendDbus = nullptr;

    static SCConfiguration *instance_;
};

#endif // CONFIGURATION_H
