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

#include "abstract-gamma-manager.h"

AbstractGammaManager::AbstractGammaManager()
{

}

AbstractGammaManager::~AbstractGammaManager()
{
    destroyQobject(m_pColorSettings);
    destroyQobject(m_pQtSettings);
    destroyQobject(m_pGtkSettings);
}

void AbstractGammaManager::init(bool isWayland)
{
    m_pCheckTimer     = new QTimer(this);
    m_pGmLocation    = new GmLocation(this);
    m_pukuiGtkConfig = new UkuiGtkConfig(this);
    m_pColorSettings = new QGSettings(USD_COLOR_SCHEMA);
    m_pQtSettings    = new QGSettings(QT_THEME_SCHEMA);
    m_pGtkSettings   = new QGSettings(GTK_THEME_SCHEMA);

    m_pCheckTimer->setTimerType(Qt::PreciseTimer);
    m_pCheckTimer->setSingleShot(false);
    m_inDark = m_pColorSettings->get(COLOR_KEY_DARK_MODE).toBool();
    m_pclockNotifier = new USD::ClockSkewNotifier(this);

    m_pGmLocation->setGsettings(m_pColorSettings);
    connect(m_pQtSettings, SIGNAL(changed(QString)), this, SLOT(doQtSettingsChanged(QString)), Qt::DirectConnection);
    USD_LOG(LOG_DEBUG,"init over!");

    m_pCheckTimer->start(60000);
    connect(m_pCheckTimer, SIGNAL(timeout()), this, SLOT(doCheckTimeout()), Qt::DirectConnection);
    connect(m_pColorSettings, SIGNAL(changed(QString)), this, SLOT(doColorSettingsChanged(QString)));
    connect(m_pclockNotifier, SIGNAL(clockSkewed(QString)), this, SLOT(doclockSkewedChanged(QString)));
    m_pukuiGtkConfig->connectGsettingSignal();
    checkEyeCareMode(EYE_CARE_MODE);
}

void AbstractGammaManager::doQtSettingsChanged(QString key)
{
    if (key == QT_THEME_KEY) {
        bool isAllDay = m_pColorSettings->get(COLOR_KEY_ALLDAY).toBool();
        bool isEnable = m_pColorSettings->get(COLOR_KEY_ENABLED).toBool();

        if (m_pQtSettings->get(key).toString() != "ukui-dark") {
            if (m_pColorSettings->get(COLOR_KEY_DARK_MODE).toBool()) {
                m_pColorSettings->set(COLOR_KEY_STYLE_NAME_DM, m_pQtSettings->get(QT_THEME_KEY).toString());
                setDarkMode(false);
                USD_LOG(LOG_DEBUG,"exit dark mode..");
                m_pQtSettings->apply();
            }
        }

        QString theme = m_pQtSettings->get(QT_THEME_KEY).toString();
        if (isAllDay && isEnable && theme == "ukui-dark") {
            setDarkMode(true);
            USD_LOG(LOG_DEBUG,"enter dark mode..");
        }
    }
}

void AbstractGammaManager::doColorSettingsChanged(QString key)
{
    USD_LOG(LOG_DEBUG,"change key:%s",key.toLatin1().data());
    checkEyeCareMode(key);
    gammaReckeck(key);
}

void AbstractGammaManager::doclockSkewedChanged(QString value)
{
    gammaReckeck("");
}

void AbstractGammaManager::setDarkMode(bool darkMode)
{
    m_pColorSettings->blockSignals(true);
    m_pColorSettings->set(COLOR_KEY_DARK_MODE,darkMode);
    m_pColorSettings->blockSignals(false);
    m_pColorSettings->apply();
}

