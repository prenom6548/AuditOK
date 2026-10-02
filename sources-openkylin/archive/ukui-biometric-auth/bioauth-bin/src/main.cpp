/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
**/
//#include "mainwindow.h"
#include <QCoreApplication>
#include <QTranslator>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <termios.h>
#include <signal.h>

#include <pwd.h>

#include "generic.h"
#include "keywatcher.h"
#include "bioauth.h"
#include "biodevices.h"

#define START_COLOR     "\033[1;31m"
#define RESULT_COLOR    "\033[1;31m"
#define PROMPT_COLOR    "\033[33m"
#define NOTIFY_COLOR    //"\033[37m"
#define QUESTION_COLOR  //"\033[1;37m"
#define RESET_COLOR     "\033[0m"

bool enableDebug;
QString logPrefix;
static BioAuth *g_bioAuth = nullptr;
static int g_pipe_fd = -1; /* 全局变量：存储管道文件描述符 */
enum MsgType
{
    START,
    PROMPT,
    NOTIFY,
    QUESTION,
    RESULT
};

enum _Option
{
    OPTION_TRY_AGAIN,
    OPTION_SELECT_DEVICE,
    OPTION_CANCEL,
    OPTION_UNDEFINED
};
typedef enum _Option Option;

static QString lastMessage;


void showMessage(const QString &message, int type)
{
    // filter out the same message
    if(message == lastMessage)
        return;

    lastMessage = message;

    char *text = message.toLocal8Bit().data();

    switch(type) {
    case START:
        fprintf(stdout, START_COLOR "=== %s ===\n", text);
        break;
    case PROMPT:
        fprintf(stdout, PROMPT_COLOR "%s\n", text);
        break;
    case NOTIFY:
        fprintf(stdout, NOTIFY_COLOR "%s\n", text);
        break;
    case QUESTION:
        fprintf(stdout, QUESTION_COLOR "%s", text);
        break;
    case RESULT:
        fprintf(stdout, RESULT_COLOR "=== %s ===\n", text);
        break;
    default:
        fprintf(stdout, "%s\n", text);
        break;
    }
    fprintf(stdout, RESET_COLOR);
}

static bool isQuickInput() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);    // stdin
    struct timeval timeout = {0, 10*1000};
    switch (select(32, &readfds, NULL, NULL, &timeout)) {
    case 0: // time out
        break;
    case -1: // error
        break;
    default:
        return true;
    }
    return false;
}

QString inputPinCode()
{
    struct termios current;
     struct termios save;
    tcgetattr(0, &save);

    current = save;
//    current.c_lflag &= ~ICANON;
    current.c_lflag &= ~ECHO;
    current.c_cc[VMIN] = 0;
    current.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &current);

    QString password = "";
    showMessage(QObject::tr("Enter the ukey password"), QUESTION);

    char line[1024] = {0};
    fgets(line, sizeof(line), stdin);
    //scanf("%s",&line);

    showMessage("\n", QUESTION);

    line[1023] = '\0';

    if(line[strnlen(line,1024) - 1] == '\n')
        line[strnlen(line,1024) - 1] = '\0';

    QString str(line);

    tcsetattr(0, TCSANOW, &save);   // 恢复原来的终端属性，以免干扰shall和之后的程序运行
    return line;
}

Option showOption(bool showSelectDevices)
{
    QStringList optionList;
    QString message;

    optionList << QObject::tr("Try it again");
    if(showSelectDevices)
        optionList << QObject::tr("Use other Devices");
    optionList << QObject::tr("Cancel");

    for(int i = 0; i < optionList.size(); i++){
        message = QString(" %1. %2\n")
                .arg(QString::number(i+1))
                .arg(optionList.at(i));
        showMessage(message, QUESTION);
    }
    message = QString("%1 (1-%2):")
            .arg(QObject::tr("Choose the option"))
            .arg(QString::number(optionList.size()));
    showMessage(message, QUESTION);

    char line[1024];
    fgets(line, sizeof(line), stdin);

    if(line[strnlen(line,1024) - 1] == '\n')
        line[strnlen(line,1024) - 1] = '\0';

    if(!strcmp(line, "1"))
        return OPTION_TRY_AGAIN;
    else if(!strcmp(line, "2"))
    {
        if(showSelectDevices)
            return OPTION_SELECT_DEVICE;
        else
            return OPTION_CANCEL;
    }
    else if(showSelectDevices && !strcmp(line, "3"))
        return OPTION_CANCEL;
    else {
        showMessage(QObject::tr("Invaild response \"") + line + "\"" + "," + QObject::tr("authentication will be canceld") , NOTIFY);
        showMessage(QObject::tr("AUTHENTICATION CANCELED"), START);
        exit(BIO_FAILED);
    }
    return OPTION_UNDEFINED;
}

