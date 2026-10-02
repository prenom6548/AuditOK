// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <linux/input-event-codes.h>
#include <locale.h>
#include <string.h>

#include <kywc/log.h>

#include "effect/effect.h"
#include "input/cursor.h"
#include "input/function.h"
#include "input/input.h"
#include "input_p.h"
#include "util/dbus.h"
#include "util/macros.h"
#include "view/view.h"
#include "view/workspace.h"

enum language {
    LANG_EN_US,
    LANG_ZH_CN,
    LANG_ZH_HK,
    LANG_BO_CN,
    LANG_MN_MN,
    LANG_COUNT,
};

static const char *service_path = "/com/kylin/Wlcom/FunctionManager";
static const char *service_interface = "com.kylin.Wlcom.FunctionManager";

static const char *function_translation[FUNC_ID_COUNT][LANG_COUNT] = {
    {
        "None",
        "无",
        "無",
        "མེད།",
        "үгүй",
    },
    {
        "Custom",
        "用户自定义",
        "用戶自定義",
        "སྤྱོད་མཁན་གྱི་མིང་འདོགས།",
        "Хэрэглэгчийн дараах тохируулга",
    },
    {
        "Configure",
        "用户配置",
        "用戶配置",
        "སྤྱོད་མཁན་བཀོད་སྒྲིག",
        "Хэрэглэгчийн тохируулга",
    },
    {
        "New Workspace",
        "新建工作区",
        "新增工作區",
        "ལས་སྒྲུབ་ཁུལ་གསར་པ་བཟོ།",
        "Шинэ ажлын зай үүсгэх",
    },
    {
        "Delete Workspace",
        "删除工作区",
        "刪除工作區",
        "ལས་སྒྲུབ་ཁུལ་སུབ།",
        "Ажлын зай устгах",
    },
    {
        "Switch to Left Workspace",
        "切换左边工作区",
        "切換左邊工作區",
        "གཡོན་ཕྱོགས་ཀྱི་བྱ་བ་བསྒྲུབ་ས་བརྗེ་རོགས།",
        "Зүүн талын ажлын зайруу шилжих",
    },
    {
        "Switch to Right Workspace",
        "切换右边工作区",
        "切換右邊工作區",
        "གཡས་ཕྱོགས་ཀྱི་ལས་སྒྲུབ་ཁུལ་བརྗེ།",
        "Баруун талын ажлын зайруу шилжих",
    },
    {
        "Switch to Workspace 1",
        "切换工作区1",
        "切換工作區1",
        "ལས་ཀའི་ཁུལ་གཅིག་པར་བརྗེ་བ།",
        "Ажлын зай 1 рүү шилжих",
    },
    {
        "Switch to Workspace 2",
        "切换工作区2",
        "切換工作區2",
        "ལས་ཀ་བྱེད་ཁུལ་གཉིས་པར་བརྗེ།",
        "Ажлын зай 2 рүү шилжих",
    },
    {
        "Switch to Workspace 3",
        "切换工作区3",
        "切換工作區3",
        "ལས་ཀའི་ཁུལ་གསུམ་བརྗེ་སྤོར་བྱས།",
        "Ажлын зай 3 рүү шилжих",
    },
    {
        "Switch to Workspace 4",
        "切换工作区4",
        "切換工作區4",
        "བརྗེ་སྤོར་ལས་སྒྲུབ་ཁུལ་༤",
        "Ажлын зай 4 рүү шилжих",
    },
    {
        "Toggle Desktop Visibility",
        "显示或隐藏桌面",
        "顯示或隱藏桌面",
        "མངོན་པའམ་ཡང་ན་གསང་བའི་སྒོ་ནས་ཅོག་ངོས་མངོན་པ།",
        "Дэлгэцийг харагдах эсвэл буцаах",
    },
    {
        "Show Desktop(Minimize All)",
        "显示桌面",
        "顯示桌面",
        "ཅོག་ངོས་འཆར་བ།",
        "Дэлгэцийг харагдах",
    },
    {
        "Restore All Desktop Windows",
        "还原所有桌面窗口",
        "還原所有桌面窗口",
        "དཀར་ངོས་སྒེའུ་ཁུང་ཡོད་ཚད་ཕྱིར་ལྡོག་བྱོས།",
        "Бүгд төрөл бүрийн цонхийг буцаах",
    },
    {
        "Minimize Other Windows",
        "最小化(隐藏)其他(非激活)窗口",
        "最小化(隱藏)其他窗口",
        "ཆེས་ཆུང་བའི་སྒེའུ་ཁུང་གཞན།",
        "Бусад цонхнуудыг минимайз хийх",
    },
    {
        "Restore Other Windows",
        "还原其他(非激活)窗口",
        "還原其他窗口",
        "སྒེའུ་ཁུང་གཞན་སླར་གསོ་བྱེད་པ།",
        "Бусад цонхнуудыг сэргээх",
    },
    {
        "Minimize or Restore Other Windows",
        "最小化或还原其他窗口",
        "最小化或還原其他窗口",
        "ཆེས་ཆུང་བའམ་སླར་གསོ་བྱེད་པའི་སྒེའུ་ཁུང་གཞན།",
        "Бүх дэлгэцийн цонхнуудыг дахин байрлуулах",
    },
    {
        "Maximize Active Window",
        "最大化活动窗口",
        "最大化活動窗口",
        "ཆེས་ཆེ་བའི་བྱ་འགུལ་གྱི་སྒེའུ་ཁུང་།",
        "Хөдөлгөөн талбарыг томруулах",
    },
    {
        "Minimize Active Window",
        "最小化活动窗口",
        "最小化活動窗口",
        "ཆེས་ཆུང་བའི་འགུལ་སྐྱོད་སྒེའུ་ཁུང་།",
        "Хөдөлгөөн талбарыг багасгах",
    },
    {
        "Close Active Window",
        "关闭活动窗口",
        "關閉活動窗口",
        "བྱེད་སྒོའི་སྒེའུ་ཁུང་རྒྱབ་པ།",
        "Хөдөлгөөн талбарыг хаах",
    },
    {
        "Show Active Window Menu",
        "显示活动窗口菜单",
        "顯示活動窗口菜單",
        "བྱ་འགུལ་དྲ་མིག་གི་ཚོད་ཐོ་མངོན་པ།",
        "Хөдөлгөөн талбарын менюг харуулах",
    },
    {
        "Capture Active Window",
        "截图活动窗口",
        "截圖活動窗口",
        "བྱ་འགུལ་སྒེའུ་ཁུང་གི་བརྙན་པར་ལེན།",
        "Идэвхтэй цонхны дэлгэцийн зураг авах",
    },
    {
        "Tile Active Window to the Top",
        "活动窗口向上贴靠",
        "活動窗口向上貼靠",
        "བྱེད་སྒོའི་སྐར་ཁུང་ཡར་སྦྱར་བར་བརྟེན།",
        "Хөдөлгөөн талбарыг дээд тал руу хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Bottom",
        "活动窗口向下贴靠",
        "活動窗口向下貼靠",
        "བྱེད་སྒོའི་སྐར་ཁུང་དེ་གཤམ་དུ་འབྱར་བ།",
        "Хөдөлгөөн талбарыг доод тал руу хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Left",
        "活动窗口向左贴靠",
        "活動窗口向左貼靠",
        "བྱ་འགུལ་གྱི་སྒེའུ་ཁུང་གཡོན་ཕྱོགས་སུ་འབྱར་བ།",
        "Хөдөлгөөн талбарыг зүүн тал руу хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Right",
        "活动窗口向右贴靠",
        "活動窗口向右貼靠",
        "བྱེད་སྒོའི་སྐར་ཁུང་གཡས་ཕྱོགས་སུ་སྦྱར་ནས།",
        "Хөдөлгөөн талбарыг баруун тал руу хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Top Half",
        "活动窗口向上半屏贴靠",
        "活動窗口向上半屏貼靠",
        "བྱ་འགུལ་སྒེའུ་ཁུང་གོང་གི་བརྙན་ཤེལ་གཡས་གཡོན་དུ་སྦྱར།",
        "Хөдөлгөөн талбарыг дээд хагас дэлгэц рүү хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Bottom Half",
        "活动窗口向下半屏贴靠",
        "活動窗口向下半屏貼靠",
        "བྱེད་སྒོའི་དཀར་ཁུང་འོག་མའི་ཕྱོགས་སུ་གཏད་དགོས།",
        "Хөдөлгөөн талбарыг доод хагас дэлгэц рүү хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Left Half",
        "活动窗口向左半屏贴靠",
        "活動窗口向左半屏貼靠",
        "བྱེད་སྒོའི་སྐར་ཁུང་གཡོན་ཕྱོགས་སུ་གཏད།",
        "Хөдөлгөөн талбарыг зүүн хагас дэлгэц рүү хүртэл нь тохируулах",
    },
    {
        "Tile Active Window to the Right Half",
        "活动窗口向右半屏贴靠",
        "活動窗口向右半屏貼靠",
        "བྱ་འགུལ་གྱི་སྒེའུ་ཁུང་གཡས་ཕྱོགས་སུ་གཏད་ཡོད།",
        "Хөдөлгөөн талбарыг баруун хагас дэлгэц рүү хүртэл нь тохируулах",
    },
    {
        "Move Window to Left(Display Wrap)",
        "移动活动窗口至左侧屏幕",
        "移動活動窗口至左側屏幕",
        "སྤོ་འགུལ་བྱ་འགུལ་དྲ་མིག་གཡོན་ཕྱོགས་ཀྱི་འཆར་ངོས་སུ་བཞག",
        "Хөдөлгөөн талбарыг зүүн талын дэлгэц рүү шилжүүлэх",
    },
    {
        "Move Window to Right(Display Wrap)",
        "移动活动窗口至右侧屏幕",
        "移動活動窗口至右側屏幕",
        "འགུལ་སྐྱོད་བྱེད་སའི་སྒེའུ་ཁུང་དེ་གཡས་ཕྱོགས་སུ་འགུལ་དགོས།",
        "Хөдөлгөөн талбарыг баруун талын дэлгэц рүү шилжүүлэх",
    },
    {
        "Move and Maximize Window Left(Display Wrap)",
        "移动活动窗口至左侧屏幕并最大化",
        "移動活動窗口至左側屏幕並最大化",
        "སྤོ་འགུལ་བྱ་འགུལ་སྒེའུ་ཁུང་དེ་གཡོན་ཕྱོགས་ཀྱི་འཆར་ངོས་དང་ཆེས་ཆེ་བར་གཏོང་བ།",
        "Хөдөлгөөн талбарыг зүүн талын дэлгэц рүү шилжүүлэх ба томруулах",
    },
    {
        "Move and Maximize Window Right(Display Wrap)",
        "移动活动窗口至右侧屏幕并最大化",
        "移動活動窗口中右側屏幕並最大化",
        "སྤོ་འགུལ་བྱ་འགུལ་སྒེའུ་ཁུང་གཡས་ཕྱོགས་སུ་གནས་པ་མ་ཟད་ཆེས་ཆེ་བའི་རང་བཞིན།",
        "Хөдөлгөөн талбарыг баруун талын дэлгэц рүү шилжүүлэх ба томруулах",
    },
    {
        "Traverse All Maximized/Fullscreen Windows",
        "遍历最大化/全屏窗口",
        "遍歷最大化/全屏窗口",
        "ཡོངས་སུ་ཁྱབ་པ་ཆེས་ཆེ་བའམ་ཡང་ན་སྒེའུ་ཁུང་གི་ཤེལ།",
        "Бүх цонхийг ихэсгэх/баруун цонх",
    },
    {
        "Toggle ColorFilter",
        "开关色滤",
        "開關色濾",
        "གློག་སྒོ་མདོག་ཚགས།",
        "Өнгөний шүүлтүүрээ хаягдал",
    },
    {
        "Magnifier Zoom In",
        "放大镜(放大)",
        "放大鏡(放大)",
        "ཆེ་ཤེལ་（ཆེར་བསྐྱེད་）།",
        "Хувьсгагч (Томруулах)",
    },
    {
        "Magnifier Zoom Out",
        "放大镜(缩小)",
        "放大鏡(縮小)",
        "ཆེ་ཤེལ་（ཇེ་ཆུང་）།",
        "Хувьсгагч (Жижигрүүлэх)",
    },
    {
        "Magnifier Zoom Stop",
        "放大镜(停止)",
        "放大鏡(停止)",
        "ཆེ་ｼེལ་（མཚམས་）།",
        "Хувьсгагч (Зогсоох)",
    },
    {
        "Show menu",
        "显示菜单",
        "顯示菜單",
        "ཚལ་ཐོ་གསལ་བར་བྱེད་པ།",
        "Менюг харуулах",
    },
    {
        "Notification Center",
        "通知中心",
        "通知中心",
        "བརྡ་ཐོ་ལྟེ་གནས།",
        "Мессежийн төв",
    },
    {
        "Left Click",
        "鼠标左键",
        "鼠標左鍵",
        "ཙི་གུའི་གཡོན་མཛུབ།",
        "Зөөлтийн зүүн товч",
    },
    {
        "Right Click",
        "鼠标右键",
        "鼠標右鍵",
        "ཙི་གུའི་གཡས་མཛུབ།",
        "Зөөлтийн баруун товч",
    },
    {
        "Middle Click",
        "鼠标中键",
        "鼠標中鍵",
        "ཙི་གུའི་དབུས་མཛུབ།",
        "Зөөлтийн дунд товч",
    },
    {
        "App Switch",
        "切换应用",
        "切換應用",
        "བརྗེ་སྤྱོད།",
        "Програмыг шилжүүлэх",
    },
    {
        "Reverse App Switch",
        "反向切换应用",
        "反向切換應用",
        "ལྡོག་ཕྱོགས་བརྗེ་སྤྱོད།",
        "Програмыг урвуу шилжүүлэх",
    },
    {
        "Multi Taskview",
        "任务视图",
        "任務視圖",
        "ལས་འགན་གྱི་མཐོང་རིས།",
        "Хөтөлбөрийн харагдах хэсэг",
    },
    {
        "Global Search",
        "打开搜索",
        "打開搜索",
        "འཚོལ་བཤེར་སྒོ་ཕྱེས།",
        "Хайхыг нээх",
    },
    {
        "Tablet Taskbar",
        "平板任务栏",
        "平板任務欄",
        "ངོས་སྙོམས་རྣམ་པའི་ལས་འགན་སྒྲོམ།",
        "Хөвөөний хөтөлбөрийн товчлуур",
    },
    {
        "Follow Gesture to Switch to Left Workspace",
        "切换左边工作区(手势跟随)",
        "切換左邊工作區(手勢跟隨)",
        "གཡོན་ཕྱོགས་ཀྱི་ལས་ཀ་བྱེད་སར་བརྗེ་བ།",
        "Зүүн ажиллах зайруудыг шилжүүлэх (гарын жигнэсийн дагуу)",
    },
    {
        "Follow Gesture to Switch to Right Workspace",
        "切换右边工作区(手势跟随)",
        "切換右邊工作區(手勢跟隨)",
        "གཡས་ཕྱོགས་ཀྱི་བྱ་བའི་ཁུལ་བརྗེ་རོགས།",
        "Баруун ажиллах зайруудыг шилжүүлэх (гарын жигнэсийн дагуу)",
    },
    {
        "Play/Pause",
        "播放/暂停",
        "播放/暫停",
        "གཏོང་།/འཇོག་པ།",
        "Тоглуулах/Түр зогсоох",
    },
    {
        "Pause",
        "暂停",
        "暫停",
        "མཚམས་འཇོག།",
        "Түр зогсоох",
    },
    {
        "Next Track",
        "下一首",
        "下一首",
        "གླུ་རྗེས་མ",
        "Дараагийн дуу",
    },
    {
        "Previous Track",
        "上一首",
        "上一首",
        "གླུ་སྔ་མ",
        "Өмнөх дуу",
    },
    {
        "Volume Up",
        "音量+",
        "音量+",
        "སྒྲ་ཚད་ཇེ་ཆེ",
        "Дууны хэмжээ нэмэх",
    },
    {
        "Volume Down",
        "音量-",
        "音量-",
        "སྒྲ་ཚད་ཇེ་ཆུང",
        "Дууны хэмжээ багасгах",
    },
    {
        "Mute",
        "静音",
        "靜音",
        "སྒྲ་མེད",
        "Дуугүй болгох",
    },
    {
        "Volume Control",
        "音量控制",
        "音量控制",
        "སྒྲ་ཚད་ཚོད་འཛིན",
        "Дууны удирдлага",
    },
    {
        "Rewind",
        "快退",
        "快退",
        "མགྱོགས་པོར་ཕྱིར་བསྐྱོད།",
        "Буцааж гүйлгэх",
    },
    {
        "Fast Forward",
        "快进",
        "快進",
        "མགྱོགས་པོར་མདུན་བསྐྱོད།",
        "Урагш гүйлгэх",
    },
    {
        "Media Progress Control",
        "播放进度控制",
        "播放進度控制",
        "གཏོང་ཚད་ཚོད་འཛིན།",
        "Тоглуулах явцыг удирдах",
    },
    {
        "Brightness Up",
        "亮度+",
        "亮度+",
        "འོད་ཚད་ཇེ་ཆེ",
        "Гэрэлтүүлгийг нэмэх",
    },
    {
        "Brightness Down",
        "亮度-",
        "亮度-",
        "འོད་ཚད་ཇེ་ཆུང",
        "Гэрэлтүүлгийг багасгах",
    },
    {
        "Brightness Control",
        "亮度控制",
        "亮度控制",
        "འོད་ཚད་ཚོད་འཛིན",
        "Гэрэлтүүлгийн удирдлага",
    },
};

