/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author: Droiing <jianglinxuan@kylinos.cn>
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
#include <biometric_intl.h>
#include <biometric_version.h>

#include "driver_ids.h"
#include "ud650.h"

char * capture_base64 = NULL;

void* GetDlFun(void *Handle, char *pFunName) {
	char* eInfo = NULL;
	void* fun = NULL;

	fun = dlsym(Handle, pFunName);
	eInfo = dlerror();
	if(eInfo != NULL)
	{
		bio_print_error(_("Gets the function '%s' error from UD650 driver: %s\n"),
						pFunName, eInfo);
		return NULL;
	}
	return fun;
}

int load_close_source_dl() {
	char* eInfo;

	bio_print_info(_("Loading the dynamic link library '%s'\n"), ud650_ext_drv);

	memset(&m_ComFun, 0, sizeof(m_ComFun));
	memset(&m_VeinFun, 0, sizeof(m_VeinFun));


	hLibHandle = dlopen(ud650_ext_drv, RTLD_LAZY);
	if(hLibHandle == NULL)
	{
		eInfo = dlerror();
		bio_print_error(_("dlopen %s error: %s\n"), ud650_ext_drv, eInfo);
		return -1;
	}

	// 操作设备方法
	m_ComFun.XG_GetVeinLibVer = GetDlFun(hLibHandle, "XG_GetVeinLibVer");
	m_ComFun.XG_DetectUsbDev = GetDlFun(hLibHandle, "XG_DetectUsbDev");
	m_ComFun.XG_OpenVeinDev = GetDlFun(hLibHandle, "XG_OpenVeinDev");
	m_ComFun.XG_CloseVeinDev = GetDlFun(hLibHandle, "XG_CloseVeinDev");
	m_ComFun.XG_SendPacket = GetDlFun(hLibHandle, "XG_SendPacket");
	m_ComFun.XG_RecvPacket = GetDlFun(hLibHandle, "XG_RecvPacket");
	m_ComFun.XG_WriteData = GetDlFun(hLibHandle, "XG_WriteData");
	m_ComFun.XG_ReadData = GetDlFun(hLibHandle, "XG_ReadData");
	m_ComFun.XG_Upgrade = GetDlFun(hLibHandle, "XG_Upgrade");
	m_ComFun.XG_WriteDevEnrollData = GetDlFun(hLibHandle, "XG_WriteDevEnrollData");
	m_ComFun.XG_ReadDevEnrollData = GetDlFun(hLibHandle, "XG_ReadDevEnrollData");
	m_ComFun.XG_SetCallBack = GetDlFun(hLibHandle, "XG_SetCallBack");
	m_ComFun.XG_GetVeinChara = GetDlFun(hLibHandle, "XG_GetVeinChara");
	m_ComFun.XG_GetFingerStatus = GetDlFun(hLibHandle, "XG_GetFingerStatus");


	// 算方法库方法
	m_VeinFun.XGV_CreateVein = GetDlFun(hLibHandle, "XGV_CreateVein");
	m_VeinFun.XGV_DestroyVein = GetDlFun(hLibHandle, "XGV_DestroyVein");
	m_VeinFun.XGV_SetSecurity = GetDlFun(hLibHandle, "XGV_SetSecurity");
	m_VeinFun.XGV_Enroll = GetDlFun(hLibHandle, "XGV_Enroll");
	m_VeinFun.XGV_Verify = GetDlFun(hLibHandle, "XGV_Verify");
	m_VeinFun.XGV_CharaVerify = GetDlFun(hLibHandle, "XGV_CharaVerify");
	m_VeinFun.XGV_GetCharaVerifyLearn = GetDlFun(hLibHandle, "XGV_GetCharaVerifyLearn");
	m_VeinFun.XGV_SaveEnrollData = GetDlFun(hLibHandle, "XGV_SaveEnrollData");
	m_VeinFun.XGV_GetEnrollData = GetDlFun(hLibHandle, "XGV_GetEnrollData");
	m_VeinFun.XGV_DelEnrollData = GetDlFun(hLibHandle, "XGV_DelEnrollData");
	m_VeinFun.XGV_GetEnrollNum = GetDlFun(hLibHandle, "XGV_GetEnrollNum");
	m_VeinFun.XGV_GetEnptyID = GetDlFun(hLibHandle, "XGV_GetEnptyID");
	m_VeinFun.XGV_GetUserTempNum = GetDlFun(hLibHandle, "XGV_GetUserTempNum");
	m_VeinFun.XGV_GetEnrollUserId = GetDlFun(hLibHandle, "XGV_GetEnrollUserId");

	bio_print_info(_("Load the dynamic link library '%s' successful\n"), ud650_ext_drv);

	return 0;
}