DeviceInfo showDevices(const QMap<int, QList<DeviceInfo>> &devicesMap)
{
    int count = 0;
    QString message;

    showMessage(QObject::tr("=== BIOMETRIC DEVICES ==="), PROMPT);
    for(auto type : devicesMap.keys()) {
        for(auto deviceInfo : devicesMap[type]) {
            message = QString(" %1. [%2] %3")
                    .arg(QString::number(++count))
                    .arg(BioDevices::bioTypeToString_tr(type))
                    .arg(deviceInfo.device_shortname);
            showMessage(message, NOTIFY);
        }
    }
    message = QString("%1 (1-%2):")
            .arg(QObject::tr("Choose the device"))
            .arg(QString::number(count));
    showMessage(message, QUESTION);

    char line[1024];
    fgets(line, sizeof(line), stdin);

    if(line[strnlen(line,1024) - 1] == '\n')
        line[strnlen(line,1024) - 1] = '\0';

    for(auto ch : QString(line)) {
        if(!ch.isDigit()){
            showMessage(QObject::tr("Invaild response \"") + line + "\"" + "," +QObject::tr("authentication will be canceld") , NOTIFY);
            showMessage(QObject::tr("AUTHENTICATION CANCELED"), START);
            exit(BIO_FAILED);
        }
    }

    int deviceIndex = QString(line).toInt();

    if(deviceIndex > count) {
        showMessage(QObject::tr("Invaild response \"") + line + "\"" + "," + QObject::tr("authentication will be canceld"), NOTIFY);
        showMessage(QObject::tr("AUTHENTICATION CANCELED"), START);
        exit(BIO_FAILED);
    }

    int i = 0;
    for(auto type : devicesMap.keys()) {
        int size = devicesMap[type].size();
        if(i + size >= deviceIndex) {
            return devicesMap[type][deviceIndex - i - 1];
        }
        i += size;
    }
    return DeviceInfo();
}

