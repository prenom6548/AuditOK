#include "settings.h"
#include <unistd.h>
#include <QFile>
#include <QMap>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <QSettings>
#include <QMessageAuthenticationCode>
#include "kom-configure.h"
#include "informationitem.h"
#include "../constant.h"

#define KEY_CLASS "class"
#define KEY_ITEM "item"
#define KEY_NAME "name"
#define KEY_INFO_NAME "itemName"
#define KEY_INFO_FILE "itemFile"
#define KEY_INFO_CMD "itemCmd"
#define KEY_INFO_OTHER "itemOther"
#define KEY_INFO_TYPES "itemTypes"

#define CONFIG_PLUGIN_SYSTEM_PATH "/etc/kylin-os-manager/kylin-os-manager-plugin.ini"
#define CONFIG_PLUGIN_USER_PATH ".kylin-os-manager/kylin-os-manager-plugin.ini"

#define SYSTEM_INFOMATION_FILE_PATH "/etc/kylin-os-manager/systemInformation.txt"
#define SYSTEM_KYINFO_FILE_PATH "/etc/.kyinfo"

#ifndef DEBUG_MODE
#include "../../conf/log-collection.h"
#endif

QString Settings::m_filePath = Settings::csvFilePath();
QByteArray Settings::m_jsonByte;
QMap<QString, QString> Settings::m_logItemClassMap;
QByteArray Settings::m_keycode = "Le++PWfQCCD~xyrfdLBLNrg6ekC2w8hcFpHKd)wVg4mPFXxXQof6RYWaGpP%ODwYnxzY885%(O7vf73twh^f!"
                                 "d7Kt@O(WhC3YZf4pdG~!)Y^dT_Xvj@"
                                 "kMLkc57wLnJvdvFg3dQmq@7X6r44V8M6Cf~";
QByteArray Settings::m_pu =
    "2d2d2d2d2d424547494e205055424c4943204b45592d2d2d2d2d0a4d494942496a414e42676b71686b6947397730424151454641414f434151"
    "38414d49494243674b43415145417376326b4d6a416432454565486b79625630654c0a456e576871736c4e4c46624e4149547551716a6a6a76"
    "34532f5056316179334672324a57554c47747347563139584834796572723146396d73325859365666310a4c574c674d645377516554706743"
    "52614432574141567533313941592f3057504d4a6652304542776736787230783556675254744e3937714b4e443268727a2b0a59776d326261"
    "476f484a6a6564616c704b4e6d3462544f574c6c57566e334e37337158656a476f3242517557735077473137464c3168474979467433495a4c"
    "390a4b53784b33434d6e4d372b437458724a4d65796a6736496b532f34447344746f482b5062784f666e453473633650716f75594e49445943"
    "4739702b444e4a32700a735130306b6b514c48344152363968394758687673434e7858614c41564f6a345444345158432b3477764e2f55546d"
    "5a6f6a506f5146613762336a54676761530a74514944415141420a2d2d2d2d2d454e44205055424c4943204b45592d2d2d2d2d0a";
