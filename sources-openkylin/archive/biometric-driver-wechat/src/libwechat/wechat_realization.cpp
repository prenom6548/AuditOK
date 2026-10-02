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

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "wechat_realization.h"
#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>
#include "wechat_internal.h"
#include "wechat_type.h"
#include "ksb_http.h"

int wechat_ops_configure(bio_dev *dev, GKeyFile *conf)
{
    bio_print_debug("wechat ops_configure start\n");

    // 动态获取驱动 ID
    dev->driver_id = 32;         //设置驱动及设备相关信息
    bio_print_debug("wechat driver id : %d\n",dev->driver_id);
    dev->bioinfo.biotype = UniT_Remote;                   //远程账户
    // dev->bioinfo.biotype = BioT_Face;                   //远程账户
    dev->bioinfo.stotype = StoT_OS;                     //存储在操作系统中
    dev->bioinfo.eigtype = EigT_Data;                   //特征值
    dev->bioinfo.vertype = VerT_Software;               //软件验证
    dev->bioinfo.idtype = IdT_Software;                 //软件识别
    dev->bioinfo.bustype = BusT_USB;                    //USB接口
    // 设置驱动的回调函数
    dev->ops_driver_init = bio_drv_wechat_ops_driver_init;
    dev->ops_discover = bio_drv_wechat_ops_discover;
    dev->ops_open = bio_drv_wechat_ops_open;
    dev->ops_enroll = bio_drv_wechat_ops_enroll;
    dev->ops_verify = bio_drv_wechat_ops_verify;
    dev->ops_identify = bio_drv_wechat_ops_identify;
    dev->ops_capture = bio_drv_wechat_ops_capture;
    dev->ops_search = bio_drv_wechat_ops_search;
    dev->ops_clean = bio_drv_wechat_ops_clean;
    dev->ops_get_feature_list = bio_drv_wechat_ops_get_feature_list;
    dev->ops_stop_by_user = bio_drv_wechat_ops_stop_by_user;
    dev->ops_close = bio_drv_wechat_ops_close;
    dev->ops_free = bio_drv_wechat_ops_free;
    dev->ops_get_ops_result_mesg = bio_drv_wechat_ops_get_ops_result_mesg;
    dev->ops_get_dev_status_mesg = bio_drv_wechat_ops_get_dev_status_mesg;
    dev->ops_get_notify_mid_mesg = bio_drv_wechat_ops_get_notify_mid_mesg;
    dev->ops_attach = NULL;                          // 框架支持的设备插入时的回调函数
    dev->ops_detach = NULL;                          // 框架支持的设备拔出时的回调函数
    //TODO:id_table
    //dev->usb_info.id_table = wechat_driver_id_table;
    // 保留字
    dev->usb_info.driver_data = 0;

    // 设置驱动使用的 DRV_API 版本号
    bio_set_drv_api_version(dev);

    // 设置驱动的初始状态
    bio_set_dev_status(dev, DEVS_COMM_IDLE);
    bio_set_ops_result(dev, OPS_COMM_SUCCESS);
    bio_set_notify_mid(dev, NOTIFY_COMM_IDLE);

    // 获取并设置驱动的使能状态(由框架从框架配置文件中读取)
    dev->enable = bio_dev_is_enable(dev, conf);
    // 为驱动分配私有结构体
    // wechat_driver 为驱动私有结构体数组，在drive_type.h中定义
    wechat_driver *priv =(wechat_driver *) malloc(sizeof(wechat_driver));
    memset(priv, 0, sizeof(wechat_driver));


    // dev_priv：设备驱动私有结构体，指向设备驱动还需要的额外结构体，用于驱动在各个处理函数中快速索引的数据及接口
    dev->dev_priv = priv;
    bio_print_debug("wechat ops_configure end\n");

    return 0;
}

int bio_drv_wechat_ops_driver_init(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_driver_init start\n");
    // 私有结构体
    //TODO:  初始化内容待定，暂时返回0
    return 0;
}



/**
 * @brief 设备检测函数
 */
int bio_drv_wechat_ops_discover(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_discover start\n");
    //TODO: 设备返回值暂定为1，默认找到设备
    return 1;
}


/**
 * @brief 驱动释放函数
 */
void bio_drv_wechat_ops_free(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_free start\n");
    //TODO:暂时无释放需求
    bio_print_debug("bio_drv_wechat_ops_free end\n");
}

/**
 * @brief 设备打开函数
 */
