/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2005 Lubos Lunak <l.lunak@kde.org>
Copyright (C) Copyright 2021 KylinSoft Co., Ltd.

relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright 1999 Matthias Ettrich <ettrich@kde.org>
Copyright 1999 Preston Brown <pbrown@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "ukuismserver.h"
#include "ukuismclient.h"
#include "ukuismconnection.h"
#include "ukuismserverdebug.h"
#include "ukuismserverdbusadaptor.h"
#include "ukuikwinsession_interface.h"
#include "../ukui-session/xdgdesktopfile.h"

#include <X11/ICE/ICElib.h>
extern "C" {
#include <X11/ICE/ICEutil.h>
#include <X11/ICE/ICEmsg.h>
#include <X11/ICE/ICEproto.h>
#include <X11/SM/SM.h>
}

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#include <QApplication>
#include <QPalette>
#include <QDesktopWidget>
#include <QDBusInterface>
#include <QDir>
#include <QTemporaryFile>
#include <QDBusMessage>

#include <KProcess>
#include <KSharedConfig>
#include <KConfigGroup>

#define VendorString "ukuismserver"
#define ReleaseString "1.0"

#define SESSION_PREVIOUS_LOGOUT "saved at previous logout"

enum KWinSessionState {
    Normal = 0,
    Saving = 1,
    Quitting = 2
};

IceAuthDataEntry *authDataEntries = nullptr;

static QTemporaryFile *remTempFile = nullptr;

static IceListenObj *listenObjs = nullptr;
int numTransports = 0;
static bool onlyLocal = false;

extern "C" int _IceTransNoListen(const char *protocol);

UKUISMServer*& getGlobalServer(const QString &wm = "")
{
    static UKUISMServer *server = new UKUISMServer(wm);
    return server;
}

static Bool HostBasedAuthProc(char *hostname)
{
    if (onlyLocal) {
        return false;
    }
}

static bool isCommandSafe(SmProp *prop)
{
    if (!prop || qstrcmp(prop->type, SmLISTofARRAY8) || prop->num_vals < 1) {
        return true;
    }

    for (int i = 0; i < prop->num_vals; i++) {
        QString arg = QString::fromLocal8Bit((const char*)prop->vals[i].value);

        if (arg.contains(QLatin1Char(';')) ||
            arg.contains(QLatin1Char('|')) ||
            arg.contains(QLatin1Char('&')) ||
            arg.contains(QLatin1Char('>')) ||
            arg.contains(QLatin1Char('<')) ||
            arg.contains(QLatin1Char('`')) ||
            arg.contains(QLatin1String("$(")) ||
            arg.contains(QLatin1Char('\n')) ||
            arg.contains(QLatin1Char('\r'))) {
            qCWarning(UKUISMSERVER) << "Rejected command with shell metacharacters:" << arg;
            return false;
        }
    }

    return true;
}

Status RegisterClientProc(SmsConn smsConn, SmPointer managerData, char *previousId)
{
    UKUISMClient *client = static_cast<UKUISMClient*>(managerData);
    client->registerClient(previousId);
    //注意，在被恢复应用注册的时候有可能发生registered这条日志出现在后的情况
    qCDebug(UKUISMSERVER) << "client " << client->clientId() << " registered.";
    return 1;
}

void InteractRequestProc(SmsConn smsConn, SmPointer managerData, int dialogType)
{
    getGlobalServer()->interactRequest(static_cast<UKUISMClient*>(managerData), dialogType );
}

void InteractDoneProc(SmsConn smsConn, SmPointer managerData, Bool cancelShutdown)
{
    getGlobalServer()->interactDone(static_cast<UKUISMClient*>(managerData), cancelShutdown);
}

void SaveYourselfRequestProc(SmsConn smsConn, SmPointer managerData, int saveType, Bool shutdown, int interactStyle, Bool fast, Bool global)
{
    //如果shutdown为true,则执行关机流程
    if (shutdown) {
        getGlobalServer()->shutdown();
    } else if (!global) {
        //如果global为false,则只向发送请求的客户端发送save yourself
        SmsSaveYourself(smsConn, saveType, false, interactStyle, fast);
        SmsSaveComplete(smsConn);
    }
}

void SaveYourselfPhase2RequestProc(SmsConn smsConn, SmPointer managerData)
{
    getGlobalServer()->phase2Request(static_cast<UKUISMClient*>(managerData));
}

void SaveYourselfDoneProc(SmsConn smsConn, SmPointer managerData, Bool success)
{
    getGlobalServer()->saveYourselfDone(static_cast<UKUISMClient*>(managerData), success);
}

void CloseConnectionProc(SmsConn smsConn, SmPointer managerData, int count, char **reasonMsgs)
{
    qCDebug(UKUISMSERVER) << "one app close connection";
    getGlobalServer()->deleteClient(static_cast<UKUISMClient*>(managerData));
    if (count) {
        SmFreeReasons(count, reasonMsgs);
    }

    IceConn iceConn = SmsGetIceConnection(smsConn);
    SmsCleanUp(smsConn);
    IceSetShutdownNegotiation(iceConn, False);
    IceCloseConnection(iceConn);
}

void SetPropertiesProc(SmsConn smsConn, SmPointer managerData, int numProps, SmProp **props)
{
    UKUISMClient *client = static_cast<UKUISMClient*>(managerData);
    for (int i = 0; i < numProps; i++) {
        if (!qstrcmp(props[i]->name, SmRestartCommand) ||
            !qstrcmp(props[i]->name, SmDiscardCommand) ||
            !qstrcmp(props[i]->name, SmCloneCommand)) {
            if (!isCommandSafe(props[i])) {
                qCWarning(UKUISMSERVER) << "Client" << client->clientId()
                                        << "attempted to set unsafe command property:"
                                        << props[i]->name;
                SmFreeProperty(props[i]);
                continue;
            }
        }

        SmProp *p = client->property(props[i]->name);
        if (p) {
            client->m_properties.removeAll( p );
            SmFreeProperty(p);
        }
        client->m_properties.append(props[i]);

        if (!qstrcmp(props[i]->name, SmProgram)) {
            getGlobalServer()->clientSetProgram(client);
            qCDebug(UKUISMSERVER) << client->clientId() << " and " << client->program();
        }
    }

    if (numProps) {
        free(props);
    }
}

void DeletePropertiesProc(SmsConn smsConn, SmPointer managerData, int numProps, char **propNames)
{
    UKUISMClient *client = static_cast<UKUISMClient*>(managerData);
    for (int i = 0; i < numProps; i++) {
        SmProp *p = client->property(propNames[i]);
        if (p) {
            client->m_properties.removeAll(p);
            SmFreeProperty(p);
        }
    }
}

void GetPropertiesProc(SmsConn smsConn, SmPointer managerData)
{
    UKUISMClient *client = static_cast<UKUISMClient*>(managerData);
    SmProp **props = new SmProp*[client->m_properties.count()];
    int i = 0;
    foreach (SmProp *prop, client->m_properties) {
        props[i++] = prop;
    }
    SmsReturnProperties(smsConn, i, props);
    delete[] props;
}

