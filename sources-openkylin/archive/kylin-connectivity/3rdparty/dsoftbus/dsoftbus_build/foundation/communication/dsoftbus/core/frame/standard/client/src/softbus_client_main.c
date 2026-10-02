/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "securec.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "discovery_service.h"

static int g_succTestCount = 0;
static int g_failTestCount = 0;
static int g_devieceFoundCount = 0;

#define TEST_ASSERT_TRUE(ret)  \
if (ret) {                 \
LOG_INFO("[test][succ]\n");    \
g_succTestCount++;       \
} else {                   \
LOG_INFO("[test][error]\n");    \
g_failTestCount++;       \
}

static void Help() {
    printf("************** select the behavior.\n");
    printf("************** 1: [dis] coap publish service.\n");
    printf("************** 2: [dis] coap unpublish service.\n");
    printf("************** 3: [dis] start discovery.\n");
    printf("************** 4: [dis] stop discovery.\n");
    printf("************** 5: [LNN] reg node devices state.\n");
    printf("************** 6: [LNN] unreg node devices state.\n");
    printf("************** 7: [TRAN] create session server.\n");
    printf("************** 8: [TRAN] remove session server.\n");
    printf("************** 9: [TRAN] open session.\n");
    printf("************** a: [TRAN] close session.\n");
    printf("************** b: [TRAN] send bytes.\n");
    printf("************** c: [TRAN] send message.\n");
    printf("************** q: exit.\n\n");
    return;
}

#define S_OK 0

#define DISC_PUBLISH_ID 8
#define DISC_SUBSCRIBE_ID 9
#define NETWORKIDSIZE 100
#define TYPE_SEND_BYTE 15
#define TYPE_SEND_MESSAGE 16

static char const *g_pkgName = "com.communication.kylin.dsoftbus";
char const *g_sessionName = "com.communication.kylin.dsoftbus.JtCreateSessionServerLimit";
static int g_sessionId = 0;
static bool g_state = true;
static char g_networkId[NETWORKIDSIZE];
char const *g_groupid = "TEST_GROUP_ID";

#define LNN_DISC_CAPABILITY "ddmpCapability"

static SessionAttribute g_sessionAttr = {
        .dataType = TYPE_BYTES
};
static char *g_tranData = NULL;

static PublishInfo g_disc_pInfo = {
        .publishId = DISC_PUBLISH_ID,
        .mode = DISCOVER_MODE_ACTIVE,
        .medium = COAP,
        .freq = HIGH,
        .capability = LNN_DISC_CAPABILITY,
        .capabilityData = (unsigned char *)LNN_DISC_CAPABILITY,
        .dataLen = strlen(LNN_DISC_CAPABILITY) + 1
};

static SubscribeInfo g_disc_sInfo = {
        .subscribeId = DISC_SUBSCRIBE_ID,
        .mode = DISCOVER_MODE_ACTIVE,
        .medium = COAP,
        .freq = HIGH,
        .capability = LNN_DISC_CAPABILITY,
        .capabilityData =  (unsigned char *)LNN_DISC_CAPABILITY,
        .dataLen = strlen(LNN_DISC_CAPABILITY) + 1,
        .isSameAccount = false,
        .isWakeRemote = false
};

static void DiscPublishSuccess(int publishId) {
    LOG_INFO("[TestCallback]DiscPublishSuccess! publishId = %d\n", publishId);
}

static void DiscPublishFail(int publishId, PublishFailReason reason) {
    LOG_INFO("[TestCallback]DiscPublishFail!publishId = %d, reason = %d\n", publishId, reason);
}

static IPublishCallback g_publishCb = {
        .OnPublishSuccess = DiscPublishSuccess,
        .OnPublishFail = DiscPublishFail
};

static void DiscDeviceFound(const DeviceInfo *device) {
    g_devieceFoundCount++;
    LOG_INFO("[TestCallback]DiscDeviceFound success! index = %d, device info devId=%s, hwAccountHash=%s, devType=%d, devName=%s,  capabilityBitMap=%d\n",
             g_devieceFoundCount, device->devId, device->hwAccountHash, device->devType, device->devName, device->capabilityBitmap[0]);
}

