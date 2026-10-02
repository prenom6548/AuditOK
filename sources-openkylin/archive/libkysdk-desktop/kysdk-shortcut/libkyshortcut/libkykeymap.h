/*
 * libkysdk-shortcut's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef LIBKEYMAP_H
#define LIBKEYMAP_H

/** @defgroup ShortcutModule
  * @{
  */

/**
 * @file libkykeymap.h
 * @author tianshaoshuai (tianshaoshuai@kylinos.cn)
 * @brief 提供快捷键模块按键定义
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025
 * @since 3.0.1.0
 */

enum KDK_KEY
{
    KDK_Key_Escape = 0x01000000, // misc keys
    KDK_Key_Tab = 0x01000001,
    KDK_Key_Backtab = 0x01000002,
    KDK_Key_Backspace = 0x01000003,
    KDK_Key_Return = 0x01000004,
    KDK_Key_Enter = 0x01000005,
    KDK_Key_Insert = 0x01000006,
    KDK_Key_Delete = 0x01000007,
    KDK_Key_Pause = 0x01000008,
    KDK_Key_Print = 0x01000009, // print screen
    KDK_Key_SysReq = 0x0100000a,
    KDK_Key_Clear = 0x0100000b,
    KDK_Key_Home = 0x01000010, // cursor movement
    KDK_Key_End = 0x01000011,
    KDK_Key_Left = 0x01000012,
    KDK_Key_Up = 0x01000013,
    KDK_Key_Right = 0x01000014,
    KDK_Key_Down = 0x01000015,
    KDK_Key_PageUp = 0x01000016,
    KDK_Key_PageDown = 0x01000017,
    KDK_Key_Shift = 0x01000020, // modifiers
    KDK_Key_Control = 0x01000021,
    KDK_Key_Meta = 0x01000022,
    KDK_Key_Alt = 0x01000023,
    KDK_Key_CapsLock = 0x01000024,
    KDK_Key_NumLock = 0x01000025,
    KDK_Key_ScrollLock = 0x01000026,
    KDK_Key_F1 = 0x01000030, // function keys
    KDK_Key_F2 = 0x01000031,
    KDK_Key_F3 = 0x01000032,
    KDK_Key_F4 = 0x01000033,
    KDK_Key_F5 = 0x01000034,
    KDK_Key_F6 = 0x01000035,
    KDK_Key_F7 = 0x01000036,
    KDK_Key_F8 = 0x01000037,
    KDK_Key_F9 = 0x01000038,
    KDK_Key_F10 = 0x01000039,
    KDK_Key_F11 = 0x0100003a,
    KDK_Key_F12 = 0x0100003b,
    KDK_Key_F13 = 0x0100003c,
    KDK_Key_F14 = 0x0100003d,
    KDK_Key_F15 = 0x0100003e,
    KDK_Key_F16 = 0x0100003f,
    KDK_Key_F17 = 0x01000040,
    KDK_Key_F18 = 0x01000041,
    KDK_Key_F19 = 0x01000042,
    KDK_Key_F20 = 0x01000043,
    KDK_Key_F21 = 0x01000044,
    KDK_Key_F22 = 0x01000045,
    KDK_Key_F23 = 0x01000046,
    KDK_Key_F24 = 0x01000047,
    KDK_Key_F25 = 0x01000048, // F25 .. F35 only on X11
    KDK_Key_F26 = 0x01000049,
    KDK_Key_F27 = 0x0100004a,
    KDK_Key_F28 = 0x0100004b,
    KDK_Key_F29 = 0x0100004c,
    KDK_Key_F30 = 0x0100004d,
    KDK_Key_F31 = 0x0100004e,
    KDK_Key_F32 = 0x0100004f,
    KDK_Key_F33 = 0x01000050,
    KDK_Key_F34 = 0x01000051,
    KDK_Key_F35 = 0x01000052,
    KDK_Key_Super_L = 0x01000053, // extra keys
    KDK_Key_Super_R = 0x01000054,
    KDK_Key_Menu = 0x01000055,
    KDK_Key_Hyper_L = 0x01000056,
    KDK_Key_Hyper_R = 0x01000057,
    KDK_Key_Help = 0x01000058,
    KDK_Key_Direction_L = 0x01000059,
    KDK_Key_Direction_R = 0x01000060,
    KDK_Key_Space = 0x20, // 7 bit printable ASCII
    KDK_Key_Any = KDK_Key_Escape,
    KDK_Key_Exclam = 0x21,
    KDK_Key_QuoteDbl = 0x22,
    KDK_Key_NumberSign = 0x23,
    KDK_Key_Dollar = 0x24,
    KDK_Key_Percent = 0x25,
    KDK_Key_Ampersand = 0x26,
    KDK_Key_Apostrophe = 0x27,
    KDK_Key_ParenLeft = 0x28,
    KDK_Key_ParenRight = 0x29,
    KDK_Key_Asterisk = 0x2a,
    KDK_Key_Plus = 0x2b,
    KDK_Key_Comma = 0x2c,
    KDK_Key_Minus = 0x2d,
    KDK_Key_Period = 0x2e,
    KDK_Key_Slash = 0x2f,
    KDK_Key_0 = 0x30,
    KDK_Key_1 = 0x31,
    KDK_Key_2 = 0x32,
    KDK_Key_3 = 0x33,
    KDK_Key_4 = 0x34,
    KDK_Key_5 = 0x35,
    KDK_Key_6 = 0x36,
    KDK_Key_7 = 0x37,
    KDK_Key_8 = 0x38,
    KDK_Key_9 = 0x39,
    KDK_Key_Colon = 0x3a,
    KDK_Key_Semicolon = 0x3b,
    KDK_Key_Less = 0x3c,
    KDK_Key_Equal = 0x3d,
    KDK_Key_Greater = 0x3e,
    KDK_Key_Question = 0x3f,
    KDK_Key_At = 0x40,
    KDK_Key_A = 0x41,
    KDK_Key_B = 0x42,
    KDK_Key_C = 0x43,
    KDK_Key_D = 0x44,
    KDK_Key_E = 0x45,
    KDK_Key_F = 0x46,
    KDK_Key_G = 0x47,
    KDK_Key_H = 0x48,
    KDK_Key_I = 0x49,
    KDK_Key_J = 0x4a,
    KDK_Key_K = 0x4b,
    KDK_Key_L = 0x4c,
    KDK_Key_M = 0x4d,
    KDK_Key_N = 0x4e,
    KDK_Key_O = 0x4f,
    KDK_Key_P = 0x50,
    KDK_Key_Q = 0x51,
    KDK_Key_R = 0x52,
    KDK_Key_S = 0x53,
    KDK_Key_T = 0x54,
    KDK_Key_U = 0x55,
    KDK_Key_V = 0x56,
    KDK_Key_W = 0x57,
    KDK_Key_X = 0x58,
    KDK_Key_Y = 0x59,
    KDK_Key_Z = 0x5a,
    KDK_Key_BracketLeft = 0x5b,
    KDK_Key_Backslash = 0x5c,
    KDK_Key_BracketRight = 0x5d,
    KDK_Key_AsciiCircum = 0x5e,
    KDK_Key_Underscore = 0x5f,
    KDK_Key_QuoteLeft = 0x60,
    KDK_Key_BraceLeft = 0x7b,
    KDK_Key_Bar = 0x7c,
    KDK_Key_BraceRight = 0x7d,
    KDK_Key_AsciiTilde = 0x7e,