int bio_drv_wechat_ops_open(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_open start\n");
    // 无设备打开函数
    wechat_driver *priv = (wechat_driver *) dev->dev_priv;



    priv->frameFD = memfd_create("wechat", 0);
    bio_print_debug("%d\n", priv->frameFD);
    // 与框架同步文件描述符
    bio_drv_set_frame_fd(dev, priv->frameFD); 


    priv->ctrlFlag = CONTROL_FLAG_RUNNING;
    bio_print_debug("bio_drv_wechat_ops_open end\n");
    return 0;
}

/**
 * @brief 设备关闭函数
 */
void bio_drv_wechat_ops_close(bio_dev *dev)
{
    wechat_driver *priv = (wechat_driver *) dev->dev_priv;
    bio_drv_release_frame_fd(dev);
    bio_print_debug("bio_drv_wechat_ops_close start\n");
    //无设备关闭函数
    return;
}

/**
 * @brief 特征捕获函数，用来捕获生物特征信息
 */
char *bio_drv_wechat_ops_capture(bio_dev *dev, OpsActions action)
{
    wechat_driver *priv = (wechat_driver *) dev->dev_priv;
    bio_print_debug("bio_drv_wechat_ops_capture start\n");

    char *feature_data = NULL;
    feature_data = ( char *) malloc(1024);

    //设备驱动不启用
    if (dev->enable == false) {
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);              //设备状态设置为：设备被禁用
        bio_set_ops_result(dev, OPS_COMM_ERROR);                 //操作结果设置为：通用操作错误
        bio_set_notify_abs_mid(dev, NOTIFY_COMM_DISABLE);        //提示消息设置为：设备不可用
        return NULL;
    }

    bio_set_dev_status(dev, DEVS_CAPTURE_DOING);                 //设备状态设置为：正在捕获信息

   // bio_set_notify_abs_mid(dev, MID_GETTING);
    int ret = wechat_capture(dev,feature_data);
    if (priv->ctrlFlag == CONTROL_FLAG_STOPING || priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_dev_status(dev, DEVS_COMM_STOP_BY_USER);
        bio_set_ops_result(dev, OPS_CAPTURE_STOP_BY_USER);
        bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_STOP_BY_USER);
        return NULL;
    }
    //bio_base64_encode(feature_data, feature_encode, priv->faceFeatureLen);                   //获取面部识别特征
    if (feature_data == NULL)
    {
        bio_print_debug("failed to capture feature data\n");
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_ops_result(dev, OPS_CAPTURE_FAIL);
        bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_FAIL);
        return NULL;
    }
    bio_set_dev_status(dev, DEVS_COMM_IDLE);                    //当前状态设置为：空闲状态
    bio_set_ops_result(dev, OPS_CAPTURE_SUCCESS);               //操作结果设置为：捕获成功
    bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_SUCCESS);        //提示消息设置为：捕获成功

    return feature_data;

}


/**
 * @brief 特征录入函数,用来录入用户的生物特征信息
 */
