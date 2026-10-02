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


#ifndef __SCREEN_H__
#define __SCREEN_H__


#include "screen_gamma.h"
#include "screen_power.h"
#include "gsettings_manager.h"
#include "usd-common.h"

class Screen: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", ORG_UKUI_FRAMEWORK_SCREEN_INTERFACE)
public:
    enum eScreenMode {
        firstScreenMode = 0,
        cloneScreenMode,
        extendScreenMode,
        secondScreenMode,
        showKDS,
        ERROR = 0x99};

    Q_ENUM(eScreenMode)

    enum eBrightnessControl {
        Unknown = -1,//获取失败，理论上应该不会获取失败，是system的dbus服务
        Gamma,//upm不控制背光则优先为gamma控制
        PowerManager,//背光由upm控制。
        None};//第三种情况，upm不控制背光并且不支持gamma调节

    Q_ENUM(eBrightnessControl)

    enum eShutcutKey {
        IncreaseBrightness = 5,//获取失败，理论上应该不会获取失败，是system的dbus服务
        DecreaseBrightness};//第三种情况，upm不控制背光并且不支持gamma调节

    Q_ENUM(eShutcutKey)

    explicit Screen(QObject *parent = nullptr);
    ~Screen();

public Q_SLOTS:
    /**
     * @brief SetScreenBrightness 设置显示器亮度，需要先获取所有显示器的亮度再使用。
     * @param name 显示器名称
     * @param screenBrightness 指定亮度
     * @return
     */
    bool SetScreenBrightness(QString name, uint screenBrightness);

    /**
     * @brief GetScreenBrightness 获取显示器的亮度
     * @param name
     * @return 指定显示器的亮度
     */
    uint GetScreenBrightness(QString name);

    /**
     * @brief SetScreensBrightness 分别设置显示器的亮度
     * @param screenMap 显示器亮度与其对应的显示器亮度
     * @return 成功标记
     */
    bool SetScreensBrightness(OutputGammaInfoList outputsInfo);

    /**
     * @brief SetAllScreenSameBrightness 设置所有显示为同一个亮度
     * @param brighenss 全部亮度
     * @return 与参数一致
     */
    bool SetAllScreenSameBrightness(uint brightness);

    /**
     * @brief GetAllScreenBrightness
     * @return 与参数一致
     */
    OutputGammaInfoList GetAllScreenBrightness();

    /**
     * @brief SetPrimaryScreenBrightness 设置主屏亮度
     * @param screenBrightness 指定主屏亮度
     * @return 与参数一致
     */
    bool SetPrimaryScreenBrightness(uint screenBrightness);

    /**
     * @brief GetPrimaryScreenBrightness 获取主屏亮度
     * @return 主屏亮度
     */
    uint GetPrimaryScreenBrightness();

    /**
     * @brief increaseBrightness 增加显示器亮度，效果等同于快捷键,目前仅笔记本可用
     * @return
     */
    bool IncreaseBuiltinScreenBrightness();

    /**
     * @brief decreaseBuiltinBrightness  降低显示器亮度，效果等同于快捷键,目前仅笔记本可用
     * @return
     */
    bool DecreaseBuiltinBrightness();
    /**
     * @brief getScreenMode 获取显示器显示模式
     *  firstScreenMode = 0,第一屏幕
        cloneScreenMode,镜像屏幕
        extendScreenMode,扩展
        secondScreenMode,第二屏
     * @return 显示模式
     */
    QString GetScreenMode();

    /**
     * @brief setScreenMode 设置显示模式
     * @param mode 指定模式
        firstScreenMode = 0,第一屏幕
        cloneScreenMode,镜像屏幕
        extendScreenMode,扩展
        secondScreenMode,第二屏
     * @return 设置标记，0失败，1成功
     */
    bool SetScreenMode(const QString &mode);

    /**
     * @brief setScreensParam 设置显示器参数
     * @param screenParam 显示器参数组合
     * @return 设置状态
     */
    bool SetScreensParam(const QString &screenParam);

    /**
     * @brief getScreensParam 获取显示器状态
     * @return 显示器信息
     */
    QString GetScreensParam();

    /**
     * @brief setEyeCare 设置护眼模式，
     * 0,进入护眼模式
     * 1,退出护眼模式
     * @param state
     * @return 设置标记，false 失败，true 成功
     */
    bool SetEyeCareEnabled(bool state);

    /**
     * @brief getEyeCare 获取护眼模式
     * @return 护眼模式
     */
    bool IsEyeCareEnabled();

    /**
     * @brief setNightMode 设置夜间模式
     * @param state 夜间模式状态
     * 0：关闭夜间模式
     * 1：开启夜间模式
     * @return 设置标记，0失败，1成功
     */
    bool SetNightModeEnable(bool state);

    /**
     * @brief getNightMode 获取是否为夜间模式
     * @return 夜间模式 设置标记，0非夜间模式，1时进入模式
     */
    bool IsNightModeEnable();

    /**
     * @brief SetScreenScale 设置单一显示器情况下的缩放率
     * @param scale 缩放率
     * @return
     */
    bool SetScreenScale(double scale);

    /**
     * @brief GetScreenScale
     * 获取缩放率
     * @return
     */
    double GetScreenScale();

    /**
     * @brief isAutoBrightEnable
     * @return 查询自动背光是否开启
     */
    bool IsAutoBrightnessEnable();

    /**
     * @brief setAutoBright
     * @param enable 设置自动亮度
     */
    void SetAutoBrightBrightnessEnable(bool enable);
