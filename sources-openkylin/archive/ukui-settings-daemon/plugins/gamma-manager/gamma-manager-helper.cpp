/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include "gamma-manager-helper.h"
#include <cmath>
static int
find_last_non_clamped(unsigned short  array[], int size) {
    int i;
    for (i = size - 1; i > 0; i--) {
        if (array[i] < 0xffff)
        return i;
    }
    return 0;
}

GmHelper::GmHelper(QObject *parent)
{
    qRegisterMetaType<OutputStruct>("OutputStruct");
    qRegisterMetaType<OutputStructList>("OutputStructList");
    qDBusRegisterMetaType<OutputStruct>();
    qDBusRegisterMetaType<OutputStructList>();
    m_temperature = COLOR_TEMPERATURE_DEFAULT;
    m_xrandrInterface  = new QDBusInterface(DBUS_XRANDR_NAME,
                                            DBUS_XRANDR_PATH,
                                            DBUS_XRANDR_INTERFACE,
                                            QDBusConnection::sessionBus(),
                                            this);

    QDBusInterface powerInterface("org.ukui.powermanagement",
                                  "/",
                                  "org.ukui.powermanagement.interface",
                                  QDBusConnection::systemBus());
    QDBusReply<bool> replay = powerInterface.call("CanSetBrightness");

    if(replay.isValid()) {
        m_powerManagerSetable = replay.value();
    }

    m_wlComItr = new QDBusInterface("com.kylin.Wlcom",
                            "/com/kylin/Wlcom/Output",
                            "com.kylin.Wlcom.Output",
                            QDBusConnection::sessionBus(), this);

    if (!m_xrandrInterface->isValid()) {
        USD_LOG(LOG_ERR, "xrandr interface init faile");
    }

    if (UsdBaseClass::isWlcom()) {
        m_wlComInitTimer = new QTimer(this);
        m_wlComInitTimer->setSingleShot(false);
        connect(m_wlComInitTimer, SIGNAL(timeout()), this, SLOT(doWlcomInitTimeOut()));
    }
}

GmHelper::~GmHelper()
{
    if (m_pScreenRes) {
        XRRFreeScreenResources(m_pScreenRes);
    }
}

bool GmHelper::getLonAndLatByJson(QString url, QByteArray bytes, QSizeF& psize)
{
    if(!url.compare(IP_API_ADDRESS,Qt::CaseInsensitive)) {
        return getLonAndLatMozilla(bytes, psize);
    } else if(!url.compare(IP_API_ADDRESS_BACKUP,Qt::CaseInsensitive)) {
        return getLonAndLatIPAPI(bytes, psize);
    } else {
        return getSunriseAndSunsetWithWeatherUrl(bytes, psize);
    }

    return 0;
}

bool GmHelper::getLonAndLatIPAPI(QByteArray jsonBytes, QSizeF& psize)
{
    QStringList keysList;
    QJsonParseError parseError;
    QJsonDocument rootDoc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        USD_LOG(LOG_ERR, "%s parse error",jsonBytes.data());
        return 0;
    }

    QJsonObject rootObj = rootDoc.object();
    keysList = rootObj.keys();
    if (!keysList.contains("lon") || !keysList.contains("lat")) {
        USD_LOG(LOG_ERR, "%s parse error can't find log or lat",jsonBytes.data());
        return 0;
    }

    QJsonValue lonValue = rootObj["lon"];
    QJsonValue latValue = rootObj["lat"];

    psize.setHeight(lonValue.toDouble());
    psize.setWidth(latValue.toDouble());
    return 1;
}

bool GmHelper::getSunriseAndSunsetWithWeatherUrl(QByteArray jsonBytes, QSizeF &psize)
{
/**
{"KylinWeather":{"weather":"tmp=4,cond_code=100,cond_txt=晴,fl=4,hum=26,pcpn=0,pres=1026,vis=14611,wind_deg=331,wind_dir=西北风,
    wind_sc=1,wind_spd=1.5,cloud=999990,sunrise=07:22:56,sunset=17:24:59,id=101030100,location=天津;"}}
*/
    WeatherInfo info;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON解析错误：" << error.errorString();
        return false;
    }

    QJsonObject rootObj = doc.object();
    QJsonObject kylinObj = rootObj["KylinWeather"].toObject();
    QString weatherStr = kylinObj["weather"].toString();

    // 解析字符串中的键值对
    QStringList pairs = weatherStr.split(",");
    for (const QString &pair : pairs) {
        QStringList keyValue = pair.split("=");
        if (keyValue.size() != 2)
            continue;

        QString key = keyValue[0].trimmed();
        QString value = keyValue[1].trimmed();

        if (key == "tmp") {
            info.tmp = value.toInt();
        } else if (key == "cond_code") {
            info.cond_code = value.toInt();
        } else if (key == "cond_txt") {
            info.cond_txt = value;
        } else if (key == "fl") {
            info.fl = value.toInt();
        } else if (key == "hum") {
            info.hum = value.toInt();
        } else if (key == "pcpn") {
            info.pcpn = value.toFloat();
        } else if (key == "pres") {
            info.pres = value.toInt();
        } else if (key == "vis") {
            info.vis = value.toInt();
        } else if (key == "wind_deg") {
            info.wind_deg = value.toInt();
        } else if (key == "wind_dir") {
            info.wind_dir = value;
        } else if (key == "wind_sc") {
            info.wind_sc = value.toInt();
        } else if (key == "wind_spd") {
            info.wind_spd = value.toFloat();
        } else if (key == "cloud") {
            info.cloud = value.toInt();
        } else if (key == "sunrise") {
            info.sunrise = value;
        } else if (key == "sunset") {
            info.sunset = value;
        } else if (key == "id") {
            info.id = value;
        } else if (key == "location") {
            info.location = value;
        }
    }

    bool sunriseIsOk, sunsetIsOk;

    psize.setHeight(timeToDouble(info.sunrise, sunriseIsOk));
    psize.setWidth(timeToDouble(info.sunset, sunsetIsOk));
    return (sunriseIsOk && sunsetIsOk);
}

