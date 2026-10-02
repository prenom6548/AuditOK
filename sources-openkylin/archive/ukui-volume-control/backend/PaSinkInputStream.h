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

#ifndef PASINKINPUTSTREAM_H
#define PASINKINPUTSTREAM_H

#include "IStream.h"
#include "AudioContext.h"

namespace UkuiAudioFramwork
{
class PaSinkInputStream : public IStream
{
public:
    PaSinkInputStream(pa_context* ctx, PaDataInfo& info)
        : IStream(ctx, info, StreamType::STREAM_TYPE_SINKINPUT) {}
    ~PaSinkInputStream() = default;

public:
    void copy(const pa_sink_input_info&);

    std::string getMediaRole() const;

    std::string getAppIconName() const;

    std::string getProcessBinary() const;

    std::string getDriver() const;

    uint32_t getSink() const;

    virtual std::string toString() const override;

    virtual NodeType getNodeType() const override;

    virtual uint32_t getIndex() const override;

    virtual std::string getName() const override;

    virtual pa_proplist* getProplist() const override;

    virtual uint32_t getCard() const override;

    virtual void setVolume(int) override;

    virtual const pa_cvolume getVolume() const override;

    virtual void setBalance(double) override;

    virtual double getBalance() const override;

    virtual void setMute(bool) override;

    virtual bool getMute() const override;

    virtual DefaultDeviceInfo getDefaultDevice() const override;

    virtual void setDefaultDevice(const std::string&, const std::string&) override;

    virtual std::vector<std::string> getDeviceList() const override;

    virtual std::list<std::shared_ptr<DeviceInfo>> getAvailablePortList() const override;

    virtual void volumeChanged(int) override;

    virtual void muteChanged(bool) override;

    virtual void portChanged(const std::string&) override;

    virtual void deviceChanged(bool = false) override;

private:
    uint32_t index {PA_INVALID_INDEX};         /**< Index of the sink input */
    std::string name {""};                     /**< Name of the sink input */
    uint32_t owner_module {PA_INVALID_INDEX};  /**< Index of the module this sink input belongs to, or PA_INVALID_INDEX when it does not belong to any module. */
    uint32_t client {PA_INVALID_INDEX};        /**< Index of the client this sink input belongs to, or PA_INVALID_INDEX when it does not belong to any client. */
    uint32_t sink {PA_INVALID_INDEX};          /**< Index of the connected sink */
    pa_sample_spec sample_spec;                /**< The sample specification of the sink input. */
    pa_channel_map channel_map;                /**< Channel map */
    pa_cvolume volume;                         /**< The volume of this sink input. */
    pa_usec_t buffer_usec;                     /**< Latency due to buffering in sink input, see pa_timing_info for details. */
    pa_usec_t sink_usec;                       /**< Latency of the sink device, see pa_timing_info for details. */
    std::string resample_method {""};          /**< The resampling method used by this sink input. */
    std::string driver {""};                   /**< Driver name */
    int mute {0};                              /**< Stream muted \since 0.9.7 */
    pa_proplist* proplist;                     /**< Property list \since 0.9.11 */
    int corked {0};                            /**< Stream corked \since 1.0 */
    int has_volume {0};                        /**< Stream has volume. If not set, then the meaning of this struct's volume member is unspecified. \since 1.0 */
    int volume_writable {0};                   /**< The volume can be set. If not set, the volume can still change even though clients can't control the volume. \since 1.0 */
    pa_format_info *format {nullptr};          /**< Stream format information. \since 1.0 */
    std::string role {""};
    std::string m_appIconName {""};            /**< stream icon name */
    std::string m_processBinary {""};          /**< stream process binary */


    std::unordered_map<std::string, std::string> m_adapterStreamNameMap = {
        {"qaxbrowser", "qaxbrowser-safe"}
    };
};
}
#endif // PASINKINPUTSTREAM_H
