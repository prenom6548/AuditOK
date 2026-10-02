#include <QDebug>
#include <QTime>

#include "decoder.h"
#include "stream.h"
#include "videosocket.h"
// #include "recorder.h"

const int HEADER_SIZE = 16;
static const AVRational SCRCPY_TIME_BASE = {1, 1000000};  // timestamps in us

const int OFFSET_TIME = 1500;
#define NO_PTS UINT64_MAX

Stream::Stream(QObject *parent) : QThread(parent)
{
}

Stream::~Stream()
{
}

static void avLogCallback(void *avcl, int level, const char *fmt, va_list vl)
{
    Q_UNUSED(avcl)
    Q_UNUSED(vl)

    QString localFmt = QString::fromUtf8(fmt);
    localFmt.prepend("[FFmpeg] ");
    switch (level) {
        case AV_LOG_PANIC:
        case AV_LOG_FATAL:
            qFatal("%s", localFmt.toUtf8().data());
            break;
        case AV_LOG_ERROR:
            qCritical() << localFmt.toUtf8();
            break;
        case AV_LOG_WARNING:
            qWarning() << localFmt.toUtf8();
            break;
        case AV_LOG_INFO:
            qInfo() << localFmt.toUtf8();
            break;
        case AV_LOG_DEBUG:
            break;
    }

    // do not forward others, which are too verbose
    return;
}

bool Stream::init()
{
#ifdef LAVF_REQUIRES_REGISTER_ALL
    av_register_all();
#endif
    if (avformat_network_init()) {
        return false;
    }
    av_log_set_callback(avLogCallback);
    return true;
}

void Stream::deInit()
{
    avformat_network_deinit();  // ignore failure
}

void Stream::setDecoder(Decoder *decoder)
{
    m_decoder = decoder;
}

static quint32 bufferRead32be(quint8 *buf)
{
    return static_cast<quint32>((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}

static quint64 bufferRead64be(quint8 *buf)
{
    quint32 msb = bufferRead32be(buf);
    quint32 lsb = bufferRead32be(&buf[4]);
    return (static_cast<quint64>(msb) << 32) | lsb;
}

void Stream::setVideoSocket(VideoSocket *videoSocket)
{
    m_videoSocket = videoSocket;
}

// void Stream::setRecoder(Recorder *recorder)
//{
//        m_recorder = recorder;
//}

qint32 Stream::recvData(quint8 *buf, qint32 bufSize)
{
    if (!buf) {
        return 0;
    }
    if (m_videoSocket) {
        qint32 len = m_videoSocket->subThreadRecvData(buf, bufSize);
        return len;
    }
    return 0;
}

bool Stream::startDecode()
{
    if (!m_videoSocket) {
        return false;
    }
    start();
    return true;
}

void Stream::stopDecode()
{
    if (m_decoder) {
        m_decoder->interrupt();
    }
    wait();
}

void Stream::run()
{
    m_codecCtx = Q_NULLPTR;
    m_parserCtx = Q_NULLPTR;

    // codec
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (strcmp(codec->name, "h264_omx_dec") == 0) {
        qInfo() << "Filtered OMX decoder.";
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    }
    if (!codec) {
        qCritical("H.264 decoder not found");
        goto runQuit;
    }

    // codeCtx
    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        qCritical("Could not allocate codec context");
        goto runQuit;
    }

    // 打开AVCodec对象
    if (m_decoder && !m_decoder->open(codec)) {
        qCritical("Could not open m_decoder");
        goto runQuit;
    }

    //    if (m_recorder) {
    //        if (!m_recorder->open(codec)) {
    //            qCritical("Could not open recorder");
    //            goto runQuit;
    //        }
    //
    //        if (!m_recorder->startRecorder()) {
    //            qCritical("Could not start recorder");
    //            goto runQuit;
    //        }
    //    }

    m_parserCtx = av_parser_init(AV_CODEC_ID_H264);
    if (!m_parserCtx) {
        qCritical("Could not initialize parser");
        goto runQuit;
    }

    // We must only pass complete frames to av_parser_parse2()!
    // It's more complicated, but this allows to reduce the latency by 1 frame!
    m_parserCtx->flags |= PARSER_FLAG_COMPLETE_FRAMES;

    for (;;) {
        AVPacket packet;
        int ok = recvPacket(&packet);
        if (!ok) {
            // end of stream
            break;
        }

        bool dropPacket = false;

        qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        if (currentTime >=
            m_initTime + m_ptsTime * 1000 + OFFSET_TIME)  // 当前音频时间已经超过了下一帧关键帧之前了，就需要丢帧了
        {
            dropPacket = true;
        }

        if (m_flag == AV_PKT_FLAG_KEY)  // 关键帧不丢
        {
            dropPacket = false;
        }
        if (dropPacket) {
            av_packet_unref(&packet);
            continue;
        }

        pushPacket(&packet);
        av_packet_unref(&packet);
        if (!ok) {
            qDebug("av_packet_unref fail");
            // cannot process packet (error already logged)
            break;
        }
    }

    qInfo("End of frames");

    if (m_hasPending) {
        av_packet_unref(&m_pending);
    }

    av_parser_close(m_parserCtx);

runQuit:
    //    if (m_recorder) {
    //        if (m_recorder->isRunning()) {
    //            m_recorder->stopRecorder();
    //            m_recorder->wait();
    //        }
    //        m_recorder->close();
    //    }
    if (m_decoder) {
        m_decoder->close();
    }
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
    }

    Q_EMIT onStreamStop();
}

