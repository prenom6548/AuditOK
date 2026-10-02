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

#include "windowmanager.h"
#include "communication/app_control_manager.h"
#include "grabberhelper.h"
#include "screensharing.h"
#include "utils.h"
#include "sessionsettings.h"
#include "grabber_x11.h"

#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <thread>

#include <unistd.h>
#include <drm_fourcc.h>

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#ifdef GL_OES_EGL_image
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES_func = NULL;
#endif

#ifdef EGL_KHR_image
static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR_func = NULL;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR_func = NULL;
#endif

static const char vShaderCode[] =
    "attribute vec3 position;\n"
    "attribute vec2 inCoord;\n"
    "varying vec2 outCoord;\n"

    "void main(void) {\n"
    "    gl_Position = vec4(position, 1.0);\n"
    "    outCoord = inCoord;\n"
    "}\n";

static const char fShaderCode[] =
    "precision mediump float;\n"
    "varying lowp vec2 outCoord;\n"
    "uniform sampler2D texture;\n"
    "uniform bool switch_rb;\n"

    "void main(void) {\n"
    "    vec4 color = texture2D(texture, outCoord);\n"
    "    if (switch_rb) {\n"
    "       gl_FragColor = vec4(color.bgr, color.a);\n"
    "    } else {\n"
    "       gl_FragColor = color;\n"
    "    }\n"
    "}\n";

namespace kmre {

GrabberHelper::GrabberHelper()
    : m_vbo(0)
    , m_ebo(0)
    , m_img_tex(0)
    , m_img_width(0)
    , m_img_height(0)
    , m_grab_next(0)
    , m_exit_grab(false)
{
    m_grabber = Grabber::GetGrabber();
    assert(m_grabber);
    mDisplayType = SessionSettings::getInstance().getDisplayType();
    if (mDisplayType == DisplayType::TYPE_EMUGL) {
        m_data_info.shm_key = KEY_SHM_DATA_INFO;
        m_data_info.shm_id = -1;
        m_data_info.shm_data = (void *) -1;
        m_sem_for_shm = SEM_FAILED;
        m_sem_for_grab = SEM_FAILED;

        GetShm();
        GetSem();
    }
    else if (mDisplayType == DisplayType::TYPE_DRM) {
        m_img_tex_ready = false;
        memset(&m_next_bo, 0, sizeof(m_next_bo));

        if (!InitEgl()) {
            syslog(LOG_ERR, "[GrabberHelper] Error: Init egl failed!");
        }
        else {
            InitVerticeData();
        }
    }
    else {
        syslog(LOG_ERR, "[GrabberHelper] Unknow display type! Will not grab screen.");
    }

    ScreenSharing *screenSharing = ScreenSharing::getInstance();
    connect(this, SIGNAL(sigShowSharingSelectDialog(int)), screenSharing, 
            SLOT(onShowSharingSelectDialog(int)), Qt::QueuedConnection);
}

GrabberHelper::~GrabberHelper()
{
    if (mDisplayType == DisplayType::TYPE_DRM) {
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);
        if (m_img_tex) glDeleteTextures(1, &m_img_tex);

        ClearBoTexBuffer();

        if (m_gles_context.egl_context)
            eglDestroyContext(m_gles_context.egl_display, m_gles_context.egl_context);
        if (m_gles_context.egl_display)
            eglTerminate(m_gles_context.egl_display);
    }
    else if (mDisplayType == DisplayType::TYPE_EMUGL) {
        if (m_data_info.shm_data != (void *) -1) {
            shmdt(m_data_info.shm_data);
        }
        if (m_data_info.shm_id != -1) {
            shmctl(m_data_info.shm_id, IPC_RMID, NULL);
        }
        if (m_sem_for_shm != SEM_FAILED) {
            sem_close(m_sem_for_shm);
            sem_unlink(SEM_NAME_FOR_SHM);
        }
        if (m_sem_for_grab != SEM_FAILED) {
            sem_close(m_sem_for_grab);
            sem_unlink(SEM_NAME_FOR_GRAB);
        }
    }

    Grabber::DeleteGrabber();
    m_grabber = nullptr;
}

