#include "speechsynthesiscontroller.h"

#include "sdl2audioplayerthread.h"
#include "speecherrormessage.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QMetaObject>
#include <QMutexLocker>
#include <QThread>
#include <thread>

namespace {
void destroySynthesisSessionAsync(SpeechSynthesizerSession *session,
                                  std::shared_ptr<void> context)
{
    if (!session) {
        return;
    }

    std::thread([session, context]() {
        Q_UNUSED(context);
        SpeechSynthesizerSession *sessionToDestroy = session;
        speech_synthesizer_destroy_session(&sessionToDestroy);
    }).detach();
}
} // namespace

struct SpeechSynthesisController::CallbackContext
{
    SpeechSynthesisController *controller = nullptr;
    std::shared_ptr<std::atomic_bool> alive;
    quint64 generation = 0;
};

struct SpeechSynthesisController::PendingAudioSegment
{
    QByteArray audioData;
    int sampleRate = 0;
    int channels = 0;
};

SpeechSynthesisController::SpeechSynthesisController(QObject *parent)
    : QObject(parent)
    , m_session(nullptr)
    , m_audioConfig(nullptr)
    , m_player(new SDL2AudioPlayerThread(this))
    , m_alive(std::make_shared<std::atomic_bool>(true))
    , m_generation(0)
    , m_acceptingStreamText(false)
    , m_synthesisInFlight(false)
    , m_audioPlaybackPending(false)
    , m_readingActive(false)
{
    connect(m_player, &SDL2AudioPlayerThread::playbackFinished,
            this, &SpeechSynthesisController::handlePlaybackFinished);
    connect(m_player, &SDL2AudioPlayerThread::initialized,
            this, &SpeechSynthesisController::handlePlayerInitialized);
}

SpeechSynthesisController::~SpeechSynthesisController()
{
    m_alive->store(false);
    close();
    m_retiredCallbackContexts.clear();
}

bool SpeechSynthesisController::speak(const QString &text)
{
    close();
    if (!beginStream()) {
        return false;
    }
    if (!enqueueText(text)) {
        close();
        return false;
    }
    finishStream();
    return true;
}

bool SpeechSynthesisController::beginStream()
{
    close();

    m_acceptingStreamText = true;
    m_player->setExpectingMoreData(true);
    return ensureSessionAsync();
}

bool SpeechSynthesisController::enqueueText(const QString &text)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return true;
    }

    if (!m_acceptingStreamText && !m_readingActive) {
        if (!beginStream()) {
            return false;
        }
    }

    m_pendingTexts.enqueue(trimmed);
    return startNextSynthesis();
}

void SpeechSynthesisController::finishStream()
{
    m_acceptingStreamText = false;
    maybeFinishReading();
}

bool SpeechSynthesisController::startNextSynthesis()
{
    if (m_synthesisInFlight || m_pendingTexts.isEmpty()) {
        return true;
    }

    if (!m_session) {
        return ensureSessionAsync();
    }

    destroyAudioConfig();
    m_audioConfig = audio_config_create_audio_output_from_pcm_data();
    if (!m_audioConfig) {
        Q_EMIT errorOccurred(tr("Failed to create speech synthesis audio config."));
        return false;
    }

    audio_config_set_input_audio_rate(m_audioConfig, 16000);
    speech_synthesizer_set_audio_config(m_session, m_audioConfig);

    const QString trimmed = m_pendingTexts.dequeue();
    resetCurrentSegmentBuffer();
    const QByteArray utf8Text = trimmed.toUtf8();
    speech_synthesizer_synthesize_text_async(m_session,
                                             utf8Text.constData(),
                                             static_cast<uint32_t>(utf8Text.size()));
    m_synthesisInFlight = true;
    if (!m_readingActive) {
        m_readingActive = true;
        Q_EMIT readingStarted();
    }
    return true;
}

