#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "fidoukey_realization.h"
#include "fidoukey_internal.h"
#include "fidoukey_type.h"

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>

#ifdef WIN32
typedef unsigned long ULONG;
#else
typedef unsigned int ULONG;
#endif

typedef char CHAR;
typedef CHAR *LPSTR;
typedef int BOOL;

// int pin_flag;
// char pincode[10];

/**
 * @brief 驱动配置项
 */
int fidoukey_ops_configure(bio_dev *dev, GKeyFile *conf)
{
    bio_print_debug("ops_configure start\n");

    // 动态获取驱动 ID
    dev->driver_id = 64; //设置驱动及设备相关信息
    bio_print_debug("%d\n", dev->driver_id);
    dev->bioinfo.biotype = BioT_FingerPrint; //高阶UKEY
    dev->bioinfo.stotype = StoT_OS;            //存储在操作系统中
    dev->bioinfo.eigtype = EigT_Data;          //特征值
    dev->bioinfo.vertype = VerT_Software;      //软件验证
    dev->bioinfo.idtype = IdT_Software;        //软件识别
    dev->bioinfo.bustype = BusT_USB;           // USB接口

    // 设置驱动的回调函数
    dev->ops_driver_init = bio_drv_fidoukey_ops_driver_init;
    dev->ops_discover = bio_drv_fidoukey_ops_discover;
    dev->ops_open = bio_drv_fidoukey_ops_open;
    dev->ops_enroll = bio_drv_fidoukey_ops_enroll;
    dev->ops_verify = NULL;
    dev->ops_identify = bio_drv_fidoukey_ops_identify;
    dev->ops_capture = NULL;
    dev->ops_search = NULL;
    dev->ops_clean = bio_drv_fidoukey_ops_clean;
    dev->ops_get_feature_list = NULL;
    dev->ops_stop_by_user = bio_drv_fidoukey_ops_stop_by_user;
    dev->ops_close = bio_drv_fidoukey_ops_close;
    dev->ops_free = bio_drv_fidoukey_ops_free;
    dev->ops_get_ops_result_mesg = bio_drv_fidoukey_ops_get_ops_result_mesg;
    dev->ops_get_dev_status_mesg = bio_drv_fidoukey_ops_get_dev_status_mesg;
    dev->ops_get_notify_mid_mesg = bio_drv_fidoukey_ops_get_notify_mid_mesg;
    dev->ops_attach = bio_drv_fidoukey_ops_attach; // 框架支持的设备插入时的回调函数
    dev->ops_detach = bio_drv_fidoukey_ops_detach; // 框架支持的设备拔出时的回调函数

    // 设置驱动使用的 DRV_API 版本号
    bio_set_drv_api_version(dev);

    // 设置驱动的初始状态
    bio_set_dev_status(dev, DEVS_COMM_IDLE);
    bio_set_ops_result(dev, OPS_COMM_SUCCESS);
    bio_set_notify_mid(dev, NOTIFY_COMM_IDLE);

    // 获取并设置驱动的使能状态(由框架从框架配置文件中读取)
    dev->enable = bio_dev_is_enable(dev, conf);

    // 为驱动分配私有结构体
    // fidoukey_driver为驱动私有结构体数组，在drive_type.h中定义
    fidoukey_driver *priv = (fidoukey_driver *)malloc(sizeof(fidoukey_driver));
    memset(priv, 0, sizeof(fidoukey_driver));
    dev->dev_priv = priv;

    priv->fidev=NULL;
    bio_print_debug("ops_configure end\n");

    return 0;
}

/**
 * @brief 驱动初始化函数
 */
int bio_drv_fidoukey_ops_driver_init(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_driver_init start\n");

    // 私有结构体
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;

    // 私有结构体超时时间成员变量设置初值为：框架通用的操作超时时间
    priv->timeoutMS = bio_get_ops_timeout_ms();

    // 私有结构体控制标志成员变量设置初值为：CONTROL_FLAG_IDLE
    priv->ctrlFlag = CONTROL_FLAG_IDLE;
    /*
     * 厂家可根据需要添加初始化内容
     */

    return 0;
}

/**
 * @brief 设备检测函数
 */