void GrabberHelper::ClearBoTexBuffer()
{
    for (auto bo : m_bo_img_tex_buffer) {
        if (bo.texture) glDeleteTextures(1, &(bo.texture));
        if (eglDestroyImageKHR_func != NULL && bo.image != NULL) {
            eglDestroyImageKHR_func(m_gles_context.egl_display, bo.image);
        }
        if (bo.fbo) glDeleteFramebuffers(1, &(bo.fbo));
    }
    m_bo_img_tex_buffer.clear();
}
// this function is only for emugl mode ! 
bool GrabberHelper::GetShm()
{
    if (mDisplayType != DisplayType::TYPE_EMUGL) {
        return false;
    }

    if ((m_data_info.shm_id == -1) || (m_data_info.shm_data == (void *) -1)) {
        m_data_info.shm_id = shmget(m_data_info.shm_key, sizeof(shm_data_info), IPC_CREAT | 0600);
        if (m_data_info.shm_id == -1) {
            syslog(LOG_ERR, "[GrabberHelper] Error: Can't create share memory (%X)!", m_data_info.shm_key);
            return false;
        }
        m_data_info.shm_data = shmat(m_data_info.shm_id, nullptr, 0);
        if (m_data_info.shm_data == (void *) -1) {
            syslog(LOG_ERR, "[GrabberHelper] Error: Can't attach share memory (%X)!", m_data_info.shm_key);
            return false;
        }

        updateShm(0, 0, PIX_FMT_UNKNOW, nullptr, 0, false);
    }
    return true;
}
// this function is only for emugl mode ! 
bool GrabberHelper::GetSem()
{
    if (mDisplayType != DisplayType::TYPE_EMUGL) {
        return false;
    }
    if (m_sem_for_shm == SEM_FAILED) {
        m_sem_for_shm = sem_open(SEM_NAME_FOR_SHM, O_CREAT|O_RDWR, 0600, 1);
        if (m_sem_for_shm == SEM_FAILED) {
            syslog(LOG_ERR, "[GrabberHelper] Error: Can't get semaphore for shm! ");
            return false;
        }
    }
    if (m_sem_for_grab == SEM_FAILED) {
        m_sem_for_grab = sem_open(SEM_NAME_FOR_GRAB, O_CREAT|O_RDWR, 0600, 0);
        if (m_sem_for_grab == SEM_FAILED) {
            syslog(LOG_ERR, "[GrabberHelper] Error: Can't get semaphore for grab! ");
            return false;
        }
    }
    return true;
}

void GrabberHelper::updateShm(uint32_t width, uint32_t height, PIX_FMT format, 
        void *data, uint32_t dataSize, bool clearData)
{
    if (m_data_info.shm_data != (void *) -1) {
        shm_data_info *data_info = static_cast<shm_data_info *>(m_data_info.shm_data);
        data_info->width = width;
        data_info->height = height;
        data_info->format = format;
        data_info->pixel_data = data;

        if (data && clearData) {
            memset(data, 0, dataSize);
        }
    }
}

void GrabberHelper::clearImageData()
{
    if (mDisplayType == DisplayType::TYPE_EMUGL) {
        sem_wait(m_sem_for_shm);
    }

    uint32_t width, height, data_size;
    PIX_FMT format;
    m_grabber->GetImageInfo(width, height, data_size, format);
    void *data = m_grabber->GetImageData();

    if (data && (data_size > 0)) {
        memset(data, 0, data_size);
    }

    if (mDisplayType == DisplayType::TYPE_EMUGL) {
        sem_post(m_sem_for_shm);
    }
}

bool GrabberHelper::updateImageAndShm(bool clearData)
{
    if (mDisplayType != DisplayType::TYPE_EMUGL) {
        return false;
    }

    sem_wait(m_sem_for_shm);

    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
        X11Grabber* x11_grabber = static_cast<X11Grabber*>(m_grabber);
        if (!x11_grabber->GrabImage()) {
            sem_post(m_sem_for_shm);
            return false;
        }
    } 
    else {
        sem_post(m_sem_for_shm);
        return false;
    }

    uint32_t width, height, data_size;
    PIX_FMT format;
    m_grabber->GetImageInfo(width, height, data_size, format);

    updateShm(width, height, format, m_grabber->GetImageData(), data_size, clearData);
    sem_post(m_sem_for_shm);

    return true;
}