void SpeechSynthesisController::close()
{
    advanceGeneration();
    m_pendingTexts.clear();
    m_acceptingStreamText = false;
    m_synthesisInFlight = false;
    m_audioPlaybackPending = false;
    m_readingActive = false;
    m_sessionStarting = false;
    m_playerInitInFlight = false;
    m_pendingAudioSegments.clear();
    resetCurrentSegmentBuffer();

    destroyAudioConfig();

    if (m_session) {
        SpeechSynthesizerSession *sessionToDestroy = m_session;
        auto context = m_callbackContext;
        m_session = nullptr;
        retireCallbackContext();
        destroySynthesisSessionAsync(sessionToDestroy, context);
    } else {
        retireCallbackContext();
    }

    if (m_player) {
        m_player->setExpectingMoreData(false);
        m_player->clearQueue();
        m_player->stop();
    }
}

bool SpeechSynthesisController::ensureSessionAsync()
{
    if (m_session) {
        return true;
    }
    if (m_sessionStarting) {
        return true;
    }

    const quint64 generation = m_generation.loadAcquire();
    m_sessionStarting = true;
    std::shared_ptr<std::atomic_bool> alive = m_alive;
    std::thread([this, alive, generation]() {
        SpeechSynthesizerSession *session = speech_synthesizer_create_session();
        int errorCode = 0;
        if (!session) {
            errorCode = -10000;
        } else {
            errorCode = speech_synthesizer_init_session(session);
        }

        QObject *app = QCoreApplication::instance();
        if (!app || !alive->load()) {
            destroySynthesisSessionAsync(session, nullptr);
            return;
        }

        QMetaObject::invokeMethod(app, [this, alive, generation, session, errorCode]() {
            if (!alive->load() || !isCurrentGeneration(generation)) {
                destroySynthesisSessionAsync(session, nullptr);
                return;
            }
            handleSessionReady(generation, session, errorCode);
        }, Qt::QueuedConnection);
    }).detach();

    return true;
}

void SpeechSynthesisController::handleSessionReady(quint64 generation,
                                                   SpeechSynthesizerSession *session,
                                                   int errorCode)
{
    m_sessionStarting = false;
    if (!isCurrentGeneration(generation)) {
        destroySynthesisSessionAsync(session, nullptr);
        return;
    }

    if (!session) {
        m_synthesisInFlight = false;
        m_readingActive = false;
        m_acceptingStreamText = false;
        m_audioPlaybackPending = false;
        m_pendingTexts.clear();
        if (m_player) {
            m_player->setExpectingMoreData(false);
            m_player->clearQueue();
            m_player->stop();
        }
        Q_EMIT errorOccurred(tr("Failed to create speech synthesis session."));
        return;
    }

    if (errorCode != 0) {
        if (errorCode == -10000) {
            Q_EMIT errorOccurred(tr("Failed to create speech synthesis session."));
        } else {
            handleSynthesisError(errorCode);
        }
        destroySynthesisSessionAsync(session, nullptr);
        return;
    }

    m_session = session;
    retireCallbackContext();
    m_callbackContext = std::make_shared<CallbackContext>();
    m_callbackContext->controller = this;
    m_callbackContext->alive = m_alive;
    m_callbackContext->generation = m_generation.loadAcquire();
    speech_synthesizer_result_set_callback(m_session, synthesisCallback, m_callbackContext.get());
    startNextSynthesis();
}

void SpeechSynthesisController::destroyAudioConfig()
{
    if (!m_audioConfig) {
        return;
    }

    AudioConfig *configToDestroy = m_audioConfig;
    audio_config_destroy(&configToDestroy);
    m_audioConfig = nullptr;
}

