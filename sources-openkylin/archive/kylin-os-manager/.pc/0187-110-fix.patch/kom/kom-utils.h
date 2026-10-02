#ifndef KYLIN_OS_MANAGER_KOM_KOM_UTILS_H
#define KYLIN_OS_MANAGER_KOM_KOM_UTILS_H

#include <QString>

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
};

}

#endif
