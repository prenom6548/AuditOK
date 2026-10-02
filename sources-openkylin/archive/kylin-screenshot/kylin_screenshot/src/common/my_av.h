/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This file is part of Kylin-Screenshot.
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
*/
#pragma once

extern "C" {
//#include <libavutil/rational.h>
#include <libavformat/avformat.h>
//#include <libavutil/frame.h>
//#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
//#include <libavutil/pixfmt.h>
//#include <libavutil/samplefmt.h>
}
#include <exception>

// convert weird types from libav/ffmpeg to doubles
inline double ToDouble(const AVRational &r)
{
    return (double)r.num / (double)r.den;
}

class LibavException : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "LibavException";
    }
};
class ResamplerException : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "ResamplerException";
    }
};

#if SSR_USE_PULSEAUDIO
class PulseAudioException : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "PulseAudioException";
    }
};
#endif