static Status NewClientProc(SmsConn conn, SmPointer manager_data, unsigned long *mask_ret, SmsCallbacks *cb, char **failure_reason_ret)
{
    //根据XSMP协议的要求，出错时，这个函数应该返回状态0,并且错误原因要包含在failure_reason_ret中
    *failure_reason_ret = nullptr;

    void *client = (static_cast<UKUISMServer*>(manager_data))->newClient(conn);

    cb->register_client.callback = RegisterClientProc;
    cb->register_client.manager_data = client;
    cb->interact_request.callback = InteractRequestProc;
    cb->interact_request.manager_data = client;
    cb->interact_done.callback = InteractDoneProc;
    cb->interact_done.manager_data = client;
    cb->save_yourself_request.callback = SaveYourselfRequestProc;
    cb->save_yourself_request.manager_data = client;
    cb->save_yourself_phase2_request.callback = SaveYourselfPhase2RequestProc;
    cb->save_yourself_phase2_request.manager_data = client;
    cb->save_yourself_done.callback = SaveYourselfDoneProc;
    cb->save_yourself_done.manager_data = client;
    cb->close_connection.callback = CloseConnectionProc;
    cb->close_connection.manager_data = client;
    cb->set_properties.callback = SetPropertiesProc;
    cb->set_properties.manager_data = client;
    cb->delete_properties.callback = DeletePropertiesProc;
    cb->delete_properties.manager_data = client;
    cb->get_properties.callback = GetPropertiesProc;
    cb->get_properties.manager_data = client;

    *mask_ret = SmsRegisterClientProcMask |
                SmsInteractRequestProcMask |
                SmsInteractDoneProcMask |
                SmsSaveYourselfRequestProcMask |
                SmsSaveYourselfP2RequestProcMask |
                SmsSaveYourselfDoneProcMask |
                SmsCloseConnectionProcMask |
                SmsSetPropertiesProcMask |
                SmsDeletePropertiesProcMask |
                SmsGetPropertiesProcMask;
    return 1;
}

/*用于打印16进制数据*/
static void fprintfhex(FILE *fp, unsigned int len, char *cp)
{
    static const char hexchars[] = "0123456789abcdef";

    for (; len > 0; len--, cp++) {
        unsigned char s = *cp;
        putc(hexchars[s >> 4], fp);
        putc(hexchars[s & 0x0f], fp);
    }
}

static void write_iceauth(FILE *addfp, FILE *removefp, IceAuthDataEntry *entry)
{
    fprintf(addfp, "add %s \"\" %s %s ", entry->protocol_name, entry->network_id, entry->auth_name);
    fprintfhex(addfp, entry->auth_data_length, entry->auth_data);
    fprintf(addfp, "\n");

    fprintf(removefp,
            "remove protoname=%s protodata=\"\" netid=%s authname=%s\n",
            entry->protocol_name,
            entry->network_id,
            entry->auth_name);
}

#define COOKIE_LEN 16


Status SetAuthentication (int count, IceListenObj *listenObjs, IceAuthDataEntry **authDataEntries)
{
    QTemporaryFile addTempFile;
    remTempFile = new QTemporaryFile;

    if (!addTempFile.open() || !remTempFile->open())
        return 0;

    for (int j = 0; j < count; j++) {
        char *prot = IceGetListenConnectionString(listenObjs[j]);
        if (!prot) continue;
        char *host = strchr(prot, '/');
        char *sock = nullptr;

        if (host) {
            *host=0;
            host++;
            sock = strchr(host, ':');
            if (sock) {
                *sock = 0;
                sock++;
            }
        }

        if (sock && !strcmp(prot, "local")) {
            chmod(sock, 0700);
        }
        free(prot);
    }

    if ((*authDataEntries = (IceAuthDataEntry *) malloc(count * 2 * sizeof(IceAuthDataEntry))) == nullptr)
        return 0;

    FILE *addAuthFile = fopen(QFile::encodeName(addTempFile.fileName()).constData(), "r+");
    FILE *remAuthFile = fopen(QFile::encodeName(remTempFile->fileName()).constData(), "r+");

    for (int i = 0; i < numTransports * 2; i += 2) {
        (*authDataEntries)[i].network_id = IceGetListenConnectionString(listenObjs[i/2]);
        (*authDataEntries)[i].protocol_name = (char*)"ICE";
        (*authDataEntries)[i].auth_name = (char*)"MIT-MAGIC-COOKIE-1";

        (*authDataEntries)[i].auth_data = IceGenerateMagicCookie(COOKIE_LEN);
        (*authDataEntries)[i].auth_data_length = COOKIE_LEN;

        (*authDataEntries)[i+1].network_id = IceGetListenConnectionString(listenObjs[i/2]);
        (*authDataEntries)[i+1].protocol_name = (char*)"XSMP";
        (*authDataEntries)[i+1].auth_name = (char*)"MIT-MAGIC-COOKIE-1";

        (*authDataEntries)[i+1].auth_data = IceGenerateMagicCookie(COOKIE_LEN);
        (*authDataEntries)[i+1].auth_data_length = COOKIE_LEN;

        write_iceauth(addAuthFile, remAuthFile, &(*authDataEntries)[i]);
        write_iceauth(addAuthFile, remAuthFile, &(*authDataEntries)[i+1]);

        IceSetPaAuthData(2, &(*authDataEntries)[i]);

        IceSetHostBasedAuthProc(listenObjs[i/2], HostBasedAuthProc);
    }

    fclose(addAuthFile);
    fclose(remAuthFile);

    QString iceAuth = QStandardPaths::findExecutable(QStringLiteral("iceauth"));
    if (iceAuth.isEmpty())
    {
        qCDebug(UKUISMSERVER) << "UKUISMServer: could not find iceauth";
        return 0;
    }

    KProcess p;
    p << iceAuth << QStringLiteral("source") << addTempFile.fileName();
    p.execute();

    return (1);
}

void FreeAuthenticationData(int count, IceAuthDataEntry *authDataEntries)
{
    if (!authDataEntries)
        return;

    /* Each transport has entries for ICE and XSMP */
    for (int i = 0; i < count * 2; i++) {
        free(authDataEntries[i].network_id);
        free(authDataEntries[i].auth_data);
    }

    free (authDataEntries);

    QString iceAuth = QStandardPaths::findExecutable(QStringLiteral("iceauth"));
    if (iceAuth.isEmpty())
    {
        qCDebug(UKUISMSERVER) << "UKUISMServer: could not find iceauth";
        return;
    }

    if (remTempFile)
    {
        KProcess p;
        p << iceAuth << QStringLiteral("source") << remTempFile->fileName();
        p.execute();
    }

    delete remTempFile;
    remTempFile = nullptr;
}

void UKUISMWatchProc(IceConn iceConn, IcePointer client_data, Bool opening, IcePointer *watch_data)
{
    UKUISMServer *ds = static_cast<UKUISMServer*>(client_data);

    if (opening) {
        *watch_data = (IcePointer)ds->watchConnection(iceConn);
    } else  {
        ds->removeConnection((UKUISMConnection*)*watch_data);
    }
}


