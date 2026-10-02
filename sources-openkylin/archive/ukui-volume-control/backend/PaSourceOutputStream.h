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

#ifndef PASOURCEOUTPUTSTREAM_H
#define PASOURCEOUTPUTSTREAM_H

#include "AudioContext.h"
#include "IStream.h"
#include <unordered_map>

namespace UkuiAudioFramwork
{
class PaSourceOutputStream : public IStream
{
public:
    PaSourceOutputStream(pa_context* ctx, PaDataInfo& info)
        : IStream(ctx, info, StreamType::STREAM_TYPE_SOURCEOUTPUT) {}
    ~PaSourceOutputStream() = default;

public:
    void copy(const pa_source_output_info&);

    std::string getMediaRole() const;

    std::string getAppIconName() const;

    std::string getProcessBinary() const;

    std::string getDriver() const;

    uint32_t getSource() const;

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
    uint32_t index;                      /**< Index of the source output */
    std::string name;                    /**< Name of the source output */
    uint32_t owner_module;               /**< Index of the module this source output belongs to, or PA_INVALID_INDEX when it does not belong to any module. */
    uint32_t client;                     /**< Index of the client this source output belongs to, or PA_INVALID_INDEX when it does not belong to any client. */
    uint32_t source;                     /**< Index of the connected source */
    pa_sample_spec sample_spec;          /**< The sample specification of the source output */
    pa_channel_map channel_map;          /**< Channel map */
    pa_usec_t buffer_usec;               /**< Latency due to buffering in the source output, see pa_timing_info for details. */
    pa_usec_t source_usec;               /**< Latency of the source device, see pa_timing_info for details. */
    std::string resample_method;         /**< The resampling method used by this source output. */
    std::string driver;                  /**< Driver name */
    pa_proplist *proplist;               /**< Property list \since 0.9.11 */
    int corked;                          /**< Stream corked \since 1.0 */
    pa_cvolume volume;                   /**< The volume of this source output \since 1.0 */
    int mute;                            /**< Stream muted \since 1.0 */
    int has_volume;                      /**< Stream has volume. If not set, then the meaning of this struct's volume member is unspecified. \since 1.0 */
    int volume_writable;                 /**< The volume can be set. If not set, the volume can still change even though clients can't control the volume. \since 1.0 */
    pa_format_info *format;              /**< Stream format information. \since 1.0 */
    std::string role;
    std::string m_appIconName;
    std::string m_processBinary;

    std::unordered_map<std::string, std::string> m_adapterStreamNameMap = {
        {"qaxbrowser", "qaxbrowser-safe"}
    };
};
}
#endif // PASOURCEOUTPUTSTREAM_H