void GmHelper::sendPrimaryChanged(int brightness, QString signalName)
{
    static int lastBrightness = -1;
    if (DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_END != signalName && (brightness == lastBrightness || m_sendPrimaryBrightnessSignal == false)) {
//        USD_LOG(LOG_DEBUG,"skip send signal.");
        return;
    }

    lastBrightness = brightness;

    QDBusMessage notifySignal =
            QDBusMessage::createSignal(DBUS_GC_BRIGHTNESS_PATH, DBUS_GC_BRIGHTNESS_INTERFACE, signalName);
    notifySignal.setArguments({QVariant::fromValue((uint)brightness)});
    QDBusConnection::sessionBus().send(notifySignal);
}

void GmHelper::updateOutputInfo(QString outputName, int state, int colorTemp)
{
    if(UsdBaseClass::isWlcom()) {
        updateWlcomOutputInfo(outputName, state, colorTemp);
    } else {
        updateXOutputInfo(outputName, state, colorTemp);
    }
    m_temperature = colorTemp;
}

void GmHelper::doWlcomInitTimeOut()
{
    updateWlcomOutputInfo("", 0, m_temperature);
    Q_EMIT outputInitSuccess();
}


void GmHelper::sendEnableChanged(int state)
{
    static int oldState = -1;
    if (oldState == state) {
        return;
    }

    oldState = state;
    QDBusMessage notifySignal =
            QDBusMessage::createSignal(DBUS_GC_BRIGHTNESS_PATH, DBUS_GC_BRIGHTNESS_INTERFACE, DBUS_GC_BRIGHTNESS_SIGNAL_ENABLECHANGED);

    notifySignal.setArguments({QVariant::fromValue((bool)state)});
    QDBusConnection::sessionBus().send(notifySignal);
}

QList<OutputInfo> GmHelper::initX11Output()
{

    if (m_pScreenRes == nullptr) {
         m_pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    }

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        OutputInfo info;
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, m_pScreenRes->outputs[k]);
        info.name = QString::fromLatin1(outputInfo->name);

        if (m_powerManagerSetable) {
            info.targetBrightness = 100;
        } else {
            info.targetBrightness = 100;//是否应该在这里初始化？
        }

        info.rtBrightness = 100;
        info.lastBrightness = 100;
        info.connectState = outputInfo->connection;
        info.lastTemp =  COLOR_TEMPERATURE_DEFAULT;
        info.rtTemp =  COLOR_TEMPERATURE_DEFAULT;
        info.targetTemp = COLOR_TEMPERATURE_DEFAULT;
        XRRFreeOutputInfo(outputInfo);
        m_outputList.append(info);
    }
    return m_outputList;
}

OutputGammaInfoList GmHelper::getX11AllOutputsInfo()
{
    OutputGammaInfoList outputGamaInfoList;

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        OutputGammaInfo outputGamaInfo;
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        if (outputInfo->connection != RR_Connected) {
            goto FREEOUPUT;
            continue;
        }
        for (int var = 0; var < m_outputList.count(); var++) {
            if (!m_outputList[var].name.compare(QString::fromLatin1(outputInfo->name),Qt::CaseInsensitive)) {
                outputGamaInfo.OutputName = m_outputList[var].name;
                outputGamaInfo.Gamma = getTemperatureWithRgb(m_colorRGB.R, m_colorRGB.G, m_colorRGB.B);
                outputGamaInfo.Temperature = m_temperature;
                outputGamaInfo.Brignthess = m_outputList[var].rtBrightness;
                outputGamaInfoList.append(std::move(outputGamaInfo));
            }
        }
FREEOUPUT:
        XRRFreeOutputInfo(outputInfo);
    }
    return outputGamaInfoList;
}

bool GmHelper::setX11PrimaryBrightness(int brightness)
{
    int ret = false;
    int primaryScreenId = 0;
    primaryScreenId = XRRGetOutputPrimary(QX11Info::display(), RootWindow(QX11Info::display(), DefaultScreen(QX11Info::display())));

    if (m_outputList.count() == 0) {
        return false;
    }

    if (m_pScreenRes == nullptr) {
         m_pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    }

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        QString outputname = QString::fromLatin1(outputInfo->name);

        for (int var = 0; var < m_outputList.count(); var++) {
            if (!m_outputList[var].name.compare(outputname,Qt::CaseInsensitive)) {
                m_outputList[var].connectState = RR_Disconnected;
                if (outputId == primaryScreenId) {
                    m_outputList[var].isPrimary = true;
                    m_outputList[var].targetBrightness = brightness;
                    ret = true;
                } else {
                    m_outputList[var].isPrimary = false;
                }
                break;
            }
        }
        XRRFreeOutputInfo(outputInfo);
    }

    return ret;
}