static struct function_entry {
    enum function_id func_id;
    enum key_binding_type type;

    const char *desc;
    const char *l10n_name;
} functions[] = {
    {
        FUNC_ID_NONE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "None",
        NULL,
    },
    {
        FUNC_ID_CUSTOM,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "User Define Function",
        NULL,
    },
    {
        FUNC_ID_CONFIG,
        KEY_BINDING_TYPE_CUSTOM_CONFIG,
        "Custom Configure Function",
        NULL,
    },
    {
        FUNC_ID_NEW_WORKSPACE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "New Workspace",
        NULL,
    },
    {
        FUNC_ID_DEL_WORKSPACE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Delete Workspace",
        NULL,
    },
    {
        FUNC_ID_SWITCH_LEFT_WORKSPACE,
        KEY_BINDING_TYPE_SWITCH_WORKSPACE,
        "Switch to Left Workspace",
        NULL,
    },
    {
        FUNC_ID_SWITCH_RIGHT_WORKSPACE,
        KEY_BINDING_TYPE_SWITCH_WORKSPACE,
        "Switch to Right Workspace",
        NULL,
    },

    {
        FUNC_ID_SWITCH_01_WORKSPACE,
        KEY_BINDING_TYPE_SWITCH_WORKSPACE,
        "Switch to Workspace 1",
        NULL,
    },
    {
        FUNC_ID_SWITCH_02_WORKSPACE,
        KEY_BINDING_TYPE_SWITCH_WORKSPACE,
        "Switch to Workspace 2",
        NULL,
    },
    {
        FUNC_ID_SWITCH_03_WORKSPACE,
        KEY_BINDING_TYPE_SWITCH_WORKSPACE,
        "Switch to Workspace 3",
        NULL,
    },
    {
        FUNC_ID_SWITCH_04_WORKSPACE,
        KEY_BINDING_TYPE_SWITCH_WORKSPACE,
        "Switch to Workspace 4",
        NULL,
    },
    {
        FUNC_ID_TOGGLE_DESKTOP_VISIBILITY,
        KEY_BINDING_TYPE_TOGGLE_SHOW_DESKTOP,
        "Toggle Desktop Visibility",
        NULL,
    },
    {
        FUNC_ID_SHOW_DESKTOP,
        KEY_BINDING_TYPE_SHOW_DESKTOP,
        "Show Desktop(Minimize All)",
        NULL,
    },
    {
        FUNC_ID_RESTORE_DESKTOP_WINDOWS,
        KEY_BINDING_TYPE_RESTORE_DESKTOP,
        "Restore All Desktop Windows",
        NULL,
    },
    {
        FUNC_ID_HIDE_INACTIVE_WINDOWS,
        KEY_BINDING_TYPE_TOGGLE_SHOW_WINDOWS,
        "Minimize Other Windows",
        NULL,
    },
    {
        FUNC_ID_RESTORE_INACTIVE_WINDOWS,
        KEY_BINDING_TYPE_TOGGLE_SHOW_WINDOWS,
        "Restore Other Windows",
        NULL,
    },
    {
        FUNC_ID_TOGGLE_INACTIVE_WINDOWS,
        KEY_BINDING_TYPE_TOGGLE_SHOW_WINDOWS,
        "Minimize or Restore Other Windows",
        NULL,
    },
    {
        FUNC_ID_MAXIMIZE_ACTIVE_WINDOW,
        KEY_BINDING_TYPE_WINDOW_ACTION_MAXIMIZE,
        "Maximize Active Window",
        NULL,
    },
    {
        FUNC_ID_MINIMIZE_ACTIVE_WINDOW,
        KEY_BINDING_TYPE_WINDOW_ACTION_MINIMIZE,
        "Minimize Active Window",
        NULL,
    },
    {
        FUNC_ID_CLOSE_ACTIVE_WINDOW,
        KEY_BINDING_TYPE_WINDOW_ACTION_CLOSE,
        "Close Active Window",
        NULL,
    },
    {
        FUNC_ID_SHOW_ACTIVE_WINDOW_MENU,
        KEY_BINDING_TYPE_WINDOW_ACTION_MENU,
        "Show Active Window Menu",
        NULL,
    },
    {
        FUNC_ID_CAPTURE_ACTIVE_WINDOW,
        KEY_BINDING_TYPE_WINDOW_ACTION_CAPTURE,
        "Capture Active Window",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_TOP,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Top",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Bottom",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_LEFT,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Left",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Right",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_TOP_HALF,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Top Half",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM_HALF,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Bottom Half",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_LEFT_HALF,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Left Half",
        NULL,
    },
    {
        FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT_HALF,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Tile Active Window to the Right Half",
        NULL,
    },
    {
        FUNC_ID_SEND_ACTIVE_WINDOW_LEFT,
        KEY_BINDING_TYPE_WINDOW_ACTION_OUTPUT,
        "Move Active Window to Left(Display Wrap)",
        NULL,
    },
    {
        FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT,
        KEY_BINDING_TYPE_WINDOW_ACTION_OUTPUT,
        "Move Active Window to Right(Display Wrap)",
        NULL,
    },
    {
        FUNC_ID_SEND_ACTIVE_WINDOW_LEFT_MAXIMIZE,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Move and Maximize Active Window Left(Display Wrap)",
        NULL,
    },
    {
        FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT_MAXIMIZE,
        KEY_BINDING_TYPE_WINDOW_ACTION_TILED,
        "Move and Maximize Active Window Right(Display Wrap)",
        NULL,
    },
    {
        FUNC_ID_TRAVERSE_MAXIMIZED_FULLSCREEN_WINDOWS,
        KEY_BINDING_TYPE_MAXIMIZED_VIEWS,
        "Traverse All Maximized Window",
        NULL,
    },
    {
        FUNC_ID_TOGGLE_COLOR_FILTER,
        KEY_BINDING_TYPE_COLOR_FILTER,
        "Toggle Color Filter",
        NULL,
    },
    {
        FUNC_ID_ZOOM_IN,
        KEY_BINDING_TYPE_ZOOM,
        "Magnifier Zoom In",
        NULL,
    },
    {
        FUNC_ID_ZOOM_OUT,
        KEY_BINDING_TYPE_ZOOM,
        "Magnifier Zoom Out",
        NULL,
    },
    {
        FUNC_ID_ZOOM_STOP,
        KEY_BINDING_TYPE_ZOOM,
        "Magnifier Zoom Stop",
        NULL,
    },
    {
        FUNC_ID_SHOW_UKUI_MENU,
        KEY_BINDING_TYPE_WIN_MENU,
        "Show menu",
        NULL,
    },
    {
        FUNC_ID_SHOW_UKUI_SIDEBAR,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Notification Center",
        NULL,
    },
    {
        FUNC_ID_SIMULATE_LEFTBTN,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Left Click",
        NULL,
    },
    {
        FUNC_ID_SIMULATE_RIGHTBTN,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Right Click",
        NULL,
    },
    {
        FUNC_ID_SIMULATE_MIDDLEBTN,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Middle Click",
        NULL,
    },
    {
        FUNC_ID_WINDOWS_SWITCH,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "App Switch",
        NULL,
    },
    {
        FUNC_ID_WINDOWS_SWITCH_BACKWARD,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Reverse App Switch",
        NULL,
    },
    {
        FUNC_ID_MULTI_TASKVIEW,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Multi Taskview",
        NULL,
    },
    {
        FUNC_ID_UKUI_GLOBAL_SEARCH,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Global Search",
        NULL,
    },
    {
        FUNC_ID_TABLET_MODE_TASK_BAR_GESTURE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Tablet Taskbar",
        NULL,
    },
    {
        FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Follow Gesture to Switch to Left Workspace",
        NULL,
    },
    {
        FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Follow Gesture to Switch to Right Workspace",
        NULL,
    },
    {
        FUNC_ID_MEDIA_PLAYPAUSE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Play/Pause",
        NULL,
    },
    {
        FUNC_ID_MEDIA_PAUSE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Pause",
        NULL,
    },
    {
        FUNC_ID_MEDIA_NEXT,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Next Track",
        NULL,
    },
    {
        FUNC_ID_MEDIA_PREV,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Previous Track",
        NULL,
    },
    {
        FUNC_ID_MEDIA_VOL_UP,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Volume Up",
        NULL,
    },
    {
        FUNC_ID_MEDIA_VOL_DOWN,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Volume Down",
        NULL,
    },
    {
        FUNC_ID_MEDIA_VOL_MUTE,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Volume Mute",
        NULL,
    },
    {
        FUNC_ID_MEDIA_VOL_CONTROL,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Volume Control",
        NULL,
    },
    {
        FUNC_ID_MEDIA_REWIND,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Rewind",
        NULL,
    },
    {
        FUNC_ID_MEDIA_FAST_FORWARD,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Fast Forward",
        NULL,
    },
    {
        FUNC_ID_MEDIA_PROGRESS_CONTROL,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Media Progress Control",
        NULL,
    },
    {
        FUNC_ID_BRIGHTNESS_UP,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Brightness Up",
        NULL,
    },
    {
        FUNC_ID_BRIGHTNESS_DOWN,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Brightness Down",
        NULL,
    },
    {
        FUNC_ID_BRIGHTNESS_CONTROL,
        KEY_BINDING_TYPE_CUSTOM_DEF,
        "Brightness Control",
        NULL,
    },
};

