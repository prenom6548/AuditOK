/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include <gtest/gtest.h>

#include "services/coreai/speech/audioconfiginfo.h"
#include "services/coreai/speech/speechsynthesizer.h"
#include "testmodulescommon.h"

TEST(SpeechSynthesizerTest, TimeoutTriggersError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceTextToSpeech", 1), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 1500, false, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    synthesizer->synthesizeTextOnce("hello");
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechSynthesizerTest, QueueFullRejectsAllRequests) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceTextToSpeech", 5), true);
    ScopedPendingLimits limits(0, 0);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, false, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        if (state.finished >= 2) {
            state.cv.notify_all();
        }
    });

    synthesizer->synthesizeTextOnce("one");
    synthesizer->synthesizeTextOnce("two");

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 2);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
}

TEST(SpeechSynthesizerTest, EngineFailureReturnsError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceTextToSpeech", 5), true);
    int errorCode = (int)ai_engine::SpeechEngineErrorCode::NomoreMemory;
    auto engine = std::make_shared<FakeSpeechEngine>(true, false, 0, false, errorCode);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int initCode = 0;
    ASSERT_TRUE(synthesizer->init(initCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    synthesizer->synthesizeTextOnce("hello");
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR);
}

TEST(SpeechSynthesizerTest, ConfigInvalidFallsBack) {
    ScopedTaskConfigOverride config("not-json", true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    synthesizer->synthesizeTextOnce("hello");
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechSynthesizerTest, ConfigMissingUsesDefault) {
    ScopedTaskConfigOverride config("", false);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    synthesizer->synthesizeTextOnce("hello");
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechSynthesizerTest, ConfigZeroFallsBack) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceTextToSpeech", 0), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    synthesizer->synthesizeTextOnce("hello");
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechSynthesizerTest, CallbackNullDoesNotCrash) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceTextToSpeech", 5), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, false, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));
    synthesizer->setSynthesisResultCallback(nullptr);
    synthesizer->synthesizeTextOnce("hello");
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(SpeechSynthesizerTest, SynthesizeTextOnceWithRequestIdPropagatesRequestId) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceTextToSpeech", 5), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto synthesizer = std::make_shared<SpeechSynthesizer>();
    int errorCode = 0;
    ASSERT_TRUE(synthesizer->init(errorCode, engine));
    synthesizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    std::string receivedRequestId;
    synthesizer->setSynthesisResultCallback([&](SpeechSynthesisResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        receivedRequestId = result.requestId;
        state.finished += 1;
        state.cv.notify_all();
    });

    const std::string expectedRequestId = "test-request-id-12345";
    synthesizer->synthesizeTextOnce("hello", expectedRequestId);
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, 0);
    EXPECT_EQ(receivedRequestId, expectedRequestId);
}
