/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: renyangguang <renyangguang@kylinos.cn>
 *
 */

// PAGFile::LoadFromVideo 实现：将外部 mp4 视频解复用为 VideoSequence 并包装成
// PAGFile，供桌面动态壁纸使用。解复用完全由 ffavc（LGPL）完成，本文件只通过
// ffavc.h 公开接口取数据，libpag 内不出现 ffmpeg 符号；输出保持 MP4 原始
// AVCC 格式（avcC extradata + 长度前缀 NAL），共享解码/渲染路径无需改动。

#include "pag/pag.h"

#ifdef PAG_USE_FFAVC
#include <cmath>
#include <string>
#include <vector>

#include "ffavc.h"
#include "pag/file.h"

namespace pag {

std::shared_ptr<PAGFile> PAGFile::LoadFromVideo(const std::string& filePath) {
  if (filePath.empty()) {
    return nullptr;
  }
  try {
    auto demuxer = ffavc::MP4Demuxer::Open(filePath);
    if (demuxer == nullptr || demuxer->info() == nullptr) {
      return nullptr;
    }
    const auto* info = demuxer->info();
    std::vector<ffavc::MP4VideoFrame> frames;
    if (!demuxer->readFrames(&frames)) {
      return nullptr;
    }
    // frameCount 在 readFrames() 读取后才会填充，因此先读帧再校验。
    if (!info->valid()) {
      return nullptr;
    }

    // 组装 VideoSequence：frame 为呈现帧号（由归一化 PTS 与帧率换算），
    // VideoSequenceDemuxer 依赖该一一对应关系。
    auto videoSequence = new VideoSequence();
    videoSequence->width = info->width;
    videoSequence->height = info->height;
    videoSequence->frameRate = info->frameRate;
    if (!info->extradata.empty()) {
      videoSequence->headers.push_back(
          ByteData::MakeCopy(info->extradata.data(), info->extradata.size()).release());
    }
    videoSequence->frames.reserve(frames.size());
    for (const auto& frame : frames) {
      auto videoFrame = new VideoFrame();
      videoFrame->frame =
          static_cast<Frame>(std::lround(frame.ptsUs * info->frameRate / 1000000.0));
      videoFrame->isKeyframe = frame.isKeyframe;
      // 只拷真实编码数据（data 尾部 64 字节填充不属于样本，混入会导致
      // 解码器把填充区解析为 NAL 长度前缀而报错）。
      videoFrame->fileBytes =
          ByteData::MakeCopy(frame.data.data(), frame.payloadSize).release();
      videoSequence->frames.push_back(videoFrame);
    }

    // duration 由实际帧数推导（不能用容器时长元数据，否则与 totalFrames 不一致）。
    auto videoComposition = new VideoComposition();
    videoComposition->width = info->width;
    videoComposition->height = info->height;
    videoComposition->frameRate = info->frameRate;
    videoComposition->duration = static_cast<Frame>(frames.size());
    videoComposition->sequences.push_back(videoSequence);
    videoSequence->composition = videoComposition;

    auto file = Codec::VerifyAndMake({videoComposition}, {});
    if (file == nullptr) {
      return nullptr;
    }
    return PAGFile::MakeFrom(file);
  } catch (...) {
    // ffmpeg 解复用与内存分配（vector/ByteData/new）可能抛出异常；
    // 异常穿透到调用线程（peony 渲染线程）会导致进程终止，这里统一转为加载失败，
    // 调用方按 loadFailed 降级为静态壁纸即可。
    return nullptr;
  }
}

}  // namespace pag
#else   // !PAG_USE_FFAVC
// PAG_USE_FFAVC 未启用时保留公共 API（pag.h 无条件声明），返回 nullptr 避免链接失败
namespace pag {
std::shared_ptr<PAGFile> PAGFile::LoadFromVideo(const std::string& /*filePath*/) {
  return nullptr;
}
}  // namespace pag
#endif  // PAG_USE_FFAVC