int ops_configure(bio_dev * dev, GKeyFile * conf)
{
	dev->driver_id = UD650_ID;
	dev->device_name = "ud650";
	dev->full_name = "Top Glory Tech Unique UD650";
	dev->bioinfo.biotype = BioT_FingerVein;
	dev->bioinfo.stotype = StoT_OS;
	dev->bioinfo.eigtype = EigT_Eigenvalue;
	dev->bioinfo.vertype = VerT_Software;
	dev->bioinfo.idtype = IdT_Software;
	dev->bioinfo.bustype = BusT_USB;

//	dev->usb_info.vendor_id = 0x200d;
//	dev->usb_info.product_id = 0x7638;
//	dev->usb_info.vendor_id = 0x2109;
//	dev->usb_info.product_id = 0x7638;
//	dev->usb_info.class_id = 0;
	dev->usb_info.id_table = ud650_id_table;

	dev->usb_info.driver_data = 0;

	dev->ops_configure = ops_configure;
	dev->ops_driver_init = ud650_ops_driver_init;
	dev->ops_discover = ud650_ops_discover;
	dev->ops_open = ud650_ops_open;
	dev->ops_enroll = ud650_ops_enroll;
	dev->ops_verify = ud650_ops_verify;
	dev->ops_identify = ud650_ops_identify;
	dev->ops_capture = ud650_ops_capture;
	dev->ops_search = ud650_ops_search;
	dev->ops_clean = ud650_ops_clean;
//	dev->ops_get_feature_list = ud650_ops_get_feature_list;
	dev->ops_get_feature_list = NULL;
	dev->ops_attach = ud650_ops_attach;
	dev->ops_detach = ud650_ops_detach;
	dev->ops_stop_by_user = ud650_ops_stop_by_user;
	dev->ops_close = ud650_ops_close;
	dev->ops_free = ud650_ops_free;
	dev->ops_get_ops_result_mesg = ud650_ops_get_ops_result_mesg;
	dev->ops_get_dev_status_mesg = ud650_ops_get_dev_status_mesg;
	dev->ops_get_notify_mid_mesg = ud650_ops_get_notify_mid_mesg;

	bio_set_drv_api_version(dev);
	bio_set_dev_status(dev, DEVS_COMM_IDLE);
	bio_set_ops_result(dev, OPS_COMM_SUCCESS);
	bio_set_notify_mid(dev, NOTIFY_COMM_IDLE);

	dev->enable = bio_dev_is_enable(dev, conf);
	return 0;
}

const char * ud650_ops_get_dev_status_mesg(bio_dev *dev)
{
	return NULL;
}

const char * ud650_ops_get_ops_result_mesg(bio_dev *dev)
{
	return NULL;
}

const char * ud650_ops_get_notify_mid_mesg(bio_dev *dev)
{
	switch (bio_get_notify_mid(dev)) {
	case GET_FV_ERROR:
		return _("Collect finger vein characteristics error");
	case GET_FV_NEED_RELEASE:
		return _("Finger vein characteristics collected, please raise your finger");
	case GET_FV_NEED_PUSH:
		return _("Collecting finger vein characteristics, please press you finger");
	case GET_FV_NEED_PUSH_2:
		return _("For the 2nd collection finger vein characteristics,"
				 " please press you finger");
	case GET_FV_NEED_PUSH_3:
		return _("For the 3rd collection finger vein characteristics,"
				 " please press you finger");
	case GET_FV_NEED_PUSH_4:
		return _("For the 4th collection finger vein characteristics,"
				 " please press you finger");
	case GET_FV_NEED_PUSH_5:
		return _("For the 5th collection finger vein characteristics,"
				 " please press you finger");

	case ERR_OPEN_VEIN_DEV:
		return _("Open UD650 finger vein device error");
	case ERR_GET_VEIN_LIB_VER:
		return _("Get UD650 algorithms library version error");
	case ERR_CREATE_VEIN:
		return _("Initialize UD650 algorithms library error");
	case ERR_SET_SECURITY:
		return _("Set security level error");
	case ERR_DEL_ENROLL_DATA:
		return _("Delete the contrast space data error");
	case ERR_ENROLL_DATA:
		return _("Load finger vein characteristics to the contrast space error");
	case ERR_SAVE_ENROLL_DATA:
		return _("Load data to the contrast space error");
	case ERR_GET_ENROLL_DATA:
		return _("Get data from the contrast space error");

	default:
		return NULL;
	}
}

