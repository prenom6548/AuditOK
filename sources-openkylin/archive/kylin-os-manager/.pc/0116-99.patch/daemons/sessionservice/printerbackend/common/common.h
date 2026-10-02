#ifndef KYLIN_PRINTER_COMMON_H
#define KYLIN_PRINTER_COMMON_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <map>

#define QTC(x) (qstringTochar(x))

enum CheckRes{
    CHECK_FALSE = 0,
    MAINWINDOW,
    POPWINDOW,
    DEBUG,
};

enum class PopWinStatus : int
{
    HIDE = 0,
    INSTALLING,
    INSTALL_SUCCESS,
    INSTALL_FAIL,
};

CheckRes checkCommand(const QStringList &command);

QString getRetFromCommand(const QStringList &command);

bool execCommand(const QStringList &command, QString *res, QString *standardError, int *exitCode, int *msec);

void runCommand(const QStringList &command);

char *qstringTochar(const QString &qstr);

QString getUserName();

bool isAdministratorUser();

const QMap<QString, QString> mfgConvertMap {
    {"Lenovo Image",    "Lenovo"},
    {"Fuji Xerox",      "Fuji-Xerox"},
    {"INDEX BRAILLE",   "INDEX-BRAILLE"},
    {"KONICA MINOLTA",  "KONICA-MINOLTA"},
    {"Hewlett-Packard", "HP"},
};

void specialDeviceCheck(QString &vendor, QString &model);

extern QString devicePathCheck(const QString &path);

extern bool ipIsValid(const QString &ipAddr);

extern bool printerNameIsLegal(const QString &printerName);

enum class CommonStatusFlag : int
{
    NONE = 0,
    SUCCESS,
    FAIL,

    ERROR_EXIST_SAME_PRINTER,
};

struct CommonStatus {
    CommonStatusFlag status{CommonStatusFlag::NONE};
    QString message{QString()};
};

extern bool processCheck(const QString &processName);

extern QString getPackageVersion(const QString &packageName);

extern QString getDebVersion(const QString &debName);

extern QString encodeUrl(const QString &in);

extern QString  decodeUrl(const QString &in);

extern void delay_msec(unsigned int msec);

#endif // KYLIN_PRINTER_COMMON_H
