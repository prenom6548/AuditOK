/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#include <QDebug>
#include "greeterscreen.h"

//#define DEBUG_LOG(...) {\
//   debug_log("greeterScreen", __FILE__, __func__, __LINE__, ##__VA_ARGS__);\
//}

//void debug_log(const char *moduleName, const char *fileName, const char *functionName, int line, const char *fmt, ...)
//{
//    char buf[2048] = {0};
//    int tagLen = 0;
//    va_list para;
//    va_start(para, fmt);
//    memset(buf, 0, sizeof buf);
//    // 使用 snprintf 的返回值确定写入的字符数
//    tagLen = snprintf(buf, sizeof(buf) - 1, "[%s] %s->%s %s line:%-5d", "DEBUG", moduleName, fileName, functionName, line);
//    if (tagLen < 0) {
//        // 处理 snprintf 出错的情况
//        va_end(para);
//        return;
//    }

//    // 确保不会越界
//    if (tagLen < static_cast<int>(sizeof(buf) - 1)) {
//        vsnprintf(buf + tagLen, sizeof(buf) - 1 - tagLen, fmt, para);
//    }
//    va_end(para);

//    qDebug()<<buf;
//}

double GreeterScreen::getModeRefresh(const XRRModeInfo *mode_info)
{
    double rate;
    double vTotal = mode_info->vTotal;

    if (mode_info->modeFlags & RR_DoubleScan) {
        /* doublescan doubles the number of lines */
        vTotal *= 2;
    }

    if (mode_info->modeFlags & RR_Interlace) {
        /* interlace splits the frame into two fields */
        /* the field rate is what is typically reported by monitors */
        vTotal /= 2;
    }

    if (mode_info->hTotal && vTotal)
        rate = ((double) mode_info->dotClock /
                ((double) mode_info->hTotal * (double) vTotal));
    else
        rate = 0;
    return rate;
}

GreeterScreen::GreeterScreen(QObject *parent): QObject(parent),
    m_pDpy(nullptr),
    m_pScreenRes(nullptr)
{

}

GreeterScreen::~GreeterScreen()
{
    if (m_pDpy != nullptr) {
        XCloseDisplay(m_pDpy);
    }

    if (m_pScreenRes != nullptr) {
        XRRFreeScreenResources(m_pScreenRes);
    }
//    DEBUG_LOG("bye~");
}

