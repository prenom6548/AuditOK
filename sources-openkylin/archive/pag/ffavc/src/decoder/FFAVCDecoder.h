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

#pragma once

#include "pag/decoder.h"
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/hwcontext.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
}

namespace ffavc {

/**
 * All input data sent to AVCDecoder must be in annex-b format, and all output buffers from
 * AVCDecoder are in I420 (YUV420P) format.
 *
 * VA-API hardware decoding is attempted first (for Intel/AMD/Phytium GPUs on Linux).
 * Falls back to libavcodec software decoding if VA-API is unavailable.
 */
class FFAVCDecoder : public pag::SoftwareDecoder {
 public:
  ~FFAVCDecoder() override;

  bool onConfigure(const std::vector<pag::HeaderData>& headers, std::string mime, int width,
                   int height) override;

  pag::DecoderResult onSendBytes(void* bytes, size_t length, int64_t frame) override;

  pag::DecoderResult onDecodeFrame() override;

  pag::DecoderResult onEndOfStream() override;

  void onFlush() override;

  std::unique_ptr<pag::YUVBuffer> onRenderFrame() override;

 private:
  enum class DecodeMode {
    Auto,
    ForceHardware,
    ForceSoftware,
  };

  const AVCodec* codec = nullptr;
  AVCodecContext* context = nullptr;
  AVFrame* frame = nullptr;    ///< decoded frame (may be HW surface or SW frame)
  AVFrame* swFrame = nullptr;  ///< CPU-side frame used for HW→CPU transfer
  AVPacket* packet = nullptr;
  bool useHWAccel = false;
  std::vector<uint8_t> i420Buffer_;  ///< temporary I420 buffer for NV12→I420 conversion
  SwsContext* swsCtx_ = nullptr;     ///< libswscale context for NV12→I420 conversion
  int swsWidth_ = 0;
  int swsHeight_ = 0;

  // zero-copy (GPU-resident frame export, PAG_VIDEO_ZERO_COPY=1):
  // probe → export → YUVBuffer{zeroCopyDesc}; any failure = session-level fallback
  // to the legacy av_hwframe_transfer_data path.
  bool zcEnabled_ = false;   ///< zero-copy active (probed once, session-level)
  bool zcProbed_ = false;    ///< probe performed
  int zcProbeRetries_ = 0;   ///< retry counter for not-yet-ready probe (EGL not loaded yet)
  int zcFd_ = -1;            ///< exported dma-buf fd (current frame; two-generation deferred close)
  int zcPrevFd_ = -1;        ///< previous frame exported dma-buf fd (closed when a newer third fd arrives)
  AVFrame* zcHeldFrame_ = nullptr;  ///< holds AVBufferRef to keep the VA surface alive
  pag::ZYUVFrameDesc zcDesc_;       ///< exported plane layout (R8 + GR88)

  int probeZeroCopy();        ///< 1=enabled 0=unsupported -1=not ready(retry)
  bool exportZeroCopyFrame();///< export current VA frame to dma-buf; fills zcDesc_

  bool openDecoder(uint8_t* headerData, size_t headerLength);
  bool tryHWDecoder(uint8_t* headerData, size_t headerLength);
  bool trySWDecoder(uint8_t* headerData, size_t headerLength);
  void closeDecoder();
  DecodeMode getDecodeModeFromEnv() const;
};

class FFAVCDecoderFactory : pag::SoftwareDecoderFactory {
 public:
  std::unique_ptr<pag::SoftwareDecoder> createSoftwareDecoder() override {
    return std::unique_ptr<pag::SoftwareDecoder>(new FFAVCDecoder());
  }
};

}  // namespace ffavc
