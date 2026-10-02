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

#include "tgfx/core/ImageBuffer.h"
#include <memory>
#include "gpu/resources/YUVTextureView.h"

namespace tgfx {
/**
 * YUVBuffer represents a pixel array described in the YUV format with multiple planes.
 */
class YUVBuffer : public ImageBuffer {
 public:
  YUVBuffer(std::shared_ptr<YUVData> data, YUVFormat format, YUVColorSpace colorSpace)
      : data(std::move(data)), colorSpace(colorSpace), format(format) {
  }

  int width() const override {
    return data->width();
  }

  int height() const override {
    return data->height();
  }

  bool isAlphaOnly() const final {
    return false;
  }

 protected:
  std::shared_ptr<TextureView> onMakeTexture(Context* context, bool) const override {
    if (format == YUVFormat::NV12) {
      return YUVTextureView::MakeNV12(context, data.get(), colorSpace);
    }
    return YUVTextureView::MakeI420(context, data.get(), colorSpace);
  }

 private:
  std::shared_ptr<YUVData> data = nullptr;
  YUVColorSpace colorSpace = YUVColorSpace::BT601_LIMITED;
  YUVFormat format = YUVFormat::Unknown;
};

std::shared_ptr<ImageBuffer> ImageBuffer::MakeI420(std::shared_ptr<YUVData> yuvData,
                                                   YUVColorSpace colorSpace) {
  if (yuvData == nullptr || yuvData->planeCount() != YUVData::I420_PLANE_COUNT) {
    return nullptr;
  }
  return std::make_shared<YUVBuffer>(std::move(yuvData), YUVFormat::I420, colorSpace);
}

std::shared_ptr<ImageBuffer> ImageBuffer::MakeNV12(std::shared_ptr<YUVData> yuvData,
                                                   YUVColorSpace colorSpace) {
  if (yuvData == nullptr || yuvData->planeCount() != YUVData::NV12_PLANE_COUNT) {
    return nullptr;
  }
  return std::make_shared<YUVBuffer>(std::move(yuvData), YUVFormat::NV12, colorSpace);
}

/**
 * zero-copy: ImageBuffer whose content is a GPU-resident NV12 frame (dma-buf fd).
 * Texture creation imports the planes via EGL_EXT_image_dma_buf_import (requires a
 * current EGL display at onMakeTexture time; render thread, inside the pag render lock).
 */
class ZYUVBuffer : public ImageBuffer {
 public:
  ZYUVBuffer(ZBufferInfo bufferInfo, YUVColorSpace colorSpace)
      : bufferInfo(bufferInfo), colorSpace(colorSpace) {
  }

  int width() const override {
    return bufferInfo.width;
  }

  int height() const override {
    return bufferInfo.height;
  }

  bool isAlphaOnly() const final {
    return false;
  }

 protected:
  std::shared_ptr<TextureView> onMakeTexture(Context* context, bool) const override {
    return TextureView::MakeZeroCopy(context, bufferInfo, colorSpace);
  }

 private:
  ZBufferInfo bufferInfo;
  YUVColorSpace colorSpace = YUVColorSpace::BT601_LIMITED;
};

std::shared_ptr<ImageBuffer> ImageBuffer::MakeZeroCopy(ZBufferInfo bufferInfo,
                                                       YUVColorSpace colorSpace) {
  if (bufferInfo.fd < 0 || bufferInfo.width <= 0 || bufferInfo.height <= 0 ||
      bufferInfo.yPitch <= 0 || bufferInfo.uvPitch <= 0) {
    return nullptr;
  }
  return std::make_shared<ZYUVBuffer>(bufferInfo, colorSpace);
}

}  // namespace tgfx
