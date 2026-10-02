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

#ifndef UD650_H
#define UD650_H

#include <stdint.h>
#include <biometric_common.h>

#if	defined(__cplusplus)
extern	"C"	{
#endif	/* defined(__cplusplus) */

#if !defined NULL
	#define	NULL	0
#endif

static const struct usb_id ud650_id_table[] = {
	{ .idVendor = 0x200d, .idProduct = 0x7638, .description = "CDROM Mode" },
	{ .idVendor = 0x2109, .idProduct = 0x7638, .description = "HID Mode" },
	{ 0, 0, NULL },
};

typedef signed char			INT8, *PINT8;
typedef signed short		INT16, *PINT16;
typedef signed int			INT32, *PINT32;
typedef unsigned char		UINT8, *PUINT8;
typedef unsigned short		UINT16, *PUINT16;
typedef unsigned int		UINT32, *PUINT32;
typedef char 				*LPCSTR;
typedef void 				*HANDLE;
typedef HANDLE 				*PHANDLE;

/******************************通讯协议*****************************************/
#define XG_PREFIX_CODE					0xAABB //包标识

#define USB_DATA_PACKET_SIZE			4096-2
#define COM_DATA_PACKET_SIZE			512-2 //串口数据包最大为512字节
#define HID_DATA_PACKET_SIZE			(60*60)-2

/*设备相关指令*/
#define XG_CMD_CONNECTION				0x01 //连接设备,必须带8位以上密码，默认密码全为0(0x30)
#define XG_CMD_CLOSE_CONNECTION			0x02 //关闭连接
#define XG_CMD_GET_SYSTEM_INFO			0x03 //获取版本号和设置信息
#define XG_CMD_FACTORY_SETTING			0x04 //恢复出厂设置
#define XG_CMD_SET_DEVICE_ID			0x05 //设置设备编号0-255
#define XG_CMD_SET_BAUDRATE				0x06 //设置波特率0-4
#define XG_CMD_SET_SECURITYLEVEL		0x07 //设置安全等级0-4
#define XG_CMD_SET_TIMEOUT				0x08 //设置等待手指放入超时1-255秒
#define XG_CMD_SET_DUP_CHECK			0x09 //设置重复登录检查0-1
#define XG_CMD_SET_PASSWORD				0x0A //设置通信密码
#define XG_CMD_CHECK_PASSWORD			0x0B //检查密码是否正确
#define XG_CMD_REBOOT					0x0C //复位重启设备
#define XG_CMD_SET_SAME_FV				0x0D //登记的时候检查是否为同一根手指
#define XG_CMD_SET_USB_MODE				0x0E //设置USB驱动模式

/*识别相关指令*/
#define XG_CMD_FINGER_STATUS			0x10 //检测手指放置状态
#define XG_CMD_CLEAR_ENROLL				0x11 //清除指定ID登录数据
#define XG_CMD_CLEAR_ALL_ENROLL			0x12 //清除所有ID登录数据
#define XG_CMD_GET_EMPTY_ID				0x13 //获取空（无登录数据）ID
#define XG_CMD_GET_ENROLL_INFO			0x14 //获取总登录用户数和模板数
#define XG_CMD_GET_ID_INFO				0x15 //获取指定ID登录信息
#define XG_CMD_ENROLL					0x16 //指定ID登录
#define XG_CMD_VERIFY					0x17 //1:1认证或1:N识别
#define XG_CMD_IDENTIFY_FREE			0x18 //FREE识别模式，自动识别并发送状态
#define XG_CMD_CANCEL					0x19 //取消FREE识别模式
#define XG_CMD_RAM_MODE					0x1A //内存操作模式
#define XG_CMD_VERIFY_MULTI				0x1B //连续多个ID验证

/******************************高级指令****************************************/
/*数据读写相关指令*/
#define XG_CMD_READ_DATA				0x20 //从设备读取数据
#define XG_CMD_WRITE_DATA				0x21 //写入数据到设备
#define XG_CMD_READ_ENROLL				0x22 //读取指定ID登录数据
#define XG_CMD_WRITE_ENROLL				0x23 //写入（覆盖）指定ID登录数据
#define XG_CMD_GET_VEIN_IMAGE			0X24 //采集指静脉图像数据到主机
#define XG_CMD_WRITE_IMAGE				0x25 //写入图像数据
#define XG_CMD_GET_DEBUG				0x26 //读取调试信息
#define XG_CMD_UPGRADE					0x27 //写入升级程序，重启后自动升级
#define XG_CMD_GET_CHARA				0x28 //采集并读取特征到主机
#define XG_CMD_READ_USER_DATA			0x29 //从用户扩展存储区读取数据，最大4K
#define XG_CMD_WRITE_USER_DATA			0x2A //写入数据到用户扩展存储区，最大4K

/******************************扩展指令****************************************/
/*数据读写相关指令*/
#define XG_CMD_GET_DOORCTR				0x30 //获取门禁设置
#define XG_CMD_SET_DOORCTRL				0x31 //设置门禁设置
#define XG_CMD_OPEN_DOOR				0x32 //开门
#define XG_CMD_READ_LOG					0x33 //读取门禁出入日志
#define XG_CMD_SET_DEVNAME				0x34 //设置设备名称
#define XG_CMD_GET_DATETIME				0x35 //获取门禁实时时钟
#define XG_CMD_SET_DATETIME				0x36 //设置门禁实时时钟
#define XG_CMD_KEY_CTRL					0x37 //发送按键
#define XG_CMD_ENROLL_EXT				0x38 //扩展语音登记
#define XG_CMD_VERIFY_EXT				0x39 //扩展语音验证
#define XG_CMD_DEL_LOG					0x3a //删除门禁日志
#define XG_CMD_PLAY_VOICE				0x3b //播放语音
#define XG_CMD_GET_USER_NAME			0x3C //读取用户名称
#define XG_CMD_SET_USER_NAME			0x3D //修改写入用户名称
#define XG_CMD_GET_USER_CG				0x3E //读取用户卡号和组号
#define XG_CMD_SET_USER_CG				0x3F //修改写入用户卡号和组号
#define XG_CMD_DEL_USER_INFO			0x40 //情况所有用户信息
#define XG_CMD_GET_ADMIN_PWD			0x41 //读取管理员密码，一个管理员可设置一个密码
#define XG_CMD_SET_ADMIN_PWD			0x42 //修改写入管理员密码


/******************************错误代码****************************************/
#define	XG_ERR_SUCCESS					0x00 //操作成功
#define	XG_ERR_FAIL						0x01 //操作失败
#define XG_ERR_COM						0x02 //通讯错误
#define XG_ERR_DATA						0x03 //数据校验错误
#define XG_ERR_INVALID_PWD				0x04 //密码错误
#define XG_ERR_INVALID_PARAM			0x05 //参数错误
#define XG_ERR_INVALID_ID				0x06 //ID错误
#define XG_ERR_EMPTY_ID					0x07 //指定ID为空（无登录数据）
#define XG_ERR_NOT_ENOUGH				0x08 //无足够登录空间
#define XG_ERR_NO_SAME_FINGER			0x09 //不是同一根手指
#define XG_ERR_DUPLICATION_ID			0x0A //有相同登录ID
#define XG_ERR_TIME_OUT					0x0B //等待手指输入超时
#define XG_ERR_VERIFY					0x0C //认证失败
#define XG_ERR_NO_NULL_ID				0x0D //已无空ID
#define XG_ERR_BREAK_OFF				0x0E //操作中断
#define XG_ERR_NO_CONNECT				0x0F //未连接
#define XG_ERR_NO_SUPPORT				0x10 //不支持此操作
#define XG_ERR_NO_VEIN					0x11 //无静脉数据
#define XG_ERR_MEMORY					0x12 //内存不足
#define XG_ERR_NO_DEV					0x13 //设备不存在
#define XG_ERR_ADDRESS					0x14 //设备地址错误
#define XG_ERR_NO_FILE					0x15 //文件不存在
#define XG_ERR_LICENSE					0x80

/******************************状态代码****************************************/
#define XG_INPUT_FINGER					0x20 //请求放入手指
#define XG_RELEASE_FINGER				0x21 //请求拿开手指

#if	defined(__cplusplus)
}
#endif	/* defined(__cplusplus) */