QString GreeterScreen::setGreeterConfigParam(int *error)
{
    int ret;
    QString msg = "";
    OutputsConfig outputConfig;

    if (false ==initXparam()) {
        *error = 0;
        return "Display open fail";
    }

    for (int koutput = 0; koutput < m_pScreenRes->noutput; koutput++) {
        XRROutputInfo * outputInfo = XRRGetOutputInfo (m_pDpy, m_pScreenRes, m_pScreenRes->outputs[koutput]);
        if (RR_Connected != outputInfo->connection) {
            XRRFreeOutputInfo(outputInfo);
            continue;
        }
        for (int kcrtc = 0; kcrtc < m_pScreenRes->ncrtc; kcrtc++) {
            XRRCrtcInfo *crtcInfo = XRRGetCrtcInfo(m_pDpy, m_pScreenRes, m_pScreenRes->crtcs[kcrtc]);
            //enabled output need find the crtc same as output->crtc,
            //disable output' output->crtc are 0,we must connect to the crtc who crtcInfo->noutput is 0
            if ((outputInfo->crtc != m_pScreenRes->crtcs[kcrtc] && outputInfo->crtc!= 0)
                    || ((crtcInfo->noutput || outputInfo->crtc) && outputInfo->crtc == 0)) {
                XRRFreeCrtcInfo(crtcInfo);
                continue;
            }

            UsdOuputProperty *output = new UsdOuputProperty();
            XRRModeInfo *mode = getPrefferedMode(outputInfo);
            double scale = getScale(mode);
            output->setProperty("enable", crtcInfo->noutput);
            output->setProperty("scale", scale);
            output->setProperty("x", outputConfig.m_x);
            output->setProperty("y", 0);
            output->setProperty("modeId",  (int)mode->id);
            output->setProperty("crtc",  (int)m_pScreenRes->crtcs[kcrtc]);
            output->setProperty("outputId", (int)m_pScreenRes->outputs[koutput]);
            output->setProperty("name", QString::fromLatin1(outputInfo->name));
            output->setProperty("width", (int)(mode->width / scale));
            output->setProperty("height", (int)(mode->height / scale));
            outputConfig.m_x += mode->width/scale;
            outputConfig.m_screenWidth += mode->width;
            outputConfig.m_screenHeight = mode->height > outputConfig.m_screenHeight? mode->height:outputConfig.m_screenHeight;

            outputConfig.m_outputList.append(output);

            XRRFreeCrtcInfo(crtcInfo);
            break;
        }
        XRRFreeOutputInfo(outputInfo);
    }

    //debug
    debugOutputInfo(outputConfig);
    if (outputConfig.m_outputList.count() == 0) {
        goto END;
    }
    //debugend;
    setScreenWithSize(outputConfig.m_screenWidth, outputConfig.m_screenHeight);

    for (int koutput = 0; koutput < outputConfig.m_outputList.count(); koutput++) {
        RROutput outputId = outputConfig.m_outputList[koutput]->property("outputId").toInt();
        RRCrtc crtc =  outputConfig.m_outputList[koutput]->property("crtc").toInt();
        QString name =  outputConfig.m_outputList[koutput]->property("name").toString();
        double scale = outputConfig.m_outputList[koutput]->property("scale").toDouble();
        int x =  outputConfig.m_outputList[koutput]->property("x").toInt();
        int y =  outputConfig.m_outputList[koutput]->property("y").toInt();
        int modeId =  outputConfig.m_outputList[koutput]->property("modeId").toInt();
        if (!koutput) {
            XRRSetOutputPrimary(m_pDpy, m_rootWindow, outputId);
        }
        if (scale != 1) {
            XRRCrtcTransformAttributes  *attr;
//            DEBUG_LOG(".crtc:%d", crtc);
            ret = XRRSetCrtcConfig (m_pDpy, m_pScreenRes, crtc, CurrentTime,
                                    0, 0, None, RR_Rotate_0, NULL, 0);
            if (XRRGetCrtcTransform (m_pDpy, crtc, &attr) && attr) {
                XTransform  transform;
                char *filter;
                XFixed *xfixed;
                for (int x = 0; x < 3; x++) {
                    for (int x1 = 0; x1 < 3; x1++) {
                        transform.matrix[x][x1] = 0;
                    }
                }

                transform.matrix[0][0] = XDoubleToFixed(1.0)/scale;
                transform.matrix[1][1] = XDoubleToFixed(1.0)/scale;
                transform.matrix[2][2] = XDoubleToFixed(1.0);

                if (scale == 1.0) {
                    filter = "nearest";
                } else {
                    filter = "bilinear";
                }
//                DEBUG_LOG("ready set transform %d,%d,%d",
//                          transform.matrix[0][0],transform.matrix[1][1],
//                        transform.matrix[2][2]);
                XRRSetCrtcTransform(m_pDpy, crtc, &transform, filter, xfixed, 0);
                XFree (attr);
//                DEBUG_LOG("transform set over");
            }
        }
//        DEBUG_LOG("ready set %s at %dx%d use mode %d",name.toLatin1().data(),x,y,modeId);
        ret = XRRSetCrtcConfig (m_pDpy, m_pScreenRes, crtc, CurrentTime,
                                x,
                                y,
                                modeId, RR_Rotate_0,
                                &outputId, 1);
//        if (ret == RRSetConfigSuccess) {
//            DEBUG_LOG("%s set success",name.toLatin1().data());
//        } else {
//            DEBUG_LOG("%s set faile",name.toLatin1().data());
//        }
    }
END:
    *error = 1;
    return msg;
}

int GreeterScreen::initXparam()
{
    m_pDpy = XOpenDisplay (m_pDisplayName);
    if (m_pDpy == NULL) {
//        DEBUG_LOG("XOpenDisplay fail...");
        return false;
    }

    m_screen = DefaultScreen(m_pDpy);
    if (m_screen >= ScreenCount (m_pDpy)) {
//        DEBUG_LOG("Invalid screen number %d (display has %d)",m_screen, ScreenCount(m_pDpy));
        return false;
    }

    m_rootWindow = RootWindow(m_pDpy, m_screen);

    m_pScreenRes = XRRGetScreenResources(m_pDpy, m_rootWindow);
    if (NULL == m_pScreenRes) {
//        DEBUG_LOG("could not get screen resources",m_screen, ScreenCount(m_pDpy));
        return false;
    }

    if (m_pScreenRes->noutput == 0) {
//        DEBUG_LOG("noutput is 0!!");
        return false;
    }

//    DEBUG_LOG("initXparam success");
    return true;
}

void GreeterScreen::debugOutputInfo(OutputsConfig outputConfig)
{
//    DEBUG_LOG("screensize:%dx%d,%d",outputConfig.m_screenWidth, outputConfig.m_screenHeight,outputConfig.m_outputList.count());
//    for (int koutput = 0; koutput < outputConfig.m_outputList.count(); koutput++) {
//         DEBUG_LOG("screensize:%dx%d",outputConfig.m_screenWidth, outputConfig.m_screenHeight);
//       DEBUG_LOG("%s(%d):use mode:%d(%dx%d),at %dx%d scale:%f crtc:%d",
//               outputConfig.m_outputList[koutput]->property("name").toString().toLatin1().data(),
//               outputConfig.m_outputList[koutput]->property("outputId").toInt(),
//               outputConfig.m_outputList[koutput]->property("modeId").toInt(),
//               outputConfig.m_outputList[koutput]->property("width").toInt(),
//               outputConfig.m_outputList[koutput]->property("height").toInt(),
//               outputConfig.m_outputList[koutput]->property("x").toInt(),
//               outputConfig.m_outputList[koutput]->property("y").toInt(),
//               outputConfig.m_outputList[koutput]->property("scale").toFloat(),
//               outputConfig.m_outputList[koutput]->property("crtc").toInt());
//    }
}