void GrabberHelper::grabInDrmMode()
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return;
    }

    ScreenSharing *screenSharing = ScreenSharing::getInstance();
    GrabCmd pre_grab_state = cmd_stop_grab;

    while(1) {
        std::unique_lock<std::mutex> lk_grab(m_cv_grab_lk);
        m_cv_grab.wait(lk_grab, [&]{ return (m_grab_next > 0); });
        lk_grab.unlock();

        if (m_exit_grab) {
            syslog(LOG_DEBUG, "[GrabberHelper] Exit grab thread.");
            return;
        }
        
        if (m_grab_next == RESET_VALUE_FOR_NEXT_GRAB) {// grab next
            if (pre_grab_state != cmd_start_grab) {
                syslog(LOG_DEBUG, "[GrabberHelper] Send start grab cmd to android.");
                AppControlManager::getInstance()->controlApp(0, " ", cmd_start_grab);

                std::vector<QRect> screenRects = m_grabber->getScreenRects();
                if (screenRects.size() < 2) {
                    screenSharing->setSharingScreenNum(0);
                    screenSharing->setSharingStatus(SharingStatus::eReady);
                }
                else {
                    emit sigShowSharingSelectDialog(screenRects.size());
                }

                pre_grab_state = cmd_start_grab;
            }

            if (screenSharing->isSharingPaused()) {
                m_grabber->UpdateGrabArea();
                if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eUnplugged) {
                    if (UpdateImageTex(false)) {// update black screen data
                        RenderToFboTex();
                    }
                }
                else if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eReady) {
                    if (UpdateImageTex()) {
                        screenSharing->setSharingStatus(SharingStatus::eReady);
                        screenSharing->updateSharingArea(m_grabber->GetGrabArea());
                        RenderToFboTex();
                    }
                }
            }
            else if (screenSharing->isSharingReady()) {
                m_grabber->UpdateGrabArea();
                if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eUnplugged) {
                    screenSharing->setSharingStatus(SharingStatus::ePaused);
                    clearImageData();
                }
                else if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eReady) {
                    if (UpdateImageTex()) {
                        screenSharing->updateSharingArea(m_grabber->GetGrabArea());
                        RenderToFboTex();
                    }
                }
            }

            std::unique_lock<std::mutex> lk_update(m_cv_update_lk);
            m_grab_next--;
            m_cv_update.notify_all();
        }
        else {
            m_grab_next--;
            if (m_grab_next <= 0) {
                if (pre_grab_state != cmd_stop_grab) {
                    syslog(LOG_DEBUG, "[GrabberHelper] Send stop grab cmd to android.");
                    AppControlManager::getInstance()->controlApp(0, " ", cmd_stop_grab);

                    screenSharing->stopSharing();

                    pre_grab_state = cmd_stop_grab;
                }
                ClearBoTexBuffer();
                m_bo_owned_by_capture_screen.clear();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_IDLE));
    }
}

