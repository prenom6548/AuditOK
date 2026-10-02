#include "subjectsegmentation.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/common/error.h>

#include <chrono>
#include <mutex>
#include <utility>

#include "taskmanager/taskrunner.h"
#include "utils/logger.h"
#include "vision.h"

namespace {
constexpr int64_t kDefaultSubjectSegmentationTimeoutMs = 5LL * 60 * 1000;
constexpr size_t kSubjectSegmentationRequestQueueMaxSize = 10;

int64_t loadSubjectSegmentationTimeoutMs() {
    const auto taskTimeout = task::loadTaskTimeoutMs(task::TaskType::SubjectSegmentation);
    if (taskTimeout.has_value()) {
        return *taskTimeout;
    }
    LOGW("Subject segmentation task timeout not found in task config, use default.");
    return kDefaultSubjectSegmentationTimeoutMs;
}

size_t loadSubjectSegmentationRequestQueueMaxSize() {
    const auto maxSize = task::loadTaskQueueMaxSize(task::TaskType::SubjectSegmentation);
    if (maxSize.has_value()) {
        return static_cast<size_t>(*maxSize);
    }
    LOGW("Subject segmentation request queue max size not found in task config, use default.");
    return kSubjectSegmentationRequestQueueMaxSize;
}
}  // namespace

SubjectSegmentation::SubjectSegmentation(
    std::unique_ptr<ai_engine::core_ai::vision::AbstractImageProcessEngine> visionEngine)
    : visionEngine_(std::move(visionEngine)) {}

SubjectSegmentation::~SubjectSegmentation() {
    subjectSegmentationResultCallback_ = nullptr;
    LOGI("~SubjectSegmentation");
}

void SubjectSegmentation::subjectSegmentationFromImageFile(const std::string& requestId,
                                                           const std::string& imageFilePath,
                                                           SubjectSegmentationMode mode,
                                                           SubjectSegmentationResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request =
        generateSubjectSegmentationRequestWithFilePath(requestId, imageFilePath, mode, std::move(resultCallback));
    if (!tryEnqueueRequest(request)) {
        return;
    }
    subjectSegmentationAsync();
}

void SubjectSegmentation::subjectSegmentationFromImageData(const std::string& requestId, const std::string& base64Data,
                                                           SubjectSegmentationMode mode,
                                                           SubjectSegmentationResultCallback resultCallback) {
    if (resultCallback == nullptr) {
        return;
    }
    const auto& request =
        generateSubjectSegmentationRequestWithBase64Data(requestId, base64Data, mode, std::move(resultCallback));
    if (!tryEnqueueRequest(request)) {
        return;
    }
    subjectSegmentationAsync();
}

SubjectSegmentation::SubjectSegmentationRequest SubjectSegmentation::generateSubjectSegmentationRequestWithFilePath(
    const std::string& requestId, const std::string& imageFilePath, SubjectSegmentationMode mode,
    SubjectSegmentationResultCallback resultCallback) {
    SubjectSegmentationRequest request{};
    request.resultCallback = std::move(resultCallback);
    request.type = SubjectSegmentationRequest::Type::ImagePath;
    request.requestId = requestId;
    request.imagePath = imageFilePath;
    request.mode = mode;
    return request;
}

SubjectSegmentation::SubjectSegmentationRequest SubjectSegmentation::generateSubjectSegmentationRequestWithBase64Data(
    const std::string& requestId, const std::string& base64Data, SubjectSegmentationMode mode,
    SubjectSegmentationResultCallback resultCallback) {
    SubjectSegmentationRequest request{};
    request.resultCallback = std::move(resultCallback);
    request.type = SubjectSegmentationRequest::Type::Base64Data;
    request.requestId = requestId;
    request.base64Data = base64Data;
    request.mode = mode;
    return request;
}