bool Stream::recvPacket(AVPacket *packet)
{
    // The video stream contains raw packets, without time information. When we
    // record, we retrieve the timestamps separately, from a "meta" header
    // added by the server before each raw packet.
    //
    // The "meta" header length is 12 bytes:
    // [. . . . . . . .|. . . .]. . . . . . . . . . . . . . . ...
    //  <-------------> <-----> <-----------------------------...
    //        PTS        packet        raw packet
    //                    size
    //
    // It is followed by <packet_size> bytes containing the packet/frame.

    quint8 header[HEADER_SIZE] = {0};
    qint32 r = recvData(header, HEADER_SIZE);

    if (r < HEADER_SIZE) {
        return false;
    }

    quint64 pts = bufferRead64be(header);
    quint32 len = bufferRead32be(&header[8]);
    m_flag = bufferRead32be(&header[12]);

    if (pts == 0) {
        m_initTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    m_ptsTime = pts * av_q2d(SCRCPY_TIME_BASE);

    Q_ASSERT(pts == NO_PTS || (pts & 0x8000000000000000) == 0);

    Q_ASSERT(len);

    if (av_new_packet(packet, static_cast<int>(len))) {
        qCritical("Could not allocate packet");
        // return false;
    }

    r = recvData(packet->data, static_cast<qint32>(len));

    if (r < 0 || static_cast<quint32>(r) < len) {
        av_packet_unref(packet);
        qCritical("receive packet fail");
        return false;
    }

    packet->pts = pts != NO_PTS ? static_cast<int64_t>(pts) : static_cast<int64_t>(AV_NOPTS_VALUE);

    return true;
}

bool Stream::pushPacket(AVPacket *packet)
{
    bool isConfig = packet->pts == AV_NOPTS_VALUE;

    // A config packet must not be decoded immetiately (it contains no
    // frame); instead, it must be concatenated with the future data packet.
    if (m_hasPending || isConfig) {
        qint32 offset;
        if (m_hasPending) {
            offset = m_pending.size;
            if (av_grow_packet(&m_pending, packet->size)) {
                qCritical("Could not grow packet");
                return false;
            }
        } else {
            offset = 0;
            if (av_new_packet(&m_pending, packet->size)) {
                qCritical("Could not create packet");
                return false;
            }
            m_hasPending = true;
        }

        memcpy(m_pending.data + offset, packet->data, static_cast<unsigned int>(packet->size));

        if (!isConfig) {
            // prepare the concat packet to send to the decoder
            m_pending.pts = packet->pts;
            m_pending.dts = packet->dts;
            m_pending.flags = packet->flags;
            packet = &m_pending;
        }
    }

    if (isConfig) {
        // config packet
        bool ok = processConfigPacket(packet);
        if (!ok) {
            return false;
        }
    } else {
        // data packet
        bool ok = parse(packet);

        if (m_hasPending) {
            // the pending packet must be discarded (consumed or error)
            m_hasPending = false;
            av_packet_unref(&m_pending);
        }

        if (!ok) {
            return false;
        }
    }
    return true;
}

bool Stream::processConfigPacket(AVPacket *packet)
{
    //    if (m_recorder && !m_recorder->push(packet)) {
    //        qCritical("Could not send config packet to recorder");
    //        return false;
    //    }
    return true;
}

bool Stream::parse(AVPacket *packet)
{
    quint8 *inData = packet->data;
    int inLen = packet->size;
    quint8 *outData = Q_NULLPTR;
    int outLen = 0;
    int r =
        av_parser_parse2(m_parserCtx, m_codecCtx, &outData, &outLen, inData, inLen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, -1);

    // PARSER_FLAG_COMPLETE_FRAMES is set
    Q_ASSERT(r == inLen);
    (void)r;
    Q_ASSERT(outLen == inLen);

    // 第一帧
    if (m_parserCtx->key_frame == 1) {
        packet->flags |= AV_PKT_FLAG_KEY;
    }

    bool ok = processFrame(packet);
    if (!ok) {
        qCritical("Could not process frame");
        return false;
    }

    return true;
}

bool Stream::processFrame(AVPacket *packet)
{
    if (m_decoder && !m_decoder->push(packet)) {
        return false;
    }

    //    if (m_recorder) {
    //        packet->dts = packet->pts;
    //
    //        if (!m_recorder->push(packet)) {
    //            qCritical("Could not send packet to recorder");
    //            return false;
    //        }
    //    }

    return true;
}