void GrabberHelper::grabInEmuglMode()
{
    if (mDisplayType != DisplayType::TYPE_EMUGL) {
        return;
    }

    if (!GetShm() || !GetSem()) {
        return;
    }

    m_grabber->UpdateGrabArea();
    if (!updateImageAndShm(true)) {
        return;
    }

    ScreenSharing *screenSharing = ScreenSharing::getInstance();
    GrabCmd pre_grab_state = cmd_stop_grab;

    while(1) {
        if (m_grab_next <= 0) {
            sem_wait(m_sem_for_grab);
            m_grab_next = RESET_VALUE_FOR_NEXT_GRAB;
        }

        if (m_exit_grab) {
            syslog(LOG_DEBUG, "[GrabberHelper] Exit grab thread.");
            return;
        }
        
        if (m_grab_next > 0) {
            if (sem_trywait(m_sem_for_grab) == 0) {
                m_grab_next = RESET_VALUE_FOR_NEXT_GRAB;// reset time
            }

            if (m_grab_next == RESET_VALUE_FOR_NEXT_GRAB) {
                if (pre_grab_state != cmd_start_grab) {
                    syslog(LOG_DEBUG, "[GrabberHelper] Send start grab cmd to android.");
                    AppControlManager::getInstance()->controlApp(0, " ", cmd_start_grab);

                    std::vector<QRect> screenRects = m_grabber->getScreenRects();
                    if (screenRects.size() < 2) {
                        screenSharing->setSharingScreenNum(0);
                        screenSharing->setSharingStatus(SharingStatus::eReady);
                    }
                    else {
                        emit sigShowSharingSelectDialog(screenRects.size());
                        clearImageData();
                    }

                    pre_grab_state = cmd_start_grab;
                }
            }

            if (screenSharing->isSharingPaused()) {
                m_grabber->UpdateGrabArea();
                if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eReady) {
                    if (updateImageAndShm(false)) {
                        screenSharing->setSharingStatus(SharingStatus::eReady);
                        screenSharing->updateSharingArea(m_grabber->GetGrabArea());
                    }
                }
            }
            else if (screenSharing->isSharingReady()) {
                m_grabber->UpdateGrabArea();
                if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eUnplugged) {
                    screenSharing->setSharingStatus(SharingStatus::ePaused);
                    clearImageData();
                }
                else if (m_grabber->GetGrabScreenStatus() == GrabScreenStatus::eReady) {
                    if (updateImageAndShm(false)) {
                        screenSharing->updateSharingArea(m_grabber->GetGrabArea());
                    }
                }
            }

            m_grab_next--;

            if (m_grab_next <= 0) {
                if (pre_grab_state != cmd_stop_grab) {
                    syslog(LOG_DEBUG, "[GrabberHelper] Send stop grab cmd to android.");
                    AppControlManager::getInstance()->controlApp(0, " ", cmd_stop_grab);

                    screenSharing->stopSharing();

                    pre_grab_state = cmd_stop_grab;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_IDLE));
        }
    }
}

void GrabberHelper::doGrab()
{
    if (mDisplayType == DisplayType::TYPE_DRM) {
        grabInDrmMode();
    }
    else if (mDisplayType == DisplayType::TYPE_EMUGL) {
        grabInEmuglMode();
    }
}

// this function called in socket communication thread
int GrabberHelper::NextBo(uint32_t at_name, uint32_t name, int width, int height, int stride, int ver)
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return -1;
    }

    if (m_bo_owned_by_capture_screen.count(at_name)) {
        if ((name == 0) && (width == 0) && (height == 0) && (stride == 0) && (ver == 0)) {
            return 1; // this cmd just check whether at_name is owned by capture screen or not
        }

        BO tmp_bo;
        tmp_bo.name = name;
        tmp_bo.width = width;
        tmp_bo.height = height;
        tmp_bo.dri_ver = (gpu_dri_ver)ver;
        tmp_bo.buf_stride = stride;
        if (stride >= width * DEFAULT_BO_FORMAT_BPP) {
            tmp_bo.stride = stride / DEFAULT_BO_FORMAT_BPP;
        } else {
            tmp_bo.stride = width;
        }

        if (IsBoValid(tmp_bo)) {
            {
                std::lock_guard<std::mutex> lk(m_lk);
                m_next_bo = std::move(tmp_bo);
            }
            std::unique_lock<std::mutex> lk_update(m_cv_update_lk);
            std::unique_lock<std::mutex> lk_grab(m_cv_grab_lk);
            m_grab_next = RESET_VALUE_FOR_NEXT_GRAB;
            lk_grab.unlock();
            m_cv_grab.notify_all();

            if (m_cv_update.wait_for(lk_update, std::chrono::microseconds(50000), 
                    [&]{ return m_grab_next != RESET_VALUE_FOR_NEXT_GRAB; })) {
                return 1;
            }
            else {
                syslog(LOG_ERR, "[GrabberHelper] Wait for updating image time out!");
                return 0;
            }
        }
    }

    return -1;
}