int ud650_ops_driver_init(bio_dev *dev)
{
	static bool loaded = false;
	if (!loaded) {
		if (load_close_source_dl() <0 ) {
			bio_print_error(_("Load the dynamic link library '%s' failed\n"), ud650_ext_drv);
			return -1;
		}
		loaded = true;
	} else
		bio_print_info(_("Dynamic link library %s has been loaded\n"), ud650_ext_drv);

	return 0;
}

int ud650_ops_discover(bio_dev *dev)
{
	int usbNum = 0;

	bio_print_info(_("Detect %s device\n"), dev->device_name);

	if (m_ComFun.XG_OpenVeinDev == NULL) {
		 bio_print_warning(_("Function XG_OpenVeinDev is NULL\n"));
		 return -1;
	}

	if (m_ComFun.XG_DetectUsbDev == NULL) {
		 bio_print_warning(_("Function XG_DetectUsbDev is NULL\n"));
		 return -1;
	}

	// 检测USB指静脉设备
	usbNum = m_ComFun.XG_DetectUsbDev();
	if( usbNum <= 0 )
	{
		 bio_print_info(_("No %s finger vein device detected\n"), dev->device_name);
		 return usbNum;
	}

	bio_print_info(_("There is %d %s finger vein device detected\n"),
				   usbNum, dev->device_name);

	return usbNum;
}

void ud650_ops_attach(bio_dev *dev) {
}

void ud650_ops_detach(bio_dev *dev) {
}

// 打开设备，创建算法库，成功返回0，失败返回1
int ud650_ops_open(bio_dev *dev)
{
	char pwd[16] = PASSWORD;
	char Ver[100] = {0};

	bio_set_dev_status(dev, DEVS_COMM_IDLE);
	bio_set_ops_result(dev, OPS_COMM_SUCCESS);
	bio_set_notify_mid(dev, NOTIFY_COMM_IDLE);

	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		return -1;
	}
	bio_set_dev_status(dev, DEVS_OPEN_DOING);

	int ret = m_ComFun.XG_OpenVeinDev("USB", 0, iDevAddress, (UINT8*)pwd,
									  strnlen(pwd,16), &m_hDevHandle);
	if (ret != XG_ERR_SUCCESS)
	{
		bio_print_error(_("XG_OpenVeinDev ERROR[%d]\n"), ret);
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_OPEN_ERROR);
		bio_set_notify_abs_mid(dev, ERR_OPEN_VEIN_DEV);
		return -1;
	}

	// 获得设备版本号
	if (m_ComFun.XG_GetVeinLibVer(Ver) == XG_ERR_SUCCESS) {
		bio_print_notice(_("UD650 driver library version: %s\n"), Ver);
	} else {
		bio_print_error(_("Get UD650 driver library version error\n"));
		bio_set_notify_abs_mid(dev, ERR_GET_VEIN_LIB_VER);
	}

	// 创建算法库
	ret = m_VeinFun.XGV_CreateVein(&m_hVeinHandle, UD650_USER_MAX);
	if (ret != XG_ERR_SUCCESS)
	{
		bio_print_error(_("Create UD650 algorithms library error[%d]\n"), ret);
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_OPEN_ERROR);
		bio_set_notify_abs_mid(dev, ERR_CREATE_VEIN);
		return -1;
	}

	ret = m_VeinFun.XGV_SetSecurity(m_hVeinHandle, 1);
	if (ret != XG_ERR_SUCCESS)
	{
		bio_print_error(_("Set UD650 security level error[%d]\n"), ret);
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_OPEN_ERROR);
		bio_set_notify_abs_mid(dev, ERR_SET_SECURITY);
		return -1;
	}

	bio_print_info(_("Open device and create UD650 algorithms library successful\n"));

	/* 分配指静脉特征数据数组 */
	int i = 0;
	for (i = 0; i < SAMPLE_TIMES; i++) {
		fv_data[i] = malloc(FV_SIZE);
		fv_base64[i] = malloc(FV_BASE64_SIZE);

		memset(fv_data[i], 0, FV_SIZE);
		memset(fv_base64[i], 0, FV_BASE64_SIZE);

		fv_data_len[i] = 0;
	}

	bio_set_dev_status(dev, DEVS_COMM_IDLE);
	bio_set_ops_abs_result(dev, OPS_OPEN_SUCCESS);
	return 0;
}

