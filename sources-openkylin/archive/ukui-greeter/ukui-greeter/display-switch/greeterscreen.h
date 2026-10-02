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
#ifndef GREETERSCREEN_H
#define GREETERSCREEN_H


#include <QObject>
#include "xrandroutput.h"

extern "C"{
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
}



class GreeterScreen :  public QObject
{
    Q_OBJECT

//    Q_PROPERTY(bool isSet READ isSet WRITE setIsSet )
//    Q_PROPERTY(bool isGet READ isGet WRITE setIsGet )

public:
    explicit GreeterScreen(QObject *parent = nullptr) ;
    ~GreeterScreen();

    QString setGreeterConfigParam(int *error);

private:
    int	m_screen;
    char *m_pDisplayName = NULL;
    Window	m_rootWindow;
    Display	*m_pDpy;
    XRRScreenResources  *m_pScreenRes;
    OutputsConfig m_kscreenConfigParam;

private:
    int initXparam();
    void debugOutputInfo(OutputsConfig outputConfig);
    double getScale(XRRModeInfo *mode);
    double getModeRefresh(const XRRModeInfo *mode_info);
    void setScreenWithSize(int width, int height);
    RRMode getModeId(XRROutputInfo	*outputInfo, UsdOuputProperty *kscreenOutputParam);
    XRRModeInfo *getPrefferedMode(XRROutputInfo *outputInfo);

};

#endif // GREETERSCREEN_H
