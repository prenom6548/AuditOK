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

#include "grabber.h"
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <vector>

class X11Grabber : public kmre::Grabber 
{
private:
	static uint32_t cur_shm_key;
	bool m_record_cursor;

	Display *m_x11_display;
	int m_x11_screen;
	Window m_x11_root;
	Visual *m_x11_visual;
	int m_x11_depth;
	XImage *m_x11_image;
	XShmSegmentInfo m_x11_shm_info;
	bool m_x11_shm_server_attached;
	bool m_grab_inited;
	bool m_grab_size_changed;
	std::vector<QRect> m_screen_dead_space;

private:
	virtual ~X11Grabber();

	
	bool AllocateImage(uint32_t width, uint32_t height);
	void FreeImage();
	void DisplayInit();
	void DrawCursor(XImage* image, int recording_area_x, int recording_area_y);
	kmre::PIX_FMT GetPixelFormat(XImage* image) ;
	void ClearRectangle(XImage* image, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

public:
	X11Grabber(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool cursor);

	uint32_t GetImageWidth() override {return m_x11_image ? m_grab_area.width() : 0;}
	uint32_t GetImageHeight() override {return m_x11_image ? m_grab_area.height() : 0;}
	kmre::PIX_FMT GetImageFormat() override {return m_x11_image ? m_fmt : kmre::PIX_FMT_UNKNOW;}
	uint32_t GetImageSize() override {return m_x11_image ? m_data_size : 0;}
	void *GetImageData() override {return m_x11_image ? m_x11_image->data : nullptr;}

	void GetImageInfo(uint32_t &w, uint32_t &h, uint32_t &ds, kmre::PIX_FMT &fmt) override {
		w = m_x11_image ? m_grab_area.width() : 0; 
		h = m_x11_image ? m_grab_area.height() : 0; 
		ds = m_x11_image ? m_data_size : 0; 
		fmt = m_x11_image ? m_fmt : kmre::PIX_FMT_UNKNOW;
	}
	bool IsGrabSizeChanged() override {return m_grab_size_changed;}

	bool GrabImage();
	void UpdateGrabArea() override;
	std::vector<QRect> getScreenRects() override;
};