bool GmHelper::setX11GammaWithTemp(const uint rtTemp)
{
    int size = 0;
    int ret = true;
    float gammaRed = 0.f;
    float gammaGreen = 0.f;
    float gammaBlue = 0.f;
    int primaryScreenId = 0;
    float brightness = 1.f;

    XRRCrtcGamma *pCrtcGamma;
    m_temperature = rtTemp;

    if (rtTemp<COLOR_MIN_TEMPERATURE) {
        return true;
    }

    QScreen *primaryScreen = QApplication::primaryScreen();
    getRgbWithTemperature(rtTemp, m_colorRGB);

    if (m_pScreenRes==nullptr) {
         m_pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    }

    primaryScreenId = XRRGetOutputPrimary(QX11Info::display(), RootWindow(QX11Info::display(), DefaultScreen(QX11Info::display())));
    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        QString outputname = QString::fromLatin1(outputInfo->name);
        if (outputInfo->connection != RR_Connected) {
            XRRFreeOutputInfo(outputInfo);
            for (int var = 0; var < m_outputList.count(); var++) {
                if (!m_outputList[var].name.compare(outputname,Qt::CaseInsensitive)) {
                    m_outputList[var].connectState = RR_Disconnected;
                }
            }
            continue;
        }

        for (int var = 0; var < m_outputList.count(); var++) {
            if (!m_outputList[var].name.compare(outputname,Qt::CaseInsensitive)) {
                m_outputList[var].rtTemp = rtTemp;
                m_outputList[var].lastTemp = rtTemp;
                m_outputList[var].connectState = RR_Connected;
                brightness = (m_outputList[var].rtBrightness / 100.0f) * MAX_GAMMA_BRIGHTNESS + MIN_GAMMA_BRIGHTNESS;
//                USD_LOG(LOG_DEBUG,"find:%s set brigntness:%0.4f", m_outputList[var].name.toLatin1().data(), m_outputList[var].rtBrightness);
                m_outputList[var].isPrimary = false;
                if (primaryScreenId == outputId) {
                    m_outputList[var].isPrimary = true;
                    sendPrimaryChanged((int)m_outputList[var].rtBrightness, DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED);
                }

                break;
            }
        }
        if (!outputInfo->crtc) {
            ret = true;
            USD_LOG(LOG_ERR,"crtc size is 0.\n");
            goto FREEOUTPUT;
        }

        size = XRRGetCrtcGammaSize(QX11Info::display(), outputInfo->crtc);
        if (!size) {
            ret = false;
            USD_LOG(LOG_ERR,"Gamma size is 0.\n");
            goto FREEOUTPUT;
        }

        /*
         * The gamma-correction lookup table managed through XRR[GS]etCrtcGamma
         * is 2^n in size, where 'n' is the number of significant bits in
         * the X Color.  Because an X Color is 16 bits, size cannot be larger
         * than 2^16.
         */
        if (size > 65536) {
            ret = false;
            USD_LOG(LOG_ERR,"Gamma correction table is impossibly large.\n");
            goto FREEOUTPUT;
        }

        pCrtcGamma = XRRAllocGamma(size);
        if (!pCrtcGamma) {
             USD_LOG(LOG_ERR,"Gamma allocation failed.\n");
            continue;
        }
        m_colorRGB.R == m_colorRGB.R ? m_colorRGB.R : 1.0;
        m_colorRGB.G == m_colorRGB.G ? m_colorRGB.G : 1.0;
        m_colorRGB.B == m_colorRGB.B ? m_colorRGB.B : 1.0;

        gammaRed = 1 / m_colorRGB.R;
        gammaGreen = 1 / m_colorRGB.G;
        gammaBlue = 1 / m_colorRGB.B;

#if 1
        for (int i = 0; i < size; i++) {\
            uint value = (i * 0xffff) / (size - 1);
            pCrtcGamma->red[i] = value * m_colorRGB.R * brightness;
            pCrtcGamma->green[i]= value * m_colorRGB.G * brightness;
            pCrtcGamma->blue[i] = value * m_colorRGB.B * brightness;
        }
#else

        for (int i = 0; i < size; i++) {
            if (gammaRed == 1.0 && m_brightness == 1.0) {
                pCrtcGamma->red[i] = (double)i / (double)(size - 1) * 65535.0;
            }  else {
                pCrtcGamma->red[i] = qMin(qPow((double)i/(double)(size - 1),
                                              gammaRed) * m_brightness,
                                          1.0) * 65535.0;
            }

            if (gammaGreen == 1.0 && m_brightness == 1.0) {
                pCrtcGamma->green[i] = (double)i / (double)(size - 1) * 65535.0;
            } else {
                pCrtcGamma->green[i] = qMin(qPow((double)i/(double)(size - 1),
                                                gammaGreen) * m_brightness,
                                            1.0) * 65535.0;
            }

            if (gammaBlue == 1.0 && m_brightness == 1.0) {
                pCrtcGamma->blue[i] = (double)i / (double)(size - 1) * 65535.0;
            } else {
                pCrtcGamma->blue[i] = qMin(qPow((double)i/(double)(size - 1),
                                               gammaBlue) * m_brightness,
                                           1.0) * 65535.0;
            }
        }
#endif
        XRRSetCrtcGamma(QX11Info::display(), outputInfo->crtc, pCrtcGamma);
        XSync(QX11Info::display(), NULL);
//        USD_LOG(LOG_DEBUG," %s color temp(%d) set ok r:%.4f,g:%.4f,b:%.4f,(bright):%0.4f", outputInfo->name, m_temperature,m_colorRGB.R,m_colorRGB.G,m_colorRGB.B,brightness);
        XRRFreeGamma(pCrtcGamma);
FREEOUTPUT:
         XRRFreeOutputInfo(outputInfo);
    }
    return ret;
}

void GmHelper::updateXOutputInfo(QString outputName, int state, int colorTemp)
{
    for (int var = 0; var < m_outputList.count(); var++) {
        if (!m_outputList[var].name.compare(outputName,Qt::CaseInsensitive) || outputName == "all") {
            if (state || (outputName=="" && m_outputList[var].connectState == RR_Connected)) {//just set gamma when output enable.
                m_outputList[var].targetTemp = colorTemp;
            }
        }
    }

    USD_LOG(LOG_DEBUG,"output:%s state:%d,output", outputName.toLatin1().data(), state);
}

int GmHelper::getX11PrimaryBrightness()
{
    int ret = -1;
    int primaryScreenId = 0;
    primaryScreenId = XRRGetOutputPrimary(QX11Info::display(), RootWindow(QX11Info::display(), DefaultScreen(QX11Info::display())));

    if (m_outputList.count() == 0) {
        return -1;
    }

    if (m_pScreenRes==nullptr) {
         m_pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    }

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo *outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        QString outputname = QString::fromLatin1(outputInfo->name);

        for (int var = 0; var < m_outputList.count(); var++) {
            if (!m_outputList[var].name.compare(outputname,Qt::CaseInsensitive)) {
                m_outputList[var].connectState = outputInfo->connection;
                if (outputId == primaryScreenId) {
                    m_outputList[var].isPrimary = true;
                    if (outputInfo->connection == RR_Connected) {
                        ret = m_outputList[var].targetBrightness;
                    }
                } else {
                    m_outputList[var].isPrimary = false;
                }
                break;
            }
        }
        XRRFreeOutputInfo(outputInfo);
    }

    return ret;
}