int GrabberHelper::updateSharingScreenDisplay(uint32_t bo_name, int32_t width, int32_t height, int32_t stride, int32_t bpp, int32_t swap)
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return -1;
    }

    BO tmp_bo;
    tmp_bo.name = bo_name;
    tmp_bo.width = width;
    tmp_bo.height = height;
    tmp_bo.dri_ver = swap ? driver_radeon : driver_amdgpu;
    tmp_bo.buf_stride = stride;
    if (stride >= width * DEFAULT_BO_FORMAT_BPP) {
        tmp_bo.stride = stride / DEFAULT_BO_FORMAT_BPP;
    } else {
        tmp_bo.stride = width;
    }

    if (IsBoValid(tmp_bo)) {
        {
            std::lock_guard<std::mutex> lk(m_lk);
            m_next_bo = std::move(tmp_bo);
        }
        std::unique_lock<std::mutex> lk_update(m_cv_update_lk);
        std::unique_lock<std::mutex> lk_grab(m_cv_grab_lk);
        m_grab_next = RESET_VALUE_FOR_NEXT_GRAB;
        lk_grab.unlock();
        m_cv_grab.notify_all();

        if (m_cv_update.wait_for(lk_update, std::chrono::microseconds(50000), 
                [&]{ return m_grab_next != RESET_VALUE_FOR_NEXT_GRAB; })) {
            //syslog(LOG_DEBUG, "[GrabberHelper] bo_name = %d, width = %d, height = %d", bo_name, width, height);
            return 1;
        }
        else {
            syslog(LOG_ERR, "[GrabberHelper] Wait for updating image time out!");
            return 0;
        }
    }

    return -1;
}

void GrabberHelper::AddBoOwnedByCaptureScreen(uint32_t name)
{
    m_bo_owned_by_capture_screen.emplace(name);
    //syslog(LOG_DEBUG, "[GrabberHelper] received color buffer name: %d", name);
}

// all below functions are only for drm mode ! 
bool GrabberHelper::InitEgl()
{
    m_gles_context.egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_gles_context.egl_display == EGL_NO_DISPLAY) {
        return GL_FALSE;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    if (!eglInitialize(m_gles_context.egl_display, &majorVersion, &minorVersion)) {
        return GL_FALSE;
    }
    //syslog(LOG_DEBUG, "[GrabberHelper] EGL Version: %d.%d", majorVersion, minorVersion);

    EGLConfig config;
    EGLint numConfigs = 0;
    EGLint attribList[] =
    {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
		
    if (!eglChooseConfig (m_gles_context.egl_display, attribList, &config, 1, &numConfigs)) {
        return GL_FALSE;
    }

    if (numConfigs < 1) {
        return GL_FALSE;
    }

    EGLint contextAttribs[] = 
    { 
        EGL_CONTEXT_CLIENT_VERSION, 2, 
        EGL_NONE 
    };

    m_gles_context.egl_context = eglCreateContext(m_gles_context.egl_display, config, EGL_NO_CONTEXT, contextAttribs );
    if (m_gles_context.egl_context == EGL_NO_CONTEXT) {
        return GL_FALSE;
    }

    if (!eglMakeCurrent(m_gles_context.egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_gles_context.egl_context)) {
        return GL_FALSE;
    }

    return GL_TRUE;
}

void GrabberHelper::InitVerticeData()
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return;
    }

    m_shader = std::make_unique<Shader>(vShaderCode, fShaderCode);

    const float vertices[] = {
        // positions          // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f
    };
    const uint32_t indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    GLuint pos =  m_shader->getAttribLoc("position");
    GLuint coord =  m_shader->getAttribLoc("inCoord");
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(coord);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Reset state.
    //glDisableVertexAttribArray(pos);
    //glDisableVertexAttribArray(coord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GrabberHelper::BindVerticeData()
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return;
    }

    GLuint pos =  m_shader->getAttribLoc("position");
    GLuint coord =  m_shader->getAttribLoc("inCoord");

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(coord);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
}

