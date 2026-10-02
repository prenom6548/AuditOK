#pragma once

#include <kylin-ai/ai-engine/speechengine.h>
#include <kylin-ai/common/error.h>
#include <uuid/uuid.h>

#include <future>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "asrnonstreaming.h"
#include "asrstreaming.h"
#include "mellotts.h"
#include "punctuation.h"
#include "voiceactivitydetection.h"

namespace ai_engine::core_ai::speech {

class OndeviceSpeechEnginePrivate : public AbstractSpeechEngine {
public:
    OndeviceSpeechEnginePrivate();
    virtual ~OndeviceSpeechEnginePrivate() override;

    virtual std::string engineName() const override { return "ondevice"; }
    virtual bool isCloud() const override { return false; }
    virtual bool isBuiltInEngine() const override { return true; }
    virtual bool isCustomModelEngine() const override { return false; }
    virtual std::string modelInfo() const override;
    virtual bool setCurrentModel(const std::string &model) override;
    virtual std::string currentModel() const override;
    virtual void setConfig(const std::string &config) override;

    virtual void setRecognitionResultCallback(RecognitionResultCallback callback) override;
    virtual void setSynthesisResultCallback(SynthesisResultCallback callback) override;

    // 流式语音识别
    virtual result::AiEngineBaseResult initContinuousRecognitionModule() override;
    virtual result::AiEngineBaseResult destroyContinuousRecognitionModule() override;
    virtual result::AiEngineBaseResult startContinuousRecognition(const std::string &params) override;
    virtual result::AiEngineBaseResult writeContinuousRecognitionAudioData(
        const std::vector<uint8_t> &audioData) override;
    virtual result::AiEngineBaseResult stopContinuousRecognition() override;
    virtual result::AiEngineBaseResult setCurrentContinuousRecognitionModel(const std::string& model) override;
    virtual std::string currentContinuousRecognitionModel() const override;

    // 非流式语音识别
    virtual result::AiEngineBaseResult initRecognizeOnceModule() override;
    virtual result::AiEngineBaseResult destroyRecognizeOnceModule() override;
    virtual result::AiEngineBaseResult recognizeOnce(const std::string &params,
                                                     const std::vector<uint8_t> &audioData) override;
    virtual result::AiEngineBaseResult stopRecognizeOnce() override;
    virtual result::AiEngineBaseResult setCurrentRecognizeOnceModel(const std::string& model) override;
    virtual std::string currentRecognizeOnceModel() const override;

    // 语音合成
    virtual result::AiEngineBaseResult initSynthesizeModule() override;
    virtual result::AiEngineBaseResult destroySynthesizeModule() override;
    virtual result::AiEngineBaseResult synthesizeText(const std::string &params, const std::string &text) override;
    virtual result::AiEngineBaseResult stopSynthesize() override;
    virtual result::AiEngineBaseResult setCurrentSynthesizeModel(const std::string& model) override;
    virtual std::string currentSynthesizeModel() const override;

private:
    // 设置输入参数（比如说话人之类的参数）
    void setSynthesisParams(const std::string &params);

    void phaseTwo(const std::vector<uint8_t> &audioData, int speechStart, int speechEnd);
    void streamingAsrResultCallback(const std::string& recognizedText, size_t index);
    void nonStreamingAsrResultCallback(const std::string& recognizedText, size_t index);
    void vadResultCallback(const std::pair<int, int> &vadResult);
    void initContinuousRecognitionParameters();
    std::pair<int, int> voiceActicityDetection(const std::vector<uint8_t> &audioData);
    // 处理音频数据的函数
    void processAudioDataInOrder(const std::string &base64AudioData, uint64_t resultId,
                                 const std::vector<uint64_t> &sequenceIds);
    void createClients();
    bool createAsrStreamingClient();
    bool createAsrNonStreamingClient();
    bool createPunctuationClient();
    bool createVadClient();
    bool createTtsClient();

private:
    std::string currentModel_{"端侧语音模型"};
    std::string currentStreamingAsrModelGroup_;
    std::string currentNonStreamingAsrModelGroup_;
    std::string currentTtsModelGroup_;

    using ModelGroupName = std::string;
    using KytensorModelName = std::string;
    static const std::unordered_map<ModelGroupName, std::vector<KytensorModelName>> ModelGroupMap;
    static const std::unordered_set<ModelGroupName> StreamingAsrModelGroups;
    static const std::unordered_set<ModelGroupName> NonStreamingAsrModelGroups;
    static const std::unordered_set<ModelGroupName> TtsModelGroups;

    RecognitionResultCallback recognitionCallback_;
    SynthesisResultCallback synthesisCallback_;
    std::mutex callbackMutex_;

    std::unique_ptr<ASRStreamingClient> streamingAsrClient_ = nullptr;
    std::unique_ptr<ASRNonStreamingClient> nonStreamingAsrClient_ = nullptr;
    std::unique_ptr<PunctuationClient> punctuationClient_ = nullptr;
    std::unique_ptr<VoiceActivityDetectionClient> vadClient_ = nullptr;
    std::unique_ptr<MelloTTSClient> ttsClient_ = nullptr;

    // 一次语音识别结束（应用侧调用 start 再调用 stop）
    bool recognizeFinished_ = true;

    // 流式识别结果
    std::string streamingTexts_;

    // 在一次语音识别中（应用侧调用 start 再调用 stop），VAD 只开始一次，也只结束一次。
    bool vadShouldStart_ = false;

    /* 下面两个变量用于表示 VAD 检测到的开始点和结束点。
     * 我们使用开始点来收集音频段，转发给非流式语音模型识别。
     * 使用结束点来通知流式语音识别模型创建一个新的序列。
     */
    bool segmentStartDetected_ = false;
    // 当前一段语音是否结束。在识别中，如果 VAD 检测的结束点后有一段时间的无声，那么我们结束点后启动一个新的 sequence。
    bool segmentEndDetected_ = true;

    std::vector<std::vector<uint8_t>> OfflineAsrSegments_;
    std::vector<std::vector<uint8_t>> AllAudioSegments_;
    size_t nonstreamingProcessedIndex_ = 0;
    size_t processedSegments_ = 0;

    std::mutex audioSegmentsMutex_;
    std::queue<std::vector<uint8_t>> pendingAudioSegments_;

    // text-to-speech
    int speaker_ = 0;



    // 添加用于缓存乱序音频数据的成员变量
    uint64_t nextIndex_ = 0;
    std::unordered_map<uint64_t, std::string> audioCache_;

    std::promise<void> synthesisPromise_;
    // 标记是否有正在等待的synthesizeText调用，用于避免析构时重复set_value导致terminate
    std::atomic<bool> synthesisPromiseHasValue_ = false;
    // 标记engine是否正在析构，用于确保synthesizeText能正常退出
    std::atomic<bool> destructing_ = false;

    int engineId_ = rand();
};

}  // namespace ai_engine::core_ai::speech