/**
 * 声音提示
 */
#define XG_CMD_EXT_CTRL			0x4B	// 扩展控制

#define FV_V_ASYN				0		// 异步
#define FV_V_SYN				1		// 同步

#define	FV_V_INPUT_FINGE_A		23		// 请再放一次
#define	FV_V_INPUT_FINGE		27		// 请自然轻放手指

#define FV_V_REG_SUCCESS		0		// 登记成功
#define	FV_V_REG_FAIL			2		// 登记失败
#define	FV_V_VERIFY_SUCCESS		33		// 验证失败
#define	FV_V_VERIFY_FAIL		32		// 验证成功

#define PACKET_LEN				24

typedef int (*SendDataCallBack)(int total, int sent);

//指静脉模块设备操作相关函数
typedef struct _FUN_COM
{
	//获取版本号
	int (*XG_GetVeinLibVer) (char *pVer);

	//获取当前连接的USB指静脉设备的个数，USB驱动必须是HID模式
	int (*XG_DetectUsbDev) (void);

	//打开并连接指静脉设备
	int (*XG_OpenVeinDev) (char* pDev, int Baud, int Addr,
						   UINT8 Password[16], int Len, PHANDLE pHandle);

	//关闭指静脉设备
	int (*XG_CloseVeinDev) (UINT8 Addr, HANDLE Handle);

	//发送一个指令包
	int (*XG_SendPacket) (UINT8 Addr, UINT8 Cmd, UINT8 Encode, UINT8 Len,
						  UINT8* pData, HANDLE Handle);

	//接收一个指令包
	int (*XG_RecvPacket) (UINT8 Addr, UINT8* pData, HANDLE Handle);

	//写入数据
	int (*XG_WriteData) (UINT8 Addr, UINT8 Cmd, UINT8* pData, UINT32 size,
						 HANDLE Handle);

	//读取数据
	int (*XG_ReadData) (UINT8 Addr, UINT8 Cmd, UINT8* pData, UINT32 size,
						HANDLE Handle);

	//更新指静脉固件
	int (*XG_Upgrade) (int iAddr, const char* fname, HANDLE Handle);

	//写入指静脉登记数据
	int (*XG_WriteDevEnrollData) (int iAddr, UINT32 User, UINT8 *pBuf,
								  HANDLE Handle);

	//读取指静脉登记数据
	int (*XG_ReadDevEnrollData) (int iAddr, UINT32 User, UINT8 *pBuf,
								 UINT32 *pSize, HANDLE Handle);

	//获取数据包发送状态，固件升级时间较长用于进度条
	int (*XG_SetCallBack) (HANDLE Handle, SendDataCallBack pSendData);

	//从设备获取指静脉特征
	int	(*XG_GetVeinChara) (int iAddr, UINT8* pBuf, UINT32 *pSize,
							HANDLE Handle);

	//功能：检测是否有手指放入，如检测到手指放入返回值大于0
	int	(*XG_GetFingerStatus) (int iAddr, HANDLE Handle);
} FunCom_t, *pFunCom_t;