    KDK_Key_nobreakspace = 0x0a0,
    KDK_Key_exclamdown = 0x0a1,
    KDK_Key_cent = 0x0a2,
    KDK_Key_sterling = 0x0a3,
    KDK_Key_currency = 0x0a4,
    KDK_Key_yen = 0x0a5,
    KDK_Key_brokenbar = 0x0a6,
    KDK_Key_section = 0x0a7,
    KDK_Key_diaeresis = 0x0a8,
    KDK_Key_copyright = 0x0a9,
    KDK_Key_ordfeminine = 0x0aa,
    KDK_Key_guillemotleft = 0x0ab, // left angle quotation mark
    KDK_Key_notsign = 0x0ac,
    KDK_Key_hyphen = 0x0ad,
    KDK_Key_registered = 0x0ae,
    KDK_Key_macron = 0x0af,
    KDK_Key_degree = 0x0b0,
    KDK_Key_plusminus = 0x0b1,
    KDK_Key_twosuperior = 0x0b2,
    KDK_Key_threesuperior = 0x0b3,
    KDK_Key_acute = 0x0b4,
    KDK_Key_mu = 0x0b5,
    KDK_Key_paragraph = 0x0b6,
    KDK_Key_periodcentered = 0x0b7,
    KDK_Key_cedilla = 0x0b8,
    KDK_Key_onesuperior = 0x0b9,
    KDK_Key_masculine = 0x0ba,
    KDK_Key_guillemotright = 0x0bb, // right angle quotation mark
    KDK_Key_onequarter = 0x0bc,
    KDK_Key_onehalf = 0x0bd,
    KDK_Key_threequarters = 0x0be,
    KDK_Key_questiondown = 0x0bf,
    KDK_Key_Agrave = 0x0c0,
    KDK_Key_Aacute = 0x0c1,
    KDK_Key_Acircumflex = 0x0c2,
    KDK_Key_Atilde = 0x0c3,
    KDK_Key_Adiaeresis = 0x0c4,
    KDK_Key_Aring = 0x0c5,
    KDK_Key_AE = 0x0c6,
    KDK_Key_Ccedilla = 0x0c7,
    KDK_Key_Egrave = 0x0c8,
    KDK_Key_Eacute = 0x0c9,
    KDK_Key_Ecircumflex = 0x0ca,
    KDK_Key_Ediaeresis = 0x0cb,
    KDK_Key_Igrave = 0x0cc,
    KDK_Key_Iacute = 0x0cd,
    KDK_Key_Icircumflex = 0x0ce,
    KDK_Key_Idiaeresis = 0x0cf,
    KDK_Key_ETH = 0x0d0,
    KDK_Key_Ntilde = 0x0d1,
    KDK_Key_Ograve = 0x0d2,
    KDK_Key_Oacute = 0x0d3,
    KDK_Key_Ocircumflex = 0x0d4,
    KDK_Key_Otilde = 0x0d5,
    KDK_Key_Odiaeresis = 0x0d6,
    KDK_Key_multiply = 0x0d7,
    KDK_Key_Ooblique = 0x0d8,
    KDK_Key_Ugrave = 0x0d9,
    KDK_Key_Uacute = 0x0da,
    KDK_Key_Ucircumflex = 0x0db,
    KDK_Key_Udiaeresis = 0x0dc,
    KDK_Key_Yacute = 0x0dd,
    KDK_Key_THORN = 0x0de,
    KDK_Key_ssharp = 0x0df,
    KDK_Key_division = 0x0f7,
    KDK_Key_ydiaeresis = 0x0ff,

