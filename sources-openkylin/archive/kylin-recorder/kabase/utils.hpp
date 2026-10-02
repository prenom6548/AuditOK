#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <string>

namespace kabase
{

/* 应用名 */
enum AppName {
    KylinIpmsg = 0,          /* 传书 */
    KylinFontViewer,         /* 字体管理器 */
    KylinCalculator,         /* 麒麟计算器 */
    KylinGpuController,      /* 显卡控制器 */
    KylinMusic,              /* 音乐 */
    KylinWeather,            /* 天气 */
    KylinPhotoViewer,        /* 看图 */
    KylinServiceSupport,     /* 服务与支持 */
    KylinPrinter,            /* 麒麟打印 */
    KylinCalendar,           /* 日历 */
    KylinRecorder,           /* 录音 */
    KylinCamera,             /* 摄像头 */
    KylinNotebook,           /* 便签 */
    KylinOsManager,          /* 麒麟管家 */
    KylinNetworkCheck,       /* 网络检测工具 */
    KylinGallery,            /* 相册 */
    KylinScanner,            /* 扫描 */
    KylinMobileAssistant,    /* 多端协同 */
    KylinTest                /* 测试预留 */
};

class Utils
{
public:
    Utils() = default;
    ~Utils() = default;

    static std::string getAppName(AppName appName)
    {
        switch (appName) {
        case AppName::KylinCalculator:
            return "kylin-calaulator";
        case AppName::KylinCalendar:
            return "kylin-calendar";
        case AppName::KylinCamera:
            return "kylin-camera";
        case AppName::KylinFontViewer:
            return "kylin-font-viewer";
        case AppName::KylinGpuController:
            return "kylin-gpu-controller";
        case AppName::KylinIpmsg:
            return "kylin-ipmsg";
        case AppName::KylinMusic:
            return "kylin-music";
        case AppName::KylinPhotoViewer:
            return "kylin-photo-viewer";
        case AppName::KylinPrinter:
            return "kylin-printer";
        case AppName::KylinRecorder:
            return "kylin-recorder";
        case AppName::KylinServiceSupport:
            return "kylin-service-support";
        case AppName::KylinWeather:
            return "kylin-weather";
        case AppName::KylinNotebook:
            return "kylin-notebook";
        case AppName::KylinOsManager:
            return "kylin-os-manager";
        case AppName::KylinNetworkCheck:
            return "kylin-network-check-tools";
        case AppName::KylinGallery:
            return "kylin-gallery";
        case AppName::KylinScanner:
            return "kylin-scanner";
        case AppName::KylinMobileAssistant:
            return "kylin-mobile-assistant";
        default:
            return "";
        }

        /* 不应该被执行 */
        return "";
    };
};

}

#endif