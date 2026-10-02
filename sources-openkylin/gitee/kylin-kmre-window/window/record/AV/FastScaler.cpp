/*
Copyright (c) 2012-2020 Maarten Baert <maarten-baert@hotmail.com>

This file is part of SimpleScreenRecorder.

SimpleScreenRecorder is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SimpleScreenRecorder is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SimpleScreenRecorder.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "FastScaler.h"
#include "AVWrapper.h"
#include "../common/TempBuffer.h"

#include "FastScaler_Convert.h"
#include "FastScaler_Scale.h"

FastScaler::FastScaler() {
	m_warn_swscale = true;
	m_sws_context = NULL;

}

FastScaler::~FastScaler() {
	if(m_sws_context != NULL) {
		sws_freeContext(m_sws_context);
		m_sws_context = NULL;
	}
}

void FastScaler::Scale(unsigned int in_width, unsigned int in_height, AVPixelFormat in_format, int in_colorspace, const uint8_t* const* in_data, const int* in_stride,
					   unsigned int out_width, unsigned int out_height, AVPixelFormat out_format, int out_colorspace, uint8_t* const* out_data, const int* out_stride) {

	// faster BGRA scaling
	if(in_format == AV_PIX_FMT_BGRA && out_format == AV_PIX_FMT_BGRA) {
		Scale_BGRA(in_width, in_height, in_data[0], in_stride[0], out_width, out_height, out_data[0], out_stride[0]);
		return;
	}

	// faster BGRA to YUV444 conversion
	if(in_format == AV_PIX_FMT_BGRA && out_format == AV_PIX_FMT_YUV444P && out_colorspace == SWS_CS_ITU709) {
		if(in_width == out_width && in_height == out_height) {
			Convert_BGRA_YUV444(in_width, in_height, in_data[0], in_stride[0], out_data, out_stride);
		} else {
			TempBuffer<uint8_t> scaled;
			int scaled_stride = grow_align16(out_width * 4);
			scaled.Alloc(scaled_stride * out_height);
			Scale_BGRA(in_width, in_height, in_data[0], in_stride[0], out_width, out_height, scaled.GetData(), scaled_stride);
			Convert_BGRA_YUV444(out_width, out_height, scaled.GetData(), scaled_stride, out_data, out_stride);
		}
		return;
	}

	// faster BGRA to YUV422 conversion
	if(in_format == AV_PIX_FMT_BGRA && out_format == AV_PIX_FMT_YUV422P && out_colorspace == SWS_CS_ITU709) {
		if(in_width == out_width && in_height == out_height) {
			Convert_BGRA_YUV422(in_width, in_height, in_data[0], in_stride[0], out_data, out_stride);
		} else {
			TempBuffer<uint8_t> scaled;
			int scaled_stride = grow_align16(out_width * 4);
			scaled.Alloc(scaled_stride * out_height);
			Scale_BGRA(in_width, in_height, in_data[0], in_stride[0], out_width, out_height, scaled.GetData(), scaled_stride);
			Convert_BGRA_YUV422(out_width, out_height, scaled.GetData(), scaled_stride, out_data, out_stride);
		}
		return;
	}

	// faster BGRA to YUV420 conversion
	if(in_format == AV_PIX_FMT_BGRA && out_format == AV_PIX_FMT_YUV420P && out_colorspace == SWS_CS_ITU709) {
		if(in_width == out_width && in_height == out_height) {
			Convert_BGRA_YUV420(in_width, in_height, in_data[0], in_stride[0], out_data, out_stride);
		} else {
			TempBuffer<uint8_t> scaled;
			int scaled_stride = grow_align16(out_width * 4);
			scaled.Alloc(scaled_stride * out_height);
			Scale_BGRA(in_width, in_height, in_data[0], in_stride[0], out_width, out_height, scaled.GetData(), scaled_stride);
			Convert_BGRA_YUV420(out_width, out_height, scaled.GetData(), scaled_stride, out_data, out_stride);
		}
		return;
	}

	// faster BGRA to NV12 conversion
	if(in_format == AV_PIX_FMT_BGRA && out_format == AV_PIX_FMT_NV12 && out_colorspace == SWS_CS_ITU709) {
		if(in_width == out_width && in_height == out_height) {
			Convert_BGRA_NV12(in_width, in_height, in_data[0], in_stride[0], out_data, out_stride);
		} else {
			TempBuffer<uint8_t> scaled;
			int scaled_stride = grow_align16(out_width * 4);
			scaled.Alloc(scaled_stride * out_height);
			Scale_BGRA(in_width, in_height, in_data[0], in_stride[0], out_width, out_height, scaled.GetData(), scaled_stride);
			Convert_BGRA_NV12(out_width, out_height, scaled.GetData(), scaled_stride, out_data, out_stride);
		}
		return;
	}

	// faster BGRA to BGR conversion
	if(in_format == AV_PIX_FMT_BGRA && out_format == AV_PIX_FMT_BGR24) {
		if(in_width == out_width && in_height == out_height) {
			Convert_BGRA_BGR(in_width, in_height, in_data[0], in_stride[0], out_data[0], out_stride[0]);
		} else {
			TempBuffer<uint8_t> scaled;
			int scaled_stride = grow_align16(out_width * 4);
			scaled.Alloc(scaled_stride * out_height);
			Scale_BGRA(in_width, in_height, in_data[0], in_stride[0], out_width, out_height, scaled.GetData(), scaled_stride);
			Convert_BGRA_BGR(out_width, out_height, scaled.GetData(), scaled_stride, out_data[0], out_stride[0]);
		}
		return;
	}

	if(m_warn_swscale) {
		m_warn_swscale = false;
		syslog(LOG_WARNING, "[FastScaler::Scale] No fast pixel format conversion available (%d,%d -> %d,%d), using swscale instead. "
							"This is not a problem, but performance will be worse.", in_format, in_colorspace, out_format, out_colorspace);
	}

	m_sws_context = sws_getCachedContext(m_sws_context,
										 in_width, in_height, in_format,
										 out_width, out_height, out_format,
										 SWS_BILINEAR, NULL, NULL, NULL);
	if(m_sws_context == NULL) {
		syslog(LOG_ERR, "[FastScaler::Scale] Can't get swscale context! Don't translate 'swscale'");
		throw LibavException();
	}
	sws_setColorspaceDetails(m_sws_context,
							 sws_getCoefficients(in_colorspace), 0,
							 sws_getCoefficients(out_colorspace), 0,
							 0, 1 << 16, 1 << 16);
	sws_scale(m_sws_context, in_data, in_stride, 0, in_height, out_data, out_stride);

}

void FastScaler::Convert_BGRA_YUV444(unsigned int width, unsigned int height, const uint8_t* in_data, int in_stride, uint8_t* const out_data[3], const int out_stride[3]) {

	Convert_BGRA_YUV444_Fallback(width, height, in_data, in_stride, out_data, out_stride);

}

void FastScaler::Convert_BGRA_YUV422(unsigned int width, unsigned int height, const uint8_t* in_data, int in_stride, uint8_t* const out_data[3], const int out_stride[3]) {
	assert(width % 2 == 0);

	Convert_BGRA_YUV422_Fallback(width, height, in_data, in_stride, out_data, out_stride);

}

void FastScaler::Convert_BGRA_YUV420(unsigned int width, unsigned int height, const uint8_t* in_data, int in_stride, uint8_t* const out_data[3], const int out_stride[3]) {
	assert(width % 2 == 0 && height % 2 == 0);

	Convert_BGRA_YUV420_Fallback(width, height, in_data, in_stride, out_data, out_stride);

}

void FastScaler::Convert_BGRA_NV12(unsigned int width, unsigned int height, const uint8_t* in_data, int in_stride, uint8_t* const out_data[2], const int out_stride[2]) {
	assert(width % 2 == 0 && height % 2 == 0);

	Convert_BGRA_NV12_Fallback(width, height, in_data, in_stride, out_data, out_stride);

}

void FastScaler::Convert_BGRA_BGR(unsigned int width, unsigned int height, const uint8_t* in_data, int in_stride, uint8_t* out_data, int out_stride) {

	Convert_BGRA_BGR_Fallback(width, height, in_data, in_stride, out_data, out_stride);

}

void FastScaler::Scale_BGRA(unsigned int in_width, unsigned int in_height, const uint8_t* in_data, int in_stride,
							unsigned int out_width, unsigned int out_height, uint8_t* out_data, int out_stride) {

	Scale_BGRA_Fallback(in_width, in_height, in_data, in_stride, out_width, out_height, out_data, out_stride);

}
