#include "ondevicespeechengine_p.h"

#include <json/json.h>

#include <chrono>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include "utils.h"

// 每一个语音段长度 40ms，runtime 里传入的语音段时长为 40ms。我们使用这个长度计算 vad 结果对应的语音段
static const int CHUNK_SIZE_MS = 40;

namespace ai_engine::core_ai::speech {

// runtime 会读取端侧语音引擎的 modelInfo 获取可用模型，然后通过 setCurrent... 接口来设置模型。
// 由于语音模型很多，所以 runtime 与引擎间通过组名来指定模型，每一个模型组包含多个 kytensor 模型。
// 下面是一些帮助完成这个功能的变量。
// 模型组-Kytensor模型 map
const std::unordered_map<OndeviceSpeechEnginePrivate::ModelGroupName,
                         std::vector<OndeviceSpeechEnginePrivate::KytensorModelName>>
    OndeviceSpeechEnginePrivate::ModelGroupMap = {
        // 流式语音识别模型组
        {"paraformer-online",
         {
             // kytensor models for streaming asr
             "asr_cif-search_streaming",
             "asr_decoder_streaming",
             "asr_encoder_streaming",
             "asr_feature-extractor_streaming",
             "asr_lfr-cmvn-pe_streaming",
             "ensemble-asr_paraformer_streaming",
             // kytensor models for non-streaming asr
             "asr_encoder_nonstreaming",
             "asr_feature-extractor_nonstreaming",
             "asr_scoring_nonstreaming",
             "ensemble-asr_paraformer_nonstreaming",
             // kytensor models for punctuation
             "ensemble-punc_ct-transformer_nonstreaming",
             "punc_ct-transformer_nonstreaming",
             // kytensor models for vad
             "asr_fsmn-vad_streaming",
             "ensemble-asr_fsmn-vad_streaming",
         }},

        // 非流式语音识别模型组
        {"paraformer-offline",
         {
             // kytensor models for non-streaming asr
             "asr_encoder_nonstreaming",
             "asr_feature-extractor_nonstreaming",
             "asr_scoring_nonstreaming",
             "ensemble-asr_paraformer_nonstreaming",
             // kytensor models for punctuation
             "ensemble-punc_ct-transformer_nonstreaming",
             "punc_ct-transformer_nonstreaming",
         }},

        // 语音合成模型组
        {"melo-tts", {"ensemble-tts_melo", "tts_melo"}},
};

// 模型组名
const std::unordered_set<OndeviceSpeechEnginePrivate::ModelGroupName>
    OndeviceSpeechEnginePrivate::StreamingAsrModelGroups = {"paraformer-online"};
const std::unordered_set<OndeviceSpeechEnginePrivate::ModelGroupName>
    OndeviceSpeechEnginePrivate::NonStreamingAsrModelGroups = {"paraformer-offline"};
const std::unordered_set<OndeviceSpeechEnginePrivate::ModelGroupName> OndeviceSpeechEnginePrivate::TtsModelGroups = {
    "melo-tts"};

// 在设置了某个模型组时，对应用于推理的 kytensor 模型名
static const std::unordered_map<std::string, std::string> StreamingAsrInferenceModel = {
    {"paraformer-online", "ensemble-asr_paraformer_streaming"},
};
static const std::unordered_map<std::string, std::string> TtsInferenceModel = {
    {"melo-tts", "ensemble-tts_melo"},
};
// 非流式语音识别模型、VAD 模型和标点符号模型在流式和非流式语音识别中都有使用，所以两者的 key 都需要包含
static const std::unordered_map<std::string, std::string> NonStreamingAsrInferenceModel = {
    {"paraformer-online", "ensemble-asr_paraformer_nonstreaming"},
    {"paraformer-offline", "ensemble-asr_paraformer_nonstreaming"},
};
static const std::unordered_map<std::string, std::string> VadInferenceModel = {
    {"paraformer-online", "ensemble-asr_fsmn-vad_streaming"},
    {"paraformer-offline", "ensemble-asr_fsmn-vad_streaming"},
};
static const std::unordered_map<std::string, std::string> PuncInferenceModel = {
    {"paraformer-online", "ensemble-punc_ct-transformer_nonstreaming"},
    {"paraformer-offline", "ensemble-punc_ct-transformer_nonstreaming"},
};

OndeviceSpeechEnginePrivate::OndeviceSpeechEnginePrivate() {
    std::cout << "ondevice speech engine model info: " << std::endl;
    std::cout << modelInfo() << std::endl;

    createClients();
}

OndeviceSpeechEnginePrivate::~OndeviceSpeechEnginePrivate() {
    std::cout << "ondevice speech engine destructing..." << engineId_ << std::endl;
    destructing_.store(true);
    // 确保 synthesizeText 能正常退出
    // 使用原子操作确保 promise 只被设置一次
    bool expected = false;
    if (synthesisPromiseHasValue_.compare_exchange_strong(expected, true)) {
        std::cout << "set promise in destructor" << engineId_ << std::endl;
        try {
            synthesisPromise_.set_value();
        } catch (const std::future_error& e) {
            std::cerr << "Failed to set promise in destructor: " << e.what() << engineId_ << std::endl;
        }
    }
}

std::string OndeviceSpeechEnginePrivate::modelInfo() const {
    Json::Value root;
    root["vendor"] = "";

    Json::Value models(Json::arrayValue);
    Json::Value model;
    model["model"] = "端侧-语音大模型";
    model["displayName"] = "端侧-语音大模型";
    model["modelName"] = "ondevice-speech-model";

    Json::Value subConfigs(Json::arrayValue);
    Json::Value subConfig;
    subConfig["name"] = "语音识别&语音合成";
    subConfig["config"] = Json::Value(Json::arrayValue);
    subConfigs.append(subConfig);

    model["subConfig"] = subConfigs;
    model["debName"] = "";
    model["is3rdModel"] = "false";

    models.append(model);
    root["models"] = models;

    Json::Value modelCatalog(Json::objectValue);

    Json::Value streamingAsrList(Json::arrayValue);
    Json::Value nonStreamingAsrList(Json::arrayValue);
    Json::Value ttsList(Json::arrayValue);

    for (const auto& entry : ModelGroupMap) {
        const std::string& groupName = entry.first;
        const std::vector<OndeviceSpeechEnginePrivate::KytensorModelName>& modelsList = entry.second;

        Json::Value groupInfo;
        groupInfo["name"] = groupName;

        Json::Value modelsArray(Json::arrayValue);
        for (const auto& model : modelsList) {
            modelsArray.append(model);
        }
        groupInfo["models"] = modelsArray;

        if (StreamingAsrModelGroups.find(groupName) != StreamingAsrModelGroups.end()) {
            streamingAsrList.append(groupInfo);
        } else if (NonStreamingAsrModelGroups.find(groupName) != NonStreamingAsrModelGroups.end()) {
            nonStreamingAsrList.append(groupInfo);
        } else if (TtsModelGroups.find(groupName) != TtsModelGroups.end()) {
            ttsList.append(groupInfo);
        }
    }

    modelCatalog["streaming_asr"] = streamingAsrList;
    modelCatalog["non_streaming_asr"] = nonStreamingAsrList;
    modelCatalog["tts"] = ttsList;

    root["model_catalog"] = modelCatalog;

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder.settings_["indentation"] = "    ";
    return Json::writeString(writerBuilder, root);
}

bool OndeviceSpeechEnginePrivate::setCurrentModel(const std::string& model) {
    // TODO: 检查 modeInfo_ 中是否有 model
    currentModel_ = model;
    return true;
}

std::string OndeviceSpeechEnginePrivate::currentModel() const { return currentModel_; }

void OndeviceSpeechEnginePrivate::setConfig(const std::string& config) { return; }

void OndeviceSpeechEnginePrivate::setRecognitionResultCallback(RecognitionResultCallback callback) {
    std::lock_guard<std::mutex> lock(callbackMutex_);
    recognitionCallback_ = callback;
}

void OndeviceSpeechEnginePrivate::setSynthesisResultCallback(SynthesisResultCallback callback) {
    synthesisCallback_ = callback;
}

void OndeviceSpeechEnginePrivate::streamingAsrResultCallback(const std::string& recognizedText, size_t index) {
    // 语音识别结束后不再发送结果
    if (recognizeFinished_ == true) {
        return;
    }

    if (recognizedText.empty() == true) {
        return;
    }

    if (index <= nonstreamingProcessedIndex_) {
        std::cout << "Skip old result: " << recognizedText << std::endl;
        return;
    }

    // 助手显示逻辑：接收到什么就显示什么，不会自动累积显示
    // 需要我们将流式识别结果追加在一起显示给用户
    streamingTexts_ += recognizedText;

    RecognitionResult result{true, {}, currentModel_, streamingTexts_, 0, SpeechResultReason::SpeechRecognizing};

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (recognitionCallback_ == nullptr) {
            std::cerr << "Asr callback is not set." << std::endl;
            return;
        }
        recognitionCallback_(result);
    }
}