    // International input method support (X keycode - 0xEE00, the
    // definition follows Qt/Embedded 2.3.7) Only interesting if
    // you are writing your own input method

    // International & multi-key character composition
    KDK_Key_AltGr = 0x01001103,
    KDK_Key_Multi_key = 0x01001120, // Multi-key character compose
    KDK_Key_Codeinput = 0x01001137,
    KDK_Key_SingleCandidate = 0x0100113c,
    KDK_Key_MultipleCandidate = 0x0100113d,
    KDK_Key_PreviousCandidate = 0x0100113e,

    // Misc Functions
    KDK_Key_Mode_switch = 0x0100117e, // Character set switch
    // Key_script_switch       = 0x0100117e,  // Alias for mode_switch

    // Japanese keyboard support
    KDK_Key_Kanji = 0x01001121,    // Kanji, Kanji convert
    KDK_Key_Muhenkan = 0x01001122, // Cancel Conversion
    // Key_Henkan_Mode         = 0x01001123,  // Start/Stop Conversion
    KDK_Key_Henkan = 0x01001123,            // Alias for Henkan_Mode
    KDK_Key_Romaji = 0x01001124,            // to Romaji
    KDK_Key_Hiragana = 0x01001125,          // to Hiragana
    KDK_Key_Katakana = 0x01001126,          // to Katakana
    KDK_Key_Hiragana_Katakana = 0x01001127, // Hiragana/Katakana toggle
    KDK_Key_Zenkaku = 0x01001128,           // to Zenkaku
    KDK_Key_Hankaku = 0x01001129,           // to Hankaku
    KDK_Key_Zenkaku_Hankaku = 0x0100112a,   // Zenkaku/Hankaku toggle
    KDK_Key_Touroku = 0x0100112b,           // Add to Dictionary
    KDK_Key_Massyo = 0x0100112c,            // Delete from Dictionary
    KDK_Key_Kana_Lock = 0x0100112d,         // Kana Lock
    KDK_Key_Kana_Shift = 0x0100112e,        // Kana Shift
    KDK_Key_Eisu_Shift = 0x0100112f,        // Alphanumeric Shift
    KDK_Key_Eisu_toggle = 0x01001130,       // Alphanumeric toggle
    // Key_Kanji_Bangou        = 0x01001137,  // Codeinput
    // Key_Zen_Koho            = 0x0100113d,  // Multiple/All Candidate(s)
    // Key_Mae_Koho            = 0x0100113e,  // Previous Candidate

