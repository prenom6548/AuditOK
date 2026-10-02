#ifndef SETTINGS_H
#define SETTINGS_H

#include <tuple>
#include <QMap>
#include "informationclassitem.h"
#include "informationitem.h"

class Settings {
public:
    enum Customize {
        NONE = 0,
        DEFAULT,
        ICBC,
        INTERNAL,
        OPENKYLIN
    };

    static InformationClassItemList getSettings();
    /* 协议，域名，端口 */
    static std::tuple<QString, QString, QString> getUrlInformation(void);
    static std::tuple<QString, QString, QString> getRvisitInformation(void);
    static void setRvisitInformation(const QString &jobNum, const QString &email, const QString &contact);
    static bool isUpload(void);
    static QString getExportPath(void);
    static void setExportPath(const QString &path);
    static void setHistoryBug(QString bugId);
    static QString getHistoryBug();
    static void setZenTaoPwd(const QString &pwd);
    static QString getZenTaoPwd();
    static bool getHistoryEnable();
    static QByteArray getKeyCode();
    static Customize getCustomize();
    static QString getSystemDeviceInfo();
    static QString getServiceNumber();
    static QMap<int,InformationItem*> getInfoItemMap();
    static QString getMilestone();

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

    static QString                    m_filePath;
    static QByteArray                 m_jsonByte;
    static QMap<QString, QString>     m_logItemClassMap;
    static QByteArray                 m_keycode;
    static Customize                  m_customize;
    static QString                    m_machineId;
    static QByteArray                 m_pu;
    static QByteArray                 m_pr1;
    static QByteArray                 m_pr2;
    static QMap<int,InformationItem*> m_infoItemMap;
    static QString                    m_serviceNumber;
    static QString                    m_milestone;
};

#endif
