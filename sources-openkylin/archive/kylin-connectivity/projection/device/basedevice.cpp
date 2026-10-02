#include "basedevice.h"
#include <QDir>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <uibase/tipwidget.h>

#include "avframeconvert.h"
#include "config.h"
#include "controller.h"
#include "decoder.h"
//#include "filehandler.h"
#include "mousetap/mousetap.h"
//#include "recorder.h"
#include "baseserver.h"
#include "stream.h"
#include "videobuffer.h"
#include "videoform.h"
#include <generatetools.h>
#include "windowmanage.hpp"

extern "C"
{
#include "libavutil/imgutils.h"
}

const std::string SNDCPYPATH = "/usr/share/kylin-connectivity/sndcpy ";
const std::string LOGPATH = "~/.log/kylin-connectivity.log";

BaseDevice::BaseDevice(DeviceParams params, QObject *parent) : QObject(parent)
{
    m_params = params;
    // m_interface = new QDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
    //                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());
}

BaseDevice::~BaseDevice()
{
    if (m_server) {
        m_server->stop();
    }
    // server must stop before decoder, because decoder block main thread
    if (m_stream) {
        m_stream->stopDecode();
    }

    //    if (m_recorder) {
    //        delete m_recorder;
    //    }
    if (m_vb) {
        m_vb->deInit();
        delete m_vb;
    }
    if (m_videoForm) {
        m_videoForm->deleteUI();
    }
    // if (m_interface) {
    //     delete m_interface;
    //     m_interface = nullptr;
    // }
    Q_EMIT deviceDisconnect(m_params.serial);
}

void BaseDevice::initSignals()
{
    connect(this, &BaseDevice::screenshot, this, &BaseDevice::onScreenshot);
    connect(this, &BaseDevice::showTouch, this, &BaseDevice::onShowTouch);
    connect(this, &BaseDevice::setControlState, this, &BaseDevice::onSetControlState);
    connect(this, &BaseDevice::grabCursor, this, &BaseDevice::onGrabCursor);
    if (m_controller) {
        connect(m_controller, &Controller::grabCursor, this, &BaseDevice::grabCursor);
    }
    if (m_controller) { // 事件处理
        connect(this, &BaseDevice::postGoBack, m_controller, &Controller::onPostGoBack);
        connect(this, &BaseDevice::postGoHome, m_controller, &Controller::onPostGoHome);
        connect(this, &BaseDevice::postGoMenu, m_controller, &Controller::onPostGoMenu);
        connect(this, &BaseDevice::postAppSwitch, m_controller, &Controller::onPostAppSwitch);
        connect(this, &BaseDevice::postPower, m_controller, &Controller::onPostPower);
        connect(this, &BaseDevice::postVolumeUp, m_controller, &Controller::onPostVolumeUp);
        connect(this, &BaseDevice::postVolumeDown, m_controller, &Controller::onPostVolumeDown);
        connect(this, &BaseDevice::postCopy, m_controller, &Controller::onCopy);
        connect(this, &BaseDevice::postCut, m_controller, &Controller::onCut);
        connect(this, &BaseDevice::setScreenPowerMode, m_controller, &Controller::onSetScreenPowerMode);
        connect(this, &BaseDevice::expandNotificationPanel, m_controller, &Controller::onExpandNotificationPanel);
        connect(this, &BaseDevice::collapsePanel, m_controller, &Controller::onCollapsePanel);
        connect(this, &BaseDevice::mouseEvent, m_controller, &Controller::onMouseEvent);
        connect(this, &BaseDevice::wheelEvent, m_controller, &Controller::onWheelEvent);
        connect(this, &BaseDevice::keyEvent, m_controller, &Controller::onKeyEvent);

        connect(this, &BaseDevice::postBackOrScreenOn, m_controller, &Controller::onPostBackOrScreenOn);
        connect(this, &BaseDevice::setDeviceClipboard, m_controller, &Controller::onSetDeviceClipboard);
        connect(this, &BaseDevice::clipboardPaste, m_controller, &Controller::onClipboardPaste);
        connect(this, &BaseDevice::postTextInput, m_controller, &Controller::onPostTextInput);
    }

    //    if (m_fileHandler) {
    //        connect(this, &BaseDevice::pushFileRequest, this, [this](const QString &file, const QString &devicePath) {
    //            m_fileHandler->onPushFileRequest(getSerial(), file, devicePath);
    //        });
    //        connect(this, &BaseDevice::installApkRequest, this, [this](const QString &apkFile) {
    //            m_fileHandler->onInstallApkRequest(getSerial(), apkFile);
    //        });
    //        connect(m_fileHandler, &FileHandler::fileHandlerResult, this,
    //                [this](FileHandler::FILE_HANDLER_RESULT processResult, bool isApk) {
    //                    QString tipsType = "";
    //                    if (isApk) {
    //                        tipsType = tr("install apk");
    //                    } else {
    //                        tipsType = tr("file transfer");
    //                    }
    //                    QString tips;
    //                    if (FileHandler::FAR_IS_RUNNING == processResult && m_videoForm) {
    //                        tips = tr("wait current %1 to complete").arg(tipsType);
    //                    }
    //                    if (FileHandler::FAR_SUCCESS_EXEC == processResult && m_videoForm) {
    //                        tips = tr("%1 complete, save in
    //                        %2").arg(tipsType).arg(Config::getInstance().getPushFilePath());
    //                    }
    //                    if (FileHandler::FAR_ERROR_EXEC == processResult && m_videoForm) {
    //                        tips = tr("%1 failed").arg(tipsType);
    //                    }
    //                    qInfo() << tips;
    //                    if (m_controlState == GCS_CLIENT) {
    //                        return;
    //                    }
    //                });
    //    }

    if (m_server) {
        connect(m_server, &BaseServer::serverStartResult, this, [this](bool success) {
            if (success) {
                m_server->connectTo();
            } else {
                deleteLater();
            }
        });

        connect(m_server, &BaseServer::connectToResult, this, &BaseDevice::onConnectToResult);

        connect(m_server, &BaseServer::onServerStop, this, [this]() {
            deleteLater();
            qInfo() << "server process stop";
        });
        connect(m_server, &BaseServer::sigSocketDisconnect, this, [this]() {
            // m_interface->call("Uninhibit", m_inhibitValue);
            system("pkill vlc");
            Q_EMIT sigSocketDisconnect();
        });
    }

    if (m_stream) {
        connect(m_stream, &Stream::onStreamStop, this, [this]() {
            deleteLater();
            qInfo() << "stream thread stop";
        });
    }

    if (m_decoder && m_vb) {
        // must be Qt::QueuedConnection, ui update must be main thread
        connect(
            m_decoder, &Decoder::onNewFrame, this,
            [this]() {
                m_vb->lock();
                const AVFrame *frame = m_vb->consumeRenderedFrame();
                if (m_videoForm) {
                    m_videoForm->updateRender(frame);
                }
                m_vb->unLock();
            },
            Qt::QueuedConnection);
    }
}


