
#include "miracle-uibcctl.h"
#include <netinet/tcp.h>
#include <fcntl.h>
#include <glib.h>
#include <gio/gio.h>

static bool s_bUseHIDC = false;
static int portno = -1;

#define KYLIN_PC_UIBC_SRV_PORT 24605
static char manufacturer = MANUFACTURER;

typedef struct
{
  int32_t pckey;
  int32_t android_key1;
  int32_t android_key2;
}T_KeyMapPc2Android;

T_KeyMapPc2Android g_tkeyMapGeneric[] = {
 //basic  
  {'0', 7, 0},
  {'1', 8, 0},
  {'2', 9, 0},
  {'3', 10, 0},
  {'4', 11, 0},
  {'5', 12, 0},
  {'6', 13, 0},
  {'7', 14, 0},
  {'8', 15, 0},
  {'9', 16, 0},
  {'a', 29, 0},
  {'b', 30, 0},
  {'c', 31, 0},
  {'d', 32, 0},
  {'e', 33, 0},
  {'f', 34, 0},
  {'g', 35, 0},
  {'h', 36, 0},
  {'i', 37, 0},
  {'j', 38, 0},
  {'k', 39, 0},
  {'l', 40, 0},
  {'m', 41, 0},
  {'n', 42, 0},
  {'o', 43, 0},
  {'p', 44, 0},
  {'q', 45, 0},
  {'r', 46, 0},
  {'s', 47, 0},
  {'t', 48, 0},
  {'u', 49, 0},
  {'v', 50, 0},
  {'w', 51, 0},
  {'x', 52, 0},
  {'y', 53, 0},
  {'z', 54, 0},
  
  {'A', 29, 2},//not valid, TODO
  {'Z', 54, 2},//not valid, TODO
//control
  {0x1000004,66,0},//enter
  {0x1000003,67,0},//backspace
  {0x1000001, 61, 0},//tab
  {0x1000000, 111, 0},//esc
  {0x1000007, 112, 0},//delete
  {0x1000024, 115, 0},//CapLock
  {0x1000020, 59, 0},//lshift
  {0x1000020, 60, 0},//rshift
  {0x1000021, 113, 0},//lctrl
  {0x1000021, 114, 0},//rctrl
  {0x1000023, 57, 0},//lalt
  {0x1000023, 58, 0},//ralt
  {0x1000006, 124, 0},//insert
  {0x1000010, 122, 0},//home
  {0x1000011, 123, 0},//end
  // {0xffeb, 123, 0},//win
  // {0xff67, 123, 0},//menu
  {0x1000016, 92, 0},//pageup
  {0x1000017, 93, 0},//pagedown
  {0x1000013, 19, 0},//up
  {0x1000015, 20, 0},//down
  {0x1000012, 21, 0},//left
  {0x1000014, 22, 0},//right

//symbols
  {0x0060, 68, 0},//` grave
  {0x002c, 55, 0},//,
  {0x002e, 56, 0},//.
  {0x002f, 76, 0},// '/'
  {0x005c, 73, 0},// '\'
  {0x005b, 71, 0},// [
  {0x005d, 72, 0},// ]
  {0x0027, 75, 0},// '
  {0x003b, 74, 0},// ;
  {0xffab, 81, 0},// +
  {0x002d, 69, 0},// -
  {0x003d, 70, 0},// =
  {0x0020, 62, 0},// space

  {0x0040, 77, 0},// @
  {0x002a, 17, 0},// *
  {0x0023, 18, 0},// #
  {0x007e, 68, 0},// ~
  {0x1000030,7,0x08}, //F1 HOME
  {0x1000031,43,0x04},//F2 FUN
  {0x1000032,41,0}, // F3 BACK
};


//  T_KeyMapPc2Android g_tkeyMapGeneric[] ={
//    {0x01000003, 8, 0},      //Backspace
//    {0x01000001, 10, 0},     //TAB
//    {0x01000000, 27, 0},
//    {0x20, 32, 0},           //Space
//    {'0', 48, 0},
//    {'1', 49, 0},
//    {'2', 50, 0},
//    {'3', 51, 0},
//    {'4', 52, 0},
//    {'5', 53, 0},
//    {'6', 54, 0},
//    {'7', 55, 0},
//    {'8', 56, 0},
//    {'9', 57, 0},
//    {0x3a, 58, 0},           //:
//    {0x3b, 59, 0},           //;
//    {0x3c, 60, 0},           //<
//    {0x3d, 61, 0},           //=
//    {0x3e, 62, 0},           //>
//    {0x3f, 63, 0},           //?
//    {0x40, 64, 0},           //@
//    {'A', 65, 1},
//    {'B', 66, 2},
//    {'C', 67, 3},
//    {'D', 68, 4},
//    {'E', 69, 5},
//    {'F', 70, 6},
//    {'G', 71, 7},
//    {'H', 72, 8},
//    {'I', 73, 9},
//    {'J', 74, 100},
//    {'K', 75, 0},
//    {'L', 76, 0},
//    {'M', 77, 0},
//    {'N', 78, 0},
//    {'O', 79, 0},
//    {'P', 80, 0},
//    {'Q', 81, 0},
//    {'R', 82, 0},
//    {'S', 83, 0},
//    {'T', 84, 0},
//    {'U', 85, 0},
//    {'V', 86, 0},
//    {'W', 87, 0},
//    {'X', 88, 0},
//    {'Y', 89, 0},
//    {'Z', 90, 0},
//    {0x22, 92, 0},          // "\"

//  };

static char s_mouse_dscp[] = {
  0x05,0x01,
  0x09,0x02,
  0xa1,0x01,
  0x85,0x28,
  0x09,0x01,
  0xa1,0x00,
  0x05,0x09,
  0x19,0x01,
  0x29,0x08,
  0x15,0x00,
  0x25,0x01,
  0x95,0x08,
  0x75,0x01,
  0x81,0x02,
  0x05,0x01,
  0x09,0x30,
  0x09,0x31,
  0x09,0x38,
  0x0a,0x38,0x02,
  0x15,0x81,
  0x25,0x7f,
  0x75,0x08,
  0x95,0x04,
  0x81,0x06,
  0xc0,
  0xc0
};

