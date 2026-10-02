///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021 Tencent. All rights reserved.
//
//  This library is free software; you can redistribute it and/or modify it under the terms of the
//  GNU Lesser General Public License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
//  the GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License along with this
//  library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//  Boston, MA  02110-1301  USA
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "FFAVCDecoder.h"
#include "ffavc.h"

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <unistd.h>
#include <pthread.h>

extern "C" {
#include <libavutil/hwcontext_vaapi.h>
}
#include <va/va.h>
#include <va/va_drmcommon.h>

// zero-copy helpers: lazy dlopen of already-loaded runtime libs
// (libva is guaranteed loaded by the VA-API decode stack). No new link-time
// dependencies; absence = capability unavailable.
namespace {

// ---------------------------------------------------------------------------
// zero-copy cross-DSO channel: EGL capability verified on the QSG render thread
// (PAGView) consumed here; a consumer import failure latches back to the CPU
// path. Exported extern "C" (build uses -fvisibility=hidden), resolved via dlsym.
// Not env vars: setenv/getenv race.
// ---------------------------------------------------------------------------
std::atomic<bool> zcReady{false};
std::atomic<bool> zcImportFailed{false};
// 解码侧零拷贝判定（1=启用 0=回退 -1=等待探测），供 pag_zc_enabled_get() 查询
std::atomic<int> zcEnabledState{-1};

}  // namespace

extern "C" {

__attribute__((visibility("default"))) void pag_zc_ready(int v) {
  zcReady.store(v != 0, std::memory_order_relaxed);
}

__attribute__((visibility("default"))) int pag_zc_ready_get(void) {
  return zcReady.load(std::memory_order_relaxed) ? 1 : 0;
}

__attribute__((visibility("default"))) void pag_zc_import_failed(void) {
  zcImportFailed.store(true, std::memory_order_relaxed);
}

__attribute__((visibility("default"))) int pag_zc_import_failed_get(void) {
  return zcImportFailed.load(std::memory_order_relaxed) ? 1 : 0;
}

__attribute__((visibility("default"))) int pag_zc_enabled_get(void) {
  return zcEnabledState.load(std::memory_order_relaxed);
}

}  // extern "C"

namespace {

void* zc_va_handle() {
  static void* handle = dlopen("libva.so.2", RTLD_NOW | RTLD_NOLOAD);
  return handle;
}

// VA surface attribute memory types & export flags (libva 2.x stable values)
constexpr unsigned int kVaMemDrmPrime2 = 0x40000000u;  // VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2 (va_drmcommon.h:88)
constexpr unsigned int kVaExportReadOnly = 1u << 0;          // VA_EXPORT_SURFACE_READ_ONLY
constexpr unsigned int kVaExportSeparateLayers = 1u << 2;    // VA_EXPORT_SURFACE_SEPARATE_LAYERS
constexpr unsigned int kDrmFormatR8 = 0x20203852u;           // DRM_FORMAT_R8
constexpr unsigned int kDrmFormatGr88 = 0x38385247u;         // DRM_FORMAT_GR88
constexpr unsigned int kDrmFormatRg88 = 0x38384752u;         // DRM_FORMAT_RG88

}  // namespace

namespace ffavc {

#define I420_PLANE_COUNT 3

FFAVCDecoder::~FFAVCDecoder() { closeDecoder(); }

bool FFAVCDecoder::onConfigure(const std::vector<pag::HeaderData>& headers, std::string mimeType,
                               int, int) {
  if (mimeType != "video/avc") {
    return false;
  }
  size_t headerLength = 0;
  for (auto& header : headers) {
    headerLength += header.length;
  }
  auto headerData = new uint8_t[headerLength];
  size_t pos = 0;
  for (auto& header : headers) {
    memcpy(headerData + pos, header.data, header.length);
    pos += header.length;
  }
  auto isValid = openDecoder(headerData, headerLength);
  delete[] headerData;
  return isValid;
}

bool FFAVCDecoder::openDecoder(uint8_t* headerData, size_t headerLength) {
  codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec) {
    return false;
  }

  packet = av_packet_alloc();
  frame = av_frame_alloc();
  if (!packet || !frame) {
    return false;
  }

