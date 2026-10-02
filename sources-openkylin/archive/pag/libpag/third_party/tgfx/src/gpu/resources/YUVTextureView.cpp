/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making tgfx available.
//
//  Copyright (C) 2023 Tencent. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "YUVTextureView.h"
#include "core/utils/Log.h"
#include "gpu/GPU.h"
#include "gpu/opengl/GLTexture.h"
#include "tgfx/gpu/opengl/GLFunctions.h"
#include "tgfx/core/YUVData.h"
#include <cstdlib>
#include <dlfcn.h>
#include <vector>

namespace tgfx {
static constexpr int YUV_SIZE_FACTORS[] = {0, 1, 1};

static std::vector<std::unique_ptr<GPUTexture>> MakeTexturePlanes(GPU* gpu, const YUVData* yuvData,
                                                                  const PixelFormat* formats) {
  std::vector<std::unique_ptr<GPUTexture>> texturePlanes = {};
  auto count = static_cast<int>(yuvData->planeCount());
  for (int index = 0; index < count; index++) {
    auto w = yuvData->width() >> YUV_SIZE_FACTORS[index];
    auto h = yuvData->height() >> YUV_SIZE_FACTORS[index];
    GPUTextureDescriptor descriptor = {w, h, formats[index]};
    auto texture = gpu->createTexture(descriptor);
    if (texture == nullptr) {
      for (auto& plane : texturePlanes) {
        plane->release(gpu);
      }
      return {};
    }
    texturePlanes.push_back(std::move(texture));
  }
  return texturePlanes;
}

static void SubmitYUVTexture(GPU* gpu, const YUVData* yuvData,
                             std::unique_ptr<GPUTexture> textures[]) {
  auto count = yuvData->planeCount();
  for (size_t index = 0; index < count; index++) {
    auto& texture = textures[index];
    auto w = yuvData->width() >> YUV_SIZE_FACTORS[index];
    auto h = yuvData->height() >> YUV_SIZE_FACTORS[index];
    auto pixels = yuvData->getBaseAddressAt(index);
    auto rowBytes = yuvData->getRowBytesAt(index);
    gpu->queue()->writeTexture(texture.get(), Rect::MakeWH(w, h), pixels, rowBytes);
    // YUV textures do not support mipmaps, so we don't need to regenerate mipmaps.
  }
}

std::shared_ptr<TextureView> TextureView::MakeI420(Context* context, const YUVData* yuvData,
                                                   YUVColorSpace colorSpace) {
  if (context == nullptr || yuvData == nullptr ||
      yuvData->planeCount() != YUVData::I420_PLANE_COUNT) {
    return nullptr;
  }
  PixelFormat yuvFormats[YUVData::I420_PLANE_COUNT] = {PixelFormat::GRAY_8, PixelFormat::GRAY_8,
                                                       PixelFormat::GRAY_8};
  auto texturePlanes = MakeTexturePlanes(context->gpu(), yuvData, yuvFormats);
  if (texturePlanes.empty()) {
    return nullptr;
  }
  auto yuvTexture = new YUVTextureView(std::move(texturePlanes), YUVFormat::I420, colorSpace);
  auto texture =
      std::static_pointer_cast<YUVTextureView>(Resource::AddToCache(context, yuvTexture));
  SubmitYUVTexture(context->gpu(), yuvData, texture->textures.data());
  return texture;
}

std::shared_ptr<TextureView> TextureView::MakeNV12(Context* context, const YUVData* yuvData,
                                                   YUVColorSpace colorSpace) {
  if (context == nullptr || yuvData == nullptr ||
      yuvData->planeCount() != YUVData::NV12_PLANE_COUNT) {
    return nullptr;
  }
  PixelFormat yuvFormats[YUVData::NV12_PLANE_COUNT] = {PixelFormat::GRAY_8, PixelFormat::RG_88};
  auto texturePlanes = MakeTexturePlanes(context->gpu(), yuvData, yuvFormats);
  if (texturePlanes.empty()) {
    return nullptr;
  }
  auto yuvTexture = new YUVTextureView(std::move(texturePlanes), YUVFormat::NV12, colorSpace);
  auto texture =
      std::static_pointer_cast<YUVTextureView>(Resource::AddToCache(context, yuvTexture));
  SubmitYUVTexture(context->gpu(), yuvData, texture->textures.data());
  return texture;
}

YUVTextureView::YUVTextureView(std::vector<std::unique_ptr<GPUTexture>> yuvTextures,
                               YUVFormat yuvFormat, YUVColorSpace colorSpace)
    : _yuvFormat(yuvFormat), _colorSpace(colorSpace) {
  DEBUG_ASSERT(_yuvFormat != YUVFormat::Unknown);
  DEBUG_ASSERT(yuvTextures.size() == textureCount());
  for (size_t i = 0; i < yuvTextures.size(); ++i) {
    textures[i] = std::move(yuvTextures[i]);
  }
}

size_t YUVTextureView::textureCount() const {
  switch (_yuvFormat) {
    case YUVFormat::I420:
      return YUVData::I420_PLANE_COUNT;
    case YUVFormat::NV12:
      return YUVData::NV12_PLANE_COUNT;
    default:
      DEBUG_ASSERT(false);
      return 0;
  }
}

GPUTexture* YUVTextureView::getTextureAt(size_t index) const {
  DEBUG_ASSERT(index < textureCount());
  return textures[index].get();
}

size_t YUVTextureView::memoryUsage() const {
  if (auto hardwareBuffer = textures.front()->getHardwareBuffer()) {
    return HardwareBufferGetInfo(hardwareBuffer).byteSize();
  }
  return static_cast<size_t>(width()) * static_cast<size_t>(height()) * 3 / 2;
}

Point YUVTextureView::getTextureCoord(float x, float y) const {
  return {x / static_cast<float>(width()), y / static_cast<float>(height())};
}

void YUVTextureView::onReleaseGPU() {
  auto count = textureCount();
  for (size_t i = 0; i < count; ++i) {
    textures[i]->release(context->gpu());
  }
}

// ---------------------------------------------------------------------------
// zero-copy: NV12 plane textures filled from an EGL dma-buf import
// (VA-API frame exported by ffavc; requires a current EGL display, render thread,
// inside the pag render lock — see ffavc FFAVCDecoder zero-copy path).
// ---------------------------------------------------------------------------
namespace {
// PAGView (peony) writes PAG_VIDEO_ZERO_COPY_EGL with the EGL library that has
// the current display, when QSG's context bypasses the glvnd loader. Prefer it,
// fall back to the glvnd loader.
void* zcEglHandle() {
  static void* handle = []() {
    const char* lib = std::getenv("PAG_VIDEO_ZERO_COPY_EGL");
    if (lib != nullptr && lib[0] != '\0') {
      void* h = dlopen(lib, RTLD_NOW | RTLD_NOLOAD);
      if (h != nullptr) {
        return h;
      }
    }
    return dlopen("libEGL.so.1", RTLD_NOW | RTLD_NOLOAD);
  }();
  return handle;
}

// Reports an EGL import failure to the ffavc decoder (resolved from libffavc;
// resolved lazily and once). This is the C1 feedback channel: without it the
// decoder would keep exporting frames that render black forever.
void zcReportImportFailure() {
  static void (*report)() = []() {
    void* h = dlopen("libffavc.so", RTLD_NOW | RTLD_NOLOAD);
    if (h == nullptr) {
      return static_cast<void (*)()>(nullptr);
    }
    return reinterpret_cast<void (*)()>(dlsym(h, "pag_zc_import_failed"));
  }();
  if (report != nullptr) {
    report();
  }
}

constexpr unsigned int kEglWidth = 0x3057;
constexpr unsigned int kEglHeight = 0x3056;
constexpr unsigned int kEglExtensions = 0x3055;
constexpr unsigned int kEglLinuxDmaBufExt = 0x3270;
constexpr unsigned int kEglLinuxDrmFourccExt = 0x3271;
constexpr unsigned int kEglDmaBufPlane0FdExt = 0x3272;
constexpr unsigned int kEglDmaBufPlane0OffsetExt = 0x3273;
constexpr unsigned int kEglDmaBufPlane0PitchExt = 0x3274;
constexpr unsigned int kEglDmaBufPlane0ModifierLoExt = 0x3443;
constexpr unsigned int kEglDmaBufPlane0ModifierHiExt = 0x3444;
constexpr unsigned int kEglNone = 0x3038;
constexpr unsigned int kDrmFormatR8 = 0x20203852u;
constexpr unsigned int kDrmFormatGr88 = 0x38385247u;
constexpr unsigned int kGlTexture2D = 0x0DE1;
constexpr uint64_t kDrmFormatModInvalid = 0x00FFFFFFFFFFFFFFULL;  // DRM_FORMAT_MOD_INVALID

bool ImportZeroCopyPlane(const GLFunctions* gl, const ZBufferInfo& info, bool isUv,
                         GLTexture* texture) {
  void* egl = zcEglHandle();
  if (egl == nullptr) {
    LOGE("ZImageBuffer: libEGL not loaded, zero-copy disabled");
    zcReportImportFailure();
    return false;
  }
  using EglGetCurrentDisplay = void* (*)();
  using EglCreateImage = void* (*)(void*, void*, unsigned int, void*, const int*);
  using EglDestroyImage = int (*)(void*, void*);
  using EglGetError = unsigned int (*)();
  using EglQueryString = const char* (*)(void*, int);
  auto eglGetCurrentDisplay = reinterpret_cast<EglGetCurrentDisplay>(dlsym(egl, "eglGetCurrentDisplay"));
  auto eglGetProcAddress = reinterpret_cast<void* (*)(const char*)>(dlsym(egl, "eglGetProcAddress"));
  auto eglGetErrorFn = reinterpret_cast<EglGetError>(dlsym(egl, "eglGetError"));
  auto eglQueryString = reinterpret_cast<EglQueryString>(dlsym(egl, "eglQueryString"));
  if (eglGetCurrentDisplay == nullptr || eglGetProcAddress == nullptr ||
      eglGetErrorFn == nullptr) {
    LOGE("ZImageBuffer: EGL base entry points unavailable, zero-copy disabled");
    zcReportImportFailure();
    return false;
  }
  // glvnd loader does not export extension entry points; resolve via eglGetProcAddress.
  auto eglCreateImageKHR = reinterpret_cast<EglCreateImage>(eglGetProcAddress("eglCreateImageKHR"));
  auto eglDestroyImageKHR = reinterpret_cast<EglDestroyImage>(eglGetProcAddress("eglDestroyImageKHR"));
  auto eglImageTarget = eglGetProcAddress("glEGLImageTargetTexture2DOES");
  if (eglCreateImageKHR == nullptr || eglDestroyImageKHR == nullptr || eglImageTarget == nullptr) {
    LOGE("ZImageBuffer: EGL extension entry points unavailable, zero-copy disabled");
    zcReportImportFailure();
    return false;
  }
  void* dpy = eglGetCurrentDisplay();
  if (dpy == nullptr) {
    LOGE("ZImageBuffer: no current EGL display (GLX session?), zero-copy disabled");
    zcReportImportFailure();
    return false;
  }
  // Modifier attributes are only legal when EGL_EXT_image_dma_buf_import_modifiers
  // is present; passing them unconditionally rejects drivers without it.
  static bool modifiersChecked = false;
  static bool modifiersOk = false;
  if (!modifiersChecked) {
    modifiersChecked = true;
    const char* exts = eglQueryString != nullptr ? eglQueryString(dpy, kEglExtensions) : nullptr;
    modifiersOk = exts != nullptr &&
                  std::strstr(exts, "EGL_EXT_image_dma_buf_import_modifiers") != nullptr;
  }
  const int w = isUv ? (info.width / 2) : info.width;
  const int h = isUv ? (info.height / 2) : info.height;
  const unsigned int fourcc = isUv ? kDrmFormatGr88 : kDrmFormatR8;
  const int offset = isUv ? info.uvOffset : 0;
  const int pitch = isUv ? info.uvPitch : info.yPitch;

  int attrs[20];
  int n = 0;
  attrs[n++] = static_cast<int>(kEglWidth);
  attrs[n++] = w;
  attrs[n++] = static_cast<int>(kEglHeight);
  attrs[n++] = h;
  attrs[n++] = static_cast<int>(kEglLinuxDrmFourccExt);
  attrs[n++] = static_cast<int>(fourcc);
  attrs[n++] = static_cast<int>(kEglDmaBufPlane0FdExt);
  attrs[n++] = info.fd;
  attrs[n++] = static_cast<int>(kEglDmaBufPlane0OffsetExt);
  attrs[n++] = offset;
  attrs[n++] = static_cast<int>(kEglDmaBufPlane0PitchExt);
  attrs[n++] = pitch;
  if (modifiersOk && info.modifier != kDrmFormatModInvalid) {
    // 64-bit modifier 须完整传递（VA-API 导出 uint64_t，截断会使导入失败）
    attrs[n++] = static_cast<int>(kEglDmaBufPlane0ModifierLoExt);
    attrs[n++] = static_cast<int>(info.modifier & 0xFFFFFFFFu);
    attrs[n++] = static_cast<int>(kEglDmaBufPlane0ModifierHiExt);
    attrs[n++] = static_cast<int>(info.modifier >> 32);
  }
  attrs[n++] = static_cast<int>(kEglNone);

  void* image = eglCreateImageKHR(dpy, nullptr, kEglLinuxDmaBufExt, nullptr, attrs);
  if (image == nullptr) {
    LOGE("ZImageBuffer: eglCreateImageKHR failed, error=0x%x", eglGetErrorFn());
    zcReportImportFailure();
    return false;
  }
  // Clear any stale error, then verify the bind actually took effect on
  // drivers where glEGLImageTargetTexture2DOES can silently fail.
  while (gl->getError() != 0) {
  }
  // The texture keeps the pixel data after the EGLImage is destroyed.
  gl->bindTexture(kGlTexture2D, texture->textureID());
  reinterpret_cast<void (*)(unsigned int, void*)>(eglImageTarget)(kGlTexture2D, image);
  auto bindError = gl->getError();
  gl->bindTexture(kGlTexture2D, 0);
  eglDestroyImageKHR(dpy, image);
  if (bindError != 0) {
    LOGE("ZImageBuffer: glEGLImageTargetTexture2DOES failed, error=0x%x", bindError);
    zcReportImportFailure();
    return false;
  }
  return true;
}
}  // namespace

std::shared_ptr<TextureView> TextureView::MakeZeroCopy(Context* context,
                                                       const ZBufferInfo& info,
                                                       YUVColorSpace colorSpace) {
  if (context == nullptr || info.fd < 0 || info.width <= 0 || info.height <= 0 ||
      info.yPitch <= 0 || info.uvPitch <= 0 || info.uvOffset < 0) {
    return nullptr;
  }
  const GLFunctions* gl = GLFunctions::Get(context);
  if (gl == nullptr) {
    return nullptr;
  }
  PixelFormat yuvFormats[YUVData::NV12_PLANE_COUNT] = {PixelFormat::GRAY_8, PixelFormat::RG_88};
  auto yTexture = context->gpu()->createTexture({info.width, info.height, yuvFormats[0]});
  auto uvTexture =
      context->gpu()->createTexture({info.width / 2, info.height / 2, yuvFormats[1]});
  if (yTexture == nullptr || uvTexture == nullptr) {
    if (yTexture != nullptr) yTexture->release(context->gpu());
    if (uvTexture != nullptr) uvTexture->release(context->gpu());
    return nullptr;
  }
  auto glY = dynamic_cast<GLTexture*>(yTexture.get());
  auto glUV = dynamic_cast<GLTexture*>(uvTexture.get());
  if (glY == nullptr || glUV == nullptr || !ImportZeroCopyPlane(gl, info, false, glY) ||
      !ImportZeroCopyPlane(gl, info, true, glUV)) {
    LOGE("ZImageBuffer: zero-copy import failed, session fallback to CPU path");
    yTexture->release(context->gpu());
    uvTexture->release(context->gpu());
    return nullptr;
  }
  std::vector<std::unique_ptr<GPUTexture>> planes;
  planes.push_back(std::move(yTexture));
  planes.push_back(std::move(uvTexture));
  auto yuvTexture = new YUVTextureView(std::move(planes), YUVFormat::NV12, colorSpace);
  return std::static_pointer_cast<YUVTextureView>(Resource::AddToCache(context, yuvTexture));
}

}  // namespace tgfx