static struct function_gesture_optional {
    enum function_id id;
    uint32_t stages;
    uint32_t follow_directions;
    double follow_threshold[2];
} function_gesture_optionals[] = {
    {
        FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_NONE,
        { 0.0, 0.0 },
    },
    {
        FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_NONE,
        { 0.0, 0.0 },
    },
    {
        FUNC_ID_TABLET_MODE_TASK_BAR_GESTURE,
        GESTURE_STAGE_BEFORE | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP,
        { 0.0, 0.0 },
    },
    {
        FUNC_ID_WINDOWS_SWITCH,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN | GESTURE_DIRECTION_LEFT |
            GESTURE_DIRECTION_RIGHT,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_WINDOWS_SWITCH_BACKWARD,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER | GESTURE_STAGE_STOP,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN | GESTURE_DIRECTION_LEFT |
            GESTURE_DIRECTION_RIGHT,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_MEDIA_VOL_UP,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_NONE,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_MEDIA_VOL_DOWN,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_NONE,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_MEDIA_VOL_CONTROL,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_MEDIA_REWIND,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_NONE,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_MEDIA_FAST_FORWARD,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_NONE,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_MEDIA_PROGRESS_CONTROL,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_LEFT | GESTURE_DIRECTION_RIGHT,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_BRIGHTNESS_UP,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_NONE,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_BRIGHTNESS_DOWN,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_NONE,
        { 50.0, 0.15 },
    },
    {
        FUNC_ID_BRIGHTNESS_CONTROL,
        GESTURE_STAGE_TRIGGER | GESTURE_STAGE_AFTER,
        GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN,
        { 50.0, 0.15 },
    },
};

