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
#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

#include "coreai/speech/audioconfig.h"
#include "coreai/speech/error.h"
#include "coreai/speech/result.h"
#include "coreai/speech/synthesizer.h"

class SpeechSynthesizerWithRequestIdTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建并启动 GLib 主循环
        mainLoop_ = g_main_loop_new(nullptr, FALSE);
        loopThread_ = std::thread([this] { g_main_loop_run(mainLoop_); });

        session_ = speech_synthesizer_create_session();
        ASSERT_NE(session_, nullptr);

        int ret = speech_synthesizer_init_session(session_);
        ASSERT_EQ(ret, AISDK_NO_ERROR);

        audioConfig_ = audio_config_create_audio_output_from_pcm_data();
        ASSERT_NE(audioConfig_, nullptr);

        speech_synthesizer_set_audio_config(session_, audioConfig_);
    }

    void TearDown() override {
        if (session_) {
            speech_synthesizer_destroy_session(&session_);
        }
        if (audioConfig_) {
            audio_config_destroy(&audioConfig_);
        }

        // 停止 GLib 主循环
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

    SpeechSynthesizerSession *session_{nullptr};
    AudioConfig *audioConfig_{nullptr};
    GMainLoop *mainLoop_{nullptr};
    std::thread loopThread_;
};

struct CallbackContext {
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> started{false};
    std::atomic<bool> synthesizing{false};
    std::atomic<bool> completed{false};
    std::atomic<bool> errorOccurred{false};
    std::atomic<bool> tooManyRequestsError{false};
    std::string requestId;
    std::atomic<int> errorCode{0};
    std::string outputFilePath;
    int requestCount{0};
};

static void synthesisCallback(SpeechSynthesisResult *result, void *userData) {
    auto *ctx = static_cast<CallbackContext *>(userData);
    std::lock_guard<std::mutex> lock(ctx->mtx);

    const char *reqId = speech_synthesis_result_get_request_id(result);
    printf("Request ID: %s, Error Code: %d\n", reqId, ctx->errorCode.load());
    uint32_t dataLength = 0;
    const uint8_t *audioData = speech_synthesis_result_get_data(result, &dataLength);
    std::fprintf(stdout, "Data length: %d\n", dataLength);

    if (dataLength > 0 && audioData != nullptr && !ctx->outputFilePath.empty()) {
        std::ofstream outFile(ctx->outputFilePath, std::ios::binary | std::ios::app);
        if (outFile.is_open()) {
            outFile.write(reinterpret_cast<const char *>(audioData), dataLength);
            outFile.close();
        }
    }

    if (reqId) {
        ctx->requestId = reqId;
    }

    SpeechResultReason reason = speech_synthesis_result_get_reason(result);
    switch (reason) {
        case SPEECH_SYNTHESIS_STARTED:
            ctx->started = true;
            break;
        case SPEECH_SYNTHESIZING:
            ctx->synthesizing = true;

            break;
        case SPEECH_SYNTHESIS_COMPLETED: {
            ctx->completed = true;
            ctx->requestCount++;
            ctx->cv.notify_all();
            break;
        }

        case SPEECH_ERROR_OCCURRED:
            ctx->errorOccurred = true;
            ctx->errorCode = speech_synthesis_result_get_error_code(result);
            if (ctx->errorCode == AI_COMMON_TOO_MANY_REQUESTS) {
                ctx->tooManyRequestsError = true;
            }
            ctx->requestCount++;
            std::fprintf(stdout, "Request count: %d\n", ctx->requestCount);
            ctx->cv.notify_all();
            break;
        default:
            break;
    }
}

TEST_F(SpeechSynthesizerWithRequestIdTest, NullSessionReturnsError) {
    const char *requestId = "test-request-id-001";
    const char *text = "测试文本";
    uint32_t textLength = strlen(text);

    int ret = speech_synthesizer_synthesize_text_async_with_request_id(nullptr, requestId, text, textLength);

    EXPECT_EQ(ret, AI_COMMON_INVALID_SESSION);
}