    // Korean keyboard support
    //
    // In fact, many Korean users need only 2 keys, Key_Hangul and
    // Key_Hangul_Hanja. But rest of the keys are good for future.

    KDK_Key_Hangul = 0x01001131,        // Hangul start/stop(toggle)
    KDK_Key_Hangul_Start = 0x01001132,  // Hangul start
    KDK_Key_Hangul_End = 0x01001133,    // Hangul end, English start
    KDK_Key_Hangul_Hanja = 0x01001134,  // Start Hangul->Hanja Conversion
    KDK_Key_Hangul_Jamo = 0x01001135,   // Hangul Jamo mode
    KDK_Key_Hangul_Romaja = 0x01001136, // Hangul Romaja mode
    // Key_Hangul_Codeinput    = 0x01001137,  // Hangul code input mode
    KDK_Key_Hangul_Jeonja = 0x01001138,    // Jeonja mode
    KDK_Key_Hangul_Banja = 0x01001139,     // Banja mode
    KDK_Key_Hangul_PreHanja = 0x0100113a,  // Pre Hanja conversion
    KDK_Key_Hangul_PostHanja = 0x0100113b, // Post Hanja conversion
    // Key_Hangul_SingleCandidate   = 0x0100113c,  // Single candidate
    // Key_Hangul_MultipleCandidate = 0x0100113d,  // Multiple candidate
    // Key_Hangul_PreviousCandidate = 0x0100113e,  // Previous candidate
    KDK_Key_Hangul_Special = 0x0100113f, // Special symbols
    // Key_Hangul_switch       = 0x0100117e,  // Alias for mode_switch

    // dead keys (X keycode - 0xED00 to avoid the conflict)
    KDK_Key_Dead_Grave = 0x01001250,
    KDK_Key_Dead_Acute = 0x01001251,
    KDK_Key_Dead_Circumflex = 0x01001252,
    KDK_Key_Dead_Tilde = 0x01001253,
    KDK_Key_Dead_Macron = 0x01001254,
    KDK_Key_Dead_Breve = 0x01001255,
    KDK_Key_Dead_Abovedot = 0x01001256,
    KDK_Key_Dead_Diaeresis = 0x01001257,
    KDK_Key_Dead_Abovering = 0x01001258,
    KDK_Key_Dead_Doubleacute = 0x01001259,
    KDK_Key_Dead_Caron = 0x0100125a,
    KDK_Key_Dead_Cedilla = 0x0100125b,
    KDK_Key_Dead_Ogonek = 0x0100125c,
    KDK_Key_Dead_Iota = 0x0100125d,
    KDK_Key_Dead_Voiced_Sound = 0x0100125e,
    KDK_Key_Dead_Semivoiced_Sound = 0x0100125f,
    KDK_Key_Dead_Belowdot = 0x01001260,
    KDK_Key_Dead_Hook = 0x01001261,
    KDK_Key_Dead_Horn = 0x01001262,
    KDK_Key_Dead_Stroke = 0x01001263,
    KDK_Key_Dead_Abovecomma = 0x01001264,
    KDK_Key_Dead_Abovereversedcomma = 0x01001265,
    KDK_Key_Dead_Doublegrave = 0x01001266,
    KDK_Key_Dead_Belowring = 0x01001267,
    KDK_Key_Dead_Belowmacron = 0x01001268,
    KDK_Key_Dead_Belowcircumflex = 0x01001269,
    KDK_Key_Dead_Belowtilde = 0x0100126a,
    KDK_Key_Dead_Belowbreve = 0x0100126b,
    KDK_Key_Dead_Belowdiaeresis = 0x0100126c,
    KDK_Key_Dead_Invertedbreve = 0x0100126d,
    KDK_Key_Dead_Belowcomma = 0x0100126e,
    KDK_Key_Dead_Currency = 0x0100126f,
    KDK_Key_Dead_a = 0x01001280,
    KDK_Key_Dead_A = 0x01001281,
    KDK_Key_Dead_e = 0x01001282,
    KDK_Key_Dead_E = 0x01001283,
    KDK_Key_Dead_i = 0x01001284,
    KDK_Key_Dead_I = 0x01001285,
    KDK_Key_Dead_o = 0x01001286,
    KDK_Key_Dead_O = 0x01001287,
    KDK_Key_Dead_u = 0x01001288,
    KDK_Key_Dead_U = 0x01001289,
    KDK_Key_Dead_Small_Schwa = 0x0100128a,
    KDK_Key_Dead_Capital_Schwa = 0x0100128b,
    KDK_Key_Dead_Greek = 0x0100128c,
    KDK_Key_Dead_Lowline = 0x01001290,
    KDK_Key_Dead_Aboveverticalline = 0x01001291,
    KDK_Key_Dead_Belowverticalline = 0x01001292,
    KDK_Key_Dead_Longsolidusoverlay = 0x01001293,