int bio_drv_wechat_ops_enroll(bio_dev *dev, OpsActions action, int uid, int idx, char *bio_idx_name)
{
    wechat_driver *priv = (wechat_driver *) dev->dev_priv;
    bio_print_debug("wechat feature id : %d\n", idx);
    bio_print_debug("wechat user id: %d\n", idx);
    bio_print_debug("bio_drv_wechats_ops_enroll start\n");

    //设备驱动不启用
    if (dev->enable == false) {
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);                           //设备状态设置为：设备被禁用
        bio_set_ops_result(dev, OPS_COMM_ERROR);                              //操作结果设置为：通用操作错误
        bio_set_notify_abs_mid(dev, NOTIFY_COMM_DISABLE);                     //提示消息设置为：设备不可用
        return -1;
    }

        //检测已录入数量
    feature_info *info_list = NULL;
    sqlite3 *db = bio_sto_connect_db();
    info_list = bio_sto_get_feature_info(db, uid, dev->bioinfo.biotype, dev->device_name, 0, -1);
    
    int feature_num = 0;
    feature_info *cursor = info_list;
    while (cursor != NULL)
    {
        cursor = cursor->next;
        feature_num++;

    }
    bio_sto_free_feature_info_list(info_list);
    
    bio_print_debug("feature_num : %d\n", feature_num);
    if (feature_num>=1)
    {
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_ops_result(dev, MID_OPS_EXCEED_ENROOL_MAX);
        return -1;
    }
    bio_sto_disconnect_db(db);
    bio_set_dev_status(dev, DEVS_ENROLL_DOING);                           //设备状态设置为：正在录入

    /*
     * 录入特征操作放在此处
     * 厂商可根据需要定义需要的变量
     */

    char* feature_data = (char*) malloc (1024);
    int ret = wechat_capture(dev,feature_data);
    bio_print_debug("network failed1\n");

    if (ret == 7)
    {
        bio_print_debug("[wechat] network failed1\n");
        bio_set_ops_abs_result(dev, MID_NETWORK_FAILED);
        bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        wechat_buf_free(feature_data);
        bio_print_debug("[wechat] network failed before return\n");
        return -1;
    }
    if (ret == 9) //GCOV_EXCL_START
    {
        bio_print_debug("[wechat] network failed1\n");
        bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);
        bio_set_ops_abs_result(dev, MID_TIME_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        wechat_buf_free(feature_data);
        bio_print_debug("[wechat] network failed before return\n");
        return -1;
    }
    if (ret)
    {
        bio_print_debug("failed to capture feature data");


        bio_set_ops_result(dev, OPS_ENROLL_ERROR);
        bio_set_ops_abs_result(dev, OPS_ENROLL_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_notify_abs_mid(dev, NOTIFY_ENROLL_FAIL);
        wechat_buf_free(feature_data);
        return -1;
    }//GCOV_EXCL_STOP
    if (priv->ctrlFlag == CONTROL_FLAG_STOPING || priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {

        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_ops_result(dev, OPS_ENROLL_STOP_BY_USER);
        bio_set_notify_abs_mid(dev, NOTIFY_ENROLL_STOP_BY_USER);
        return -1;
    }

    //将nickname写入文件
    char *nickname = (char *) wechat_buf_alloc(512);
    safe_strcpy(nickname, feature_data+28, strnlen(feature_data,2048));
    FILE * fp;
    fp = fopen("/var/lib/biometric-auth/nickname.txt","w");
    printf("nickname:%s\n",nickname);
    fputs(nickname,fp);
    fclose(fp);



    sleep(1);
    // 存储模板
    feature_info *info;
    // 创建特征结构体
    //info = bio_sto_new_feature_info(uid, dev->bioinfo.biotype, dev->device_name, idx, bio_idx_name);
    info = bio_sto_new_feature_info(uid, dev->bioinfo.biotype, dev->device_name, idx, nickname);
    // 创建新的采样结构体
    info->sample = bio_sto_new_feature_sample(-1, NULL);
    info->sample->no = 1;
    info->sample->data = bio_sto_new_str((char *)feature_data);
    // 打印特征结构体
    print_feature_info(info);
    sqlite3 *db2 = bio_sto_connect_db();
    // 连接数据库
    // 保存特征信息链表
    bio_sto_set_feature_info(db2, info);
    // 断开数据库连接
    bio_sto_disconnect_db(db2);

    // 回收特征数据项
    bio_sto_free_feature_info_list(info);

    // 资源回收
    wechat_buf_free(feature_data);

    bio_set_ops_abs_result(dev, OPS_ENROLL_SUCCESS);          // 设置操作结果：录入信息成功
    bio_set_notify_abs_mid(dev, NOTIFY_ENROLL_SUCCESS);       // 用户提醒消息：录入信息成功
    bio_set_dev_status(dev, DEVS_COMM_IDLE);                  // 设备状态：空闲状态
    sleep(1);

    bio_print_debug("bio_drv_wechat_ops_enroll end\n");
    return 0;
}


/**
 * @brief 特征验证函数,使用当前生物特征与指定特征对比,判断是否是同一个特征
 */
int bio_drv_wechat_ops_verify(bio_dev *dev, OpsActions action, int uid, int idx)
{
    bio_print_debug("bio_drv_wechat_ops_verify start\n");

    feature_info *found = NULL;

    if (dev->enable == FALSE) {       // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return -1;
    }

    bio_set_dev_status(dev, DEVS_VERIFY_DOING);         // 设备状态：正在验证

    // 验证特征操作放在此处
    wechat_driver *priv = (wechat_driver *)dev->dev_priv;
    int ret = 0;
    char *feature_data = NULL;
    feature_data = (char *)wechat_buf_alloc(1024);



    ret = wechat_capture(dev,feature_data);


    if (ret == 7)
    {
        bio_print_debug("network failed1");

        bio_set_ops_abs_result(dev, MID_NETWORK_FAILED);
        bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);
        bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return 0;
    }
    if (ret)
    {
        bio_print_debug("failed to capture feature data");

        bio_set_ops_result(dev, OPS_VERIFY_ERROR);
        bio_set_notify_mid(dev, NOTIFY_VERIFY_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return 0;
    }
    if (priv->ctrlFlag == CONTROL_FLAG_STOPING || priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_dev_status(dev, DEVS_COMM_STOP_BY_USER);
        bio_set_ops_result(dev, OPS_VERIFY_STOP_BY_USER);
        bio_set_notify_abs_mid(dev, NOTIFY_VERIFY_STOP_BY_USER);
        return 0;
    }

    //bio_set_notify_abs_mid(dev, MID_IDENTIFY);              // 用户提示：正在识别

    // 特征搜索
    found = wechat_internel_search(dev, feature_data, uid, idx, idx);

    if (priv->ctrlFlag == CONTROL_FLAG_STOPPED) {
        bio_set_ops_result(dev, OPS_COMM_STOP_BY_USER);
        bio_set_notify_mid(dev, NOTIFY_COMM_STOP_BY_USER);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        return -1;
    }

    if (found != NULL) {  // 有匹配内容
        bio_sto_free_feature_info(found);

        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Verify face feature data successful"));

        bio_set_ops_abs_result(dev, OPS_VERIFY_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_VERIFY_MATCH);
        // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        sleep(1);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));

        ret = 1;
    } else {           // 没有匹配内容
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Verify face feature data fail"));

        bio_set_ops_abs_result(dev, OPS_VERIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_VERIFY_NO_MATCH);
        // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        sleep(1);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));

        ret = 0;
    }
    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    bio_print_debug("bio_drv_wechat_ops_verify end\n");

    return ret;
}

/**
 * @brief 特征识别函数,使用当前生物特征与指定范围的特征比对,识别出当前特征与指定范围内的哪个特征匹配
 */
int bio_drv_wechat_ops_identify(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_wechat_ops_identify start\n");

    int found_uid = -1;
    feature_info *found = NULL;

    if (dev->enable == FALSE) {      //设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return -1;
    }

    bio_set_dev_status(dev, OPS_TYPE_IDENTIFY);                    // 设备状态设置为：正在识别指定特征

    // 验证特征操作放在此处
    wechat_driver *priv = (wechat_driver *)dev->dev_priv;
    char* feature_data = (char*) malloc (1024);


    int ret = wechat_capture(dev,feature_data);
    if (ret == 7)
    {
        bio_print_debug("[wechat] network failed1\n");
        bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);
        bio_set_ops_abs_result(dev, MID_NETWORK_FAILED);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        wechat_buf_free(feature_data);
        bio_print_debug("[wechat] network failed before return\n");
        return -1;
    }
    if (ret == 9)
    {
        bio_print_debug("[wechat] network failed1\n");
        bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);
        bio_set_ops_abs_result(dev, MID_TIME_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        wechat_buf_free(feature_data);
        bio_print_debug("[wechat] network failed before return\n");
        return -1;
    }
    if (ret == -1)
    {
        //收到中断的时候
        bio_print_debug("failed to capture feature data");
        bio_print_error("enter indentify stop");
        bio_set_ops_result(dev, OPS_VERIFY_ERROR);
        bio_set_notify_mid(dev, NOTIFY_VERIFY_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        wechat_buf_free(feature_data);
        return found_uid;
    }
    if (ret)
    {
        bio_print_debug("failed to capture feature data");
        bio_print_error("enter indentify stop");
        bio_set_ops_result(dev, OPS_VERIFY_ERROR);
        bio_set_notify_mid(dev, NOTIFY_VERIFY_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        wechat_buf_free(feature_data);
        return found_uid;
    }
    // 无需检测手指抬起，直接开始查找比对指纹
    if (priv->ctrlFlag == CONTROL_FLAG_STOPING || priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_ops_result(dev, OPS_IDENTIFY_STOP_BY_USER);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_STOP_BY_USER);
        return found_uid;
    }



    // 特征搜索
    found = wechat_internel_search(dev, feature_data, uid, idx_start, idx_end);

    // 中止状态
    if (priv->ctrlFlag == CONTROL_FLAG_STOPPED) {
        bio_print_error("enter indentify stop");
        bio_set_ops_result(dev, OPS_COMM_STOP_BY_USER);
        bio_set_notify_mid(dev, NOTIFY_COMM_STOP_BY_USER);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        return -1;
    }

    if (found != NULL) {
        // 获取最小的UID
        feature_info *info = found;
        while (info != NULL) {
            if ((found_uid == -1) || (found_uid > info->uid))
                found_uid = info->uid;

            info = info->next;
        }
        bio_sto_free_feature_info(found);

        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_identify face feature successful, its Minimum UID is %d"), found_uid);

        bio_set_ops_abs_result(dev, OPS_IDENTIFY_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_MATCH);
        // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
    } else {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_identify face feature fail"));

        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        //bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
    }
    bio_set_dev_status(dev, DEVS_COMM_IDLE);
    if(feature_data)
        free(feature_data);
    bio_print_debug("bio_drv_wechat_ops_identify end\n");

    return found_uid;
}


/**
 * @brief 特征搜索函数,使用当前生物特征与指定范围的特征比对,搜索出指定范围内所有匹配的特征
 */
feature_info *bio_drv_wechat_ops_search(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_wechat_ops_search start\n");

    feature_info *found = NULL;

    if (dev->enable == FALSE) {        // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return NULL;
    }

    bio_set_dev_status(dev, OPS_TYPE_SEARCH);            // 设备状态设置为：正在搜索指定特征

    // 验证特征操作放在此处
    wechat_driver *priv = (wechat_driver *)dev->dev_priv;
    char *feature_data = NULL;
    feature_data = (char *) malloc(1024);
    memset(feature_data, 0, sizeof(feature_data) + 1);


    int ret = wechat_capture(dev,feature_data);
    if (priv->ctrlFlag == CONTROL_FLAG_STOPING || priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_dev_status(dev, DEVS_COMM_STOP_BY_USER);
        bio_set_ops_result(dev, OPS_SEARCH_STOP_BY_USER);
        bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_STOP_BY_USER);
        return NULL;
    }
    if(feature_data == NULL)
    {
        bio_set_ops_result(dev, OPS_SEARCH_ERROR);
        bio_set_notify_mid(dev, NOTIFY_SEARCH_ERROR);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return NULL;
    }

    if (priv->ctrlFlag == CONTROL_FLAG_STOPING) {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        return NULL;
    }

    // 无需检测手指抬起，直接开始查找比对指纹
    //bio_set_notify_abs_mid(dev, MID_IDENTIFY);
    found = wechat_internel_search(dev, feature_data, uid, idx_start, idx_end);

    // 状态被中断
    if (priv->ctrlFlag == CONTROL_FLAG_STOPPED) {
        bio_set_ops_result(dev, OPS_COMM_STOP_BY_USER);
        bio_set_notify_mid(dev, NOTIFY_COMM_STOP_BY_USER);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);

        return NULL;
    }

    if (found != NULL) {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_search face feature successful"));

        bio_set_ops_abs_result(dev, OPS_SEARCH_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_MATCH);
        // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
    } else {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_search face feature fail"));

        bio_set_ops_abs_result(dev, OPS_SEARCH_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_NO_MATCH);
        // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
    }
    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    return found;
}

