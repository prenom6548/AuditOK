/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMCONTEXT_H__
#define __KMCONTEXT_H__

#include <string>
#include <map>

using namespace std;

typedef enum
{
    KAIMING_POLICY_NONE,
    KAIMING_POLICY_SEE,
    KAIMING_POLICY_TALK,
    KAIMING_POLICY_OWN
} KMPolicy;

typedef enum
{
    KAIMING_CONTEXT_SHARED_NONE = 0,
    KAIMING_CONTEXT_SHARED_NETWORK = 1 << 0,
    KAIMING_CONTEXT_SHARED_IPC = 1 << 1,
} KMContextShares;

typedef enum
{
    KAIMING_CONTEXT_SOCKET_NONE = 0,
    KAIMING_CONTEXT_SOCKET_X11 = 1 << 0,
    KAIMING_CONTEXT_SOCKET_WAYLAND = 1 << 1,
    KAIMING_CONTEXT_SOCKET_PULSEAUDIO = 1 << 2,
    KAIMING_CONTEXT_SOCKET_SESSION_BUS = 1 << 3,
    KAIMING_CONTEXT_SOCKET_SYSTEM_BUS = 1 << 4,
    KAIMING_CONTEXT_SOCKET_FALLBACK_X11 = 1 << 5, /* For backwards compat, also set SOCKET_X11 */
    KAIMING_CONTEXT_SOCKET_SSH_AUTH = 1 << 6,
    KAIMING_CONTEXT_SOCKET_PCSC = 1 << 7,
    KAIMING_CONTEXT_SOCKET_CUPS = 1 << 8,
    KAIMING_CONTEXT_SOCKET_GPG_AGENT = 1 << 9,
} KMContextSockets;

typedef enum
{
    KAIMING_CONTEXT_DEVICE_NONE = 0,
    KAIMING_CONTEXT_DEVICE_DRI = 1 << 0,
    KAIMING_CONTEXT_DEVICE_ALL = 1 << 1,
    KAIMING_CONTEXT_DEVICE_KVM = 1 << 2,
    KAIMING_CONTEXT_DEVICE_SHM = 1 << 3,
} KMContextDevices;

typedef enum
{
    KAIMING_CONTEXT_FEATURE_NONE = 0,
    KAIMING_CONTEXT_FEATURE_DEVEL = 1 << 0,
    KAIMING_CONTEXT_FEATURE_MULTIARCH = 1 << 1,
    KAIMING_CONTEXT_FEATURE_BLUETOOTH = 1 << 2,
    KAIMING_CONTEXT_FEATURE_CANBUS = 1 << 3,
    KAIMING_CONTEXT_FEATURE_PER_APP_DEV_SHM = 1 << 4,
} KMContextFeatures;

typedef enum
{
    KAIMING_FILESYSTEM_MODE_NONE = 0,
    KAIMING_FILESYSTEM_MODE_READ_ONLY = 1,
    KAIMING_FILESYSTEM_MODE_READ_WRITE = 2,
    KAIMING_FILESYSTEM_MODE_CREATE = 3,
    KAIMING_FILESYSTEM_MODE_LAST = KAIMING_FILESYSTEM_MODE_CREATE
} KMFilesystemMode;

/**
 * @brief : 主要用于封装各种参数上下文
 */
class KMContext
{
public:
    KMContext();
    ~KMContext();

    const map<string, string> &getEnvVars() { return m_envVars; }

    void addEnvVar(const std::string &env, const std::string &value);

    void merge(const KMContext &other);

    void resetNonPermissions() { m_envVars.clear(); }

private:

    map<string, string> m_envVars;
};

#endif