double GreeterScreen::getScale(XRRModeInfo *mode)
{
    double scale = 1.0f;

    if (mode->width <= 1920 && mode->height <= 1080) {
        scale = 1.0;
    } else if (mode->width > 1920 && mode->height > 1080 &&
               mode->width <= 2560 && mode->height <= 1500) {
        scale = 1.5;
    } else if (mode->width > 2560 && mode->height >1440){
        scale = 2.0;
    }

    return scale;
}

void GreeterScreen::setScreenWithSize(int width, int height)
{
    int screenInt = DefaultScreen (m_pDpy);
    if (width != DisplayWidth(m_pDpy, screenInt) ||
            height != DisplayHeight(m_pDpy, screenInt))
    {
        int fb_width_mm;
        int fb_height_mm;
        double dpi = (25.4 * DisplayHeight(m_pDpy, screenInt)) / DisplayHeightMM(m_pDpy,screenInt);
        fb_width_mm = (25.4 * width) /dpi;
        fb_height_mm = (25.4 * height) /dpi;
        //dpi = Dot Per Inch，一英寸是2.54cm即25.4mm
//        USD_LOG(LOG_DEBUG,"set screen size ok!!!%dx%d!@%f",width, height,dpi);
//        DebugPrintf(QString::asprintf("set screen size ok!!!%dx%d!@%f",width, height,dpi));

//        DEBUG_LOG("ready set screenSize %d x %d. %dmm x %dmm dpi:%f.",width, height,
//                  fb_width_mm, fb_height_mm, dpi);
        XRRSetScreenSize(m_pDpy, m_rootWindow, width, height,
                        fb_width_mm, fb_height_mm);
//        DEBUG_LOG("screenSize set ok!..");
    }
}

RRMode GreeterScreen::getModeId(XRROutputInfo *outputInfo, UsdOuputProperty *kscreenOutputParam)
{
    double rate;
    double vTotal;

    RRMode ret = 0;

    for (int m = 0; m < m_pScreenRes->nmode; m++) {
        XRRModeInfo *mode = &m_pScreenRes->modes[m];
        vTotal = mode->vTotal;
        if (mode->modeFlags & RR_DoubleScan) {
            /* doublescan doubles the number of lines */
            vTotal *= 2;
        }

        if (mode->modeFlags & RR_Interlace) {
            /* interlace splits the frame into two fields */
            /* the field rate is what is typically reported by monitors */
            vTotal /= 2;
        }
        rate = mode->dotClock / ((double) mode->hTotal * (double) vTotal);
        if (mode->width == kscreenOutputParam->property("width").toInt()
                && mode->height == kscreenOutputParam->property("height").toInt()) {
            double kscreenRate = kscreenOutputParam->property("rate").toDouble();
            if (qAbs(kscreenRate - rate) < 0.02) {
                for (int k = 0; k< outputInfo->nmode; k++) {
                    if (outputInfo->modes[k] == mode->id) {
                        return mode->id;
                    }
                }
            } else {
//                SYS_LOG(LOG_DEBUG,"%dx%d %f!=%f",mode->width,mode->height,rate,kscreenRate);
            }
        }
    }

    return ret;
}

XRRModeInfo *GreeterScreen::getPrefferedMode(XRROutputInfo *outputInfo)
{
    int maxValue = 0;
    double refresh = 0;
    double temprefresh = 0;
    XRRModeInfo *mode = nullptr;
    for (int m = 0; m < m_pScreenRes->nmode; m++) {
        if (outputInfo->npreferred != 0) {
            mode = &m_pScreenRes->modes[m];
            if (mode->id == outputInfo->modes[outputInfo->npreferred-1]) {
                return mode;
            }
        } else {
            for (int kmode = 0; kmode < outputInfo->nmode; kmode++) {
                XRRModeInfo *tempMode = &m_pScreenRes->modes[m];
                if (tempMode->id == outputInfo->modes[kmode]) {
                    if (tempMode->height * tempMode->width > maxValue) {
                        maxValue = tempMode->height * tempMode->width;
                        refresh = getModeRefresh(tempMode);
                        mode = tempMode;
                    } else if (tempMode->height * tempMode->width == maxValue){
                        temprefresh = getModeRefresh(tempMode);
                        if (temprefresh > refresh) {
                            mode = tempMode;
                        }
                    }
                }
            }
        }
    }
    return mode;
}