void AbstractGammaManager::checkEyeCareMode(QString key)
{
    QStringList keys = {COLOR_KEY_ENABLED,
                       EYE_CARE_MODE,
                       COLOR_KEY_MAIN_SWITCH};

    if (!keys.contains(key)) {
        USD_LOG_SHOW_PARAMS(qPrintable(key));
        return;
    }

    bool isColorEnable = m_pColorSettings->get(COLOR_KEY_ENABLED).toBool();
    bool isEyeCareEnable = m_pColorSettings->get(EYE_CARE_MODE).toBool();
    bool isMainSwitchEnable = m_pColorSettings->get(COLOR_KEY_MAIN_SWITCH).toBool();

    USD_LOG_SHOW_PARAM1(isEyeCareEnable);
    USD_LOG_SHOW_PARAM1(isColorEnable);
    //总开关开启时，其他两个关闭，默认开启护眼
    m_pColorSettings->blockSignals(true);
    if (key == COLOR_KEY_MAIN_SWITCH) {
        if (isMainSwitchEnable &&
                !isEyeCareEnable &&
                !isColorEnable) {
            m_pColorSettings->set(EYE_CARE_MODE, true);
            isEyeCareEnable = true;
        }
    }
    //两个关闭时，关闭总开关
    if (!isEyeCareEnable && !isColorEnable && isMainSwitchEnable) {
        m_pColorSettings->set(COLOR_KEY_MAIN_SWITCH, false);
    }

   if (key == COLOR_KEY_ENABLED && isColorEnable) {
       m_pColorSettings->set(EYE_CARE_MODE,false);
   } else if (key == EYE_CARE_MODE && isEyeCareEnable) {
       m_pColorSettings->set(COLOR_KEY_ENABLED,false);
   }

   m_pColorSettings->apply();
   m_pColorSettings->blockSignals(false);
}

bool AbstractGammaManager::isFracDayBetween(double value, double start, double end)
{
    if (end <= start) {
        end += 24;
    }

    if (value < start && value < end) {
        value += 24;
    }

    return value >= start && value < end;
}

double AbstractGammaManager::getFracTimeFromDt(QTime dt)
{
    return dt.hour() + (double) dt.minute() / 60 + (double) dt.second() / 3600;
}

double AbstractGammaManager::linearInterpolate(double val1, double val2, double factor)
{
    USD_CHECK_RETURN (factor >= 0.f, -1.f);
    USD_CHECK_RETURN (factor <= 1.f, -1.f);
    return ((val1 - val2) * factor) + val2;
}

double AbstractGammaManager::hourMinuteToDouble(int hour, int minute)
{
    double value = (double)minute/60;
    return (double) hour + value;
}

