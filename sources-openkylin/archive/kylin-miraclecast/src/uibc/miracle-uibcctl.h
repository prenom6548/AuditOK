
#ifndef MIRACLE_UIBCCTL_H
#define MIRACLE_UIBCCTL_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netdb.h>
#include<arpa/inet.h>
#include <math.h>
#include "shl_log.h"

typedef enum {
        EVENT_HID_KEY_TYPE = '0',
        EVENT_HID_MOUSE_TYPE,
}EventType;

typedef enum {
        MANUFACTURER = '5',
	HUAWEI = '6',
	XIAOMI = '7',
	OTHER = '8',
}ManufacturerType;

typedef enum {
        HID_LEFT_BUTTON_PRESSED = '0',
        HID_NO_BUTTON_PRESSED,               
}HIDMouseClickSubEvent;

typedef enum {
        HID_NULL_BUTTON = '0',
        HID_LEFT_BUTTON = '1',
        HID_MID_BUTTON = '2',
        HID_RIGHT_BUTTON = '4',
}HIDMouseButtonValue;

typedef enum {
        GENERIC_TOUCH_DOWN = 0,//left_mouse_down
        GENERIC_TOUCH_UP,//left_mouse_up
        GENERIC_TOUCH_MOVE,//mouse move
        GENERIC_KEY_DOWN,
        GENERIC_KEY_UP,
        GENERIC_ZOOM,
        GENERIC_VERTICAL_SCROLL,
        GENERIC_HORIZONTAL_SCROLL,
        GENERIC_ROTATE,
} MessageType;

typedef enum {
        HID_KEYBOARD = 0,
        HID_MOUSE,
        HID_SIGLE_TOUCH,
} HIDType;

typedef enum {
        HID_MOUSE_PRESSED = 0,
        HID_MOUSE_RELEASED,
        HID_KEY_PRESSED,
        HID_KEY_RELEASED,
}HIDAction;

enum {
        HID_MOUSE_L_BUTTON = 1,
        HID_MOUSE_R_BUTTON = 2,
        HID_MOUSE_M_BUTTON = 4,
};
typedef struct {
   char* m_PacketData;
   size_t m_PacketDataLen;
   bool m_DataValid;
} UibcMessage;

UibcMessage buildUibcMessage(MessageType type, const char* inEventDesc, double widthRatio, double heightRatio);
UibcMessage buildHIDC_UibcMessage(HIDType type, const char* rawMsg, double widthRatio, double heightRatio);

static char** str_split(char* pStr, const char* pDelim, size_t* size);

void getUIBCGenericTouchPacket(const char *inEventDesc, UibcMessage* uibcmessage, double widthRatio, double heightRatio);
void getUIBCGenericKeyPacket(const char *inEventDesc, UibcMessage* uibcmessage);
void getUIBCGenericZoomPacket(const char *inEventDesc,UibcMessage* uibcmessage);
void getUIBCGenericScalePacket(const char *inEventDesc, UibcMessage* uibcmessage);
void getUIBCGenericRotatePacket(const char *inEventDesc, UibcMessage* uibcmessage);

void hexdump(void *_data, size_t len);
void binarydump(void *_data, size_t len);

int sendUibcMessage(UibcMessage* uibcmessage, int sockfd);
#endif
