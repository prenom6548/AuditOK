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

#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#define FFAVC_EXPORT __declspec(dllexport)
#else
#define FFAVC_EXPORT __attribute__((visibility("default")))
#endif

namespace ffavc {
class FFAVC_EXPORT DecoderFactory {
 public:
  /**
   * Returns a handle of the decoder factory which implements the pag::SoftwareDecoderFactory API.
   * The returned handle can be type-casted to a pag::SoftwareDecoderFactory pointer, then passed in
   * the decoderFactory parameter of pag::PAGVideoDecoder::RegisterSoftwareDecoderFactory().
   */
  static void* GetHandle();
};
/**
 * 一帧外部视频数据（解复用结果，保持 MP4 容器原始格式：
 * AVCC/4 字节长度前缀 NALU + avcC extradata，libavcodec 原生支持，无需转 Annex-B）。
 */
struct FFAVC_EXPORT MP4VideoFrame {
  // 编码数据，尾部含 AV_INPUT_BUFFER_PADDING_SIZE 字节填充（供解码器安全访问）。
  std::vector<uint8_t> data;
  // 真实编码数据长度（不含尾部填充），交接给解码器时必须用它作为 packet 大小。
  size_t payloadSize = 0;
  // 归一化呈现时间戳（微秒，按最小 PTS 归零，CFR 下与帧号一一对应）。
  int64_t ptsUs = 0;
  // 是否为关键帧（IDR）。
  bool isKeyframe = false;
};

/**
 * 外部视频流的元信息。
 */
struct FFAVC_EXPORT MP4VideoInfo {
  int width = 0;
  int height = 0;
  float frameRate = 0.0f;
  int frameCount = 0;
  // avcC 配置记录原始字节（即 codecpar->extradata）。
  std::vector<uint8_t> extradata;

  bool valid() const {
    return width > 0 && height > 0 && frameRate > 0.0f && frameCount > 0 && !extradata.empty();
  }
};

/**
 * 基于 FFmpeg(avformat) 的最小 MP4 视频解复用器（KylinOS 定制）。
 * 仅支持单一 H.264 视频流（音频流忽略），要求恒定帧率且时间戳连续，
 * 输出格式与 libpag 的 VideoSequence 期望一致（AVCC 包 + avcC extradata），
 * 供 PAGFile::LoadFromVideo() 组装动态壁纸合成使用。
 */
class FFAVC_EXPORT MP4Demuxer {
 public:
  /**
   * 打开指定视频文件并解析视频流信息。
   * 打开失败、无有效视频流或视频流非 H.264 时返回 nullptr。
   */
  static std::unique_ptr<MP4Demuxer> Open(const std::string& filePath);

  ~MP4Demuxer();
  MP4Demuxer(const MP4Demuxer&) = delete;
  MP4Demuxer& operator=(const MP4Demuxer&) = delete;

  /**
   * 返回视频流信息；Open 失败时为 nullptr。
   */
  const MP4VideoInfo* info() const;

  /**
   * 按解码顺序读取全部视频帧。帧率非恒定或时间戳无法映射为连续帧号时返回 false。
   */
  bool readFrames(std::vector<MP4VideoFrame>* frames);

 private:
  MP4Demuxer();
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace ffavc

// zero-copy contract: 解码线程无法探测 EGL（display 线程本地），集成方须在渲染
// 线程验证 EGL dma-buf import 后调 pag_zc_ready(1)；否则 300 次后永久回退 CPU
extern "C" FFAVC_EXPORT void pag_zc_ready(int ready);
extern "C" FFAVC_EXPORT int pag_zc_ready_get(void);
extern "C" FFAVC_EXPORT void pag_zc_import_failed(void);
extern "C" FFAVC_EXPORT int pag_zc_import_failed_get(void);
extern "C" FFAVC_EXPORT int pag_zc_enabled_get(void);