/**
 * @brief 特征清理(删除)函数,删除指定范围内的所有特征
 */
int bio_drv_wechat_ops_clean(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_wechat_ops_clean start\n");

    if (dev->enable == FALSE) {       // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return 0;
    }

    bio_set_dev_status(dev, DEVS_CLEAN_DOING);       // 设备状态设置为：正在清理特征数据

    sqlite3 *db;
    int ret = 0;
    // 连接数据库
    db = bio_sto_connect_db();
    // 从数据库中删除特征
    ret = bio_sto_clean_feature_info(db, uid, dev->bioinfo.biotype,
                                     dev->device_name, idx_start,
                                     idx_end);
    // 断开数据库
    bio_sto_disconnect_db(db);

    if (ret == 0) {
        bio_set_ops_abs_result(dev, OPS_CLEAN_SUCCESS);              // 操作结果设置为：清理特征成功
        bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_SUCCESS);           // 用户提示设置为：清理特征成功
    } else {
        bio_set_ops_result(dev, OPS_CLEAN_FAIL);                     // 操作结果设置为：清理特征失败
        bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_FAIL);              // 用户提示设置为：清理特征失败
    }

    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    return ret;
}

/**
 * @brief 获取指定设备的特征列表
 */
feature_info *bio_drv_wechat_ops_get_feature_list(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_wechat_ops_get_feature_list start\n");

    feature_info *found = NULL;
    wechat_driver *priv = (wechat_driver *)dev->dev_priv;

    if (dev->enable == FALSE) {           // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return NULL;
    }

    bio_set_dev_status(dev, OPS_TYPE_GET_FLIST);                        // 设备状态设置为：正在获取特征列表
    //连接数据库
    sqlite3 *db = bio_sto_connect_db();

    /*
	 * 获取特征列表如有硬件相关操作，则放在此处
	*/

    // 从数据库中获取特征列表
    found = bio_sto_get_feature_info(db, uid, dev->bioinfo.biotype,
                                     dev->device_name, idx_start,
                                     idx_end);
    // 打印特征列表
    print_feature_info(found);
    // 断开数据库
    bio_sto_disconnect_db(db);
    // 设置状态
    snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_get_feature_list face feature seccessful"));
    bio_set_dev_status(dev, DEVS_COMM_IDLE);
    bio_set_ops_abs_result(dev, OPS_GET_FLIST_SUCCESS);
    // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

    bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));

    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    return found;
}