//指静脉算法库操作相关函数
typedef struct _FUN_VEIN
{
	//创建算法库实例
	int (*XGV_CreateVein) (PHANDLE pHandle, int UserNum);

	//销毁算法库实例
	int (*XGV_DestroyVein) (HANDLE hHandle);

	//登记
	int (*XGV_Enroll) (HANDLE hHandle, UINT32 User, UINT8* pBuf, UINT32 size,
					   UINT8 CheckSameFinger, UINT16* pQuality);

	//验证，一般1：N识别比对时使用
	int (*XGV_Verify) (HANDLE hHandle, UINT32* pUser, UINT32 Num, UINT8* pBuf,
					   UINT32 size, UINT8 Group, UINT16* pQuality);

	//1:1验证
	UINT8* (*XGV_CharaVerify) (HANDLE hHandle, UINT8* pEnroll, int EnrollSize,
							   UINT8* pChara, int CharaSize);

	//获取1:1验证成功后自学习后的登记数据，可替换原有的登记数据
	int (*XGV_GetCharaVerifyLearn) (HANDLE hHandle, UINT8* pBuf);

	//设置安全等级，1,2,3，安全等级越高误识率越低
	int (*XGV_SetSecurity) (HANDLE hHandle, UINT8 Level);

	//获取登记数据
	int (*XGV_GetEnrollData) (HANDLE hHandle, UINT32 User, UINT8* pData,
							  UINT32* pSize);

	//获取登记数据登记时候指定的用户ID
	int (*XGV_GetEnrollUserId) (HANDLE hHandle, UINT8* pData);

	//保存登记数据
	int (*XGV_SaveEnrollData) (HANDLE hHandle, UINT32 User, UINT8 Group,
							   UINT8* pData, UINT16 Size);

	//删除登记数据
	int (*XGV_DelEnrollData) (HANDLE hHandle, UINT32 User);

	//获取登记用户数
	int (*XGV_GetEnrollNum) (HANDLE hHandle, UINT32 *pMaxUser,
							 UINT32 *pMaxTempNum, UINT32 *pEnrollUser,
							 UINT32 *pEnrollTempNum);

	//获取空ID
	int (*XGV_GetEnptyID) (HANDLE hHandle, UINT32 *pEnptyId, UINT32 StartId,
						   UINT32 EndId);

	//获取指定用户登记的模板数
	int (*XGV_GetUserTempNum) (HANDLE hHandle, UINT32 User, UINT32 *pTempNum);
} FunVein_t, *pFunVein_t;

//指静脉算法库操作相关函数
typedef struct
{
	int	(*EncodeBase64)(const unsigned char* pSrc, char* pDst, int nSrcLen);
	int (*DecodeBase64)(const char* pSrc, unsigned char* pDst, int nSrcLen);
}B64_Fun_t;

