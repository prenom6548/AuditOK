#include "wifidevice.h"
#include "avframeconvert.h"
#include "decoder.h"
#include "wifiserver.h"
#include "stream.h"
#include "videobuffer.h"
#include "videoform.h"
//#include "recorder.h"


WifiDevice::WifiDevice(DeviceParams params) : BaseDevice(params, nullptr)
{
    if (!params.display && m_params.recordFileName.trimmed().isEmpty()) {
        qCritical("not display must be recorded");
        deleteLater();
        return;
    }

    if (params.display) {
        m_vb = new VideoBuffer();
        m_vb->init(params.renderExpiredFrames);
        m_decoder = new Decoder(m_vb, this);
        m_videoForm = new VideoForm(nullptr, params.screenIndex);
        isUsbDevice = false;
        m_videoForm->setDevice(this);
    }

    m_stream = new Stream(this);
    if (m_decoder) {
        m_stream->setDecoder(m_decoder);
    }

    m_server = new WifiServer(this, params.serial, params.localPort);

    //    if (!m_params.recordFileName.trimmed().isEmpty()) {
    //        m_recorder = new Recorder(m_params.recordFileName);
    //        m_stream->setRecoder(m_recorder);
    //    }
    initSignals();
}

WifiDevice::~WifiDevice() {}