  auto mode = getDecodeModeFromEnv();
  switch (mode) {
    case DecodeMode::ForceSoftware:
      fprintf(stderr, "[ffavc] decode mode from env: FORCE_SOFTWARE\n");
      return trySWDecoder(headerData, headerLength);
    case DecodeMode::ForceHardware:
      fprintf(stderr, "[ffavc] decode mode from env: FORCE_HARDWARE\n");
      if (tryHWDecoder(headerData, headerLength)) {
        return true;
      }
      fprintf(stderr, "[ffavc] FORCE_HARDWARE requested but VA-API init failed\n");
      return false;
    case DecodeMode::Auto:
    default:
      // Try VA-API hardware decode first, fallback to software if failed
      if (!tryHWDecoder(headerData, headerLength)) {
        // Clean up half-initialized state left by tryHWDecoder
        if (context) {
          avcodec_free_context(&context);
          context = nullptr;
        }
        if (swFrame) {
          av_frame_free(&swFrame);
          swFrame = nullptr;
        }
        useHWAccel = false;
        if (!trySWDecoder(headerData, headerLength)) {
          return false;
        }
      }
      return true;
  }
}

bool FFAVCDecoder::tryHWDecoder(uint8_t* headerData, size_t headerLength) {
  // Create VA-API hardware device context (automatically find the system's default DRM render node)
  AVBufferRef* hwDeviceCtx = nullptr;
  if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI,
                              nullptr, nullptr, 0) < 0) {
    fprintf(stderr, "[ffavc] VA-API hwdevice_ctx_create FAILED, falling back to software decode\n");
    return false;
  }

  context = avcodec_alloc_context3(codec);
  if (!context) {
    av_buffer_unref(&hwDeviceCtx);
    return false;
  }

  // Attach VA-API device to decoder context, ownership is transferred to context
  context->hw_device_ctx = hwDeviceCtx;
  // VA-API hardware decode doesn't require software decoding threads on the CPU, force single-thread to avoid extra workers
  context->thread_count = 1;

  AVCodecParameters params = {};
  params.extradata = headerData;
  params.extradata_size = static_cast<int>(headerLength);
  // Set video_delay to ensure the correct decoding of the first B-frame in videos containing B-frames
  params.video_delay = 1;

  if (avcodec_parameters_to_context(context, &params) < 0) {
    avcodec_free_context(&context);
    return false;
  }
  if (avcodec_open2(context, codec, nullptr) < 0) {
    fprintf(stderr, "[ffavc] VA-API avcodec_open2 FAILED, falling back to software decode\n");
    avcodec_free_context(&context);
    return false;
  }

  // Pre-allocate frame for the CPU to receive data, used for HW surface → CPU YUV transfer
  swFrame = av_frame_alloc();
  if (!swFrame) {
    avcodec_free_context(&context);
    return false;
  }

  // fprintf(stderr, "[ffavc] VA-API hardware decode ENABLED (Phytium FTG340)\n");
  useHWAccel = true;
  return true;
}

bool FFAVCDecoder::trySWDecoder(uint8_t* headerData, size_t headerLength) {
  context = avcodec_alloc_context3(codec);
  if (!context) {
    return false;
  }

  // For wallpaper scenario with frame-by-frame stepping (15fps), single-threaded software decoding is enough.
  // Avoid FFmpeg automatically enabling multi-threading (frame/slice threading),
  // which could result in extra worker threads with high CPU usage.
  context->thread_count = 1;

  AVCodecParameters params = {};
  params.extradata = headerData;
  params.extradata_size = static_cast<int>(headerLength);
  params.video_delay = 1;

  if (avcodec_parameters_to_context(context, &params) < 0) {
    avcodec_free_context(&context);
    return false;
  }
  if (avcodec_open2(context, codec, nullptr) < 0) {
    avcodec_free_context(&context);
    return false;
  }

  fprintf(stderr, "[ffavc] software H.264 decode ENABLED (single-thread, 1 worker)\n");
  useHWAccel = false;
  return true;
}

void FFAVCDecoder::closeDecoder() {
  if (context != nullptr) {
    avcodec_free_context(&context);
    context = nullptr;
  }
  if (frame != nullptr) {
    av_frame_free(&frame);
    frame = nullptr;
  }
  if (swFrame != nullptr) {
    av_frame_free(&swFrame);
    swFrame = nullptr;
  }
  if (packet != nullptr) {
    av_packet_free(&packet);
    packet = nullptr;
  }
  if (zcHeldFrame_ != nullptr) {
    av_frame_free(&zcHeldFrame_);
    zcHeldFrame_ = nullptr;
  }
  if (zcFd_ >= 0) {
    ::close(zcFd_);
    zcFd_ = -1;
  }
  if (zcPrevFd_ >= 0) {
    ::close(zcPrevFd_);
    zcPrevFd_ = -1;
  }
  zcEnabled_ = false;
  zcProbed_ = false;
  zcProbeRetries_ = 0;
  zcDesc_ = {};
  if (swsCtx_ != nullptr) {
    sws_freeContext(swsCtx_);
    swsCtx_ = nullptr;
  }
  swsWidth_ = 0;
  swsHeight_ = 0;
  useHWAccel = false;
  i420Buffer_.clear();
}

