/*
 * Copyright (C) 2022 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author: Fnoily <liuyang@kylinos.cn>
 *         wangweiran <wangweiran@kylinos.cn>
 *         wumuti <wumuti@kylinos.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WX_WECHAT_INTERNAL_H
#define WX_WECHAT_INTERNAL_H

#include "wechat_realization.h"
#include "wechat_type.h"

#ifdef __cplusplus
extern "C"{
#endif

void wechat_buf_clean(void *buf, unsigned long size);

void wechat_buf_free(void *buf);

void *wechat_buf_alloc(unsigned long size);

int  wechat_capture(bio_dev *dev,char* open_id);

feature_info *wechat_internel_search(bio_dev *dev, char *feature_open_id, int uid, int idx_start, int idx_end);

#ifdef __cplusplus
}
#endif 

#endif //WX_WECHAT_INTERNAL_H