static const struct function_key_binding_optional {
    enum function_id id;
    uint32_t options;
} function_key_binding_optionals[] = {
    {
        FUNC_ID_SHOW_UKUI_MENU,
        KEY_BINDING_TRIGGER_IN_INACTIVE,
    },
};

const char *function_entry_get_desc(enum function_id id)
{
    return functions[id].desc;
}

const char *function_entry_get_l10n_name(enum function_id id)
{
    return functions[id].l10n_name;
}

enum key_binding_type function_entry_get_type(enum function_id id)
{
    return functions[id].type;
}

uint32_t function_entry_get_key_binding_options(enum function_id id)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(function_key_binding_optionals); ++i) {
        if (function_key_binding_optionals[i].id != id) {
            continue;
        }

        return function_key_binding_optionals[i].options;
    }

    return 0;
}

static uint32_t function_get_gesture_follow_directions(enum function_id id,
                                                       enum gesture_direction binding_direction,
                                                       uint32_t follow_directions)
{
    if (id != FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE &&
        id != FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE) {
        return follow_directions;
    }

    switch (binding_direction) {
    case GESTURE_DIRECTION_LEFT:
    case GESTURE_DIRECTION_RIGHT:
        return GESTURE_DIRECTION_LEFT | GESTURE_DIRECTION_RIGHT;
    case GESTURE_DIRECTION_UP:
    case GESTURE_DIRECTION_DOWN:
        return GESTURE_DIRECTION_UP | GESTURE_DIRECTION_DOWN;
    default:
        return follow_directions;
    }
}