// ---------------------------------------------------------------------------
// zero-copy: probe + export (GPU-resident frame path)
// ---------------------------------------------------------------------------

// returns: 1 = enabled, 0 = permanently unsupported, -1 = not ready yet (retry)
// PAG_VIDEO_ZERO_COPY: unset = auto (probe verdict via pag_zc_ready),
// "0" = force off (escape hatch), "1" = force on (import failure still latches).
int FFAVCDecoder::probeZeroCopy() {
  const char* env = std::getenv("PAG_VIDEO_ZERO_COPY");
  if (env != nullptr && std::strcmp(env, "0") == 0) {
    return 0;  // force off: legacy path is byte-identical
  }
  // A platform-level import failure stays latched for the whole process.
  if (zcImportFailed.load(std::memory_order_relaxed)) {
    static bool loggedLatch = false;
    if (!loggedLatch) {
      loggedLatch = true;
      fprintf(stderr, "[ffavc] zc: EGL import failure latched, zero-copy disabled\n");
    }
    return 0;
  }
  const bool forced = (env != nullptr && std::strcmp(env, "1") == 0);
  if (!forced) {
    // Auto mode: trust the render-thread verdict (eglGetCurrentDisplay is
    // thread-local, so probing EGL here would always fail on the decode thread).
    if (!zcReady.load(std::memory_order_relaxed)) {
      static bool loggedWait = false;
      if (!loggedWait) {
        loggedWait = true;
        fprintf(stderr, "[ffavc] zc: EGL not verified by render thread yet, will retry\n");
      }
      return -1;  // not ready; retry on a later render frame
    }
    fprintf(stderr, "[ffavc] zero-copy ENABLED (PAGView verified EGL dma-buf import)\n");
  } else {
    fprintf(stderr, "[ffavc] zero-copy ENABLED (forced by PAG_VIDEO_ZERO_COPY=1)\n");
  }
  return 1;
}

