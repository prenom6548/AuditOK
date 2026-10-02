#ifndef PLATFORMINFO_H
#define PLATFORMINFO_H
#include <QString>
#include <ukuisdk/kylin-com4c.h>
#include <ukuisdk/kylin-com4cxx.h>

namespace PLATFORM
{
// http://172.17.66.192/biz/task-view-43830.html  桌面操作系统版本信息统一接口参考此链接
//平台信息头文件，平台包括：主线V101、Intel、HW9X0
const QString V101 = "v10sp1";
const QString Intel = "v10sp1-edu";
// const QString HW9X0     = "HW9X0";

extern QString g_platformType;
} // namespace PLATFORM

#endif // PLATFORMINFO_H