UKUISMServer::UKUISMServer(const QString &windowManager) : m_kwinInterface(new OrgUkuiKWinSessionInterface(QStringLiteral("org.ukui.KWin"), QStringLiteral("/Session"), QDBusConnection::sessionBus(), this))
                                                         , m_state(Idle), m_saveSession(false), m_wmPhase1WaitingCount(0), m_clientInteracting(nullptr), m_sessionGroup(QStringLiteral(""))
                                                         , m_wm(windowManager.isEmpty() ? QStringLiteral("kwin_x11") : windowManager), m_wmCommands(QStringList({m_wm}))
{
    new ukuismserverdbusadaptor(this);
    m_restartNum = 0;
    bool success = QDBusConnection::sessionBus().registerObject(QStringLiteral("/UKUISMServer"), this);//注册d-bus对象
    qCDebug(UKUISMSERVER) << "registered object is " << success;
//    connect(this, &UKUISMServer::UKUISMServerStartFinished, adaptor, &ukuismserverdbusadaptor::UKUISMServerStartFinished);

    onlyLocal = true;
    if (onlyLocal) {
        _IceTransNoListen("tcp");
    }

    char errormsg[256];
    if (!SmsInitialize((char*)VendorString, (char*)ReleaseString, NewClientProc, (SmPointer) this, HostBasedAuthProc, 256, errormsg)) {
        qCDebug(UKUISMSERVER) << "UKUISMServer: could not register XSM protocol";
    }

    if (!IceListenForConnections(&numTransports, &listenObjs, 256, errormsg)) {
        qCDebug(UKUISMSERVER) << "UKUISMServer: Error listening for connections: " << errormsg;
        qCDebug(UKUISMSERVER) << "UKUISMServer: Aborting.";
        exit(1);
    }

    {
        QByteArray fName = QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)
                                             + QDir::separator()
                                             + QStringLiteral("UKUISMServer"));

        QString display = QString::fromLocal8Bit(::getenv("DISPLAY"));
        display.remove(QRegExp(QStringLiteral("\\.[0-9]+$")));
        int i = 0;
        while ((i = display.indexOf(QLatin1Char(':'))) >= 0) {
            display[i] = '_';
        }

        while ((i = display.indexOf(QLatin1Char('/'))) >= 0) {
            display[i] = '_';
        }

        fName += '_'+display.toLocal8Bit();
        FILE *f;
        //w+ 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件
        f = ::fopen(fName.data(), "w+");
        if (!f) {
            QString str = QString(QStringLiteral("UKUISMServer: cannot open %s: %s")).arg(fName.data()).arg(strerror(errno));
            qCDebug(UKUISMSERVER) << str;
            qCDebug(UKUISMSERVER) << "UKUISMServer: Aborting.";
            exit(1);
        }
        char *session_manager = IceComposeNetworkIdList(numTransports, listenObjs);
        fprintf(f, "%s\n%i\n", session_manager, getpid());
        fclose(f);
        setenv("SESSION_MANAGER", session_manager, true);

        //这里要将session_manager变量传递给其他需要的进程
        //同步环境变量到D-Bus中
        syncDBusEnvironment();
        //同步环境变量到ukui-session中
        syncSessionEnv("SESSION_MANAGER", session_manager);

        free(session_manager);
    }

    if (!SetAuthentication(numTransports, listenObjs, &authDataEntries)) {
        qFatal("ukuismserver : authentication setup failed.");
    }

    IceAddConnectionWatch(UKUISMWatchProc, (IcePointer)this);

    UKUISMListener *con;
    for (int i = 0; i < numTransports; i++) {
        fcntl(IceGetListenConnectionNumber(listenObjs[i]), F_SETFD, FD_CLOEXEC);
        con = new UKUISMListener(listenObjs[i]);
        m_listener.append(con);
        connect(con, &UKUISMListener::activated, this, &UKUISMServer::newConnection);
    }


    connect(qApp, &QApplication::aboutToQuit, this, &UKUISMServer::cleanUp);
    connect(&m_restoreTimer, &QTimer::timeout, this, &UKUISMServer::tryRestoreNext);
    connect(&m_protectionTimer, &QTimer::timeout, this, &UKUISMServer::protectionTimeout);

    qCDebug(UKUISMSERVER) << "finish construct ukuismserver";
//    QTimer::singleShot(0, [this](){
//        this->UKUISMServerStartFinished();
//    });
}

UKUISMServer::~UKUISMServer()
{
    qDeleteAll(m_listener);
    cleanUp();
}

UKUISMClient *UKUISMServer::newClient(SmsConn conn)
{
    UKUISMClient *client = new UKUISMClient(conn);
    m_clients.append(client);
    m_activeClients[client] = 0;
    return client;
}

void UKUISMServer::deleteClient(UKUISMClient *client)
{
    int num = m_clients.removeAll(client);
    qCDebug(UKUISMSERVER) << "m_clients remove client " << client->clientId() << " num is " << num << ", remain " << m_clients.size();

    if (client == m_clientInteracting) {
        m_clientInteracting = nullptr;
        handlePendingInteractions();
    }

    delete client;

    if (m_state == Shutdown) {
        completeShutdownOrCheckpoint();
    }

    if (m_state == Killing) {
        completeKilling();
    }

    if (m_state == KillingWM) {
        completeKillingWM();
    }
}

void UKUISMServer::clientRegistered(const char *previousId)
{
    if (previousId && m_lastIdRestore == QString::fromLocal8Bit(previousId)) {
        qCDebug(UKUISMSERVER) << "call tryRestoreNext";
        tryRestoreNext();
    }
}

void UKUISMServer::interactRequest(UKUISMClient *client, int dialogType)
{
    //如果是关机阶段，该客户端的请求就要暂时挂起
    if (m_state == Shutdown) {
        //将pendingInteraction属性设置为true,以便在handlePendingInteractions中处理
        qCDebug(UKUISMSERVER) << client->clientId() << "ask for interact" << "pending it";
        client->m_pendingInteraction = true;
    } else {
        //非关机阶段，则直接授予客户端交互权限
        qCDebug(UKUISMSERVER) << "sending interact permission to client " << client->clientId();
        SmsInteract(client->connection());
    }
    //处理被挂起的客户端请求
    handlePendingInteractions();
}

void UKUISMServer::interactDone(UKUISMClient *client, bool cancelShutdown_)
{
    //如果交互完成的客户端与服务器保存的m_clientInteraction信息不一致，则返回，一般不会发生，因为退出时的交互对话框是模态的
    if (client != m_clientInteracting) return;
    //重置m_clientInteraciton，以便处理下一个挂起的客户端
    qCDebug(UKUISMSERVER) << m_clientInteracting->clientId() << "interact done";
    m_clientInteracting = nullptr;
    //如果客户端取消关机，则向所有客户端发送取消关机信号
    if (cancelShutdown_) {
        QString programPath = client->program();
        QString programName = programPath.mid(programPath.lastIndexOf(QDir::separator()) + 1);
        if (programName != QLatin1String("ukui-screensaver-default")) {
            qCDebug(UKUISMSERVER) << client->clientId() << "cancel shutdown";
            QString localName = findLocalName(programName);
            // QString message;
            // message = localName + QObject::tr(" canceled this operation");
            // bubbleNotify(message);
            cancelShutdown(client);
        } else {
            //屏保程序不正常退出时，会在注销阶段发送一个取消注销信号过来，忽略这个信号，才能正常完成注销
            qCDebug(UKUISMSERVER) << "ukui-screensaver-default send cancel shutdown, ignore it";
            handlePendingInteractions();
        }
    } else {
        //处理下一个挂起的客户端
        handlePendingInteractions();
    }
}

void UKUISMServer::phase2Request(UKUISMClient *client)
{
    qCDebug(UKUISMSERVER()) << "wm ask for phase2 request";
    //这两个成员变量对于窗管才有用
    client->m_waitForPhase2 = true;
    client->m_wasPhase2 = true;

    completeShutdownOrCheckpoint();
    if (isWM(client) && m_wmPhase1WaitingCount > 0) {
        --m_wmPhase1WaitingCount;
        //窗管完成phase1保存，请求phase2，服务器先向所有其他客户端发送保存命令
        if (m_wmPhase1WaitingCount == 0) {
            foreach (UKUISMClient *c, m_clients) {
                if (!isWM(c)) {
                    qCDebug(UKUISMSERVER) << "wm done phase1, sending saveyourself to " << c->clientId();
                    SmsSaveYourself(c->connection(), m_saveType, m_saveType != SmSaveLocal,
                                    m_saveType != SmSaveLocal ? SmInteractStyleAny : SmInteractStyleNone, false);
                }
            }
        }
    }
}