void GrabberHelper::FreeImageTex()
{
    if (mDisplayType == DisplayType::TYPE_DRM) {
        if (m_img_tex != 0) {
            glDeleteTextures(1, &m_img_tex);
            m_img_tex = 0;
        }
    }
}
void GrabberHelper::InitImageTex(bool fill)
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return;
    }

    if (m_img_tex == 0) {
        glGenTextures(1, &m_img_tex);
    }
    glBindTexture(GL_TEXTURE_2D, m_img_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uint32_t width, height, data_size;
    PIX_FMT format;
    m_grabber->GetImageInfo(width, height, data_size, format);
    if (getGLFormatAndType(format, &m_img_tex_gl_format, &m_img_tex_data_type)) {
        void *pixel = NULL;
        if (fill) {
            pixel = m_grabber->GetImageData();
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                m_img_tex_gl_format, m_img_tex_data_type, pixel);
        //glGenerateMipmap(GL_TEXTURE_2D);
        m_img_tex_ready = true;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool GrabberHelper::UpdateImageTex(bool update)
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return false;
    }
 
    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
        if (update) {
            X11Grabber* x11_grabber = static_cast<X11Grabber*>(m_grabber);
            if (!x11_grabber->GrabImage()) {
                return false;
            }
        }
    } 
    else {
        return false;
    }

    if (!m_img_tex_ready) {
        InitImageTex(true);

        return m_img_tex_ready;
    }
    else {
        if (m_grabber->IsGrabSizeChanged()) {
            FreeImageTex();
            InitImageTex(true);

            return m_img_tex_ready;
        }
        else {
            uint32_t width, height, data_size;
            PIX_FMT format;
            m_grabber->GetImageInfo(width, height, data_size, format);
            if (getGLFormatAndType(format, &m_img_tex_gl_format, &m_img_tex_data_type)) {
                glBindTexture(GL_TEXTURE_2D, m_img_tex);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, 
                    m_img_tex_gl_format, m_img_tex_data_type, m_grabber->GetImageData());
                //glGenerateMipmap(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
                m_img_width = width;
                m_img_height = height;

                return true;
            }
        }
    }

	return false;
}

bool GrabberHelper::CreateImageTexFromBo(BO bo)
{
    if (!LoadExtensionFuncs()) {
        return false;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &(bo.texture));
    if (bo.texture == 0) {
        syslog(LOG_ERR, "[GrabberHelper] Failed to generate fbo texture.");
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, bo.texture);

    EGLint host_attribs[] = {
        EGL_WIDTH, bo.width,
        EGL_HEIGHT, bo.height,
        EGL_DRM_BUFFER_FORMAT_MESA, EGL_DRM_BUFFER_FORMAT_ARGB32_MESA,
        EGL_DRM_BUFFER_STRIDE_MESA, bo.stride,
        EGL_NONE
    };

    bo.image = eglCreateImageKHR_func(m_gles_context.egl_display, m_gles_context.egl_context,
                    EGL_DRM_BUFFER_MESA, (EGLClientBuffer)(uintptr_t)(bo.name), host_attribs);

    if (bo.image == EGL_NO_IMAGE_KHR) {
        syslog(LOG_ERR, "[GrabberHelper] Failed to create EGL image from DRM BO! Try to use DMA buffer.");
        bo.image = createImageFromDmaBuf(bo);
        if (bo.image == EGL_NO_IMAGE_KHR) {
            syslog(LOG_CRIT, "[GrabberHelper] Failed to create EGL image from DMA buffer!");
            glDeleteTextures(1, &(bo.texture));
            bo.texture = 0;
            return false;
        }
    }

    glEGLImageTargetTexture2DOES_func(GL_TEXTURE_2D, bo.image);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // bind bo texture image to fbo
    glGenFramebuffers(1, &(bo.fbo));
	glBindFramebuffer(GL_FRAMEBUFFER, bo.fbo);
	glBindTexture(GL_TEXTURE_2D, bo.texture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bo.texture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        if (eglDestroyImageKHR_func != NULL) {
            eglDestroyImageKHR_func(m_gles_context.egl_display, bo.image);
        }
        glDeleteTextures(1, &(bo.texture));
		glDeleteFramebuffers(1, &(bo.fbo));
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        syslog(LOG_ERR, "[GrabberHelper] Error: Create framebuffer texture failed!");
		return false;
	}

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    syslog(LOG_DEBUG, "[GrabberHelper] CreateImageTexFromBo: %d.", bo.name);

    m_bo_img_tex_buffer.emplace_back(bo);

    return true;
}