bool function_entry_get_gesture_optional(enum function_id id, enum gesture_device device,
                                         enum gesture_direction binding_direction, uint32_t *stages,
                                         uint32_t *follow_directions, double *follow_threshold)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(function_gesture_optionals); ++i) {
        if (function_gesture_optionals[i].id != id) {
            continue;
        }
        *stages = function_gesture_optionals[i].stages;
        *follow_directions = function_get_gesture_follow_directions(
            id, binding_direction, function_gesture_optionals[i].follow_directions);
        *follow_threshold = device == GESTURE_DEVICE_TOUCHPAD
                                ? function_gesture_optionals[i].follow_threshold[0]
                                : function_gesture_optionals[i].follow_threshold[1];
        return true;
    }

    *stages = GESTURE_STAGE_TRIGGER;
    *follow_directions = GESTURE_DIRECTION_NONE;
    *follow_threshold = 0.0;

    return false;
};

static enum language get_language_from_system(void)
{
    const char *system_lang = setlocale(LC_ALL, "");
    if (!strncmp(system_lang, "zh_CN", 5)) {
        return LANG_ZH_CN;
    } else if (!strncmp(system_lang, "zh_HK", 5)) {
        return LANG_ZH_HK;
    } else if (!strncmp(system_lang, "en_US", 5)) {
        return LANG_EN_US;
    } else if (!strncmp(system_lang, "bo_CN", 5)) {
        return LANG_BO_CN;
    } else if (!strncmp(system_lang, "mn_MN", 5)) {
        return LANG_MN_MN;
    }