/**
 * @brief	获得指静脉信息
 * @param[in,out]	unsigned char* fvData	存放指静脉信息的buffer
 * @param[in]		unsigned int size		buffer大小
 * @param[in]		unsigned int* len		数据长度
 * @param[in]		unsigned int g_num		采集次数
 * @param[in]		unsigned int interval	时间间隔，毫秒
 * @exception
 * @return	OpsStatus	操作状态
 * @retval	OPS_COMM_SUCCESS(0)	成功
 *			其他					错误
 * @note	fvData一般是2048大小;
 *			循环判断g_num次，每次等待时间interval(ms)
 */
int getFingerVeinData(bio_dev *dev, unsigned int sample_times, unsigned int interval)
{
	int ret = -1;
	int timeout = bio_get_ops_timeout_ms();
	int timeused = 0;
	unsigned int num = 0;
	enum FingerStatus{
		S_ERROR = -1,
		S_RELEASE = 0,
		S_PUSH = 1,
	};
	int now_status = S_PUSH;
	int need_status = S_RELEASE;

	bio_set_notify_abs_mid(dev, GET_FV_NEED_PUSH);
	bio_print_info("%s\n", ud650_ops_get_notify_mid_mesg(dev));
	num = 0;
	while (num < sample_times) {
		usleep(interval * 1000);
		timeused += interval;

		// 判断是否放入手指
		ret = 0;
		now_status = -1;
		ret = m_ComFun.XG_GetFingerStatus(iDevAddress, m_hDevHandle);
		if (ret > 0)
			now_status = S_PUSH;
		else if (ret == 0)
			now_status = S_RELEASE;
		else {
			now_status = GET_FV_ERROR;
			bio_set_notify_abs_mid(dev, GET_FV_ERROR);
			bio_print_info("%s\n", ud650_ops_get_notify_mid_mesg(dev));
			return OPS_COMM_ERROR;
		}

		if (now_status == need_status) {
			if (now_status == S_PUSH) {
				memset(fv_data[num], 0, FV_SIZE);
				memset(fv_base64[num], 0, FV_TEMP_SIZE);
				ret = m_ComFun.XG_GetVeinChara(iDevAddress, fv_data[num],
											   (UINT32 *)&fv_data_len[num],
											   m_hDevHandle);
				if (ret <= 0) {
					bio_set_ops_result(dev, OPS_COMM_ERROR);
					bio_set_notify_abs_mid(dev, GET_FV_ERROR);
					bio_print_info("%s (%d:%d)\n", bio_get_notify_mid_mesg(dev),
						   ret, fv_data_len[num]);
					return OPS_COMM_ERROR;
				}
				fv_data_len[num] = ret;
				bio_base64_encode(fv_data[num], fv_base64[num],
								  fv_data_len[num]);

				need_status = S_RELEASE;
				timeused = 0;
				bio_print_info(_("The %dth sampling is successful and the eigenvalue "
						 "is: %.32s\n"), num + 1, fv_base64[num]);
				num++;
				bio_set_notify_abs_mid(dev, GET_FV_NEED_RELEASE);
				bio_print_info("%s\n", ud650_ops_get_notify_mid_mesg(dev));
			}

			if (now_status == S_RELEASE) {
				need_status = S_PUSH;
				bio_set_notify_abs_mid(dev, GET_FV_NEED_PUSH + num);
				bio_print_info("%s\n", ud650_ops_get_notify_mid_mesg(dev));
				timeused = 0;
			}
		}

		if (bio_is_stop_by_user(dev)) {
			bio_set_notify_mid(dev, NOTIFY_COMM_STOP_BY_USER);
			bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
			return OPS_COMM_STOP_BY_USER;
		}

		if (timeused > timeout) {
			bio_set_notify_mid(dev, NOTIFY_COMM_TIMEOUT);
			bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
			return OPS_COMM_TIMEOUT;
		}
	}

	bio_set_notify_abs_mid(dev, GET_FV_NEED_RELEASE);

	return OPS_COMM_SUCCESS;
}