void UKUISMServer::saveYourselfDone(UKUISMClient *client, bool success)
{
    //此处是非shutdown状态时，客户端注册进来仍然会走saveyourselfdone,参考kde代码中的注释
    if (m_state == Idle || m_state == LaunchingWM || m_state == Restoring) {
        m_activeClients[client] = 1;
        QStringList discard = client->discardCommand();

        if (!discard.isEmpty()) {
            executeCommand(discard);
        }

        return;
    }

    if (success) {
        qCDebug(UKUISMSERVER) << client->clientId() << "successfully done save";
        client->m_saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    } else {
        //即使保存不成功也要按照成功的方式进行下一步，否则无法保存下一个客户端
        client->m_saveYourselfDone = true;
        completeShutdownOrCheckpoint();
    }
    startProtection();
}

void UKUISMServer::clientSetProgram(UKUISMClient *client)
{
    //kde根据窗管开始设置client属性来确定窗管已经启动完成，ukui-session中不需要通过这种方式来确定，暂时注释这一段
//    if (isWM(client)) {
//        qCDebug(UKUISMSERVER) << "windowManager loaded, wm set property";
//    }
}

void UKUISMServer::ioError(IceConn iceConn)
{
    //找出iceConn包含的信息，为什么一定需要这个
}

void *UKUISMServer::watchConnection(IceConn iceConn)
{
    UKUISMConnection *conn = new UKUISMConnection(iceConn);
    connect(conn, &UKUISMConnection::activated, this, &UKUISMServer::processData);
    return (void*)conn;
}

void UKUISMServer::restoreWM(const QString &sessionName)
{
    if (m_state != Idle) return;

    m_state = LaunchingWM;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    m_sessionGroup = QLatin1String("Session: ") + sessionName;
    KConfigGroup configSessionGroup(config, m_sessionGroup);

    //如果以后要加上恢复会话功能，m_appsToStart会被用到
    int count = configSessionGroup.readEntry("count", 0);
    m_appsToStart = count;

    //以下这段是从保存的会话文件中寻找wm的重启命令
    QList<QStringList> wmStartCommands;
    if (!m_wm.isEmpty()) {
        for (int i = 1; i <= count; i++) {
            QString n = QString::number(i);
            if (isWM(configSessionGroup.readEntry(QStringLiteral("program") + n, QString()))) {
                wmStartCommands << configSessionGroup.readEntry(QStringLiteral("restartCommand") + n, QStringList());
            }
        }
    }

    if (wmStartCommands.isEmpty()) {
        wmStartCommands << m_wmCommands;
    }

    launchWM(wmStartCommands);
}

void UKUISMServer::startDefaultSession()
{
    if (m_state != Idle ) {
        return;
    }

    m_state = LaunchingWM;

    m_sessionGroup = QString();
    launchWM(QList<QStringList>() << m_wmCommands);
}

void UKUISMServer::shutdown()
{
    //保存关机
    qCDebug(UKUISMSERVER) << "begin performlogout";
    performLogout();
}

QStringList listFileList()
{
    QDir dir("/etc/ukui/ukui-session/logout/");
    if (!dir.exists()) {
          qWarning("Cannot find the example directory");
          return QStringList();
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QStringList filelist;

    for (int i = list.size() - 1 ; i >= 0 ; --i) {
        QFileInfo fileInfo = list.at(i);
        filelist<<fileInfo.fileName();
    }
    return filelist;
}

void execPro(QStringList proList)
{
    for (int i = 0 ; i < proList.size(); ++i){
        qDebug() << "script name is " << proList.at(i);
        QString pro = "/etc/ukui/ukui-session/logout/" + proList.at(i);
        QProcess::startDetached(pro ,QStringList());
    }
}

void UKUISMServer::performLogout()
{
    //已经在执行关机，直接返回
    if (m_state >= Shutdown) {
        qCDebug(UKUISMSERVER) << "already perform Logout";
        return;
    }

    deleteInactiveClients();

    m_kwinInterface->setState(KWinSessionState::Saving);
    m_state = Shutdown;
    m_saveSession = true;

    if (m_saveSession) {
        m_sessionGroup = QStringLiteral("Session: ") + QString::fromLocal8Bit(SESSION_PREVIOUS_LOGOUT);
    }

    m_wmPhase1WaitingCount = 0;
    m_saveType = SmSaveBoth;

    //将该函数的调用放到注销开始阶段
    //9a0的机器上session是由kwin启动的，所以kwin不会注册到ukuismserver中，注销的时候也不会先保存kwin，而是直接向所有客户端发送保存信号
    changeClientOrder();

    startProtection();
    foreach (UKUISMClient *c, m_clients) {
        c->resetState();
        if (isWM(c)) {
            ++m_wmPhase1WaitingCount;
        }
    }

    if (m_wmPhase1WaitingCount > 0) {
        foreach (UKUISMClient *c, m_clients) {
            //先向窗管发送保存自身的信号
            if (isWM(c)) {
                qCDebug(UKUISMSERVER) << "sending saveyourself to wm first";
                SmsSaveYourself(c->connection(), m_saveType, true, SmInteractStyleAny, false);
            }

        }
    } else {
        foreach (UKUISMClient *c, m_clients) {
            qCDebug(UKUISMSERVER) << "sending saveourself to client " << " " << c->clientId();
            SmsSaveYourself(c->connection(), m_saveType, true, SmInteractStyleAny, false);
        }

    }

    if (m_clients.isEmpty()) {
        completeShutdownOrCheckpoint();
    }

    return;
}

void UKUISMServer::cleanUp()
{
    if (clean) {
        return;
    }

    clean = true;
    IceFreeListenObjs(numTransports, listenObjs);

    QByteArray fName = QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + QLatin1Char('/') + QStringLiteral("UKUISMServer"));

    QString  display = QString::fromLocal8Bit(::getenv("DISPLAY"));

    display.remove(QRegExp(QStringLiteral("\\.[0-9]+$")));
    int i;
    while ((i = display.indexOf(QLatin1Char(':'))) >= 0) {
         display[i] = '_';
    }
    while ((i = display.indexOf(QLatin1Char('/'))) >= 0) {
         display[i] = '_';
    }

    fName += '_' + display.toLocal8Bit();
    ::unlink(fName.data());

    FreeAuthenticationData(numTransports, authDataEntries);
}

bool UKUISMServer::closeSession()
{
    Q_ASSERT(calledFromDBus());
    qCDebug(UKUISMSERVER) << "begin logout";
    performLogout();
    qCDebug(UKUISMSERVER) << "Setting D-Bus reply delayed";
    setDelayedReply(true);
    //不清楚这一步是否有意义，但是kde目前是这么做的
    m_performLogoutCall = message();
    return false;
}

void UKUISMServer::newConnection(int socket)
{
    IceAcceptStatus status;
    IceConn iceConn = IceAcceptConnection(((UKUISMListener*)sender())->listenObj, &status);
    if (iceConn == nullptr) return;
    IceSetShutdownNegotiation(iceConn, False);
    IceConnectStatus cstatus;
    while ((cstatus = IceConnectionStatus(iceConn)) == IceConnectPending) {
        (void)IceProcessMessages(iceConn, nullptr, nullptr);
    }

    if (cstatus != IceConnectAccepted) {
        if (cstatus == IceConnectIOError) {
            qCDebug(UKUISMSERVER) << "IO error opening ICE Connection!";
        } else {
            qCDebug(UKUISMSERVER) << "ICE Connection rejected!";
        }

        (void)IceCloseConnection(iceConn);
        return;
    }

    fcntl(IceConnectionNumber(iceConn), F_SETFD, FD_CLOEXEC);
}

