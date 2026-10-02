#ifndef KM_OVERLAY_H
#define KM_OVERLAY_H

#include <string>

class KMOverlay
{
public:
    /**
     * @brief 检查 overlay 配置文件
     */
    static void checkOverlayConfig();

    /**
     * @brief : 获取内核版本号
     * @param : [out] major 内核主版本号
     * @param : [out] minor 内核次版本号
     * @return: true 成功获取内核版本号，false 获取失败
     */
    static bool getKerVersion();

    /**
     * @brief : 判断是overlay是否支持rootless挂载
     * @return: true 支持，false 不支持
     */
    static bool isOverlayRootless();

    /**
     * @brief : 获取当前用户是否支持userxattr
     * @return: true 支持，false 不支持
     */
    static bool isOverlayUserXAttr();

    /**
     * @brief 检查内核配置文件中是否设置了 CONFIG_OVERLAY_FS_INDEX=y
     */
    static bool isOverlayFsIndexOff();

public:
    static int indexFlag;
    static int major;
    static int minor;
};

#endif // KM_OVERLAY_H
