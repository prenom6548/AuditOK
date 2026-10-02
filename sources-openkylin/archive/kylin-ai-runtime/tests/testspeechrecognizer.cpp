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
#include "services/coreai/speech/speechrecognizer.h"
#include "testmodulescommon.h"

TEST(SpeechRecognizerTest, TimeoutTriggersError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceSpeechRecognition", 1), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 1500, false, -1);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int errorCode = 0;
    ASSERT_TRUE(recognizer->init(errorCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    recognizer->setRecognitionResultCallback([&](SpeechRecognitionResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    recognizer->recognizeOnce();
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechRecognizerTest, QueueFullRejectsAllRequests) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceSpeechRecognition", 5), true);
    ScopedPendingLimits limits(0, 0);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, false, -1);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int errorCode = 0;
    ASSERT_TRUE(recognizer->init(errorCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    recognizer->setRecognitionResultCallback([&](SpeechRecognitionResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        if (state.finished >= 2) {
            state.cv.notify_all();
        }
    });

    recognizer->recognizeOnce();
    recognizer->recognizeOnce();

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 2);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
}

TEST(SpeechRecognizerTest, EngineFailureReturnsError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceSpeechRecognition", 5), true);
    int errorCode = (int)ai_engine::SpeechEngineErrorCode::NomoreMemory;
    auto engine = std::make_shared<FakeSpeechEngine>(false, true, 0, false, errorCode);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int initCode = 0;
    ASSERT_TRUE(recognizer->init(initCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    recognizer->setRecognitionResultCallback([&](SpeechRecognitionResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    recognizer->recognizeOnce();
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR);
}

TEST(SpeechRecognizerTest, ConfigInvalidFallsBack) {
    ScopedTaskConfigOverride config("not-json", true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int errorCode = 0;
    ASSERT_TRUE(recognizer->init(errorCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    recognizer->setRecognitionResultCallback([&](SpeechRecognitionResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    recognizer->recognizeOnce();
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechRecognizerTest, ConfigMissingUsesDefault) {
    ScopedTaskConfigOverride config("", false);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int errorCode = 0;
    ASSERT_TRUE(recognizer->init(errorCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    recognizer->setRecognitionResultCallback([&](SpeechRecognitionResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    recognizer->recognizeOnce();
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechRecognizerTest, ConfigZeroFallsBack) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceSpeechRecognition", 0), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, true, -1);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int errorCode = 0;
    ASSERT_TRUE(recognizer->init(errorCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));

    CallbackState state;
    recognizer->setRecognitionResultCallback([&](SpeechRecognitionResult result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    recognizer->recognizeOnce();
    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SpeechRecognizerTest, CallbackNullDoesNotCrash) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("OnceSpeechRecognition", 5), true);
    auto engine = std::make_shared<FakeSpeechEngine>(true, true, 0, false, -1);
    auto recognizer = std::make_shared<SpeechRecognizer>();
    int errorCode = 0;
    ASSERT_TRUE(recognizer->init(errorCode, engine));
    recognizer->setAudioConfig(AudioConfigInfo(makeRawAudioConfigJson()));
    recognizer->setRecognitionResultCallback(nullptr);
    recognizer->recognizeOnce();
    task::TaskRunner::getInstance().waitForTasks();
}