void UKUISMServer::processData(int socket)
{
    IceConn iceConn = ((UKUISMConnection*)sender())->iceConn;
    IceProcessMessagesStatus status = IceProcessMessages(iceConn, nullptr, nullptr);
    if (status == IceProcessMessagesIOError) {
        //如果有同类应用再次注册进来就会触发此处
        IceSetShutdownNegotiation(iceConn, False);
        QList<UKUISMClient*>::iterator it = m_clients.begin();
        QList<UKUISMClient*>::iterator const itEnd = m_clients.end();
        //这里做的就是找出发出错误信息的那个client，然后从m_clients中移除掉
        while ((it != itEnd) && *it && (SmsGetIceConnection((*it)->connection()) != iceConn)) {
            ++it;
        }

        if ((it != itEnd) && *it) {
            SmsConn smsConn = (*it)->connection();
            qCDebug(UKUISMSERVER) << "earse duplicate app, or some app close connection";
            deleteClient(*it);
            SmsCleanUp(smsConn);
        }

        (void)IceCloseConnection(iceConn);
    }
}

void UKUISMServer::wmProcessChange()
{
    if (m_state != LaunchingWM) {
        m_wmProcess = nullptr;
        return;
    }

    if (m_wmProcess->state() == QProcess::NotRunning) {
        if (m_restartNum > 10) {
            qCDebug(UKUISMSERVER) << "windowmanager 重启次数用尽";
            return;
        }

        m_restartNum++;
        launchWM(QList<QStringList>() << m_wmCommands);
        qCDebug(UKUISMSERVER) << "windowmanager " << m_wmCommands[0] << "异常退出，重启";

//        if (m_wm == QLatin1String("ukui-kwin_x11")) {
//            return;
//        }
//        m_wm = QStringLiteral("ukui-kwin_x11");
//        m_wmCommands = (QStringList() << QStringLiteral("ukui-kwin_x11"));

//        launchWM(QList<QStringList>() << m_wmCommands);
        return;
    }
}

void UKUISMServer::protectionTimeout()
{
    if ((m_state != Shutdown) || m_clientInteracting) {//如果状态不是Shutdown，或者clientInteracing有值，则条件成立
        //m_clientInteracting有可能为nullptr，所以在这里获取clientid不是很方便
        qCDebug(UKUISMSERVER) << "protectionTimeout: state is " << m_state << "clientInteracting is " << m_clientInteracting << "protectionTimeout returned";
        //如果有一个客户端正在interact,而这里已经超时了，则会直接return，protectiontimer不能用于更改正在interect的客户端的状态。
        //也即protectionTimer只能作用于那些不响应saveyourself信号的客户端，但是现在因为加入了清理不响应信号客户端的机制，所以实际上protectionTimer可能不会再有起作用的机会
        //目前主线取消了超时机制，所以如果有客户端正在交互且超时，应该不需要提醒
//        if (m_clientInteracting) {
//            QString path = m_clientInteracting->program();
//            QString programName = path.mid(path.lastIndexOf(QDir::separator()) + 1);
//            qCDebug(UKUISMSERVER()) << "protectionTimeou: state is" << m_state << "clientInteracting is " << programName << "protectionTimeout returned";
//            QString localName = findLocalName(programName);
//            QString message;
//            message = localName + QObject::tr(" cancel shutdown");
//            bubbleNotify(message);
//        }
        return;
    }

    bool hasAppNotSave = false;
    foreach (UKUISMClient *c, m_clients) {
        if (!c->m_saveYourselfDone && !c->m_waitForPhase2) {//非窗管这类客户端且没有完成保存自身
            qCDebug(UKUISMSERVER) << "protectionTimeout: client " << c->clientId();
            QString path = c->program();
            QString programName = path.mid(path.lastIndexOf(QDir::separator()) + 1);
            qCDebug(UKUISMSERVER()) << "protectionTimeout: client " << programName;
            QString localName = findLocalName(programName);
            // QString message;
            // message = localName + QObject::tr(" not respond");
            // bubbleNotify(message);
            hasAppNotSave = true;
            c->m_saveYourselfDone = true;
        }
    }

    if (hasAppNotSave) {
        qCDebug(UKUISMSERVER()) << "some App not respond, add a time dalay to show the message";
        QTimer::singleShot(3000, [this](){
            qCDebug(UKUISMSERVER()) << "show message time delay reached";
            this->completeShutdownOrCheckpoint();
            this->startProtection();
        });
    } else {
        completeShutdownOrCheckpoint();
        startProtection();
    }

}

void UKUISMServer::timeoutQuit()
{
    qCDebug(UKUISMSERVER()) << "killing count timeout";
//    killWM();
    //杀死客户端阶段有一个2秒的等待，若2秒过后还有客户端没有响应killconnection信号，则会强制调用killwm
    //由于现在改成了不杀死窗管，所以这里的强制操作也改为调用systemd的注销
    //如果一个客户端无法响应interect信号，那他肯定也无法响应killing信号，因此这个客户端一共会造成protectiontimer + killingcount秒的等待
    killingCompleted();
}

void UKUISMServer::timeoutWMQuit()
{
    if (m_state == KillingWM) {
        qCDebug(UKUISMSERVER) << "SmsDie WM timeout";
    }
    killingCompleted();
}

void UKUISMServer::completeShutdownOrCheckpoint()
{
    //只能在shutdown阶段调用
    if (m_state != Shutdown)
        return;

    QList<UKUISMClient*> pendingClients;
    pendingClients = m_clients;
    //此处判断除窗管之外的客户端是否全部完成保存，没有的话就返回
    foreach (UKUISMClient *c, pendingClients ) {
        if (!c->m_saveYourselfDone && !c->m_waitForPhase2) {
            qCDebug(UKUISMSERVER) << c->clientId() << " haven't save";
            return;
        }
    }
    //窗管正在等待phase2阶段的保存，则向其发送保存phase2的信号
    bool waitForPhase2 = false;
    foreach (UKUISMClient *c, pendingClients) {
        if (!c->m_saveYourselfDone && c->m_waitForPhase2) {
            c->m_waitForPhase2 = false;
            qCDebug(UKUISMSERVER) << "sending saveyourselfphase2 to " << c->clientId();
            SmsSaveYourselfPhase2(c->connection());
            waitForPhase2 = true;
        }
    }

    if (waitForPhase2) {
        return;
    }
    //运行到这里说明窗管和普通客户端都完成了保存，开始保存会话信息到磁盘中
    if (m_saveSession) {
        qCDebug(UKUISMSERVER) << "store session informantion in rcfile";
        storeSession();
    }

    //会话信息保存完毕后开始退出，杀死客户端
    if (m_state == Shutdown) {
        m_state = WaitingForKNotify;
        if (m_state == WaitingForKNotify) {
            qCDebug(UKUISMSERVER) << "begin killint client";
            startKilling();
        }
    }
}

void UKUISMServer::storeSession()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    config->reparseConfiguration();//更新配置文件

    KConfigGroup configSessionGroup(config, m_sessionGroup);
    int count =  configSessionGroup.readEntry("count", 0);
    for (int i = 1; i <= count; i++) {
        //读取discardcommand
        QStringList discardCommand = configSessionGroup.readPathEntry(QLatin1String("discardCommand") + QString::number(i), QStringList());
        if (discardCommand.isEmpty()) {
            continue;
        }

        //这一步的目的是寻找是否有重复的discardcommand， 如果没有就直接执行discardcommand
        QList<UKUISMClient*>::iterator it = m_clients.begin();
        QList<UKUISMClient*>::iterator const itEnd = m_clients.end();
        while ((it != itEnd) && *it && (discardCommand != (*it)->discardCommand())) {
            ++it;
        }

        if ((it != itEnd) && *it) {
            continue;
        }

        qCDebug(UKUISMSERVER) << "execute discardCommand " << discardCommand;
        executeCommand(discardCommand);
    }
    //删除上一次保存的会话信息
    config->deleteGroup(m_sessionGroup);
    KConfigGroup cg(config, m_sessionGroup);
    count =  0;
    //将wm移动到客户端列表的第一个
    foreach (UKUISMClient *c, m_clients) {
        if (isWM(c)) {
            m_clients.removeAll(c);
            m_clients.prepend(c);
            break;
        }
    }

    //遍历每一个客户端，存储客户端的信息到文件中
    foreach (UKUISMClient *c, m_clients) {
        int restartHint = c->restartStyleHint();
        if (restartHint == SmRestartNever) {
            continue;
        }

        QString program = c->program();
        QStringList restartCommand = c->restartCommand();
        if (program.isEmpty() && restartCommand.isEmpty()) {
           continue;
        }

        count++;
        QString n = QString::number(count);
        cg.writeEntry(QStringLiteral("program") + n, program);
        cg.writeEntry(QStringLiteral("clientId") + n, c->clientId());
        cg.writeEntry(QStringLiteral("restartCommand") + n, restartCommand);
        cg.writePathEntry(QStringLiteral("discardCommand") + n, c->discardCommand());
        cg.writeEntry(QStringLiteral("restartStyleHint") + n, restartHint);
        cg.writeEntry(QStringLiteral("userId") + n, c->userId());
        cg.writeEntry(QStringLiteral("wasWm") + n, isWM(c));
    }
    cg.writeEntry( "count", count );

    config->sync();
}