int bio_drv_fidoukey_ops_discover(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_discover start\n");
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;
    fido_dev_info_t *devlist = NULL;
    fido_dev_t *fidev = NULL;
    const fido_dev_info_t *di = NULL;
    size_t ndevs = 0;
    int r;

    // fido_init(0);
    fido_init(FIDO_DEBUG);

    // if ((cred = prepare_cred(&args)) == NULL)
    //     goto err;

    devlist = fido_dev_info_new(64);
    if (!devlist)
    {
        bio_print_debug("error: fido_dev_info_new failed\n");
        return 0;
    }

    r = fido_dev_info_manifest(devlist, 64, &ndevs);
    if (r != FIDO_OK)
    {
        bio_print_debug("Unable to discover device(s), %s (%d)\n", fido_strerr(r),
                        r);
        return 0;
    }
    fidev = fido_dev_new();
    if (!fidev)
    {
        bio_print_debug("fido_dev_new failed\n");
        return 0;
    }

    di = fido_dev_info_ptr(devlist, 0);
    if (!di)
    {
        bio_print_debug("error: fido_dev_info_ptr returned NULL\n");
        return 0;
    }
    const char *devpath;
    if ((devpath = fido_dev_info_path(di)) == NULL)
    {
        bio_print_debug("error: fido_dev_path returned NULL\n");
        return 0;
    }
    strcpy(priv->path,devpath);

    bio_print_debug("device path %s \n", priv->path);

    return ndevs;
}

/**
 * @brief 驱动释放函数
 * @param
 * @return
 * @retval
 *
 */
void bio_drv_fidoukey_ops_free(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_free start\n");

    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;

    /*
     * 厂商可根据驱动初始化函数中所自定义的内容，进行资源回收
     */

    // 回收fidoukey句柄空间

    // 回收私有结构体资源
    if (priv != NULL)
        free(priv);
    dev->dev_priv = NULL;

    bio_print_debug("bio_drv_fidoukey_ops_free end\n");
}

/**
 * @brief 设备打开函数
 * @param
 * @return
 * @retval
 *
 */
int bio_drv_fidoukey_ops_open(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_open start\n");

    // 私有结构体
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;
    ULONG ret;
    bio_set_dev_status(dev, DEVS_COMM_IDLE);       //当前状态初设置为：空闲状态
    bio_set_ops_result(dev, OPS_COMM_SUCCESS);     //操作结果初设置为：空闲状态
    bio_set_notify_abs_mid(dev, NOTIFY_COMM_IDLE); //提示消息设置为：空闲状态

    if (dev->enable == false)
    {
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);       //设备状态设置为：设备被禁用
        bio_set_ops_result(dev, OPS_COMM_ERROR);          //操作结果设置为：通用操作错误
        bio_set_notify_abs_mid(dev, NOTIFY_COMM_DISABLE); //提示消息设置为：设备不可用
        return -1;
    }

    bio_set_dev_status(dev, DEVS_OPEN_DOING); //设备状态设置为：正在打开设备

    // 私有结构体控制变量成员设置为:ONTROL_FLAG_RUNNING
    priv->ctrlFlag = CONTROL_FLAG_RUNNING;

    // 设置状态
    snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Open ukey device success"));
    bio_set_dev_status(dev, DEVS_COMM_IDLE);           //当前状态设置为：空闲状态
    bio_set_ops_abs_result(dev, OPS_OPEN_SUCCESS);     //操作结果设置为：打开设备成功
    bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE); //提示消息设置为：使用扩展消息

    bio_print_debug("bio_drv_fidoukey_ops_open end\n");
    return 0;
}

/*
 * 设备关闭函数
 */
void bio_drv_fidoukey_ops_close(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_close start\n");
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;


    if (dev->enable == false)
    {
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);       //设备状态设置为：设备被禁用
        bio_set_ops_result(dev, OPS_COMM_ERROR);          //操作结果设置为：通用操作错误
        bio_set_notify_abs_mid(dev, NOTIFY_COMM_DISABLE); //提示消息设置为：设备不可用
    }

    bio_print_debug("bio_drv_fidoukey_ops_close end\n");
    return;
}

/**
 * @brief 特征捕获函数，用来捕获生物特征信息
 */
