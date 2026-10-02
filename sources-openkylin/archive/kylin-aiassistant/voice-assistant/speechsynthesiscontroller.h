#ifndef SPEECHSYNTHESISCONTROLLER_H
#define SPEECHSYNTHESISCONTROLLER_H

#include <QObject>
#include <QQueue>
#include <QByteArray>
#include <QMutex>
#include <QAtomicInteger>
#include <QVector>

#include <kylin-ai/coreai/speech/audioconfig.h>
#include <kylin-ai/coreai/speech/result.h>
#include <kylin-ai/coreai/speech/synthesizer.h>

#include <atomic>
#include <memory>

class SDL2AudioPlayerThread;

class SpeechSynthesisController : public QObject
{
    Q_OBJECT

public:
    explicit SpeechSynthesisController(QObject *parent = nullptr);
    ~SpeechSynthesisController();

    bool speak(const QString &text);
    bool beginStream();
    bool enqueueText(const QString &text);
    void finishStream();
    void close();

signals:
    void readingStarted();
    void readingFinished();
    void errorOccurred(const QString &message);

private:
    struct CallbackContext;
    struct PendingAudioSegment;

    bool ensureSessionAsync();
    void handleSessionReady(quint64 generation, SpeechSynthesizerSession *session, int errorCode);
    void destroyAudioConfig();
    static void synthesisCallback(SpeechSynthesisResult *result, void *userData);
    void handleSynthesisError(int errorCode);
    bool handleSynthesizedSegment(const QByteArray &audioData, int sampleRate, int channels);
    void handleSynthesisCompleted();
    void handlePlaybackFinished();
    bool startNextSynthesis();
    void maybeFinishReading();
    void resetCurrentSegmentBuffer();
    void handlePlayerInitialized(bool success);
    quint64 advanceGeneration();
    bool isCurrentGeneration(quint64 generation) const;
    void retireCallbackContext();

    SpeechSynthesizerSession *m_session;
    AudioConfig *m_audioConfig;
    SDL2AudioPlayerThread *m_player;
    QQueue<QString> m_pendingTexts;
    QQueue<PendingAudioSegment> m_pendingAudioSegments;
    std::shared_ptr<std::atomic_bool> m_alive;
    std::shared_ptr<CallbackContext> m_callbackContext;
    QVector<std::shared_ptr<CallbackContext>> m_retiredCallbackContexts;
    QByteArray m_currentSegmentBuffer;
    QMutex m_callbackMutex;
    int m_cachedSampleRate = 0;
    int m_cachedChannels = 0;
    QAtomicInteger<quint64> m_generation;
    bool m_sessionStarting = false;
    bool m_playerInitInFlight = false;
    bool m_acceptingStreamText;
    bool m_synthesisInFlight;
    bool m_audioPlaybackPending = false;
    bool m_readingActive;
};

#endif // SPEECHSYNTHESISCONTROLLER_H
