#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#include <QObject>
#include <QIODevice>
#include <QList>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDeviceInfo>

class AudioInfo : public QIODevice
{
    Q_OBJECT
public:
    AudioInfo(const QAudioFormat &format, QObject *paren = nullptr);
    ~AudioInfo();

    qint64 readInfo(char *data, qint64 len);

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

Q_SIGNALS:
    void updateLevel(double);

private:
    QAudioFormat m_format;
    QList<char> m_data;
};

#endif // AUDIOINFO_H
