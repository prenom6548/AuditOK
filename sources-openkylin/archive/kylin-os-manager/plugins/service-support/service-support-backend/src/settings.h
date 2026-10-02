#ifndef SETTINGS_H
#define SETTINGS_H

#include <tuple>
#include <QMap>
#include "informationclassitem.h"

class Settings
{
public:
    Settings() {}
    static InformationClassItemList getSettings();

    /* 协议，域名，端口 */
    static std::tuple<QString, QString, QString> getUrlInformation(void);
    static bool isUpload(void);
    static QString getExportPath(void);
    static void setExportPath(const QString &path);
    static void setHistoryBug(QString bugId);
    static QString getHistoryBug();
    static QString getSystemDeviceInfo();

private:
    static void creatJson();
    static void saveFile();
    static void creatMap();
    static QString csvFilePath();
    static QString getMachineId();
    static QByteArray getPr();
    static QByteArray encrypto(const QByteArray &msg);
    static QByteArray encryptRSA(const QByteArray &text);
    static QByteArray decryptRSA(const QByteArray &text);
    static QString m_filePath;
    static QByteArray m_jsonByte;
    static QMap<QString, QString> m_logItemClassMap;
    static QByteArray m_keycode;
    static QString m_machineId;
};

#endif // SETTINGS_H