QList<OutputInfo> GmHelper::initWlComOutput()
{
    if (UsdBaseClass::isWlcom()) {
        m_wlComInitTimer->start(50);
    }
    return m_outputList;
}

OutputGammaInfoList GmHelper::getWlComAllOutputsInfo()
{
    OutputGammaInfoList list;
    return list;
}

bool GmHelper::setWlcomPrimaryBrightness(int brightness)
{
    USD_LOG(LOG_DEBUG,"brightness:%d", brightness);
    for (int k = 0; k < m_outputList.count(); k++) {
        if (m_outputList[k].isPrimary) {
            m_outputList[k].targetBrightness = brightness;
        }
        USD_LOG(LOG_DEBUG,"%s:%f,primary:%d", m_outputList[k].name.toLatin1().data(),
                m_outputList[k].targetBrightness,m_outputList[k].isPrimary);
    }

    return true;
}

bool GmHelper::setWlcomTemp(const uint rtTemp)
{
    QMutexLocker locker(&m_outputListMutex);
    for (int k = 0; k < m_outputList.count(); k++) {
        if (m_outputList[k].connectState == RR_Connected) {
            USD_LOG(LOG_DEBUG,"set %s %d.%f",m_outputList[k].name.toLatin1().data(), m_outputList[k].rtTemp, m_outputList[k].rtBrightness);
            m_outputList[k].rtTemp = rtTemp;
            m_outputList[k].lastTemp = rtTemp;
            if (m_wlComItr->isValid()) {
                m_wlComItr->asyncCall("SetColortemp", m_outputList[k].name, uint(rtTemp));
                m_wlComItr->asyncCall("SetBrightness", m_outputList[k].name, uint(normalizeBrightness(m_outputList[k].rtBrightness)));
            }
        }
    }

    return true;
}

void GmHelper::updateWlcomOutputInfo(QString outputName, int state, int colorTemp)
{
    /**/
    Q_UNUSED(outputName);
    Q_UNUSED(colorTemp);
    Q_UNUSED(state);
    m_wlComInitTimer->stop();

    if (!m_wlComItr->isValid()) {
        return;
    }

    QString primaryScreenName = getPrimaryScreenNameFormXrandr();
    OutputStructList outstatusList;

    QDBusMessage msg = m_wlComItr->call("ListAllOutputs");
    if (msg.type() != QDBusMessage::ReplyMessage) {
        USD_LOG(LOG_ERR,"ListAllOutputs call failed:", qPrintable(msg.errorMessage()));
        return;
    }

    QDBusArgument wlcomReplay =  msg.arguments().at(0).value<QDBusArgument>();

    wlcomReplay >> outstatusList;
    QMutexLocker locker(&m_outputListMutex);
    m_outputList.clear();
    int count = 0;
    for (const OutputStruct &curOutput : outstatusList) {
        OutputInfo newOutput;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(curOutput.outputInfo.toUtf8());
        QJsonObject jsonObject = jsonDoc.object();

        int brightness = getGlobalBrightness(curOutput.outputName);
        newOutput.name = curOutput.outputName;
        newOutput.isPrimary = newOutput.name == primaryScreenName;

        if (UsdBaseClass::upmSupportAdjustBrightness() && count == 0) {
            newOutput.targetBrightness = 100;
        } else {
            newOutput.targetBrightness = (brightness >= 0 && brightness <= 100) ? brightness : 100;
        }

        newOutput.rtBrightness = denormalizeBrightness(jsonObject["brightness"].toInt());
        newOutput.lastBrightness = jsonObject["brightness"].toInt();
        newOutput.rtTemp = jsonObject["color_temp"].toInt();
        newOutput.targetTemp = jsonObject["color_temp"].toInt();
        newOutput.lastTemp = jsonObject["color_temp"].toInt();
        newOutput.connectState = RR_Connected;

        USD_LOG(LOG_DEBUG,"get %s brightness %f temp:%d primary:%d ", newOutput.name.toLatin1().data(),
                newOutput.rtBrightness,
                newOutput.lastTemp,newOutput.isPrimary);

        m_outputList.append(newOutput);
        count++;
    }

}

int GmHelper::getWlcomPrimaryBrightness()
{
    for (int k = 0; k < m_outputList.count(); k++) {
        if (UsdBaseClass::upmSupportAdjustBrightness() && k == 0) {
            USD_LOG(LOG_DEBUG,"ignore notebook screen.");
            continue;
        }
        if (m_outputList[k].isPrimary) {
            return m_outputList[k].rtBrightness;
        }
    }

    return 100;
}

void GmHelper::doScreensParamChanged(QString param)
{

    QStringList outputList;
    QString primaryOutput;
    getScreenInfo(param, outputList, primaryOutput);

    Q_FOREACH(const QString str , outputList) {
        USD_LOG_SHOW_PARAMS(str.toLatin1().data());
    }

    USD_LOG_SHOW_PARAMS(primaryOutput.toLatin1().data());

    Q_FOREACH(const QString str, outputList) {
        bool hadUpdate = false;
        for(int k = 0; k < m_outputList.count(); k++) {
            if (outputList.contains(m_outputList[k].name)) {//已链接的显示器都在outputlist内，但是否enable不一定，是否需要做判断？
                m_outputList[k].connectState = RR_Connected;
                if (primaryOutput == m_outputList[k].name) {
                    m_outputList[k].isPrimary = true;
                }

                if (str == m_outputList[k].name) {
                    hadUpdate = true;
                }

            } else {
                m_outputList[k].connectState = RR_Disconnected;
                m_outputList[k].isPrimary = false;
                m_outputList[k].targetBrightness = 100;
                m_outputList[k].rtBrightness = 100;
                m_outputList[k].lastBrightness = 100;
                m_outputList[k].lastTemp =  m_outputList[k].rtTemp =  m_outputList[k].targetTemp = COLOR_TEMPERATURE_DEFAULT;
            }
        }

        if (false == hadUpdate) {//insert a new output
            OutputInfo newOutput;
            newOutput.name = str;
            newOutput.isPrimary = false;
            newOutput.targetBrightness = 100;
            newOutput.rtBrightness = 100;
            newOutput.lastBrightness = 100;
            newOutput.lastTemp =  newOutput.rtTemp =  newOutput.targetTemp = COLOR_TEMPERATURE_DEFAULT;
            newOutput.connectState = RR_Connected;
            if (str == primaryOutput) {
                newOutput.isPrimary = true;
            }
            m_outputList.append(newOutput);
            USD_LOG(LOG_DEBUG, "add %s in list,primary:%d", newOutput.name.toLatin1().data(), newOutput.isPrimary);
        }

    }
}

