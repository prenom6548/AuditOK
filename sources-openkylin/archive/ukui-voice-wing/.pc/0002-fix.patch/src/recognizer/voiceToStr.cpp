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

// 麒麟 AI SDK 头文件（需要根据实际安装路径调整）
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <coreai/speech/recognizer.h>

// 订阅配置变化
#include "configmonitor.h"

VoiceToStr *VoiceToStr::instance()
{
    static VoiceToStr instance;
    return &instance;
}

VoiceToStr::VoiceToStr(QObject *parent)
    : QObject(parent)
{
    // 订阅 ConfigMonitor：enabled true 时初始化 AI，会话被禁用时清理会话
    ConfigMonitor *cfg = ConfigMonitor::instance();
    if (cfg && cfg->enabled()) {
        initAIConfig();
    }
    if (cfg) {
        connect(cfg, &ConfigMonitor::enabledChanged, this, [this](bool enabled){
            if (enabled) {
                initAIConfig();
            } else {
                cleanupSession();
            }
        }, Qt::UniqueConnection);
    }
}

void VoiceToStr::initAIConfig()
{
    // 如果已经有会话则无需重复初始化
    if (m_session) return;

    // 1. 创建会话
    m_session = speech_recognizer_create_session();
    if (!m_session) {
        const QString msg = QStringLiteral("Failed to create speech recognition session");
        VW_LOG(Logger::Level::Error) << msg;
        emit errorOccurred(-1, msg);
        return;
    }

    // 2. 创建模型配置
    m_modelConfig = speech_model_config_create();
    if (!m_modelConfig) {
        const QString msg = QStringLiteral("Failed to create model config");
        VW_LOG(Logger::Level::Error) << msg;
        cleanupSession();
        emit errorOccurred(-2, msg);
        return;
    }
    speech_model_config_set_deploy_type(m_modelConfig, ModelDeployType::OnDevice);

    // 3. 创建音频配置：从默认麦克风流式输入
    m_audioConfig = audio_config_create_continuous_audio_input_from_default_microphone();
    if (!m_audioConfig) {
        const QString msg = QStringLiteral("Failed to create audio config from default microphone");
        VW_LOG(Logger::Level::Error) << msg;
        cleanupSession();
        emit errorOccurred(-3, msg);
        return;
    }
    audio_config_set_input_audio_rate(m_audioConfig, 16000);

    // 4. 将配置应用到会话
    speech_recognizer_set_model_config(m_session, m_modelConfig);
    speech_recognizer_set_audio_config(m_session, m_audioConfig);

    // 5. 设置结果回调
    speech_recognizer_result_set_callback(m_session, onRecognitionResult, this);

    // 7. 初始化会话（同步，通常很快）
    int initRet = speech_recognizer_init_session(m_session);
    if (initRet != 0) {
        const QString msg = QStringLiteral("Init session failed with code %1").arg(initRet);
        VW_LOG(Logger::Level::Error) << msg;
        emit errorOccurred(initRet, msg);
        cleanupSession();
        return;
    }
}

VoiceToStr::~VoiceToStr()
{
    cleanupSession();
}

void VoiceToStr::cleanupSession()
{
    if (m_session) {
        // 如果还在识别中，尝试停止
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

void VoiceToStr::startRecognition()
{
    if (!m_session) {
        initAIConfig();
        if (!m_session) {
            VW_LOG(Logger::Level::Error) << "failed to initialize AI config, cannot start recognition";
            return;
        }
    }

    if (m_isRecognizing) {
        VW_LOG(Logger::Level::Warning) << "already recognizing, stop first";
        stopRecognition();
    }

    // 8. 开始异步流式识别
    speech_recognizer_start_continuous_recognition_async(m_session);
    m_isRecognizing = true;
    VW_LOG(Logger::Level::Debug) << "recognition started";
}

void VoiceToStr::stopRecognition()
{
    if (!m_isRecognizing || !m_session) {
        return;
    }

    // 停止流式识别（异步，无需等待）
    speech_recognizer_stop_continuous_recognition_async(m_session);
    m_isRecognizing = false;
}

// 静态回调：运行在 SDK 内部线程
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
        // 发生错误时自动停止识别
        if (m_isRecognizing) {
            stopRecognition();
        }
        return;
    }

    SpeechResultReason reason = speech_recognition_result_get_reason(result);
    const char *text = speech_recognition_result_get_text(result);
    QString qtext = QString::fromUtf8(text ? text : "");

    // 根据状态发送文本
    switch (reason) {
    case SPEECH_RECOGNITION_STARTED:
        Q_EMIT voiceInputStarted();
        break;

    case SPEECH_RECOGNIZING:
        // 中间结果，实时输出
        if (!qtext.isEmpty())
            Q_EMIT textRecognizing(qtext);
        break;
    case SPEECH_RECOGNIZED:
        // 最终结果（经过修正）
        if (!qtext.isEmpty())
            Q_EMIT textRecognized(qtext);
        break;
    case SPEECH_RECOGNITION_COMPLETED:
        // 识别自然结束（通常因为静音或语音结束）
        VW_LOG(Logger::Level::Debug) << "recognition completed naturally";
        if (m_isRecognizing) {
            Q_EMIT voiceInputPaused(); // 可以在 UI 上显示“识别完成”状态
        }
        break;
    case SPEECH_ERROR_OCCURRED:
        // 已经通过上面的 errorCode 处理，这里做二次清理
        if (m_isRecognizing)
            stopRecognition();
        break;
    default:
        break;
    }
}

