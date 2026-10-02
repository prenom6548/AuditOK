#ifndef GENERATEQRCODE_H
#define GENERATEQRCODE_H

#include <QPixmap>
#include <string>
#include "buriedpoint.hpp"

class GenerateTools
{
public:
    enum KEY_USERPASS {
        KEY_USER = 0,  // 用户名
        KEY_PASSWORD   // 密码
    };

    GenerateTools();

    static QPixmap getCode(QString info, int width = 170, int height = 170, bool *ok = nullptr);

    static void getUserPassword(QStringList &list);

    static std::string getUuid();

    static void delayMs(int ms);

    static void buriedPoint(kabase::BuriedPoint::PT pt);

    static QString getIptoCode(QString ip);

    static QString getCodetoIp(QString code);

    static bool isEnough(qint64 size);

    static QString intSizeToString(qint64 size);
};

#endif  // GENERATEQRCODE_H