void UKUISMServer::completeKilling()
{
    if (m_state == Killing) {
        //这一段的含义是只要客户端列表中还有非窗管的客户端存在，则等待，直到客户端中只有一个窗管，则开始杀死窗管
        bool wait = false;
        foreach (UKUISMClient *c, m_clients) {
            if (isWM(c)) {
                continue;
            }
            wait = true;
        }

        if (wait) {
            return;
        }

        //killWM();
        //修改为不杀死窗管，直接结束会话
        killingCompleted();
    }
}

void UKUISMServer::startKilling()
{
    if (m_state == Killing) {
        return;
    }

    m_state = Killing;

    m_kwinInterface->setState(KWinSessionState::Quitting);

    foreach (UKUISMClient *c, m_clients) {
        if (isWM(c)) {//最后再杀死窗管
            continue;
        }
        qCDebug(UKUISMSERVER) << c->clientId() << "kill connection";
        SmsDie(c->connection());
    }

    completeKilling();
    qCDebug(UKUISMSERVER()) << "begin killing signal timeout count";
    QTimer::singleShot(2000, this, &UKUISMServer::timeoutQuit);
}

void UKUISMServer::killWM()
{
    if (m_state != Killing) {
        return;
    }

    m_state = KillingWM;
    bool iswm = false;
    foreach (UKUISMClient *c, m_clients) {
        if (isWM(c)) {
            iswm = true;
            qCDebug(UKUISMSERVER) << "wm kill connection";
            SmsDie(c->connection());
        }
    }

    if (iswm) {
        completeKillingWM();
        //5秒后窗管没有杀死，就直接终止程序
        QTimer::singleShot(5000, this, &UKUISMServer::timeoutWMQuit);
    } else {
        killingCompleted();
    }

}

void UKUISMServer::completeKillingWM()
{
    if (m_state == KillingWM) {
        if (m_clients.isEmpty()) {
            killingCompleted();
        }
    }
}

void UKUISMServer::killingCompleted()
{
    //这里构造d-bus的返回值，此处返回true，说明注销过程正常完成，用来通知plasma-session的关机接口，注销保存已经完成，可以开始关机
    if (m_performLogoutCall.type() == QDBusMessage::MethodCallMessage) {
        auto reply = m_performLogoutCall.createReply(true);
        qCDebug(UKUISMSERVER) << "sending D-Bus reply to plasma shutdown";
        QDBusConnection::sessionBus().send(reply);
        m_performLogoutCall = QDBusMessage();
    }
    //qApp->quit();

    m_state = Idle;
    m_clientInteracting = nullptr;
    m_saveSession = false;

    m_clients.clear();
    m_activeClients.clear();

//    emit logoutFinished();
//    //目前不清楚如果不做清理会有什么影响，看日志没有发现问题，使用上也没有区别，但为了保险还是加上
//    cleanUp();
}

void UKUISMServer::cancelShutdown(UKUISMClient *c)
{
    m_clientInteracting = nullptr;

    foreach (UKUISMClient *c, m_clients) {
        qCDebug(UKUISMSERVER) << "sending cancel shutdown to client " << c->clientId();
        SmsShutdownCancelled(c->connection());
        if (c->m_saveYourselfDone) {
            QStringList discard = c->discardCommand();
            if (!discard.isEmpty()) {
                qCDebug(UKUISMSERVER) << c->clientId() << "discard saveing state, discardCommand is " << discard;
                executeCommand(discard);
            }
        }
        c->resetState();
    }

    m_state = Idle;
    //取消注销会向D-Bus调用方返回值，此处返回false,说明注销被取消，保证不会被强制注销
    m_kwinInterface->setState(KWinSessionState::Normal);

    if (m_performLogoutCall.type() == QDBusMessage::MethodCallMessage) {
        auto reply = m_performLogoutCall.createReply(false);
        QDBusConnection::sessionBus().send(reply);
        m_performLogoutCall = QDBusMessage();
    }
}

KProcess *UKUISMServer::startApplication(const QStringList &command, bool wm)
{
    if (wm) {
        KProcess *process = new KProcess(this);
        qCDebug(UKUISMSERVER) << "the wm start command is " << command;
        *process << command;
        //应该需要在此处更新m_wm，因为m_wm需要在后面用来判断应用是否是窗管
        m_wm = command[0];
        connect(process, static_cast<void (KProcess::*)(QProcess::ProcessError)>(&KProcess::error), process, &KProcess::deleteLater);
        connect(process, static_cast<void (KProcess::*)(int, QProcess::ExitStatus)>(&KProcess::finished), process, &KProcess::deleteLater);
        process->start();
        return process;
    } else {
        qCDebug(UKUISMSERVER) << "The Restart Command is :" << command;
        int n = command.count();
        QString app = command[0];
        QStringList argList;
        for (int i = 1; i < n; i++) {
            argList.append(command[i]);
        }

        //调用ukui-session的接口启动应用
        QDBusInterface *sessionInterface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                                              "org.gnome.SessionManager", QDBusConnection::sessionBus(), this);

        if (!sessionInterface->isValid()) {
            qWarning() << "dbusCall: Session QDBusInterface is invalid";
            return nullptr;
        }

        QList<QVariant> argumentList;
        QVariant appVar(app);
        QVariant arguVar(argList);
        argumentList.append(appVar);
        argumentList.append(arguVar);

        sessionInterface->callWithArgumentList(QDBus::CallMode::NoBlock, "startApp", argumentList);

        return nullptr;
    }
}

bool UKUISMServer::restoreApplication(const QStringList &command)
{
    qCDebug(UKUISMSERVER) << "The Restart Command is :" << command;
    int n = command.count();
    QString app = command[0];
    QStringList argList;
    for (int i = 1; i < n; i++) {
        argList.append(command[i]);
    }

    //调用ukui-session的接口启动应用
    QDBusInterface *sessionInterface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                                          "org.gnome.SessionManager", QDBusConnection::sessionBus(), this);

    if (!sessionInterface->isValid()) {
        qWarning() << "dbusCall: Session QDBusInterface is invalid";
        return false;
    }

    QList<QVariant> argumentList;
    QVariant appVar(app);
    QVariant arguVar(argList);
    argumentList.append(appVar);
    argumentList.append(arguVar);

    //如果要使用dbus接口的返回值，则应设置为阻塞的模式
    QDBusReply<bool> reply = sessionInterface->call(QDBus::CallMode::Block, "startApp", appVar, arguVar);

    if (!(reply.isValid())) {
        qCDebug(UKUISMSERVER) << "call startApp getting error " << reply.error().name();
    }

    if (reply.value()) {
        qCDebug(UKUISMSERVER) << "success restore " << app << " and the reply is " << reply.value();
        return true;
    }

    qCDebug(UKUISMSERVER) << "can not restore " << app;

    return false;
}

