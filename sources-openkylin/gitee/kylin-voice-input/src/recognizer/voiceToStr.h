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

#ifndef VOICETOSTR_H
#define VOICETOSTR_H

#include <QObject>
#include <QString>

// 前置声明 SDK 类型 (避免包含头文件增加编译依赖)
struct _SpeechRecognitionSession;
typedef struct _SpeechRecognitionSession SpeechRecognitionSession;
struct _SpeechModelConfig;
typedef struct _SpeechModelConfig SpeechModelConfig;
struct _AudioConfig;
typedef struct _AudioConfig AudioConfig;

class VoiceToStr : public QObject
{
    Q_OBJECT
public:
    static VoiceToStr* instance();

    ~VoiceToStr();

    /**
     * 初始化并连接 AI 语音识别模块（每次触发都应调用）。
     * 若已有会话会先清理再重建；失败返回 false。
     */
    bool initializeAndConnect();

    // 开始识别：会话就绪后开始流式识别；失败返回 false
    bool startRecognition();
    // 停止识别：停止流式识别
    void stopRecognition();
    // 断开连接并清理初始化（释放快捷键时调用）
    void disconnectAndCleanup();

signals:
    // 识别开始（可以在 UI 上显示“正在识别”状态）
    void voiceInputStarted();

    // 实时识别到的文本（中间结果）
    void textRecognizing(const QString &text);

    // 实时识别到的文本（最终结果）
    void textRecognized(const QString &text);

    // 识别自然结束（如静音或语音结束）
    void voiceInputPaused();

    // 识别出错（错误码 + 错误描述）
    void errorOccurred(int code, const QString &message);

private:
    explicit VoiceToStr(QObject *parent = nullptr);

    /** 创建并初始化识别会话；成功返回 true */
    bool initAIConfig();

    // SDK 回调（静态函数）
    static void onRecognitionResult(struct _SpeechRecognitionResult *result, void *userData);
    void handleRecognitionResult(struct _SpeechRecognitionResult *result);

    // 清理当前会话
    void cleanupSession();

    SpeechRecognitionSession *m_session = nullptr;
    SpeechModelConfig       *m_modelConfig = nullptr;
    AudioConfig             *m_audioConfig = nullptr;

    bool m_isRecognizing = false;   // 是否正在识别中
};

#endif // VOICETOSTR_H
