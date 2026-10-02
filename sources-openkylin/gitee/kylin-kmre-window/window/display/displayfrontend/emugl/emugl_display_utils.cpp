/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/syslog.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <QString>
#include <QMutexLocker>
#include <QDir>
#include <QFile>
#include "emugl_display_utils.h"
#include "dbusclient.h"
#include "app_control_manager.h"
#include "kmreenv.h"

#define CPUINFO_PATH "/proc/cpuinfo"
#define MAX_EPOLL_EVENTS 128



typedef int (*ANDROID_INIT_ES_EMULATION)(void);
typedef int (*ANDROID_START_ES_RENDERER)(int width, int height, int* glesMajorVersion_out, int* glesMinorVersion_out);
typedef int (*ANDROID_SHOW_ES_WINDOW)(int wx, int wy, int ww, int wh, int fbw, int fbh, float dpr, float rotation, bool deleteExisting);
typedef int (*ANDROID_HIDE_ES_WINDOW)(void);
typedef void (*ANDROID_REDRAW_ES_WINDOW)(void);
typedef void (*ANDROID_STOP_ES_RENDERER)(bool wait);
typedef int (*ANDROID_UPDATE_WINDOW_ATTRI) (uint32_t display_id,unsigned long win_id,unsigned int p_colorbuffer,int32_t width,int32_t height,int32_t orientation, bool need_post);
typedef int (*ANDROID_DELETE_WINDOW_ATTRI) (uint32_t display_id);
typedef int (*ANDROID_PREPARE_SHM_DATA) (int display_id, uint32_t handle, bool needReDraw);
typedef int (*ANDROID_SET_SUPPORT_DYNAMIC_SIZE) (uint32_t display_id, unsigned int p_colorbuffer, int width, int height, bool support);

ANDROID_INIT_ES_EMULATION android_init_es_Emulation = nullptr;
ANDROID_START_ES_RENDERER android_start_es_Renderer = nullptr;
ANDROID_HIDE_ES_WINDOW android_hide_es_Window = nullptr;
ANDROID_SHOW_ES_WINDOW android_show_es_Window = nullptr;
ANDROID_REDRAW_ES_WINDOW android_redraw_es_Window = nullptr;
ANDROID_STOP_ES_RENDERER android_stop_es_Renderer = nullptr;
ANDROID_UPDATE_WINDOW_ATTRI android_update_window_attri = nullptr;
ANDROID_DELETE_WINDOW_ATTRI android_delete_window_attri = nullptr;
ANDROID_PREPARE_SHM_DATA android_prepare_shm_data = nullptr;
ANDROID_SET_SUPPORT_DYNAMIC_SIZE android_set_support_dynamic_size = nullptr;

static int cmd_get_result(const char *cmd, char *result, int maxlen)
{
    if (!result) {
        return -1;
    }

    if (!cmd || (strlen(cmd) == 0)) {// For fortify scan
        return -1;
    }

    FILE *pp = popen(cmd, "r");
    if (!pp) {
        printf("error, cannot popen cmd: %s\n", cmd);
        return -1;
    }

    int i = 0;
    char tmp[512] = {0};
    memset(tmp, 0, sizeof(tmp));

    while (fgets(tmp, sizeof(tmp), pp) != NULL) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }
        if (strlen(tmp) == 0) {// For fortify scan
            continue;
        }
        //printf("%d.get return results: %s\n", i, tmp);
        strcpy(result + i, tmp);
        i += strlen(tmp);
        if (i >= maxlen) {
            printf("get enough results, return\n");
            break;
        }
        memset(tmp, 0, sizeof(tmp));
    }

    if (pp) {
        pclose(pp);
    }

    return i;
}

static bool isFT1500From_lscpu()
{
    char* result = (char*)calloc(1024, sizeof(char));
    if (!result) {
        return true;
    }
    //memset(result, 0, 1024);
    cmd_get_result("lscpu | grep -i Phytium", result, 1024);
    syslog(LOG_DEBUG,"FT1500 CHECK result=%s",result);
    if (strcasestr(result, "FT-1500") || strcasestr(result, "FT1500")) {
        free(result);
        return true;
    }

    free(result);
    return false;
}