/*
 * Global Data
 */

// 动态链接库位置
#define _STR(s)	#s
#define DEF2STR(s)	_STR(s)

#ifdef BIO_DRIVER_DIR
	#define UD650_DRIVER_DIR	DEF2STR(BIO_DRIVER_DIR)
	#define UD650_EXTRA_DRIVER	UD650_DRIVER_DIR "/extra/ud650-extra.so"
#else
	#define UD650_EXTRA_DRIVER	"/usr/lib/biometric-authentication/driver/extra/ud650-extra.so"
#endif

static char* ud650_ext_drv = UD650_EXTRA_DRIVER;

static void *hLibHandle = NULL;

// 动态链接库中方法
static FunCom_t m_ComFun;
static FunVein_t m_VeinFun;

// 句柄
static HANDLE m_hDevHandle;		// 指静脉设备文件句柄
static HANDLE m_hVeinHandle;	// 指静脉算法库文件句柄

// 采样次数
#define SAMPLE_TIMES	5

// 检测间隔，单位毫秒(ms)
#define INTERVAL	100

/**
 * 指静脉信息和注册模板buffer的大小
 */
#define FV_SIZE				4096
#define FV_TEMP_SIZE		8192
#define FV_BASE64_SIZE		FV_SIZE << 2
#define FV_TEMP_BASE64_SIZE	FV_TEMP_SIZE << 2
static unsigned char * fv_data[SAMPLE_TIMES];
static char * fv_base64[SAMPLE_TIMES];
static int fv_data_len[SAMPLE_TIMES];

// 设备密码
//static char Password[16] = "00000000";
#define PASSWORD	"00000000"
static unsigned char iDevAddress = 0;

//
static int UD650_USER_MAX = 1;
static int UD650_USER_INDEX = 1;
//static int UD650_GROUP_INDEX = 1;

typedef struct ud650_priv_t {
}ud650_priv;

//static ud650_priv priv;

//#define GET_FV_STATUS_FALL COMM_MAX + 1
//#define GET_FV_

enum ud650_notify_mid {
	GET_FV_S_S = NOTIFY_COMM_MAX + 1,
	GET_FV_ERROR,
	GET_FV_NEED_RELEASE,
	GET_FV_NEED_PUSH,
	GET_FV_NEED_PUSH_2,
	GET_FV_NEED_PUSH_3,
	GET_FV_NEED_PUSH_4,
	GET_FV_NEED_PUSH_5,

	ERR_OPEN_VEIN_DEV = NOTIFY_OPEN_MAX + 1,
	ERR_GET_VEIN_LIB_VER,
	ERR_CREATE_VEIN,
	ERR_SET_SECURITY,
	ERR_DEL_ENROLL_DATA,
	ERR_ENROLL_DATA,
	ERR_SAVE_ENROLL_DATA,
	ERR_GET_ENROLL_DATA,

};

/*
 * Prototype
 */

int ops_configure(bio_dev * dev, GKeyFile * conf);
int ud650_ops_driver_init(bio_dev *dev);
int ud650_ops_discover(bio_dev *dev);
int ud650_ops_open(bio_dev *dev);
int ud650_ops_enroll(bio_dev *dev, OpsActions action, int uid, int bio_idx,
					 char * bio_idx_name);
static int getFingerVeinData(bio_dev *dev, unsigned int sample_times,
							 unsigned int interval);
char * ud650_ops_capture(bio_dev *dev, OpsActions action);
int ud650_fv_find(bio_dev *dev, int uid, int idx_start, int idx_end,
				  feature_info ** found);
int ud650_ops_verify(bio_dev *dev, OpsActions action, int uid, int idx);
int ud650_ops_identify(bio_dev *dev, OpsActions action, int uid,
					   int idx_start, int idx_end);
feature_info * ud650_ops_search(bio_dev *dev, OpsActions action, int uid,
								int idx_start, int idx_end);
int ud650_ops_clean(bio_dev *dev, OpsActions action, int uid,
					int idx_start, int idx_end);
feature_info * ud650_ops_get_feature_list(bio_dev *dev, OpsActions action,
										  int uid, int idx_start, int idx_end);
void ud650_ops_attach(bio_dev *dev);
void ud650_ops_detach(bio_dev *dev);
int ud650_ops_stop_by_user(bio_dev * dev, int waiting_ms);
void ud650_ops_close(bio_dev *dev);
void ud650_ops_free(bio_dev *dev);
void ud650_empty_func(void);

const char * ud650_ops_get_dev_status_mesg(bio_dev *dev);
const char * ud650_ops_get_ops_result_mesg(bio_dev *dev);
const char * ud650_ops_get_notify_mid_mesg(bio_dev *dev);


#endif // UD650_H