static void signal_handler(int signo)
{
    if (signo == SIGHUP && g_bioAuth) {
        g_bioAuth->stopAuth();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication a(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator, translator_bin;
    QString qmfile = QString("%1/i18n_qm/%2.qm").arg(GET_STR(UKUI_BIOMETRIC)).arg(locale);
    QString qmfile_bin = QString("%1/i18n_qm/bioauth-bin/%2.qm").arg(GET_STR(UKUI_BIOMETRIC)).arg(locale);
    qDebug() << "load translation file " << qmfile << qmfile_bin;
    translator.load(qmfile);
    translator_bin.load((qmfile_bin));
    a.installTranslator(&translator);
    a.installTranslator(&translator_bin);

    QCommandLineParser parser;
    QCommandLineOption serviceOption({"s", "service"}, QObject::tr("Sevice Name"), "service", "");
    QCommandLineOption displayOption({"x", "display"}, QObject::tr("DISPLAY env"), "display", ":0");
    QCommandLineOption usernameOption({"u", "user"}, QObject::tr("User"), "user", "");
    QCommandLineOption debugOption({"d", "debug"}, QObject::tr("Display debug information"));
    QCommandLineOption deviceOption({"e", "device"}, QObject::tr("Device short name"), "");

    parser.addOptions({serviceOption, displayOption, usernameOption, debugOption, deviceOption});
    parser.process(a);

    if(parser.isSet(debugOption))
        enableDebug = true;
    else
        enableDebug = false;

    logPrefix = "[pam-diaglog]:";
    qInstallMessageHandler(outputMessage);

    QString serverName = parser.value(serviceOption);
    if (serverName == "sudo" && isQuickInput()) {
        exit(BIO_ERROR);
    }

    QString userName = parser.value(usernameOption);
    if(userName.isEmpty())
        exit(BIO_ERROR);
    qDebug() << "authentication user: " << userName;

    uid_t uid;
    struct passwd *pwd = getpwnam(userName.toLocal8Bit().data());
    if(pwd)
        uid = pwd->pw_uid;
    else
        exit(BIO_ERROR);

    BioDevices bioDevices(true);
    bioDevices.setUId(uid);
    int maxFailedTimes = bioDevices.getFailedTimes();
    DeviceInfoPtr deviceInfo = bioDevices.getDefaultDevice(uid);
    bool isHiddenSwitchButton = bioDevices.GetHiddenSwitchButton();
    if(!deviceInfo){    // 只检查默认设备，不再使用第一个设备
        exit(BIO_ERROR);
    }
    if(bioDevices.getFeatureCount(uid)<1)
        exit(BIO_ERROR);

    if(deviceInfo->biotype == 6){
        showMessage(QObject::tr("UKEY AUTHENTICATION"), START);   
    }else{
        showMessage(QObject::tr("BIOMETRIC AUTHENTICATION"), START);
        if(!isHiddenSwitchButton)
            showMessage(QObject::tr("Press Q or Esc to cancel"), PROMPT);
    }

    BioAuth bioAuth;
    g_bioAuth = &bioAuth;  // 设置静态指针，供 signal_handler 使用
    
    bioAuth.setUid(uid);
    bioAuth.setDevice(deviceInfo);
    KeyWatcher watcher;
    QMap<int,int>   m_failedTimes;

    QObject::connect(&bioAuth, &BioAuth::notify, &a, [&](const QString &msg){
        showMessage(msg, NOTIFY);
    });
    QObject::connect(&bioAuth, &BioAuth::authComplete, &a, [&](uid_t uid_, int result, int retErrNo){
        Q_UNUSED(retErrNo);

        watcher.stop();
        bool isBioEnable = bioDevices.GetBioAuthEnable(uid_);
        bool isUkeyDevice = false;
        if(deviceInfo && deviceInfo->biotype == 6){
            isUkeyDevice = true;
        }
        if(!isBioEnable && !isUkeyDevice){
            showMessage(QObject::tr("BIOMETRIC AUTHENTICATION IS CLOSED"), RESULT);
            exit(BIO_IGNORE);
        }
        if(result && uid == uid_) {
            showMessage(QObject::tr("AUTHENTICATION SUCCESS"), RESULT);
            exit(BIO_SUCCESS);
        }
        else {
            showMessage(QObject::tr("AUTHENTICATION FAILED"), RESULT);
            DeviceInfoPtr curDeviceInfo = bioAuth.getDevice();
            if (m_failedTimes.contains(deviceInfo->device_id)) {
                m_failedTimes[curDeviceInfo->device_id] = m_failedTimes[curDeviceInfo->device_id] + 1;
            } else {
                m_failedTimes[curDeviceInfo->device_id] = 1;
            }

            if(m_failedTimes[curDeviceInfo->device_id]<maxFailedTimes)
            {
                showMessage(QObject::tr("Failed to verify %1, you still have %2 verification opportunities").arg(bioDevices.bioTypeToString_tr(curDeviceInfo->biotype)).arg(maxFailedTimes-m_failedTimes[deviceInfo->device_id]),RESULT);
            }else{
                showMessage(QObject::tr("Unable to verify %1, please enter password.").arg(bioDevices.bioTypeToString_tr(curDeviceInfo->biotype)),RESULT);
                exit(BIO_IGNORE);
            }
            Option option = showOption(bioDevices.getAllDevices().count() > 1);
            switch(option) {
            case OPTION_TRY_AGAIN:
                
                if(deviceInfo->biotype == 6){
                    QString code = inputPinCode();
                    bioAuth.SetExtraInfo("pincode",code);
                }

                bioAuth.startAuth();
                watcher.start();
                break;
            case OPTION_SELECT_DEVICE:
            {
                deviceInfo = std::make_shared<DeviceInfo>();
                *deviceInfo = showDevices(bioDevices.getAllDevices());
                bioAuth.setDevice(deviceInfo);

                if(deviceInfo->biotype == 6){
                    QString code = inputPinCode();
                    bioAuth.SetExtraInfo("pincode",code);
                }

                bioAuth.startAuth();
                watcher.start();
                break;
            }
            case OPTION_CANCEL:
                if(isHiddenSwitchButton)
                {
                    exit(BIO_ERROR);
                }else{
                    if(deviceInfo->biotype == 6){
                        showMessage(QObject::tr("AUTHENTICATION END"), START);
                    }else{
                        showMessage(QObject::tr("BIOMETRIC AUTHENTICATION END"), START);
                    }
                    exit(BIO_IGNORE);
                    break;
                }
            default:
                break;
            }
        }
    });

    if(deviceInfo->biotype == 6){
        QString code = inputPinCode();
        bioAuth.SetExtraInfo("pincode",code);
    }

    bioAuth.startAuth();

    if (!isHiddenSwitchButton) {
        QObject::connect(&watcher, &KeyWatcher::exit, &a, [&]{
            showMessage(QObject::tr("AUTHENTICATION CANCELED"), START);
            bioAuth.stopAuth();
            exit(BIO_IGNORE);
        });
    }
    watcher.start();
    signal(SIGHUP, signal_handler);

    return a.exec();
}
