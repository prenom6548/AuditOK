#ifndef KYLIN_OS_MANAGER_KOM_KOM_UTILS_H
#define KYLIN_OS_MANAGER_KOM_KOM_UTILS_H

#include <QString>

namespace kdk {
class QtSingleApplication;
}

namespace kom {

struct SystemVersion {
    QString os_version;           // 系统版本
    QString update_version;       // 补丁版本
    QString authorize_version;    // 授权版本
    QString build_id;             // 构建 id
};

class KomUtils {
public:
    enum WhiteListType {
        Service = 2,
        Application = 3
    };

    static void LogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static double adaptFontSize(double pixelSize = 14.0);
    static void setNetworkWhiteListFromPackage(WhiteListType p);
    /**
     * @brief 获取系统版本
     */
    static struct SystemVersion SystemVersion();
    /**
     * @brief 判断是否存在全屏窗口
     * @note 阻塞函数，内部会阻塞执行命令
     */
    static bool ExistsFullScreenWindow();
    /**
     * @brief 判断当前是否是 live 环境
     */
    static bool LiveEnv();
    /**
     * @brief 判断当前是否是 wayland 环境
     */
    static bool WaylandPlatform();
    /**
     * @brief 设置 wayland 环境下最小化拉起
     * @note 应该在 QtSingleApplication 实例化后，主界面实例化前调用
     */
    static void SetWaylandActivationWindow(kdk::QtSingleApplication &app);
};

}

#endif