    // multimedia/internet keys - ignored by default - see QKeyEvent c'tor
    KDK_Key_Back = 0x01000061,
    KDK_Key_Forward = 0x01000062,
    KDK_Key_Stop = 0x01000063,
    KDK_Key_Refresh = 0x01000064,
    KDK_Key_VolumeDown = 0x01000070,
    KDK_Key_VolumeMute = 0x01000071,
    KDK_Key_VolumeUp = 0x01000072,
    KDK_Key_BassBoost = 0x01000073,
    KDK_Key_BassUp = 0x01000074,
    KDK_Key_BassDown = 0x01000075,
    KDK_Key_TrebleUp = 0x01000076,
    KDK_Key_TrebleDown = 0x01000077,
    KDK_Key_MediaPlay = 0x01000080,
    KDK_Key_MediaStop = 0x01000081,
    KDK_Key_MediaPrevious = 0x01000082,
    KDK_Key_MediaNext = 0x01000083,
    KDK_Key_MediaRecord = 0x01000084,
    KDK_Key_MediaPause = 0x1000085,
    KDK_Key_MediaTogglePlayPause = 0x1000086,
    KDK_Key_HomePage = 0x01000090,
    KDK_Key_Favorites = 0x01000091,
    KDK_Key_Search = 0x01000092,
    KDK_Key_Standby = 0x01000093,
    KDK_Key_OpenUrl = 0x01000094,
    KDK_Key_LaunchMail = 0x010000a0,
    KDK_Key_LaunchMedia = 0x010000a1,
    KDK_Key_Launch0 = 0x010000a2,
    KDK_Key_Launch1 = 0x010000a3,
    KDK_Key_Launch2 = 0x010000a4,
    KDK_Key_Launch3 = 0x010000a5,
    KDK_Key_Launch4 = 0x010000a6,
    KDK_Key_Launch5 = 0x010000a7,
    KDK_Key_Launch6 = 0x010000a8,
    KDK_Key_Launch7 = 0x010000a9,
    KDK_Key_Launch8 = 0x010000aa,
    KDK_Key_Launch9 = 0x010000ab,
    KDK_Key_LaunchA = 0x010000ac,
    KDK_Key_LaunchB = 0x010000ad,
    KDK_Key_LaunchC = 0x010000ae,
    KDK_Key_LaunchD = 0x010000af,
    KDK_Key_LaunchE = 0x010000b0,
    KDK_Key_LaunchF = 0x010000b1,
    KDK_Key_MonBrightnessUp = 0x010000b2,
    KDK_Key_MonBrightnessDown = 0x010000b3,
    KDK_Key_KeyboardLightOnOff = 0x010000b4,
    KDK_Key_KeyboardBrightnessUp = 0x010000b5,
    KDK_Key_KeyboardBrightnessDown = 0x010000b6,
    KDK_Key_PowerOff = 0x010000b7,
    KDK_Key_WakeUp = 0x010000b8,
    KDK_Key_Eject = 0x010000b9,
    KDK_Key_ScreenSaver = 0x010000ba,
    KDK_Key_WWW = 0x010000bb,
    KDK_Key_Memo = 0x010000bc,
    KDK_Key_LightBulb = 0x010000bd,
    KDK_Key_Shop = 0x010000be,
    KDK_Key_History = 0x010000bf,
    KDK_Key_AddFavorite = 0x010000c0,
    KDK_Key_HotLinks = 0x010000c1,
    KDK_Key_BrightnessAdjust = 0x010000c2,
    KDK_Key_Finance = 0x010000c3,
    KDK_Key_Community = 0x010000c4,
    KDK_Key_AudioRewind = 0x010000c5, // Media rewind
    KDK_Key_BackForward = 0x010000c6,
    KDK_Key_ApplicationLeft = 0x010000c7,
    KDK_Key_ApplicationRight = 0x010000c8,
    KDK_Key_Book = 0x010000c9,
    KDK_Key_CD = 0x010000ca,
    KDK_Key_Calculator = 0x010000cb,
    KDK_Key_ToDoList = 0x010000cc,
    KDK_Key_ClearGrab = 0x010000cd,
    KDK_Key_Close = 0x010000ce,
    KDK_Key_Copy = 0x010000cf,
    KDK_Key_Cut = 0x010000d0,
    KDK_Key_Display = 0x010000d1, // Output switch key
    KDK_Key_DOS = 0x010000d2,
    KDK_Key_Documents = 0x010000d3,
    KDK_Key_Excel = 0x010000d4,
    KDK_Key_Explorer = 0x010000d5,
    KDK_Key_Game = 0x010000d6,
    KDK_Key_Go = 0x010000d7,
    KDK_Key_iTouch = 0x010000d8,
    KDK_Key_LogOff = 0x010000d9,
    KDK_Key_Market = 0x010000da,
    KDK_Key_Meeting = 0x010000db,
    KDK_Key_MenuKB = 0x010000dc,
    KDK_Key_MenuPB = 0x010000dd,
    KDK_Key_MySites = 0x010000de,
    KDK_Key_News = 0x010000df,
    KDK_Key_OfficeHome = 0x010000e0,
    KDK_Key_Option = 0x010000e1,
    KDK_Key_Paste = 0x010000e2,
    KDK_Key_Phone = 0x010000e3,
    KDK_Key_Calendar = 0x010000e4,
    KDK_Key_Reply = 0x010000e5,
    KDK_Key_Reload = 0x010000e6,
    KDK_Key_RotateWindows = 0x010000e7,
    KDK_Key_RotationPB = 0x010000e8,
    KDK_Key_RotationKB = 0x010000e9,
    KDK_Key_Save = 0x010000ea,
    KDK_Key_Send = 0x010000eb,
    KDK_Key_Spell = 0x010000ec,
    KDK_Key_SplitScreen = 0x010000ed,
    KDK_Key_Support = 0x010000ee,
    KDK_Key_TaskPane = 0x010000ef,
    KDK_Key_Terminal = 0x010000f0,
    KDK_Key_Tools = 0x010000f1,
    KDK_Key_Travel = 0x010000f2,
    KDK_Key_Video = 0x010000f3,
    KDK_Key_Word = 0x010000f4,
    KDK_Key_Xfer = 0x010000f5,
    KDK_Key_ZoomIn = 0x010000f6,
    KDK_Key_ZoomOut = 0x010000f7,
    KDK_Key_Away = 0x010000f8,
    KDK_Key_Messenger = 0x010000f9,
    KDK_Key_WebCam = 0x010000fa,
    KDK_Key_MailForward = 0x010000fb,
    KDK_Key_Pictures = 0x010000fc,
    KDK_Key_Music = 0x010000fd,
    KDK_Key_Battery = 0x010000fe,
    KDK_Key_Bluetooth = 0x010000ff,
    KDK_Key_WLAN = 0x01000100,
    KDK_Key_UWB = 0x01000101,
    KDK_Key_AudioForward = 0x01000102,    // Media fast-forward
    KDK_Key_AudioRepeat = 0x01000103,     // Toggle repeat mode
    KDK_Key_AudioRandomPlay = 0x01000104, // Toggle shuffle mode
    KDK_Key_Subtitle = 0x01000105,
    KDK_Key_AudioCycleTrack = 0x01000106,
    KDK_Key_Time = 0x01000107,
    KDK_Key_Hibernate = 0x01000108,
    KDK_Key_View = 0x01000109,
    KDK_Key_TopMenu = 0x0100010a,
    KDK_Key_PowerDown = 0x0100010b,
    KDK_Key_Suspend = 0x0100010c,
    KDK_Key_ContrastAdjust = 0x0100010d,