static char s_keyboard_dscp[] = {
  0x05,0x01,
  0x09,0x06,
  0xa1,0x01,
  0x85,0x29,
  0x05,0x07,
  0x19,0xe0,
  0x29,0xe7,
  0x15,0x00,
  0x25,0x01,
  0x75,0x01,
  0x95,0x08,
  0x81,0x02,
  0x95,0x01,
  0x75,0x08,
  0x81,0x03,
  0x95,0x05,
  0x75,0x01,
  0x05,0x08,
  0x19,0x01,
  0x29,0x05,
  0x91,0x02,
  0x95,0x01,
  0x75,0x03,
  0x91,0x03,
  0x95,0x06,
  0x75,0x08,
  0x15,0x00,
  0x25,0x65,
  0x05,0x07,
  0x19,0x00,
  0x29,0x65,
  0x81,0x00,
  0xc0
};

static char s_touch_dscp[] = {
  0x05,0x0d,
  0x09,0x04,
  0xa1,0x01,
  0x85,0x54,

  0x09,0x22,
  0xa1,0x02, //physical
  0x09,0x42, //tip switch
  0x15,0x00,
  0x25,0x01,
  0x75,0x01,
  0x95,0x01,
  0x81,0x02, // 1bit input

  0x09,0x32, //InRange
  0x81,0x02,  
  0x95,0x06, // 6bits padding
  0x81,0x03, 

  0x75,0x08,
  0x09,0x51, //Contact Id
  0x95,0x01, // 8bits input
  0x81,0x02,

  0x05,0x01, //Generic desktop
  0x26,0xff,0x7f,
  0x75,0x10,
  0x95,0x01, // 16bits

  0x09,0x30, //X
  0x35,0x00,
  0x46,0x00, 0x00,  
  0x81,0x02,

  0x09,0x31, //Y
  0x46,0x00, 0x00,
  0x81,0x02,
  0xc0,
  0xc0
};


T_KeyMapPc2Android g_tkeyMapHid[] = {
 //basic  
  // {'0', 7, 0},
  // {'1', 8, 0},
  // {'2', 9, 0},
  // {'3', 10, 0},
  // {'4', 11, 0},
  // {'5', 12, 0},
  // {'6', 13, 0},
  // {'7', 14, 0},
  // {'8', 15, 0},
  // {'9', 16, 0},

  {'a', 4, 0},
  {'b', 5, 0},
  {'c', 6, 0},
  {'d', 7, 0},
  {'e', 8, 0},
  {'f', 9, 0},
  {'g', 10, 0},
  {'h', 11, 0},
  {'i', 12, 0},
  {'j', 13, 0},
  {'k', 14, 0},
  {'l', 15, 0},
  {'m', 16, 0},
  {'n', 17, 0},
  {'o', 18, 0},
  {'p', 19, 0},
  {'q', 20, 0},
  {'r', 21, 0},
  {'s', 22, 0},
  {'t', 23, 0},
  {'u', 24, 0},
  {'v', 25, 0},
  {'w', 26, 0},
  {'x', 27, 0},
  {'y', 28, 0},
  {'z', 29, 0},
  
  {'A', 4, 2},
  {'B', 5, 2},
  {'C', 6, 2},
  {'D', 7, 2},
  {'E', 8, 2},
  {'F', 9, 2},
  {'G', 10, 2},
  {'H', 11, 2},
  {'I', 12, 2},
  {'J', 13, 2},
  {'K', 14, 2},
  {'L', 15, 2},
  {'M', 16, 2},
  {'N', 17, 2},
  {'O', 18, 2},
  {'P', 19, 2},
  {'Q', 20, 2},
  {'R', 21, 2},
  {'S', 22, 2},
  {'T', 23, 2},
  {'U', 24, 2},
  {'V', 25, 2},
  {'W', 26, 2},
  {'X', 27, 2},
  {'Y', 28, 2},
  {'Z', 29, 2},
//num
  {'1', 30, 0},
  {'2', 31, 0},
  {'3', 32, 0},
  {'4', 33, 0},
  {'5', 34, 0},
  {'6', 35, 0},
  {'7', 36, 0},
  {'8', 37, 0},
  {'9', 38, 0},
  {'0', 39, 0},

//control
  {0x1000004, 40, 0},//enter
  {0x1000003, 42, 0},//backspace
  {0x1000001,43, 0},//tab
  {0x1000000, 41, 0},//esc
  {0x1000007, 76, 0},//delete
 // {0x1000024, 57, 0},//CapLock
  {0x1000020, 0, 0x02},//lshift  {0xffe1, 225, 0},//this modifier key should be reported as modifier but not as keyval
  {0x1000020, 0, 0x20},//rshift  {0xffe2, 229, 0},
  {0x1000021, 0, 0x01},//lctrl  {0xffe3, 224, 0},
  {0x1000021, 0, 0x10},//rctrl  {0xffe4, 228, 0}
  {0x1000023, 0, 0x04},//lalt  {0xffe9, 226, 0}
  {0x1000023, 0, 0x40},//ralt  {0xffea, 230, 0}
  {0x1000006, 73, 0},//insert
  {0x1000010, 74, 0},//home
  {0x1000011, 77, 0},//end
  // {0xffeb, 123, 0},//win
  {0xff67, 118, 0},//menu
  {0x1000016, 75, 0},//pageup
  {0x1000017, 78, 0},//pagedown
  {0x1000013, 82, 0},//up
  {0x1000015, 81, 0},//down
  {0x1000012, 80, 0},//left
  {0x1000014, 79, 0},//right
  {0xff61, 70, 0},//PrtScreen
  {0xff14, 71, 0},//ScrollLock
  {0xff13, 72, 0},//Pause
//pad
  {0xff7f, 83, 0},//num lock
  {0xffaf, 84, 0},//'/'
  {0xffaa, 85, 0},//'*'
  {0xffad, 86, 0},//'-'
  {0xffab, 87, 0},//'+'
  {0xff8d, 88, 0},//'enter'
  {0xffb1, 89, 0},//'1'
  {0xff9c, 89, 2},//'end select'
  {0xffb2, 90, 0},//'2'
  {0xff99, 90, 2},//'down'
  {0xffb3, 91, 0},//'3'
  {0xff9b, 91, 2},//'page down'
  {0xffb4, 92, 0},//'4'
  {0xff96, 92, 2},//'left'
  {0xffb5, 93, 0},//'5'
  {0xffb6, 94, 0},//'6'
  {0xff98, 94, 2},//'right'
  {0xffb7, 95, 0},//'7'
  {0xff95, 95, 2},//'home select'
  {0xffb8, 96, 0},//'8'
  {0xff97, 96, 2},//'up'
  {0xffb9, 97, 0},//'9'
  {0xff9a, 97, 2},//'page up'
  {0xffb0, 98, 0},//'0'
  {0xff9e, 98, 2},//'Ins'
  {0xffae, 99, 0},//'.'
  {0xff9f, 99, 2},//'Del'
//symbols
  {0x0060, 53, 0},//` grave
  {0x007e, 53, 2},// ~
  {0x0021, 30, 2},//!
  {0x0040, 31, 2},// @
  {0x0023, 32, 2},// #
  {0x0024, 33, 2},// $
  {0x0025, 34, 2},// %
  {0x005e, 35, 2},// ^
  {0x0026, 36, 2},// &
  {0x002a, 37, 2},// *
  {0x0028, 38, 2},// (
  {0x0029, 39, 2},// )
  {0x002d, 45, 0},// -
  {0x005f, 45, 2},// _
  {0x003d, 46, 0},// =
  {0x002b, 46, 2},// +
  {0x005b, 47, 0},// [
  {0x005d, 48, 0},// ]
  {0x007b, 47, 2},// {
  {0x007d, 48, 2},// }
  {0x005c, 49, 0},// '\'
  {0x007c, 49, 2},// '|'
  {0x003b, 51, 0},// ;
  {0x003a, 51, 2},// :
  {0x0027, 52, 0},// '
  {0x0022, 52, 2},// "
  {0x002c, 54, 0},//,
  {0x003c, 54, 2},//<
  {0x002e, 55, 0},//.
  {0x003e, 55, 2},//>
  {0x002f, 56, 0},// '/'
  {0x003f, 56, 2},// '?'
  {0x0020, 44, 0},// space
};

