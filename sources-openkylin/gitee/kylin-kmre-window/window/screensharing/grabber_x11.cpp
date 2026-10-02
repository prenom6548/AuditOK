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

#include <QGuiApplication>
#include <QScreen>

#include "screensharing.h"
#include "grabber_x11.h"
#include <string.h>
#include <iostream>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <syslog.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>

using namespace kmre;

uint32_t X11Grabber::cur_shm_key = KEY_SHM_SCREEN_DATA;

// if w == 0 or h == 0, then will grab full screen
X11Grabber::X11Grabber(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool cursor) 
	: kmre::Grabber(x, y, width, height)
	, m_record_cursor(cursor)
	, m_x11_display(nullptr)
	, m_x11_image(nullptr)
	, m_x11_shm_info{0, -1, (char*) -1, false}
	, m_x11_shm_server_attached(false)
	, m_grab_inited(false)
	, m_grab_size_changed(false)
{
	cur_shm_key++;
	DisplayInit();
}

X11Grabber::~X11Grabber() 
{
	FreeImage();
	if(m_x11_display != nullptr) {
		XCloseDisplay(m_x11_display);
		m_x11_display = nullptr;
	}
}

void X11Grabber::DisplayInit() 
{
	m_x11_display = XOpenDisplay(NULL);
	if(m_x11_display == NULL) {
		syslog(LOG_ERR, "[%s] Can't open X display for X11Grabber!", __func__);
		return;
	}
	m_x11_screen = DefaultScreen(m_x11_display);
	m_x11_root = RootWindow(m_x11_display, m_x11_screen);
	m_x11_visual = DefaultVisual(m_x11_display, m_x11_screen);
	m_x11_depth = DefaultDepth(m_x11_display, m_x11_screen);
	if (!XShmQueryExtension(m_x11_display)) {
		syslog(LOG_ERR, "[%s] Can't find shared memory extension for X11 !", __func__);
		return;
	}

	if (m_record_cursor) {
		int event, error;
		if (!XFixesQueryExtension(m_x11_display, &event, &error)) {
			syslog(LOG_WARNING, "[%s] XFixes is not supported by X server! The cursor has been hidden.", __func__);
			m_record_cursor = false;
		}
	}

	m_grab_inited = true;
}

bool X11Grabber::AllocateImage(unsigned int width, unsigned int height) 
{
	if(m_x11_shm_server_attached && m_x11_image->width == (int) width && m_x11_image->height == (int) height) {
		return true; // reuse existing image
	}

	FreeImage();

	m_x11_image = XShmCreateImage(m_x11_display, m_x11_visual, m_x11_depth, ZPixmap, NULL, &m_x11_shm_info, width, height);
	if(m_x11_image == NULL) {
		syslog(LOG_ERR, "[%s] Can't create shared image!", __func__);
		goto err;
	}
	m_x11_shm_info.shmid = shmget(cur_shm_key, m_x11_image->bytes_per_line * m_x11_image->height, IPC_CREAT | 0600);
	if(m_x11_shm_info.shmid == -1) {
		syslog(LOG_ERR, "[%s] Can't get shared memory!", __func__);
		goto err;
	}
	m_x11_shm_info.shmaddr = (char*) shmat(m_x11_shm_info.shmid, NULL, SHM_RND);
	if(m_x11_shm_info.shmaddr == (char*) -1) {
		syslog(LOG_ERR, "[%s] Can't attach to shared memory!", __func__);
		goto err;
	}
	m_x11_image->data = m_x11_shm_info.shmaddr;
	if(!XShmAttach(m_x11_display, &m_x11_shm_info)) {
		syslog(LOG_ERR, "[%s] Can't attach server to shared memory!", __func__);
		goto err;
	}
	m_x11_shm_server_attached = true;
	return true;
err:
	FreeImage();
	return false;
}

void X11Grabber::FreeImage() {
	if(m_x11_shm_server_attached) {
		XShmDetach(m_x11_display, &m_x11_shm_info);
		m_x11_shm_server_attached = false;
	}
	if(m_x11_shm_info.shmaddr != (char*) -1) {
		shmdt(m_x11_shm_info.shmaddr);
		m_x11_shm_info.shmaddr = (char*) -1;
	}
	if(m_x11_shm_info.shmid != -1) {
		shmctl(m_x11_shm_info.shmid, IPC_RMID, NULL);
		m_x11_shm_info.shmid = -1;
	}
	if(m_x11_image != NULL) {
		XDestroyImage(m_x11_image);
		m_x11_image = NULL;
	}
}

std::vector<QRect> X11Grabber::getScreenRects() 
{
	QList<QScreen *> screenList =  QGuiApplication::screens();  //多显示器
	std::vector<QRect> rectList;

	for (const auto &screen : screenList) {
		QRect rect = screen->geometry();
		// syslog(LOG_DEBUG, "[%s] Screen: x = %u, y = %u, w = %u, h = %u", 
		// 		__func__, rect.x(), rect.y(), rect.width(), rect.height());
		rectList.emplace_back(std::move(rect));
	}

	return rectList;
}

