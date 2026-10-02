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

#include <set>

#include "services/genai/vision/imagegenerator.h"
#include "testmodulescommon.h"

namespace {

constexpr size_t kImageGenerationRequestQueueMaxSizeForTest = 10;

void waitForTaskDrain() {
    ASSERT_TRUE(task::TaskRunner::getInstance().waitForTasksForTesting(std::chrono::milliseconds(3000)));
}

}  // namespace

TEST(ImageGeneratorTest, TimeoutTriggersError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 1), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(
        true, true, 0, false, (int)ai_engine::VisionEngineErrorCode::ImageGenerationFailed);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::string seenRequestId;
    const std::string requestId = "req-timeout";
    generator->generateImage("prompt",
                             requestId,
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 state.lastError = result.errorCode;
                                 seenRequestId = result.requestId;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    EXPECT_EQ(seenRequestId, requestId);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, QueueFullRejectsAllRequests) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    ScopedPendingLimits limits(0, 0);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, false, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::set<std::string> seenRequestIds;
    auto onResult = [&](const ImageGenerationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        seenRequestIds.insert(result.requestId);
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished += 1;
        if (state.finished >= 2) {
            state.cv.notify_all();
        }
    };

    generator->generateImage("p1", "req-queue-1", onResult, onFinished);
    generator->generateImage("p2", "req-queue-2", onResult, onFinished);

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 2);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
    EXPECT_EQ(seenRequestIds.size(), 2U);
    EXPECT_TRUE(seenRequestIds.count("req-queue-1") > 0);
    EXPECT_TRUE(seenRequestIds.count("req-queue-2") > 0);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, ExceedQueueMaxRejectsExtraRequests) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, true, 0, false, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::set<std::string> rejectedRequestIds;
    auto onResult = [&](const ImageGenerationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (result.errorCode == AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS) {
            rejectedRequestIds.insert(result.requestId);
        }
        state.results += 1;
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished += 1;
        if (rejectedRequestIds.size() >= 2) {
            state.cv.notify_all();
        }
    };

    generator->generateImage("p-block", "req-block", onResult, onFinished);
    const size_t extraRejects = 2;
    for (size_t i = 0; i < kImageGenerationRequestQueueMaxSizeForTest + extraRejects; ++i) {
        generator->generateImage("p", "req-overflow-" + std::to_string(i), onResult, onFinished);
    }

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return rejectedRequestIds.size() >= extraRejects; },
                        std::chrono::milliseconds(1000)));
    EXPECT_EQ(rejectedRequestIds.size(), extraRejects);
    EXPECT_TRUE(rejectedRequestIds.count("req-overflow-10") > 0);
    EXPECT_TRUE(rejectedRequestIds.count("req-overflow-11") > 0);
    generator->stopImageGeneration();
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, EngineFailureReturnsError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    int errorCode = (int)ai_engine::VisionEngineErrorCode::ImageGenerationFailed;
    auto engine = std::make_unique<FakeImageGenerationEngine>(false, false, 0, false, errorCode);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::string seenRequestId;
    const std::string requestId = "req-engine-failed";
    generator->generateImage("prompt",
                             requestId,
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 state.lastError = result.errorCode;
                                 seenRequestId = result.requestId;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, genai::VISION_IMAGE_GENERATION_FAILED);
    EXPECT_EQ(seenRequestId, requestId);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, ConfigInvalidFallsBack) {
    ScopedTaskConfigOverride config("not-json", true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    generator->generateImage("prompt",
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 state.lastError = result.errorCode;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, ConfigMissingUsesDefault) {
    ScopedTaskConfigOverride config("", false);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    generator->generateImage("prompt",
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 state.lastError = result.errorCode;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, ConfigZeroFallsBack) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 0), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    generator->generateImage("prompt",
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 state.lastError = result.errorCode;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, CallbackNullDoesNotCrash) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, false, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));
    generator->generateImage("prompt", nullptr, nullptr);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, RequestIdPropagatesToResult) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::string seenRequestId;
    const std::string requestId = "req-123";
    generator->generateImage("prompt", requestId,
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 seenRequestId = result.requestId;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1 && state.results == 1; },
                        std::chrono::milliseconds(1000)));
    EXPECT_EQ(seenRequestId, requestId);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, TimeoutSuppressesLateCallbacks) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 1), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 1500, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::string seenRequestId;
    const std::string requestId = "req-timeout-late";
    generator->generateImage("prompt",
                             requestId,
                             [&](const ImageGenerationResult& result) {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.results += 1;
                                 state.lastError = result.errorCode;
                                 seenRequestId = result.requestId;
                             },
                             [&]() {
                                 std::lock_guard<std::mutex> lock(state.mutex);
                                 state.finished += 1;
                                 state.cv.notify_all();
                             });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
    EXPECT_EQ(seenRequestId, requestId);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, MultipleRequestsPropagateRequestIds) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 0, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::set<std::string> seenRequestIds;
    auto onResult = [&](const ImageGenerationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        seenRequestIds.insert(result.requestId);
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished += 1;
        if (state.finished >= 3) {
            state.cv.notify_all();
        }
    };

    generator->generateImage("p1", "req-1", onResult, onFinished);
    generator->generateImage("p2", "req-2", onResult, onFinished);
    generator->generateImage("p3", "req-3", onResult, onFinished);

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished >= 3; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 3);
    EXPECT_EQ(seenRequestIds.size(), 3U);
    EXPECT_TRUE(seenRequestIds.count("req-1") > 0);
    EXPECT_TRUE(seenRequestIds.count("req-2") > 0);
    EXPECT_TRUE(seenRequestIds.count("req-3") > 0);
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, StopImageGenerationStopsCurrentAndPending) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, true, 200, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::vector<std::string> stoppedRequestIds;
    
    auto onResult = [&](const ImageGenerationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (result.errorMessage == "stop") {
             stoppedRequestIds.push_back(result.requestId);
        }
        state.results++;
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished++;
        state.cv.notify_all();
    };

    generator->generateImage("p1", "req-1", onResult, onFinished);
    generator->generateImage("p2", "req-2", onResult, onFinished);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    generator->stopImageGeneration();

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    
    // req-1 is interrupted but no "stop" result is sent by stopImageGeneration for current request, only finishedCallback.
    // req-2 is pending and should get "stop" result.
    bool req2Stopped = false;
    for(const auto& id : stoppedRequestIds) { if (id == "req-2") req2Stopped = true; }
    EXPECT_TRUE(req2Stopped);
    
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, StopSpecificPendingRequest) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, false, 200, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    std::vector<std::string> stoppedRequestIds;
    
    auto onResult = [&](const ImageGenerationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (result.errorMessage == "stop") {
             stoppedRequestIds.push_back(result.requestId);
        }
        state.results++;
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished++;
        state.cv.notify_all();
    };

    generator->generateImage("p1", "req-1", onResult, onFinished);
    generator->generateImage("p2", "req-2", onResult, onFinished);
    generator->generateImage("p3", "req-3", onResult, onFinished);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    generator->stopImageGeneration("req-2");

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished >= 3; }, std::chrono::milliseconds(2000)));

    EXPECT_EQ(stoppedRequestIds.size(), 1);
    EXPECT_EQ(stoppedRequestIds[0], "req-2");
    
    waitForTaskDrain();
}

TEST(ImageGeneratorTest, StopSpecificCurrentRequest) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("TextToImage", 5), true);
    auto engine = std::make_unique<FakeImageGenerationEngine>(true, true, 500, true, -1);
    auto generator = std::make_shared<ImageGenerator>(std::move(engine));

    CallbackState state;
    bool req1Stopped = false;
    
    auto onResult = [&](const ImageGenerationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        if (result.requestId == "req-1" && result.errorMessage == "stop") {
             req1Stopped = true;
        }
        state.results++;
    };
    auto onFinished = [&]() {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.finished++;
        state.cv.notify_all();
    };

    generator->generateImage("p1", "req-1", onResult, onFinished);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    generator->stopImageGeneration("req-1");
    
    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished >= 1; }, std::chrono::milliseconds(1000)));
    
    EXPECT_TRUE(req1Stopped);
    
    waitForTaskDrain();
}