QByteArray Settings::m_pr1 =
    "2d2d2d2d2d424547494e205253412050524956415445204b45592d2d2d2d2d0a4d4949456f77494241414b43415145417376326b4d6a416432"
    "454565486b79625630654c456e576871736c4e4c46624e4149547551716a6a6a7634532f5056310a6179334672324a57554c47747347563139"
    "584834796572723146396d73325859365666314c574c674d64537751655470674352614432574141567533313941590a2f3057504d4a665230"
    "4542776736787230783556675254744e3937714b4e443268727a2b59776d326261476f484a6a6564616c704b4e6d3462544f574c6c57560a6e"
    "334e37337158656a476f3242517557735077473137464c3168474979467433495a4c394b53784b33434d6e4d372b437458724a4d65796a6736"
    "496b532f34440a7344746f482b5062784f666e453473633650716f75594e494459434739702b444e4a3270735130306b6b514c483441523639"
    "68394758687673434e7858614c410a564f6a345444345158432b3477764e2f55546d5a6f6a506f5146613762336a5467676153745149444151"
    "4142416f49424145514c3332646847596545516748380a31375758736b786a516748656f364d756f4b4d49356c656251312b7951642b2f4a56"
    "4f49784458483663366c7263563433523568704e61476f2f394b415935450a4d76474c43642b6d546c7444593371356e76656a576b57754c62"
    "7257646f3771517761524744322b4366726b394b46715a4665506d61774947364c3438515a780a597265596c424673754e6a323879566b4448"
    "616f344279796f4d574b693146563845764a4d726c57543933496f787578654e31544169727a7247586e4b582b550a62577457576f5557742b"
    "627769756a58565832426a665945454c3866586165395550446f4c6342722b615769625869544b6b57634e444d50456c334c364d43340a4371"
    "73326b79557a757a7a53526c4245584e797436444b74757a47615342663975566451717352437a64767231447a746e553132715541474e786b"
    "59703730610a6b6c39597847554367594541324a353958447a6e6477525362416a6869752f66794e6a2b306e786965726956396f656b71544b"
    "7958382f4a34704c686a496b390a34324c5373367648646a6777777";
QByteArray Settings::m_pr2 =
    "7e534a1e616154672175254d4a4e45505d787478794005025c5e7606460e0e5720447f7d511d426056005f65766c1a6f615c5505626a6f5475"
    "47621277734e6f0b4e486c0f0d0313107c0e4350035140415f6f524057027f4174781e615d76006a6955014154264b181a6c6a53616e6f4358"
    "225f7b7e0d500602437a5b7c1451407250065465544478016a02460307625a790f7507497c51191c626051647571224a1c4d4a50007e207b7f"
    "465201515f7101400e59507745297e051f446002025b66726e496c605b0300606a6157744368102e70136d5d4e4d6c010c0011197c07455201"
    "054345583f501952567d45762c1f625c20046a6e070741527248451f6a6a0067686e135e765f7f78095507044e795e7f17514074010753655b"
    "4272006b01470701630c795575544b7e561a1d32645f647271224a1d4f16515c7a217a2a455103505e7004400f5d5076447a78551f16615500"
    "08667f6f4a6b635a5200616e6752764768167d74166a5d4e436d5c0f041149780e4207020040135c6d551752047f1574761b635c2006606d07"
    "0146532648131d386950616c6e455f765f29790a5703044f7b5b79425146755e070566584723003900170105645a7b5775034b7b534a1f3263"
    "04627b70754a1e4f46505178237f76445501545c7305460c0c567646717d011d42655e040c65756e4c6d68590701616d315674466310767140"
    "6d5c4c196a0b0b0c121b7a0e4354020747475b68521852047c4677761b645c2704606955034751724d121f6b6957636f6f4f5d735c797f5e55"
    "0603447a597f4f504471540502655c427104610247070065097f5474514b79521c1e646100652577764d4d4d10555178717b7d4206015d5c7a"
    "07460c5a5477457a78501a426204005f66756c406e605a5205616d3655704767112b714f6e574c196e0b0d0010197c064200020b424f5e6c51"
    "1352567f1673771f645b7a076d6c510243517f4d151c6f6b53616c6d4259795c7e7f5a5506004678587e14561472540650630f4675006a0217"
    "0705645a7b077054497d554a1d616254672077254d4d4f43515278247b78465503575f7701470f505676462e7e5c1d466554075964726e1c6b"
    "665a5e03336a6255734664167871416f0d4f436f5b0c5712107901405f000741435f3f511452037f11742a1a315e22046c6c54034656214918"
    "1e6a6d5563696f455e745c75795250060345785b7c405345705f030562554273036d02400100630c7e057456482e56131f336207627a707148"
    "494c46535c7824782c475001525e7206150c5a5571447a7d5d1f4f6351060b65746c1d6f66585700656e66527247691676704e6c5e4b4a6a0f"
    "0f0413497c024557030440425c66521253027e47777f1e355b7b046d6d5e0014577748151c6a6850623d68175f2158757e0d500c034e7a567f"
    "4553417202065c625c4526026d054001006009780376074b74534e1f686457652577264d4f4d4352057f727b7745540304587b01470b5f557f"
    "477f7c051a46655f025c63756e1a6d665b5e036b6d3652734562127e70436f0b4b4c6d5b0a53101a7a064250040442165e3c504550547e4776"
    "7b18365f75066b6e52014153704d141e6c6b54616d6e105e245d2c7a0d500c01447b5f7c4e51177352055165554475046d04140702620a7a0e"
    "765e497d50181c606355667a71264d404f43515379767a79415200575e77054f0c515525437d79021f406353005966736c196c665a5403606d"
    "3556774667167871416d5b4d4b6d5b0d0415497b55435e030140145c6f504750007f46747e1c6e5a21056c6c570216522249181f3d6956626d"
    "6d475d725f7c7b5350070341785a7e44524f7005005d640c4326033e0210000363007a027705487d514a1e6861036771777248404f1752057f"
    "727f7d4552005c5d7407160d595474422a7c571a42625e015c66736c196f615a5106336c6056704767132b71436e0f4d4d6f5e0e50161e7806"
    "4152040542465a3c5212520e7f40747b1e335f7005616f07014857744a161d616852616b6e145d775d2b7f525007004f795e7f4f5146755105"
    "52645e4472036a0142030c650878577451497c511c18676454657176254b4d4c4453527f767a78415000525f7504160c0e532744797f501d14"
    "6255025c65706e4d6c64585500346f32567f4761112c72406d564c496b5a0e56101b79044205000742445f665443530f7916737d1f645d7706"
    "6f6c510145502648471d3f6b5c626a6d445e725c7e795a560200457958794257477357050765584273073904160650645c7f5271024a79514e"
    "1f646556647176774a494b42535479707877475102545e7706420a585724447d7e5d1b136403060962226a1c6830";
