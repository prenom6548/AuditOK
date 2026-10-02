/* Copyright (C) 2007-2008 The Android Open Source Project
** Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

#pragma once

#include <stdbool.h>                                 // for bool, true, false
#include <stdint.h>                                  // for uint8_t, uint32_t
#include <stdio.h>                                   // for fprintf, NULL
#include <string.h>
#include <stdlib.h>

#define MAX_KEYCODE_COUNTS 64

typedef struct {
    int scancode;
    int linux_keycode;
} KmreLinuxKeyCodeMap;

typedef enum {
    KeyModLCtrl = (1U << 0),     // left-control key
    KeyModRCtrl = (1U << 1),     // right-control
    kKeyModLAlt = (1U << 2),      // left-alt
    KeyModRAlt = (1U << 3),      // right-alt
    KeyModLShift = (1U << 4),    // left-shift
    KeyModRShift = (1U << 5),    // right-shift
    KeyModNumLock = (1U << 6),   // numlock
    KeyModCapsLock = (1U << 7),  // capslock
} KmreKeyMod;

typedef enum {
    EventDefault,
    EventTextInput,
    EventInvalid,
} KeyEventType;

typedef struct {
    uint8_t text[32];
    bool down;
    int32_t keycode;
    int32_t mod;
} TextInputData;

typedef struct {
    KeyEventType type;
    TextInputData text;
} KeyEventData;

typedef struct
{
    int keycode_count;
    int keycodes[MAX_KEYCODE_COUNTS];
} KeycodeBufferData;

struct KmreKeyboard {
    KeycodeBufferData keycode_buffer[1];
};

typedef struct KmreKeyboard KmreKeyboard;


//-----------------android keycodes-----------------
#define KEY_APPSWITCH 580
#define KEY_STEM_PRIMARY 581
#define KEY_STEM_1 582
#define KEY_STEM_2 583
#define KEY_STEM_3 584
//--------------------------------------------------


//-----------------Linux keycodes-----------------
#define LINUX_KEY_RESERVED 0
#define LINUX_KEY_ESC 1
#define LINUX_KEY_1 2
#define LINUX_KEY_2 3
#define LINUX_KEY_3 4
#define LINUX_KEY_4 5
#define LINUX_KEY_5 6
#define LINUX_KEY_6 7
#define LINUX_KEY_7 8
#define LINUX_KEY_8 9
#define LINUX_KEY_9 10
#define LINUX_KEY_0 11
#define LINUX_KEY_MINUS 12
#define LINUX_KEY_EQUAL 13
#define LINUX_KEY_BACKSPACE 14
#define LINUX_KEY_TAB 15
#define LINUX_KEY_Q 16
#define LINUX_KEY_W 17
#define LINUX_KEY_E 18
#define LINUX_KEY_R 19
#define LINUX_KEY_T 20
#define LINUX_KEY_Y 21
#define LINUX_KEY_U 22
#define LINUX_KEY_I 23
#define LINUX_KEY_O 24
#define LINUX_KEY_P 25
#define LINUX_KEY_LEFTBRACE 26
#define LINUX_KEY_RIGHTBRACE 27
#define LINUX_KEY_ENTER 28
#define LINUX_KEY_LEFTCTRL 29
#define LINUX_KEY_A 30
#define LINUX_KEY_S 31
#define LINUX_KEY_D 32
#define LINUX_KEY_F 33
#define LINUX_KEY_G 34
#define LINUX_KEY_H 35
#define LINUX_KEY_J 36
#define LINUX_KEY_K 37
#define LINUX_KEY_L 38
#define LINUX_KEY_SEMICOLON 39
#define LINUX_KEY_APOSTROPHE 40
#define LINUX_KEY_GRAVE 41
#define LINUX_KEY_LEFTSHIFT 42
#define LINUX_KEY_BACKSLASH 43
#define LINUX_KEY_Z 44
#define LINUX_KEY_X 45
#define LINUX_KEY_C 46
#define LINUX_KEY_V 47
#define LINUX_KEY_B 48
#define LINUX_KEY_N 49
#define LINUX_KEY_M 50
#define LINUX_KEY_COMMA 51
#define LINUX_KEY_DOT 52
#define LINUX_KEY_SLASH 53
#define LINUX_KEY_RIGHTSHIFT 54
#define LINUX_KEY_KPASTERISK 55
#define LINUX_KEY_LEFTALT 56
#define LINUX_KEY_SPACE 57
#define LINUX_KEY_CAPSLOCK 58
#define LINUX_KEY_F1 59
#define LINUX_KEY_F2 60
#define LINUX_KEY_F3 61
#define LINUX_KEY_F4 62
#define LINUX_KEY_F5 63
#define LINUX_KEY_F6 64
#define LINUX_KEY_F7 65
#define LINUX_KEY_F8 66
#define LINUX_KEY_F9 67
#define LINUX_KEY_F10 68
#define LINUX_KEY_NUMLOCK 69
#define LINUX_KEY_KP7 71
#define LINUX_KEY_KP8 72
#define LINUX_KEY_KP9 73
#define LINUX_KEY_KPMINUS 74
#define LINUX_KEY_KP4 75
#define LINUX_KEY_KP5 76
#define LINUX_KEY_KP6 77
#define LINUX_KEY_KPPLUS 78
#define LINUX_KEY_KP1 79
#define LINUX_KEY_KP2 80
#define LINUX_KEY_KP3 81
#define LINUX_KEY_KP0 82
#define LINUX_KEY_KPDOT 83

#define LINUX_KEY_KPENTER 96
#define LINUX_KEY_RIGHTCTRL 97
#define LINUX_KEY_KPSLASH 98

#define LINUX_KEY_RIGHTALT 100
#define LINUX_KEY_HOME 102
#define LINUX_KEY_UP 103
#define LINUX_KEY_PAGEUP 104
#define LINUX_KEY_LEFT 105
#define LINUX_KEY_RIGHT 106
#define LINUX_KEY_END 107
#define LINUX_KEY_DOWN 108
#define LINUX_KEY_PAGEDOWN 109
#define LINUX_KEY_INSERT 110
#define LINUX_KEY_DELETE 111

#define LINUX_KEY_POWER 116
#define LINUX_KEY_KPEQUAL 117
#define LINUX_KEY_KPPLUSMINUS 118
#define LINUX_KEY_PAUSE 119

#define LINUX_KEY_KPCOMMA 121

#define LINUX_KEY_LEFTMETA 125
#define LINUX_KEY_RIGHTMETA 126

#define LINUX_KEY_HOMEPAGE 172

#define LINUX_KEY_POWER2 0x164

#define LINUX_KEY_GREEN 0x18f
//--------------------------------------------------