/**
 * @brief 中止设备的当前操作
 */
int bio_drv_wechat_ops_stop_by_user(bio_dev *dev, int waiting_ms)
{
    bio_print_debug("bio_drv_wechat_ops_stop_by_user start\n");

    bio_print_info(("_Device %s[%d] received interrupt request\n"), dev->device_name, dev->driver_id);

    if (bio_get_dev_status(dev) == DEVS_COMM_IDLE){
        // 如果设备为空闲状态
        return 0;
    }
    wechat_driver *priv = (wechat_driver *)dev->dev_priv;
    // int timeout = bio_get_ops_timeout_ms();          // 获取通用超时时间
    int timeout =5000;
    int timeused = 0;

    if (waiting_ms < timeout)
        timeout = waiting_ms;

    // 设置状态位，用于通知采样过程用户中断
    priv->ctrlFlag = CONTROL_FLAG_STOPING;
    snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Device %s[%d] received interrupt request\n"), dev->device_name, dev->driver_id);
    // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
    while ((priv->ctrlFlag != CONTROL_FLAG_STOPPED)
           && (priv->ctrlFlag != CONTROL_FLAG_DONE)
           && (priv->ctrlFlag != CONTROL_FLAG_IDLE)
           && (timeused < timeout))
    {
        timeused += 1000;
        // usleep(1000 * 1000);
    }
    bio_print_debug("device idle to return7\n");

    return 0;
}