Settings::Customize Settings::m_customize = Settings::NONE;
QString Settings::m_machineId = Settings::getMachineId();
QMap<int,InformationItem*> Settings::m_infoItemMap;
QString Settings::m_serviceNumber = "";
QString Settings::m_milestone = "";

InformationClassItemList Settings::getSettings()
{
    creatMap();
#ifdef DEBUG_MODE
    creatJson();
#else
    m_jsonByte = QByteArray::fromHex(SettingsCMD::g_SettingsCMD);
#endif
    InformationClassItemList list;
    if (m_jsonByte.isEmpty()) {
        qCritical() << "Settings CMD isEmpty !";
        return list;
    }
    QJsonDocument doc = QJsonDocument::fromJson(m_jsonByte);
    if (!doc.isObject()) {
        qDebug() << "json error !";
        return list;
    }
    QJsonObject obj = doc.object();
    QJsonArray carr = obj.value(KEY_CLASS).toArray();
    int typeNum = 0;
    for (int i = 0; i < carr.size(); i++) {
        QJsonObject cjo = carr[i].toObject();
        InformationClassItem *citem = new InformationClassItem(cjo.value(KEY_NAME).toString());
        citem->setItemNameShow(m_logItemClassMap.value(citem->getItemName()));
        QJsonArray arr = cjo.value(KEY_ITEM).toArray();
        for (int j = 0; j < arr.size(); j++) {
            QJsonObject jo = arr[j].toObject();
            QString tmpName = jo.value(KEY_INFO_NAME).toString();
            InformationItem::CollectionType t = InformationItem::File;
            QString tmpMessage = jo.value(KEY_INFO_FILE).toString();
            QString tmpTypes = jo.value(KEY_INFO_TYPES).toString();
            ModuleTypeSet set = ModuleTypeSet::fromList(tmpTypes.split("|"));
            if (tmpMessage.isEmpty()) {
                t = InformationItem::CMD;
                tmpMessage = jo.value(KEY_INFO_CMD).toString();
            }
            if (tmpMessage.isEmpty()) {
                t = InformationItem::Others;
                tmpMessage = jo.value(KEY_INFO_OTHER).toString();
            }
            typeNum++;
            InformationItem *item = new InformationItem(tmpName, t, tmpMessage, set);
            citem->children()->append(item);
            m_infoItemMap.insert(typeNum,item);
        }
        list.append(citem);
    }
    return list;
}