void SubjectSegmentation::subjectSegmentationAsync() {
    bool expected = false;
    if (!taskRunning_.compare_exchange_strong(expected, true)) {
        return;
    }
    SubjectSegmentationRequest nextRequest{};
    if (!subjectSegmentationRequests_.try_dequeue(nextRequest)) {
        taskRunning_.store(false);
        return;
    }
    decrementRequestQueueSize();
    std::shared_ptr sharedObject = getSharedObject();
    auto enqueueResult = task::TaskRunner::getInstance().addTask(
        {task::generateTaskId(), task::TaskType::SubjectSegmentation, [sharedObject, nextRequest]() mutable {
             sharedObject->subjectSegmentationTask(std::move(nextRequest));
             sharedObject->taskRunning_.store(false);
             sharedObject->subjectSegmentationAsync();
         }});
    if (enqueueResult != task::TaskRunner::EnqueueResult::Accepted) {
        LOGE("Failed to enqueue SubjectSegmentation task.");
        taskRunning_.store(false);
        notifyRequestFailed(nextRequest, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.");
        rejectPendingRequests(AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                              "Too many requests, please try again later.");
    }
}

bool SubjectSegmentation::tryEnqueueRequest(SubjectSegmentationRequest request) {
    const size_t maxQueueSize = loadSubjectSegmentationRequestQueueMaxSize();
    size_t pending = requestQueueSize_.fetch_add(1, std::memory_order_relaxed);
    if (pending >= maxQueueSize) {
        requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
        LOGW("Subject segmentation request queue is full, reject request.");
        notifyRequestFailed(request, AiSdkCommonErrorCode::AI_COMMON_TOO_MANY_REQUESTS,
                            "Too many requests, please try again later.");
        return false;
    }
    subjectSegmentationRequests_.enqueue(std::move(request));
    return true;
}

void SubjectSegmentation::decrementRequestQueueSize() {
    size_t current = requestQueueSize_.fetch_sub(1, std::memory_order_relaxed);
    if (current == 0) {
        requestQueueSize_.store(0, std::memory_order_relaxed);
    }
}

void SubjectSegmentation::subjectSegmentationTask(SubjectSegmentationRequest request) {
    currentRequest_ = std::move(request);
    timedOut_.store(false);
    util::Watchdog watchdog;
    watchdog.Start(loadSubjectSegmentationTimeoutMs(), [this]() {
        timedOut_.store(true);
        notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_MODEL_RUN_TIME_OUT,
                            "Subject segmentation timed out.");
        visionEngine_->stopMattingProcess();
    });
    ai_engine::result::AiEngineBaseResult result{};
    switch (currentRequest_.type) {
        case SubjectSegmentationRequest::Type::Base64Data: {
            result = visionEngine_->mattingFromBase64ImageData(
                currentRequest_.base64Data, ai_engine::core_ai::vision::MattingMode(currentRequest_.mode));
            break;
        }
        case SubjectSegmentationRequest::Type::ImagePath: {
            result = visionEngine_->mattingFromImageFile(currentRequest_.imagePath,
                                                         ai_engine::core_ai::vision::MattingMode(currentRequest_.mode));
            break;
        }
        default: {
            LOGE("Unknown input type for subject segmentation.");
            if (watchdog.TryFinish()) {
                notifyRequestFailed(currentRequest_, AiSdkCommonErrorCode::AISDK_BAD_REQUEST,
                                    "Unknown subject segmentation input type.");
            }
            watchdog.Finish();
            return;
        }
    }
    if (!result.success) {
        LOGE("Failed to subject segmentation: {} {}", result.error.getCode(), result.error.getMessage());
        if (watchdog.TryFinish()) {
            callSubjectSegmentationResultCallback(makeResultFromEngineError(result.error));
        }
        watchdog.Finish();
        return;
    }
    watchdog.TryFinish();
    watchdog.Finish();
}

int SubjectSegmentation::init() {
    auto result = visionEngine_->initMattingsModule();
    if (!result.success) {
        LOGE("Failed to init subject segmentation engine: {} {}", result.error.getCode(), result.error.getMessage());
        callSubjectSegmentationResultCallback(makeResultFromEngineError(result.error));
        return result.error.getCode();
    }
    std::shared_ptr sharedObject = getSharedObject();
    if (sharedObject == nullptr) {
        return AI_COMMON_RUNTIME_ERROR;
    }
    visionEngine_->setMattingResultCallback([sharedObject](const auto& result) {
        if (sharedObject->timedOut_.load()) {
            return;
        }
        SubjectSegmentationResult subjectSegmentationResult{sharedObject->currentRequest_.requestId,
                                                            result.imageFormat,
                                                            result.imageBase64Data,
                                                            result.maskBase64Data,
                                                            result.width,
                                                            result.height,
                                                            result.edgePointsData,
                                                            result.error.getCode(),
                                                            result.error.getMessage()};

        sharedObject->callSubjectSegmentationResultCallback(subjectSegmentationResult);
    });
    return AI_COMMON_SUCCESS;
}

void SubjectSegmentation::destroy() {
    if (visionEngine_) {
        visionEngine_->setMattingResultCallback(nullptr);
        visionEngine_->destroyMattingsModule();
    }
}

void SubjectSegmentation::callSubjectSegmentationResultCallback(const SubjectSegmentationResult& result) const {
    if (timedOut_.load()) {
        return;
    }
    if (currentRequest_.resultCallback != nullptr) {
        currentRequest_.resultCallback(result);
    }
}

void SubjectSegmentation::notifyRequestFailed(const SubjectSegmentationRequest& request, int errorCode,
                                              const std::string& message) {
    SubjectSegmentationResult result{};
    result.requestId = request.requestId;
    result.errorCode = errorCode;
    result.errorMessage = message;
    if (request.resultCallback) {
        request.resultCallback(result);
    }
}

void SubjectSegmentation::rejectPendingRequests(int errorCode, const std::string& message) {
    SubjectSegmentationRequest dropped{};
    while (subjectSegmentationRequests_.try_dequeue(dropped)) {
        decrementRequestQueueSize();
        notifyRequestFailed(dropped, errorCode, message);
    }
}

SubjectSegmentationResult SubjectSegmentation::makeResultFromEngineError(const ai_engine::EngineError& error) const {
    return SubjectSegmentationResult{currentRequest_.requestId, "", "", "", -1, -1, "", error.getCode(),
                                     error.getMessage()};
}
