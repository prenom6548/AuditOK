#ifndef KYLIN_OS_MANAGER_KOM_UTILS_H
#define KYLIN_OS_MANAGER_KOM_UTILS_H
namespace kom
{

class KomUtils
{
public:
    static double adaptFontSize(double pixelSize = 14.0);
    enum WhiteListType{
        Service = 2,
        Application = 3
    };
    static void setNetworkWhiteListFromPackage(WhiteListType p);
};

} // namespace kom

#endif