void UKUISMServer::executeCommand(const QStringList &command)
{
    if (command.isEmpty()) {
        return;
    }

    KProcess::execute(command);
}

void UKUISMServer::handlePendingInteractions()
{
    //该函数在保存退出阶段，第一次进入该函数时，clientInteracting一定是nullptr
    if (m_clientInteracting) {
        return;
    }
    //遍历客户端，找到第一个正在挂起的客户端，将其初始化为m_clientInteracting
    foreach (UKUISMClient *c, m_clients) {
        if (c->m_pendingInteraction) {
            m_clientInteracting = c;
            c->m_pendingInteraction = false;
            break;
        }
    }
    //向m_clientInteracting授予交互权限
    if (m_clientInteracting) {
        endProtection();
        qCDebug(UKUISMSERVER) << "sending interact to " << m_clientInteracting->clientId();
        SmsInteract(m_clientInteracting->connection());
    } else {
        qCDebug(UKUISMSERVER) << "no more client is pending";
        startProtection();
    }
}

void UKUISMServer::startProtection()
{
    //m_protectionTimer用于在第一个客户端获得交互权限前的定时保护作用，若存在一个进程处于忙碌状态，不处理我们对其发送的保存自身信号
    //则会造成服务器一直等待该进程的响应，当定时器走完的时候，我们直接将所有客户端的状态设置为保存完毕，然后进行下一阶段的保存
    m_protectionTimer.setSingleShot(true);
    m_protectionTimer.start(5000);
    qCDebug(UKUISMSERVER) << "start protectionTimer";
}

void UKUISMServer::endProtection()
{
    m_protectionTimer.stop();
}

void UKUISMServer::launchWM(const QList<QStringList> &wmStartCommands)
{
    assert(m_state == LaunchingWM);

    if (!(qEnvironmentVariableIsSet("WAYLAND_DISPLAY") || qEnvironmentVariableIsSet("WAYLAND_SOCKET"))) {
        m_wmProcess = startApplication(wmStartCommands[0], true);
        connect(m_wmProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(wmProcessChange()));
        connect(m_wmProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(wmProcessChange()));
    }
}

void UKUISMServer::tryRestoreNext()
{
    if (m_state != Restoring) {
        return;
    }

    m_restoreTimer.stop();
    KConfigGroup config(KSharedConfig::openConfig(), m_sessionGroup);

    while (m_appRestored < m_appsToStart) {
        //最开始m_appRestored为0,而rc文件中为0的为kwin，kwin已经被处理过，所以这里将++放在前，等于从id为1开始处理
        QString n = QString::number(++m_appRestored);
        QString clientId = config.readEntry(QLatin1String("clientId") + n, QString());
        QString clientName = config.readEntry(QLatin1String("program") + n, QString());

        bool alreadyStarted = false;
        foreach (UKUISMClient *c, m_clients) {
            if (QString::fromLocal8Bit(c->clientId()) == clientId) {
                qCDebug(UKUISMSERVER) << c->program() << " is already started";
                alreadyStarted = true;
                break;
            } else if (c->program() == clientName) {
                qCDebug(UKUISMSERVER) << c->program() << " already started";
                alreadyStarted = true;
                break;
            }
        }

        if (alreadyStarted) {
            continue;
        }

        QStringList restartCommand = config.readEntry(QLatin1String("restartCommand") + n, QStringList());

        if (restartCommand.isEmpty() || (config.readEntry(QStringLiteral("restartStyleHint") + n, 0) == SmRestartNever)) {
            continue;
        }

        if (isWM(config.readEntry( QStringLiteral("program") + n, QString()))) {
            continue;
        }
        if (config.readEntry(QStringLiteral("wasWm") + n, false)) {
            continue;
        }

        if (restoreApplication(restartCommand)) {
            m_lastIdRestore = clientId;
            if (!m_lastIdRestore.isEmpty()) {
                m_restoreTimer.setSingleShot(true);
                m_restoreTimer.start(2000);
                return;
            }
        } else {
            QCoreApplication::processEvents();
            continue;
        }
    }

    qCDebug(UKUISMSERVER) << "finish restore all clients";
    m_appRestored = 0;
    m_lastIdRestore.clear();

    m_state = Idle;
}

bool UKUISMServer::isWM(const UKUISMClient *client) const
{
    return isWM(client->program());
}

bool UKUISMServer::isWM(const QString &program) const
{
    //m_wm一般情况下是个完整的二进制程序路径，此处获得程序名，如果不是完整二进制路径，获取到的也是程序名，
    QString wmName = m_wm.mid(m_wm.lastIndexOf(QDir::separator()) + 1);
    QString programName = program.mid(program.lastIndexOf(QDir::separator()) + 1);
    return programName == wmName;
}

void UKUISMServer::changeClientOrder()
{
    //桌面和控制栏等没有实现xsmp信号保存的应用会在interect done后发过来一个保存完成的信号，然后不等server的kill connection信号
    //自己退出，所以将他们从客户端列表中去掉。
    //这只是一个暂时解决的方法，应该由desktop和panel做出修改，在收到服务器的退出信号后才退出进程
    //经过测试，即使将桌面和任务栏放到最后杀死，任然会出现桌面和任务栏先消失，应用还在的情况，要防止这种情况
    //要么在storeSession后直接调用systemd的注销接口，要么让桌面和任务栏按照xsmp规范修改。
    foreach (UKUISMClient *c, m_clients) {
        QString programPath = c->program();
        QString programName = programPath.mid(programPath.lastIndexOf(QDir::separator()) + 1);
        if (programName == QLatin1String("ukui-panel")) {
            m_clients.removeAll(c);
            //改为任务栏不退出
//            m_clients.append(c);
        } else if (programName == QLatin1String("peony-qt-desktop")) {
            m_clients.removeAll(c);
            //改为桌面不退出
//            m_clients.append(c);
        } else if (programName == QLatin1String("ukui-menu")) {
            m_clients.removeAll(c);
            m_clients.append(c);
        } else if (programName == QLatin1String("ukui-tablet-desktop")) {
            //平板桌面的进程
            m_clients.removeAll(c);
        } else if (programName == QLatin1String("et")) {
            //wps表格
            if (!isWPSWinActive(programName)) {
                m_clients.removeAll(c);
            } else {
                m_clients.removeAll(c);
                m_clients.prepend(c);
            }
        } else if (programName == QLatin1String("wps")) {
            //wps文字
            if (!isWPSWinActive(programName)) {
                m_clients.removeAll(c);
            } else {
                m_clients.removeAll(c);
                m_clients.prepend(c);
            }
        } else if (programName == QLatin1String("wpp")) {
            //wps演示
            if (!isWPSWinActive(programName)) {
                m_clients.removeAll(c);
            } else {
                m_clients.removeAll(c);
                m_clients.prepend(c);
            }
        }
    }
}

bool UKUISMServer::syncDBusEnvironment()
{
    QString program;
    QStringList args = {QStringLiteral("--systemd"), QStringLiteral("--all")};;
    QStringList env;
    QProcess p;

    if (!QStandardPaths::findExecutable(QStringLiteral("dbus-update-activation-environment")).isEmpty()) {
        program = "dbus-update-activation-environment";
    }
    p.setEnvironment(QProcess::systemEnvironment() << env);
    p.setProcessChannelMode(QProcess::ForwardedChannels);

    if (!program.isEmpty()) {
        p.start(program, args);
    } else {
        qWarning() << "dbus-update-activation-environment don't exist";
        return false;
    }

    p.waitForFinished(-1);//等待程序执行完成
    if (p.exitCode()) {
        qWarning() << program << args << "exited with code" << p.exitCode();
    }
    return p.exitCode() == 0;//QProcess::NormalExit	0   QProcess::CrashExit	1
}