static bool isFT1500From_cpuinfo()
{
    char line[512] = {0};
    FILE* fp = NULL;
    const char* modelLinePrefix = "model name";
    int modelLinePrefixLength = strlen("model name");
    bool isFT1500 = false;

    syslog(LOG_DEBUG, "VirtualDisplaySocketManager: Try to detect cpu type.");

    fp = fopen(CPUINFO_PATH, "r");
    if (!fp) {
        syslog(LOG_ERR, "VirtualDisplaySocketManager: Failed to read cpuinfo.");
        return false;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, modelLinePrefix, modelLinePrefixLength) != 0) {
            continue;
        }

        if (strcasestr(line, "FT1500") ||
            strcasestr(line, "FT-1500")) {
            syslog(LOG_INFO, "VirtualDisplaySocketManager: CPU type FT1500a detected.");
            isFT1500 = true;
            break;
        }
    }

    fclose(fp);

    return isFT1500;
}

static bool isFT1500()
{
    return (isFT1500From_lscpu() || isFT1500From_cpuinfo());
}

static bool isKirin990From_lscpu()
{
    char* result = (char*)calloc(1024, sizeof(char));
    if (!result) {
        return true;
    }
    //memset(result, 0, 1024);
    cmd_get_result("lscpu | grep -i Kirin", result, 1024);
    //syslog(LOG_DEBUG,"Kirin 990 CHECK result=%s",result);
    if (strcasestr(result, "Kirin 990") || strcasestr(result, "Kirin990")) {
        free(result);
        return true;
    }

    free(result);
    return false;
}

static bool isKirin9006CFrom_lscpu()
{
    char* result = (char*)calloc(1024, sizeof(char));
    if (!result) {
        return true;
    }
    cmd_get_result("lscpu | grep -i Kirin", result, 1024);
    if (strcasestr(result, "Kirin 9006C")) {
        free(result);
        return true;
    }

    free(result);
    return false;
}

static bool is_in_cpuinfo(const char *fmt, const char *str)
{
    FILE *cpuinfo;
    char field[256];
    char format[256];
    bool found = false;

    sprintf(format, "%s : %s", fmt, "%255s");

    cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        do {
            if (fscanf(cpuinfo, format, field) == 1) {
                if (strncmp(field, str, strlen(str)) == 0)
                    found = true;
                break;
            }
        } while (fgets(field, 256, cpuinfo));
        fclose(cpuinfo);
    }

    return found;
}

static int is_kirin990()
{
    int type = 0;
    char line[512] = {0};
    FILE* fp = NULL;

    fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        syslog(LOG_ERR, "ContainerManager: Failed to read cpuinfo.");
        return type;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strcasestr(line, "Kirin") && (strcasestr(line, "990"))) {
            type = 1;
            break;
        }
    }

    fclose(fp);

    return type;
}

static int is_kirin9006C()
{
    int type = 0;
    char line[512] = {0};
    FILE* fp = NULL;

    fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        syslog(LOG_ERR, "ContainerManager: Failed to read cpuinfo.");
        return type;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strcasestr(line, "Kirin") && (strcasestr(line, "9006C"))) {
            type = 1;
            break;
        }
    }

    fclose(fp);

    return type;
}

static int is_PANGU_M900()
{
    int type = 0;
    char line[512] = {0};
    FILE* fp = NULL;

    fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        syslog(LOG_ERR, "ContainerManager: Failed to read cpuinfo.");
        return type;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strcasestr(line, "PANGU") && (strcasestr(line, "M900"))) {
            type = 1;
            break;
        }
    }

    fclose(fp);

    return type;
}

static bool isKirin990()
{
    return (isKirin990From_lscpu() || is_in_cpuinfo("Hardware", "Kirin990") || is_kirin990());
}

static bool isKirin9006C()
{
    return (isKirin9006CFrom_lscpu() || is_kirin9006C());
}

