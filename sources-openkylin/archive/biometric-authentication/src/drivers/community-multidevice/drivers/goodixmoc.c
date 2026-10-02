/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author: Droiing <jianglinxuan@kylinos.cn>
 *         chenziyi <chenziyi@kylinos.cn>
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <unistd.h>

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>

#include "community_ops.h"

static const struct usb_id  goodixmoc_id_table[] = {
	{ .idVendor = 0x27c6, .idProduct = 0x5840, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x609c, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x60a2, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x631c, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x634c, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x6384, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x639c, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x63ac, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x63bc, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x63cc, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x6496, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x6584, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x658c, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x6592, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x6594, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x659a, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x659c, .description = "Goodix MOC Fingerprint Sensor" },
    { .idVendor = 0x27c6, .idProduct = 0x6a94, .description = "Goodix MOC Fingerprint Sensor" },
	{ 0, 0, NULL },
};


int ops_configure(bio_dev * dev, GKeyFile * conf)
{
	dev->driver_id = bio_get_empty_driver_id();
	dev->device_name = GOODIXMOC_NAME;
	dev->full_name = "goodixmoc";
	dev->bioinfo.biotype = BioT_FingerPrint;
	dev->bioinfo.stotype = StoT_OS;
	dev->bioinfo.eigtype = EigT_Eigenvalue;
	dev->bioinfo.vertype = VerT_Software;
	dev->bioinfo.idtype = IdT_Software;
	dev->bioinfo.bustype = BusT_USB;

	dev->usb_info.id_table = goodixmoc_id_table;
	dev->usb_info.driver_data = 0;

	dev->ops_configure = ops_configure;
	dev->ops_driver_init = community_ops_driver_init;
	dev->ops_discover = community_ops_discover;
	dev->ops_open = community_ops_open;
	dev->ops_enroll = community_ops_enroll;
	dev->ops_verify = community_ops_verify;
	dev->ops_identify = community_ops_identify;
	dev->ops_capture = community_ops_capture;
	dev->ops_search = community_ops_search;
	dev->ops_clean = community_ops_clean;
	dev->ops_get_feature_list = community_ops_get_feature_list;
	dev->ops_attach = community_ops_attach;
	dev->ops_detach = community_ops_detach;
	dev->ops_stop_by_user = community_ops_stop_by_user;
	dev->ops_feature_rename = NULL;	// Use the default function provided by the framework
	dev->ops_close = community_ops_close;
	dev->ops_free = community_ops_free;

	dev->ops_get_ops_result_mesg = community_ops_get_ops_result_mesg;
	dev->ops_get_dev_status_mesg = community_ops_get_dev_status_mesg;
	dev->ops_get_notify_mid_mesg = community_ops_get_notify_mid_mesg;

	bio_set_drv_api_version(dev);
	bio_set_dev_status(dev, DEVS_COMM_IDLE);
	bio_set_ops_result(dev, OPS_COMM_SUCCESS);
	bio_set_notify_mid(dev, NOTIFY_COMM_IDLE);

	dev->enable = bio_dev_is_enable(dev, conf);

	driver_info *priv = g_new0 (driver_info, 1);
	dev->dev_priv = priv;

	priv->community_driver_id = GOODIXMOC_NAME;

	community_para_config(dev, conf);

	return 0;
}