char *bio_drv_fidoukey_ops_capture(bio_dev *dev, OpsActions action)
{
    bio_print_debug("bio_drv_fidoukey_ops_capture start\n");

    char *feature_data = NULL;

    feature_data = (char *)fidoukey_buf_alloc(1024 * 50);

    //设备驱动不启用
    if (dev->enable == false)
    {
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);       //设备状态设置为：设备被禁用
        bio_set_ops_result(dev, OPS_COMM_ERROR);          //操作结果设置为：通用操作错误
        bio_set_notify_abs_mid(dev, NOTIFY_COMM_DISABLE); //提示消息设置为：设备不可用
        return NULL;
    }

    bio_set_dev_status(dev, DEVS_CAPTURE_DOING); //设备状态设置为：正在捕获信息
    bio_set_notify_abs_mid(dev, MID_DETECTING);
    int ret = 0;
    if (ret == 0)
    {
        bio_set_dev_status(dev, DEVS_COMM_IDLE);             //当前状态设置为：空闲状态
        bio_set_ops_result(dev, OPS_CAPTURE_SUCCESS);        //操作结果设置为：捕获成功
        bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_SUCCESS); //提示消息设置为：捕获成功
    }
    else if (ret == -1)
    {
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_ops_result(dev, OPS_CAPTURE_TIMEOUT);
        bio_set_notify_abs_mid(dev, NOTIFY_CAPTURE_TIMEOUT);
    }
    bio_print_debug("bio_drv_fidoukey_ops_capture end \n");

    return feature_data;
}

/**
 * @brief 特征录入函数,用来录入用户的生物特征信息
 */
