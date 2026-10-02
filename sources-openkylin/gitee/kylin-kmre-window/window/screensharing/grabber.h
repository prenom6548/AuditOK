/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#pragma once

#include <QRect>
#include <stdint.h>
#include <vector>

#include "sessionsettings.h"

#define MAX_IMAGE_WIDTH 20000
#define MAX_IMAGE_HEIGHT 20000

#define KEY_SHM_SCREEN_DATA 0x58687888

using DisplayPlatform = SessionSettings::DisplayPlatform;

namespace kmre {

enum PIX_FMT{
	PIX_FMT_UNKNOW = 0,
	PIX_FMT_PAL8,
	PIX_FMT_RGB565,
	PIX_FMT_RGB555,
	PIX_FMT_BGR24,
	PIX_FMT_RGB24,
	PIX_FMT_BGRA,
	PIX_FMT_RGBA,
	PIX_FMT_ABGR,
	PIX_FMT_ARGB,
	PIX_FMT_YUV420P,
	PIX_FMT_YUV422P,
	PIX_FMT_YUV444P,
	PIX_FMT_NV12,
	PIX_FMT_YUYV422,
};

enum class GrabScreenStatus {
	eError,// error
	eReady, // screen monitor is ready
	eUnplugged, // screen monitor is unplugged
};

class Grabber 
{
public:
	Grabber(uint32_t x, uint32_t y, uint32_t w, uint32_t h) 
		: m_grab_area(x, y, w, h)
		, m_grab_screen_status(GrabScreenStatus::eError)
		, m_data_size(0)
		, m_fmt(PIX_FMT_UNKNOW) {}

protected:
	QRect m_grab_area;
	GrabScreenStatus m_grab_screen_status;
	uint32_t m_data_size;
	PIX_FMT m_fmt;

public:
	static Grabber* GetGrabber(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0, bool c = true);
	static void DeleteGrabber();

    virtual ~Grabber();

	virtual uint32_t GetImageWidth() = 0;
	virtual uint32_t GetImageHeight() = 0;
	virtual PIX_FMT GetImageFormat() = 0;
	virtual uint32_t GetImageSize() = 0;
	virtual void *GetImageData() = 0;
	virtual bool IsGrabSizeChanged() = 0;
	virtual void GetImageInfo(uint32_t &w, uint32_t &h, uint32_t &ds, PIX_FMT &fmt) = 0;
	virtual void UpdateGrabArea() = 0;
	virtual std::vector<QRect> getScreenRects() = 0;
	GrabScreenStatus GetGrabScreenStatus() {return m_grab_screen_status;}
	QRect GetGrabArea() {return m_grab_area;}

private:
	static Grabber *pGrabber;
};

}