void Settings::creatJson()
{
    QString csvFile = m_filePath;
    csvFile.chop(1);
    csvFile += "csv";
    if (!QFile::exists(csvFile)) {
        qDebug() << "can not find csv file :" << csvFile;
        return;
    }
    QFile file(csvFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "read csv err";
        return;
    }
    bool skipHeader = true;
    QJsonObject root;
    QJsonArray rootArray;
    QJsonObject classItem;
    QJsonArray classArray;
    QString lastClassItemName;
    while (!file.atEnd()) {
        QString line = file.readLine();
        if (skipHeader) {
            skipHeader = false;
            continue;
        }
        line.remove('\n');
        QStringList tmpList = line.split(',');
        if (tmpList.length() < 6) {
            continue;
        }
        QString tmpClass = tmpList.first();
        QString tmpName = tmpList.at(1);
        QString tmpFile = tmpList.at(2);
        if (!tmpFile.isEmpty()) {
            //            qDebug() << tmpFile << tmpFile.at(tmpFile.length() - 1);
        }
        QString tmpCmd = tmpList.at(3);
        QString tmpTypes = tmpList.at(4);
        QString tmpOther = tmpList.at(5);
        QJsonObject info;
        info.insert(KEY_INFO_NAME, tmpName);
        info.insert(KEY_INFO_FILE, tmpFile);
        info.insert(KEY_INFO_CMD, tmpCmd);
        info.insert(KEY_INFO_OTHER, tmpOther);
        info.insert(KEY_INFO_TYPES, tmpTypes);
        if (lastClassItemName != tmpClass) {
            lastClassItemName = tmpClass;
            if (!classArray.isEmpty()) {
                classItem.insert(KEY_ITEM, classArray);
                rootArray.append(classItem);
                classItem = QJsonObject();
                classArray = QJsonArray();
            }
            classItem.insert(KEY_NAME, tmpClass);
        }
        classArray.append(info);
    }
    file.close();
    if (!classArray.isEmpty()) {
        classItem.insert(KEY_ITEM, classArray);
        rootArray.append(classItem);
    }
    root.insert(KEY_CLASS, rootArray);
    QJsonDocument jd(root);
    m_jsonByte = jd.toJson(QJsonDocument::Compact);
    QFile jsonFile(m_filePath);
    bool replaceFile = false;
    if (jsonFile.exists(m_filePath)) {
        qDebug() << "json file already exist!";
        if (!replaceFile) {
            return;
        }
        jsonFile.remove();
    }
    saveFile();
}

void Settings::saveFile()
{
    QFile saveFile(m_filePath);
    if (!saveFile.open(QIODevice::NewOnly)) {
        qDebug() << "can not creat json file !";
    }
    QByteArray fileH;
    fileH.append("#ifndef SETTINGS_CMD_H \n");
    fileH.append("#define SETTINGS_CMD_H \n");
    fileH.append("#include <QByteArray> \n");
    fileH.append("namespace SettingsCMD { \n");
    fileH.append("const QByteArray g_SettingsCMD=\"");
    fileH.append(m_jsonByte.toHex());
    fileH.append("\";\n");
    fileH.append("} \n");
    fileH.append("#endif");
    saveFile.write(fileH);
    saveFile.close();
}