extern "C" {
extern int drm_helper_get_dmabuf_from_name(uint32_t name);
}
EGLImageKHR GrabberHelper::createImageFromDmaBuf(const BO &bo)
{
    int buffer_fd = -1;
    EGLImageKHR image = EGL_NO_IMAGE_KHR;

    buffer_fd = drm_helper_get_dmabuf_from_name(bo.name);
    if (buffer_fd < 0) {
        return EGL_NO_IMAGE_KHR;
    }

    EGLint const attribute_list[] = {
        EGL_WIDTH, bo.width,
        EGL_HEIGHT, bo.height,
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_XRGB8888,
        EGL_DMA_BUF_PLANE0_FD_EXT, buffer_fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, bo.buf_stride,
        EGL_NONE
    };

    image = eglCreateImageKHR_func(m_gles_context.egl_display,
                                   EGL_NO_CONTEXT,
                                   EGL_LINUX_DMA_BUF_EXT,
                                   (EGLClientBuffer)NULL, attribute_list);

    if (buffer_fd >= 0) {
        close(buffer_fd);
    }

    return image;
}

bool GrabberHelper::GetCurrBoImageTex(BO &bo)
{
    bool ret = false;
    std::lock_guard<std::mutex> lk(m_lk);

    if (!IsBoImageTexExist(m_next_bo, bo)) {
        if (CreateImageTexFromBo(m_next_bo)) {
            bo = m_bo_img_tex_buffer.back();
            //syslog(LOG_DEBUG, "[GrabberHelper] Create new Bo image %d", bo.name);
            ret = true;
        }
    }
    else {
        //syslog(LOG_DEBUG, "[GrabberHelper] Bo image %d existing ", bo.name);
        ret = true;
    }
    //memset(&m_next_bo, 0, sizeof(m_next_bo));
    return ret;
}

bool GrabberHelper::RenderToFboTex()
{
    if (mDisplayType != DisplayType::TYPE_DRM) {
        return false;
    }
    BO bo_tex;
    if (!GetCurrBoImageTex(bo_tex)) {
        return false;
    }
    //syslog(LOG_DEBUG, "[GrabberHelper] RenderToFboTex, bo_tex = %d", bo_tex.name);

    glBindFramebuffer(GL_FRAMEBUFFER, bo_tex.fbo);
    BindVerticeData();
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // keep screen aspect
    GLint view_x, view_y, view_w, view_h;
    float screenAspect = m_img_width / (float)m_img_height;
    float boAspect = bo_tex.width / (float)bo_tex.height;
    if (screenAspect >= boAspect) {
        view_w = bo_tex.width;
        view_h = view_w / screenAspect;
        view_x = 0;
        view_y = (bo_tex.height - view_h) / 2;
    }
    else {
        view_h = bo_tex.height;
        view_w = view_h * screenAspect;
        view_x = (bo_tex.width - view_w) / 2;
        view_y = 0;
    }
    //syslog(LOG_DEBUG, "[GrabberHelper] bo_name = %d, width = %d, height = %d, view_w = %d, view_h = %d", 
    //    bo_tex.name, bo_tex.width, bo_tex.height, view_w, view_h);
    glViewport(view_x, view_y, view_w, view_h);

    m_shader->use();
    m_shader->setBool("switch_rb", (bo_tex.dri_ver == driver_radeon) ? true : false);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_img_tex);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glFinish();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

bool GrabberHelper::IsBoValid(BO &bo)
{
    return (bo.name && 
            ((bo.width > 0) && (bo.width < MAX_IMAGE_WIDTH)) && 
            ((bo.height > 0) && (bo.height < MAX_IMAGE_HEIGHT)) && 
            ((bo.stride > 0) && (bo.stride < MAX_IMAGE_WIDTH)) && 
            ((bo.dri_ver == driver_radeon) || (bo.dri_ver == driver_amdgpu)));
}

