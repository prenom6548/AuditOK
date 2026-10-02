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

#include "ffavc.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
}

namespace ffavc {

namespace {
// ffmpeg 解码器要求样本数据末尾有安全填充，AVPacket 自带该 padding，拷贝时一并保留。
constexpr size_t kPacketPadding = AV_INPUT_BUFFER_PADDING_SIZE;
// 帧率可信范围，动态壁纸场景下超出该范围的数据视为异常。
constexpr double kMinFrameRate = 1.0;
constexpr double kMaxFrameRate = 240.0;
}  // namespace

struct MP4Demuxer::Impl {
  AVFormatContext* formatContext = nullptr;
  int streamIndex = -1;
  AVRational timeBase = {};
  MP4VideoInfo info;
};

MP4Demuxer::MP4Demuxer() : impl_(new Impl()) {}

MP4Demuxer::~MP4Demuxer() {
  if (impl_ && impl_->formatContext) {
    avformat_close_input(&impl_->formatContext);
  }
}

const MP4VideoInfo* MP4Demuxer::info() const {
  return impl_ ? &impl_->info : nullptr;
}

std::unique_ptr<MP4Demuxer> MP4Demuxer::Open(const std::string& filePath) {
  if (filePath.empty()) {
    return nullptr;
  }
  std::unique_ptr<MP4Demuxer> demuxer(new MP4Demuxer());
  auto& impl = *demuxer->impl_;

  if (avformat_open_input(&impl.formatContext, filePath.c_str(), nullptr, nullptr) < 0) {
    return nullptr;
  }
  if (avformat_find_stream_info(impl.formatContext, nullptr) < 0) {
    return nullptr;
  }

  // 仅关注视频流（音频流直接忽略，动态壁纸静音）。
  const AVCodec* dummyCodec = nullptr;
  impl.streamIndex =
      av_find_best_stream(impl.formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &dummyCodec, 0);
  if (impl.streamIndex < 0) {
    return nullptr;
  }
  const auto* stream = impl.formatContext->streams[impl.streamIndex];
  const auto* codecpar = stream->codecpar;
  if (codecpar == nullptr || codecpar->codec_id != AV_CODEC_ID_H264) {
    // v1 仅支持 H.264，与 libpag 软件解码器（mime video/avc）保持一致。
    return nullptr;
  }
  if (codecpar->width <= 0 || codecpar->height <= 0 || codecpar->extradata == nullptr ||
      codecpar->extradata_size <= 0) {
    return nullptr;
  }

  // 帧率优先取 avg_frame_rate，其次 r_frame_rate，再退化为 time_base 反推。
  AVRational rational = stream->avg_frame_rate;
  if (rational.num <= 0 || rational.den <= 0) {
    rational = stream->r_frame_rate;
  }
  double frameRate = av_q2d(rational);
  if (frameRate <= 0.0 && stream->time_base.den > 0 && stream->time_base.num > 0) {
    frameRate = 1.0 / av_q2d(stream->time_base);
  }
  if (frameRate < kMinFrameRate || frameRate > kMaxFrameRate) {
    return nullptr;
  }

  impl.info.width = codecpar->width;
  impl.info.height = codecpar->height;
  impl.info.frameRate = static_cast<float>(frameRate);
  impl.info.frameCount = 0;  // 样本数需要 readFrames() 读取后才能确定
  impl.timeBase = stream->time_base;
  impl.info.extradata.assign(codecpar->extradata, codecpar->extradata + codecpar->extradata_size);
  return demuxer;
}

bool MP4Demuxer::readFrames(std::vector<MP4VideoFrame>* frames) {
  if (frames == nullptr || impl_ == nullptr || impl_->formatContext == nullptr ||
      impl_->streamIndex < 0 || impl_->info.width <= 0 || impl_->info.height <= 0 ||
      !(impl_->info.frameRate > 0.0f) || impl_->info.extradata.empty()) {
    return false;
  }
  frames->clear();

  // 第一步：按解码顺序收集全部视频样本（含关键帧标记与原始 PTS）。
  struct RawSample {
    std::vector<uint8_t> data;
    int64_t ptsUs = 0;
    bool key = false;
  };
  std::vector<RawSample> samples;
  int64_t minPts = std::numeric_limits<int64_t>::max();
  bool hasTimestamp = false;

  AVPacket packet = {};
  while (av_read_frame(impl_->formatContext, &packet) >= 0) {
    if (packet.stream_index != impl_->streamIndex) {
      av_packet_unref(&packet);
      continue;
    }
    if (packet.pts == AV_NOPTS_VALUE || packet.size < 0) {
      // 缺失 PTS 无法建立帧号映射，直接拒绝（CFR 约束）。
      av_packet_unref(&packet);
      return false;
    }
    RawSample sample;
    sample.data.resize(static_cast<size_t>(packet.size) + kPacketPadding);
    if (packet.size > 0) {
      memcpy(sample.data.data(), packet.data, static_cast<size_t>(packet.size));
    }
    std::memset(sample.data.data() + packet.size, 0, kPacketPadding);
    // 容器 PTS 单位是流 time_base，必须换算为微秒（AV_TIME_BASE_Q）。
    sample.ptsUs = av_rescale_q(packet.pts, impl_->timeBase, AV_TIME_BASE_Q);
    sample.key = (packet.flags & AV_PKT_FLAG_KEY) != 0;
    minPts = std::min(minPts, sample.ptsUs);
    hasTimestamp = true;
    av_packet_unref(&packet);
    samples.push_back(std::move(sample));
  }
  if (!hasTimestamp || samples.empty()) {
    return false;
  }
  // 首帧必须为关键帧：关键帧列表为空会导致 seek 越界，非关键帧开头会花屏。
  if (!samples.front().key) {
    return false;
  }

  // 第二步：PTS 归零并校验帧号连续唯一（VFR/时间戳异常直接拒绝）。
  std::vector<bool> seen(samples.size(), false);
  for (const auto& sample : samples) {
    const auto deltaUs = sample.ptsUs - minPts;
    const auto frameIndex = static_cast<int64_t>(
        std::llround(static_cast<double>(deltaUs) * impl_->info.frameRate / 1000000.0));
    if (frameIndex < 0 || frameIndex >= static_cast<int64_t>(samples.size()) ||
        seen[static_cast<size_t>(frameIndex)]) {
      return false;
    }
    seen[static_cast<size_t>(frameIndex)] = true;
  }
  for (size_t i = 0; i < samples.size(); i++) {
    if (!seen[i]) {
      return false;
    }
  }

  impl_->info.frameCount = static_cast<int>(samples.size());

  // 第三步：按解码顺序输出，ptsUs 归一化到 0 起始。
  frames->reserve(samples.size());
  for (const auto& sample : samples) {
    MP4VideoFrame frame;
    frame.data = sample.data;
    frame.payloadSize = sample.data.size() - kPacketPadding;
    frame.ptsUs = sample.ptsUs - minPts;
    frame.isKeyframe = sample.key;
    frames->emplace_back(std::move(frame));
  }
  return true;
}
}  // namespace ffavc