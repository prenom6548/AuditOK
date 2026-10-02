/* Copyright (C) 2020, KylinSoft Co., Ltd.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/*
 * BuriedPoint 类使用了 kysdk-log 和 kysdk-diagnostics 库，调用方需要链接 kysdk-log 和 kysdk-diagnostics 库
 */

#ifndef BURIEDPOINT_HPP_
#define BURIEDPOINT_HPP_

#include <sstream>
#include <iostream>
#include <string.h>
#include <map>

#include <libkydiagnostics.h>
#include <libkylog.h>

#include "utils.hpp"

namespace kabase
{

class BuriedPoint
{
public:
    BuriedPoint() = default;
    ~BuriedPoint() = default;

    /* 埋点类型 */
    enum BuriedPointType {
        FunctionType = 0, /* 功能性打点 */
        PerformanceType,  /* 性能型打点 */
        StabilityType,    /* 稳定型打点 */
        TestType          /* 测试预留 */
    };

    /* 点位 */
    /* 规则说明 */
    /* 0x 00       0       000
     *    |        |        |
     * 应用标识   类型标识   点位
     */
    enum PT {
        KylinIpmsgNicknameModify = 0x010001,        /* 昵称修改 */
        KylinIpmsgOpenSaveDir = 0x010002,           /* 打开文件保存目录(主界面入口) */
        KylinIpmsgMainSearch = 0x010003,            /* 主界面搜索 */
        KylinIpmsgHistorySearch = 0x010004,         /* 历史记录搜索 */
        KylinIpmsgSetTop = 0x010005,                /* 设置置顶 */
        KylinIpmsgModifyFriendNotes = 0x010006,     /* 修改好友备注 */
        KylinIpmsgViewInformation = 0x010007,       /* 查看资料 */
        KylinIpmsgChangeDir = 0x010008,             /* 更改目录 */
        KylinIpmsgCleanCache = 0x010009,            /* 清空缓存 */
        KylinIpmsgClearChatRecord = 0x010010,       /* 清空聊天记录 */
        KylinIpmsgClearSingleChatRecord = 0x010011, /* 清空单人聊天记录 */
        KylinIpmsgDeleteRecord = 0x010012,          /* 删除记录 */
        KylinIpmsgBatchDelete = 0x010013,           /* 批量删除 */
        KylinIpmsgSendMessage = 0x010014,           /* 发送消息 */
        KylinIpmsgSendFiles = 0x010015,             /* 发送文件 */
        KylinIpmsgSendDir = 0x010016,               /* 发送目录 */
        KylinIpmsgSendScreenshot = 0x010017,        /* 发送截屏 */
        KylinIpmsgResend = 0x010018,                /* 重新发送 */
        KylinIpmsgCopy = 0x010019,                  /* 复制 */
        KylinIpmsgOpen = 0x010020,                  /* 打开 */
        KylinIpmsgOpenDir = 0x010021,               /* 打开目录 */
        KylinIpmsgSaveAs = 0x010022,                /* 另存为 */
        KylinIpmsgTray = 0x010023,                  /* 托盘 */

        KylinFontViewInstallFont = 0x020001,          /* 安装字体 */
        KylinFontViewUninstallFont = 0x020002,        /* 卸载字体 */
        KylinFontViewSearchFont = 0x020003,           /* 搜索字体 */
        KylinFontViewApplyFont = 0x020004,            /* 应用字体 */
        KylinFontViewExportFont = 0x020005,           /* 导出字体 */
        KylinFontViewCollectionFont = 0x020006,       /* 收藏字体 */
        KylinFontViewCancelCollectionFont = 0x020007, /* 取消收藏字体 */
        KylinFontViewFontInformation = 0x020008,      /* 字体信息 */
        KylinFontViewCopywritingChange = 0x020009,    /* 文案更改 */
        KylinFontViewFontSizeChange = 0x020010,       /* 预览字号调整 */

        KylinCalaulatorStandardMode = 0x030001,   /* 标准模式 */
        KylinCalaulatorScientificMode = 0x030002, /* 科学模式 */
        KylinCalaulatorRateMode = 0x030003,       /* 汇率模式 */
        KylinCalaulatorProgrammerMode = 0x030004, /* 程序员模式 */