void SpeechSynthesisController::synthesisCallback(SpeechSynthesisResult *result, void *userData)
{
    CallbackContext *context = static_cast<CallbackContext *>(userData);
    SpeechSynthesisController *controller = context ? context->controller : nullptr;
    if (!context || !context->alive || !context->alive->load() || !controller || !result) {
        return;
    }
    const quint64 generation = context->generation;
    const auto alive = context->alive;
    if (!controller->isCurrentGeneration(generation)) {
        return;
    }

    const int errorCode = speech_synthesis_result_get_error_code(result);
    if (errorCode != 0 && errorCode != -1) {
        QObject *app = QCoreApplication::instance();
        if (!app) {
            return;
        }
        QMetaObject::invokeMethod(app, [controller, alive, generation, errorCode]() {
            if (!alive->load() || !controller->isCurrentGeneration(generation)) {
                return;
            }
            controller->handleSynthesisError(errorCode);
        }, Qt::QueuedConnection);
        return;
    }

    const SpeechResultReason reason = speech_synthesis_result_get_reason(result);
    if (reason == SPEECH_SYNTHESIZING
            || reason == SPEECH_SYNTHESIS_STARTED
            || reason == SPEECH_SYNTHESIS_COMPLETED) {
        const int sampleRate = speech_synthesis_result_get_audio_rate(result);
        const int channels = speech_synthesis_result_get_audio_channel(result);
        if (sampleRate > 0 && channels > 0) {
            QMutexLocker locker(&controller->m_callbackMutex);
            if (!controller->isCurrentGeneration(generation)) {
                return;
            }
            if (controller->m_cachedSampleRate <= 0 || controller->m_cachedChannels <= 0) {
                controller->m_cachedSampleRate = sampleRate;
                controller->m_cachedChannels = channels;
            }
        }
    }

    uint32_t dataLength = 0;
    const uint8_t *pcmData = speech_synthesis_result_get_data(result, &dataLength);
    if (pcmData && dataLength > 0) {
        QMutexLocker locker(&controller->m_callbackMutex);
        if (!controller->isCurrentGeneration(generation)) {
            return;
        }
        controller->m_currentSegmentBuffer.append(reinterpret_cast<const char *>(pcmData),
                                                  static_cast<int>(dataLength));
    }

    if (reason == SPEECH_SYNTHESIS_COMPLETED) {
        QByteArray segmentData;
        int sampleRate = 0;
        int channels = 0;
        {
            QMutexLocker locker(&controller->m_callbackMutex);
            if (!controller->isCurrentGeneration(generation)) {
                return;
            }
            segmentData = controller->m_currentSegmentBuffer;
            controller->m_currentSegmentBuffer.clear();
            sampleRate = controller->m_cachedSampleRate;
            channels = controller->m_cachedChannels;
            controller->m_cachedSampleRate = 0;
            controller->m_cachedChannels = 0;
        }

        QObject *app = QCoreApplication::instance();
        if (!app) {
            return;
        }
        QMetaObject::invokeMethod(app, [controller, alive, generation, segmentData, sampleRate, channels]() {
            if (!alive->load() || !controller->isCurrentGeneration(generation)) {
                return;
            }
            if (!controller->handleSynthesizedSegment(segmentData, sampleRate, channels)) {
                controller->m_synthesisInFlight = false;
                controller->m_readingActive = false;
                controller->m_acceptingStreamText = false;
                controller->m_audioPlaybackPending = false;
                controller->m_pendingTexts.clear();
                controller->maybeFinishReading();
                return;
            }
            controller->handleSynthesisCompleted();
        }, Qt::QueuedConnection);
    }
}

void SpeechSynthesisController::handleSynthesisError(int errorCode)
{
    m_synthesisInFlight = false;
    m_readingActive = false;
    m_acceptingStreamText = false;
    m_audioPlaybackPending = false;
    m_pendingTexts.clear();
    resetCurrentSegmentBuffer();
    if (m_player) {
        m_player->setExpectingMoreData(false);
        m_player->clearQueue();
        m_player->stop();
    }
    const QString mapped = speechErrorMessageForCode(errorCode);
    Q_EMIT errorOccurred(mapped.isEmpty()
                                 ? tr("Speech synthesis error: %1").arg(errorCode)
                                 : mapped);
}

