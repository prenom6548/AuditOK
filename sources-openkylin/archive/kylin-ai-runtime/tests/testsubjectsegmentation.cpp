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

#include "services/coreai/vision/subjectsegmentation.h"
#include "testmodulescommon.h"

TEST(SubjectSegmentationTest, TimeoutTriggersError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 1), true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 1500, false, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       state.results += 1;
                                                       state.lastError = result.errorCode;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SubjectSegmentationTest, QueueFullRejectsAllRequests) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 5), true);
    ScopedPendingLimits limits(0, 0);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, false, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    auto onResult = [&](const SubjectSegmentationResult& result) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.results += 1;
        state.lastError = result.errorCode;
        state.finished += 1;
        if (state.finished >= 2) {
            state.cv.notify_all();
        }
    };
    segmentation->subjectSegmentationFromImageData("request-id-1", "data1", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   onResult);
    segmentation->subjectSegmentationFromImageData("request-id-2", "data2", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   onResult);

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished >= 2; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 2);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS);
}

TEST(SubjectSegmentationTest, RequestQueueOverflowRejectsNewRequest) {
    constexpr size_t kMaxQueueSize = 64;
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 5), true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 20, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    std::mutex mutex;
    std::condition_variable cv;
    int callbacks = 0;
    int rejected = 0;

    auto onResult = [&](const SubjectSegmentationResult& result) {
        std::lock_guard<std::mutex> lock(mutex);
        callbacks += 1;
        if (result.errorCode == AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS) {
            rejected += 1;
        }
        cv.notify_all();
    };

    for (size_t i = 0; i < kMaxQueueSize + 1; ++i) {
        segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                       onResult);
    }

    const int expectedCallbacks = static_cast<int>(kMaxQueueSize + 1);
    ASSERT_TRUE(waitFor(cv, mutex, [&]() { return callbacks == expectedCallbacks; },
                        std::chrono::milliseconds(4000)));
    EXPECT_GE(rejected, 1);

    task::TaskRunner::getInstance().waitForTasks();
}

TEST(SubjectSegmentationTest, EngineFailureReturnsError) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 5), true);
    int errorCode = (int)ai_engine::VisionEngineErrorCode::MattingModelError;
    auto engine = std::make_unique<FakeImageProcessEngine>(false, 0, false, errorCode);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       state.results += 1;
                                                       state.lastError = result.errorCode;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(2500)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(state.lastError, AiSdkCommonErrorCode::AI_COMMON_MODEL_RUN_FAILED);
}

TEST(SubjectSegmentationTest, ImageDataSuccessReturnsResult) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 5), true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    SubjectSegmentationResult callbackResult;
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       callbackResult = result;
                                                       state.results += 1;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(callbackResult.imageFormat, "png");
    EXPECT_EQ(callbackResult.imageBase64Data, "data");
    EXPECT_EQ(callbackResult.maskBase64Data, "mask");
    EXPECT_EQ(callbackResult.width, 1);
    EXPECT_EQ(callbackResult.height, 1);
    EXPECT_EQ(callbackResult.errorCode, AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    EXPECT_TRUE(callbackResult.errorMessage.empty());
}

TEST(SubjectSegmentationTest, ImageFileSuccessReturnsResult) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 5), true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    SubjectSegmentationResult callbackResult;
    segmentation->subjectSegmentationFromImageFile("request-id", "/tmp/test.png", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       callbackResult = result;
                                                       state.results += 1;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_EQ(callbackResult.imageFormat, "png");
    EXPECT_EQ(callbackResult.imageBase64Data, "data");
    EXPECT_EQ(callbackResult.maskBase64Data, "mask");
    EXPECT_EQ(callbackResult.width, 1);
    EXPECT_EQ(callbackResult.height, 1);
    EXPECT_EQ(callbackResult.errorCode, AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    EXPECT_TRUE(callbackResult.errorMessage.empty());
}

TEST(SubjectSegmentationTest, ConfigInvalidFallsBack) {
    ScopedTaskConfigOverride config("not-json", true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       state.results += 1;
                                                       state.lastError = result.errorCode;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SubjectSegmentationTest, ConfigMissingUsesDefault) {
    ScopedTaskConfigOverride config("", false);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       state.results += 1;
                                                       state.lastError = result.errorCode;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SubjectSegmentationTest, ConfigZeroFallsBack) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 0), true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);

    CallbackState state;
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL,
                                                   [&](const SubjectSegmentationResult& result) {
                                                       std::lock_guard<std::mutex> lock(state.mutex);
                                                       state.results += 1;
                                                       state.lastError = result.errorCode;
                                                       state.finished += 1;
                                                       state.cv.notify_all();
                                                   });

    ASSERT_TRUE(waitFor(state.cv, state.mutex, [&]() { return state.finished == 1; }, std::chrono::milliseconds(1000)));
    EXPECT_EQ(state.results, 1);
    EXPECT_NE(state.lastError, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT);
}

TEST(SubjectSegmentationTest, CallbackNullDoesNotCrash) {
    ScopedTaskConfigOverride config(makeTimeoutConfig("SubjectSegmentation", 5), true);
    auto engine = std::make_unique<FakeImageProcessEngine>(true, 0, true, -1);
    auto segmentation = std::make_shared<SubjectSegmentation>(std::move(engine));
    ASSERT_EQ(segmentation->init(), AiSdkCommonErrorCode::AI_COMMON_SUCCESS);
    segmentation->subjectSegmentationFromImageData("request-id", "data", SUBJECT_SEGMENTATION_MODE_GENERAL, nullptr);
    task::TaskRunner::getInstance().waitForTasks();
}
