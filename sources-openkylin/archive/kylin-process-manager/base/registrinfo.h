#ifndef REGISTRINFO_H
#define REGISTRINFO_H

#include <string>
#include <vector>
#include <QMetaType>

struct RegistrWinInfo
{
    std::string title_;
    std::vector<std::string> state_;
};

struct RegistrProcessInfo
{
    std::string name_;
    std::string cmdline_;
};

struct RegistrAppInfo
{
    RegistrProcessInfo process_;
    RegistrWinInfo window_;
};

Q_DECLARE_METATYPE(RegistrWinInfo)
Q_DECLARE_METATYPE(RegistrProcessInfo)
Q_DECLARE_METATYPE(RegistrAppInfo)

#endif // !REGISTRINFO_H