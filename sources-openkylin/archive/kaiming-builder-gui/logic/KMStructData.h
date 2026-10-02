#ifndef KMSTRUCTDATA_H
#define KMSTRUCTDATA_H
#include <QString>
struct InstalledPackage
{
    bool isInstalled;
    QString p_name;
    QString p_version;
    QString p_arch;
    QString p_description;
};
#endif // KMSTRUCTDATA_H