int ud650_ops_enroll(bio_dev *dev, OpsActions action, int uid, int bio_idx,
					 char * bio_idx_name)
{
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		bio_set_notify_mid(dev, NOTIFY_COMM_DISABLE);
		return -1;
	}

	if (action == ACTION_START) {
		bio_set_dev_status(dev, DEVS_ENROLL_DOING);

		int ret = getFingerVeinData(dev, SAMPLE_TIMES, INTERVAL);
		if (ret != OPS_COMM_SUCCESS) {
			bio_print_error(_("getFingerVeinData error: %s\n"), bio_get_notify_mid_mesg(dev));
			if ((ret == OPS_COMM_STOP_BY_USER) || (ret == OPS_COMM_TIMEOUT))
				bio_set_ops_result(dev, ret);
			else
				bio_set_ops_abs_result(dev, OPS_ENROLL_ERROR);
			bio_set_dev_status(dev, DEVS_COMM_IDLE);

			return -1;
		}

		int i = 0;
		for (i = 0; i < SAMPLE_TIMES; i++) {
			bio_print_info(_("The %dth sampling is successful and the eigenvalue(Base64) "
				   "is: %.*s\n"), i, 16, fv_base64[i]);
		}

		ret = m_VeinFun.XGV_DelEnrollData(m_hVeinHandle, UD650_USER_INDEX);
		if (ret != XG_ERR_SUCCESS) {
			bio_set_notify_abs_mid(dev, ERR_DEL_ENROLL_DATA);
			bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
			return -1;
		}

		for (i = 0; i < SAMPLE_TIMES; i++) {
			ret = m_VeinFun.XGV_Enroll(m_hVeinHandle, UD650_USER_INDEX,
									   fv_data[i], fv_data_len[i], 0, NULL);
			if (ret != XG_ERR_SUCCESS) {
				bio_set_notify_abs_mid(dev, ERR_ENROLL_DATA);
				bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
				return -1;
			}
		}

		ret = m_VeinFun.XGV_SaveEnrollData(m_hVeinHandle, UD650_USER_INDEX,
										   0, NULL, 0);
		if (ret != XG_ERR_SUCCESS) {
			bio_set_notify_abs_mid(dev, ERR_SAVE_ENROLL_DATA);
			bio_print_info("%s[%d]\n", bio_get_notify_mid_mesg(dev), ret);
			return -1;
		}

		UINT32 len = 0;
		unsigned char * fv_temp_data = NULL;
		static char * fv_temp_base64 = NULL;
		fv_temp_data = malloc(FV_TEMP_SIZE);
		fv_temp_base64 = malloc(FV_TEMP_BASE64_SIZE);
		memset(fv_temp_data, 0, FV_TEMP_SIZE);
		memset(fv_temp_base64, 0, FV_TEMP_BASE64_SIZE);

		ret = m_VeinFun.XGV_GetEnrollData(m_hVeinHandle, UD650_USER_INDEX,
										  fv_temp_data, &len);
		if (ret != XG_ERR_SUCCESS) {
			bio_set_notify_abs_mid(dev, ERR_GET_ENROLL_DATA);
			bio_print_info("%s[%d]\n", bio_get_notify_mid_mesg(dev), ret);
			free(fv_temp_base64);
			free(fv_temp_data);
			return -1;
		}
		bio_base64_encode(fv_temp_data, fv_temp_base64, len);

		feature_info * info;
		info = bio_sto_new_feature_info(uid, dev->bioinfo.biotype,
												dev->device_name, bio_idx,
												bio_idx_name);
		info->sample = bio_sto_new_feature_sample(-1, NULL);
		info->sample->no = 1;
		info->sample->data = bio_sto_new_str(fv_temp_base64);

		print_feature_info(info);

		sqlite3 *db = bio_sto_connect_db();
		bio_sto_set_feature_info(db, info);
		bio_sto_disconnect_db(db);

		bio_sto_free_feature_info_list(info);
		free(fv_temp_base64);
		free(fv_temp_data);


		bio_set_ops_abs_result(dev, OPS_ENROLL_SUCCESS);
		bio_set_notify_abs_mid(dev, NOTIFY_ENROLL_SUCCESS);
	} else {
		bio_set_ops_abs_result(dev, OPS_ENROLL_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_ENROLL_STOP_BY_USER);
	}

	bio_set_dev_status(dev, DEVS_COMM_IDLE);
	return 0;
}