    KDK_Key_LaunchG = 0x0100010e,
    KDK_Key_LaunchH = 0x0100010f,

    KDK_Key_TouchpadToggle = 0x01000110,
    KDK_Key_TouchpadOn = 0x01000111,
    KDK_Key_TouchpadOff = 0x01000112,

    KDK_Key_MicMute = 0x01000113,

    KDK_Key_Red = 0x01000114,
    KDK_Key_Green = 0x01000115,
    KDK_Key_Yellow = 0x01000116,
    KDK_Key_Blue = 0x01000117,

    KDK_Key_ChannelUp = 0x01000118,
    KDK_Key_ChannelDown = 0x01000119,

    KDK_Key_Guide = 0x0100011a,
    KDK_Key_Info = 0x0100011b,
    KDK_Key_Settings = 0x0100011c,

    KDK_Key_MicVolumeUp = 0x0100011d,
    KDK_Key_MicVolumeDown = 0x0100011e,

    KDK_Key_New = 0x01000120,
    KDK_Key_Open = 0x01000121,
    KDK_Key_Find = 0x01000122,
    KDK_Key_Undo = 0x01000123,
    KDK_Key_Redo = 0x01000124,
    KDK_Key_RFKill = 0x01000125,
    KDK_Key_MediaLast = 0x0100ffff,

