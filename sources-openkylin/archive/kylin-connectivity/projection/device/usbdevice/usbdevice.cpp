#include "usbdevice.h"
#include "usbserver.h"
#include "controller.h"
#include "decoder.h"
//#include "filehandler.h"
#include "mousetap/mousetap.h"
//#include "recorder.h"
#include "stream.h"
#include "videobuffer.h"
#include "videoform.h"


UsbDevice::UsbDevice(DeviceParams params) : BaseDevice(params, nullptr)
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
        //        m_fileHandler = new FileHandler(this);
        m_controller = new Controller(params.gameScript, this);
        m_videoForm = new VideoForm(nullptr, params.screenIndex);
        isUsbDevice = true;
        m_videoForm->setDevice(this);
    }
    m_stream = new Stream(this);
    if (m_decoder) {
        m_stream->setDecoder(m_decoder);
    }
    m_server = new UsbServer(this);
    connect(m_server, &UsbServer::sigGetInputState, m_videoForm, &VideoForm::onGetInputState);
    connect(m_server, &UsbServer::sigGetInputPos, m_videoForm, &VideoForm::onGetInputPos);
    connect(m_server, &UsbServer::sigLostInputState, m_videoForm, &VideoForm::onLostInputState);
    connect(this, &UsbDevice::sigInputChinese, m_server, &BaseServer::onInputChinese);

    //    if (!m_params.recordFileName.trimmed().isEmpty()) {
    //        m_recorder = new Recorder(m_params.recordFileName);
    //        m_stream->setRecoder(m_recorder);
    //    }

    initSignals();
    startServer();
}

UsbDevice::~UsbDevice() {}