char * ud650_ops_capture(bio_dev *dev, OpsActions action)
{
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		return NULL;
	}

	if (action == ACTION_START) {
		bio_set_dev_status(dev, DEVS_CAPTURE_DOING);

		int ret = getFingerVeinData(dev, 1, INTERVAL);
		if (ret != OPS_COMM_SUCCESS) {
			bio_print_error(_("getFingerVeinData error: %s\n"), bio_get_notify_mid_mesg(dev));
			if ((ret == OPS_COMM_STOP_BY_USER) || (ret == OPS_COMM_TIMEOUT))
				bio_set_ops_result(dev, ret);
			else
				bio_set_ops_abs_result(dev, OPS_CAPTURE_ERROR);
			bio_set_dev_status(dev, DEVS_COMM_IDLE);

			return NULL;
		}

		if (capture_base64 != NULL)
			free(capture_base64);
		capture_base64 = bio_sto_new_str(fv_base64[0]);

		bio_set_ops_abs_result(dev, OPS_CAPTURE_SUCCESS);
		bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_SUCCESS);
	} else {
		if (capture_base64 != NULL)
			free(capture_base64);
		capture_base64 = NULL;
		bio_set_ops_abs_result(dev, OPS_CAPTURE_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_STOP_BY_USER);
	}

	bio_set_dev_status(dev, DEVS_COMM_IDLE);
	return capture_base64;
}

/**
 * @brief ud650_fv_find
 * 当前录入指静脉信息与数据库中信息逐个匹配
 * @param[in] bio_dev * dev		设备结构体
 * @param[in] int uid			指定用户ID，-1代表匹配所有用户
 * @param[in] int bio_idx_start	指定开始的索引
 * @param[in] int bio_idx_end	指定结束的索引，-1代表到最后
 * @return feature_info *	匹配结果链表
 * @retval	NULL	未匹配到
 */
int ud650_fv_find(bio_dev *dev, int uid, int idx_start, int idx_end,
                            feature_info ** found)
{
    *found = NULL;
    int ret = getFingerVeinData(dev, 1, INTERVAL);
    if (ret != OPS_COMM_SUCCESS) {
        bio_print_error(_("getFingerVeinData error: %s\n"), bio_get_notify_mid_mesg(dev));
        if ((ret == OPS_COMM_STOP_BY_USER) ||
                (ret == OPS_COMM_TIMEOUT) ||
                (ret == OPS_COMM_ERROR))
            bio_set_ops_result(dev, ret);
        else
            bio_set_ops_abs_result(dev, ret);

        return -1;
    }

    bio_print_info(_("The eigenvalue(Base64) is: %.*s\n"), 32, fv_base64[0]);

    sqlite3 *db = bio_sto_connect_db();
    feature_info * info, * info_list;

    info_list = bio_sto_get_feature_info(db, uid, dev->bioinfo.biotype,
                                                 dev->device_name, idx_start,
                                                 idx_end);
    print_feature_info(info_list);
    bio_sto_disconnect_db(db);

    ret = 0;
    info = info_list;
    unsigned char * verify_data = NULL;
    verify_data = malloc(FV_TEMP_SIZE);
    if (verify_data == NULL) {
        bio_set_notify_mid(dev, NOTIFY_COMM_OUT_OF_MEM);
        bio_set_ops_result(dev, OPS_COMM_OUT_OF_MEM);
        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
        bio_sto_free_feature_info_list(info_list);
        return -1;
    }
    memset(verify_data, 0, FV_TEMP_SIZE);

    int verify_len = 0;
    unsigned char * char_ret = NULL;
    feature_info found_head;
    found_head.next = NULL;
    *found = &found_head;
    while (info != NULL) {
        feature_sample * sample = info->sample;
        while (sample != NULL) {
            memset(verify_data, 0, FV_TEMP_SIZE);

            verify_len = bio_base64_decode(sample->data, verify_data);
            char_ret = m_VeinFun.XGV_CharaVerify(m_hVeinHandle,
                                                 verify_data, verify_len,
                                                 fv_data[0], fv_data_len[0]);
            if (char_ret != NULL) {
                (*found)->next= bio_sto_new_feature_info(info->uid, info->biotype,
                                                  info->driver, info->index,
                                                  info->index_name);
                (*found)->next->sample = bio_sto_new_feature_sample(sample->no, sample->data);
                (*found) = (*found)->next;
                break;
            }
            sample = sample->next;
        }
        info = info->next;
    }
    *found = found_head.next;
    bio_sto_free_feature_info_list(info_list);
	free(verify_data);

    return 0;
}