    // Keypad navigation keys
    KDK_Key_Select = 0x01010000,
    KDK_Key_Yes = 0x01010001,
    KDK_Key_No = 0x01010002,

    // Newer misc keys
    KDK_Key_Cancel = 0x01020001,
    KDK_Key_Printer = 0x01020002,
    KDK_Key_Execute = 0x01020003,
    KDK_Key_Sleep = 0x01020004,
    KDK_Key_Play = 0x01020005, // Not the same as Key_MediaPlay
    KDK_Key_Zoom = 0x01020006,
    // Key_Jisho   = 0x01020007, // IME: Dictionary key
    // Key_Oyayubi_Left = 0x01020008, // IME: Left Oyayubi key
    // Key_Oyayubi_Right = 0x01020009, // IME: Right Oyayubi key
    KDK_Key_Exit = 0x0102000a,

    // Device keys
    KDK_Key_Context1 = 0x01100000,
    KDK_Key_Context2 = 0x01100001,
    KDK_Key_Context3 = 0x01100002,
    KDK_Key_Context4 = 0x01100003,
    KDK_Key_Call = 0x01100004,   // set absolute state to in a call (do not toggle state)
    KDK_Key_Hangup = 0x01100005, // set absolute state to hang up (do not toggle state)
    KDK_Key_Flip = 0x01100006,
    KDK_Key_ToggleCallHangup = 0x01100007, // a toggle key for answering, or hanging up, based on current call state
    KDK_Key_VoiceDial = 0x01100008,
    KDK_Key_LastNumberRedial = 0x01100009,

    KDK_Key_Camera = 0x01100020,
    KDK_Key_CameraFocus = 0x01100021,

    KDK_Key_unknown = 0x01ffffff
};

enum KDK_KeyboardModifier
{
    KDK_NoModifier = 0x00000000,
    KDK_ShiftModifier = 0x02000000,
    KDK_ControlModifier = 0x04000000,
    KDK_AltModifier = 0x08000000,
    KDK_MetaModifier = 0x10000000,
    KDK_KeypadModifier = 0x20000000,
    KDK_GroupSwitchModifier = 0x40000000,
    // Do not extend the mask to include 0x01000000
    KDK_KeyboardModifierMask = 0xfe000000
};

// 控制键
enum KDK_Modifier
{
    KDK_META = KDK_MetaModifier,
    KDK_SHIFT = KDK_ShiftModifier,
    KDK_CTRL = KDK_ControlModifier,
    KDK_ALT = KDK_AltModifier,
    KDK_MODIFIER_MASK = KDK_KeyboardModifierMask,
    KDK_UNICODE_ACCEL = 0x00000000
};

#endif // LIBKEYMAP_H
