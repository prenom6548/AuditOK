/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#include "windowmanager.h"
#include "displayconnection.h"
#include "display_control_api.h"
#include "DisplayControlDecoder.h"
#include "display_control.h"
#include "displaybackend.h"
#include "app_control_manager.h"
#include "utils.h"
#include "preferences.h"
#include "dbusclient.h"
#include "kmreenv.h"
#include "screensharing.h"
#include "sessionsettings.h"

#include <QSize>
#include <QString>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <libgen.h>
#include <syslog.h>
#include <xf86drm.h>

extern "C" {
extern int authenticate_magic(drm_magic_t magic);
}
extern int initOpenGLESRenderForEmu(int width,int height);
extern void deinitOpenGLESRenderForEmu();

static void postVirtualDisplay(const char* name, uint32_t id, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t orientation)
{
    DisplayBackend::getInstance()->addDirtyDisplay(name, id, bo_name, width, height, stride, bpp, orientation);
}

static int postAllVirtualDisplaysDone()
{
    DisplayBackend::getInstance()->updateDisplays();
    return 0;
}

static uint32_t getDisplayWidth()
{
    return SessionSettings::getInstance().getVirtualDisplaySize().width();
}

static uint32_t getDisplayHeight()
{
    return SessionSettings::getInstance().getVirtualDisplaySize().height();
}

static uint32_t getDisplayFPS()
{
    return 60;
}

static uint32_t getDisplayXDpi()
{
    return 100;
}

static uint32_t getDisplayYDpi()
{
    return 100;
}

static int32_t postBoForScreenRecord(uint32_t at_name, uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t ver)
{
    return ScreenSharing::getInstance()->grabNextBo(at_name, name, width, height, stride, ver);
}

static void postCaptureScreenColorBuffer(uint32_t name)
{
    ScreenSharing::getInstance()->addBo(name);
}

static int32_t updateSharingScreenDisplay(uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t swap)
{
    return ScreenSharing::getInstance()->updateSharingScreenDisplay(bo_name, width, height, stride, bpp, swap);
}

static int32_t authenticateMagic(uint32_t magic)
{
    return authenticate_magic(magic);
}

//-------------------------------------------------------------
DisplayConnection::DisplayConnection()
{
    initDisplayControl();
    if (SessionSettings::getInstance().getDisplayType() == DisplayType::TYPE_EMUGL) {
        initEmulator();
    }
}

DisplayConnection::~DisplayConnection()
{
    if (SessionSettings::getInstance().getDisplayType() == DisplayType::TYPE_EMUGL) {
        disconnectEmuSocket();
        deinitOpenGLESRenderForEmu();
    }
    android_stopDisplayControl();
}

int DisplayConnection::initDisplayControl()
{
    kmre::DbusClient::getInstance()->Prepare(kmre::utils::getUserName(), getuid());
    syslog(LOG_DEBUG, "[%s] Start display control server.", __func__);

    if (android_initDisplayControl() != 0) {
        syslog(LOG_ERR, "[%s] Failed to initialize display control server.", __func__);
    }
    else {
        QString addr = KmreEnv::GetContainerSocketPath() + "/display-control";
        if (android_display_control_set_path(addr.toStdString().c_str()) != 0) {
            syslog(LOG_ERR, "[%s] Failed to set bind path for display control server.", __func__);
        }
        else {
            if (android_startDisplayControl() != 0) {
                syslog(LOG_ERR, "[%s] Failed to start display control server.", __func__);
            }
            else {
                mDecoder = DisplayControlDecoder::getInstance();
                if(mDecoder) {
                    //mDecoder->fbPost = FBPost;
                    //mDecoder->postLayer = postLayer;
                    //mDecoder->postAllLayersDone = postAllLayersDone;
                    mDecoder->getDisplayWidth = getDisplayWidth;
                    mDecoder->getDisplayHeight = getDisplayHeight;
                    mDecoder->getDisplayFPS = getDisplayFPS;
                    mDecoder->getDisplayXDpi = getDisplayXDpi;
                    mDecoder->getDisplayYDpi = getDisplayYDpi;
                    mDecoder->postVirtualDisplay = postVirtualDisplay;
                    mDecoder->postAllVirtualDisplaysDone = postAllVirtualDisplaysDone;
                    mDecoder->postBoForScreenRecord = postBoForScreenRecord;
                    mDecoder->postCaptureScreenColorBuffer = postCaptureScreenColorBuffer;
                    mDecoder->authenticateMagic = authenticateMagic;
                    mDecoder->updateSharingScreenDisplay = updateSharingScreenDisplay;
                }

                char serverPath[256] = {0};
                android_display_control_server_path(serverPath, sizeof(serverPath));
                chmod(serverPath, 0777);
                chmod(dirname(serverPath), 0777);
                return 0;
            }
        }
    }

    return -1;
}