void GmHelper::getScreenInfo(const QString &param, QStringList &outputList, QString &primaryOutput)
{
    QJsonDocument doc = QJsonDocument::fromJson(param.toUtf8());
    if (doc.isNull()) {
        USD_LOG(LOG_ERR, "Invalid JSON data");
        return;
    }

    const QVariantList screens = doc.toVariant().toList();
    for (const auto &screen : screens) {
        const QVariantMap screenMap = screen.toMap();
        const QString name = screenMap["metadata"].toMap()["name"].toString();

        if (screenMap["primary"].toBool()) {
            primaryOutput = name;
            USD_LOG(LOG_DEBUG, "Primary: %s", qPrintable(name));
        }

        outputList << name;
        USD_LOG(LOG_DEBUG, "Output: %s", qPrintable(name));
    }
}
void GmHelper::syncBrightnessToConfig()
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                          "/globalconfig", \
                          SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                          QDBusConnection::systemBus());

    for (int var = 0; var < m_outputList.count(); ++var) {
        QList<QVariant> arguments;
        arguments << QVariant(QString::fromLatin1(MODULE_NAME)) <<
                     QVariant(m_outputList[var].name) <<
                     QVariant::fromValue(QDBusVariant(m_outputList[var].targetBrightness));

        globalManager.asyncCallWithArgumentList("setGlobalConf", arguments);
    }
}

void GmHelper::syncBrightnessFromConfig()
{
//    QMap<QString, QVariant> brightnessMap =
//            QGSettings(USD_COLOR_SCHEMA).get(COLOR_KEY_BRIGHTNESS_MAP).value<QMap<QString, QVariant>>();

//    for (int var = 0; var < m_outputList.count(); ++var) {
//        if (brightnessMap.keys().contains(m_outputList[var].name)) {
//            m_outputList[var].targetBrightness = brightnessMap[m_outputList[var].name].toDouble();
//        }
    //    }
}

int GmHelper::normalizeBrightness(int userValue)
{
    //=ROUND((MIN(MAX(A2,30),100)-30)/0.7,0) 与 denormalizeBrightness 反算结果一致
    userValue = std::round(c_minBrightness + userValue * c_convertFactor);
    return std::min(userValue, c_maxBrightness);
}

int GmHelper::denormalizeBrightness(int wlcomValue)
{
    //=ROUND((MIN(MAX(B2,0),100))*0.7+30,0) 与 normalizeBrightness 反算结果一致
     wlcomValue = std::max(wlcomValue,c_minBrightness);
     wlcomValue = std::min(wlcomValue,c_maxBrightness);

     return std::round((wlcomValue - c_minBrightness)
                       / c_convertFactor);
}

QString GmHelper::getPrimaryScreenNameFormXrandr()
{
    const int MIN_VALID_LENGTH = 10;
    QString primaryOutput;

    if (m_xrandrInterface == nullptr) {
        USD_LOG(LOG_WARNING, "XRandR D-Bus interface not initialized");
        return primaryOutput;
    }

    QDBusReply<QString> reply = m_xrandrInterface->call(
        DBUS_XRANDR_GET_SCREEN_PARAM,
        qAppName()
        );

    if (!reply.isValid()) {
        USD_LOG(LOG_DEBUG, "Invalid D-Bus reply received");
        return primaryOutput;
    }

    QString screenInfo = reply.value();
    if (screenInfo.toUtf8().length() < MIN_VALID_LENGTH) {
        USD_LOG(LOG_ERR, "Invalid screen info length: %d", screenInfo.length());
        return primaryOutput;
    }

    QStringList outputList;
    getScreenInfo(screenInfo, outputList, primaryOutput);

    return primaryOutput;
}

double GmHelper::timeToDouble(QString timeStr, bool &isOk)
{
    QStringList parts = timeStr.split(":");
    bool ok1, ok2, ok3;
    int hours = parts[0].toInt(&ok1);
    int minutes = parts[1].toInt(&ok2);
    int seconds = parts[2].toInt(&ok3);
    isOk = false;

    if (!ok1 || !ok2 || !ok3 ||
            hours < 0 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59) {
        return false;
    }
    isOk = true;
    double totalSeconds = hours + minutes / 60.0 + seconds / 3600;
    return totalSeconds;
}

int GmHelper::getGlobalBrightness(QString outputName)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/globalconfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());

    QDBusReply<QVariant> reply = globalManager.call("getGlobalConf", MODULE_NAME,  outputName);
    if (!reply.isValid()) {
        return -1;
    }
    return reply.value().toInt();
}

bool GmHelper::getLonAndLatMozilla(QByteArray jsonBytes, QSizeF &psize)
{
    QStringList keysList;
    QJsonParseError parseError;
    QJsonDocument rootDoc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        USD_LOG(LOG_ERR, "%s parse error", jsonBytes.data());
        return 0;
    }

    QJsonObject rootObj = rootDoc.object();
    keysList = rootObj.keys();
    if (!keysList.contains("location")) {
        USD_LOG(LOG_ERR, "%s parse error can't find log or lat", jsonBytes.data());
        return 0;
    }

    QJsonValue valueArray = rootObj["location"];
    QJsonValue latValue = valueArray["lat"];
    QJsonValue lonValue = valueArray["lng"];
    if (!lonValue.toDouble() || !latValue.toDouble()) {
        USD_LOG(LOG_ERR, "%s parse error can't find log or lng(%f,%f)",jsonBytes.data(), lonValue.toDouble(), latValue.toDouble());
        return 0;
    }

    psize.setHeight(lonValue.toDouble());
    psize.setWidth(latValue.toDouble());
    return 1;
}