int bio_drv_fidoukey_ops_enroll(bio_dev *dev, OpsActions action, int uid, int idx, char *bio_idx_name)
{
    if (dev->enable == FALSE)
    {
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        bio_set_notify_mid(dev, NOTIFY_COMM_DISABLE);

        return -1;
    }
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;

    //检测已录入数量
    feature_info *info_list = NULL;
    sqlite3 *db1 = bio_sto_connect_db();
    info_list = bio_sto_get_feature_info(db1, uid, dev->bioinfo.biotype, dev->device_name, 0, -1);
    
    int feature_num = 0;
    feature_info *cursor = info_list;
    while (cursor != NULL)
    {
        cursor = cursor->next;
        feature_num++;

    }
	bio_sto_disconnect_db(db1);
    bio_sto_free_feature_info_list(info_list);
    
    bio_print_debug("feature_num : %d\n", feature_num);
    if (feature_num>=1)
    {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("Exceed the maximum enroll limit"));
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        bio_set_ops_abs_result(dev, MID_OPS_EXCEED_ENROOL_MAX);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }

    bio_set_dev_status(dev, DEVS_ENROLL_DOING);
    unsigned char *feature_data = NULL;
    unsigned char *template_data = NULL;
    feature_data = (unsigned char *)malloc(1024);
    memset(feature_data, 0, 1024);
    template_data = (unsigned char *)malloc(1024);
    memset(template_data, 0, 1024);
    unsigned char cdh[32];
    unsigned char userid[32];
    char  feature_encode[1024] ;
    char origin[1024];
    int r;
    fido_cred_t *cred = NULL;
    const uint8_t *ptr;
    const char *user = NULL;

    fido_dev_t *fidev = NULL;
    fidev = fido_dev_new();
    if (!fidev)
    {
        bio_print_debug("fido_dev_new failed\n");
        return 0;
    }
    priv->fidev=fidev;
    bio_print_debug("path %s\n",priv->path);
    r = fido_dev_open(fidev, priv->path);

    if ((cred = fido_cred_new()) == NULL)
    {
        printf("fido_cred_new failed");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    if ((r = fido_cred_set_type(cred, COSE_EDDSA)) != FIDO_OK)
    {
        printf("fido_cred_set_type: %s", fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    if (!random_bytes(cdh, sizeof(cdh)))
    {
        bio_print_error("random_bytes failed\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }

    if ((r = fido_cred_set_clientdata_hash(cred, cdh, sizeof(cdh))) != FIDO_OK)
    {
        bio_print_error("error: fido_cred_set_clientdata_hash (%d): %s\n", r,
                        fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }

    if ((r = fido_cred_set_rk(cred, FIDO_OPT_OMIT)) != FIDO_OK)
    {
        bio_print_error("fido_cred_set_rk: %s", fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    if ((r = fido_cred_set_user(cred, userid, sizeof(userid),
                                "test", "test", NULL)) != FIDO_OK)
    {
        bio_print_error("fido_cred_set_user: %s", fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }

    // if ((n = strlcpy(origin, PAM_PREFIX, sizeof(origin))) >= sizeof(origin))
    // {
    //     printf("error: strlcpy failed\n");
    //     goto err;
    // }
    //     if (gethostname(origin + n, sizeof(origin) - n) == -1)
    // {
    //     perror("gethostname");
    //     goto err;
    // }

    struct passwd* pwd;
    pwd=getpwuid(uid);


    bio_print_info("Setting origin to %s\n", pwd->pw_name);

    if ((r = fido_cred_set_rp(cred, pwd->pw_name, NULL)) != FIDO_OK)
    {
        bio_print_error("fido_cred_set_rp: %s", fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    if ((r = fido_cred_set_rk(cred, FIDO_OPT_OMIT)) != FIDO_OK)
    {
        bio_print_error("error: fido_cred_set_rk (%d) %s\n", r, fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }

    if ((r = fido_cred_set_uv(cred, FIDO_OPT_OMIT)) != FIDO_OK)
    {
        bio_print_error("error: fido_cred_set_uv (%d) %s\n", r, fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }

    if (priv->path == NULL || dev == NULL || cred == NULL)
    {
        bio_print_error("%s: args\n", __func__);
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }

    char prompt[1024];
    char pin[1024];

    r = fido_dev_make_cred(fidev, cred, NULL);
    if (r == FIDO_ERR_PIN_REQUIRED)
    {
    }
    if(r == FIDO_ERR_ACTION_TIMEOUT)
    {
        bio_set_ops_result(dev, OPS_ENROLL_TIMEOUT);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_TIMEOUT);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    explicit_bzero(pin, sizeof(pin));
    if (r != FIDO_OK)
    {
        bio_print_error("error: fido_dev_make_cred (%d) %s\n", r, fido_strerr(r));
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    bio_print_info("%d\n", r);
    r = fido_dev_close(fidev);
    if(r != FIDO_OK)
        bio_print_error("fido_dev_close(%d):%s\n",r,fido_strerr(r));
    fido_dev_free(&fidev);

    if (fido_cred_x5c_ptr(cred) != NULL)
    {
        if ((r = fido_cred_verify(cred)) != FIDO_OK)
        {
            bio_print_error("fido_cred_verify: %s",
                            fido_strerr(r));
            bio_set_ops_result(dev, OPS_ENROLL_FAIL);
            bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
            bio_set_dev_status(dev, DEVS_COMM_IDLE);
            return -1;
        }
    }
    else
    {
        bio_print_error("self-attested credential");
        if ((r = fido_cred_verify_self(cred)) != FIDO_OK)
        {
            bio_print_error("fido_cred_verify_self: %s",
                            fido_strerr(r));
            bio_set_ops_result(dev, OPS_ENROLL_FAIL);
            bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
            bio_set_dev_status(dev, DEVS_COMM_IDLE);
            return -1;
        }
    }
    size_t len;

    if ((ptr = fido_cred_id_ptr(cred)) != NULL)
    {
        len = fido_cred_id_len(cred);
    }
    if ((ptr = fido_cred_sig_ptr(cred)) != NULL)
    {
        len = fido_cred_sig_len(cred);
    }
    if ((ptr = fido_cred_x5c_ptr(cred)) != NULL)
    {
        len = fido_cred_x5c_len(cred);
        bio_print_debug("attestation cert len=%zu\n", len);
    }
    if ((ptr = fido_cred_authdata_ptr(cred)) != NULL)
    {
        len = fido_cred_authdata_len(cred);
        bio_print_debug("authdata len=%zu\n", len);
    }

    const unsigned char *kh = NULL;
    const unsigned char *pk = NULL;
    char b64_kh[256];
    char b64_pk[256];
    size_t kh_len;
    size_t pk_len;
    int ok = -1;

    if ((kh = fido_cred_id_ptr(cred)) == NULL)
    {
        printf("error: fido_cred_id_ptr returned NULL\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }
    bio_print_debug("fido_cred_id_ptr\n");
    if ((kh_len = fido_cred_id_len(cred)) == 0)
    {
        printf("error: fido_cred_id_len returned 0\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }
    bio_print_debug("fido_cred_id_len\n");
    if ((pk = fido_cred_pubkey_ptr(cred)) == NULL)
    {
        printf("error: fido_cred_pubkey_ptr returned NULL\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }

    if ((pk_len = fido_cred_pubkey_len(cred)) == 0)
    {
        printf("error: fido_cred_pubkey_len returned 0\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }
    bio_print_debug("pklen:%d,khlen:%d\n",pk_len,kh_len);
    if (!bio_base64_encode(kh, b64_kh, kh_len))
    {
        printf("error: failed to encode key handle\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }

    if (!bio_base64_encode(pk, b64_pk, pk_len))
    {
        printf("error: failed to encode public key\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }
     bio_print_debug("bio_base64_encode\n");
    if ((user = fido_cred_user_name(cred)) == NULL)
    {
        printf("error: fido_cred_user_name returned NULL\n");
        bio_set_ops_result(dev, OPS_ENROLL_FAIL);
        bio_set_notify_mid(dev, NOTIFY_ENROLL_FAIL);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
    }
    bio_print_debug("%s", user);

    bio_print_debug(":%s,%s,%s\n", b64_kh, b64_pk, cose_string(fido_cred_type(cred)));
    sprintf(feature_encode,"%s,%s",b64_kh, b64_pk);
    // 存储模板
    feature_info *info;
    info = bio_sto_new_feature_info(uid, dev->bioinfo.biotype, dev->device_name,
                                    idx, bio_idx_name);
    info->sample = bio_sto_new_feature_sample(-1, NULL);
    info->sample->no = 1;
    info->sample->data = bio_sto_new_str(feature_encode);

    print_feature_info(info);

    sqlite3 *db = bio_sto_connect_db();
    bio_sto_set_feature_info(db, info);
    bio_sto_disconnect_db(db);

    bio_sto_free_feature_info_list(info);

    // if (feature_data != NULL)
    //     free(feature_data);
    // if (template_data != NULL)
    //     free(template_data);
    // if (feature_encode != NULL)
    //     free(feature_encode);


    bio_set_ops_abs_result(dev, OPS_ENROLL_SUCCESS);
    bio_set_notify_abs_mid(dev, NOTIFY_ENROLL_SUCCESS);

    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    return 0;

}

/**
 * @brief 特征验证函数,使用当前生物特征与指定特征对比,判断是否是同一个特征
 */
int bio_drv_fidoukey_ops_verify(bio_dev *dev, OpsActions action, int uid, int idx)
{
    return 0;
}

/**
 * @brief 特征识别函数,使用当前生物特征与指定范围的特征比对,识别出当前特征与指定范围内的哪个特征匹配
 */
int bio_drv_fidoukey_ops_identify(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_fidoukey_ops_identify start\n");
    int found_uid = -1;
    int r;
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;
    fido_assert_t *assert = NULL;
    unsigned char cdh[32];
    if (dev->enable == FALSE)
    { //设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return -1;
    }
    struct timeval start_time, end_time;
    int diff_time = 0;
    gettimeofday(&start_time, NULL);
    bio_print_debug("Detect will end in %d ms\n", priv->timeoutMS);

    bio_set_dev_status(dev, OPS_TYPE_IDENTIFY); // 设备状态设置为：正在识别指定特征
    fido_dev_t *fidev = NULL;
    fidev = fido_dev_new();
    if (!fidev)
    {
        bio_print_debug("fido_dev_new failed\n");
        return 0;
    }
    bio_print_debug("path %s\n",priv->path);

    priv->fidev=fidev;

    r = fido_dev_open(fidev, priv->path);

    unsigned char *authdata = NULL;
    unsigned char *sig = NULL;
    size_t authdata_len;
    size_t sig_len;
    char *b64_authdata = NULL;
    char *b64_sig = NULL;

    const unsigned char *kh = NULL;
    const unsigned char *pk = NULL;
    char *b64_kh = NULL;
    char *b64_pk = NULL;
    char *b64_code=NULL;
    size_t kh_len;
    size_t pk_len;
    int ok = -1;
    char *origin;

    sqlite3 *db = bio_sto_connect_db();
    feature_info *info, *info_list, *found;
    feature_info found_head;
    unsigned char * template_data = NULL;

    int ret=0;
    int similarity = 0;
    int match = 0;

    template_data = (unsigned char *)malloc(1024);
    memset(template_data, 0, 1024);


    //提示消息，正在检测
    bio_set_notify_abs_mid(dev, MID_RECOGNIZING);

    // 从数据库中获取特征列表
    info_list = bio_sto_get_feature_info(db, uid, dev->bioinfo.biotype,
                                                 dev->device_name, idx_start,
                                                 idx_end);
    print_feature_info(info_list);
    bio_sto_disconnect_db(db);

    info = info_list;
    found_head.next = NULL;
    found = &found_head;
    bio_print_debug("b64_code\n");
    // 为匹配的特征构建新的特征链表，每个特征值对应一个采样特征值
    priv->ctrlFlag = CONTROL_FLAG_RUNNING;
    while (info != NULL)
    {
        bio_print_debug("b64_code1\n");
        feature_sample * sample = info->sample;
        b64_code=sample->data;
        origin=info->index_name;
        info = info->next;
    }
    bio_print_debug("b64_code : %s\n",b64_code);
    b64_kh=strtok(b64_code, ",");
    b64_pk=strtok(NULL, ",");
    bio_print_debug("b64_code : %s\n",b64_kh);
    bio_print_debug("b64_code : %s\n",b64_pk);
    if (!b64_decode(b64_kh, (void **) &kh, &kh_len))
    {
        bio_print_error("Unable to decode");
        return 0;
    }
    // if (!b64_decode(b64_pk, (void **) &pk, &pk_len))
    // {
    //     bio_print_error("Unable to decode");
    //     return 0;
    // }
    // bio_base64_decode(b64_kh,kh);
    // bio_base64_decode(b64_pk,pk);

    struct passwd* pwd;
    pwd=getpwuid(uid);


   if ((assert = fido_assert_new()) == NULL)
    {
        bio_print_error("Unable to allocate assertion");
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("Get assert error"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }
    r = fido_assert_set_rp(assert, pwd->pw_name);
    if (r != FIDO_OK)
    {
        bio_print_error("Unable to set rp: %s (%d)", fido_strerr(r), r);
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Get assert error"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }

    r = fido_assert_allow_cred(assert, kh, kh_len);
    if (r != FIDO_OK)
    {
        bio_print_error("Unable to set keyHandle: %s (%d)", fido_strerr(r), r);
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Get assert error"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }

    ok = 0;

    if (fido_assert_set_up(assert, FIDO_OPT_OMIT) != FIDO_OK)
    {
        bio_print_error("Failed to set UP");
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Get assert error"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }

    r = fido_assert_set_uv(assert, FIDO_OPT_OMIT);
    if (r != FIDO_OK)
    {
        bio_print_error("Failed to set UV");
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Get assert error"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }
    // r = fido_assert_set_extensions(assert, ext);
    // if (r != FIDO_OK)
    //     printf("fido_assert_set_extensions: %s (0x%x)", fido_strerr(r),r);

    // if (!random_bytes(cdh, sizeof(cdh)))
    // {
    //     bio_print_error("Failed to generate challenge");
    //     return 0;
    // }
    bio_print_debug("fido_assert_set_clientdata_hash");
    r = fido_assert_set_clientdata_hash(assert, cd, sizeof(cd));
    if (r != FIDO_OK)
    {
        bio_print_error("Unable to set challenge: %s (%d)", fido_strerr(r), r);
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Get assert error"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        return -1;
    }


    r = fido_dev_get_assert(fidev, assert, NULL);
    if (r == FIDO_OK)
    {
        fido_cbor_info_free(&info);
        if (fido_assert_count(assert) != 1)
            bio_print_error("fido_assert_count: %d signatures returned\n",
                   (int)fido_assert_count(assert));
        authdata = fido_assert_authdata_ptr(assert, 0);
        authdata_len = fido_assert_authdata_len(assert, 0);
        sig = fido_assert_sig_ptr(assert, 0);
        sig_len = fido_assert_sig_len(assert, 0);

        // b64_encode(authdata, authdata_len, &b64_authdata);
        // b64_encode(sig, sig_len, &b64_sig);
        bio_print_debug("%d,%d\n", authdata_len, sig_len);
    }
    else if (r == FIDO_ERR_ACTION_TIMEOUT)
    {
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    else if (r == FIDO_ERR_KEEPALIVE_CANCEL)
    {
        priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_STOP_BY_USER);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_STOP_BY_USER);
        // bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    else
    {
        bio_print_error("get assert fail: %s (%d)", fido_strerr(r), r);
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Verify cert title error "));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    r = fido_dev_close(fidev);
    if(r != FIDO_OK)
        bio_print_error("fido_dev_close(%d):%s\n",r,fido_strerr(r));
    fido_dev_free(&fidev);

    ret=verify_assert(COSE_EDDSA, authdata,authdata_len, sig,sig_len, FIDO_OPT_OMIT, FIDO_OPT_OMIT, 0, b64_pk, cdh,pwd->pw_name);

    if (ret == 0)
    {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_identify successful"));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return uid;
    }
    else
    {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Verify cert title error "));
        bio_set_ops_abs_result(dev, OPS_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_IDENTIFY_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return -1;
    }
    bio_set_dev_status(dev, DEVS_COMM_IDLE);
    bio_print_debug("bio_drv_fidoukey_ops_identify end\n");
    return uid;
}

/**
 * @brief 特征搜索函数,使用当前生物特征与指定范围的特征比对,搜索出指定范围内所有匹配的特征
 */
feature_info *bio_drv_fidoukey_ops_search(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_fidoukey_ops_search start\n");

    feature_info *found = NULL;

    if (dev->enable == FALSE)
    { // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return NULL;
    }
    bio_set_dev_status(dev, OPS_TYPE_SEARCH); // 设备状态设置为：正在搜索指定特征
    // 验证特征操作放在此处
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;
    bio_set_notify_abs_mid(dev, MID_DETECTING);
    int ret = 0;
    if (ret == -1)
    {
        bio_set_ops_abs_result(dev, OPS_SEARCH_TIMEOUT);
        bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_TIMEOUT);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return NULL;
    }
    if (priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {
        // priv->ctrlFlag = CONTROL_FLAG_STOPPED;
        bio_set_ops_result(dev, OPS_COMM_STOP_BY_USER);
        bio_set_notify_mid(dev, NOTIFY_COMM_STOP_BY_USER);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return NULL;
    }

    // 无需检测手指抬起，直接开始查找比对指纹
    bio_set_notify_abs_mid(dev, MID_RECOGNIZING);
    // found = fidoukey_internel_search(dev, feature_data, uid, idx_start, idx_end);
    //  状态被中断
    if (priv->ctrlFlag == CONTROL_FLAG_STOPPED)
    {
        bio_set_ops_result(dev, OPS_COMM_STOP_BY_USER);
        bio_set_notify_mid(dev, NOTIFY_COMM_STOP_BY_USER);
        bio_set_dev_status(dev, DEVS_COMM_IDLE);
        return NULL;
    }

    if (found != NULL)
    {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_search feature successful"));
        bio_set_ops_abs_result(dev, OPS_SEARCH_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
    }
    else
    {
        snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_search feature fail"));
        bio_set_ops_abs_result(dev, OPS_SEARCH_NO_MATCH);
        bio_set_notify_abs_mid(dev, NOTIFY_SEARCH_NO_MATCH);
        bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);
        bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));
    }
    bio_set_dev_status(dev, DEVS_COMM_IDLE);
    return found;
}

/**
 * @brief 特征清理(删除)函数,删除指定范围内的所有特征
 */
int bio_drv_fidoukey_ops_clean(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_fidoukey_ops_clean start\n");

    if (dev->enable == FALSE)
    { // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return 0;
    }
    bio_set_dev_status(dev, DEVS_CLEAN_DOING); // 设备状态设置为：正在清理特征数据
    /*
     * 如果特征清理删除有硬件操作，则放到此处，厂商可根据需要设置
     */
    int ret;

    ret = 0;

    sqlite3 *db;
    // 连接数据库
    db = bio_sto_connect_db();
    // 从数据库中删除特征
    ret = bio_sto_clean_feature_info(db, uid, dev->bioinfo.biotype,
                                             dev->device_name, idx_start,
                                             idx_end);
    // 断开数据库
    bio_sto_disconnect_db(db);


    if (ret == 0)
    {
        bio_set_ops_abs_result(dev, OPS_CLEAN_SUCCESS);    // 操作结果设置为：清理特征成功
        bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_SUCCESS); // 用户提示设置为：清理特征成功
    }
    else
    {
        bio_set_ops_result(dev, OPS_CLEAN_FAIL);        // 操作结果设置为：清理特征失败
        bio_set_notify_abs_mid(dev, NOTIFY_CLEAN_FAIL); // 用户提示设置为：清理特征失败
    }

    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    return ret;
}

/*
 * 获取指定设备的特征列表
 */
feature_info *bio_drv_fidoukey_ops_get_feature_list(bio_dev *dev, OpsActions action, int uid, int idx_start, int idx_end)
{
    bio_print_debug("bio_drv_fidoukey_ops_get_feature_list start\n");

    feature_info *found = NULL;
    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;

    if (dev->enable == FALSE)
    { // 设备不可用
        bio_set_dev_status(dev, DEVS_COMM_DISABLE);
        return NULL;
    }

    bio_set_dev_status(dev, OPS_TYPE_GET_FLIST); // 设备状态设置为：正在获取特征列表
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
    bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

    bio_print_info("%s\n", bio_get_notify_mid_mesg(dev));

    bio_set_dev_status(dev, DEVS_COMM_IDLE);

    return found;
}

/*
 * 中止设备的当前操作
 */
int bio_drv_fidoukey_ops_stop_by_user(bio_dev *dev, int waiting_ms)
{
    bio_print_debug("bio_drv_fidoukey_ops_stop_by_user start\n");

    bio_print_info(_("_Device %s[%d] received interrupt request\n"), dev->device_name, dev->driver_id);

    if (bio_get_dev_status(dev) == DEVS_COMM_IDLE) // 如果设备为空闲状态
        return 0;

    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;
    // int timeout = bio_get_ops_timeout_ms(); // 获取通用超时时间
    int timeout =20;
    int timeused = 0;

    if (waiting_ms < timeout)
        timeout = waiting_ms;

    fido_dev_cancel(priv->fidev);

    // 设置状态位，用于通知采样过程用户中断
    priv->ctrlFlag = CONTROL_FLAG_STOPING;
    snprintf(priv->extra_info, EXTRA_INFO_LENGTH, _("_Device %s[%d] received interrupt request\n"), dev->device_name, dev->driver_id);
    bio_set_notify_abs_mid(dev, MID_EXTENDED_MESSAGE);

    while ((priv->ctrlFlag != CONTROL_FLAG_STOPPED) && (priv->ctrlFlag != CONTROL_FLAG_DONE) && (priv->ctrlFlag != CONTROL_FLAG_IDLE) && (timeused < timeout))
    {

        // TODO
        timeused += 1000;
        usleep(1000 * 1000);
    }

    if ((priv->ctrlFlag == CONTROL_FLAG_STOPPED) || (priv->ctrlFlag == CONTROL_FLAG_DONE) || (priv->ctrlFlag == CONTROL_FLAG_IDLE))
    {
        return 0;
    }

    return -1;
}

/*
 * 获取设备状态的文本消息
 */
const char *bio_drv_fidoukey_ops_get_dev_status_mesg(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_get_dev_status_mesg end\n");
    return NULL;
}

/*
 * 获取操作结果的文本消息
 */
const char *bio_drv_fidoukey_ops_get_ops_result_mesg(bio_dev *dev)
{
    
    bio_print_debug("bio_drv_fidoukey_ops_get_ops_result_mesg end\n");
    switch (bio_get_ops_result(dev))
    {
    case MID_OPS_EXCEED_ENROOL_MAX:
        return _("Exceed the maximum enroll limit");
    default:
	return NULL;
	}
}

/*
 * 获取面向用户的提示消息
 */
const char *bio_drv_fidoukey_ops_get_notify_mid_mesg(bio_dev *dev)
{
    bio_print_debug("bio_drv_fidoukey_ops_get_notify_mid_mesg start\n");

    fidoukey_driver *priv = (fidoukey_driver *)dev->dev_priv;

    switch (bio_get_notify_mid(dev))
    {
    case MID_EXTENDED_MESSAGE:
        return priv->extra_info;

    case MID_DETECTING:
        return _("_Detecting PIN.Please wait...");
    case MID_RECOGNIZING:
        return _("_PIN is recognizing.Please wait...");

    default:
        return NULL;
    }
}
// 检测到设备连接时的回调函数（目前仅针对支持USB热插拔的设备）
void bio_drv_fidoukey_ops_attach(bio_dev *dev)
{
    // 驱动支持的USB设备拔出会调用此函数
    // 框架会自动调用ops_discover函数进行设备数量更新
}

// 检测到设备断开连接时的回调函数（目前仅针对支持USB热插拔的设备）
void bio_drv_fidoukey_ops_detach(bio_dev *dev)
{
    // 驱动支持的USB设备拔出会调用此函数
    // 框架会自动调用ops_discover函数进行设备数量更新
}
