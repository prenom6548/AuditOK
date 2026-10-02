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

#ifndef PASINKSTREAM_H
#define PASINKSTREAM_H

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <pulse/introspect.h>

#include "IStream.h"
#include "AudioContext.h"

namespace UkuiAudioFramwork
{
class PaSinkStream : public IStream
{
public:
    PaSinkStream(pa_context* ctx, PaDataInfo& info)
        : IStream(ctx, info, StreamType::STREAM_TYPE_SINK) {}
    ~PaSinkStream() = default;

public:
    std::shared_ptr<PaSinkPortInfo> getActivePort() const;

    std::string getActivePortName() const;

    std::string getActivePortDescription() const;

    std::string getCardName() const;

    std::string getDeviceClass() const;

    void setMasterDevice(const std::string&);

    std::string getMasterDevice() const;

    uint32_t getNPorts() const;

    uint32_t getMasterDeviceIndex() const;

    void copy(const pa_sink_info&);

    std::vector<std::shared_ptr<PaSinkPortInfo>> getPorts() const;

    bool getCombineStatus() const;

    void setCombineStatus(bool);

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
    static void setCardProfileCb(pa_context*, int, void*);

private:
    std::string cardName;              /**< Name of the card */
    std::string name;                  /**< Name of the sink */
    bool m_bIsCombine = false;         /**< Is Combine */
    uint32_t index = PA_INVALID_INDEX; /**< Index of the sink */
    std::string description;           /**< Description of this sink */
    pa_sample_spec sample_spec;        /**< Sample spec of this sink */
    pa_channel_map channel_map;        /**< Channel map */
    uint32_t owner_module;             /**< Index of the owning module of this sink, or PA_INVALID_INDEX. */
    pa_cvolume volume;                 /**< Volume of the sink */
    int mute;                          /**< Mute switch of the sink */
    uint32_t monitor_source;           /**< Index of the monitor source connected to this sink. */
    std::string monitor_source_name;   /**< The name of the monitor source. */
    pa_usec_t latency;                 /**< Length of queued audio in the output buffer. */
    std::string driver;                /**< Driver name */
    pa_sink_flags_t flags;             /**< Flags */
    pa_proplist *proplist;             /**< Property list \since 0.9.11 */
    pa_usec_t configured_latency;      /**< The latency this device has been configured to. \since 0.9.11 */
    pa_volume_t base_volume;           /**< Some kind of "base" volume that refers to unamplified/unattenuated volume in the context of the output device. \since 0.9.15 */
    pa_sink_state_t state;             /**< State \since 0.9.15 */
    uint32_t n_volume_steps;           /**< Number of volume steps for sinks which do not support arbitrary volumes. \since 0.9.15 */
    uint32_t card;                     /**< Card index, or PA_INVALID_INDEX. \since 0.9.15 */
    uint32_t n_ports;                  /**< Number of entries in port array \since 0.9.16 */
    std::vector<std::shared_ptr<PaSinkPortInfo>> ports;         /**< Array of available ports, or NULL. Array is terminated by an entry set to NULL. The number of entries is stored in n_ports. \since 0.9.16 */
    std::shared_ptr<PaSinkPortInfo> active_port;    /**< Pointer to active port in the array, or NULL. \since 0.9.16 */
    uint8_t n_formats;                 /**< Number of formats supported by the sink. \since 1.0 */
    pa_format_info **formats;          /**< Array of formats supported by the sink. \since 1.0 */
    uint32_t priority;
    mutable std::mutex m_mutex;
    std::string m_deviceClass = "";
    std::string m_masterDevice = "";

    uint32_t m_masterDeviceIndex = PA_INVALID_INDEX;
};
}
#endif // PASINKSTREAM_H