bool GmHelper::getSunriseSunset(QDateTime &rtDate, QSizeF &location, QSizeF &SunriseSunset)
{
//    g_autoptr(GDateTime) dt_zero = g_date_time_new_utc (1900, 1, 1, 0, 0, 0);
//    GTimeSpan ts = g_date_time_difference (dt, dt_zero);
    QDateTime  dtZero = QDateTime::fromString("1900-01-01 00:00.000", Qt::ISODate);//1900, 1, 1, 0, 0, 0
    quint64 ts = dtZero.msecsTo(rtDate);

    USD_CHECK_RETURN(location.width() <=90.f && location.width() >= -90.f, false);
    USD_CHECK_RETURN(location.height() <=180.f && location.height() >= -180.f, false);
    double pos_lat = location.width();
    double pos_long = location.height();
    double tz_offset = rtDate.offsetFromUtc() / 60 / 60;
    double date_as_number = ts / 1000 / 24 / 60 / 60 + 2;  // B7
    double time_past_local_midnight = 0;  // E2, unused in this calculation
    double julian_day = date_as_number + 2415018.5 +
            time_past_local_midnight - tz_offset / 24;
    double julian_century = (julian_day - 2451545) / 36525;
    double geom_mean_long_sun =  fmod (280.46646 + julian_century *
                                      (36000.76983 + julian_century * 0.0003032), 360); // I2
    double geom_mean_anom_sun = 357.52911 + julian_century *
            (35999.05029 - 0.0001537 * julian_century);  // J2
    double eccent_earth_orbit = 0.016708634 - julian_century *
            (0.000042037 + 0.0000001267 * julian_century); // K2
    double sun_eq_of_ctr = sin (deg2rad (geom_mean_anom_sun)) *
            (1.914602 - julian_century * (0.004817 + 0.000014 * julian_century)) +
            sin (deg2rad (2 * geom_mean_anom_sun)) * (0.019993 - 0.000101 * julian_century) +
            sin (deg2rad (3 * geom_mean_anom_sun)) * 0.000289; // L2
    double sun_true_long = geom_mean_long_sun + sun_eq_of_ctr; // M2
    double sun_app_long = sun_true_long - 0.00569 - 0.00478 *
            sin (deg2rad (125.04 - 1934.136 * julian_century)); // P2
    double mean_obliq_ecliptic = 23 +  (26 +  ((21.448 - julian_century *
                                                (46.815 + julian_century * (0.00059 - julian_century * 0.001813)))) / 60) / 60; // Q2
    double obliq_corr = mean_obliq_ecliptic + 0.00256 *
            cos (deg2rad (125.04 - 1934.136 * julian_century)); // R2
    double sun_declin = rad2deg (asin (sin (deg2rad (obliq_corr)) *
                                       sin (deg2rad (sun_app_long)))); // T2
    double var_y = tan (deg2rad (obliq_corr/2)) * tan (deg2rad (obliq_corr / 2)); // U2
    double eq_of_time = 4 * rad2deg (var_y * sin (2 * deg2rad (geom_mean_long_sun)) -
                                     2 * eccent_earth_orbit * sin (deg2rad (geom_mean_anom_sun)) +
                                     4 * eccent_earth_orbit * var_y *
                                     sin (deg2rad (geom_mean_anom_sun)) *
                                     cos (2 * deg2rad (geom_mean_long_sun)) -
                                     0.5 * var_y * var_y * sin (4 * deg2rad (geom_mean_long_sun)) -
                                     1.25 * eccent_earth_orbit * eccent_earth_orbit *
                                     sin (2 * deg2rad (geom_mean_anom_sun))); // V2
    double ha_sunrise = rad2deg (acos (cos (deg2rad (90.833)) / (cos (deg2rad (pos_lat)) *
                                                                 cos (deg2rad (sun_declin))) - tan (deg2rad (pos_lat)) *
                                       tan (deg2rad (sun_declin)))); // W2
    double solar_noon =  (720 - 4 * pos_long - eq_of_time + tz_offset * 60) / 1440; // X2
    double sunrise_time = solar_noon - ha_sunrise * 4 / 1440; //  Y2
    double sunset_time = solar_noon + ha_sunrise * 4 / 1440; // Z2

    /* convert to hours */
    SunriseSunset.setHeight(sunrise_time * 24);
    SunriseSunset.setWidth(sunset_time * 24);
    return true;
}

bool GmHelper::getRtSunriseSunset(QSizeF &location, QSizeF &SunriseSunset)
{
    QDateTime rtDate = QDateTime::currentDateTime();
    //
    USD_LOG_SHOW_PARAM1(rtDate.offsetFromUtc());
    getSunriseSunset(rtDate, location, SunriseSunset);
    USD_LOG_SHOW_PARAM2F(SunriseSunset.width(), SunriseSunset.height());

    return true;
}

bool GmHelper::getRgbWithTemperature(double temp, ColorRGB &result)
{
    bool ret = true;
    const ColorRGB *pColorRgb = blackbodyDataD65plankian;
    uint tempQuot = 0;
    uint tempRem = 0;
    if (temp < 1000.0 || temp > 10000.0) {
        return false;
    }
    tempQuot = (uint) temp / 100;
    tempRem = (uint) temp % 100;
    tempQuot -= 10;

    if (!tempRem) {
        result = pColorRgb[tempQuot];
        return ret;
    }

    getRgbInterpolate(pColorRgb[tempQuot], pColorRgb[tempQuot + 1], tempRem / 100.0f, result);
    return ret;
}