void UKUISMServer::syncSessionEnv(const QString &key, const QString &value)
{
    QDBusInterface *sessionInterface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                                          "org.gnome.SessionManager", QDBusConnection::sessionBus(), this);

    if (!sessionInterface->isValid()) {
        qWarning() << "dbusCall: Session QDBusInterface is invalid";
        return;
    }

    QList<QVariant> argumentList;
    QVariant envKey(key);
    QVariant envValue(value);
    argumentList.append(envKey);
    argumentList.append(envValue);

    qCDebug(UKUISMSERVER) << "sync env to ukui-session";
    QDBusMessage msg = sessionInterface->callWithArgumentList(QDBus::CallMode::NoBlock, "setSessionEnv", argumentList);
    QString error = msg.errorMessage();
    if (!error.isEmpty()) {
        qCDebug(UKUISMSERVER) << "the error is " << error;
    }
}

bool UKUISMServer::isWPSWinActive(const QString &wpsAPP)
{
    bool isActive = false;
    FILE *fp = nullptr;
    const char *command = nullptr;

    //反斜杠注意前面还要加一个反斜杠转义
    //xwininfo -id `xwininfo -tree -root |grep -E "WPS表格|WPS 表格|WPS Spreadsheets|Spreadsheets"|awk '{ print $1 }'`|grep "Map\s\State"|awk '{ print $3 }'
    if (wpsAPP == "et") {
        command = "xwininfo -id `xwininfo -tree -root |grep -E \"WPS表格|WPS 表格|WPS Spreadsheets|Spreadsheets\"|awk '{ print $1 }'`|grep \"Map\\s\\State\"|awk '{ print $3 }'";
    } else if (wpsAPP == "wps") {
        command = "xwininfo -id `xwininfo -tree -root |grep -E \"WPS文字|WPS 文字|WPS Writer|Writer\"|awk '{ print $1 }'`|grep \"Map\\s\\State\"|awk '{ print $3 }'";
    } else if (wpsAPP == "wpp") {
        command = "xwininfo -id `xwininfo -tree -root |grep -E \"WPS演示|WPS 演示|WPS Presentation|Presentation\"|awk '{ print $1 }'`|grep \"Map\\s\\State\"|awk '{ print $3 }'";
    }

    fp = popen(command, "r");

    char c;
    QString str;
    while (!feof(fp)) {
        c = fgetc(fp);
        try  {
            str += c;
        } catch (std::bad_alloc&) {
            str.clear();
            qDebug() << "loop error";
            break;
        };
    }

    qCDebug(UKUISMSERVER()) << str;
    if (str.isEmpty()) {
        qCDebug(UKUISMSERVER()) << wpsAPP << "未启动";
    } else if (str.contains(QString("IsUnMapped"))) {
        qCDebug(UKUISMSERVER()) << wpsAPP << "启动但窗口已关闭";
    } else if (str.contains(QString("IsViewable"))) {
        qCDebug(UKUISMSERVER()) << wpsAPP << "已启动且窗口开启";
        isActive = true;
    }

    pclose(fp);

    return isActive;
}

void UKUISMServer::deleteInactiveClients()
{
    for (const auto &iter : m_activeClients) {
        if (iter.second == 0) {
            //会导致崩溃?
//            qCDebug(UKUISMSERVER) << "m_clients remove inactive client " << iter.first->program();
            m_clients.removeAll(iter.first);
        }
    }
}

void UKUISMServer::bubbleNotify(QString &message)
{
    QDBusInterface face("org.freedesktop.Notifications",
                        "/org/freedesktop/Notifications",
                        "org.freedesktop.Notifications",
                        QDBusConnection::sessionBus());

    QString name = QObject::tr("ukui-session");
    uint id = 0;
    QString icon = "";
    QString title = QObject::tr("Tips");
    QVariantMap hints;
    //    QString desktopFileName = QGuiApplication::desktopFileName();
    //    hints.insert(QStringLiteral("desktop-entry"), desktopFileName);
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(name) << QVariant::fromValue(id) << QVariant::fromValue(icon) << QVariant::fromValue(title)
                 << QVariant::fromValue(message) << QVariant::fromValue(QStringList()) << QVariant::fromValue(hints) << QVariant::fromValue(3000);

    QDBusPendingCall reply = face.asyncCallWithArgumentList(QStringLiteral("Notify"), argumentList);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished,[](){
        qCDebug(UKUISMSERVER()) << "notify call finished";
    });
}

QString UKUISMServer::findLocalName(QString &name)
{
    QString localName;
    QStringList desktop_paths = {"/usr/share/applications", "/etc/xdg/autostart"};

    for (const QString &dirName : const_cast<const QStringList&>(desktop_paths)) {
        QDir dir(dirName);
        if (!dir.exists()) {
            continue;
        }

        const QFileInfoList files = dir.entryInfoList(QStringList(QLatin1String("*.desktop")), QDir::Files | QDir::Readable);
        for (const QFileInfo &fi : files) {
            XdgDesktopFile desktopFile;
            desktopFile.load(fi.absoluteFilePath());
            //不再使用basename判断，使用命令判断
            //QString base = fi.baseName();
            if (desktopFile.expandExecString().isEmpty()) continue;
            QString command = desktopFile.expandExecString().takeFirst();
            if (command == NULL) {
                continue;
            }
            QStringList list = command.split("/");
            QString exec = list.last();
            if (exec == name) {
                localName = desktopFile.localizedValue("Name").toString();
                //localName有可能不存在，所以要做判断
                if (!localName.isEmpty()) {
                    return localName;
                }
            }
//            QString fName = fi.fileName();
//            if (fName == name || fName.contains(name)) {
//                qCDebug(UKUISMSERVER()) << "name is " << name << " and fileName is " << fName;
//                XdgDesktopFile desktopFile;
//                desktopFile.load(fi.absoluteFilePath());
//                //只有Name与进程名相同，才能证明是这个应用的desktop文件，但是某些应用的进程名和desktopfile中的Name不一样
//                //例如wps
//                QString engName = desktopFile.value("Name").toString();
////                qCDebug(UKUISMSERVER()) << "engName is " << engName;
//                if (engName == name || areStringsEqualIgnoreCase(engName, name)) {
//                    localName = desktopFile.localizedValue("Name").toString();
//                    //localName有可能不存在，所以要做判断
//                    if (!localName.isEmpty()) {
//                        return localName;
//                    }
//                }
//            }
        }
    }

    //运行到这一步，说明没找到localname,那就只能使用原来的进程名
    return name;
}

bool UKUISMServer::areStringsEqualIgnoreCase(const QString &str1, const QString &str2)
{
    if (str1.length() != str2.length()) {
        return false;
    }

    std::string lowerStr1(str1.toLocal8Bit().constData());
    std::string lowerStr2(str2.toLocal8Bit().constData());
    std::transform(lowerStr1.begin(), lowerStr1.end(), lowerStr1.begin(), [](unsigned char c) { return std::tolower(c); });
    std::transform(lowerStr2.begin(), lowerStr2.end(), lowerStr2.begin(), [](unsigned char c) { return std::tolower(c); });
    return (lowerStr1 == lowerStr2);
}

void UKUISMServer::removeConnection(UKUISMConnection *conn)
{
    delete conn;
}

void UKUISMServer::restoreSession()
{
    qCDebug(UKUISMSERVER) << "begin restore session";
    m_appRestored = 0;
    m_lastIdRestore.clear();
    m_state = Restoring;

    tryRestoreNext();
}

bool UKUISMServer::isCloseSession()
{
    qCDebug(UKUISMSERVER) << "m_state = " << m_state;
    return m_state >= Shutdown;
}