int ud650_ops_verify(bio_dev *dev, OpsActions action, int uid, int idx)
{
	feature_info * found;
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		return -1;
	}

	if (action == ACTION_START) {
		int found_idx = -1;

		bio_set_dev_status(dev, DEVS_VERIFY_DOING);

		int fv_find_ret = ud650_fv_find(dev, uid, idx, idx, &found);
		if (fv_find_ret != 0)
		{
			bio_sto_free_feature_info_list(found);
			found = NULL;
		} else {
			if (found != NULL) {
				bio_set_ops_abs_result(dev, OPS_VERIFY_MATCH);
				bio_set_notify_abs_mid(dev, NOTIFY_VERIFY_MATCH);
				found_idx = found->index;
				bio_sto_free_feature_info(found);
			} else {
				bio_set_ops_abs_result(dev, OPS_VERIFY_NO_MATCH);
				bio_set_notify_abs_mid(dev, NOTIFY_VERIFY_NO_MATCH);
			}
		}

		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		return found_idx;
	} else {
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_result(dev, OPS_VERIFY_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_VERIFY_STOP_BY_USER);
		return -1;
	}
}

int ud650_ops_identify(bio_dev *dev, OpsActions action, int uid,
					   int idx_start, int idx_end)
{
	feature_info * found;
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev,DEVS_COMM_DISABLE);
		return -1;
	}

	if (action == ACTION_START) {
		int found_uid = -1;

		bio_set_dev_status(dev, DEVS_IDENTIFY_DOING);

		int fv_find_ret = ud650_fv_find(dev, uid, idx_start, idx_end, &found);
		if (fv_find_ret != 0)
		{
			bio_sto_free_feature_info_list(found);
			found = NULL;
		} else {
			if (found != NULL) {
				bio_set_ops_abs_result(dev, OPS_IDENTIFY_MATCH);
				bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_MATCH);
				found_uid = found->uid;
				bio_sto_free_feature_info(found);
			} else {
				bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
				bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
			}
		}

		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		return found_uid;
	} else {
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_IDENTIFY_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_STOP_BY_USER);
		return -1;
	}
}

feature_info * ud650_ops_search(bio_dev *dev, OpsActions action, int uid,
										int idx_start, int idx_end)
{
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		return NULL;
	}

	if (action == ACTION_START) {
		feature_info * found = NULL;

		bio_set_dev_status(dev, DEVS_SEARCH_DOING);

		int fv_find_ret = ud650_fv_find(dev, uid, idx_start, idx_end, &found);
		if (fv_find_ret != 0)
		{
			bio_sto_free_feature_info_list(found);
			found = NULL;
		} else {
			if (found != NULL) {
				bio_set_ops_abs_result(dev, OPS_SEARCH_MATCH);
				bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_MATCH);
			} else {
				bio_set_ops_abs_result(dev, OPS_SEARCH_NO_MATCH);
				bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_NO_MATCH);
			}
		}

		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		return found;
	} else {
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_result(dev, OPS_SEARCH_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_STOP_BY_USER);
		return NULL;
	}
}