template<typename FUNC>
FUNC getFuncAndCheck(void *libHandle, const char *funcName)
{
    FUNC func = nullptr;

    if (libHandle) {
        func = (FUNC)dlsym(libHandle, funcName);
        if (!func) {
            syslog(LOG_ERR, "[%s]Get function '%s' from library failed!", __func__, funcName);
        }
    }
    else {
        syslog(LOG_ERR, "[%s]Invalid library handle!", __func__);
    }
    
    return func;
}

int GetOpenGLES3RenderFunc()
{
    static void *libHandle = nullptr;
    std::string renderLibraryName;
    static bool isWaylandRender = false;

    
    if (!libHandle) {
        if (isFT1500()) {
            renderLibraryName = std::string("libOpenglRender_ft1500.so");
        } 
        else if (isKirin990() || is_PANGU_M900() || isKirin9006C()) {
            // Kirin990 use wayland render
            renderLibraryName = std::string("libOpenglRender_wayland.so");
            isWaylandRender = true;
        } 
        else {
            renderLibraryName = std::string("libOpenglRender.so");
        }

        if (!(libHandle = dlopen(renderLibraryName.c_str(), RTLD_LAZY))) {
            const char *err;
            if (err = dlerror()) {
                syslog(LOG_CRIT, "Open library '%s' failed!, error: '%s'", renderLibraryName.c_str(), err);
            }
            return -1;
        }
    }

    // get functions
    android_init_es_Emulation = getFuncAndCheck<ANDROID_INIT_ES_EMULATION>(libHandle, "android_initOpenglesEmulation");
    android_start_es_Renderer = getFuncAndCheck<ANDROID_START_ES_RENDERER>(libHandle, "android_startOpenglesRenderer");
    android_hide_es_Window = getFuncAndCheck<ANDROID_HIDE_ES_WINDOW>(libHandle, "android_hideOpenglesWindow");
    android_show_es_Window = getFuncAndCheck<ANDROID_SHOW_ES_WINDOW>(libHandle, "android_showOpenglesWindow");
    android_redraw_es_Window = getFuncAndCheck<ANDROID_REDRAW_ES_WINDOW>(libHandle, "android_redrawOpenglesWindow");
    android_stop_es_Renderer = getFuncAndCheck<ANDROID_STOP_ES_RENDERER>(libHandle, "android_stopOpenglesRenderer");
    android_update_window_attri = getFuncAndCheck<ANDROID_UPDATE_WINDOW_ATTRI>(libHandle, "android_updateWindowAttri");
    android_delete_window_attri = getFuncAndCheck<ANDROID_DELETE_WINDOW_ATTRI>(libHandle, "android_deleteWindowAttri");
    if (isWaylandRender) {
        android_prepare_shm_data = getFuncAndCheck<ANDROID_PREPARE_SHM_DATA>(libHandle, "android_prepareShmData");
    }
    android_set_support_dynamic_size = getFuncAndCheck<ANDROID_SET_SUPPORT_DYNAMIC_SIZE>(libHandle, "android_setSupportDynamicSize");
    
    // have no 'android_set_support_dynamic_size' is not critical error, so don't check it below!
    if (!android_init_es_Emulation || !android_start_es_Renderer || !android_hide_es_Window || 
        !android_show_es_Window || !android_redraw_es_Window || !android_update_window_attri ||
        !android_delete_window_attri || (isWaylandRender && !android_prepare_shm_data)) {
        dlclose(libHandle);
        libHandle = nullptr;
        return -2;
    }

    return 0;
}

