/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Ma Chao    machao@kylinos.cn
 * Alan Xie   xiehuijun@kylinos.cn
 * Clom       huangcailong@kylinos.cn
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#include "DisplayControl/display_control_api.h"
#include "DisplayControl/DisplayControlDecoder.h"
#include "display_control.h"

#define SERVER_PATH_SIZE 256


static int32_t FBPost(uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t bpp)
{
    printf("FBPost received\n");
    printf("name: %d\n", name);
    printf("width: %d\n", width);
    printf("height: %d\n", height);
    printf("stride: %d\n", stride);
    printf("bpp: %d\n\n\n", bpp);
    return 0;
}

static void postLayer(const char* name, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t sourceCropLeft, int32_t sourceCropTop, int32_t sourceCropRight, int32_t sourceCropBottom, int32_t displayFrameLeft, int32_t displayFrameTop, int32_t displayFrameRight, int32_t displayFrameBottom)
{
    printf("postLayer received\n");
    printf("name: %s\n", name);
    printf("bo_name: %d\n", bo_name);
    printf("width: %d\n", width);
    printf("height: %d\n", height);
    printf("stride: %d\n", stride);
    printf("bpp: %d\n", bpp);
    printf("sourceCropLeft: %d\n", sourceCropLeft);
    printf("sourceCropTop: %d\n", sourceCropTop);
    printf("sourceCropRight: %d\n", sourceCropRight);
    printf("sourceCropBottom: %d\n", sourceCropBottom);
    printf("displayFrameLeft: %d\n", displayFrameLeft);
    printf("displayFrameTop: %d\n", displayFrameTop);
    printf("displayFrameRight: %d\n", displayFrameRight);
    printf("displayFrameBottom: %d\n\n\n", displayFrameBottom);
}

static int postAllLayersDone()
{
    printf("postAllLayersDone received\n\n\n");
    return 0;
}

static uint32_t getDisplayWidth()
{
    printf("getDisplayWidth received\n\n\n");
    return 9999;
}

static uint32_t getDisplayHeight()
{
    printf("getDisplayHeight received\n\n\n");
    return 9999;
}

static uint32_t getDisplayFPS()
{
    printf("getDisplayFPS received\n\n\n");
    return 9999;
}

static uint32_t getDisplayXDpi()
{
    printf("getDisplayXDpi received\n\n\n");
    return 9999;
}

static uint32_t getDisplayYDpi()
{
    printf("getDisplayYDpi received\n\n\n");
    return 9999;
}

static void postVirtualDisplay(const char* name, uint32_t id, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t orientation)
{
    printf("postVirtualDisplay received\n");
    printf("name: %s\n", name);
    printf("id: %d\n", id);
    printf("bo_name: %d\n", bo_name);
    printf("width: %d\n", width);
    printf("height: %d\n", height);
    printf("stride: %d\n", stride);
    printf("bpp: %d\n", bpp);
    printf("orientation: %d\n", orientation);

}

static int postAllVirtualDisplaysDone()
{
    printf("postAllVirtualDisplaysDone received\n\n\n");
    return 0;
}

static int postBoForScreenRecord(uint32_t at_name, uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t ver)
{
    printf("postBoForScreenRecord received\n");
    printf("at_name: %u\n", at_name);
    printf("name: %u\n", name);
    printf("width: %d\n", width);
    printf("height: %d\n", height);
    printf("stride: %d\n", stride);
    printf("ver: %d\n", ver);
    return 0;
}

static void postCaptureScreenColorBuffer(uint32_t name)
{
    printf("postCaptureScreenColorBuffer received\n");
    printf("name: %u\n", name);
}

static int authenticateMagic(uint32_t magic)
{
    printf("authenticateMagic received\n");
    printf("magic: %u\n", magic);
    return 0;
}

int main(int argc, char** argv)
{
    printf("pid %d\n", getpid());

    char addrStr[SERVER_PATH_SIZE] = {0};
    char serverPath[SERVER_PATH_SIZE] = {0};
    DisplayControlDecoder* decoder;
    
    if (android_initDisplayControl() != 0) {
        fprintf(stderr,"Failed to initialize display control\n");
        return -1;
    }

    strcpy(addrStr, "/tmp/android/display-control");
    if (android_display_control_set_path(addrStr) != 0) {
        fprintf(stderr,"Failed to set bind path\n");
        return -1;
    }

    if (android_startDisplayControl() != 0) {
        fprintf(stderr,"Failed to start Opengles Renderer\n");
        return -1;
    }

    decoder = DisplayControlDecoder::getInstance();
    if(decoder)
    {
        decoder->fbPost = FBPost;
        decoder->postLayer = postLayer;
        decoder->postAllLayersDone = postAllLayersDone;
        decoder->getDisplayWidth = getDisplayWidth;
        decoder->getDisplayHeight = getDisplayHeight;
        decoder->getDisplayFPS = getDisplayFPS;
        decoder->getDisplayXDpi = getDisplayXDpi;
        decoder->getDisplayYDpi = getDisplayYDpi;
        decoder->postVirtualDisplay = postVirtualDisplay;
        decoder->postAllVirtualDisplaysDone = postAllVirtualDisplaysDone;
        decoder->postBoForScreenRecord = postBoForScreenRecord;
        decoder->postCaptureScreenColorBuffer = postCaptureScreenColorBuffer;
        decoder->authenticateMagic = authenticateMagic;
    }

    memset(serverPath, 0, sizeof(serverPath));
    android_display_control_server_path(serverPath, sizeof(serverPath));
    printf("server path: %s\n", serverPath);
    chmod(serverPath, 0777);
    chmod(dirname(serverPath), 0777);

    while(1)
        sleep(1);

    android_stopDisplayControl();

    return 0;

}
