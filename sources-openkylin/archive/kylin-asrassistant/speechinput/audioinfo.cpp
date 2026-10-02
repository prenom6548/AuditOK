#include "audioinfo.h"
#include <QtEndian>
#include <QDebug>

const quint16 MAXAMPLITUDE = 32767;

AudioInfo::AudioInfo(const QAudioFormat &format, QObject *paren) : QIODevice(paren), m_format(format) {}

AudioInfo::~AudioInfo() {}

qint64 AudioInfo::readInfo(char *data, qint64 len)
{
    if (m_data.count() == 0) {
        return 0;
    }
    qint64 count = qMin(len, (qint64)m_data.count());
    for (qint64 i = 0; i < count; i++) {
        data[i] = m_data[i];
    }
    m_data.erase(m_data.begin(), m_data.begin() + count);

    return count;
}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    return 0;
}

qint64 AudioInfo::writeData(const char *data, qint64 len)
{
    for (qint64 i = 0; i < len; i++) {
        m_data.push_back(data[i]);
    }
    const int channelBytes = m_format.sampleSize() / 8;
    const int sampleBytes = m_format.channelCount() * channelBytes;
    const int numSamples = len / sampleBytes;

    quint16 maxValue = 0;
    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < m_format.channelCount(); ++j) {
            quint16 value = 0;
            value = qAbs(qFromLittleEndian<qint16>(ptr));
            maxValue = qMax(value, maxValue);
            ptr += channelBytes;
        }
    }
    maxValue = qMin(maxValue, MAXAMPLITUDE);
    Q_EMIT updateLevel(qreal(maxValue) / MAXAMPLITUDE);

    return len;
}