TEST_F(SpeechSynthesizerWithRequestIdTest, SynthesizeWithRequestIdSuccess) {
    CallbackContext ctx;
    ctx.outputFilePath = "/tmp/synthesis_with_request_id.pcm";
    std::remove(ctx.outputFilePath.c_str());
    speech_synthesizer_result_set_callback(session_, synthesisCallback, &ctx);

    const char *requestId = "test-request-id-002";
    const char *text = "你好，这是一段测试文本。";
    uint32_t textLength = strlen(text);

    int ret = speech_synthesizer_synthesize_text_async_with_request_id(session_, requestId, text, textLength);

    EXPECT_EQ(ret, AISDK_NO_ERROR);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    bool finished = ctx.cv.wait_for(lock, std::chrono::seconds(30),
                                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_TRUE(finished) << "Synthesis did not complete within timeout";
    EXPECT_TRUE(ctx.completed.load() || ctx.errorOccurred.load());

    if (ctx.completed.load()) {
        EXPECT_EQ(ctx.requestId, requestId);
    }
}

TEST_F(SpeechSynthesizerWithRequestIdTest, RequestIdPassedToCallback) {
    CallbackContext ctx;
    ctx.outputFilePath = "/tmp/synthesis_request_id_passed.pcm";
    std::remove(ctx.outputFilePath.c_str());
    speech_synthesizer_result_set_callback(session_, synthesisCallback, &ctx);

    const std::string uniqueRequestId = "unique-request-id-12345";
    const char *text = "测试请求ID传递";
    uint32_t textLength = strlen(text);

    int ret =
        speech_synthesizer_synthesize_text_async_with_request_id(session_, uniqueRequestId.c_str(), text, textLength);

    EXPECT_EQ(ret, AISDK_NO_ERROR);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    ctx.cv.wait_for(lock, std::chrono::seconds(30),
                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_EQ(ctx.requestId, uniqueRequestId);
}

TEST_F(SpeechSynthesizerWithRequestIdTest, EmptyTextSynthesis) {
    CallbackContext ctx;
    speech_synthesizer_result_set_callback(session_, synthesisCallback, &ctx);

    const char *requestId = "test-request-id-empty";
    const char *text = "";
    uint32_t textLength = 0;

    int ret = speech_synthesizer_synthesize_text_async_with_request_id(session_, requestId, text, textLength);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    ctx.cv.wait_for(lock, std::chrono::seconds(10),
                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });
}

TEST_F(SpeechSynthesizerWithRequestIdTest, LongTextSynthesis) {
    CallbackContext ctx;
    ctx.outputFilePath = "/tmp/synthesis_long_text.pcm";
    std::remove(ctx.outputFilePath.c_str());
    speech_synthesizer_result_set_callback(session_, synthesisCallback, &ctx);

    const char *requestId = "test-request-id-long";
    std::string longText;
    for (int i = 0; i < 10; ++i) {
        longText += "这是一段较长的测试文本，用于测试语音合成接口的处理能力。";
    }

    int ret = speech_synthesizer_synthesize_text_async_with_request_id(session_, requestId, longText.c_str(),
                                                                       longText.size());

    EXPECT_EQ(ret, AISDK_NO_ERROR);

    std::unique_lock<std::mutex> lock(ctx.mtx);
    bool finished = ctx.cv.wait_for(lock, std::chrono::seconds(60),
                                    [&ctx] { return ctx.completed.load() || ctx.errorOccurred.load(); });

    EXPECT_TRUE(finished) << "Long text synthesis did not complete withintimeout";
}

TEST_F(SpeechSynthesizerWithRequestIdTest, TooManyRequestsReturnsError) {
    CallbackContext ctx;
    ctx.outputFilePath = "/tmp/synthesis_too_many_requests.pcm";
    std::remove(ctx.outputFilePath.c_str());
    speech_synthesizer_result_set_callback(session_, synthesisCallback, &ctx);

    const char *text = "测试请求限流";
    uint32_t textLength = strlen(text);

    for (int i = 0; i < 15; ++i) {
        std::string requestId = "test-request-id-" + std::to_string(i + 1);
        int ret =
            speech_synthesizer_synthesize_text_async_with_request_id(session_, requestId.c_str(), text, textLength);

        printf("Request %d returned: %d\n", i + 1, ret);
        EXPECT_EQ(ret, AISDK_NO_ERROR);
    }

    std::unique_lock<std::mutex> lock(ctx.mtx);
    bool finished = ctx.cv.wait_for(lock, std::chrono::seconds(30),
                                    [&ctx] { return ctx.tooManyRequestsError.load() && ctx.requestCount == 15; });

    EXPECT_TRUE(finished) << "At least one request should return AI_COMMON_TOO_MANY_REQUESTS "
                             "via callback";
    EXPECT_TRUE(ctx.tooManyRequestsError.load());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
