#ifndef APPLICATION_INFO_H
#define APPLICATION_INFO_H

#include <QString>
#include <QtGlobal>

namespace UkuiSearch {

class ApplicationInfo
{
public:
    ApplicationInfo() = default;
    ~ApplicationInfo() = default;

    void tranPidToDesktopFp(const uint &pid, QString &desktop)
    {
        desktop = QStringLiteral("imitate-desktop-%1").arg(pid);
    }
};

} // namespace UkuiSearch

#endif // APPLICATION_INFO_H
