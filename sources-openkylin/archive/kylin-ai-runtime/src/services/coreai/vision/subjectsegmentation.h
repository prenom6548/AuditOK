#ifndef VISION_SUBJECTSEGMENTATION_H
#define VISION_SUBJECTSEGMENTATION_H

#include <kylin-ai/aiengine.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "concurrentqueue/blockingconcurrentqueue.h"
#include "corevisionserviceglue.h"
#include "taskmanager/taskrunner.h"
#include "utils/watchdog.h"

struct SubjectSegmentationResult {
    std::string requestId;
    std::string imageFormat;
    std::string imageBase64Data;
    std::string maskBase64Data;
    int width;
    int height;
    // json data
    // [
    // [{"x":1,"y":2},{"x":2,"y":3}],      // 第一组
    // [{"x":10,"y":20},{"x":11,"y":21}]   // 第二组
    // ]
    std::string edgePoints;
    int errorCode;
    std::string errorMessage;
};

enum SubjectSegmentationMode {
    SUBJECT_SEGMENTATION_MODE_PORTRAIT = 0,  // 人像抠图
    SUBJECT_SEGMENTATION_MODE_GENERAL = 1,   // 通用抠图
};

class SubjectSegmentation : public std::enable_shared_from_this<SubjectSegmentation> {
public:
    using SubjectSegmentationResultCallback = std::function<void(const SubjectSegmentationResult&)>;

    explicit SubjectSegmentation(std::unique_ptr<ai_engine::core_ai::vision::AbstractImageProcessEngine> visionEngine);
    ~SubjectSegmentation();

    int init();
    void destroy();

    void subjectSegmentationFromImageFile(const std::string& requestId, const std::string& imageFilePath,
                                          SubjectSegmentationMode mode,
                                          SubjectSegmentationResultCallback resultCallback);
    void subjectSegmentationFromImageData(const std::string& requestId, const std::string& base64Data,
                                          SubjectSegmentationMode mode,
                                          SubjectSegmentationResultCallback resultCallback);

private:
    struct SubjectSegmentationRequest {
        enum Type { Unknown, ImagePath, Base64Data };
        Type type{Unknown};
        std::string requestId{};
        std::filesystem::path imagePath{};
        std::string base64Data{};
        SubjectSegmentationMode mode;
        SubjectSegmentationResultCallback resultCallback{nullptr};
    };

    SubjectSegmentationRequest generateSubjectSegmentationRequestWithFilePath(
        const std::string& requestId, const std::string& imageFilePath, SubjectSegmentationMode mode,
        SubjectSegmentationResultCallback resultCallback);

    SubjectSegmentationRequest generateSubjectSegmentationRequestWithBase64Data(
        const std::string& requestId, const std::string& base64Data, SubjectSegmentationMode mode,
        SubjectSegmentationResultCallback resultCallback);

    std::shared_ptr<SubjectSegmentation> getSharedObject() { return shared_from_this(); }

    SubjectSegmentationResult makeResultFromEngineError(const ai_engine::EngineError& error) const;
    void callSubjectSegmentationResultCallback(const SubjectSegmentationResult& result) const;
    void subjectSegmentationAsync();
    void subjectSegmentationTask(SubjectSegmentationRequest request);

    bool tryEnqueueRequest(SubjectSegmentationRequest request);
    void decrementRequestQueueSize();

    void notifyRequestFailed(const SubjectSegmentationRequest& request, int errorCode, const std::string& message);
    void rejectPendingRequests(int errorCode, const std::string& message);

private:
    std::unique_ptr<ai_engine::core_ai::vision::AbstractImageProcessEngine> visionEngine_{nullptr};
    SubjectSegmentationResultCallback subjectSegmentationResultCallback_{nullptr};
    moodycamel::BlockingConcurrentQueue<SubjectSegmentationRequest> subjectSegmentationRequests_{};
    SubjectSegmentationRequest currentRequest_{};
    std::atomic_bool taskRunning_{false};
    std::atomic_bool timedOut_{false};
    std::atomic<size_t> requestQueueSize_{0};
    std::mutex taskMutex_;
};

#endif