void AbstractGammaManager::getEyeCareDate(int &temperature, int &interpolateStart, double &scheduleFrom, double &scheduleTo)
{
    double autoScheduleFrom,autoScheduleTo,fracDay;

    QTime rtTime = QTime::currentTime();
    fracDay = getFracTimeFromDt(rtTime);

    autoScheduleFrom = m_pColorSettings->get(COLOR_KEY_AUTOMATIC_FROM).toDouble();
    autoScheduleTo = m_pColorSettings->get(COLOR_KEY_AUTOMATIC_TO).toDouble();

    QVariant qVar = m_pColorSettings->get(COLOR_KEY_LAST_COORDINATES);
    QVariantList qVarList =  qVar.value<QVariantList>();

    if(!(qVarList[1].toDouble() > -180 && qVarList[1].toDouble() < 180 &&
            qVarList[0].toDouble() > -90 && qVarList[0].toDouble() < 90)) {
        autoScheduleFrom = m_pColorSettings->get(COLOR_KEY_FROM).toDouble();
        autoScheduleTo = m_pColorSettings->get(COLOR_KEY_TO).toDouble();
        USD_LOG(LOG_DEBUG,"lon lat error use user config");
    }

    /* smear the temperature for a short duration before the set limits
    *
    *  ---|                                         |------ = to->from(EYE_CARE_VALUE_DAY)
    *     |-|                                     |-| = smear down
    *       |---|                             |---| = from->1, 3->to  evening and dawn (EYE_CARE_VALUE_EVENING_DAWN)
    *           |-|                         |-| = smear down
    *             |-------------------------| =  late at night  (EYE_CARE_VALUE_LATE_NIGHT)
    */

    // evening
    if (isFracDayBetween (fracDay,
                           autoScheduleFrom,
                          DEEP_NIGHT_START)) {
        temperature = m_pColorSettings->get(EYE_CARE_VALUE_EVENING_DAWN).toInt();
        scheduleFrom = autoScheduleFrom;
        scheduleTo = DEEP_NIGHT_START;
        interpolateStart = m_pColorSettings->get(EYE_CARE_VALUE_LATE_NIGHT).toInt();
        USD_LOG(LOG_DEBUG,"in evening");
        return;
    }

    //dawn
    if (isFracDayBetween (fracDay,
                          DEEP_NIGHT_END,
                          autoScheduleTo)) {
        temperature =  m_pColorSettings->get(EYE_CARE_VALUE_EVENING_DAWN).toInt();
        scheduleFrom = DEEP_NIGHT_END;
        scheduleTo = autoScheduleTo;
        interpolateStart = m_pColorSettings->get(EYE_CARE_VALUE_DAY).toInt();
        USD_LOG(LOG_DEBUG,"in dawn1");
        return;
    }

    // late at night
    if (isFracDayBetween (fracDay,
                          DEEP_NIGHT_START,
                          DEEP_NIGHT_END)) {
        temperature =  m_pColorSettings->get(EYE_CARE_VALUE_LATE_NIGHT).toInt();
        scheduleFrom = DEEP_NIGHT_START;
        scheduleTo = DEEP_NIGHT_END;
        interpolateStart = m_pColorSettings->get(EYE_CARE_VALUE_EVENING_DAWN).toInt();
        USD_LOG(LOG_DEBUG,"in late at night");
        return;
    }

    // all night
    if (isFracDayBetween (fracDay,
                           autoScheduleTo,
                           autoScheduleFrom)) {
        temperature =  m_pColorSettings->get(EYE_CARE_VALUE_DAY).toInt();
        scheduleFrom = autoScheduleTo;
        scheduleTo = autoScheduleFrom;
        interpolateStart = m_pColorSettings->get(EYE_CARE_VALUE_EVENING_DAWN).toInt();
        USD_LOG(LOG_DEBUG,"in day");
        return;
    }
}

void AbstractGammaManager::doCheckTimeout()
{
    gammaReckeck("");
    USD_LOG(LOG_DEBUG,".....");
}

void AbstractGammaManager::destroyQobject(QObject *object)
{
    if (object != nullptr) {
        delete object;
        object = nullptr;
    }
}