void X11Grabber::UpdateGrabArea() 
{
	if (!m_grab_inited) {
		syslog(LOG_ERR, "[%s] Grabber haven't inited yet!", __func__);
		m_grab_screen_status = GrabScreenStatus::eError;
		return;
	}

	std::vector<QRect> screen_rects = getScreenRects();
	if (screen_rects.size() == 0) {
		syslog(LOG_ERR, "[%s] No monitors detected!", __func__);
		m_grab_screen_status = GrabScreenStatus::eError;
		return;
	}

	int sharing_screen_index = ScreenSharing::getInstance()->getSharingScreenNum();
	if (sharing_screen_index < 0) {
		syslog(LOG_ERR, "[%s] Invalid screen index: '%d', Using default now.", __func__, sharing_screen_index);
		sharing_screen_index = 0;
	}
	else if (sharing_screen_index >= screen_rects.size()) {
		if (m_grab_screen_status != GrabScreenStatus::eUnplugged) {
			syslog(LOG_ERR, "[%s] Screen index(%d) error! Maybe monitor unpluged!", __func__, sharing_screen_index);
		}
		m_grab_screen_status = GrabScreenStatus::eUnplugged;
		return;
	}

	QRect new_grab_area = screen_rects[sharing_screen_index];
	m_grab_size_changed = (new_grab_area.size() != m_grab_area.size());

	// update grab area (full screen)
	if (m_grab_area != new_grab_area) {
		m_grab_area = new_grab_area;
		syslog(LOG_DEBUG, "[%s] Grab area changed. New grab area:{x:%d, y:%d, w:%d, h:%d}", 
				__func__, m_grab_area.x(), m_grab_area.y(), m_grab_area.width(), m_grab_area.height());
	}

	m_grab_screen_status = GrabScreenStatus::eReady;
}

bool X11Grabber::GrabImage() 
{
	if (m_grab_screen_status != GrabScreenStatus::eReady) {
		syslog(LOG_ERR, "[%s] Invalid screen status: '%d'!", __func__, m_grab_screen_status);
		return false;
	}

	if (m_grab_area.x() < 0 || m_grab_area.y() < 0 || m_grab_area.width() <= 0 || m_grab_area.height() <= 0) {
		syslog(LOG_ERR, "[%s] Invalid grab area (%d:%d:%d:%d)!", __func__,
			m_grab_area.x(), m_grab_area.y(), m_grab_area.width(), m_grab_area.height());
		return false;
	}

	// grab the image
	bool ret = false;
	do {
		if (!AllocateImage(m_grab_area.width(), m_grab_area.height())) {
			ret = false;
			break;
		}
		if (!XShmGetImage(m_x11_display, m_x11_root, m_x11_image, 
				m_grab_area.x(), m_grab_area.y(), AllPlanes)) {
			syslog(LOG_ERR, "[%s] Can't get image (using shared memory)!", __func__);
			FreeImage();
			ret = false;
			break; 
		}

		// draw the cursor
		if(m_record_cursor) {
			DrawCursor(m_x11_image, m_grab_area.x(), m_grab_area.y());
		}

		m_data_size = m_x11_image->bytes_per_line * m_grab_area.height();
		m_fmt = GetPixelFormat(m_x11_image);
		ret = true;
	} while(0);

	//syslog(LOG_DEBUG, "[%s] Grab %s, Image: width = %d, height = %d, size = %d, format = %d, shm_key = 0x%X",
	//	__func__, ret ? "succeed" : "failed!", m_x11_image->width, m_x11_image->height, m_data_size, m_fmt, cur_shm_key);

	return ret;
}

kmre::PIX_FMT X11Grabber::GetPixelFormat(XImage* image) 
{
	if (!image) {
		return kmre::PIX_FMT_UNKNOW;
	}
	switch(image->bits_per_pixel) {
		case 8: return kmre::PIX_FMT_PAL8;
		case 16: {
			if(image->red_mask == 0xf800 && image->green_mask == 0x07e0 && image->blue_mask == 0x001f) return kmre::PIX_FMT_RGB565;
			if(image->red_mask == 0x7c00 && image->green_mask == 0x03e0 && image->blue_mask == 0x001f) return kmre::PIX_FMT_RGB555;
			break;
		}
		case 24: {
			if(image->red_mask == 0xff0000 && image->green_mask == 0x00ff00 && image->blue_mask == 0x0000ff) return kmre::PIX_FMT_BGR24;
			if(image->red_mask == 0x0000ff && image->green_mask == 0x00ff00 && image->blue_mask == 0xff0000) return kmre::PIX_FMT_RGB24;
			break;
		}
		case 32: {
			if(image->red_mask == 0xff0000 && image->green_mask == 0x00ff00 && image->blue_mask == 0x0000ff) return kmre::PIX_FMT_BGRA;
			if(image->red_mask == 0x0000ff && image->green_mask == 0x00ff00 && image->blue_mask == 0xff0000) return kmre::PIX_FMT_RGBA;
			if(image->red_mask == 0xff000000 && image->green_mask == 0x00ff0000 && image->blue_mask == 0x0000ff00) return kmre::PIX_FMT_ABGR;
			if(image->red_mask == 0x0000ff00 && image->green_mask == 0x00ff0000 && image->blue_mask == 0xff000000) return kmre::PIX_FMT_ARGB;
			break;
		}
	}
	
	syslog(LOG_ERR, "[%s] Unsupported X11 image pixel format!", __func__);
	return kmre::PIX_FMT_UNKNOW;
}