T_KeyMapPc2Android huawei[] = {
  //华为
  {0x1000030,7,0x08}, // F1 home
  {0x1000031,43,0x04}, // F2 fun
  {0x1000032,41,0}, //F3 back
};

T_KeyMapPc2Android xiaomi[] = {
  //小米
  {0x1000030,40,0x08}, // F1 home
  {0x1000031,43,0x04}, // F2 fun
  {0x1000032,42,0x08}, //F3 back
};

static int keycode_trans_pc_2_generic(int32_t in, int32_t *pOut1, int32_t *pOut2)
{
  int i = 0;
  const int entry_num = sizeof(g_tkeyMapGeneric)/sizeof(g_tkeyMapGeneric[0]);

  for(; i<entry_num; i++)
  {
    if(in == g_tkeyMapGeneric[i].pckey)
    {
      *pOut1 = g_tkeyMapGeneric[i].android_key1;
      *pOut2 = g_tkeyMapGeneric[i].android_key2;
      return 0;
    }
  }
  return -1;
}

static int keycode_trans_pc_2_hdic(int32_t in, int32_t *pOut1, int32_t *pOut2)
{
  int i = 0;
  int entry_num = sizeof(g_tkeyMapHid)/sizeof(g_tkeyMapHid[0]);

  for(; i<entry_num; i++)
  {    
    if(in == g_tkeyMapHid[i].pckey)
    {
      *pOut1 = g_tkeyMapHid[i].android_key1;
      *pOut2 = g_tkeyMapHid[i].android_key2;
      return 0;
    }
  }

  if (manufacturer == HUAWEI){
    i = 0;
    entry_num = sizeof(huawei)/sizeof(huawei[0]);
    for(; i<entry_num; i++)
    {    
      if(in == huawei[i].pckey)
      {
        *pOut1 = huawei[i].android_key1;
        *pOut2 = huawei[i].android_key2;
        return 0;
      }
    }
  }
  else if (manufacturer == XIAOMI){
    i = 0;
    entry_num = sizeof(xiaomi)/sizeof(xiaomi[0]);
    for(; i<entry_num; i++)
    {    
      if(in == xiaomi[i].pckey)
      {
        *pOut1 = xiaomi[i].android_key1;
        *pOut2 = xiaomi[i].android_key2;
        return 0;
      }
    }
  }
  return -1;
}


