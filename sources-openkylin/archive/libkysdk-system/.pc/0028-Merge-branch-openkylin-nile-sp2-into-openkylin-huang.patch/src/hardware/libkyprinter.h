/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LIBKYPRINTER_H
#define LIBKYPRINTER_H

/**
 * @file libkyprinter.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 打印信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2023
 * @defgroup libkysdk-hardware
 * @{
 * @since 2.0.0.0
 */

//打印机状态
#define CUPS_PRINTER_STATE_BUSY 6       //"Busy"
#define CUPS_PRINTER_STATE_STOPPED 4    //"Stop"
#define CUPS_PRINTER_STATE_IDLE 2       //"Idle"
#define CUPS_PRINTER_STATE_PROCESSING 3 //"Processing"

//打印任务状态
#define IPP_JOB_PENDING	3               //挂起等待打印
#define IPP_JOB_HELD	4                 //等待打印
#define IPP_JOB_PROCESSING	5           //打印中
#define IPP_JOB_STOPPED	6               //停止
#define IPP_JOB_CANCELED	7             //取消
#define IPP_JOB_ABORTED	8               //由于错误跳过该打印任务
#define IPP_JOB_COMPLETED	9             //正常完成

//下载远程相关状态
#define URL_UNAVAILABLE	-5                
#define CAN_NOT_CHECK_URL -6
#define PATH_NOT_EXIST -7
#define PATH_NO_PERMISSION -8
#define PATH_ERROR -9

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取本地的所有打印机(包含可用打印机和不可用打印机)
 * 
 * @return char** 打印机名称列表，由NULL字符串表示结尾；由alloc生成，需要被kdk_printer_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_printer_get_list();

/**
 * @brief 获取本地的所有可用打印机(打印机处于idle状态)
 * 
 * @return char** 打印机名称列表，由NULL字符串表示结尾；由alloc生成，需要被kdk_printer_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_printer_get_available_list();


/**
 * @brief 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印
 * 
 * @param printername       打印机名，kdk_printer_get_list返回的值
 * @param number_up         cups属性，一张纸打印几页，如2 4等
 * @param media             cups属性，纸张类型，如A4
 * @param number_up_layout  cups属性，如lrtb
 * @param sides             cups属性，单面如one-sided，双面如two-sided-long-edge(长边翻转),two-sided-short-edge(短边翻转)
 * 
 * @return 无
 */
extern void kdk_printer_set_options(int number_up,const char *media,const char *number_up_layout,const char *sides);

/**
 * @brief 下载网络文件
 * 
 * @param url               要打印的url
 * @param filepath          保存的文件路径
 * 
 * @return 打印作业的id，失败返回-1
 * 
 */
extern int kdk_printer_print_download_remote_file(const char *url, const char *filepath);

/**
 * @brief 打印本地文件
 * 
 * @param printername       打印机名，kdk_printer_get_list返回的值
 * @param filepath          文件绝对路径
 * 
 * @return 打印作业的id
 * 
 * @notice 虽然支持多种格式的打印，但是打印除pdf之外的格式打印的效果都不够好，建议打印pdf格式的文件
 *         打印机处于stop状态函数返回0
 *         格式不对的文件可以成功创建打印任务，但是打印不出来。什么叫格式不对，举个例子，比如当前要打印日志文件a.log,
 *         然后把a.log改名叫a.pdf,这个时候a.pdf就变成了格式不对的文件，打印不出来了
 * 
 */
extern int kdk_printer_print_local_file(const char *printername, const char *filepath);

/**
 * @brief 清除某个打印机的所有打印队列
 * 
 * @param printername       打印机名，kdk_printer_get_list返回的值
 * 
 * @return 成功返回0,失败返回-1
 * 
 * @notice 同kdk_printer_print_local_file
 */
extern int kdk_printer_cancel_all_jobs(const char *printername);

/**
 * @brief  获取当前打印机状态(状态不是实时更新)
 * 
 * @param  printername      打印机名，kdk_printer_get_list返回的值
 * 
 * @return  状态码
 */
extern int kdk_printer_get_status(const char *printername);

/**
 * @brief  获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）
 * 
 * @param  printername      打印机名，kdk_printer_get_list返回的值
 * 
 * @return  状态码
 */
extern int kdk_printer_get_job_status(const char *printername, int jobid);

/**
 * @brief  获取url内的文件名
 * 
 * @param  url     
 * 
 * @return 解析出来的函数名
 */
extern char* kdk_printer_get_filename(const char *url);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 * 
 * @since 2.2.3.5
 */
extern inline void kdk_printer_freeall(char **ptr);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example kysdk-system/src/hardware/test/kyprinterlist-test.c
 * \example kysdk-system/src/hardware/test/kyprinterprint-test.c
 * 
 */

/**
  * @}
  */