void AbstractGammaManager::gammaReckeck(QString key)
{
    double scheduleFrom = -1.f;
    double scheduleTo = -1.f;
    double smear = USD_NIGHT_LIGHT_POLL_SMEAR; /* hours */
    int interpolateStart = 0;

    int temperature = m_pColorSettings->get(COLOR_KEY_TEMPERATURE).toInt();
    uint tempSmeared = temperature;
    QTime rtTime = QTime::currentTime();
    double fracDay = getFracTimeFromDt(rtTime);

    //跟随日出日落
    if (m_pColorSettings->get(COLOR_KEY_AUTOMATIC).toBool()) {
        scheduleFrom = m_pColorSettings->get(COLOR_KEY_AUTOMATIC_FROM).toDouble();
        scheduleTo = m_pColorSettings->get(COLOR_KEY_AUTOMATIC_TO).toDouble();
        if (scheduleFrom < 0.f || scheduleTo < 0.f) {
            scheduleFrom = m_pColorSettings->get(COLOR_KEY_FROM).toDouble();
            scheduleTo = m_pColorSettings->get(COLOR_KEY_TO).toDouble();
        }
    } else {
        scheduleFrom = m_pColorSettings->get(COLOR_KEY_FROM).toDouble();
        scheduleTo = m_pColorSettings->get(COLOR_KEY_TO).toDouble();
    }

    //自动主题
    if (m_pColorSettings->get(COLOR_KEY_AUTO_THEME).toBool()) {
        if (isFracDayBetween(fracDay, scheduleFrom, scheduleTo)) {
            m_pGtkSettings->set(GTK_THEME_KEY, "ukui-black");
            m_pQtSettings->set(QT_THEME_KEY, "ukui-dark");
        } else {
            m_pGtkSettings->set(GTK_THEME_KEY, "ukui-white");
            m_pQtSettings->set(QT_THEME_KEY, "ukui-light");
        }

        if(key == COLOR_KEY_AUTO_THEME) {
            return;
        }
    }

    if (!m_pColorSettings->get(COLOR_KEY_MAIN_SWITCH).toBool()) {
        setTemperature(COLOR_TEMPERATURE_DEFAULT);
        return;
    }

    if (!m_pColorSettings->get(EYE_CARE_MODE).toBool() && !m_pColorSettings->get(COLOR_KEY_ENABLED).toBool()) {
        setTemperature(COLOR_TEMPERATURE_DEFAULT);
        return;
    }

    //色温使能
    if (m_pColorSettings->get(COLOR_KEY_ENABLED).toBool()) {
        interpolateStart = COLOR_TEMPERATURE_DEFAULT;
    }

    if (m_pColorSettings->get(EYE_CARE_MODE).toBool()) {

        getEyeCareDate(temperature, interpolateStart, scheduleFrom, scheduleTo);
        USD_LOG_SHOW_PARAM2(temperature, interpolateStart);
        USD_LOG_SHOW_PARAM2F(scheduleFrom, scheduleTo);
    }

    //全天生效
    if (m_pColorSettings->get(COLOR_KEY_ALLDAY).toBool() && m_pColorSettings->get(COLOR_KEY_ENABLED).toBool()) {
        setTemperature(temperature);
        return;
    }

    /* lower smearing period to be smaller than the time between start/stop */
    smear = qMin (smear,
                (qMin (qAbs ((scheduleTo - scheduleFrom)),
                     (24 - qAbs ((scheduleTo - scheduleFrom))))));

    USD_LOG(LOG_DEBUG,"fracDay:%.2f, %.2f %.2f", fracDay,scheduleFrom - smear, scheduleTo);
    if (!isFracDayBetween (fracDay,
                           scheduleFrom - smear,
                           scheduleTo)) {
        setTemperature(interpolateStart);
        USD_LOG(LOG_DEBUG,"in smeared...");
        return;
    }

    /* smear the temperature for a short duration before the set limits
    *
    *   |----------------------| = from->to
    * |-|                        = smear down
    *                        |-| = smear up
    *
    * \                        /
    *  \                      /
    *   \--------------------/
    */

    if (smear < 0.01) {
        /* Don't try to smear for extremely short or zero periods */
        tempSmeared = temperature;
        USD_LOG(LOG_DEBUG,"can't smeared...");
    } else if (isFracDayBetween(fracDay,
                                           scheduleFrom - smear,
                                           scheduleFrom)) {
        double factor = 1.f - ((fracDay - (scheduleFrom - smear)) / smear);
        tempSmeared = linearInterpolate (interpolateStart,
                                          temperature, factor);
        USD_LOG(LOG_DEBUG,"interpolateStart:%d temperature:%d factor:%f,frac_day:%f,schedule_from:%f",interpolateStart, temperature, factor,
                fracDay,scheduleFrom);
    } else if (isFracDayBetween (fracDay,
                                           scheduleTo - smear,
                                           scheduleTo)) {
        double factor = (fracDay - (scheduleTo - smear)) / smear;
        tempSmeared = linearInterpolate (interpolateStart,
                                          temperature, factor);
        USD_LOG(LOG_DEBUG,"interpolateStart:%d temperature:%d factor:%f,frac_day:%f,schedule_from:%f",interpolateStart, temperature, factor,
                fracDay,scheduleTo);
    } else {
        tempSmeared = temperature;
    }
    USD_LOG_SHOW_PARAM2(tempSmeared, (interpolateStart-tempSmeared));
    setTemperature(tempSmeared);
}