int ud650_ops_clean(bio_dev *dev, OpsActions action, int uid,
					int idx_start, int idx_end)
{
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		return 0;
	}

	if (action == ACTION_START) {
		bio_set_dev_status(dev, DEVS_CLEAN_DOING);

		sqlite3 * db;
		db = bio_sto_connect_db();
		int ret = 0;
		ret = bio_sto_clean_feature_info(db, uid, dev->bioinfo.biotype,
												 dev->device_name, idx_start,
												 idx_end);
		bio_sto_disconnect_db(db);
		if (ret == 0) {
			bio_set_ops_abs_result(dev, OPS_CLEAN_SUCCESS);
			bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_SUCCESS);
		} else {
			bio_set_ops_result(dev, OPS_CLEAN_FAIL);
			bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_FAIL);
		}

		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		return ret;
	} else {
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_CLEAN_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_STOP_BY_USER);
		return 0;
	}

}

feature_info * ud650_ops_get_feature_list(bio_dev *dev, OpsActions action,
										  int uid, int idx_start, int idx_end)
{
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
		return NULL;
	}

	if (action == ACTION_START) {
		feature_info * finfo_list = NULL;
		sqlite3 *db = bio_sto_connect_db();

		bio_set_dev_status(dev, DEVS_GET_FLIST_DOING);
		usleep(100000);

		finfo_list = bio_sto_get_feature_info(db, uid, dev->bioinfo.biotype,
													 dev->device_name, idx_start,
													 idx_end);
		print_feature_info(finfo_list);
		bio_sto_disconnect_db(db);

		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_GET_FLIST_SUCCESS);
		bio_set_notify_abs_mid(dev, NOTIFY_GET_FLIST_SUCCESS);
		return finfo_list;
	} else {
		bio_set_dev_status(dev, DEVS_COMM_IDLE);
		bio_set_ops_abs_result(dev, OPS_GET_FLIST_STOP_BY_USER);
		bio_set_notify_abs_mid(dev, NOTIFY_GET_FLIST_STOP_BY_USER);
		return NULL;
	}
}

int ud650_ops_stop_by_user(bio_dev * dev, int waiting_ms)
{
	bio_print_info(_("Device %s[%d] received interrupt request\n"),
		   dev->device_name, dev->driver_id);
	int timeout = bio_get_ops_timeout_ms();
	int timeused = 0;
	int dev_status = bio_get_dev_status(dev);
    int ops_class = -1;
    ops_class = dev_status / 100;

	if (waiting_ms < timeout)
		timeout = waiting_ms;

	if (bio_get_dev_status(dev) % 100 != DEVS_COMM_IDLE)
		bio_set_dev_status(dev, ops_class * 100 + DEVS_COMM_STOP_BY_USER);
	while ((bio_get_dev_status(dev) % 100 != DEVS_COMM_IDLE) &&
		   (timeused < timeout)){
		timeused += INTERVAL;
		usleep(INTERVAL * 1000);
	}

	if (bio_get_dev_status(dev) % 100 == DEVS_COMM_IDLE)
		return 0;

	// 中断失败，还原操作状态
	bio_set_dev_status(dev, dev_status);
	return -1;
}

void ud650_ops_close(bio_dev *dev)
{
	if (dev->enable == FALSE) {
		bio_set_dev_status(dev, DEVS_COMM_DISABLE);
	}
	bio_set_dev_status(dev, DEVS_CLOSE_DOING);

	if (m_hVeinHandle != NULL) {
		m_VeinFun.XGV_DestroyVein(m_hVeinHandle);
		m_hVeinHandle = NULL;
	}

	if (m_hDevHandle != NULL) {
		m_ComFun.XG_CloseVeinDev(iDevAddress, m_hDevHandle);
		m_hDevHandle = NULL;
	}

	int i = 0;
	for (i = 0; i < SAMPLE_TIMES; i++) {
		if (fv_data[i] != NULL) {
			free(fv_data[i]);
			fv_data[i] = NULL;
		}
		if (fv_base64 != NULL) {
			free(fv_base64[i]);
			fv_base64[i] = NULL;
		}
	}
	if (capture_base64 != NULL) {
		free(capture_base64);
		capture_base64 = NULL;
	}

	bio_set_dev_status(dev, DEVS_COMM_IDLE);
}

void ud650_ops_free(bio_dev *dev)
{
	bio_print_info(_("Unload the dynamic link library '%s'\n"), ud650_ext_drv);
	if (hLibHandle != NULL) {
		dlclose(hLibHandle);
		hLibHandle = NULL;
	}
}

void ud650_empty_func(void) {

}


