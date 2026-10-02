#ifndef COREAISPEECHENGINE_H
#define COREAISPEECHENGINE_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace core_ai {
namespace speech {

enum class SpeechResultReason {
    // 出错
    SpeechErrorOccurred = 1,
    // 已启动语音识别
    SpeechRecognitionStarted = 2,
    // 正在进行语音识别，中间结果
    SpeechRecognizing = 3,
    // 语音识别的最终结果，经过修正
    SpeechRecognized = 4,
    // 语音识别完成
    SpeechRecognitionCompleted = 5,
    // 已启动语音合成
    SpeechSynthesisStarted = 6,
    // 正在进行语音合成，语音合成的结果输出
    SpeechSynthesizing = 7,
    // 语音合成已经完成
    SpeechSynthesisCompleted = 8,
};

struct RecognitionResult : result::AiEngineModelResult {
    std::string text;
    int speakerId;
    SpeechResultReason reason;
};

struct SynthesisResult : result::AiEngineModelResult {
    std::string audioFormat;
    int audioRate;
    int audioChannel;
    std::string base64AudioData;
    SpeechResultReason reason;
};

using RecognitionResultCallback =
    std::function<void(const RecognitionResult &result)>;
using SynthesisResultCallback =
    std::function<void(const SynthesisResult &result)>;

class AbstractSpeechEngine : public ai_engine::AbstractAiEngine {
public:
    virtual ~AbstractSpeechEngine() {}

    virtual void setRecognitionResultCallback(
        RecognitionResultCallback callback) = 0;
    virtual void setSynthesisResultCallback(
        SynthesisResultCallback callback) = 0;

    ///[0] 流式语音识别
    virtual result::AiEngineBaseResult initContinuousRecognitionModule() = 0;
    virtual result::AiEngineBaseResult destroyContinuousRecognitionModule() = 0;

    virtual result::AiEngineBaseResult startContinuousRecognition(
        const std::string &params) = 0;

    virtual result::AiEngineBaseResult writeContinuousRecognitionAudioData(
        const std::vector<uint8_t> &audioData) = 0;

    virtual result::AiEngineBaseResult stopContinuousRecognition() = 0;
    virtual result::AiEngineBaseResult setCurrentContinuousRecognitionModel(
        const std::string &model) = 0;
    virtual std::string currentContinuousRecognitionModel() const = 0;
    ///[0]

    ///[1] 非流式语音识别
    virtual result::AiEngineBaseResult initRecognizeOnceModule() = 0;
    virtual result::AiEngineBaseResult destroyRecognizeOnceModule() = 0;
    virtual result::AiEngineBaseResult recognizeOnce(
        const std::string &params, const std::vector<uint8_t> &audioData) = 0;
    virtual result::AiEngineBaseResult stopRecognizeOnce() = 0;
    virtual result::AiEngineBaseResult setCurrentRecognizeOnceModel(
        const std::string &model) = 0;
    virtual std::string currentRecognizeOnceModel() const = 0;
    ///[1]

    ///[2] 语音合成
    virtual result::AiEngineBaseResult initSynthesizeModule() = 0;
    virtual result::AiEngineBaseResult destroySynthesizeModule() = 0;
    virtual result::AiEngineBaseResult synthesizeText(
        const std::string &params, const std::string &text) = 0;
    virtual result::AiEngineBaseResult stopSynthesize() = 0;
    virtual result::AiEngineBaseResult setCurrentSynthesizeModel(
        const std::string &model) = 0;
    virtual std::string currentSynthesizeModel() const = 0;
    ///[2]
};

}  // namespace speech
}  // namespace core_ai
}  // namespace ai_engine

#endif