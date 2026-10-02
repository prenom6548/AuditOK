//
// Created by sqp on 2022/6/22.
//

#ifndef KYLIN_CONNECTIVITY_UNIX_UTIL_H
#define KYLIN_CONNECTIVITY_UNIX_UTIL_H

namespace KDiscovery
{
enum MessageType {
    REQUEST = 1,
    RESPONSE = 2,

    DEVICE_UUID = 3,
    DEVICE_TYPE = 4,
    DEVICE_NAME = 5,
    DEVICE_IP = 6,
    DEVICE_PORT = 7,
};

enum RequestType {
    START_DISCOVERY = 1,
    STOP_DISCOVERY = 2,
};

enum ReponseType { DEVICE_INFO = 1 };

enum ErrorType {
    SOCKET_CREATE_ERROR = -1,
    BIND_ERROR = -2,
    LISTEN_ERROR = -3,
    ACCEPT_ERROR = -4,
    CONNECT_ERROR = -5,
    SEND_ERROR = -6,
    RECV_ERROR = -7
};
} // namespace KDiscovery

#endif // KYLIN_CONNECTIVITY_UNIX_UTIL_H