bool FFAVCDecoder::exportZeroCopyFrame() {
  if (context == nullptr || frame == nullptr || frame->data[3] == nullptr) {
    return false;
  }
  void* va = zc_va_handle();
  if (va == nullptr) {
    fprintf(stderr, "[ffavc] zc: libva not loaded, disable\n");
    zcEnabled_ = false;
    return false;
  }
  // vaSyncSurface(VADisplay, VASurfaceID)
  typedef unsigned int (VAFUNC_SYNC)(void*, unsigned int);
  // vaExportSurfaceHandle(VADisplay, VASurfaceID, memType, flags, VADRMPRIMESurfaceDescriptor*)
  typedef unsigned int (VAFUNC_EXPORT)(void*, unsigned int, unsigned int, unsigned int,
                                       VADRMPRIMESurfaceDescriptor*);
  auto vaSync = reinterpret_cast<VAFUNC_SYNC*>(dlsym(va, "vaSyncSurface"));
  auto vaExport = reinterpret_cast<VAFUNC_EXPORT*>(dlsym(va, "vaExportSurfaceHandle"));
  auto vaErrorStrFn =
      reinterpret_cast<const char* (*)(int)>(dlsym(va, "vaErrorStr"));
  if (vaSync == nullptr || vaExport == nullptr) {
    fprintf(stderr, "[ffavc] zc: vaExportSurfaceHandle unavailable, disable\n");
    zcEnabled_ = false;
    return false;
  }

  AVHWDeviceContext* hwdevctx =
      reinterpret_cast<AVHWDeviceContext*>(context->hw_device_ctx->data);
  AVVAAPIDeviceContext* vactx =
      reinterpret_cast<AVVAAPIDeviceContext*>(hwdevctx->hwctx);
  void* vdpy = vactx->display;
  VASurfaceID vids = static_cast<VASurfaceID>(reinterpret_cast<uintptr_t>(frame->data[3]));

  VAStatus syncStatus = vaSync(vdpy, vids);
  if (syncStatus != VA_STATUS_SUCCESS) {
    // A sync failure means the surface may be incomplete; exporting it risks
    // displaying a corrupted/stale frame. Fall back (session-level).
    static bool loggedSyncFail = false;
    if (!loggedSyncFail) {
      loggedSyncFail = true;
      fprintf(stderr, "[ffavc] zc: vaSyncSurface returned %d (%s), session fallback\n",
              syncStatus, vaErrorStrFn != nullptr ? vaErrorStrFn(syncStatus) : "?");
    }
    zcEnabled_ = false;
    return false;
  }
  VADRMPRIMESurfaceDescriptor desc;
  std::memset(&desc, 0, sizeof(desc));
  unsigned int status =
      vaExport(vdpy, vids, kVaMemDrmPrime2,
               kVaExportReadOnly | kVaExportSeparateLayers, &desc);
  if (status != VA_STATUS_SUCCESS) {
    static bool loggedExportFail = false;
    if (!loggedExportFail) {
      loggedExportFail = true;
      fprintf(stderr, "[ffavc] zc: vaExportSurfaceHandle failed (%u) %s, session fallback\n",
              status, vaErrorStrFn != nullptr ? vaErrorStrFn(status) : "?");
    }
    zcEnabled_ = false;
    return false;
  }

  // Locate Y (R8) and UV (GR88/RG88) layers (SEPARATE_LAYERS layout).
  int yl = -1, uvl = -1;
  for (unsigned int l = 0; l < desc.num_layers; l++) {
    if (desc.layers[l].drm_format == kDrmFormatR8 && yl < 0) {
      yl = static_cast<int>(l);
    }
    if ((desc.layers[l].drm_format == kDrmFormatGr88 ||
         desc.layers[l].drm_format == kDrmFormatRg88) &&
        uvl < 0) {
      uvl = static_cast<int>(l);
    }
  }
  if (yl < 0 || uvl < 0 || desc.num_objects < 1) {
    fprintf(stderr, "[ffavc] zc: unexpected export layout (layers=%u), session fallback\n",
            desc.num_layers);
    zcEnabled_ = false;
    return false;
  }
  // Guard against malformed/buggy drivers: object index bounds and Y/UV living
  // in the same exported object (multi-object exports would make uvOffset
  // point into a different dma-buf => silent color corruption).
  int obj = static_cast<int>(desc.layers[yl].object_index[0]);
  if (obj >= static_cast<int>(desc.num_objects) ||
      desc.layers[uvl].object_index[0] != desc.layers[yl].object_index[0]) {
    fprintf(stderr, "[ffavc] zc: multi-object or invalid export layout, session fallback\n");
    zcEnabled_ = false;
    return false;
  }
  int fd = desc.objects[obj].fd;

  // Two-generation deferred close: the previous frame's EGLImage is created
  // during its own render pass (synchronous model), but tgfx may defer texture
  // creation; closing one generation later gives a safety margin so a stale
  // ZYUVBuffer never imports an already-closed fd.
  if (zcPrevFd_ >= 0) {
    ::close(zcPrevFd_);
  }
  zcPrevFd_ = zcFd_;
  zcFd_ = fd;

  zcDesc_.fd = fd;
  zcDesc_.modifier = desc.objects[obj].drm_format_modifier;
  zcDesc_.width = frame->width;
  zcDesc_.height = frame->height;
  zcDesc_.yPitch = static_cast<int>(desc.layers[yl].pitch[0]);
  zcDesc_.uvOffset =
      static_cast<int>(desc.layers[uvl].offset[0]);
  zcDesc_.uvPitch = static_cast<int>(desc.layers[uvl].pitch[0]);

  // Keep the VA surface alive until the next zero-copy frame replaces it.
  if (zcHeldFrame_ == nullptr) {
    zcHeldFrame_ = av_frame_alloc();
  }
  av_frame_unref(zcHeldFrame_);
  if (zcHeldFrame_ == nullptr || av_frame_ref(zcHeldFrame_, frame) < 0) {
    fprintf(stderr, "[ffavc] zc: cannot hold frame reference, session fallback\n");
    zcEnabled_ = false;
    return false;
  }
  static bool loggedExport = false;
  if (!loggedExport) {
    loggedExport = true;
    fprintf(stderr,
            "[ffavc] zc: exported fd=%d %dx%d yPitch=%d uvOff=%d uvPitch=%d mod=0x%llx\n",
            fd, zcDesc_.width, zcDesc_.height, zcDesc_.yPitch, zcDesc_.uvOffset,
            zcDesc_.uvPitch, (unsigned long long)zcDesc_.modifier);
  }
  return true;
}

