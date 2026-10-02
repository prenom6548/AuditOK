#ifndef FIDOUKEY_TYPE_H
#define FIDOUKEY_TYPE_H

#include <stdio.h>

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>

#include <fido.h>
#include <fido/bio.h>
#include <fido/credman.h>
#include <fido/eddsa.h>
#include <fido/es256.h>
#include <fido/param.h>
#include <fido/rs256.h>
#include <fido/err.h>
#include <errno.h>
#include <pwd.h>
#include <libintl.h>

// 控制标志定义
#define CONTROL_FLAG_IDLE 0
#define CONTROL_FLAG_RUNNING 1
#define CONTROL_FLAG_STOPING 2
#define CONTROL_FLAG_STOPPED 3
#define CONTROL_FLAG_DONE 4

// 采样次数
// #define ARATEK_SAMPLE_TIMES 5

// 扩展提示信息的长度
#define EXTRA_INFO_LENGTH 1024

// 状态检测结果
#define STATUS_CHECK_OK 0
#define STATUS_CHECK_TIMEOUT 1
#define STATUS_CHECK_STOP_BY_USER 2

// 需要的动作
//#define ACTION_PRESS 1
//#define ACTION_LEFT -1

// 检测时间间隔
// #define EM1600DEV_FINGER_CHECK_INTERVAL_MS 100


#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIBBIOMETRIC_DOMAIN_NAME
	#define LIBBIOMETRIC_DOMAIN_NAME "biometric-driver-fidoukey-detect"
#endif

#ifndef LOCALEDIR
	#define LOCALEDIR "/usr/local/share/locale/"
#endif

#define _(String) dgettext (LIBBIOMETRIC_DOMAIN_NAME, String)
#define N_(String) (String)


static const unsigned char cd[32] = {
	0xec, 0x8d, 0x8f, 0x78, 0x42, 0x4a, 0x2b, 0xb7,
	0x82, 0x34, 0xaa, 0xca, 0x07, 0xa1, 0xf6, 0x56,
	0x42, 0x1c, 0xb6, 0xf6, 0xb3, 0x00, 0x86, 0x52,
	0x35, 0x2d, 0xa2, 0x62, 0x4a, 0xbe, 0x89, 0x76,
};

// 定义操作提示
enum fidoukey_notify_mid {
    MID_EXTENDED_MESSAGE = NOTIFY_COMM_MAX + 1, // 定义扩展信息

    MID_ADJUST_FACE,        // 调整面部姿态
    MID_DETECTING,          // 正在检测人脸
    MID_RECOGNIZING,        // 人脸正在识别
    MID_CLOSING,            // 正在关闭摄像头
    MID_CLOSED,             // 正在打开摄像头
    //MID_GEN_TEMPLATE,	// 生成特征模板
    //MID_IDENTIFY,		// 正在识别
};

// 驱动私有结构体
// 设备驱动私有结构体，指向设备驱动还需要的额外结构体
typedef struct fidoukey_driver_t fidoukey_driver;
struct fidoukey_driver_t {
    // 超时时间，单位毫秒
    int timeoutMS;
    // 控制标志
    int ctrlFlag;
    // 扩展提示信息
    char extra_info[EXTRA_INFO_LENGTH];
    //设备名称
    const char path[48];
    //char name_list[2000];
    //LPSTR name_list = (LPSTR) malloc (1024 * sizeof(LPSTR));
    // char name[2048];
    // char applicationname[4096];
    // char container[4096];
    fido_dev_t *fidev ;
};

#ifdef __cplusplus
}
#endif


#endif // SEETAFACE_TYPE_H