static void send_report_dscp(int sock, HIDType type, char *dscp, int dscp_len)
{
  UibcMessage report_dscp_msg;
  char* outData;
  size_t uibcBodyLen = 0;
  int32_t reportDscpHeaderLen = 9;
  int32_t i;

  uibcBodyLen = reportDscpHeaderLen + dscp_len; 
  //Padding to even number
  uibcBodyLen = (uibcBodyLen % 2 == 0) ? uibcBodyLen : (uibcBodyLen + 1);

  outData = (char*)malloc(uibcBodyLen);
  // UIBC header Octets
  
  int offset_header = 0;
  //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
  outData[offset_header++] = 0x00; // 000 0 0000
  outData[offset_header++] = 0x01; // 0000 0001 HIDC

  //Length(16 bits)
  outData[offset_header++] = (uibcBodyLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = uibcBodyLen & 0xFF; // last 8 bytes

  //HIDC Input Body Format
  outData[offset_header++] = 1; // InputPath,  USB  
  outData[offset_header++] = type; // Type, mouse
  outData[offset_header++] = 0x01; // Usage, report descriptor

  // Length, 2 octets
  outData[offset_header++] = (dscp_len >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = dscp_len & 0xFF; //last 8 bytes

  for(i=0; i<dscp_len; i++)
    outData[offset_header++] = dscp[i];

  report_dscp_msg.m_DataValid = true;
  report_dscp_msg.m_PacketData = outData;  
  report_dscp_msg.m_PacketDataLen = uibcBodyLen;

  sendUibcMessage(&report_dscp_msg, sock);
}

static void fill_keyboard_data(char** splitedStr, char *outData)
{
  char i = 0;
  bool bPressed = (HID_KEY_PRESSED == (*splitedStr[i++] - 0x30)) ? true : false;
  int pckeyval = 0;
  int keycode = 0, modifyKey = 0;
  int ret = 0;

  sscanf(*(splitedStr + i), " 0x%08X", &pckeyval);

  ret = keycode_trans_pc_2_hdic(pckeyval, &keycode, &modifyKey);

  i = 0;
  
  if(!ret && bPressed)
  {    
    outData[i++] = (char)modifyKey;
    outData[i++] = 0;
    outData[i++] = (char)keycode;
  }
  else
  {
    outData[i++] = 0;
    outData[i++] = 0;
    outData[i++] = 0;
  }


  for(; i<8; i++)
    outData[i] = 0;
}

static void fill_mouse_data(char** splitedStr, char *outData)
{
  char i = 0, j = 0;  
  int bPressed = atoi(splitedStr[i++]);
  char button = *splitedStr[i++] - 0x30;
  char x = atoi(splitedStr[i++]);
  char y = atoi(splitedStr[i++]);
  char scroll;

  if(bPressed == 0)
  {    
    outData[j++] = button;
    outData[j++] = x;
    outData[j++] = y;
    
    for(; j<4; j++)
      outData[j] = 0;
  }
  else if(bPressed == 1)
  {
    outData[j++] = 0;
    outData[j++] = x;
    outData[j++] = y;
    
    for(; j<4; j++)
      outData[j] = 0;
  }
  else if(bPressed == 2)
  { 
    outData[j++] = 0;
    outData[j++] = 0;
    outData[j++] = 0;
    scroll = atoi(splitedStr[i++]);
    outData[j++] = scroll;
    for(; j<3; j++)
      outData[j] = 0; 
  }
}



static void getHIDCKeyboardPacket(const char* rawMsg, UibcMessage* uibcmessage)
{
  char* outData;
  int32_t typeId = 0;
  int32_t numberOfPointers = 0;
  size_t uibcBodyLen = 0;
  int32_t genericPacketLen = 9;
  int32_t temp;
  size_t size;

  char** splitedStr = str_split((char*)rawMsg, ",", &size);

  int offset_split = 0;
  typeId = atoi(*(splitedStr + offset_split++));

  uibcBodyLen = genericPacketLen + 9; // Generic header length = 10
  //Padding to even number
  uibcBodyLen = (uibcBodyLen % 2 == 0) ? uibcBodyLen : (uibcBodyLen + 1);

  outData = (char*)malloc(uibcBodyLen);
  // UIBC header Octets
  
  int offset_header = 0;
  //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
  outData[offset_header++] = 0x00; // 000 0 0000
  outData[offset_header++] = 0x01; // 0000 0001 HIDC

  //Length(16 bits)
  outData[offset_header++] = (uibcBodyLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = uibcBodyLen & 0xFF; // last 8 bytes

  //HIDC Input Body Format
  outData[offset_header++] = 1; // InputPath,  USB  
  outData[offset_header++] = HID_KEYBOARD; // Type, mouse
  outData[offset_header++] = 0x00; // Usage, input report in data field

  // Length, 2 octets
  outData[offset_header++] = (genericPacketLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = genericPacketLen & 0xFF; //last 8 bytes

  outData[offset_header++] = 0x29; // don't known the meaning yet, may be report id for mouse type

  fill_keyboard_data((splitedStr + offset_split), &outData[offset_header]);

  // while (offset <= uibcBodyLen) {
  //   outData[offset++] = 0x00;
  // }

  for (int i = 0; i < size; i++) {
    free(*(splitedStr + i));
  }
  free(splitedStr);

  // binarydump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketData = outData;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}


static void getHIDCMousePacket(const char* rawMsg, UibcMessage* uibcmessage)
{
  char* outData;
  int32_t typeId = 0;
  int32_t numberOfPointers = 0;
  size_t uibcBodyLen = 0;
  int32_t genericPacketLen = 6;
  int32_t temp;
  size_t size;

  char** splitedStr = str_split((char*)rawMsg, ",", &size);

  int offset_split = 0;
  typeId = atoi(*(splitedStr + offset_split++));

  uibcBodyLen = genericPacketLen + 10; // Generic header length = 10
  //Padding to even number
  uibcBodyLen = (uibcBodyLen % 2 == 0) ? uibcBodyLen : (uibcBodyLen + 1);

  outData = (char*)malloc(uibcBodyLen);
  // UIBC header Octets
  
  int offset_header = 0;
  //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
  outData[offset_header++] = 0x00; // 000 0 0000
  outData[offset_header++] = 0x01; // 0000 0001 HIDC

  //Length(16 bits)
  outData[offset_header++] = (uibcBodyLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = uibcBodyLen & 0xFF; // last 8 bytes

  //HIDC Input Body Format
  outData[offset_header++] = 1; // InputPath,  USB  
  outData[offset_header++] = HID_MOUSE; // Type, mouse
  outData[offset_header++] = 0x00; // Usage, input report in data field

  // Length, 2 octets
  outData[offset_header++] = (genericPacketLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = genericPacketLen & 0xFF; //last 8 bytes

  outData[offset_header++] = 0x28; // don't known the meaning yet, may be report id for mouse type

  fill_mouse_data((splitedStr + offset_split), &outData[offset_header]);

  // while (offset <= uibcBodyLen) {
  //   outData[offset++] = 0x00;
  // }

  for (int i = 0; i < size; i++) {
    free(*(splitedStr + i));
  }
  free(splitedStr);

  // binarydump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketData = outData;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}

const char *int2binary(int x, int padding)
{
  char *b;
  int min_padding = x > 0 ? floor(log2(x)) : 0;
  if (padding < min_padding) {
    padding = min_padding;
  }

  b = (char *)malloc (sizeof (char *) * padding + 1);
  strcpy(b, "");

  int z;
  for (z = pow(2,padding); z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

int sendUibcMessage(UibcMessage* uibcmessage, int sockfd) {
  size_t n = 0, r = 0;

  do{
    r = write(sockfd, uibcmessage->m_PacketData , uibcmessage->m_PacketDataLen);  
    if (r < 0) {
      if(errno == EAGAIN || errno == EWOULDBLOCK)
        log_info("write SOCKET blocked, try again");
      else
      {
        log_error("write SOCKET ERROR(%d)", errno);
        return EXIT_FAILURE;
      }
    }
    else if (r == 0)
    {
      log_info("write SOCKET RET 0");
      return EXIT_SUCCESS;
    }
    else
    {
      n += r;
      if(n < uibcmessage->m_PacketDataLen)
        log_info("write SOCKET blocked as buffer full, send pending");
    }
  }while(n < uibcmessage->m_PacketDataLen);

  return EXIT_SUCCESS;
}

UibcMessage buildUibcMessage(MessageType type,
    const char* inEventDesc,
    double widthRatio,
    double heightRatio) {
  UibcMessage uibcmessage;
  uibcmessage.m_PacketData = NULL;
  uibcmessage.m_PacketDataLen = 0;
  uibcmessage.m_DataValid = false;

  switch (type) {
    case GENERIC_TOUCH_DOWN:
    case GENERIC_TOUCH_UP:
    case GENERIC_TOUCH_MOVE:
      getUIBCGenericTouchPacket(inEventDesc,
          &uibcmessage,
          widthRatio,
          heightRatio);
      break;

    case GENERIC_KEY_DOWN:
    case GENERIC_KEY_UP:
      getUIBCGenericKeyPacket(inEventDesc, &uibcmessage);
      break;

    case GENERIC_ZOOM:
      getUIBCGenericZoomPacket(inEventDesc, &uibcmessage);
      break;

    case GENERIC_VERTICAL_SCROLL:
    case GENERIC_HORIZONTAL_SCROLL:
      getUIBCGenericScalePacket(inEventDesc, &uibcmessage);
      break;

    case GENERIC_ROTATE:
      getUIBCGenericRotatePacket(inEventDesc, &uibcmessage);
      break;
  };
  return uibcmessage;
}

UibcMessage buildHIDC_UibcMessage(HIDType type,
    const char* rawMsg,
    double widthRatio,
    double heightRatio) {
  UibcMessage uibcmessage;
  uibcmessage.m_PacketData = NULL;
  uibcmessage.m_PacketDataLen = 0;
  uibcmessage.m_DataValid = false;

  switch (type) {
    case HID_KEYBOARD:
      getHIDCKeyboardPacket(rawMsg, &uibcmessage);
      break;
    case HID_MOUSE:
      getHIDCMousePacket(rawMsg, &uibcmessage);
      break;
    default:
      log_error("received other HIDC devices");
      break;
  };

  return uibcmessage;
}


// format: "typeId, number of pointers, pointer Id1, X coordnate, Y coordinate, pointer Id2, X coordnate, Y coordnate,..."
void getUIBCGenericTouchPacket(const char *inEventDesc,
    UibcMessage* uibcmessage,
    double widthRatio,
    double heightRatio) 
{
  char* outData;
  int32_t typeId = 0;
  int32_t numberOfPointers = 0;
  size_t uibcBodyLen = 0;
  int32_t genericPacketLen = 0;
  int32_t temp;
  size_t size;

  char** splitedStr = str_split((char*)inEventDesc, ",", &size);

  if ((int)size - 5 < 0 || ((int)size - 5) % 3 != 0) {
    log_error("getUIBCGenericTouchPacket (%s)", "bad input event");
    return;
  }
  int offset_split = 0;
  typeId = atoi(*(splitedStr + offset_split++));
  numberOfPointers = atoi(*(splitedStr + offset_split++));

  genericPacketLen = numberOfPointers * 5 + 1;
  uibcBodyLen = genericPacketLen + 7; // Generic header length = 7
  //Padding to even number
  uibcBodyLen = (uibcBodyLen % 2 == 0) ? uibcBodyLen : (uibcBodyLen + 1);

  outData = (char*)malloc(uibcBodyLen);
  // UIBC header Octets
  int offset_header = 0;
  //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
  outData[offset_header++] = 0x00; // 000 0 0000
  outData[offset_header++] = 0x00; // 0000 0000

  //Length(16 bits)
  outData[offset_header++] = (uibcBodyLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = uibcBodyLen & 0xFF; // last 8 bytes

  //Generic Input Body Format
  outData[offset_header++] = typeId & 0xFF; // Tyoe ID, 1 octet

  // Length, 2 octets
  outData[offset_header++] = (genericPacketLen >> 8) & 0xFF; // first 8 bytes
  outData[offset_header++] = genericPacketLen & 0xFF; //last 8 bytes

  // Number of pointers, 1 octet
  outData[offset_header++] = numberOfPointers & 0xFF; 

  int offset = 0;
  for (int i = 0; i < numberOfPointers; i++) {
    int splitoffset = offset_split + (i * 3);
    temp = atoi(*(splitedStr + splitoffset++));
    offset = offset_header + ( i * 5);
    outData[offset++] = temp & 0xFF;

    temp = atoi(*(splitedStr + splitoffset++));
    temp = (int32_t)((double)temp / widthRatio);
    outData[offset++] = (temp >> 8) & 0xFF;
    outData[offset++] = temp & 0xFF;

    temp = atoi(*(splitedStr + splitoffset++));
    temp = (int32_t)((double)temp / heightRatio);
    outData[offset++] = (temp >> 8) & 0xFF;
    outData[offset++] = temp & 0xFF;
  }

  while (offset <= uibcBodyLen) {
    outData[offset++] = 0x00;
  }

  for (int i = 0; i < size; i++) {
    free(*(splitedStr + i));
  }
  free(splitedStr);

  binarydump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketData = outData;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}

void hexdump(void *_data, size_t len)
{
  unsigned char *data = _data;
  size_t count;

  int line = 15;
  for (count = 0; count < len; count++) {
    if ((count & line) == 0) {
      fprintf(stderr,"%04zu: ", count);
    }
    fprintf(stderr,"%02x ", *data);
    data++;
    if ((count & line) == line) {
      fprintf(stderr,"\n");
    }
  }
  if ((count & line) != 0) {
    fprintf(stderr,"\n");
  }
}

void binarydump(void *_data, size_t len)
{
  unsigned char *data = _data;
  size_t count;

  int line = 7;
  for (count = 0; count < len; count++) {
    if ((count & line) == 0) {
      fprintf(stderr,"%04zu: ", count);
    }
    fprintf(stderr,"%s ", int2binary(*data, 8));
    data++;
    if ((count & line) == line) {
      fprintf(stderr,"\n");
    }
  }
  if ((count & line) != 0) {
    fprintf(stderr,"\n");
  }
}
// format: "typeId, Key code 1(0x00), Key code 2(0x00)"
void getUIBCGenericKeyPacket(const char *inEventDesc,
    UibcMessage* uibcmessage) {
  char* outData = uibcmessage->m_PacketData;
  int32_t typeId = 0;
  int32_t uibcBodyLen = 0;
  int32_t genericPacketLen = 0;
  int32_t temp = 0;
  int32_t key1 = 0, key2 = 0;
  size_t size;
  char** splitedStr = str_split((char*)inEventDesc, ",", &size);

  if (size > 0) {

    if (((int)size) % 3 != 0) {
        log_error("getUIBCGenericKeyPacket (%s)", "bad input event");
        return;
    }
    int i;
    for (i = 0; i < size; i++) {

      switch (i) {
        case 0: {
                  typeId = atoi(*(splitedStr + i));
                  genericPacketLen = 5;
                  uibcBodyLen = genericPacketLen + 7; // Generic header legth = 7
                  outData = (char*)malloc(uibcBodyLen + 1);
                  // UIBC header
                  outData[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                  outData[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                  //Generic Input Body Format
                  outData[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                  outData[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                  outData[6] = genericPacketLen & 0xFF; // Length, 2 octets
                  outData[7] = 0x00; // reserved
                  break;
                }
        case 1: {
                  sscanf(*(splitedStr + i), " 0x%X", &temp);
                  if (temp == 0) {
                    outData[8] = 0x00;
                    outData[9] = 0x00;
                  }
                    outData[8] = (temp >> 8) & 0xff;//(key1 >> 8) & 0xff;//(temp >> 8) & 0xFF;
                    outData[9] = temp & 0xff;//temp & 0xFF;
                  break;
                }
        case 2: {
                  sscanf(*(splitedStr + i), " 0x%X", &temp);
                  if (temp == 0) {
                    outData[10] = 0x00;
                    outData[11] = 0x00;
                  }
                  outData[10] = (temp >> 8) & 0xFF;
                  outData[11] = temp & 0xFF;
                  break;
                }
        default: 
          break;
      }
      free(*(splitedStr + i));
    }
  }
  free(splitedStr);

  binarydump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketData = outData;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}

// format: "typeId,  X coordnate, Y coordnate, integer part, fraction part"
void getUIBCGenericZoomPacket(const char *inEventDesc, UibcMessage* uibcmessage) {
  log_info("getUIBCGenericZoomPacket (%s)", inEventDesc);
  char* outData = uibcmessage->m_PacketData;
  int32_t typeId;
  int32_t uibcBodyLen, genericPacketLen;
  int32_t xCoord, yCoord, integerPart, FractionPart;
  size_t size;

  char** splitedStr = str_split((char*)inEventDesc, ",", &size);

  if (splitedStr) {
    int i;
    for (i = 0; * (splitedStr + i); i++) {

      switch (i) {
        case 0: {
                  typeId = atoi(*(splitedStr + i));

                  genericPacketLen = 6;
                  uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                  outData = (char*)malloc(uibcBodyLen + 1);
                  // UIBC header
                  outData[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                  outData[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                  //Generic Input Body Format
                  outData[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                  outData[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                  outData[6] = genericPacketLen & 0xFF; // Length, 2 octets
                  break;
                }

        case 1: {
                  xCoord = atoi(*(splitedStr + i));
                  outData[7] = (xCoord >> 8) & 0xFF;
                  outData[8] = xCoord & 0xFF;
                  break;
                }
        case 2: {
                  yCoord = atoi(*(splitedStr + i));
                  break;
                }
        case 3: {
                  integerPart = atoi(*(splitedStr + i));
                  outData[11] = integerPart & 0xFF;
                  break;
                }
        case 4: {
                  FractionPart = atoi(*(splitedStr + i));
                  outData[12] = FractionPart & 0xFF;

                  break;
                }
        default: {
                   break;
                 }
      }

      free(*(splitedStr + i));
    }
    free(splitedStr);
  }
  //hexdump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}

// format: "typeId,  unit, direction, amount to scroll"
void getUIBCGenericScalePacket(const char *inEventDesc, UibcMessage* uibcmessage) {
  log_info("getUIBCGenericScalePacket (%s)", inEventDesc);
  char* outData = uibcmessage->m_PacketData;
  int32_t typeId;
  int32_t uibcBodyLen, genericPacketLen;
  int32_t temp;
  size_t size;
  char** splitedStr = str_split((char*)inEventDesc, ",", &size);

  if (splitedStr) {
    int i;
    for (i = 0; * (splitedStr + i); i++) {

      switch (i) {
        case 0: {
                  typeId = atoi(*(splitedStr + i));
                  genericPacketLen = 2;
                  uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                  outData = (char*)malloc(uibcBodyLen + 1);
                  // UIBC header
                  outData[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                  outData[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                  //Generic Input Body Format
                  outData[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                  outData[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                  outData[6] = genericPacketLen & 0xFF; // Length, 2 octets
                  outData[7] = 0x00; // Clear the byte
                  outData[8] = 0x00; // Clear the byte
                  /*
                     B15B14; Scroll Unit Indication bits.
                     0b00; the unit is a pixel (normalized with respect to the WFD Source display resolution that is conveyed in an RTSP M4 request message).
                     0b01; the unit is a mouse notch (where the application is responsible for representing the number of pixels per notch).
                     0b10-0b11; Reserved.

                     B13; Scroll Direction Indication bit.
                     0b0; Scrolling to the right. Scrolling to the right means the displayed content being shifted to the left from a user perspective.
                     0b1; Scrolling to the left. Scrolling to the left means the displayed content being shifted to the right from a user perspective.

B12:B0; Number of Scroll bits.
Number of units for a Horizontal scroll.
*/
                  break;
                }
        case 1: {
                  temp = atoi(*(splitedStr + i));
                  outData[7] = (temp >> 8) & 0xFF;
                  break;
                }
        case 2: {
                  temp = atoi(*(splitedStr + i));
                  outData[7] |= ((temp >> 10) & 0xFF);
                  break;

                }
        case 3: {
                  temp = atoi(*(splitedStr + i));
                  outData[7] |= ((temp >> 12) & 0xFF);
                  outData[8] = temp & 0xFF;

                  break;
                }
        default: {
                   break;
                 }
      }

      free(*(splitedStr + i));
    }

    free(splitedStr);
  }
  //hexdump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}

// format: "typeId,  integer part, fraction part"
void getUIBCGenericRotatePacket(const char * inEventDesc, UibcMessage* uibcmessage) {
  log_info("getUIBCGenericRotatePacket (%s)", inEventDesc);
  char* outData = uibcmessage->m_PacketData;
  int32_t typeId;
  int32_t uibcBodyLen, genericPacketLen;
  int32_t integerPart, FractionPart;
  size_t size;

  char** splitedStr = str_split((char*)inEventDesc, ",", &size);

  if (splitedStr) {
    int i;
    for (i = 0; * (splitedStr + i); i++) {
      switch (i) {
        case 0: {
                  typeId = atoi(*(splitedStr + i));
                  genericPacketLen = 2;
                  uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                  outData = (char*)malloc(uibcBodyLen + 1);
                  // UIBC header
                  outData[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                  outData[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                  outData[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                  //Generic Input Body Format
                  outData[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                  outData[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                  outData[6] = genericPacketLen & 0xFF; // Length, 2 octets
                  break;
                }
        case 1: {
                  integerPart = atoi(*(splitedStr + i));
                  outData[7] = integerPart & 0xFF;
                  break;
                }
        case 2: {
                  FractionPart = atoi(*(splitedStr + i));
                  outData[8] = FractionPart & 0xFF;

                  break;
                }
        default: {
                   break;
                 }
      }
      free(*(splitedStr + i));
    }

    free(splitedStr);
  }
  //hexdump(outData, uibcBodyLen);
  uibcmessage->m_DataValid = true;
  uibcmessage->m_PacketDataLen = uibcBodyLen;
}

UibcMessage buildKylinPC_UibcMsg(const char*buffer)
{
  UibcMessage uibcmessage;
  uibcmessage.m_PacketData = buffer;
  uibcmessage.m_PacketDataLen = strlen(buffer)+1;
  uibcmessage.m_DataValid = true;

  return uibcmessage;
}
char** str_split(char* pStr, const char* pDelim, size_t* size) {
  char** result    = 0;
  size_t count     = 0;
  char* tmp        = pStr;
  char* tmpStr     = NULL;
  char* last_comma = 0;

  asprintf(&tmpStr, "%s", pStr);

  /* Count how many elements will be extracted. */
  while (*tmp) {
    if (*pDelim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  /* Add space for trailing token. */
  count += last_comma < (pStr + strlen(pStr) - 1) ? 1 : 0;

  result = (char**)malloc(sizeof(char*) * count);

  *size = count;

  tmp = tmpStr = strdup(pStr);
  size_t idx  = 0;
  char* token;
  while ((token = strsep(&tmp, pDelim)) != NULL) {
    * (result + idx++) = strdup(token);
  }
  free(tmpStr);
  return result;
}

typedef struct
{
  int pipefd;
  int sockfd;
}T_UibcHandle;
#define IS_KYLIN_PC_BY_PORT(port) (port == KYLIN_PC_UIBC_SRV_PORT)

static bool is_peer_vendor_identify(char type)
{
    if(type == HUAWEI)
    {
      manufacturer = HUAWEI;
      return true;
    }
    else if (type == XIAOMI)
    {
      manufacturer = XIAOMI;
      return true;
    }
    else if (type == MANUFACTURER || type == OTHER)
    {
      return true;
    }

    return false;
}

static ssize_t get_a_line_len(const char *pBuf)
{
  const char* key = "\n";
  const char *pStart = pBuf;
  const char *pEnd = strstr(pBuf, key);

  if(!pEnd)
    return 0;

  return (ssize_t)((pStart < pEnd) ? (pEnd - pStart) : (pStart - pEnd)) + strlen(key);
}

static int send_one_uibc_messages(T_UibcHandle *ptHandle, const char type, const char* pBuf, int len)
{    
    UibcMessage tMsg;
    int r = EXIT_FAILURE;
    char buffer[1024] = {0};

    memcpy(buffer, pBuf, len);
    memset(&tMsg, 0, sizeof(tMsg));

    if(false == s_bUseHIDC)
    {
      if (type == '0' || type == '1') {
        tMsg = buildUibcMessage(GENERIC_TOUCH_DOWN, buffer, 1, 1);
      } else if (type == '3' || type == '4') {
        tMsg = buildUibcMessage(GENERIC_KEY_DOWN, buffer, 1, 1);
      } else {
        log_info("Player gives invalid uibc generic type %c", type);
      }
    }
    else
    {
      if(IS_KYLIN_PC_BY_PORT(portno)) {
        tMsg = buildKylinPC_UibcMsg(buffer);        
      } else {
        if(type == EVENT_HID_KEY_TYPE) {//keyboard
          tMsg = buildHIDC_UibcMessage(HID_KEYBOARD, buffer, 1, 1);
        } else if(type == EVENT_HID_MOUSE_TYPE) {//mouse
          tMsg = buildHIDC_UibcMessage(HID_MOUSE, buffer, 1, 1);
        } else {//other device
          log_info("Player gives invalid uibc hidc type %c", type);
        }
      }
    }

    if(tMsg.m_PacketDataLen)
      r = sendUibcMessage(&tMsg, ptHandle->sockfd);

    return r;
}

static void dispatch_uibc_event(T_UibcHandle *ptHandle, const char* buffer, ssize_t contentLen)
{  
  const char *pUibcCmd = buffer;
  ssize_t srcBufOffset = 0;
  int r = 0;

  while(srcBufOffset < contentLen)
  {
    char cmdtype = pUibcCmd[0];
    int r = 0;

    if(is_peer_vendor_identify(cmdtype))
      return;

    r = get_a_line_len(pUibcCmd);

    if(!r)
    {
      log_error("Invalid uibc event input: %s, offset %lu, total %lu", pUibcCmd, srcBufOffset, contentLen);
      break;
    }

    send_one_uibc_messages(ptHandle, cmdtype, pUibcCmd, r);

    srcBufOffset += r;    
    pUibcCmd = buffer + srcBufOffset;//next cmd, if exists
  }
}

static gboolean handle_uibc_event(GIOChannel *channel, GIOCondition condition, gpointer data)
{
    UibcMessage uibcmessage;
    T_UibcHandle tHandle = *((T_UibcHandle *)data);
    char type = 0;
    char buffer[1024] = {0};
    ssize_t bufflen = sizeof(buffer) - 1;
    ssize_t offset = 0;
    int r = 0;

    do{
      r = read(tHandle.pipefd, &buffer[r], ((bufflen-r) < SSIZE_MAX ? (bufflen-r) : SSIZE_MAX));
      if(r < 0)
      {
        if(errno != EAGAIN && errno != EWOULDBLOCK)//pipe read throw an error
          perror("Read uibc pipe failed as: ");
      } 
    }while(r > 0);

    dispatch_uibc_event(&tHandle, buffer, strlen(buffer));

    return true;
}



int main(int argc, char *argv[]) {
    //TODO: Add miracle TUI interface
    //TODO: Add parsearg
    //--daemon (read stdin)
  // int portno;
  int pipefd;
  struct hostent *server;
  int sockfd;
  struct sockaddr_in serv_addr;
  T_UibcHandle tHandle;  
  int r;
  int opt_sw;
  GMainLoop *mainloop;
  GIOChannel* channel;


  log_max_sev = LOG_INFO;

  if (argc < 3) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "   %s <hostname> <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  server = gethostbyname(argv[1]);
  portno = atoi(argv[2]);
  pipefd = atoi(argv[3]);
  s_bUseHIDC = atoi(argv[4]);

  log_info("uibc-daemon server %s port %d", argv[1], portno);

  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    return EXIT_FAILURE;
  }

  opt_sw = 1;
  r = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt_sw, sizeof(opt_sw));
  if (r < 0) {
    perror("ERROR set socket");
    return EXIT_FAILURE;
  }
  r = setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &opt_sw, sizeof(opt_sw));
  if (r < 0) {
    perror("ERROR set socket");
    return EXIT_FAILURE;
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    return EXIT_FAILURE;
  }
  
  if(s_bUseHIDC)
  {
    send_report_dscp(sockfd, HID_MOUSE, s_mouse_dscp, sizeof(s_mouse_dscp));
    send_report_dscp(sockfd, HID_KEYBOARD, s_keyboard_dscp, sizeof(s_keyboard_dscp));
  }
  tHandle.pipefd = pipefd;
  tHandle.sockfd = sockfd;

  int pipe_flags = fcntl(pipefd, F_GETFL, 0);
  if(pipe_flags < 0)
  {
    perror("GET pipe flags failed as: ");
  }
  else
  {
    if(pipe_flags & O_NONBLOCK){
      log_info("uibc socket is non block");
    }
    else{
      pipe_flags |= O_NONBLOCK;
      if(fcntl(pipefd, F_SETFL, pipe_flags) < 0)
        perror("SET pipe noblock failed as:");
    }
  }

  channel = g_io_channel_unix_new(pipefd);
  g_io_add_watch(channel, G_IO_IN, handle_uibc_event, &tHandle);

	mainloop = g_main_loop_new(NULL, false);

	g_main_loop_run(mainloop);

  g_main_loop_unref(mainloop);

  close(sockfd);
  return EXIT_SUCCESS;
}