VideoForm *BaseDevice::getVideoForm()
{
    return m_videoForm;
}

const QString &BaseDevice::getSerial()
{
    return m_params.serial;
}


const QSize BaseDevice::frameSize()
{
    QSize size;
    if (!m_videoForm) {
        return size;
    }
    return m_videoForm->frameSize();
}

void BaseDevice::updateScript(QString script)
{
    if (m_controller) {
        m_controller->updateScript(script);
    }
}

BaseDevice::GroupControlState BaseDevice::controlState()
{
    return m_controlState;
}

void BaseDevice::startServer()
{
    qInfo() << "start screen shot server";
    // fix: macos cant recv finished signel, timer is ok
    QTimer::singleShot(0, this, [this]() {
        m_startTimeCount.start();
        BaseServer::ServerParams params;
        params.serial = m_params.serial;
        params.localPort = m_params.localPort;
        params.maxSize = m_params.maxSize;
        params.bitRate = m_params.bitRate;
        params.maxFps = m_params.maxFps;
        params.crop = "";
        params.control = true;
        params.useReverse = m_params.useReverse;
        params.lockVideoOrientation = m_params.lockVideoOrientation;
        params.stayAwake = m_params.stayAwake;
        m_server->start(params);
    });
}

bool BaseDevice::isCurrentCustomKeymap()
{
    if (!m_controller) {
        return false;
    }
    return m_controller->isCurrentCustomKeymap();
}