    return LANG_EN_US;
}

static int list_functions(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    sd_bus_message *reply = NULL;
    CK(sd_bus_message_new_method_return(m, &reply));
    CK(sd_bus_message_open_container(reply, 'a', "{uv}"));
    for (size_t i = 0; i < ARRAY_SIZE(functions); ++i) {
        uint32_t func_id = functions[i].func_id;
        CK(sd_bus_message_open_container(reply, 'e', "uv"));
        CK(sd_bus_message_append_basic(reply, 'u', &func_id));
        CK(sd_bus_message_open_container(reply, 'v', "a{sv}"));
        CK(sd_bus_message_open_container(reply, 'a', "{sv}"));

        CK(sd_bus_message_append(reply, "{sv}", "name", "s", function_entry_get_desc(func_id)));
        CK(sd_bus_message_append(reply, "{sv}", "l10n", "s",
                                 function_entry_get_l10n_name(func_id)));

        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
        CK(sd_bus_message_close_container(reply));
    }

    CK(sd_bus_message_close_container(reply));
    sd_bus_send(NULL, reply, NULL);
    sd_bus_message_unref(reply);

    return 1;
}

static const sd_bus_vtable service_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("ListFunctions", "", "a{uv}", list_functions, 0),
    SD_BUS_VTABLE_END,
};

bool function_manager_init(void)
{
    enum language lan = get_language_from_system();

    for (size_t i = 0; i < ARRAY_SIZE(functions); ++i) {
        if (functions[i].func_id != FUNC_ID_CUSTOM) {
            functions[i].l10n_name = function_translation[i][lan];
        }
    }
    return dbus_register_object(NULL, service_path, service_interface, service_vtable, NULL);
}

static void simulate_button(struct seat *seat, uint32_t button)
{
    struct cursor *cursor = seat->cursor;
    seat_feed_pointer_motion(seat, cursor->lx, cursor->ly, true);

    if (seat->pointer_grab && cursor->last_click_button == BTN_LEFT && button == BTN_RIGHT) {
        seat_feed_pointer_button(seat, BTN_LEFT, false);
    }

    seat_feed_pointer_button(seat, button, true);
    seat_feed_pointer_button(seat, button, false);

    seat_notify_leave(seat, NULL);
}

static void simulate_key_send(struct seat *seat, uint32_t key, bool pressed)
{
    seat_feed_keyboard_key(seat, key, pressed);
}

static void ukui_global_search(void)
{
    if (!dbus_call_method("com.ukui.search.service", "/", "org.ukui.search.service",
                          "mainWindowSwitch", NULL, NULL)) {
        kywc_log(KYWC_ERROR, "Dbus call org.ukui.search.service.mainWindowSwitch failed");
    }
}

static void ukui_menu_show(void)
{
    if (!dbus_call_method("org.ukui.menu", "/org/ukui/menu", "org.ukui.menu", "WinKeyResponse",
                          NULL, NULL)) {
        kywc_log(KYWC_ERROR, "Dbus call org.ukui.menu failed");
    }
}

static void ukui_media_do_action(const char *action)
{
    if (!dbus_call_methodv("org.ukui.SettingsDaemon", "/org/ukui/SettingsDaemon/MediaKeys",
                           "org.ukui.SettingsDaemon.MediaKeys", "externalDoActionWithName", NULL,
                           NULL, "ss", action, "kylin-wlcom")) {
        kywc_log(KYWC_ERROR, "Dbus call MediaKeys.externalDoActionWithName(%s) failed", action);
    }
}

static enum gesture_direction gesture_direction_from_delta(double dx, double dy)
{
    double abs_dx = dx > 0.0 ? dx : -dx;
    double abs_dy = dy > 0.0 ? dy : -dy;

    if (abs_dx > abs_dy) {
        return dx > 0.0 ? GESTURE_DIRECTION_RIGHT : GESTURE_DIRECTION_LEFT;
    }

    return dy > 0.0 ? GESTURE_DIRECTION_DOWN : GESTURE_DIRECTION_UP;
}

static enum gesture_direction control_gesture_direction(enum gesture_direction binding_direction,
                                                        uint32_t follow_direction, double dx,
                                                        double dy)
{
    if (follow_direction != GESTURE_DIRECTION_NONE) {
        return follow_direction;
    }
    if (binding_direction == GESTURE_DIRECTION_UP || binding_direction == GESTURE_DIRECTION_DOWN ||
        binding_direction == GESTURE_DIRECTION_LEFT ||
        binding_direction == GESTURE_DIRECTION_RIGHT) {
        return binding_direction;
    }

    return gesture_direction_from_delta(dx, dy);
}

static void ukui_media_control_do_action(enum gesture_direction direction)
{
    switch (direction) {
    case GESTURE_DIRECTION_UP:
    case GESTURE_DIRECTION_RIGHT:
        ukui_media_do_action("VOLUME_UP_KEY");
        break;
    case GESTURE_DIRECTION_DOWN:
    case GESTURE_DIRECTION_LEFT:
        ukui_media_do_action("VOLUME_DOWN_KEY");
        break;
    default:
        break;
    }
}

static void ukui_media_progress_control_do_action(enum gesture_direction direction)
{
    switch (direction) {
    case GESTURE_DIRECTION_LEFT:
    case GESTURE_DIRECTION_DOWN:
        ukui_media_do_action("REWIND_KEY");
        break;
    case GESTURE_DIRECTION_RIGHT:
    case GESTURE_DIRECTION_UP:
        ukui_media_do_action("FORWARD_KEY");
        break;
    default:
        break;
    }
}

