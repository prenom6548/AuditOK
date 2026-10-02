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

#include "wechat_internal.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>
#include "pthread.h"
#include "wechat_type.h"
#include "ksb_http.h"

int new_state;
int new_flag;
char new_auth_code[100];

void *wechat_buf_alloc(unsigned long size) {
    void *buf = malloc(size);
    memset(buf, 0, size);

    return buf;
}


int wechat_para_callback(int state, char *auth_code) {
    // bio_print_debug("[callback] state : %d\n", state);
    new_state = state;
    new_flag = 0;
    if (state == 5)
        safe_strcpy(new_auth_code, auth_code,1024);
    return 0;
}

// 释放空间
void wechat_buf_free(void *buf) {
    if (buf != NULL)
        free(buf);

    return;
}

// 清零空间
void wechat_buf_clean(void *buf, unsigned long size) {
    memset(buf, 0, size);

    return;
}
void strip_wechat_nickname(char *openid, char *open){

    safe_strcpy(open, openid, 28);
}
//获取扫码后的uuid的值
int wechat_capture(bio_dev *dev, char *open_id) {

    bio_print_debug("[wechat] we are in wechat\n");
    bio_print_notice("[wechat] Wechat QRcode start: kylin appid :wxfd0de93f7bd963be , Authorization protocol: OAuth 2.0\n");
    wechat_driver *priv = (wechat_driver *) dev->dev_priv;
    //char* qr_image = (char*) malloc (1024*1024);
    char qr_image[1024];
    cv::Mat image;
    std::vector<unsigned char> buff;
    new_state = 0;
    //TODO:openid 外面传入
    // char* open_id = (char*) malloc(1024);

    bio_print_debug("[wechat] we are in wechat4\n");
    int ret = GetLoginQR(qr_image, wechat_para_callback);
    bio_print_debug("[wechat] we are in wechat2\n");
    if (ret == 7 ||ret == 9) {
        image = cv::imread("/var/lib/biometric-auth/network.jpg");
        cv::imwrite("/var/lib/biometric-auth/qr_image.jpg", image);
        //把图片处理成4的倍数，否则会发生撕裂
        cv::Size dsize = cv::Size(image.cols * 48 / 47, image.rows * 48 / 47);
        cv::Mat image2 = cv::Mat(dsize, CV_32S);
        cv::resize(image, image2, dsize);
        //写入传递
        cv::imencode(".jpg", image2, buff);
        lseek(priv->frameFD, 0, SEEK_SET);
        write(priv->frameFD, buff.data(), buff.size());
        bio_notify_frame_written(dev);
        bio_print_debug("newwork disconnect : %d\n", ret);
        return ret;
    }

    if (ret) {
        bio_print_debug("failed to get qrimage : %d\n", ret);
        return -1;
    }
    //判断条件扫码结束后执行

    bio_set_notify_abs_mid(dev, MID_GETTING);

    image = cv::imread("/var/lib/biometric-auth/image.jpg");
    cv::imwrite("/var/lib/biometric-auth/qr_image.jpg", image);
    //把图片处理成4的倍数，否则会发生撕裂
    cv::Size dsize = cv::Size(image.cols * 48 / 47, image.rows * 48 / 47);
    cv::Mat image2 = cv::Mat(dsize, CV_32S);
    cv::resize(image, image2, dsize);
    //写入传递
    cv::imencode(".jpg", image2, buff);
    lseek(priv->frameFD, 0, SEEK_SET);
    write(priv->frameFD, buff.data(), buff.size());
    bio_notify_frame_written(dev);

    //初始化合成图片变量
    // double beta; double input;
    cv::Mat src1, src2, dst;
    double alpha = 0.1; // something
    int min_x = ((src1.cols - src2.cols) / 2);
    int min_y = ((src1.rows - src2.rows) / 2);

    int width, height;
    //如果出现网络错误，将网络错误的图片合并到二维码的图片中写入
    //TODO:待修改运行时机      

    while ((new_state != 5) && (priv->ctrlFlag == CONTROL_FLAG_RUNNING)) {

        if (new_state == 6 && new_flag == 0) {
            //TODO:待添加释放
            int ret = GetLoginQR(qr_image, wechat_para_callback);
            if (ret) {
                bio_print_debug("failed to get qrimage : %d\n", ret);
                return -1;
            }

            image = cv::imread("/var/lib/biometric-auth/image.jpg");
            cv::imwrite("/var/lib/biometric-auth/qr_image.jpg", image);
            //把图片处理成4的倍数，否则会发生撕裂
            dsize = cv::Size(image.cols * 48 / 47, image.rows * 48 / 47);
            image2 = cv::Mat(dsize, CV_32S);
            cv::imwrite("/var/lib/biometric-auth/qr_image.jpg", image2);
            cv::resize(image, image2, dsize);
            cv::imencode(".jpg", image2, buff);
            lseek(priv->frameFD, 0, SEEK_SET);
            write(priv->frameFD, buff.data(), buff.size());
            new_flag = 1;
            bio_notify_frame_written(dev);

        }
        if (new_state == 7 && new_flag == 0) {
            bio_set_ops_result(dev, MID_NETWORK_FAILED);
            bio_set_notify_abs_mid(dev, MID_NETWORK_ERROR);

            image = cv::imread("/var/lib/biometric-auth/network.jpg");
            cv::imwrite("/var/lib/biometric-auth/qr_image.jpg", image);
            //把图片处理成4的倍数，否则会发生撕裂
            cv::Size dsize = cv::Size(image.cols * 48 / 47, image.rows * 48 / 47);
            cv::Mat image2 = cv::Mat(dsize, CV_32S);
            cv::resize(image, image2, dsize);
            //写入传递
            cv::imencode(".jpg", image2, buff);
            lseek(priv->frameFD, 0, SEEK_SET);
            write(priv->frameFD, buff.data(), buff.size());
            bio_notify_frame_written(dev);

//            src1 = cv::imread("/var/lib/biometric-auth/image.jpg");
//            src2 = cv::imread("/var/lib/biometric-auth/network.png");
//            if (!src1.data) {
//                printf("Error loading src1 \n");
//                return NULL;
//            }
//            if (!src2.data) {
//                printf("Error loading src2 \n");
//                return NULL;
//            }
//
//            // min_x, min_y should be valid in src1 and [width height] = size(src2)
//            cv::Rect roi = cv::Rect(min_x, min_y, src2.cols, src2.rows);
//            // "out_image" is the output ; i.e. src1 with a part of it blended with  src2
//            cv::Mat out_image = src1.clone();
//            // Set the ROIs for the selected sections of src1 and out_image (the same at the moment)
//            cv::Mat src1_roi = src1(roi);
//            cv::Mat out_image_roi = out_image(roi);
//            cv::addWeighted(src1_roi, alpha, src2, 0.8, 0.0, out_image_roi);
//            cv::imwrite("/var/lib/biometric-auth/qr_image.jpg", out_image_roi);
//
//            dsize = cv::Size(out_image_roi.cols * 48 / 47, out_image_roi.rows * 48 / 47);
//            cv::Mat out_image_roi2 = cv::Mat(dsize, CV_32S);
//            cv::resize(out_image_roi, out_image_roi2, dsize);
//
//            cv::imencode(".jpg", out_image_roi2, buff);
//            lseek(priv->frameFD, 0, SEEK_SET);
//            write(priv->frameFD, buff.data(), buff.size());
//            new_flag = 1;
//            bio_notify_frame_written(dev);

            return 7;

        }

    }
    bio_print_debug("[wechat] here1\n");
    if (priv->ctrlFlag == CONTROL_FLAG_STOPING || priv->ctrlFlag == CONTROL_FLAG_STOPPED) {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_print_debug("[wechat] here2\n");
        //遇到相关问题直接return
        StopLoginQR();

        return -1;
    }
    printf("11111\n");
    bio_print_debug("[wechat] before get wechat user info\n");
    char *nickname = (char*) malloc(1024);
    ret = GetWechatUserInfo(new_auth_code, open_id, nickname);
    bio_print_debug("[wechat] after get wechat user info, nickname:%s\n",nickname);
    strcat(open_id, nickname);
    printf("444444\n");

    bio_print_debug("[wechat] after get wechat user info, openid: %s\n", open_id);

    if (ret) {
        bio_print_debug("failed to get user info : %d\n", ret);
        return -1;
    }
    return 0;
}