void Settings::creatMap()
{
    m_logItemClassMap.insert("系统日志", QObject::tr("System log"));
    m_logItemClassMap.insert("整机信息", QObject::tr("Machine"));
    m_logItemClassMap.insert("硬件参数", QObject::tr("Hardware"));
    m_logItemClassMap.insert("驱动信息", QObject::tr("Drive"));
    m_logItemClassMap.insert("软件列表", QObject::tr("APP list"));
    m_logItemClassMap.insert("终端策略", QObject::tr("Rules"));
    m_logItemClassMap.insert("网络状态", QObject::tr("Network"));
    m_logItemClassMap.insert("系统状态", QObject::tr("System"));
}

QString Settings::csvFilePath()
{
    char path[4096];
    getcwd(path, 4096);
    QString res = path;
    if (res.endsWith("mini")) {
        res.chop(21);
    }
    res += "/../../../kylin-os-manager/plugins/service-support/conf/log-collection.h";
    //res = "/home/snowsi/snowsi/clion_code/kylin-os-manager/plugins/service-support/conf/log-collection.h";
    return res;
}

std::tuple<QString, QString, QString> Settings::getUrlInformation(void)
{
    QString protocol;
    QString domain;
    QString port;

    QString path = QString("%1/%2").arg(getenv("HOME")).arg(CONFIG_PLUGIN_USER_PATH);
    QFile userConfigFile(path);
    if (userConfigFile.exists()) {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
        protocol = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PROTOCOL).toString();
        domain = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_DOMAIN).toString();
        port = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PORT).toString();
        settings.endGroup();
    }

    if (!protocol.isEmpty() && !domain.isEmpty() && !port.isEmpty())
        return std::make_tuple(protocol, domain, port);

    QFile systemConfigfile(CONFIG_PLUGIN_SYSTEM_PATH);
    if (systemConfigfile.exists()) {
        QSettings settings(CONFIG_PLUGIN_SYSTEM_PATH, QSettings::IniFormat);
        settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
        if (protocol.isEmpty()) {
            protocol = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PROTOCOL).toString();
        }
        if (domain.isEmpty()) {
            domain = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_DOMAIN).toString();
        }
        if (port.isEmpty()) {
            port = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_PORT).toString();
        }
        settings.endGroup();
    }

    return std::make_tuple(protocol, domain, port);
}

std::tuple<QString, QString, QString> Settings::getRvisitInformation()
{
    QString jobNum;
    QString email;
    QString contact;

    QString path = QString("%1/%2").arg(getenv("HOME")).arg(CONFIG_PLUGIN_USER_PATH);
    QFile userConfigFile(path);
    if (userConfigFile.exists()) {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
        jobNum = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_JOB_NUM).toString();
        email = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL).toString();
        contact = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString();
        settings.endGroup();
    }

    if (!jobNum.isEmpty() && !email.isEmpty() && !contact.isEmpty())
        return std::make_tuple(jobNum, email, contact);

    QFile systemConfigfile(CONFIG_PLUGIN_SYSTEM_PATH);
    if (systemConfigfile.exists()) {
        QSettings settings(CONFIG_PLUGIN_SYSTEM_PATH, QSettings::IniFormat);
        settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
        if (jobNum.isEmpty()) {
            jobNum = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_JOB_NUM).toString();
        }
        if (email.isEmpty()) {
            email = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL).toString();
        }
        if (contact.isEmpty()) {
            contact = settings.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT).toString();
        }
        settings.endGroup();
    }

    return std::make_tuple(jobNum, email, contact);
}

void Settings::setRvisitInformation(const QString &jobNum, const QString &email, const QString &contact)
{
    QString path = QString("%1/%2").arg(getenv("HOME")).arg(CONFIG_PLUGIN_USER_PATH);
    QFile userConfigFile(path);
    if (!userConfigFile.exists()) {
        if (!userConfigFile.open(QIODevice::ReadWrite)) {
            qCritical() << "create user config file fail !";
            return;
        } else {
            userConfigFile.close();
        }
    }

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(CONFIG_PLUGIN_PROBLEM_FEEDBACK);
    settings.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK_JOB_NUM, jobNum);
    settings.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL, email);
    settings.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK_CONTACT, contact);
    settings.endGroup();
}