static void ukui_brightness_control_do_action(enum gesture_direction direction)
{
    switch (direction) {
    case GESTURE_DIRECTION_UP:
    case GESTURE_DIRECTION_RIGHT:
        ukui_media_do_action("BRIGHT_UP_KEY");
        break;
    case GESTURE_DIRECTION_DOWN:
    case GESTURE_DIRECTION_LEFT:
        ukui_media_do_action("BRIGHT_DOWN_KEY");
        break;
    default:
        break;
    }
}

static void ukui_sidebar_show(void)
{
    if (!dbus_call_methodv("org.ukui.Sidebar", "/org/ukui/Sidebar", "org.ukui.Sidebar", "active",
                           NULL, NULL, "s", "sidebar")) {
        kywc_log(KYWC_ERROR, "Dbus call org.ukui.Sidebar.active failed");
    }
}

static uint32_t get_key_code(enum gesture_direction direction)
{
    uint32_t keycode = KEY_RESERVED;
    switch (direction) {
    case GESTURE_DIRECTION_LEFT:
        keycode = KEY_LEFT;
        break;
    case GESTURE_DIRECTION_RIGHT:
        keycode = KEY_RIGHT;
        break;
    case GESTURE_DIRECTION_UP:
        keycode = KEY_UP;
        break;
    case GESTURE_DIRECTION_DOWN:
        keycode = KEY_DOWN;
        break;
    default:
        break;
    }
    return keycode;
}

static void ukui_windows_switch_cycle(struct seat *seat)
{
    simulate_key_send(seat, KEY_TAB, true);
    simulate_key_send(seat, KEY_TAB, false);
}

static void ukui_windows_switch_internal(struct seat *seat, enum gesture_stage stage,
                                         enum gesture_direction follow_direction, bool backward)
{
    switch (stage) {
    case GESTURE_STAGE_TRIGGER:
        simulate_key_send(seat, KEY_LEFTALT, true);
        if (backward) {
            simulate_key_send(seat, KEY_LEFTSHIFT, true);
        }
        ukui_windows_switch_cycle(seat);
        break;
    case GESTURE_STAGE_AFTER:;
        uint32_t keycode = get_key_code(follow_direction);
        if (!keycode) {
            break;
        }
        simulate_key_send(seat, keycode, true);
        simulate_key_send(seat, keycode, false);
        break;
    case GESTURE_STAGE_STOP:
        if (backward) {
            simulate_key_send(seat, KEY_LEFTSHIFT, false);
        }
        simulate_key_send(seat, KEY_LEFTALT, false);
        break;

    default:
        break;
    }
}

static void ukui_windows_switch_by_shortcut(struct seat *seat, enum gesture_stage stage,
                                            enum gesture_direction follow_direction)
{
    ukui_windows_switch_internal(seat, stage, follow_direction, false);
}

static void ukui_windows_switch_backward_by_shortcut(struct seat *seat, enum gesture_stage stage,
                                                     enum gesture_direction follow_direction)
{
    ukui_windows_switch_internal(seat, stage, follow_direction, true);
}

void function_manager_handle_key_binding(struct key_binding *binding, void *data, struct seat *seat,
                                         enum key_binding_stage stage)
{
    enum function_id id = bindings_get_key_binding_function_id(binding);

    if (!seat) {
        seat = input_manager_get_default_seat();
    }

    switch (id) {
    case FUNC_ID_CONFIG:
        input_action_handle_key_binding(binding, seat);
        break;
    case FUNC_ID_CUSTOM:
        global_shortcuts_manager_action(data, stage);
        break;
    case FUNC_ID_SWITCH_LEFT_WORKSPACE:
    case FUNC_ID_SWITCH_RIGHT_WORKSPACE:
    case FUNC_ID_SWITCH_01_WORKSPACE:
    case FUNC_ID_SWITCH_02_WORKSPACE:
    case FUNC_ID_SWITCH_03_WORKSPACE:
    case FUNC_ID_SWITCH_04_WORKSPACE:
        workspace_manager_handle_function(id, GESTURE_STAGE_NONE, GESTURE_DEVICE_NONE,
                                          GESTURE_DIRECTION_NONE, GESTURE_DIRECTION_NONE, 0.0, 0.0);
        break;
    case FUNC_ID_TOGGLE_DESKTOP_VISIBILITY:
    case FUNC_ID_SHOW_DESKTOP:
    case FUNC_ID_RESTORE_DESKTOP_WINDOWS:
    case FUNC_ID_TOGGLE_INACTIVE_WINDOWS:
    case FUNC_ID_MAXIMIZE_ACTIVE_WINDOW:
    case FUNC_ID_MINIMIZE_ACTIVE_WINDOW:
    case FUNC_ID_CLOSE_ACTIVE_WINDOW:
    case FUNC_ID_SHOW_ACTIVE_WINDOW_MENU:
    case FUNC_ID_CAPTURE_ACTIVE_WINDOW:
    case FUNC_ID_TILE_ACTIVE_WINDOW_TOP:
    case FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM:
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT:
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT:
    case FUNC_ID_TILE_ACTIVE_WINDOW_TOP_HALF:
    case FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM_HALF:
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT_HALF:
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT_HALF:
    case FUNC_ID_SEND_ACTIVE_WINDOW_LEFT:
    case FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT:
    case FUNC_ID_SEND_ACTIVE_WINDOW_LEFT_MAXIMIZE:
    case FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT_MAXIMIZE:
    case FUNC_ID_TRAVERSE_MAXIMIZED_FULLSCREEN_WINDOWS:
        view_manager_handle_function(id, GESTURE_STAGE_NONE, 0.0, 0.0);
        break;
    case FUNC_ID_TOGGLE_COLOR_FILTER:
    case FUNC_ID_ZOOM_IN:
    case FUNC_ID_ZOOM_OUT:
    case FUNC_ID_ZOOM_STOP:
        effect_manager_handle_function(id);
        break;
    case FUNC_ID_SHOW_UKUI_MENU:
        ukui_menu_show();
        break;
    case FUNC_ID_BRIGHTNESS_UP:
        ukui_media_do_action("BRIGHT_UP_KEY");
        break;
    case FUNC_ID_BRIGHTNESS_DOWN:
        ukui_media_do_action("BRIGHT_DOWN_KEY");
        break;
    default:
        break;
    }
}

