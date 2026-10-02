/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once

#include "PipewireMetadata.h"


namespace UkuiAudioFramwork
{
class PipewireMetadata;
class Pipewire;
class PipewireSettings : public PipewireMetadata
{
    void logLevelChanged(int);
    void force_sampleRateChanged(int);
    void force_bufferChanged(int);
    void minBufferChanged(int);
    void maxBufferChanged(int);

public:
    explicit PipewireSettings(Pipewire *parent, uint32_t id, const spa_dict* props);
    virtual ~PipewireSettings() = default;

    int logLevel() { return m_logLevel; }
    int minBuffer() { return m_minBuffer; }
    int maxBuffer() { return m_maxBuffer; }
    int force_sampleRate() { return m_force_sampleRate; }
    int force_buffer() { return m_force_buffer; }

    void setLogLevel(int newLogLevel);
    void setMinBuffer(int newMinBuffer);
    void setMaxBuffer(int newMaxBuffer);
    void setForce_sampleRate(int newSampleRate);
    void setForce_buffer(int newBuffer);

private:
    void keyUpdated(uint32_t id, const char* key, const char* type, const char* value);

private:
    int m_logLevel = static_cast<int>(spa_log_level::SPA_LOG_LEVEL_WARN);
    int m_minBuffer = 0;
    int m_maxBuffer = 0;
    int m_force_sampleRate = 0;
    int m_force_buffer = 0;
};
}