bool Settings::isUpload(void) {
    kom::Configure configure;
    return configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_ENABLE_UPLOAD, true).toBool();
}

QString Settings::getExportPath(void)
{
    kom::Configure configure;
    return configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH, "").toString();
}

void Settings::setExportPath(const QString &path)
{
    kom::Configure configure;
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EXPORT_PATH, path);
}

void Settings::setHistoryBug(QString bugId)
{
    kom::Configure configure;
    QString bugIdList = getHistoryBug();
    for (const QString &hb : bugIdList.split(",")) {
        if (hb == bugId) {
            return;
        }
    }
    if (!bugIdList.isEmpty()) {
        bugIdList += ',';
    }

    bugIdList += bugId;
    QString key = bugIdList + m_machineId;
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG, key);
    QString hmacSha256 =
        QMessageAuthenticationCode::hash(key.toLocal8Bit(), Settings::getKeyCode(), QCryptographicHash::Sha256).toHex();
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG_KEY, hmacSha256);
}

QString Settings::getHistoryBug()
{
    kom::Configure configure;
    QString key =
        configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG).toString();
    if (key.isEmpty()) {
        return QString();
    }
    QString hmacSha256 =
        QMessageAuthenticationCode::hash(key.toLocal8Bit(), Settings::getKeyCode(), QCryptographicHash::Sha256).toHex();
    QString hmacSha256Key =
        configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_HISTORY_BUG_KEY).toString();
    if (hmacSha256Key != hmacSha256) {
        return QString();
    }
    if (key.right(m_machineId.length()) != m_machineId) {
        //旧版配置
        QString old = encrypto(key.toLocal8Bit());
        if (!old.contains(",") && old.toInt() < 1) {
            return QString();
        }
        key = "";
        for (const QString &str : old.split(",")) {
            key += QString::number(ProjectMode) + ":" + str + ",";
        }
        if (key.isEmpty()) {
            return QString();
        }
        key.chop(1);
        return key;
    }
    key.chop(m_machineId.length());
    return key;
}

void Settings::setZenTaoPwd(const QString &pwd)
{
    kom::Configure configure;
    QString password = pwd + m_machineId;
    QByteArray result = encryptRSA(password.toLocal8Bit());
    QString hmacSha256 =
        QMessageAuthenticationCode::hash(password.toLocal8Bit(), Settings::getKeyCode(), QCryptographicHash::Sha256)
            .toHex();
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_PASSWD_KEY, hmacSha256);
    configure.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_PASSWD, result.toHex());
}

QString Settings::getZenTaoPwd()
{
    kom::Configure configure;
    QByteArray tmp =
        configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_PASSWD).toByteArray();
    QByteArray pwd = decryptRSA(QByteArray::fromHex(tmp));
    QString hmacSha256 =
        QMessageAuthenticationCode::hash(pwd, Settings::getKeyCode(), QCryptographicHash::Sha256).toHex();
    QString key =
        configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_INTERNAL_PASSWD_KEY).toString();
    if (key != hmacSha256) {
        return QString();
    }
    if (pwd.right(m_machineId.length()) != m_machineId) {
        return QString();
    }
    pwd.chop(m_machineId.length());
    return pwd;
}

QString Settings::getMachineId()
{
    QFile file("/etc/machine-id");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "打开 /etc/machine-id 失败 ！";
        return "";
    }
    QString machine = file.readAll();
    file.close();
    if (machine.isEmpty()) {
        qDebug() << "读取 /etc/machine-id 失败 ！";
        return "";
    }
    return machine;
}

QByteArray Settings::getPr()
{
    return QByteArray::fromHex(m_pr1 + encrypto(QByteArray::fromHex(m_pr2)));
}

bool Settings::getHistoryEnable()
{
    kom::Configure configure;
    return configure.value(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_ENABLE_HISTORY, false)
        .toBool();
}

