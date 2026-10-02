/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "voiceToStr.h"
#include "logger.h"

#include <coreai/speech/recognizer.h>

#include "configmonitor.h"

VoiceToStr *VoiceToStr::instance()
{
    static VoiceToStr instance;
    return &instance;
}

VoiceToStr::VoiceToStr(QObject *parent)
    : QObject(parent)
{
    // 功能关闭时清理会话；初始化/连接仅在每次触发语音输入时进行
    ConfigMonitor *cfg = ConfigMonitor::instance();
    if (cfg) {
        connect(cfg, &ConfigMonitor::enabledChanged, this, [this](bool enabled){
            if (!enabled) {
                disconnectAndCleanup();
            }
        }, Qt::UniqueConnection);
    }
}

bool VoiceToStr::initializeAndConnect()
{
    // 每次触发都重新初始化并连接，避免复用过期会话
    cleanupSession();
    return initAIConfig();
}

bool VoiceToStr::initAIConfig()
{
    // 1. 创建会话
    m_session = speech_recognizer_create_session();
    if (!m_session) {
        const QString msg = QStringLiteral("Failed to create speech recognition session");
        VW_LOG(Logger::Level::Error) << msg;
        emit errorOccurred(-1, msg);
        return false;
    }

    // 2. 创建模型配置
    m_modelConfig = speech_model_config_create();
    if (!m_modelConfig) {
        const QString msg = QStringLiteral("Failed to create model config");
        VW_LOG(Logger::Level::Error) << msg;
        cleanupSession();
        emit errorOccurred(-2, msg);
        return false;
    }
    speech_model_config_set_deploy_type(m_modelConfig, ModelDeployType::OnDevice);

    // 3. 创建音频配置：从默认麦克风流式输入
    m_audioConfig = audio_config_create_continuous_audio_input_from_default_microphone();
    if (!m_audioConfig) {
        const QString msg = QStringLiteral("Failed to create audio config from default microphone");
        VW_LOG(Logger::Level::Error) << msg;
        cleanupSession();
        emit errorOccurred(-3, msg);
        return false;
    }
    audio_config_set_input_audio_rate(m_audioConfig, 16000);

    // 4. 将配置应用到会话
    speech_recognizer_set_model_config(m_session, m_modelConfig);
    speech_recognizer_set_audio_config(m_session, m_audioConfig);

    // 5. 设置结果回调
    speech_recognizer_result_set_callback(m_session, onRecognitionResult, this);

    // 6. 初始化会话并连接 AI 运行时（同步）
    int initRet = speech_recognizer_init_session(m_session);
    if (initRet != 0) {
        const QString msg = QStringLiteral("Init/connect AI session failed with code %1").arg(initRet);
        VW_LOG(Logger::Level::Error) << msg;
        emit errorOccurred(initRet, msg);
        cleanupSession();
        return false;
    }

    VW_LOG(Logger::Level::Debug) << "AI speech session initialized and connected";
    return true;
}

VoiceToStr::~VoiceToStr()
{
    disconnectAndCleanup();
}

void VoiceToStr::cleanupSession()
{
    if (m_session) {
        if (m_isRecognizing) {
            speech_recognizer_stop_continuous_recognition_async(m_session);
            m_isRecognizing = false;
        }
        speech_recognizer_destroy_session(&m_session);
    }
    if (m_modelConfig) {
        speech_model_config_destroy(&m_modelConfig);
    }
    if (m_audioConfig) {
        audio_config_destroy(&m_audioConfig);
    }
}

bool VoiceToStr::startRecognition()
{
    if (!m_session) {
        VW_LOG(Logger::Level::Error) << "AI session not initialized, cannot start recognition";
        return false;
    }

    if (m_isRecognizing) {
        VW_LOG(Logger::Level::Warning) << "already recognizing, stop continuous recognition first";
        speech_recognizer_stop_continuous_recognition_async(m_session);
        m_isRecognizing = false;
    }

    speech_recognizer_start_continuous_recognition_async(m_session);
    m_isRecognizing = true;
    VW_LOG(Logger::Level::Debug) << "recognition started";
    return true;
}

void VoiceToStr::stopRecognition()
{
    if (m_isRecognizing && m_session) {
        speech_recognizer_stop_continuous_recognition_async(m_session);
        m_isRecognizing = false;
    }
}

void VoiceToStr::disconnectAndCleanup()
{
    stopRecognition();
    cleanupSession();
    VW_LOG(Logger::Level::Debug) << "AI speech session disconnected and cleaned up";
}

void VoiceToStr::onRecognitionResult(SpeechRecognitionResult *result, void *userData)
{
    VoiceToStr *self = static_cast<VoiceToStr*>(userData);
    if (!self) return;
    self->handleRecognitionResult(result);
}

void VoiceToStr::handleRecognitionResult(SpeechRecognitionResult *result)
{
    if (!result) return;

    int errorCode = speech_recognition_result_get_error_code(result);
    if (errorCode != 0) {
        const char *errMsg = speech_recognition_result_get_error_message(result);
        const QString msg = QString::fromUtf8(errMsg ? errMsg : "Unknown error");
        VW_LOG(Logger::Level::Error) << "recognition error" << errorCode << ":" << msg;
        emit errorOccurred(errorCode, msg);
        if (m_isRecognizing) {
            stopRecognition();
        }
        return;
    }

    SpeechResultReason reason = speech_recognition_result_get_reason(result);
    const char *text = speech_recognition_result_get_text(result);
    QString qtext = QString::fromUtf8(text ? text : "");

    switch (reason) {
    case SPEECH_RECOGNITION_STARTED:
        Q_EMIT voiceInputStarted();
        break;

    case SPEECH_RECOGNIZING:
        if (!qtext.isEmpty())
            Q_EMIT textRecognizing(qtext);
        break;
    case SPEECH_RECOGNIZED:
        if (!qtext.isEmpty())
            Q_EMIT textRecognized(qtext);
        break;
    case SPEECH_RECOGNITION_COMPLETED:
        VW_LOG(Logger::Level::Debug) << "recognition completed naturally";
        if (m_isRecognizing) {
            Q_EMIT voiceInputPaused();
        }
        break;
    case SPEECH_ERROR_OCCURRED:
        if (m_isRecognizing)
            stopRecognition();
        break;
    default:
        break;
    }
}