private:

    QHash<QString, OnKeyChanged> setColorChangedHandler();
    QHash<QString, OnKeyChanged> setAutoBrightChangedHandler();
    /**
     * @brief brightControlByUPM 获取电源管理是否可以控制亮度
     * @return
     */
    eBrightnessControl brightControlByUPM();

    /**
     * @brief watchDbusServer
     */
    void watchUpmRegisterDbusServer();

    template <typename...Args>
    QDBusMessage callGamma(const QString &method, Args &&...args);

    template <typename...Args>
    QDBusMessage callXrandr(const QString &method, Args &&...args);

    template <typename...Args>
    QDBusMessage callBrighenss(const QString &method, Args &&...args);

    template <typename...Args>
    QDBusMessage callMediaKeys(const QString &method, Args &&...args);

private Q_SLOTS:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void doPrimaryScreenBrightnessChanged(uint brightness);
    void doScreenBrightnessChanged(QString screenName, uint brightness);
private:
    ScreenAbstract *m_pscreen = nullptr;
Q_SIGNALS:
    /**
     * @brief nightModeChanged 夜间模式变化
     */
    void NightModeChanged(bool);

    /**
     * @brief EyeCareChanged 护眼模式变化
     */
    void EyeCareChanged(bool);

    /**
     * @brief primaryScreenBrightnessChanged 主屏信号变化
     */
    void PrimaryScreenBrightnessChanged(uint);

    /**
     * @brief screenBrightnessChanged 亮度变化信号
     */
    void ScreenBrightnessChanged(QString name,uint brightness);

    /**
     * @brief modeChanged 模式改变信号
     * @param mode
     */
    void SceenModeChanged(QString mode);

    /**
     * @brief paramChanged 显示器参数
     * @param param
     */
    void ParamChanged(QString param);

    /**
     * @brief AutoBrightChanged 自动背光变化
     */
    void AutoBrightChanged(bool);
private:
    QString m_screenMode = "";
    QString m_screenParam = "";
    QString m_primaryName = "";

    QMap<QString, int> m_allScreenBright;

    uint8_t m_primaryBright = 0;
    uint8_t m_screenCount = 0;

    int m_brightControlByUPM = -1;

    QMetaEnum m_outputModeEnum;
    QDBusInterface *m_pGlobalManager;
    QDBusInterface *m_pGammaManager;
    GsettingsManager *m_pColorGsettingsManager = nullptr;
    GsettingsManager *m_pAutoGsettingsManager = nullptr;
    QHash<QString, OnKeyChanged> m_colorSettingsChangedHandlers;
    QHash<QString, OnKeyChanged> m_autoBrightSettingsChangedHandlers;
};
#endif