bool BaseDevice::saveFrame(const AVFrame *frame)
{
    if (!frame) {
        return false;
    }

    // create buffer
    QImage rgbImage(frame->width, frame->height, QImage::Format_RGB32);
    AVFrame *rgbFrame = av_frame_alloc();
    if (!rgbFrame) {
        return false;
    }

    // bind buffer to AVFrame
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, rgbImage.bits(), AV_PIX_FMT_RGB32, frame->width,
                         frame->height, 4);

    // convert
    AVFrameConvert convert;
    convert.setSrcFrameInfo(frame->width, frame->height, AV_PIX_FMT_YUV420P);
    convert.setDstFrameInfo(frame->width, frame->height, AV_PIX_FMT_RGB32);
    bool ret = false;
    ret = convert.init();
    if (!ret) {
        return false;
    }
    ret = convert.convert(frame, rgbFrame);
    if (!ret) {
        return false;
    }
    convert.deInit();
    av_free(rgbFrame);

    // save
    QString absFilePath;
    QString fileDir(m_params.recordPath);
    if (fileDir.isEmpty()) {
        qWarning() << "please select record save path!!!";
        return false;
    }
    QDateTime dateTime = QDateTime::currentDateTime();
    QString fileName = dateTime.toString("_yyyyMMdd_hhmmss_zzz");
    fileName = Config::getInstance().getTitle() + fileName + ".png";
    QDir dir(fileDir);
    absFilePath = dir.absoluteFilePath(fileName);
    ret = rgbImage.save(absFilePath, "PNG", 100);
    if (!ret) {
        return false;
    }

    qInfo() << "screenshot save to " << absFilePath;
    return true;
}

void BaseDevice::onScreenshot()
{
    if (!m_vb) {
        return;
    }

    m_vb->lock();
    // screenshot
    saveFrame(m_vb->peekRenderedFrame());
    m_vb->unLock();
}

void BaseDevice::onShowTouch(bool show)
{
    AdbProcess *adb = new AdbProcess();
    if (!adb) {
        return;
    }
    connect(adb, &AdbProcess::adbProcessResult, this, [this](AdbProcess::ADB_EXEC_RESULT processResult) {
        if (AdbProcess::AER_SUCCESS_START != processResult) {
            sender()->deleteLater();
        }
    });
    adb->setShowTouchesEnabled(getSerial(), show);

    qInfo() << getSerial() << " show touch " << (show ? "enable" : "disable");
}

void BaseDevice::onSetControlState(BaseDevice *device, BaseDevice::GroupControlState state)
{
    Q_UNUSED(device)
    if (m_controlState == state) {
        return;
    }
    GroupControlState oldState = m_controlState;
    m_controlState = state;
    Q_EMIT controlStateChange(this, oldState, m_controlState);
}

void BaseDevice::onGrabCursor(bool grab)
{
    if (!m_videoForm) {
        return;
    }
    if (m_controlState == GCS_CLIENT) {
        return;
    }
    QRect rc = m_videoForm->getGrabCursorRect();
    MouseTap::getInstance()->enableMouseEventTap(rc, grab);
}

void BaseDevice::onConnectToResult(bool success, const QString &deviceName, const QSize &size, const QString &clientIP)
{
    if (success) {
        double diff = m_startTimeCount.elapsed() / 1000.0;
        qInfo() << QString("server start finish in %1s").arg(diff).toStdString().c_str();
        m_size = size;
        m_clientIP = clientIP;

        if (m_videoForm) {
            startAudio();
            startVideo();
            showSupportControl();
            Q_EMIT sigScreenConnected();
        }
    }
}

bool BaseDevice::getDeviceStatus()
{
    return isUsbDevice;
}

void BaseDevice::startAudio()
{
    auto sndcpy = [this]() {
        qInfo() << "start sndcpy";
        std::string serials = SNDCPYPATH + QString::number(m_params.localPort).toStdString() + " "
                              + m_params.serial.toStdString() + ">>" + LOGPATH;
        system(serials.data());
    };
    std::thread sndcpyThread(sndcpy);
    sndcpyThread.detach();
}

void BaseDevice::startVideo()
{
    // 添加窗管协议
    kabase::WindowManage::removeHeader(m_videoForm);
    //    m_videoForm->show();
    // init recorder
    //                            if (m_recorder) {
    //                                m_recorder->setFrameSize(m_size);
    //                            }
    // init decoder
    m_stream->setVideoSocket(m_server->getVideoSocket());
    m_stream->startDecode();

    // init controller
    if (m_controller) {
        m_controller->setControlSocket(m_server->getControlSocket());
    }

    // 显示界面时才自动息屏（m_params.display）
    if (m_params.closeScreen && m_params.display && m_controller) {
        Q_EMIT m_controller->onSetScreenPowerMode(ControlMsg::SPM_OFF);
    }
}

void BaseDevice::showSupportControl()
{
    TipWidget *tipWidget = new TipWidget();
    tipWidget->setParent(m_videoForm);
    if (isUsbDevice) {
        tipWidget->SetMesseage(tr("Control Devices Supported"), nullptr, 20);
        m_videoForm->showToolForm(true);
        //                                Q_EMIT this->setScreenPowerMode(ControlMsg::SPM_OFF);
    } else {
        tipWidget->SetMesseage(tr("Control device not supported"), nullptr, 20);
        m_videoForm->showToolForm(false);
    }
}