void OndeviceSpeechEnginePrivate::nonStreamingAsrResultCallback(const std::string& recognizedText, size_t index) {
    nonstreamingProcessedIndex_ = index;

    // 语音识别结束后不再发送结果
    if (recognizeFinished_ == true) {
        return;
    }

    if (recognizedText.empty() == true) {
        return;
    }

    std::string sentence = punctuationClient_->addPunctuation(recognizedText);

    RecognitionResult res{true, {}, currentModel_, sentence, 0, SpeechResultReason::SpeechRecognized};

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (recognitionCallback_ == nullptr) {
            std::cerr << "Asr callback is not set." << std::endl;
            return;
        }
        recognitionCallback_(res);
    }

    // 助手处的逻辑：接收到 SpeechRecognized 状态的结果，会替换掉 SpeechRecognizing 结果。
    // 所以这里要清空 streamingTexts_，使之后的 SpeechRecognizing 结果重新开始累积。
    streamingTexts_ = "";
}

void OndeviceSpeechEnginePrivate::vadResultCallback(const std::pair<int, int>& vadResult) {
    if (recognizeFinished_ == true) {
        return;
    }

    auto [speechStart, speechEnd] = vadResult;
    vadShouldStart_ = false;

    if (speechStart != -1 || segmentEndDetected_ == true) {
        streamingAsrClient_->setSequenceStart();
        segmentEndDetected_ = false;
    }

    if (speechEnd != -1 || recognizeFinished_ == true) {
        // 这里 vad 检测到语音结束，代码逻辑想要让流式语音识别结束，将其中缓存的音频做识别，
        // 所以这个 setSequenceEnd() 的调用不仅需要设置一个标记位，还需要立刻触发流式语音识别。
        // 若不立刻触发识别，缓存的音频会在下一轮才进行识别。而下面 phaseTwo 的调用是立刻进行非流式语音识别，
        // 就有可能出现在非流式语音识别后多出流式语音识别结果的情况，例如：我是谁？谁
        streamingAsrClient_->setSequenceEnd();
        segmentEndDetected_ = true;
    }

    std::vector<uint8_t> audioData;
    {
        std::lock_guard<std::mutex> lock(audioSegmentsMutex_);
        if (pendingAudioSegments_.empty()) {
            return;
        }
        audioData = pendingAudioSegments_.front();
        pendingAudioSegments_.pop();
    }

    if (!audioData.empty()) {
        phaseTwo(audioData, speechStart, speechEnd);
    }
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::initContinuousRecognitionModule() {
    return result::AiEngineBaseResult{true, EngineError{}};
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::destroyContinuousRecognitionModule() {
    result::AiEngineBaseResult result{true, EngineError{}};
    return result;
}

void OndeviceSpeechEnginePrivate::initContinuousRecognitionParameters() {
    recognizeFinished_ = false;

    streamingTexts_ = "";

    vadShouldStart_ = true;
    segmentStartDetected_ = false;
    segmentEndDetected_ = true;

    nonstreamingProcessedIndex_ = 0;
    processedSegments_ = 0;

    streamingAsrClient_->setSequenceStart();
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::startContinuousRecognition(const std::string& params) {
    result::AiEngineBaseResult result{true, EngineError{}};

    initContinuousRecognitionParameters();

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (recognitionCallback_ != nullptr) {
            RecognitionResult res{true, {}, currentModel_, {}, 0, SpeechResultReason::SpeechRecognitionStarted};
            recognitionCallback_(res);
        }
    }

    return result;
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::writeContinuousRecognitionAudioData(
    const std::vector<uint8_t>& audioData) {
    result::AiEngineBaseResult result{true, EngineError{}};

    bool ret = streamingAsrClient_->automaticSpeechRecognitionAsync(audioData, processedSegments_);
    if (ret == false) {
        std::cerr << "Failed to async streaming recognize" << std::endl;
    }

    processedSegments_++;

    {
        std::lock_guard<std::mutex> lock(audioSegmentsMutex_);
        AllAudioSegments_.push_back(audioData);
        pendingAudioSegments_.push(audioData);
    }

    ret = vadClient_->voiceActivityDetectAsync(audioData, vadShouldStart_, recognizeFinished_);
    if (ret == false) {
        std::cerr << "Failed to async voice activity detection" << std::endl;
    }

    return result;
}

std::pair<int, int> OndeviceSpeechEnginePrivate::voiceActicityDetection(const std::vector<uint8_t>& audioData) {
    auto [speechStart, speechEnd] = vadClient_->voiceActivityDetect(audioData, vadShouldStart_, recognizeFinished_);
    vadShouldStart_ = false;

    if (speechStart != -1 || segmentEndDetected_ == true) {
        streamingAsrClient_->setSequenceStart();
        segmentEndDetected_ = false;
    }

    if (speechEnd != -1 || recognizeFinished_ == true) {
        streamingAsrClient_->setSequenceEnd();
        segmentEndDetected_ = true;
    }

    return {speechStart, speechEnd};
}

void OndeviceSpeechEnginePrivate::phaseTwo(const std::vector<uint8_t>& audioData, int speechStart, int speechEnd) {
    // VAD语音激活检测到开始点后，到检测到结束点前，中间音频段的识别结果可能都是(-1,-1)，这些数据需要存储下来进行非流式识别。
    if (segmentStartDetected_) {
        OfflineAsrSegments_.push_back(audioData);
    }

    // VAD 检测到语音起始点
    if (speechStart != -1) {
        std::vector<std::vector<uint8_t>> audioSegments;
        {
            std::lock_guard<std::mutex> lock(audioSegmentsMutex_);
            audioSegments = AllAudioSegments_;
        }
        segmentStartDetected_ = true;
        int segmentIndex = speechStart / CHUNK_SIZE_MS;
        for (size_t i = segmentIndex; i < audioSegments.size(); ++i) {
            OfflineAsrSegments_.push_back(audioSegments[i]);
        }
    }

    // VAD 检测到语音结束点
    if (speechEnd != -1) {
        std::vector<uint8_t> offlineAudioStream;
        for (const auto& segment : OfflineAsrSegments_) {
            offlineAudioStream.insert(offlineAudioStream.end(), segment.begin(), segment.end());
        }

        auto future = std::async(std::launch::async, [this, offlineAudioStream]() {
            if (offlineAudioStream.empty() == true) {
                return;
            }
            bool ret = nonStreamingAsrClient_->automaticSpeechRecognitionAsync(offlineAudioStream, processedSegments_);
            if (ret == false) {
                std::cerr << "Failed to async non-streaming recognize" << std::endl;
            }
        });

        segmentStartDetected_ = false;
        OfflineAsrSegments_.clear();
    }
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::stopContinuousRecognition() {
    result::AiEngineBaseResult result{true, EngineError{}};
    // sdk 中的 speech_recognizer_stop_continuous_recognition_async, speech_recognizer_destroy_session 都会调用
    // 引擎的 stop 接口。为了避免重复调用，在这里判断是否已经结束。#501885
    if (recognizeFinished_ == true) {
        return result;
    }

    recognizeFinished_ = true;

    {
        std::lock_guard<std::mutex> lock(audioSegmentsMutex_);
        AllAudioSegments_.clear();
        pendingAudioSegments_ = std::queue<std::vector<uint8_t>>();
    }

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (recognitionCallback_ != nullptr) {
            RecognitionResult res{true, {}, currentModel_, {}, 0, SpeechResultReason::SpeechRecognitionCompleted};
            recognitionCallback_(res);
        }
    }

    // 当识别结束时，传入的空音频段做 vad 不会触发 vad
    // 的回调。所以需要主动调用语音识别的接口，以完成最后一段语音的识别。
    // 流式语音识别即设置语音结束标志位，以完成最后一段语音的识别。
    // 非流式语音识别则直接调用 phaseTwo 进行处理，传入 -1 表示无起点，0 表示有结束点。
    streamingAsrClient_->setSequenceEnd();
    phaseTwo({}, -1, 0);

    nonstreamingProcessedIndex_ = 0;
    processedSegments_ = 0;

    return writeContinuousRecognitionAudioData({});
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::setCurrentContinuousRecognitionModel(const std::string& model) {
    if (StreamingAsrModelGroups.count(model) == 0) {
        std::cerr << "Invalid streaming ASR model group: " << model << std::endl;
        return result::AiEngineBaseResult{false, EngineError{AiSdkCommonErrorCode::AI_COMMON_SPEECH_PARAM_INVALID, "Invalid streaming ASR model group: " + model}};
    }

    currentStreamingAsrModelGroup_ = model;

    // 流式语音识别用到了所有模型，所以都需要设置
    streamingAsrClient_->setInferenceModel(StreamingAsrInferenceModel.at(currentStreamingAsrModelGroup_));
    nonStreamingAsrClient_->setInferenceModel(NonStreamingAsrInferenceModel.at(currentStreamingAsrModelGroup_));
    vadClient_->setInferenceModel(VadInferenceModel.at(currentStreamingAsrModelGroup_));
    punctuationClient_->setInferenceModel(PuncInferenceModel.at(currentStreamingAsrModelGroup_));

    return result::AiEngineBaseResult{true, EngineError{}};
}

std::string OndeviceSpeechEnginePrivate::currentContinuousRecognitionModel() const {
    return currentStreamingAsrModelGroup_;
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::initRecognizeOnceModule() {
    return result::AiEngineBaseResult{true, EngineError{}};
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::destroyRecognizeOnceModule() {
    result::AiEngineBaseResult result{true, EngineError{}};
    return result;
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::recognizeOnce(const std::string& params,
                                                                      const std::vector<uint8_t>& audioData) {
    result::AiEngineBaseResult result{true, EngineError{}};
    if (audioData.empty() == true) {
        return result;
    }

    std::cout << "Recognize once input size: " << audioData.size() << std::endl;

    // 定义分段大小阈值（1MB），超过此大小则进行分段识别
    const size_t SEGMENT_THRESHOLD = 1024 * 1024;

    std::string recognizedText;
    if (audioData.size() <= SEGMENT_THRESHOLD) {
        recognizedText = nonStreamingAsrClient_->automaticSpeechRecognition(audioData);
        if (recognizedText.empty() == true) {
            result.success = false;
            result.error = ai_engine::EngineError(AiSdkCommonErrorCode::AI_COMMON_SPEECH_UNKNOWN_ERROR,
                                                  "Get empty non-streaming recognition result.");
            return result;
        }
    } else {
        std::vector<std::string> segmentResults;
        size_t numSegments = (audioData.size() + SEGMENT_THRESHOLD - 1) / SEGMENT_THRESHOLD;
        std::cout << "recognize once segments: " << numSegments << std::endl;

        for (size_t i = 0; i < numSegments; ++i) {
            size_t startIdx = i * SEGMENT_THRESHOLD;
            size_t endIdx = std::min((i + 1) * SEGMENT_THRESHOLD, audioData.size());
            size_t segmentSize = endIdx - startIdx;

            std::vector<uint8_t> segmentAudio(audioData.begin() + startIdx, audioData.begin() + endIdx);

            std::string segmentText = nonStreamingAsrClient_->automaticSpeechRecognition(segmentAudio);

            if (!segmentText.empty()) {
                std::cout << "recognize once " << i << ": " << segmentText << std::endl;
                segmentResults.push_back(segmentText);
            }
        }

        if (segmentResults.empty()) {
            result.success = false;
            result.error = ai_engine::EngineError(AiSdkCommonErrorCode::AI_COMMON_SPEECH_UNKNOWN_ERROR,
                                                  "Get empty non-streaming recognition result from all segments.");
            return result;
        }

        recognizedText = segmentResults[0];
        for (size_t i = 1; i < segmentResults.size(); ++i) {
            recognizedText += segmentResults[i];
        }
    }

    recognizedText = punctuationClient_->addPunctuation(recognizedText);

    RecognitionResult res{true, {}, currentModel_, recognizedText, 0, SpeechResultReason::SpeechRecognized};

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        if (recognitionCallback_ != nullptr) {
            recognitionCallback_(res);
        }
    }

    return result;
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::stopRecognizeOnce() {
    // TODO triton 没有取消推理的接口
    return result::AiEngineBaseResult{true, EngineError{}};
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::setCurrentRecognizeOnceModel(const std::string& model) {
    if (NonStreamingAsrModelGroups.count(model) == 0) {
        std::cerr << "Invalid non-streaming ASR model group: " << model << std::endl;
        return result::AiEngineBaseResult{false, EngineError{AiSdkCommonErrorCode::AI_COMMON_SPEECH_PARAM_INVALID, "Invalid non-streaming ASR model group: " + model}};
    }

    currentNonStreamingAsrModelGroup_ = model;

    nonStreamingAsrClient_->setInferenceModel(NonStreamingAsrInferenceModel.at(currentNonStreamingAsrModelGroup_));
    punctuationClient_->setInferenceModel(PuncInferenceModel.at(currentNonStreamingAsrModelGroup_));

    return result::AiEngineBaseResult{true, EngineError{}};
}

std::string OndeviceSpeechEnginePrivate::currentRecognizeOnceModel() const { return currentNonStreamingAsrModelGroup_; }

void OndeviceSpeechEnginePrivate::processAudioDataInOrder(const std::string& base64AudioData, uint64_t resultId,
                                                          const std::vector<uint64_t>& sequenceIds) {
    std::cout << "Processing audio data in order..." << engineId_ << std::endl;
    // 缓存此次到达的音频
    audioCache_[resultId] = base64AudioData;

    // 按照 sequenceIds 的顺序处理
    while (nextIndex_ < sequenceIds.size()) {
        uint64_t expectedId = sequenceIds[nextIndex_];

        auto it = audioCache_.find(expectedId);
        if (it == audioCache_.end()) {
            // 下一个需要的 ID 还没来，停止
            break;
        }

        // 取出音频并删除缓存
        std::string audioData = it->second;
        audioCache_.erase(it);

        // 构造结果
        SynthesisResult synthesisResult{true,          ai_engine::EngineError(),
                                        currentModel_, "pcm",
                                        16000,         1,
                                        audioData,     SpeechResultReason::SpeechSynthesizing};

        // 设置 reason
        if (sequenceIds.size() == 1) {
            synthesisResult.reason = SpeechResultReason::SpeechSynthesisCompleted;
        } else {
            if (nextIndex_ == 0) {
                synthesisResult.reason = SpeechResultReason::SpeechSynthesisStarted;
            } else if (nextIndex_ == sequenceIds.size() - 1) {
                synthesisResult.reason = SpeechResultReason::SpeechSynthesisCompleted;
            }
        }

        nextIndex_++;

        // 阻塞式 synthesizeText：处理完最后一个 segment 后通知等待
        // engine 析构时跳过，避免重复 set_value 导致 broken_promise
        if (nextIndex_ >= sequenceIds.size() && !destructing_.load()) {
            std::cout << "set promise in process" << engineId_ << std::endl;
            // 使用原子操作确保 promise 只被设置一次
            bool expected = false;
            if (synthesisPromiseHasValue_.compare_exchange_strong(expected, true)) {
                try {
                    synthesisPromise_.set_value();
                } catch (const std::future_error& e) {
                    std::cerr << "Failed to set promise in process: " << engineId_ << e.what() << std::endl;
                }
            }
        }

        if (synthesisCallback_ == nullptr) {
            std::cerr << "process: Synthesis callback is not set." << engineId_ << std::endl;
            continue;
        }
        // 回调
        synthesisCallback_(synthesisResult);
    }
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::initSynthesizeModule() {
    return result::AiEngineBaseResult{true, EngineError{}};
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::destroySynthesizeModule() {
    result::AiEngineBaseResult result{true, ai_engine::EngineError()};
    return result;
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::synthesizeText(const std::string& params,
                                                                       const std::string& text) {
    std::cout << "synthesizeText begin for text: " << text << engineId_ << std::endl;
    result::AiEngineBaseResult result{true, ai_engine::EngineError()};
    if (synthesisCallback_ == nullptr) {
        result.success = false;
        result.error = EngineError{AiSdkCommonErrorCode::AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID,
                                   "synthesizeText: Synthesis callback is not set."};
        return result;
    }

    if (text.empty()) {
        return result;
    }

    setSynthesisParams(params);

    SynthesisResult synthesisResult{
        true, ai_engine::EngineError(), currentModel_, {}, {}, {}, {}, SpeechResultReason::SpeechSynthesisStarted};

    // 分割文本，如果文本长度大于 50 个字符，则分段处理
    std::vector<std::string> segments = Utils::splitText(
        text, 20, 25);  // 在 arm 机器和 x86 机器适合的参数中间取得中间值，尽量避免强制截断，保证语音合成效果

    // 创建 promise 用于阻塞等待所有 segment 合成完成
    // 重置状态，确保使用新的 promise
    synthesisPromiseHasValue_.store(false);
    synthesisPromise_ = std::promise<void>();
    std::future<void> synthesisFuture = synthesisPromise_.get_future();

    for (size_t i = 0; i < segments.size(); ++i) {
        // 使用异步接口
        std::cout << "synthesizeText: segment " << i << ": " << segments[i] << engineId_ << std::endl;
        bool success = ttsClient_->textToSpeechAsync(segments[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(
            10));  // 每次请求暂停10毫秒，确保异步输入顺序正确，避免接收返回值时的排序浪费时间和资源

        if (!success) {
            std::cout << "Failed to start async text to speech." << i << " " << engineId_ << std::endl;
            result.success = false;
            result.error = ai_engine::EngineError(AiSdkCommonErrorCode::AI_COMMON_SPEECH_UNKNOWN_ERROR,
                                                  "Failed to start async text to speech.");
            return result;
        }
    }

    std::cout << "synthesizeText wait for audio" << engineId_ << std::endl;
    // 阻塞等待所有segment合成完成后再返回
    synthesisFuture.get();
    std::cout << "synthesizeText end" << engineId_ << std::endl;

    return result;
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::stopSynthesize() {
    // TODO triton 没有取消推理的接口
    return result::AiEngineBaseResult{true, EngineError{}};
}

result::AiEngineBaseResult OndeviceSpeechEnginePrivate::setCurrentSynthesizeModel(const std::string& model) {
    if (TtsModelGroups.count(model) == 0) {
        std::cerr << "Invalid TTS model group: " << model << std::endl;
        return result::AiEngineBaseResult{false, EngineError{AiSdkCommonErrorCode::AI_COMMON_SPEECH_PARAM_INVALID, "Invalid TTS model group: " + model}};
    }

    currentTtsModelGroup_ = model;

    ttsClient_->setInferenceModel(TtsInferenceModel.at(currentTtsModelGroup_));

    return result::AiEngineBaseResult{true, EngineError{}};
}

std::string OndeviceSpeechEnginePrivate::currentSynthesizeModel() const { return currentTtsModelGroup_; }

void OndeviceSpeechEnginePrivate::setSynthesisParams(const std::string& params) {
    // default speaker
    speaker_ = 0;

    if (params.empty() == true) {
        return;
    }
    std::cout << "Synthesis params: " << params << std::endl;

    Json::Value jsonParams = Utils::formatJsonFromString(params);
    if (jsonParams.isNull() == true) {
        std::cout << "Failed to parse input params." << std::endl;
        return;
    }

    if (jsonParams.isMember("speaker") == false) {
        std::cout << "Failed to parse speaker." << std::endl;
        return;
    }

    if (jsonParams["speaker"].isInt() == false) {
        std::cout << "Speaker must be an integer." << std::endl;
        return;
    }

    speaker_ = jsonParams["speaker"].asInt();
    ttsClient_->setSpeaker(speaker_);
}

void OndeviceSpeechEnginePrivate::createClients() {
    bool streaming = createAsrStreamingClient();
    bool nonstreaming = createAsrNonStreamingClient();
    bool punc = createPunctuationClient();
    bool vad = createVadClient();
    bool tts = createTtsClient();

    assert(streaming == true && nonstreaming == true && punc == true && vad == true && tts == true);
}

bool OndeviceSpeechEnginePrivate::createAsrStreamingClient() {
    if (streamingAsrClient_ != nullptr) {
        return true;
    }

    streamingAsrClient_ = ASRStreamingClient::create();
    if (streamingAsrClient_ == nullptr) {
        std::cerr << "Failed to create streaming asr client." << std::endl;
        return false;
    }

    streamingAsrClient_->setRecognitionResultCallback(
        [this](const std::string& recognizedText, size_t index) { streamingAsrResultCallback(recognizedText, index); });

    return true;
}

bool OndeviceSpeechEnginePrivate::createAsrNonStreamingClient() {
    if (nonStreamingAsrClient_ != nullptr) {
        return true;
    }

    nonStreamingAsrClient_ = ASRNonStreamingClient::create();
    if (nonStreamingAsrClient_ == nullptr) {
        std::cerr << "Failed to create non-streaming asr client" << std::endl;
        return false;
    }

    nonStreamingAsrClient_->setRecognitionResultCallback([this](const std::string& recognizedText, size_t index) {
        nonStreamingAsrResultCallback(recognizedText, index);
    });

    return true;
}

bool OndeviceSpeechEnginePrivate::createPunctuationClient() {
    if (punctuationClient_ != nullptr) {
        return true;
    }

    punctuationClient_ = PunctuationClient::create();
    if (punctuationClient_ == nullptr) {
        std::cerr << "Failed to create punctuation client" << std::endl;
        return false;
    }

    return true;
}

bool OndeviceSpeechEnginePrivate::createVadClient() {
    if (vadClient_ != nullptr) {
        return true;
    }

    vadClient_ = VoiceActivityDetectionClient::create();
    if (vadClient_ == nullptr) {
        std::cerr << "Failed to create voice activity detection client." << std::endl;
        return false;
    }

    vadClient_->setVADResultCallback([this](const std::pair<int, int>& vadResult) { vadResultCallback(vadResult); });

    return true;
}

bool OndeviceSpeechEnginePrivate::createTtsClient() {
    if (ttsClient_ != nullptr) {
        return true;
    }

    ttsClient_ = MelloTTSClient::create();
    if (ttsClient_ == nullptr) {
        std::cerr << "Failed to create TTS client" << std::endl;
        return false;
    }

    ttsClient_->setSynthesisResultCallback(
        [this](const std::string& base64AudioData, const uint64_t resultId, const std::vector<uint64_t>& sequenceIds) {
            processAudioDataInOrder(base64AudioData, resultId, sequenceIds);
        });

    return true;
}

}  // namespace ai_engine::core_ai::speech