        KylinWeatherCollection = 0x040001, /* 收藏 */
        KylinWeatherChangeCity = 0x040002, /* 切换城市 */
        KylinWeatherCitySearch = 0x040003, /* 城市搜索 */
        KylinWeatherFullScreen = 0x040004, /* 全屏 */

        KylinPhotoViewerOpenPicture = 0x050001,           /* 打开图片 */
        KylinPhotoViewerSwitchPicture = 0x050002,         /* 切换图片 */
        KylinPhotoViewerFlip = 0x050003,                  /* 翻转 */
        KylinPhotoViewerRotate = 0x050004,                /* 旋转 */
        KylinPhotoViewerAddPicture = 0x050005,            /* 添加图片 */
        KylinPhotoViewerZoomInandOut = 0x050006,          /* 放大缩小 */
        KylinPhotoViewerPictureDetails = 0x050007,        /* 图片详情 */
        KylinPhotoViewerCutting = 0x050008,               /* 裁剪 */
        KylinPhotoViewerCoyp = 0x050009,                  /* 复制 */
        KylinPhotoViewerPrint = 0x050010,                 /* 打印 */
        KylinPhotoViewerDelete = 0x050011,                /* 删除 */
        KylinPhotoViewerSaveAs = 0x050012,                /* 另存为 */
        KylinPhotoViewerRename = 0x050013,                /* 重命名 */
        KylinPhotoViewerSetAsDesktopWallpaper = 0x050014, /* 设置为桌面壁纸 */
        KylinPhotoViewerShowInFolder = 0x050015,          /* 在文件夹中显示 */
        KylinPhotoViewerPicturePreview = 0x050016,        /* 图片预览 */
        KylinPhotoViewerCurrentPointZoom = 0x050017,      /* 当前点缩放 */

        KylinServiceSupportFileUpload = 0x060001,           /* 文件上传 */
        KylinServiceSupportVideoape = 0x060002,             /* 录像 */
        KylinServiceSupportFileCopy = 0x060003,             /* 文件拷贝 */
        KylinServiceSupportJumpOfficicalWebsite = 0x060004, /* 跳转官网主页 */
        KylinServiceSupportJumpOnlineService = 0x060005,    /* 跳转在线客服 */

        KylinPrinterManuallyInstallPrinter = 0x070001, /* 手动安装打印机 */
        KylinPrinterSetResolution = 0x070002,          /* 设置分辨率 */
        KylinPrinterSetPaperSize = 0x070003,           /* 设置纸张大小 */
        KylinPrinterSetPaperType = 0x070004,           /* 设置纸张类型 */
        KylinPrinterSetCationSource = 0x070005,        /* 设置纸盒来源 */
        KylinPrinterSetDuplexPrint = 0x070006,         /* 设置双面打印 */
        KylinPrinterSetInkType = 0x070007,             /* 设置墨水类型 */
        KylinPrinterADDRemovePrinter = 0x070008,       /* 添加删除打印机 */
        KylinPrinterSetShareStartup = 0x070009,        /* 设置共享启动 */
        KylinPrintTestPage = 0x070010,                 /* 打印测试页 */
        KylinPrinterCancelPrintJob = 0x070011,         /* 取消打印任务 */
        KylinPrinterDeletePrintJob = 0x070012,         /* 删除打印任务 */
        KylinPrinterRePrint = 0x070013,                /* 重新打印 */
        KylinPrinterManualyModifyDrive = 0x070014,     /* 手动修改驱动 */
        KylinPrinterRename = 0x070015,                 /* 重命名 */

        KylinRecorderRecording = 0x080001,        /* 录音 */
        KylinRecorderPlayPause = 0x080002,        /* 播放暂停 */
        KylinRecorderClip = 0x080003,             /* 剪辑 */
        KylinRecorderSign = 0x080004,             /* 标记 */
        KylinRecorderDelete = 0x080005,           /* 删除 */
        KylinRecorderBluetoothRecord = 0x080006,  /* 蓝牙录音 */
        KylinRecorderSaveAs = 0x080007,           /* 另存为 */
        KylinRecorderOpenFileLocation = 0x080008, /* 打开文件位置 */
        KylinRecorderTopicSwitch = 0x080009,      /* 主题切换 */