static void DiscoverFailed(int subscribeId, DiscoveryFailReason failReason) {
    LOG_INFO("[TestCallback]DiscoverFailed! subscribeId = %d, failReason = %d\n", subscribeId, failReason);
}

static void DiscoverySuccess(int subscribeId) {
    LOG_INFO("[TestCallback]DiscoverySuccess! subscribeId = %d \n", subscribeId);
}

static IDiscoveryCallback g_subscribeCb = {
        .OnDeviceFound = DiscDeviceFound,
        .OnDiscoverFailed = DiscoverFailed,
        .OnDiscoverySuccess = DiscoverySuccess
};

static int32_t DiscCoapPulbishService() {
    int ret = PublishService(g_pkgName, &g_disc_pInfo, &g_publishCb);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t UnDiscCoapPulbishService() {
    int ret = UnPublishService(g_pkgName, g_disc_pInfo.publishId);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t DiscStartDiscovery() {
    g_devieceFoundCount = 0;
    int ret = StartDiscovery(g_pkgName, &g_disc_sInfo, &g_subscribeCb);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t DiscStopDiscovery() {
    int ret = StopDiscovery(g_pkgName, g_disc_sInfo.subscribeId);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static void OnNodeOnline(NodeBasicInfo *info) {
    LOG_INFO("[TestCallback]OnNodeOnline! NodeBasicInfo networkId = %s, deviceName = %s, deviceTypeId = %d \n",
             info->networkId, info->deviceName, info->deviceTypeId);
    memcpy(g_networkId, info->networkId, NETWORKIDSIZE);
    return;
}

static void OnNodeOffline(NodeBasicInfo *info) {
    LOG_INFO("[TestCallback]OnNodeOffline!  NodeBasicInfo networkId = %s, deviceName = %s, deviceTypeId = %d \n",
             info->networkId, info->deviceName, info->deviceTypeId);
    memcpy(g_networkId, info->networkId, NETWORKIDSIZE);
    return;
}

static void OnNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info) {
    LOG_INFO(
            "[TestCallback]OnNodeBasicInfoChanged! NodeBasicInfoType = %d, NodeBasicInfo networkId = %s, deviceName = %s, deviceTypeId = %d\n",
            type, info->networkId, info->deviceName, info->deviceTypeId);
    return;
}

static INodeStateCb g_nodeStateCallback = {
        .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED,
        .onNodeOnline = OnNodeOnline,
        .onNodeOffline = OnNodeOffline,
        .onNodeBasicInfoChanged = OnNodeBasicInfoChanged
};

static int32_t LnnRegNodeDeviceState() {
    int ret = RegNodeDeviceStateCb(g_pkgName, &g_nodeStateCallback);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t LnnUnregNodeDeviceState() {
    int ret = UnregNodeDeviceStateCb(&g_nodeStateCallback);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

void Wait() {
    LOG_INFO("[TestCallback]Wait enter");
    SLEEP:
    sleep(1);
    if (g_state == false) {
        goto SLEEP;
    }
    LOG_INFO("[TestCallback]Wait end");
//    g_state = false;
}

void Start() {
    LOG_INFO("[TestCallback]Start\n");
    g_state = true;
}

void Stop() {
    LOG_INFO("[TestCallback]Stop\n");
    g_state = false;
}

static int TranOnSessionOpened(int sessionId, int result) {
    LOG_INFO("[TestCallback]TranOnSession Opened, sesison id = %d, result = %d\n", sessionId, result);
    g_sessionId = sessionId;
    Start();
    return 0;
}

static void TranOnSessionClosed(int sessionId) {
    LOG_INFO("[TestCallback]session closed, session id = %d\n", sessionId);
    g_sessionId = 0;
    Stop();
}

static void TranOnBytesReceived(int sessionId, const void *data, unsigned int len) {
    LOG_INFO("[TestCallback]session bytes received, session id = %d data = %s\n", sessionId, (char *) data);
    Start();
}

static void TranOnMessageReceived(int sessionId, const void *data, unsigned int len) {
    LOG_INFO("[TestCallback]session msg received, session id = %d data = %s, len = %d", sessionId, (char *) data,
             len);
    Start();
}

static ISessionListener g_sessionlistener = {
        .OnSessionOpened = TranOnSessionOpened,
        .OnSessionClosed = TranOnSessionClosed,
        .OnBytesReceived = TranOnBytesReceived,
        .OnMessageReceived = TranOnMessageReceived
};

static int32_t TranCreateSessionServer() {
    int ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionlistener);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t TranRemoveSessionServer() {
    int ret = RemoveSessionServer(g_pkgName, g_sessionName);
    TEST_ASSERT_TRUE(ret == 0);
    return ret;
}

static int32_t TranOpenSession() {
    LOG_INFO("[TranOpenSession] input target networkId");
//    if (scanf_s("%s", g_networkId, NETWORKIDSIZE) < 0) {
//        return SOFTBUS_ERR;
//    }
    LOG_INFO("g_networkId = %s", g_networkId);
    int sessionId = OpenSession(g_sessionName, g_sessionName, g_networkId, g_groupid, &g_sessionAttr);
    LOG_INFO("OpenSession sessionId = %d", sessionId);
    TEST_ASSERT_TRUE(sessionId != 0);
    return S_OK;
}

static int32_t TranCloseSession() {
    LOG_INFO("[TranCloseSession] call");
    CloseSession(g_sessionId);
    return S_OK;
}

static int DataSend(int size, int type) {
    int ret;
    g_tranData = (char *)calloc(1, size * sizeof(char));
    memset(g_tranData, 'x', size);
    Wait();
    if (type == TYPE_SEND_BYTE) {
        LOG_INFO("[Test]DataSend call SendBytes TYPE_SEND_BYTE data = %s, size = %d", g_tranData, size);
        ret = SendBytes(g_sessionId, g_tranData, size);
        TEST_ASSERT_TRUE(ret == 0);
    }
    if (type == TYPE_SEND_MESSAGE) {
        LOG_INFO("[Test]DataSend call SendBytes TYPE_SEND_MESSAGE data = %s, size = %d", g_tranData, size);
        ret = SendMessage(g_sessionId, g_tranData, size);
        TEST_ASSERT_TRUE(ret == 0);
    }
    LOG_INFO("[Test]DataSend ok");
    free(g_tranData);
    return ret;
}

static int32_t TranSendBytes() {
    int ret = DataSend(1, TYPE_SEND_BYTE);
    TEST_ASSERT_TRUE(ret == 0);
    return S_OK;
}

static int32_t TranSendMessage() {
    int ret = DataSend(50, TYPE_SEND_MESSAGE);
    TEST_ASSERT_TRUE(ret == 0);
    return S_OK;
}

int main(int argc, char *argv[]) {
    LOG_INFO("************** start kylin softbus client test sample!!!!!!***************");
    do {
        Help();
        sleep(1);
        LOG_INFO("************** input=");
        char input = getchar();
        getchar();
        LOG_INFO("%c", input);
        switch (input) {
            case '1': {
                if (DiscCoapPulbishService() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '2': {
                if (UnDiscCoapPulbishService() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '3': {
                if (DiscStartDiscovery() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '4': {
                if (DiscStopDiscovery() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '5': {
                if (LnnRegNodeDeviceState() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '6': {
                if (LnnUnregNodeDeviceState() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '7': {
                if (TranCreateSessionServer() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '8': {
                if (TranRemoveSessionServer() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case '9': {
                if (TranOpenSession() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case 'a': {
                if (TranCloseSession() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case 'b': {
                if (TranSendBytes() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case 'c': {
                if (TranSendMessage() != S_OK) {
                    goto EXIT;
                }
                break;
            }
            case 'q':
                goto EXIT;

            default:
                break;
        }
    } while (true);

    EXIT:
    printf("*************quit the coap discovery test sample!!!!!************");
    return 0;
}