feature_info *wechat_internel_search(bio_dev *dev, char *feature_open_id, int uid, int idx_start, int idx_end) {
    wechat_driver *priv = (wechat_driver *) dev->dev_priv;
    // 连接数据库
    sqlite3 *db = bio_sto_connect_db();
    feature_info *info, *info_list, *found;
    feature_info found_head;
    char *template_data = NULL;
    template_data = (char *) wechat_buf_alloc(10240);
    if (feature_open_id == NULL)
        bio_print_debug("search feature data is NULL!\n");
    char *feature_name_id = (char *) wechat_buf_alloc(512);
    char *template_name = (char *) wechat_buf_alloc(512);

    safe_strcpy(feature_name_id, feature_open_id, 28);
//    strip_wechat_nickname(*feature_open_id,*feature_name_id);
    int ret;
    // 从数据库中获取特征列表
    info_list = bio_sto_get_feature_info(db, uid, dev->bioinfo.biotype, dev->device_name, idx_start, idx_end);
    // 打印特征列表
    print_feature_info(info_list);
    // 断开数据库
    bio_sto_disconnect_db(db);

    info = info_list;            // info指向info_list
    found_head.next = NULL;      // found_head的下一个特征信息指向空
    found = &found_head;         // found指针指向found_head

    // 为匹配的特征构建新的特征链表，每个特征值对应一个采样特征值
    priv->ctrlFlag = CONTROL_FLAG_RUNNING;
    while (info != NULL) {
        feature_sample *sample = info->sample;
        while (sample != NULL) {

            template_data = sample->data;
            //值保留
            safe_strcpy(template_name, template_data, 28);
            bio_print_debug("[wechat] after get database data, openid: %s\n", template_name);
            bio_print_debug("[wechat] after get from capture, openid: %s\n", feature_name_id);
           // strip_wechat_nickname(*template_data, *template_name);
//            如果两个id相同，说明存在相同的特征值
            if (strcmp(feature_name_id, template_name) == 0) {
                ret = 0;
            } else {
                ret = 1;
            }
            bio_print_debug("比对结果：%d\n", ret);
            if (ret == 0) {
                found->next = bio_sto_new_feature_info(info->uid, info->biotype, info->driver, info->index,
                                                       info->index_name);
                found->next->sample = bio_sto_new_feature_sample(sample->no, sample->data);
                found = found->next;
            }
            // 比对特征失败
            if (ret != 0) {
                if (sample->next != NULL)
                    sample = sample->next;
                else
                    break;
            }            //ret = Extra_Verify(feature_data, template_data);
            //ret = 0;
            // 操作中止
            if (priv->ctrlFlag == CONTROL_FLAG_STOPING) {   // 控制标志设置为：2
                bio_sto_free_feature_info_list(info_list);    // 释放原始的特征列表
                if (found_head.next != NULL)
                    bio_sto_free_feature_info_list(found_head.next);

                priv->ctrlFlag = CONTROL_FLAG_STOPPED;      // 控制标志设置为：3

                return NULL;
            }
            sample = sample->next;
        }
        info = info->next;
    }
    priv->ctrlFlag = CONTROL_FLAG_DONE;         // 控制标志设置为：4
    bio_sto_free_feature_info_list(info_list);    // 释放原始的特征列表
    found = found_head.next;

    bio_print_debug("wechat_internel_search close\n");
    return found;
}