bool GrabberHelper::IsBoImageTexExist(BO &new_bo, BO &exist_bo)
{
    for (auto bo : m_bo_img_tex_buffer) {
        if ((new_bo.name == bo.name) && (new_bo.width == bo.width) &&
            (new_bo.height == bo.height) && (new_bo.stride == bo.stride) && 
            (new_bo.dri_ver == bo.dri_ver)) {
            exist_bo = bo;
            return true;
        }
    }
    return false;
}

bool GrabberHelper::LoadExtensionFuncs()
{
    if (glEGLImageTargetTexture2DOES_func == NULL) {
#ifdef GL_OES_EGL_image
        glEGLImageTargetTexture2DOES_func = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
            eglGetProcAddress("glEGLImageTargetTexture2DOES");
        if (glEGLImageTargetTexture2DOES_func == NULL) {
            syslog(LOG_ERR, "[GrabberHelper] Failed to get glEGLImageTargetTexture2DOES_func.");
        }
#endif
    }

    if (eglCreateImageKHR_func == NULL) {
#ifdef EGL_KHR_image
        eglCreateImageKHR_func = (PFNEGLCREATEIMAGEKHRPROC)
            eglGetProcAddress("eglCreateImageKHR");
        if (eglCreateImageKHR_func == NULL) {
            syslog(LOG_ERR, "[GrabberHelper] Failed to get eglCreateImageKHR_func.");
        }
#endif
    }

    if (eglDestroyImageKHR_func == NULL) {
#ifdef EGL_KHR_image
        eglDestroyImageKHR_func = (PFNEGLDESTROYIMAGEKHRPROC)
            eglGetProcAddress("eglDestroyImageKHR");
        if (eglDestroyImageKHR_func == NULL) {
            syslog(LOG_ERR, "[GrabberHelper] Failed to get eglDestroyImageKHR_func.");
        }
#endif
    }

    if (glEGLImageTargetTexture2DOES_func == NULL ||
            eglCreateImageKHR_func == NULL ||
            eglDestroyImageKHR_func == NULL) {
        syslog(LOG_ERR, "[GrabberHelper] Can't load egl extension functions!");
        return false;
    }

    return true;
}

bool GrabberHelper::getGLFormatAndType(PIX_FMT fmt, GLenum *format, GLenum *type)
{
    switch (fmt){
	    case PIX_FMT_RGB565:
        case PIX_FMT_RGB555:{
            *format = GL_RGB;
            *type = GL_UNSIGNED_SHORT_5_6_5;
            return true;
        }
	    case PIX_FMT_BGR24:
	    case PIX_FMT_RGB24:{
            *format = GL_RGB;
            *type = GL_UNSIGNED_BYTE;
            return true;
        }
	    case PIX_FMT_BGRA:
	    case PIX_FMT_RGBA:
	    case PIX_FMT_ABGR:
	    case PIX_FMT_ARGB:{
            *format = GL_RGBA;
            *type = GL_UNSIGNED_BYTE;
            return true;
        }
	    case PIX_FMT_YUV420P:
        case PIX_FMT_YUV422P:
	    case PIX_FMT_YUV444P:
	    case PIX_FMT_NV12:
	    case PIX_FMT_YUYV422:
        {
            *format = GL_LUMINANCE;
            *type = GL_UNSIGNED_BYTE;
            return true;
        }
	    default:
        break;
    }
    syslog(LOG_ERR, "[GrabberHelper] Error: Unsupported image pixel format!");
    return false;
}

void GrabberHelper::exitGrab()
{
    if (mDisplayType == DisplayType::TYPE_EMUGL) {
        if (m_sem_for_grab) {
            m_exit_grab = true;
            sem_post(m_sem_for_grab);
        }
    }
    else if (mDisplayType == DisplayType::TYPE_DRM) {
        std::unique_lock<std::mutex> lk_grab(m_cv_grab_lk);
        m_grab_next = RESET_VALUE_FOR_NEXT_GRAB;
        m_exit_grab = true;
        m_cv_grab.notify_all();
    }
}

}