QByteArray Settings::encrypto(const QByteArray &msg)
{
    QByteArray arr = msg;
    for (int i = 0; i < arr.length(); i++) {
        arr[i] = arr[i] ^ m_keycode[i % m_keycode.size()];
    }
    return arr;
}

QByteArray Settings::encryptRSA(const QByteArray &text)
{
    QByteArray result;
    BIO *keybio = BIO_new_mem_buf((unsigned char *)QByteArray::fromHex(m_pu).data(), -1);
    RSA *rsa = NULL;
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, NULL, NULL, NULL);
    if (!rsa) {
        BIO_free_all(keybio);
        return result;
    }
    int len = RSA_size(rsa);
    char *tmpText = new char[len + 1];
    memset(tmpText, 0, len + 1);
    int ret = RSA_public_encrypt(text.length(), (const unsigned char *)text.data(), (unsigned char *)tmpText, rsa,
                                 RSA_PKCS1_PADDING);
    if (ret >= 0) {
        result = QByteArray(tmpText, ret);
    }
    free(tmpText);
    BIO_free_all(keybio);
    RSA_free(rsa);
    return result;
}

QByteArray Settings::decryptRSA(const QByteArray &text)
{
    QByteArray result;
    BIO *keybio = BIO_new_mem_buf((unsigned char *)getPr().data(), -1);
    RSA *rsa = NULL;
    rsa = PEM_read_bio_RSAPrivateKey(keybio, NULL, NULL, NULL);
    if (!rsa) {
        unsigned long err = ERR_get_error();
        char errMsg[1024] = {0};
        ERR_error_string(err, errMsg);
        qDebug() << "decryptRSA error :" << err << errMsg;
        BIO_free_all(keybio);
        return result;
    }
    int len = RSA_size(rsa);
    char *tmpText = new char[len + 1];
    memset(tmpText, 0, len + 1);
    int ret = RSA_private_decrypt(text.length(), (const unsigned char *)text.data(), (unsigned char *)tmpText, rsa,
                                  RSA_PKCS1_PADDING);
    if (ret >= 0) {
        result = QByteArray(tmpText, ret);
    }
    free(tmpText);
    BIO_free_all(keybio);
    RSA_free(rsa);
    return result;
}

QByteArray Settings::getKeyCode()
{
    return m_keycode;
}

Settings::Customize Settings::getCustomize() {
    if (m_customize != NONE) {
        return m_customize;
    }

    kom::Configure conf;
    QString customize = conf.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_CUSTOMIZE).toString();
    if (customize.isEmpty()) {
        m_customize = DEFAULT;
    } else if (customize == "icbc") {
        m_customize = ICBC;
    } else if (customize == "internal") {
        m_customize = INTERNAL;
    } else if (customize == "openkylin") {
        m_customize = OPENKYLIN;
    }

    return m_customize;
}

QString Settings::getSystemDeviceInfo()
{
    QFile file(SYSTEM_INFOMATION_FILE_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Settings::getSystemDeviceInfo 打开文件失败";
        return "";
    }
    QString res = file.readAll();
    while (res.endsWith("\n")) {
        res.chop(1);
    }
    res += "\n";
    return res;
}

QString Settings::getServiceNumber()
{
    if (m_serviceNumber.isEmpty()) {
        QSettings setting(SYSTEM_KYINFO_FILE_PATH, QSettings::IniFormat);
        setting.beginGroup("servicekey");
        m_serviceNumber = setting.value("key").toString();
    }
    return m_serviceNumber;
}

QString Settings::getMilestone()
{
    if (m_milestone.isEmpty()) {
        QSettings setting(SYSTEM_KYINFO_FILE_PATH, QSettings::IniFormat);
        setting.beginGroup("dist");
        m_milestone = setting.value("milestone").toString();
    }
    return m_milestone;
}

QMap<int,InformationItem*> Settings::getInfoItemMap()
{
    return m_infoItemMap;
}
