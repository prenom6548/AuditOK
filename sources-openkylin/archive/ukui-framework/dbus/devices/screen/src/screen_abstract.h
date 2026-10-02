/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SCREEN_ABSTRACT_H__
#define __SCREEN_ABSTRACT_H__

#include <QObject>
#include <QProcess>
#include <QMetaEnum>
#include <QMetaObject>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include <QStringLiteral>
#include <QtCore/QCoreApplication>

#include "screen_define.h"
#include "qgsettings.h"

struct AbstractResult
{
    QVariant value;
    QString errorMsg;
};

class ScreenAbstract: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ukui.framework.devices.screen")
public:

    explicit ScreenAbstract(QObject *parent = nullptr);
    ~ScreenAbstract();


public:
    /**
     * @brief setScreenBrightness 设置显示器亮度，需要先获取所有显示器的亮度再使用。
     * @param name 显示器名称
     * @param screenBrightness 指定亮度
     * @return
     */
    virtual AbstractResult setScreenBrightness(QString name, uint screenBrightness) = 0;

    /**
     * @brief getScreenBrightness 获取显示器的亮度
     * @param name
     * @return 指定显示器的亮度
     */
    virtual AbstractResult getScreenBrightness(QString name) = 0;

    /**
     * @brief setAllScreenBrightness 分别设置显示器的亮度
     * @param screenMap 显示器亮度与其对应的显示器亮度
     * @return 成功标记
     */
    virtual AbstractResult setScreensBrightness(OutputGammaInfoList outputsInfo) = 0;

    /**
     * @brief setAllScreenSameBrightness 设置所有显示为同一个亮度
     * @param brighenss 全部亮度
     * @return 与参数一致
     */
    virtual AbstractResult setAllScreenSameBrightness(uint brighenss) = 0;

    /**
     * @brief getAllScreenBrightness
     * @return 与参数一致
     */
    virtual OutputGammaInfoList getAllScreenBrightness() = 0;

    /**
     * @brief getPrimaryScreenBrightness
     * @return 与参数一致
     */
    virtual uint getPrimaryScreenBrightness() = 0;
private:
    int brightControlByUPM();
    bool isControlBright();
};

#define PEEK_RESULT_BOOL(a)  AbstractResult ret = a;\
    if (ret.errorMsg.isEmpty()) {   \
         return true;    \
    }   \
    sendErrorReply(QDBusError::ErrorType::Failed, (ret.errorMsg));    \
    return false;

#define PEEK_RESULT_INT(a)  AbstractResult ret = a;\
    if (ret.errorMsg.isEmpty()) {   \
         ret.value.toInt();    \
    }   \
    sendErrorReply(QDBusError::ErrorType::Failed, (ret.errorMsg));    \
    return ret.value.toInt();

#define PEEK_RESULT_QSTRING(a)  AbstractResult ret = a;\
    if (ret.errorMsg.isEmpty()) {   \
         ret.value.toString();    \
    }   \
    sendErrorReply(QDBusError::ErrorType::Failed, (ret.errorMsg));    \
    return ret.value.toString();

#endif