void GmHelper::getRgbInterpolate(const ColorRGB &p1, const ColorRGB &p2, double index, ColorRGB &result)
{
    result.R =  (1.0 - index) * p1.R + index * p2.R;
    result.G =  (1.0 - index) * p1.G + index * p2.G;
    result.B =  (1.0 - index) * p1.B + index * p2.B;
}

uint GmHelper::getTempInterpolate(const double svalue, const double bvalue, double value)
{

    USD_CHECK_RETURN((svalue > 0 && svalue <=1.f), 0);
    USD_CHECK_RETURN((bvalue > 0 && bvalue <=1.f), 0);
    USD_CHECK_RETURN((value > 0 && value <=1.f), 0);

    return ((value - svalue) / (bvalue - svalue)) * 100;
}


uint GmHelper::getTemperatureWithRgb(const double red, const double green, const double blue)
{
    USD_CHECK_RETURN((red >= 0 && red <=1.f), 0);
    USD_CHECK_RETURN((green >= 0 && green <=1.f), 0);
    USD_CHECK_RETURN((blue >= 0 && blue <=1.f), 0);
    uint space = 0;
    uint temp;

    const ColorRGB *pColorRgb = blackbodyDataD65plankian;
    uint circleLength = sizeof(blackbodyDataD65plankian) / sizeof(ColorRGB);
    USD_LOG_SHOW_PARAM1(circleLength);
    for (int k = 0; k < circleLength; k++) {
        if (red <= pColorRgb[k].R && green <= pColorRgb[k].G && blue <= pColorRgb[k].B) {
            USD_LOG(LOG_DEBUG,"%.02f(%.02f),%.02f(%.02f),%.02f(%.02f)",red, pColorRgb[k].R, green, pColorRgb[k].G, blue, pColorRgb[k].B);
            space = k;
            break;
        }
    }

    if (space == 0) {
        return COLOR_MIN_TEMPERATURE;
    }

    temp = 1000 + (space - 1) * 100 + getTempInterpolate(pColorRgb[space-1].B, pColorRgb[space].B, blue);
    USD_LOG(LOG_DEBUG,"%.2f--%.2f--%.2f=====>%d",red, green, blue, temp);
    return temp;
}

void GmHelper::recheckPrimary()
{
    bool findPrimary = false;
    int outputCount = 0;
    int primaryScreenId = 0;
    primaryScreenId = XRRGetOutputPrimary(QX11Info::display(), RootWindow(QX11Info::display(), DefaultScreen(QX11Info::display())));

    if (m_outputList.count() == 0) {
        USD_LOG(LOG_DEBUG,"!!! start.. !!");
        return;
    }

    if (m_pScreenRes==nullptr) {
         m_pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    }

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        if (outputInfo->connection == RR_Connected) {
            outputCount++;
        }
        XRRFreeOutputInfo(outputInfo);
    }

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        QString outputname = QString::fromLatin1(outputInfo->name);

        for (int var = 0; var < m_outputList.count(); var++) {
            if (!m_outputList[var].name.compare(outputname,Qt::CaseInsensitive)) {
                m_outputList[var].connectState = RR_Disconnected;
                if ((outputId == primaryScreenId || outputCount == 1) && outputInfo->connection == RR_Connected) {//一块屏幕也需要发信号
                    m_outputList[var].isPrimary = true;
                    sendPrimaryChanged(m_outputList[var].targetBrightness, DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_END);
                    sendEnableChanged(true);
                    findPrimary = true;
                } else {
                    m_outputList[var].isPrimary = false;
                }
            }
        }

        XRRFreeOutputInfo(outputInfo);
        if (findPrimary) {
            break;
        }
    }

    if (!findPrimary) {
        sendEnableChanged(false);
    }
}

bool GmHelper::setGammaWithTemp(const uint rtTemp)
{
    m_temperature = rtTemp;
    if (UsdBaseClass::isWlcom()) {
        return setWlcomTemp(rtTemp);
    } else {
        return setX11GammaWithTemp(rtTemp);
    }
}

void GmHelper::freeScreenResource()
{
    if (m_pScreenRes != nullptr) {
        XRRFreeScreenResources(m_pScreenRes);
        m_pScreenRes = nullptr;
    }
}

bool GmHelper::setPrimaryBrightness(int brightness)
{
    if (UsdBaseClass::isWlcom()) {
        setWlcomPrimaryBrightness(brightness);
    } else {
        setX11PrimaryBrightness(brightness);
    }

    syncBrightnessToConfig();

    return true;
}

int GmHelper::getPrimaryBrightness()
{
   int ret = 100;
   if (UsdBaseClass::isWlcom()) {
       ret = getWlcomPrimaryBrightness();
   } else {
       ret = getX11PrimaryBrightness();
   }

   return ret;
}

void GmHelper::enablePrimarySignal(int state)
{
    m_sendPrimaryBrightnessSignal = state;
}

void GmHelper::setAllOutputsBrightness(const uint brightness)
{
    for (int var = 0; var < m_outputList.count(); ++var) {
        if (UsdBaseClass::upmSupportAdjustBrightness() && var == 0) {
            USD_LOG(LOG_DEBUG,"ignore notebook screen.");
            continue;
        }
        m_outputList[var].rtBrightness = brightness;
    }
    syncBrightnessToConfig();
}

/**
 * @brief GmHelper::setBrightness brightness的线程由上级开启。
 * @param outputName
 * @param brightness
 */
void GmHelper::setBrightness(const QString outputName, const double brightness)
{//我要如何判断是否是笔记本内屏呢？？？？
    for (int var = 0; var < m_outputList.count(); ++var) {
        if (UsdBaseClass::upmSupportAdjustBrightness() && var == 0) {
            USD_LOG(LOG_DEBUG,"ignore notebook screen.");
            continue;
        }

        if (!m_outputList[var].name.compare(outputName,Qt::CaseInsensitive) || !outputName.compare("all",Qt::CaseInsensitive) ) {
            m_outputList[var].targetBrightness = brightness;
        }
    }
    syncBrightnessToConfig();
}

