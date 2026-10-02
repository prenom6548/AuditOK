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
#ifndef WX_WECHAT_REALIZATION_H
#define WX_WECHAT_REALIZATION_H

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>


#ifdef __cplusplus
extern "C" {
#endif

int wechat_ops_configure(bio_dev *dev, GKeyFile *conf);
int bio_drv_wechat_ops_driver_init(bio_dev *dev);
int bio_drv_wechat_ops_discover(bio_dev *dev);
void bio_drv_wechat_ops_free(bio_dev *dev);
int bio_drv_wechat_ops_open(bio_dev *dev);
void bio_drv_wechat_ops_close(bio_dev *dev);
char *bio_drv_wechat_ops_capture(bio_dev *dev, OpsActions action);
int bio_drv_wechat_ops_enroll(bio_dev *dev, OpsActions action, int uid, int idx, char *bio_idx_name);
int bio_drv_wechat_ops_verify(bio_dev *dev, OpsActions action, int uid, int idx);
int bio_drv_wechat_ops_identify(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);
feature_info *bio_drv_wechat_ops_search(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);
int bio_drv_wechat_ops_clean(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);
feature_info *bio_drv_wechat_ops_get_feature_list(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end);
int bio_drv_wechat_ops_stop_by_user(bio_dev *dev, int waiting_ms);
const char *bio_drv_wechat_ops_get_dev_status_mesg(bio_dev *dev);
const char *bio_drv_wechat_ops_get_ops_result_mesg(bio_dev *dev);
const char *bio_drv_wechat_ops_get_notify_mid_mesg(bio_dev *dev);


#ifdef __cplusplus
}
#endif

#endif