        KylinCameraPreviewMonitor = 0x090001,         /* 预览画面 */
        KylinCameraSingleShot = 0x090002,             /* 单拍 */
        KylinCameraContinuousShot = 0x090003,         /* 连拍 */
        KylinCameraDelay = 0x090004,                  /* 延时拍照 */
        KylinCameraVideotape = 0x090005,              /* 录像 */
        KylinCameraCameraSelection = 0x090006,        /* 摄像头选用 */
        KylinCameraResolutionSelection = 0x090007,    /* 分辨率选用 */
        KylinCameraVideoFormatSelection = 0x090008,   /* 视频格式选用 */
        KylinCameraPictureFormatSelection = 0x090009, /* 图片格式选用 */
        KylinCameraChangeSavePath = 0x090010,         /* 更改存储路径 */
        KylinCameraThumbnail = 0x090011,              /* 缩略图 */
        KylinCameraGridLine = 0x090012,               /* 网格线 */
        KylinCameraMirrorFun = 0x090013,              /* 镜像功能 */

        KylinNotebookOrderList = 0x100001,      /* 有序列表 */
        KylinNotebookUnorderList = 0x100002,    /* 无序列表 */
        KylinNotebookBold = 0x100003,           /* 加粗 */
        KylinNotebookItalics = 0x100004,        /* 斜体 */
        KylinNotebookUnderline = 0x100005,      /* 下划线 */
        KylinNotebookDeleteline = 0x100006,     /* 删除线 */
        KylinNotebookFontSize = 0x100007,       /* 字号 */
        KylinNotebookFontColor = 0x100008,      /* 字体颜色 */
        KylinNotebookInsertPicture = 0x100009,  /* 插入图片 */
        KylinNotebookInterfaceColor = 0x100010, /* 界面配色 */
        KylinNotebookDeleteCurrent = 0x100011,  /* 删除当前 */
        KylinNotebookUiTop = 0x100012,          /* UI 置顶 */
        KylinNotebookListMode = 0x100013,       /* 列表模式 */
        KylinNotebookIconMode = 0x100014,       /* 图标模式 */
        KylinNotebookNewNote = 0x100015,        /* 新建便签 */
        KylinNotebookSearch = 0x100016,         /* 搜索 */
        KylinNotebookDelete = 0x100017,         /* 删除 */
        KylinNotebookModeChange = 0x100018,     /* 模式切换 */

        KylinOsManagerGarbageClean = 0x110001,    /* 垃圾清理 */
        KylinOsManagerFileShredding = 0x110002,   /* 文件粉碎 */

        KylinGpuControllerBaseInfo = 0x120001,     /* 基本信息 */
        KylinGpuControllerRunState = 0x120002,     /* 运行状态 */
        KylinGpuControllerDriveInfo = 0x120003,    /* 驱动信息 */
        KylinGpuControllerSwitch = 0x120003,       /* 显卡切换 */

        KylinNetworkCheckStartCheck = 0x130001,    /* 开始检测 */

        KylinGallerySwitchFolder = 0x140001,    /* 切换目录 */
        KylinGalleryOpenViewer = 0x140002,      /* 打开麒麟看图 */

