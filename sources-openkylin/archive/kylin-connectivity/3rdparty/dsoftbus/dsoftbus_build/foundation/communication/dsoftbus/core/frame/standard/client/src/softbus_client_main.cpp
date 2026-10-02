//
// Created by sqp on 2022/6/23.
//

#include "discunixserver.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "unix-util.h"
#include "discunixdeviceinfo.h"
#include <thread>

#include "securec.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "discovery_service.h"
#include "discunixdeviceinfo.h"

using namespace tlv;

static DiscUnixServer *g_server = nullptr;

const int BUF_LEN = 1024;
const char *SERVER_UNIX_SOCKET_ADDR = "/tmp/kylin.deviceDiscover.socket";
#define QLEN 10
char buf[BUF_LEN];

#define TEST_ASSERT_TRUE(ret)                                                                                          \
    if (ret) {                                                                                                         \
        LOG_INFO("[test][succ]\n");                                                                                    \
        g_succTestCount++;                                                                                             \
    } else {                                                                                                           \
        LOG_INFO("[test][error]\n");                                                                                   \
        g_failTestCount++;                                                                                             \
    }

static int g_succTestCount = 0;
static int g_failTestCount = 0;
static int g_devieceFoundCount = 0;
#define S_OK 0

#define DISC_PUBLISH_ID 8
#define DISC_SUBSCRIBE_ID 9
#define NETWORKIDSIZE 100
#define TYPE_SEND_BYTE 15
#define TYPE_SEND_MESSAGE 16

static char const *g_pkgName = "com.communication.kylin.dsoftbus";
char const *g_sessionName = "com.communication.kylin.dsoftbus.JtCreateSessionServerLimit";
char const *g_groupid = "TEST_GROUP_ID";

#define LNN_DISC_CAPABILITY "ddmpCapability"

static SubscribeInfo g_disc_sInfo = {.subscribeId = DISC_SUBSCRIBE_ID,
                                     .mode = DISCOVER_MODE_ACTIVE,
                                     .medium = COAP,
                                     .freq = HIGH,
                                     .isSameAccount = false,
                                     .isWakeRemote = false,
                                     .capability = LNN_DISC_CAPABILITY,
                                     .capabilityData = (unsigned char *)LNN_DISC_CAPABILITY,
                                     .dataLen = strlen(LNN_DISC_CAPABILITY) + 1

};

static void DiscDeviceFound(const DeviceInfo *device)
{
    g_devieceFoundCount++;
    // if (client->isConnected())
    // {
    //     client->sendDevInfo((char *)device->devId, device->devType, (char *)device->devName, (char
    //     *)device->addr[0].info.ip.ip, device->addr[0].info.ip.port);
    // }

    g_server->sendDevInfo((char *)device->devId, device->devType, (char *)device->devName,
                          (char *)device->addr[0].info.ip.ip, device->addr[0].info.ip.port);

    LOG_INFO("[TestCallback]DiscDeviceFound success! index = %d, device info devId=%s, hwAccountHash=%s, devType=%d, "
             "devName=%s,  capabilityBitMap=%d, addrNum=%d\n",
             g_devieceFoundCount, device->devId, device->hwAccountHash, device->devType, device->devName,
             device->capabilityBitmap[0], device->addrNum);
}

static void DiscoverFailed(int subscribeId, DiscoveryFailReason failReason)
{
    LOG_INFO("[TestCallback]DiscoverFailed! subscribeId = %d, failReason = %d\n", subscribeId, failReason);
}

static void DiscoverySuccess(int subscribeId)
{
    LOG_INFO("[TestCallback]DiscoverySuccess! subscribeId = %d \n", subscribeId);
}

static IDiscoveryCallback g_subscribeCb = {.OnDeviceFound = DiscDeviceFound,
                                           .OnDiscoverFailed = DiscoverFailed,
                                           .OnDiscoverySuccess = DiscoverySuccess};

