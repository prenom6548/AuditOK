/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#pragma once

#include <QObject>
#include <syslog.h>
#include <semaphore.h>
#include <string>
#include <shared_mutex>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <set>
#include <condition_variable>
#include "shader.h"

#include "grabber.h"

enum class DisplayType;

namespace kmre {

#define KEY_SHM_DATA_INFO       0x68788898
#define SEM_NAME_FOR_SHM        "sem_shm"
#define SEM_NAME_FOR_GRAB       "sem_grab"
#define DEFAULT_BO_FORMAT_BPP   4
#define DELAY_TIME_TO_STOP_GRAB 1.5 //s
#define SLEEP_TIME_IDLE         10  //ms
//reset value to enable next grab
#define RESET_VALUE_FOR_NEXT_GRAB ((DELAY_TIME_TO_STOP_GRAB * 1000) / SLEEP_TIME_IDLE)

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t size;
    PIX_FMT format;
    void *pixel_data;
}shm_data_info;

typedef struct {
    uint32_t shm_key;
    int shm_id;
    void *shm_data;
}shm_handle;

typedef enum {
	driver_unknow,
	driver_radeon,
	driver_amdgpu,
	driver_unsupported,
}gpu_dri_ver;

class GrabberHelper : public QObject
{
    Q_OBJECT
public:
    GrabberHelper();
    virtual ~GrabberHelper();

private:
    Grabber* m_grabber;
    std::unique_ptr<Shader> m_shader;
    DisplayType mDisplayType;

    struct {
        EGLDisplay egl_display;
        EGLContext egl_context;
    }m_gles_context;

    typedef enum {
        cmd_unknow = 0,
        // refer to dbus_client definition
        cmd_start_grab = 10,
        cmd_stop_grab = 11,
    }GrabCmd;

    bool m_exit_grab;
    std::atomic_int m_grab_next;
    
    typedef struct {
        uint32_t name;
        int width;
        int height;
        int stride;
        int buf_stride;
        gpu_dri_ver dri_ver;
        uint32_t texture;
        EGLImageKHR image;
        uint32_t fbo;
    }BO;
    std::vector<BO> m_bo_img_tex_buffer;
    BO m_next_bo;
    std::mutex m_lk;
    std::set<uint32_t> m_bo_owned_by_capture_screen;
    std::mutex m_cv_update_lk, m_cv_grab_lk;
    std::condition_variable m_cv_update, m_cv_grab;

    uint32_t m_vbo, m_ebo;
    uint32_t m_img_tex;
    uint32_t m_img_width, m_img_height;

    bool m_img_tex_ready;
    GLenum m_img_tex_gl_format, m_img_tex_data_type;
    shm_handle m_data_info;
    sem_t *m_sem_for_shm, *m_sem_for_grab;

signals:
    void sigShowSharingSelectDialog(int winNum);

public:
    static bool isGrabingSupported();
    int  NextBo(uint32_t at_name, uint32_t name, int width, int height, int stride, int ver);
    void AddBoOwnedByCaptureScreen(uint32_t name);
    int updateSharingScreenDisplay(uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t swap);
    void doGrab();
    void exitGrab();

private:
    bool InitEgl();
    void InitVerticeData();
    void BindVerticeData();
    bool getGLFormatAndType(PIX_FMT fmt, GLenum *format, GLenum *type);
    void InitImageTex(bool fill = false);
    void FreeImageTex();
    void clearImageData();
    bool RenderToFboTex();
    bool UpdateImageTex(bool update = true);
    // for emugl mode
    bool GetShm();
    void updateShm(uint32_t width, uint32_t height, PIX_FMT format, void *data, uint32_t dataSize, bool clearData);
    bool GetSem();
    // for drm mode
    bool LoadExtensionFuncs();
    bool CreateImageTexFromBo(BO bo);
    EGLImageKHR createImageFromDmaBuf(const BO &bo);
    bool GetCurrBoImageTex(BO &bo);
    bool IsBoValid(BO &bo);
    bool IsBoImageTexExist(BO &bo, BO &exist_bo);
    void ClearBoTexBuffer();
    bool updateImageAndShm(bool clearData);
    void grabInEmuglMode();
    void grabInDrmMode();
};

}