        KylinMobileAssistantAndroidConn = 0x150001,        /* 安卓链接 */
        KylinMobileAssistantPcConn = 0x150002,             /* PC链接 */
        KylinMobileAssistantUsbConn = 0x150003,            /* Usb链接 */
        KylinMobileAssistantWifiConn = 0x150004,           /* Wifi链接 */
        KylinMobileAssistantDeviceDiscovery = 0x150005,    /* 设备发现 */
        KylinMobileAssistantDisconnect = 0x150006,         /* 断开链接 */
        KylinMobileAssistantMobileScreen = 0x150007,       /* 手机投屏 */
        KylinMobileAssistantPcScreen = 0x150008,           /* PC 投屏 */
        KylinMobileAssistantPictureList = 0x150009,        /* 图片列表 */
        KylinMobileAssistantVideoList = 0x150010,          /* 视频列表 */
        KylinMobileAssistantAudioList = 0x150011,          /* 音频列表 */
        KylinMobileAssistantDocList = 0x150012,            /* 文档列表 */
        KylinMobileAssistantQQFileList = 0x150013,         /* QQ 文件列表 */
        KylinMobileAssistantWechatFileList = 0x150014,     /* 微信文件列表 */
        KylinMobileAssistantMobileStorage = 0x150015,      /* 手机存储 */
        KylinMobileAssistantSwitchView = 0x150016,         /* 切换视图 */
        KylinMobileAssistantRefreshList = 0x150017,        /* 刷新列表 */
        KylinMobileAssistantUploadFile = 0x150018,         /* 上传文件 */
        KylinMobileAssistantDownloadFile = 0x150019,       /* 下载文件 */
        KylinMobileAssistantOpenFile = 0x150020,           /* 打开文件 */

        KylinScannerOneClickBeautification = 0x160001,    /* 一键美化 */
        KylinScannerRectification = 0x160002,             /* 自动纠偏 */
        KylinScannerTextRecognition = 0x160003,           /* 文本识别 */
        KylinScannerCutting = 0x160004,                   /* 裁剪 */
        KylinScannerRotate = 0x160005,                    /* 旋转 */
        KylinScannerImage = 0x160006,                     /* 镜像 */
        KylinScannerAddWatermark = 0x160007,              /* 加水印 */
        KylinScannerSendMail = 0x160008,                  /* 发送邮件 */
        KylinScannerSaveAs = 0x160009,                    /* 另存为 */
        KylinScannerSinglePageScan = 0x160010,            /* 单页扫描 */
        KylinScannerMultiPageScan = 0x160011,             /* 多页扫描 */

        KylinCalendarMonthDetails = 0x170001,    /* 查看月详情 */
        KylinCalendarMonthSwitch = 0x170002,     /* 月切换 */
        KylinCalendarDoday = 0x170003,           /* 定位到今天 */

        TestFunPoint = 0x999999 /* 测试预留 */
    };

    static bool functionBuriedPoint(AppName packageName, PT point)
    {
        static char *messageType = (char *)"FunctionType";
        char *appName = strdup(Utils::getAppName(packageName).c_str());

        std::string hex;
        std::stringstream ss;

        ss << std::hex << point;
        ss >> hex;

        KBuriedPoint pt;
        pt.key = "FunctionName";
        pt.value = hex.c_str();

        if (kdk_buried_point(appName, messageType, &pt, 1)) {
            free(appName);
            klog_err("kabase: buried point fail !\n");
            return false;
        }

        free(appName);

        return true;
    };

    static bool buriedPoint(AppName packageName, BuriedPointType buriedPointType, std::map<std::string, std::string> data)
    {
        char *appName = strdup(Utils::getAppName(packageName).c_str());

        char type[128];
        memset(type, '\0', sizeof(type));
        switch (buriedPointType) {
        case BuriedPointType::FunctionType:
            strcpy(type, "FunctionType");
            break;
        case BuriedPointType::PerformanceType:
            strcpy(type, "PerformanceType");
            break;
        case BuriedPointType::StabilityType:
            strcpy(type, "StabilityType");
            break;
        default:
            break;
        }

        if (!strlen(type)) {
            klog_err("kabase: buried point type is empty !\n");
            return false;
        }

        int size = data.size();
        if (size == 0) {
            klog_err("kabase: buried point data is empty !\n");
            return false;
        }

        KBuriedPoint *pt = (KBuriedPoint *)malloc(sizeof(KBuriedPoint) * size);
        {
            int i;
            std::map<std::string, std::string>::iterator it;
            for (i = 0, it = data.begin(); it != data.end() && i < size; it++, i++) {
                pt[i].key = it->first.c_str();
                pt[i].value = it->second.c_str();
            }
        }

        if (kdk_buried_point(appName, type, pt, size)) {
            free(appName);
            free(pt);
            klog_err("kabase: buried point fail !\n");
            return false;
        }

        free(appName);
        free(pt);

        return true;
    };
};

}

#endif