int initOpenGLESRenderForEmu(int width,int height)
{
    QString noCallFile = KmreEnv::GetConfigPath() + "/no_call_emugl";
    if (QFile(noCallFile).exists()) {
        syslog(LOG_INFO, "[%s] No need call emugl.", __func__);
        return 0;
    }

    if (GetOpenGLES3RenderFunc() < 0) {
        syslog(LOG_ERR, " OpenGLES emulation library mismatch. Be sure to use the correct version.");
        return -1;
    }

    if (android_init_es_Emulation == NULL) {
        syslog(LOG_ERR, " android_init_es_Emulation == NULL.");
        return -2;
    }

    if (android_init_es_Emulation() != 0) {
        fprintf(stderr,"Failed to initialize Opengles Emulation\n");
        return -3;
    }

    syslog(LOG_DEBUG, " Start OpenGLES renderer.");
    if (android_start_es_Renderer == NULL) {
        syslog(LOG_ERR, " android_start_es_Renderer == NULL.");
        return -4;
    }

    int gles_major_version = 2;
    int gles_minor_version = 0;
    if (android_start_es_Renderer(width, height, &gles_major_version , &gles_minor_version) != 0) {
        fprintf(stderr,"Failed to start Opengles Renderer\n");
        return -5;
    }

    syslog(LOG_DEBUG, " gles_major_version = %d", gles_major_version );
    syslog(LOG_DEBUG, " gles_minor_version = %d", gles_minor_version);

    if (android_show_es_Window == NULL) {
        syslog(LOG_ERR, " android_show_es_Window == NULL.");
        return -6;
    }

    android_show_es_Window( 0, 0, width, height, width, height, 1.0, 0.0,false);

    return 0;
}

void deinitOpenGLESRenderForEmu()
{
    syslog(LOG_INFO, "[%s] Deinit opengles render for Emu.", __func__);
    if (android_hide_es_Window) {
        android_hide_es_Window();
    }
    if (android_stop_es_Renderer) {
        android_stop_es_Renderer(false);
    }
}

EmuGLDisplayUtils::EmuGLDisplayUtils(unsigned long winId, QObject *parent)
    : QObject(parent)
    , mWinId(winId)
    , mDisplayWidth(0)
    , mDisplayHeight(0)
    , mWidth(0)
    , mHeight(0)
    , mNeedRequestCloseAndroidApp(false)
{

}

EmuGLDisplayUtils::~EmuGLDisplayUtils()
{
}

void EmuGLDisplayUtils::updateWindowAttribute(int displayId, uint32_t nativeHandle, int width, int height, int orientation, bool needPost)
{
    bool needUpdate = false;
    {
        QMutexLocker lock(&mDisplayWindowsMapLock);
        if(!mDisplayWindowsMap.contains(displayId)) {
            mDisplayWindowsMap.insert(displayId, {mWinId, width, height, orientation});
            needUpdate = true;
        }
        else {
            DisplayWindowAttri displayAttri = mDisplayWindowsMap[displayId];
            if((displayAttri.winId != mWinId) || 
                (displayAttri.width != width) || 
                (displayAttri.height != height) || 
                (displayAttri.orientation != orientation)) {
                mDisplayWindowsMap.insert(displayId, {mWinId, width, height, orientation});
                needUpdate = true;
            }
        }
    }

    if ((needUpdate || needPost) && android_update_window_attri) {
        android_update_window_attri(displayId, mWinId, nativeHandle, width, height, orientation , needPost);
    }
}

void EmuGLDisplayUtils::deleteWindowAttribute(int displayId)
{
    if (android_delete_window_attri) {
        android_delete_window_attri(displayId);
    }

    {
        QMutexLocker lock(&mDisplayWindowsMapLock);
        mDisplayWindowsMap.remove(displayId);
    }
    //mLastBufferHandleMap.remove(displayId);

    if (!mNeedRequestCloseAndroidApp) {
        if (!QFile::exists(KmreEnv::GetConfigPath() + "/no_close_android_app")) {
            mNeedRequestCloseAndroidApp = true;
        }
        else {
            return;
        }
    }

    //AppControlManager::getInstance()->closeAppById(displayId);
}

void EmuGLDisplayUtils::setSupportDynamicSize(int displayId, uint32_t nativeHandle, int width, int height, bool support)
{
    if (android_set_support_dynamic_size) {
        android_set_support_dynamic_size(displayId, nativeHandle, width, height, support);
    }
}

int EmuGLDisplayUtils::getShmFd(int displayId, uint32_t nativeHandle, bool needPost)
{
    int shmFd = -1;
    if (android_prepare_shm_data) {
        shmFd = android_prepare_shm_data(displayId, nativeHandle, needPost);      
    }

    return shmFd;
}