/**
 * @brief 获取设备状态的文本消息
 */
const char *bio_drv_wechat_ops_get_dev_status_mesg(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_get_dev_status_mesg end\n");
    return NULL;
}


/**
 * @brief 获取操作结果的文本消息
 */
const char *bio_drv_wechat_ops_get_ops_result_mesg(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_get_ops_result_mesg start\n");
    wechat_driver *priv = (wechat_driver *)dev->dev_priv;

    switch (bio_get_ops_result(dev))
    {
        case MID_OPS_EXCEED_ENROOL_MAX:
            return _("The account has been bound to a WeChat account");
        case MID_NETWORK_FAILED:
            bio_print_debug("[wechat] enum MID_NETWORK_FAILED is %d\n",MID_NETWORK_FAILED);
            return _("Network failed");
        case MID_TIME_ERROR:
            bio_print_debug("[wechat] enum MID_TIME_ERROR is %d\n",MID_TIME_ERROR);
            return _("peer's certificate or fingerprint wasn't verified fine");
        default:
            return NULL;
    }
    bio_print_debug("bio_drv_wechat_ops_get_ops_result_mesg end\n");
}

/**
 * @brief 获取面向用户的提示消息
 */
const char *bio_drv_wechat_ops_get_notify_mid_mesg(bio_dev *dev)
{
    bio_print_debug("bio_drv_wechat_ops_get_notify_mid_mesg start\n");

    wechat_driver *priv = (wechat_driver *)dev->dev_priv;

    switch (bio_get_notify_mid(dev))

    {
        case MID_EXTENDED_MESSAGE:
            return priv->extra_info;
        case MID_GETTING:
            return _("Waiting for scanning qr image");
        case MID_NETWORK_ERROR:
            bio_print_debug("[wechat] enum MID_NETWORK_ERROR is %d\n",MID_NETWORK_ERROR);
            return _("Network error ");
        default:
            return NULL;
    }
}