bool SpeechSynthesisController::handleSynthesizedSegment(const QByteArray &audioData, int sampleRate, int channels)
{
    if (audioData.isEmpty()) {
        return true;
    }

    if (sampleRate <= 0 || channels <= 0) {
        Q_EMIT errorOccurred(tr("Failed to read speech synthesis audio format."));
        return false;
    }

    if (!m_player->isInitialized()) {
        PendingAudioSegment segment;
        segment.audioData = audioData;
        segment.sampleRate = sampleRate;
        segment.channels = channels;
        m_pendingAudioSegments.enqueue(segment);
        m_audioPlaybackPending = true;

        if (!m_playerInitInFlight && !m_player->isInitializing()) {
            m_playerInitInFlight = true;
            m_player->initAsync(sampleRate, channels, AUDIO_S16LSB);
        }
        return true;
    }

    m_player->enqueueData(audioData);
    m_audioPlaybackPending = true;
    if (!m_player->isPlaying()) {
        m_player->play();
    }
    return true;
}

void SpeechSynthesisController::handlePlayerInitialized(bool success)
{
    if (!m_playerInitInFlight && m_pendingAudioSegments.isEmpty()) {
        return;
    }

    m_playerInitInFlight = false;
    if (!success) {
        m_pendingAudioSegments.clear();
        m_synthesisInFlight = false;
        m_readingActive = false;
        m_acceptingStreamText = false;
        m_audioPlaybackPending = false;
        m_pendingTexts.clear();
        Q_EMIT errorOccurred(tr("Failed to initialize SDL audio output."));
        maybeFinishReading();
        return;
    }

    while (!m_pendingAudioSegments.isEmpty()) {
        const PendingAudioSegment segment = m_pendingAudioSegments.dequeue();
        Q_UNUSED(segment.sampleRate);
        Q_UNUSED(segment.channels);
        m_player->enqueueData(segment.audioData);
    }

    m_audioPlaybackPending = true;
    if (!m_player->isPlaying()) {
        m_player->play();
    }
    maybeFinishReading();
}

void SpeechSynthesisController::handleSynthesisCompleted()
{
    m_synthesisInFlight = false;
    startNextSynthesis();
    maybeFinishReading();
}

void SpeechSynthesisController::handlePlaybackFinished()
{
    if (m_player) {
        m_player->stop();
    }
    m_audioPlaybackPending = false;
    const bool waitingForText = m_acceptingStreamText || m_synthesisInFlight || !m_pendingTexts.isEmpty();
    if (waitingForText || !m_readingActive) {
        return;
    }

    m_readingActive = false;
    Q_EMIT readingFinished();
}

void SpeechSynthesisController::resetCurrentSegmentBuffer()
{
    QMutexLocker locker(&m_callbackMutex);
    m_currentSegmentBuffer.clear();
    m_cachedSampleRate = 0;
    m_cachedChannels = 0;
}

void SpeechSynthesisController::maybeFinishReading()
{
    const bool waitingForText = m_acceptingStreamText || m_synthesisInFlight || !m_pendingTexts.isEmpty();
    const bool hasBufferedAudio = m_player && m_player->hasBufferedAudio();
    if (m_player) {
        m_player->setExpectingMoreData(waitingForText);
    }
    if (waitingForText || m_audioPlaybackPending || hasBufferedAudio || !m_readingActive) {
        return;
    }
    if (m_player && m_player->isPlaying()) {
        return;
    }

    m_readingActive = false;
    Q_EMIT readingFinished();
}

quint64 SpeechSynthesisController::advanceGeneration()
{
    return ++m_generation;
}

bool SpeechSynthesisController::isCurrentGeneration(quint64 generation) const
{
    return generation == m_generation.loadAcquire();
}

void SpeechSynthesisController::retireCallbackContext()
{
    if (!m_callbackContext) {
        return;
    }

    m_callbackContext->controller = nullptr;
    m_retiredCallbackContexts.append(m_callbackContext);
    m_callbackContext.reset();
}
