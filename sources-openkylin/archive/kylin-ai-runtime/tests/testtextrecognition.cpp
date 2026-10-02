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

#include <algorithm>

#include "services/coreai/vision/textrecognition.h"
#include "testmodulescommon.h"

TEST(TextRecognitionTest, TimeoutTriggersError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 1), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 1500, false, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    std::string lastRequestId;
    recognizer->recognizeTextFromImageData("data", "req-timeout", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        lastRequestId = result.requestId;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        EXPECT_EQ(lastRequestId, "req-timeout");
    }
}

TEST(TextRecognitionTest, TimeoutTriggersErrorOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 1), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 1500, false, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    std::string lastRequestId;
    recognizer->recognizeTextFromImageFile("dummy-path", "req-timeout-file", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        lastRequestId = result.requestId;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        EXPECT_EQ(lastRequestId, "req-timeout-file");
    }
}

TEST(TextRecognitionTest, QueueFullRejectsAllRequests) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    ScopedPendingLimits limits(0, 0);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, false, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    auto onResult = [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        if (state.finished >= 2) {
            state.cv.notify_all();
        }
    };

    recognizer->recognizeTextFromImageData("data1", onResult);
    recognizer->recognizeTextFromImageData("data2", onResult);

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 2);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
}

TEST(TextRecognitionTest, QueueFullRejectsAllRequestsOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    ScopedPendingLimits limits(0, 0);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, false, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    auto onResult = [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        if (state.finished >= 2) {
            state.cv.notify_all();
        }
    };

    recognizer->recognizeTextFromImageFile("path1", onResult);
    recognizer->recognizeTextFromImageFile("path2", onResult);

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 2);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
}

TEST(TextRecognitionTest, EngineFailureReturnsError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    int errorCode = (int)ai_engine::VisionEngineErrorCode::FileNotFound;
    auto engine = std::make_unique<FakeTextRecognitionEngine>(false, 0, false, errorCode);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageData("data", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR);
}

TEST(TextRecognitionTest, EngineFailureReturnsErrorOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    int errorCode = (int)ai_engine::VisionEngineErrorCode::FileNotFound;
    auto engine = std::make_unique<FakeTextRecognitionEngine>(false, 0, false, errorCode);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageFile("dummy-path", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_SERVICE_ERROR);
}

TEST(TextRecognitionTest, RequestIdPropagatesOnSuccess) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    std::string lastRequestId;
    recognizer->recognizeTextFromImageData("data", "req-success", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        lastRequestId = result.requestId;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        EXPECT_EQ(lastRequestId, "req-success");
    }
}

TEST(TextRecognitionTest, RequestIdPropagatesOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    std::string lastRequestId;
    recognizer->recognizeTextFromImageFile("dummy-path", "req-file", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        lastRequestId = result.requestId;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        EXPECT_EQ(lastRequestId, "req-file");
    }
}

TEST(TextRecognitionTest, MultipleRequestsPreserveRequestId) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    std::vector<std::string> requestIds;
    auto onResult = [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.finished += 1;
        requestIds.push_back(result.requestId);
        if (state.finished >= 3) {
            state.cv.notify_all();
        }
    };

    recognizer->recognizeTextFromImageData("data1", "req-1", onResult);
    recognizer->recognizeTextFromImageData("data2", "req-2", onResult);
    recognizer->recognizeTextFromImageData("data3", "req-3", onResult);

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 3; }, std::chrono::milliseconds(1500)));
    EXPECT_EQ(state.results, 3);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        std::sort(requestIds.begin(), requestIds.end());
        EXPECT_EQ(requestIds, (std::vector<std::string>{"req-1", "req-2", "req-3"}));
    }
}

TEST(TextRecognitionTest, MultipleRequestsPreserveRequestIdOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    std::vector<std::string> requestIds;
    auto onResult = [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.finished += 1;
        requestIds.push_back(result.requestId);
        if (state.finished >= 3) {
            state.cv.notify_all();
        }
    };

    recognizer->recognizeTextFromImageFile("path1", "req-1", onResult);
    recognizer->recognizeTextFromImageFile("path2", "req-2", onResult);
    recognizer->recognizeTextFromImageFile("path3", "req-3", onResult);

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished >= 3; }, std::chrono::milliseconds(1500)));
    EXPECT_EQ(state.results, 3);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        std::sort(requestIds.begin(), requestIds.end());
        EXPECT_EQ(requestIds, (std::vector<std::string>{"req-1", "req-2", "req-3"}));
    }
}

TEST(TextRecognitionTest, ConfigInvalidFallsBack) {
    ScopedTaskConfigOverride config("not-json", true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageData("data", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(TextRecognitionTest, ConfigInvalidFallsBackOnImageFile) {
    ScopedTaskConfigOverride config("not-json", true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageFile("dummy-path", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(TextRecognitionTest, ConfigMissingUsesDefault) {
    ScopedTaskConfigOverride config("", false);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageData("data", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(TextRecognitionTest, ConfigMissingUsesDefaultOnImageFile) {
    ScopedTaskConfigOverride config("", false);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageFile("dummy-path", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(TextRecognitionTest, ConfigZeroFallsBack) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 0), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageData("data", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(TextRecognitionTest, ConfigZeroFallsBackOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 0), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());

    CallbackState state;
    recognizer->recognizeTextFromImageFile("dummy-path", [&](const TextRecognitionResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        state.cv.notify_all();
    });

    ASSERT_TRUE(waitFor(
        state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(TextRecognitionTest, CallbackNullDoesNotCrash) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());
    recognizer->recognizeTextFromImageData("data", nullptr);
    task::TaskRunner::getInstance().waitForTasks();
}

TEST(TextRecognitionTest, CallbackNullDoesNotCrashOnImageFile) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextRecognition", 5), true);
    auto engine = std::make_unique<FakeTextRecognitionEngine>(true, 0, true, -1);
    auto recognizer = std::make_shared<TextRecognition>(std::move(engine));
    ASSERT_TRUE(recognizer->init());
    recognizer->recognizeTextFromImageFile("dummy-path", nullptr);
    task::TaskRunner::getInstance().waitForTasks();
}