static int32_t DiscStartDiscovery()
{
    g_devieceFoundCount = 0;
    int ret = StartDiscovery(g_pkgName, &g_disc_sInfo, &g_subscribeCb);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t DiscStopDiscovery()
{
    int ret = StopDiscovery(g_pkgName, g_disc_sInfo.subscribeId);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

DiscUnixServer::DiscUnixServer() {}

DiscUnixServer::~DiscUnixServer() {}

int DiscUnixServer::serverListen()
{
    struct sockaddr_un serverAddr;

    //创建本地domain套接字
    if ((m_serverFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("server create socket error! errno = %d\n", errno);
        return KDiscovery::SOCKET_CREATE_ERROR;
    }

    // bind API 函数将允许地址的立即重用
    int on = 1;
    setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int nNetTimeout = 2000; // 2秒
    //设置发送时限
    setsockopt(m_serverFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
    //设置接收时限
    setsockopt(m_serverFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));

    //删除套接字文件，避免因文件存在导致bind()绑定失败
    unlink(SERVER_UNIX_SOCKET_ADDR);

    //初始化套接字结构体地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, SERVER_UNIX_SOCKET_ADDR);

    if (bind(m_serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        close(m_serverFd);
        printf("server bind error!errno = %d\n", errno);
        return KDiscovery::BIND_ERROR;
    }

    if (listen(m_serverFd, QLEN) < 0) { //告知内核这是一个服务器进程
        close(m_serverFd);
        printf("server listen error! errno = %d\n", errno);
        return KDiscovery::LISTEN_ERROR;
    }
    return 1;
}

int DiscUnixServer::serverAccept()
{
    struct sockaddr_un acceptAddr;
    socklen_t socklen = sizeof(acceptAddr);
    if ((m_clientFd = accept(m_serverFd, (struct sockaddr *)&acceptAddr, &socklen)) < 0) {
        printf("Accept failed , errno = %d", errno);
        return KDiscovery::ACCEPT_ERROR;
    }
    //到此成功获取路径名
    unlink(acceptAddr.sun_path);
    return m_clientFd;
}

void DiscUnixServer::startServer()
{
    if (serverListen() < 0) {
        return;
    }

    printf("start server... \n");

    fd_set fdsr; //文件描述符集的定义

    while (true) {

        FD_ZERO(&fdsr); //每次进入循环都重建描述符集
        FD_SET(m_serverFd, &fdsr);
        FD_SET(m_clientFd, &fdsr);

        int ret = select(FD_SETSIZE, &fdsr, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

        if (ret < 0) {
            perror("select");
            break;
        }

        // check every fd in the set
        if (FD_ISSET(m_clientFd, &fdsr)) {
            ret = recv(m_clientFd, buf, sizeof(buf), 0);
            if (ret <= 0) { // client close
                printf("client close");
                close(m_clientFd);
                FD_CLR(m_clientFd, &fdsr);
                m_clientFd = 0;
            } else { // receive data
                printf("client[%d] send:%s\n", m_clientFd, buf);
                getRequest(buf, ret);
            }
        }

        // check whether a new connection comes
        if (FD_ISSET(m_serverFd, &fdsr)) {
            serverAccept();
        }
    }
}

void DiscUnixServer::stopServer()
{
    close(m_clientFd);
    close(m_serverFd);
}

int DiscUnixServer::sendDevInfo(char *uuid, const int devType, char *deName, char *devIp, int devPort)
{
    DiscoveryDeviceInfo deviceInfo;

    deviceInfo.PutResponseValue(KDiscovery::RESPONSE, KDiscovery::DEVICE_INFO);
    deviceInfo.PutUuidValue(KDiscovery::DEVICE_UUID, uuid);
    deviceInfo.PutDevTypeValue(KDiscovery::DEVICE_TYPE, devType);
    deviceInfo.PutDevNameValue(KDiscovery::DEVICE_NAME, deName);
    deviceInfo.PutDevIpValue(KDiscovery::DEVICE_IP, devIp);
    deviceInfo.PutDevPortValue(KDiscovery::DEVICE_PORT, devPort);

    if (!deviceInfo.Serialize()) {
        std::cout << "deviceInfo Serialize Failed !\n";
        return -1;
    }

    DiscoveryDeviceInfo deviceInfoAll;
    deviceInfoAll.PutObjectValue(KDiscovery::RESPONSE, deviceInfo);

    if (!deviceInfoAll.Serialize()) {
        std::cout << "deviceInfoAll Serialize Failed !\n";
        return -1;
    }

    printf("send data %d!\n", deviceInfoAll.GetSerializedBytes());
    if (send(m_clientFd, deviceInfoAll.GetSerializedBuffer(), deviceInfoAll.GetSerializedBytes(), 0) < 0) {
        printf("send data fail!\n");
        return KDiscovery::SEND_ERROR;
    }
    return 1;
}

void DiscUnixServer::getRequest(char *buf, int len)
{
    DiscoveryDeviceInfo parsedBoxes;
    if (!parsedBoxes.Parse((unsigned char *)buf, len)) {
        std::cout << "boxes Parse Failed !\n";
        return;
    }

    std::cout << "boxes Parse Success, " << parsedBoxes.GetSerializedBytes() << " bytes \n";

    DiscoveryDeviceInfo parsedBox;
    if (!parsedBoxes.GetObjectValue(KDiscovery::REQUEST, parsedBox)) {
        std::cout << "GetObjectValue Failed !\n";
        return;
    }

    {
        int value;
        if (!parsedBox.GetRequestValue(KDiscovery::REQUEST, value)) {
            std::cout << "GetIntValue Failed !\n";
            return;
        }

        switch (value) {
        case KDiscovery::START_DISCOVERY: {
            DiscStartDiscovery();
            break;
        }
        case KDiscovery::STOP_DISCOVERY: {
            DiscStopDiscovery();
            break;
        }
        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{

    g_server = new DiscUnixServer();
    g_server->startServer();
}