int DisplayConnection::initEmulator()
{
    QSize displaySize = SessionSettings::getInstance().getVirtualDisplaySize();
    return initOpenGLESRenderForEmu(displaySize.width(), displaySize.height());
}

bool DisplayConnection::connectEmuSocket()
{
    if (mEmuglConnectFd >= 0) {// socket already connected.
        return true;
    }

    mEmuglSocketPath = KmreEnv::GetContainerSocketPath() + "/qemu_pipe";
    
    //create unix socket
    mEmuglConnectFd = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (mEmuglConnectFd < 0) {
        syslog(LOG_ERR, "[%s] Can not create communication socket!", __func__);
        return false;
    }

    struct sockaddr_un srv_addr;
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, mEmuglSocketPath.toStdString().c_str(), mEmuglSocketPath.length() + 1);
    //connect server
    if (::connect(mEmuglConnectFd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
        syslog(LOG_ERR, "[%s] Can not connect to the server!", __func__);
        close(mEmuglConnectFd);
        mEmuglConnectFd = -1;
        return false;
    }

    const QString pipe = "pipe:transfer";
    write(mEmuglConnectFd, pipe.toStdString().c_str(), pipe.length() + 1);

    const QString resultStr = "OK";
    char readbuf[128] = {0};
    int len = resultStr.length() + 1;
    int read_len = 0;

    while (read_len < len) {
        ssize_t stat = recv(mEmuglConnectFd, readbuf, len, 0);
        if (stat > 0) {
			read_len += stat;
            continue;
        } 
        if (stat <= 0) {
            syslog(LOG_ERR, "[%s] Recv mEmuglConnectFd failed, may emugl outdated Version!", __func__);
            close(mEmuglConnectFd);
            mEmuglConnectFd = -1;
            return false;
        }
    }

    if (QString(readbuf) != resultStr) {
        syslog(LOG_ERR, "[%s] Something error! readbuf: '%s'", __func__, readbuf);
        close(mEmuglConnectFd);
        mEmuglConnectFd = -1;
        return false;
    }

    unsigned int clientFlags = 0;
    write(mEmuglConnectFd, &clientFlags, sizeof(clientFlags));

    syslog(LOG_INFO, "[%s] Connect emugl socket successfully.", __func__);
    return true;
}

void DisplayConnection::disconnectEmuSocket()
{
    syslog(LOG_INFO, "[%s] Disconnect emugl socket.", __func__);
    if (mEmuglConnectFd >= 0) {// socket already connected.
        close(mEmuglConnectFd);
        mEmuglConnectFd = -1;
    }
}

//opcode sync emugl renderControl_opcodes.h
#define OP_rcTransferAppStatus 18886

bool DisplayConnection::enableDisplayUpdateForEmu(int displayId, bool enable)
{
    if (!connectEmuSocket()) {
        return false;
    }

    struct CmdData {
        int OPcode;
        int totalSize;
        int displayId;
        int disableDraw;
    } cmdData = {
        OP_rcTransferAppStatus,
        16,
        displayId,
        enable ? 0 : 1
    };

    write(mEmuglConnectFd, &cmdData, sizeof(cmdData));

    const QString resultStr = "transferOK";
    char readbuf[128] = {0};

    int len = resultStr.length() + 1;
    int read_len = 0;
    while (read_len < len) {
        ssize_t stat = recv(mEmuglConnectFd, readbuf, len, 0);
        if (stat > 0) {
            read_len += stat;
            continue;
        } 
        if (stat <= 0) {
            syslog(LOG_ERR, "[%s] Recv mEmuglConnectFd failed, may emugl outdated Version!", __func__);
            close(mEmuglConnectFd);
            mEmuglConnectFd = -1;
            return false;
        }
    }

    if (QString(readbuf) != resultStr) {
        syslog(LOG_ERR, "[%s] Something error! readbuf: '%s'", __func__, readbuf);
        close(mEmuglConnectFd);
        mEmuglConnectFd = -1;
        return false;
    }
    
    return true;
}