pag::DecoderResult FFAVCDecoder::onSendBytes(void* bytes, size_t length, int64_t) {
  if (context == nullptr) {
    return pag::DecoderResult::Error;
  }
  packet->data = static_cast<uint8_t*>(bytes);
  packet->size = static_cast<int>(length);
  auto result = avcodec_send_packet(context, packet);
  if (result >= 0 || result == AVERROR_EOF) {
    return pag::DecoderResult::Success;
  } else if (result == AVERROR(EAGAIN)) {
    return pag::DecoderResult::TryAgainLater;
  } else {
    return pag::DecoderResult::Error;
  }
}

pag::DecoderResult FFAVCDecoder::onDecodeFrame() {
  auto result = avcodec_receive_frame(context, frame);
  if (result >= 0) {
    // VAAPI hardware frames store the surface handle in data[3]; data[0] is always NULL.
    // Software frames have pixel data starting at data[0].
    bool hasData = (useHWAccel && frame->format == AV_PIX_FMT_VAAPI)
                   ? (frame->data[3] != nullptr)
                   : (frame->data[0] != nullptr);
    if (hasData) {
      return pag::DecoderResult::Success;
    }
  }
  if (result == AVERROR(EAGAIN)) {
    return pag::DecoderResult::TryAgainLater;
  }
  return pag::DecoderResult::Error;
}

pag::DecoderResult FFAVCDecoder::onEndOfStream() { return onSendBytes(nullptr, 0, -1); }

void FFAVCDecoder::onFlush() { avcodec_flush_buffers(context); }

