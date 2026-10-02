/*
 * Copyright 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gio/gio.h>
#include <gtest/gtest.h>

#include <atomic>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "coreai/speech/audioconfig.h"
#include "coreai/speech/error.h"
#include "coreai/speech/recognizer.h"
#include "coreai/speech/result.h"

class SpeechRecognizerWithRequestIdTest : public ::testing::Test {
protected:
    void SetUp() override {
        mainLoop_ = g_main_loop_new(nullptr, FALSE);
        loopThread_ = std::thread([this] { g_main_loop_run(mainLoop_); });

        session_ = speech_recognizer_create_session();
        ASSERT_NE(session_, nullptr);

        int ret = speech_recognizer_init_session(session_);
        ASSERT_EQ(ret, AISDK_NO_ERROR);
    }

    void TearDown() override {
        if (session_) {
            speech_recognizer_destroy_session(&session_);
        }
        if (audioConfig_) {
            audio_config_destroy(&audioConfig_);
        }

        if (mainLoop_) {
            g_main_loop_quit(mainLoop_);
        }
        if (loopThread_.joinable()) {
            loopThread_.join();
        }
        if (mainLoop_) {
            g_main_loop_unref(mainLoop_);
            mainLoop_ = nullptr;
        }
    }

    std::vector<uint8_t> generateTestAudioData(size_t samples = 16000) {
        std::vector<uint8_t> audioData(samples * 2);
        for (size_t i = 0; i < samples; ++i) {
            int16_t sample = static_cast<int16_t>(32767 * std::sin(2 * M_PI * 440 * i / 16000));
            audioData[i * 2] = sample & 0xFF;
            audioData[i * 2 + 1] = (sample >> 8) & 0xFF;
        }
        return audioData;
    }

    SpeechRecognitionSession *session_{nullptr};
    AudioConfig *audioConfig_{nullptr};
    GMainLoop *mainLoop_{nullptr};
    std::thread loopThread_;
};

struct RecognitionCallbackContext {
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> started{false};
    std::atomic<bool> recognizing{false};
    std::atomic<bool> recognized{false};
    std::atomic<bool> completed{false};
    std::atomic<bool> errorOccurred{false};
    std::atomic<bool> tooManyRequestsError{false};
    std::string requestId;
    std::string recognizedText;
    std::atomic<int> errorCode{0};
    std::atomic<int> requestCount{0};
};

static void recognitionCallback(SpeechRecognitionResult *result, void *userData) {
    auto *ctx = static_cast<RecognitionCallbackContext *>(userData);
    std::lock_guard<std::mutex> lock(ctx->mtx);

    const char *reqId = speech_recognition_result_get_request_id(result);
    const char *text = speech_recognition_result_get_text(result);
    int errorCode = speech_recognition_result_get_error_code(result);
    printf("Request ID: %s, Error Code: %d\n", reqId ? reqId : "null", errorCode);

    if (reqId) {
        ctx->requestId = reqId;
    }

    if (text) {
        ctx->recognizedText = text;
        std::fprintf(stdout, "Recognized text: %s with request ID: %s\n", text, reqId ? reqId : "null");
    }

    SpeechResultReason reason = speech_recognition_result_get_reason(result);
    switch (reason) {
        case SPEECH_RECOGNITION_STARTED:
            ctx->started = true;
            break;
        case SPEECH_RECOGNIZING:
            ctx->recognizing = true;
            break;
        case SPEECH_RECOGNIZED:
            ctx->recognized = true;
            break;
        case SPEECH_RECOGNITION_COMPLETED: {
            ctx->completed = true;
            ctx->requestCount++;
            ctx->cv.notify_all();
            break;
        }
        case SPEECH_ERROR_OCCURRED:
            ctx->errorOccurred = true;
            ctx->errorCode = speech_recognition_result_get_error_code(result);
            if (ctx->errorCode == AI_COMMON_TOO_MANY_REQUESTS) {
                ctx->tooManyRequestsError = true;
            }
            ctx->requestCount++;
            std::fprintf(stdout, "Request count: %d, Error code: %d\n", ctx->requestCount.load(),
                         ctx->errorCode.load());
            ctx->cv.notify_all();
            break;
        default:
            break;
    }
}

TEST_F(SpeechRecognizerWithRequestIdTest, NullSessionReturnsError) {
    const char *requestId = "test-request-id-001";

    int ret = speech_recognizer_recognize_once_async_with_request_id(nullptr, requestId);

    EXPECT_EQ(ret, AI_COMMON_INVALID_SESSION);
}

TEST_F(SpeechRecognizerWithRequestIdTest, RecognizeWithRequestIdSuccess) {
    RecognitionCallbackContext ctx;
    speech_recognizer_result_set_callback(session_, recognitionCallback, &ctx);

    auto audioData = generateTestAudioData(16000);
    audioConfig_ = audio_config_create_once_audio_input_from_pcm_data(audioData.data(), audioData.size());
    ASSERT_NE(audioConfig_, nullptr);
    speech_recognizer_set_audio_config(session_, audioConfig_);

    const char *requestId = "test-request-id-002";

    int ret = speech_recognizer_recognize_once_async_with_request_id(session_, requestId);

    EXPECT_EQ(ret, AISDK_NO_ERROR);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    bool finished = ctx.cv.wait_for(lock, std::chrono::seconds(30),
                                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_TRUE(finished) << "Recognition did not complete within timeout";
    EXPECT_TRUE(ctx.completed.load() || ctx.errorOccurred.load());

    if (ctx.completed.load()) {
        EXPECT_EQ(ctx.requestId, requestId);
    }
}

TEST_F(SpeechRecognizerWithRequestIdTest, RequestIdPassedToCallback) {
    RecognitionCallbackContext ctx;
    speech_recognizer_result_set_callback(session_, recognitionCallback, &ctx);

    auto audioData = generateTestAudioData(16000);
    audioConfig_ = audio_config_create_once_audio_input_from_pcm_data(audioData.data(), audioData.size());
    ASSERT_NE(audioConfig_, nullptr);
    speech_recognizer_set_audio_config(session_, audioConfig_);

    const std::string uniqueRequestId = "unique-request-id-12345";

    int ret = speech_recognizer_recognize_once_async_with_request_id(session_, uniqueRequestId.c_str());

    EXPECT_EQ(ret, AISDK_NO_ERROR);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    ctx.cv.wait_for(lock, std::chrono::seconds(30),
                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_EQ(ctx.requestId, uniqueRequestId);
}

TEST_F(SpeechRecognizerWithRequestIdTest, EmptyAudioRecognition) {
    RecognitionCallbackContext ctx;
    speech_recognizer_result_set_callback(session_, recognitionCallback, &ctx);

    std::vector<uint8_t> emptyAudio;
    audioConfig_ = audio_config_create_once_audio_input_from_pcm_data(emptyAudio.data(), 0);
    speech_recognizer_set_audio_config(session_, audioConfig_);

    const char *requestId = "test-request-id-empty";

    int ret = speech_recognizer_recognize_once_async_with_request_id(session_, requestId);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    ctx.cv.wait_for(lock, std::chrono::seconds(10),
                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_EQ(ctx.requestId, requestId);
    EXPECT_EQ(ctx.errorCode, AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID);
}

TEST_F(SpeechRecognizerWithRequestIdTest, LongAudioRecognition) {
    RecognitionCallbackContext ctx;
    speech_recognizer_result_set_callback(session_, recognitionCallback, &ctx);

    auto audioData = generateTestAudioData(16000 * 10);
    audioConfig_ = audio_config_create_once_audio_input_from_pcm_data(audioData.data(), audioData.size());
    ASSERT_NE(audioConfig_, nullptr);
    speech_recognizer_set_audio_config(session_, audioConfig_);

    const char *requestId = "test-request-id-long";

    int ret = speech_recognizer_recognize_once_async_with_request_id(session_, requestId);

    EXPECT_EQ(ret, AISDK_NO_ERROR);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    bool finished = ctx.cv.wait_for(lock, std::chrono::seconds(60),
                                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_TRUE(finished) << "Long audio recognition did not complete within timeout";

    EXPECT_EQ(ctx.requestId, requestId);
}

TEST_F(SpeechRecognizerWithRequestIdTest, TooManyRequestsReturnsError) {
    RecognitionCallbackContext ctx;
    speech_recognizer_result_set_callback(session_, recognitionCallback, &ctx);

    auto audioData = generateTestAudioData(16000);

    for (int i = 0; i < 15; ++i) {
        AudioConfig *config = audio_config_create_once_audio_input_from_pcm_data(audioData.data(), audioData.size());
        speech_recognizer_set_audio_config(session_, config);

        std::string requestId = "test-request-id-" + std::to_string(i + 1);
        int ret = speech_recognizer_recognize_once_async_with_request_id(session_, requestId.c_str());

        printf("Request %d returned: %d\n", i + 1, ret);
        EXPECT_EQ(ret, AISDK_NO_ERROR);

        if (i < 14) {
            audio_config_destroy(&config);
        } else {
            audioConfig_ = config;
        }
    }

    std::unique_lock<std::mutex> lock(ctx.mtx);
    bool finished = ctx.cv.wait_for(lock, std::chrono::seconds(30),
                                    [&ctx] { return ctx.tooManyRequestsError.load() && ctx.requestCount >= 15; });

    EXPECT_TRUE(finished) << "At least one request should return AI_COMMON_TOO_MANY_REQUESTS "
                             "via callback";
    EXPECT_TRUE(ctx.tooManyRequestsError.load());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