void X11Grabber::ClearRectangle(XImage* image, unsigned int x, unsigned int y, unsigned int w, unsigned int h) 
{
	// check the image format
	if(image->bits_per_pixel % 8 != 0)
		return;
	unsigned int pixel_bytes = image->bits_per_pixel / 8;

	// fill the rectangle with zeros
	for(unsigned int j = 0; j < h; ++j) {
		uint8_t *image_row = (uint8_t*) image->data + image->bytes_per_line * (y + j);
		memset(image_row + pixel_bytes * x, 0, pixel_bytes * w);
	}

}

void X11Grabber::DrawCursor(XImage* image, int recording_area_x, int recording_area_y) 
{
	// check the image format
	unsigned int pixel_bytes, r_offset, g_offset, b_offset;
	if(image->bits_per_pixel == 24 && image->red_mask == 0xff0000 && image->green_mask == 0x00ff00 && image->blue_mask == 0x0000ff) {
		pixel_bytes = 3;
		r_offset = 2; g_offset = 1; b_offset = 0;
	} else if(image->bits_per_pixel == 24 && image->red_mask == 0x0000ff && image->green_mask == 0x00ff00 && image->blue_mask == 0xff0000) {
		pixel_bytes = 3;
		r_offset = 0; g_offset = 1; b_offset = 2;
	} else if(image->bits_per_pixel == 32 && image->red_mask == 0xff0000 && image->green_mask == 0x00ff00 && image->blue_mask == 0x0000ff) {
		pixel_bytes = 4;
		r_offset = 2; g_offset = 1; b_offset = 0;
	} else if(image->bits_per_pixel == 32 && image->red_mask == 0x0000ff && image->green_mask == 0x00ff00 && image->blue_mask == 0xff0000) {
		pixel_bytes = 4;
		r_offset = 0; g_offset = 1; b_offset = 2;
	} else if(image->bits_per_pixel == 32 && image->red_mask == 0xff000000 && image->green_mask == 0x00ff0000 && image->blue_mask == 0x0000ff00) {
		pixel_bytes = 4;
		r_offset = 3; g_offset = 2; b_offset = 1;
	} else if(image->bits_per_pixel == 32 && image->red_mask == 0x0000ff00 && image->green_mask == 0x00ff0000 && image->blue_mask == 0xff000000) {
		pixel_bytes = 4;
		r_offset = 1; g_offset = 2; b_offset = 3;
	} else {
		return;
	}

	// get the cursor
	XFixesCursorImage *xcim = XFixesGetCursorImage(m_x11_display);
	if(xcim == NULL)
		return;

	// calculate the position of the cursor
	int x = xcim->x - xcim->xhot - recording_area_x;
	int y = xcim->y - xcim->yhot - recording_area_y;

	// calculate the part of the cursor that's visible
	int cursor_left = std::max(0, -x), cursor_right = std::min((int) xcim->width, image->width - x);
	int cursor_top = std::max(0, -y), cursor_bottom = std::min((int) xcim->height, image->height - y);

	// draw the cursor
	// XFixesCursorImage uses 'long' instead of 'int' to store the cursor images, which is a bit weird since
	// 'long' is 64-bit on 64-bit systems and only 32 bits are actually used. The image uses premultiplied alpha.
	for(int j = cursor_top; j < cursor_bottom; ++j) {
		unsigned long *cursor_row = xcim->pixels + xcim->width * j;
		uint8_t *image_row = (uint8_t*) image->data + image->bytes_per_line * (y + j);
		for(int i = cursor_left; i < cursor_right; ++i) {
			unsigned long cursor_pixel = cursor_row[i];
			uint8_t *image_pixel = image_row + pixel_bytes * (x + i);
			int cursor_a = (uint8_t) (cursor_pixel >> 24);
			int cursor_r = (uint8_t) (cursor_pixel >> 16);
			int cursor_g = (uint8_t) (cursor_pixel >> 8);
			int cursor_b = (uint8_t) (cursor_pixel >> 0);
			if(cursor_a == 255) {
				image_pixel[r_offset] = cursor_r;
				image_pixel[g_offset] = cursor_g;
				image_pixel[b_offset] = cursor_b;
			} else {
				image_pixel[r_offset] = (image_pixel[r_offset] * (255 - cursor_a) + 127) / 255 + cursor_r;
				image_pixel[g_offset] = (image_pixel[g_offset] * (255 - cursor_a) + 127) / 255 + cursor_g;
				image_pixel[b_offset] = (image_pixel[b_offset] * (255 - cursor_a) + 127) / 255 + cursor_b;
			}
		}
	}

	// free the cursor
	XFree(xcim);

}