std::unique_ptr<pag::YUVBuffer> FFAVCDecoder::onRenderFrame() {
  AVFrame* renderFrame = frame;

  if (useHWAccel && frame->format == AV_PIX_FMT_VAAPI) {
    // zero-copy: probe once (session-level), then export VA surface to
    // dma-buf when active. Any probe/export failure = permanent session fallback
    // to the legacy transfer path below (never per-frame retry).
    if (!zcProbed_) {
      int probe = probeZeroCopy();
      if (probe > 0) {
        zcProbed_ = true;
        zcEnabled_ = true;
        zcEnabledState.store(1, std::memory_order_relaxed);
      } else if (probe < 0) {
        // Not ready yet (EGL not verified / capability not confirmed yet):
        // retry a few times, then give up permanently to avoid per-frame
        // probing forever.
        if (++zcProbeRetries_ > 300) {
          zcProbed_ = true;
          zcEnabledState.store(0, std::memory_order_relaxed);
          fprintf(stderr,
                  "[ffavc] zc: probe not ready after %d tries, fall back to CPU path\n",
                  zcProbeRetries_);
        }
      } else {
        zcProbed_ = true;
        zcEnabledState.store(0, std::memory_order_relaxed);
      }
    }
    // C1: if the consumer (tgfx MakeZeroCopy) reported an EGL import failure,
    // latch this session back to the CPU path instead of producing frames that
    // would render black forever.
    if (zcEnabled_ && zcImportFailed.load(std::memory_order_relaxed)) {
      fprintf(stderr, "[ffavc] zc: EGL import failure reported by render side, session fallback\n");
      zcEnabled_ = false;
      zcEnabledState.store(0, std::memory_order_relaxed);
    }
    if (zcEnabled_ && exportZeroCopyFrame()) {
      auto zcBuffer = std::make_unique<pag::YUVBuffer>();
      zcBuffer->data[0] = nullptr;
      zcBuffer->data[1] = nullptr;
      zcBuffer->data[2] = nullptr;
      zcBuffer->lineSize[0] = 0;
      zcBuffer->lineSize[1] = 0;
      zcBuffer->lineSize[2] = 0;
      zcBuffer->zeroCopyDesc = &zcDesc_;
      return zcBuffer;
    }

    // Transfer VA-API surface to CPU memory.
    // Do NOT set swFrame->format: forcing AV_PIX_FMT_YUV420P causes
    // "invalid VAImageFormat" (EINVAL) on the Phytium FTG340 driver.
    // Let FFmpeg pick the driver's native format (NV12) automatically.
    av_frame_unref(swFrame);
    if (av_hwframe_transfer_data(swFrame, frame, 0) < 0) {
      return nullptr;
    }
    renderFrame = swFrame;
  }

  auto buffer = std::make_unique<pag::YUVBuffer>();

  if (renderFrame->format == AV_PIX_FMT_YUV420P) {
    // I420（软解输出，或少数 VA-API 驱动直接输出 YUV420P）：三平面路径
    for (int i = 0; i < I420_PLANE_COUNT; i++) {
      buffer->data[i] = renderFrame->data[i];
      buffer->lineSize[i] = renderFrame->linesize[i];
    }
    return buffer;
  }

  if (renderFrame->format == AV_PIX_FMT_NV12) {
    // NV12（Phytium FTG340 等 VA-API 驱动的原生输出格式）：
    // 直接返回 Y + UV 两平面，跳过 sws_scale CPU 颜色空间转换。
    // 上层（SoftwareDecoderWrapper）通过 data[2]==nullptr 哨兵识别 NV12 格式，
    // 并调用 tgfx::ImageBuffer::MakeNV12() 将两平面数据直接上传 GPU。
    buffer->data[0] = renderFrame->data[0];   // Y 平面
    buffer->data[1] = renderFrame->data[1];   // UV 交错平面
    buffer->data[2] = nullptr;                // NV12 哨兵：data[2]==nullptr 表示 NV12
    buffer->lineSize[0] = renderFrame->linesize[0];
    buffer->lineSize[1] = renderFrame->linesize[1];
    buffer->lineSize[2] = 0;
    return buffer;
  }

  // 兜底：其他非预期格式（如 P010 等），用 libswscale 转为 I420。
  // 正常运行时不应进入此分支。
  {
    int w = renderFrame->width;
    int h = renderFrame->height;
    int uvW = (w + 1) / 2;
    int uvH = (h + 1) / 2;

    if (swsCtx_ == nullptr || swsWidth_ != w || swsHeight_ != h) {
      if (swsCtx_ != nullptr) {
        sws_freeContext(swsCtx_);
      }
      swsCtx_ = sws_getContext(w, h, static_cast<AVPixelFormat>(renderFrame->format),
                               w, h, AV_PIX_FMT_YUV420P,
                               SWS_BILINEAR, nullptr, nullptr, nullptr);
      swsWidth_  = w;
      swsHeight_ = h;
    }
    if (swsCtx_ == nullptr) {
      fprintf(stderr, "[ffavc] unsupported pixel format %d, sws_getContext failed\n",
              renderFrame->format);
      return nullptr;
    }

    i420Buffer_.resize(static_cast<size_t>(w * h + uvW * uvH * 2));
    uint8_t* dstY = i420Buffer_.data();
    uint8_t* dstU = dstY + w * h;
    uint8_t* dstV = dstU + uvW * uvH;

    uint8_t* dstData[4]    = { dstY, dstU, dstV, nullptr };
    int      dstLineSize[4] = { w,    uvW,  uvW,  0 };

    sws_scale(swsCtx_,
              (const uint8_t* const*)renderFrame->data, renderFrame->linesize,
              0, h,
              dstData, dstLineSize);

    buffer->data[0] = dstY; buffer->lineSize[0] = w;
    buffer->data[1] = dstU; buffer->lineSize[1] = uvW;
    buffer->data[2] = dstV; buffer->lineSize[2] = uvW;
    return buffer;
  }
}

void* DecoderFactory::GetHandle() {
  static auto factory = *new FFAVCDecoderFactory();
  return &factory;
}

FFAVCDecoder::DecodeMode FFAVCDecoder::getDecodeModeFromEnv() const {
  const char* mode = std::getenv("FFAVC_DECODE_MODE");
  if (mode == nullptr || mode[0] == '\0') {
    return DecodeMode::Auto;
  }
  std::string value(mode);
  for (char& c : value) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  if (value == "sw" || value == "soft" || value == "software") {
    return DecodeMode::ForceSoftware;
  }
  if (value == "hw" || value == "hard" || value == "hardware") {
    return DecodeMode::ForceHardware;
  }
  if (value != "auto") {
    fprintf(stderr, "[ffavc] unknown FFAVC_DECODE_MODE=%s, fallback to auto\n", mode);
  }
  return DecodeMode::Auto;
}

}  // namespace ffavc