QList<OutputInfo> GmHelper::initOutput()
{
    if (UsdBaseClass::isWlcom()) {
         initWlComOutput();
    } else {
         initX11Output();
    }
    Q_EMIT outputInitSuccess();
    return m_outputList;
}

OutputGammaInfoList GmHelper::getAllOutputsInfo()
{
    OutputGammaInfoList outputGamaInfoList;
    OutputGammaInfo outputGamaInfo;

    for (int k = 0; k < m_outputList.count(); k++) {
        if (m_outputList[k].connectState == RR_Connected) {
            outputGamaInfo.OutputName = m_outputList[k].name;
            outputGamaInfo.Gamma = getTemperatureWithRgb(m_colorRGB.R, m_colorRGB.G, m_colorRGB.B);
            outputGamaInfo.Temperature = m_temperature;
            outputGamaInfo.Brignthess = m_outputList[k].rtBrightness;
            outputGamaInfoList.append(std::move(outputGamaInfo));
        }
    }
    return outputGamaInfoList;
}

QList<OutputInfo>& GmHelper::getOutputInfo()
{
    return m_outputList;
}
//rgb 会修正亮度。所以，需要记录亮度值，然后根据亮度值回调出正常的色温再带入重新进行计算。
#if 0
OutputGammaInfoList GmHelper::getAllOutputGammaInfo()
{
    double i1, v1, i2, v2;
    int size, middle, lastBest, lastRed, lastGreen, lastBlue;
    float red = 0;
    float green = 0;
    float blue = 0;

    float brightness = 0;
    unsigned short  *bestArray;
    XRRCrtcGamma *pCrtcGamma;
    XRRScreenResources  *pScreenRes;
    OutputGammaInfoList outputGamaInfoList;
//    pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        ColorRGB colorRgb;
        OutputGammaInfo outputGamaInfo;
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        if (outputInfo->connection != RR_Connected) {
            XRRFreeOutputInfo(outputInfo);
            continue;
        }

        size = XRRGetCrtcGammaSize(QX11Info::display(), outputInfo->crtc);
        if (!size) {
            USD_LOG(LOG_ERR, "size get error");
            goto FREEOUTPUT;;
        }
        pCrtcGamma = XRRGetCrtcGamma(QX11Info::display(), outputInfo->crtc);
        if (!pCrtcGamma) {
            USD_LOG(LOG_ERR, "CrtcGamma get error");
            goto FREEOUTPUT;;
        }

        /*
         * The gamma-correction lookup table managed through XRR[GS]etCrtcGamma
         * is 2^n in size, where 'n' is the number of significant bits in
         * the X Color.  Because an X Color is 16 bits, size cannot be larger
         * than 2^16.
         */
        if (size > 65536) {
            USD_LOG(LOG_ERR, "size get error");
            goto FREEOUTPUT;;
        }
        lastRed = find_last_non_clamped(pCrtcGamma->red, size);
        lastBlue = find_last_non_clamped(pCrtcGamma->blue, size);
        lastGreen = find_last_non_clamped(pCrtcGamma->green, size);
        USD_LOG_SHOW_PARAM1(lastRed);
        USD_LOG_SHOW_PARAM1(lastBlue);
        USD_LOG_SHOW_PARAM1(lastGreen);
        bestArray = pCrtcGamma->red;
        lastBest = lastRed;

        if (lastGreen > lastBest) {
            lastBest = lastGreen;
            bestArray = pCrtcGamma->green;
        }

        if (lastBlue > lastBest) {
            lastBest = lastBlue;
            bestArray = pCrtcGamma->blue;
        }

        if (lastBest == 0) {
            lastBest == 1;
        }

        middle = lastBest / 2;
        i1 = (double)(middle + 1) / size;
        v1 = (double)(bestArray[middle]) / 65535;
        i2 = (double)(lastBest + 1) / size;
        v2 = (double)(bestArray[lastBest]) / 65535;
        USD_LOG_SHOW_PARAM1(lastBest);
        USD_LOG_SHOW_PARAM1(v2);
        if (v2 < 0.0001) {
            USD_LOG(LOG_ERR, "size get error");
            goto FREEOUTPUT;
        } else if ((lastBest + 1) == size) {
            brightness = v2;
        } else {
            brightness = exp((log(v2)*log(i1) - log(v1)*log(i2))/log(i1/i2));
        }

        brightness = 1;
        red = log((double)(pCrtcGamma->red[lastRed / 2]) / brightness
                / 65535) / log((double)((lastRed / 2) + 1) / size);
        green =  log((double)(pCrtcGamma->green[lastGreen / 2]) / brightness
                / 65535) / log((double)((lastGreen / 2) + 1) / size);
        blue = log((double)(pCrtcGamma->blue[lastBlue / 2]) / brightness
                / 65535) / log((double)((lastBlue / 2) + 1) / size);

        colorRgb.R = red == 0 ? 0 : 1/red;
        colorRgb.G = green == 0 ? 0 : 1/green;
        colorRgb.B = blue == 0 ? 0 : 1/blue;
        USD_LOG(LOG_DEBUG,"[%s] r:%.2f,g:%.2f,b:%.2f brightness:%.2f", outputInfo->name, colorRgb.R, colorRgb.G, colorRgb.B, brightness);
        outputGamaInfo.OutputName = QString::fromLatin1(outputInfo->name);
        outputGamaInfo.Gamma = getTemperatureWithRgb(colorRgb.R, colorRgb.G, colorRgb.B);
        outputGamaInfoList.append(std::move(outputGamaInfo));

FREEOUTPUT:
        XRRFreeGamma(pCrtcGamma);
        XRRFreeOutputInfo(outputInfo);
    }
    return outputGamaInfoList;
}
#endif

double GmHelper::deg2rad(double degrees)
{
     return (M_PI * degrees) / 180.f;
}

double GmHelper::rad2deg(double radians)
{
    return radians * (180.f / M_PI);
}

QString GmHelper::getVendor(QString outputTypeName)
{
    return UsdBaseClass::getOutputVender(outputTypeName);
}