void function_manager_handle_gesture_binding(struct gesture_binding *binding, void *data,
                                             struct seat *seat, enum gesture_stage stage,
                                             enum gesture_device device, uint32_t follow_direction,
                                             double dx, double dy)
{
    if (!seat) {
        seat = input_manager_get_default_seat();
    }

    enum function_id id = kywc_gesture_binding_function_id(binding);
    switch (id) {
    case FUNC_ID_CONFIG:
        input_action_handle_gesture_binding(binding, seat, stage, follow_direction);
        break;
    case FUNC_ID_NEW_WORKSPACE:
    case FUNC_ID_DEL_WORKSPACE:
    case FUNC_ID_SWITCH_LEFT_WORKSPACE:
    case FUNC_ID_SWITCH_RIGHT_WORKSPACE:
    case FUNC_ID_SWITCH_LEFT_WORKSPACE_GESTURE:
    case FUNC_ID_SWITCH_RIGHT_WORKSPACE_GESTURE:
        workspace_manager_handle_function(
            id, stage, device, kywc_gesture_binding_direction(binding), follow_direction, dx, dy);
        break;
    case FUNC_ID_TOGGLE_DESKTOP_VISIBILITY:
    case FUNC_ID_SHOW_DESKTOP:
    case FUNC_ID_RESTORE_DESKTOP_WINDOWS:
    case FUNC_ID_HIDE_INACTIVE_WINDOWS:
    case FUNC_ID_RESTORE_INACTIVE_WINDOWS:
    case FUNC_ID_TOGGLE_INACTIVE_WINDOWS:
    case FUNC_ID_MAXIMIZE_ACTIVE_WINDOW:
    case FUNC_ID_MINIMIZE_ACTIVE_WINDOW:
    case FUNC_ID_CLOSE_ACTIVE_WINDOW:
    case FUNC_ID_SHOW_ACTIVE_WINDOW_MENU:
    case FUNC_ID_CAPTURE_ACTIVE_WINDOW:
    case FUNC_ID_TILE_ACTIVE_WINDOW_TOP:
    case FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM:
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT:
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT:
    case FUNC_ID_TILE_ACTIVE_WINDOW_TOP_HALF:
    case FUNC_ID_TILE_ACTIVE_WINDOW_BOTTOM_HALF:
    case FUNC_ID_TILE_ACTIVE_WINDOW_LEFT_HALF:
    case FUNC_ID_TILE_ACTIVE_WINDOW_RIGHT_HALF:
    case FUNC_ID_SEND_ACTIVE_WINDOW_LEFT:
    case FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT:
    case FUNC_ID_SEND_ACTIVE_WINDOW_LEFT_MAXIMIZE:
    case FUNC_ID_SEND_ACTIVE_WINDOW_RIGHT_MAXIMIZE:
    case FUNC_ID_TRAVERSE_MAXIMIZED_FULLSCREEN_WINDOWS:
    case FUNC_ID_MULTI_TASKVIEW:
    case FUNC_ID_TABLET_MODE_TASK_BAR_GESTURE:
        view_manager_handle_function(id, stage, dx, dy);
        break;
    case FUNC_ID_SHOW_UKUI_SIDEBAR:
        ukui_sidebar_show();
        break;
    case FUNC_ID_WINDOWS_SWITCH:
        ukui_windows_switch_by_shortcut(seat, stage, follow_direction);
        break;
    case FUNC_ID_WINDOWS_SWITCH_BACKWARD:
        ukui_windows_switch_backward_by_shortcut(seat, stage, follow_direction);
        break;
    case FUNC_ID_UKUI_GLOBAL_SEARCH:
        ukui_global_search();
        break;
    case FUNC_ID_SIMULATE_LEFTBTN:
        simulate_button(seat, BTN_LEFT);
        break;
    case FUNC_ID_SIMULATE_RIGHTBTN:
        simulate_button(seat, BTN_RIGHT);
        break;
    case FUNC_ID_SIMULATE_MIDDLEBTN:
        simulate_button(seat, BTN_MIDDLE);
        break;
    case FUNC_ID_MEDIA_PLAYPAUSE:
        ukui_media_do_action("PLAY_KEY");
        break;
    case FUNC_ID_MEDIA_PAUSE:
        ukui_media_do_action("PAUSE_KEY");
        break;
    case FUNC_ID_MEDIA_NEXT:
        ukui_media_do_action("NEXT_KEY");
        break;
    case FUNC_ID_MEDIA_PREV:
        ukui_media_do_action("PREVIOUS_KEY");
        break;
    case FUNC_ID_MEDIA_VOL_UP:
        ukui_media_do_action("VOLUME_UP_KEY");
        break;
    case FUNC_ID_MEDIA_VOL_DOWN:
        ukui_media_do_action("VOLUME_DOWN_KEY");
        break;
    case FUNC_ID_MEDIA_VOL_MUTE:
        ukui_media_do_action("MUTE_KEY");
        break;
    case FUNC_ID_MEDIA_VOL_CONTROL:
        ukui_media_control_do_action(control_gesture_direction(
            kywc_gesture_binding_direction(binding), follow_direction, dx, dy));
        break;
    case FUNC_ID_MEDIA_REWIND:
        ukui_media_do_action("REWIND_KEY");
        break;
    case FUNC_ID_MEDIA_FAST_FORWARD:
        ukui_media_do_action("FORWARD_KEY");
        break;
    case FUNC_ID_MEDIA_PROGRESS_CONTROL:
        ukui_media_progress_control_do_action(control_gesture_direction(
            kywc_gesture_binding_direction(binding), follow_direction, dx, dy));
        break;
    case FUNC_ID_BRIGHTNESS_UP:
        ukui_media_do_action("BRIGHT_UP_KEY");
        break;
    case FUNC_ID_BRIGHTNESS_DOWN:
        ukui_media_do_action("BRIGHT_DOWN_KEY");
        break;
    case FUNC_ID_BRIGHTNESS_CONTROL:
        ukui_brightness_control_do_action(control_gesture_direction(
            kywc_gesture_binding_direction(binding), follow_direction, dx, dy));
        break;
    default:
        break;
    }
}
