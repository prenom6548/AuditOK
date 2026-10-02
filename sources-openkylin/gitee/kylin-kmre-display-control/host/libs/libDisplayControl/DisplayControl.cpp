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

#include <unistd.h>

#include "DisplayControl.h"
#include "DisplayControlDecoder.h"

extern "C" {
#include "kmre_display_control.h"

}
static int32_t dec_FBPost(uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t bpp)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();

    if(decoder && decoder->fbPost)
        return decoder->fbPost(name, width, height, stride, bpp);

    return -1;
}

static void dec_postLayer(const char* name, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t sourceCropLeft, int32_t sourceCropTop, int32_t sourceCropRight, int32_t sourceCropBottom, int32_t displayFrameLeft, int32_t displayFrameTop, int32_t displayFrameRight, int32_t displayFrameBottom)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();

    if(decoder && decoder->postLayer)
        decoder->postLayer( name,
                            bo_name,
                            width,
                            height,
                            stride,
                            bpp,
                            sourceCropLeft,
                            sourceCropTop,
                            sourceCropRight,
                            sourceCropBottom,
                            displayFrameLeft,
                            displayFrameTop,
                            displayFrameRight,
                            displayFrameBottom);
/*
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
*/
}

static int dec_postAllLayersDone()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->postAllLayersDone)
        return decoder->postAllLayersDone();

    return -1;
}

static uint32_t dec_getDisplayWidth()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->getDisplayWidth)
        return decoder->getDisplayWidth();

    return 480;
}

static uint32_t dec_getDisplayHeight()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->getDisplayHeight)
        return decoder->getDisplayHeight();

    return 800;
}

static uint32_t dec_getDisplayFPS()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->getDisplayFPS)
        return decoder->getDisplayFPS();

    return 60;
}

static uint32_t dec_getDisplayXDpi()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->getDisplayXDpi)
        return decoder->getDisplayXDpi();

    return 100;
}

static uint32_t dec_getDisplayYDpi()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->getDisplayYDpi)
        return decoder->getDisplayYDpi();

    return 100;
}

static void dec_postVirtualDisplay(const char* name, uint32_t id, uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t orientation)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();

    if(decoder && decoder->postVirtualDisplay)
        decoder->postVirtualDisplay( name,
                            id,
                            bo_name,
                            width,
                            height,
                            stride,
                            bpp,
                            orientation);
}

static int dec_postAllVirtualDisplaysDone()
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    if(decoder && decoder->postAllVirtualDisplaysDone)
        return decoder->postAllVirtualDisplaysDone();

    return -1;
}

static int32_t dec_postBoForScreenRecord(uint32_t at_name, uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t ver)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();

    if(decoder && decoder->postBoForScreenRecord)
        return decoder->postBoForScreenRecord(at_name, name, width, height, stride, ver);
    return -1;
}

static void dec_postCaptureScreenColorBuffer(uint32_t name)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();

    if(decoder && decoder->postCaptureScreenColorBuffer)
        decoder->postCaptureScreenColorBuffer(name);
}

static int32_t dec_authenticateMagic(uint32_t magic)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    
    if(decoder && decoder->authenticateMagic)
        return decoder->authenticateMagic(magic);
    return -1;

}

static int32_t dec_updateSharingScreenDisplay(uint32_t name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t swap)
{
    DisplayControlDecoder* decoder = DisplayControlDecoder::getInstance();
    
    if(decoder && decoder->updateSharingScreenDisplay)
        return decoder->updateSharingScreenDisplay(name, width, height, stride, bpp, swap);
    return -1;

}

void initDisplayControlContext(displayControl_decoder_context_t *dec)
{
    dec->FBPost                     = dec_FBPost;
    dec->postLayer                  = dec_postLayer;
    dec->postAllLayersDone          = dec_postAllLayersDone;
    dec->getDisplayWidth            = dec_getDisplayWidth;
    dec->getDisplayHeight           = dec_getDisplayHeight;
    dec->getDisplayFPS              = dec_getDisplayFPS;
    dec->getDisplayXDpi             = dec_getDisplayXDpi;
    dec->getDisplayYDpi             = dec_getDisplayYDpi;
    dec->postVirtualDisplay         = dec_postVirtualDisplay;
    dec->postAllVirtualDisplaysDone = dec_postAllVirtualDisplaysDone;
    dec->postBoForScreenRecord      = dec_postBoForScreenRecord;
    dec->postCaptureScreenColorBuffer      = dec_postCaptureScreenColorBuffer;
    dec->authenticateMagic          = dec_authenticateMagic;
    dec->updateSharingScreenDisplay = dec_updateSharingScreenDisplay;

}
