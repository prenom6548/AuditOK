#include "longchatimageparser.h"

#include "assistant/error.h"
#include "assistant/sessionbase.h"
#include "engine/aienginepluginmanager.h"
#include "engineselector/engineselector.h"
#include "utils/logger.h"

LongChatImageParser::LongChatImageParser() {}

LongChatImageParser::~LongChatImageParser() {}

bool LongChatImageParser::init() {
    try {
        EngineSelector engineSelector(ai_engine::AiEnginePluginManager::getInstance(),
                                      ModelConfigManager::getInstance());
        std::unique_ptr ocrEngine = engineSelector.selectTextRecognitionEngine();
        if (ocrEngine == nullptr) {
            return false;
        }
        parser_ = std::make_shared<parser::LongImageParser>(std::move(ocrEngine));
        return true;
    } catch (std::exception &exception) {
        parser_.reset();
        LOGE("LongChatImageParser::init(): {}", exception.what());
        return false;
    }
}

LongChatImageParser::LongChatImageParserResult LongChatImageParser::extractImageContents(
    const std::string &imagePath) const {
    try {
        LongChatImageParserResult result{};
        if (parser_ == nullptr) {
            result.errorCode = int(assistant::ErrorCode::UnknownError);
            result.errorMessage = "Select text recognition engine error.";
            LOGE("Select text recognition engine error: parser is nullptr.");
            return result;
        }
        utils::ProcessResult processResult = parser_->extractImageContents(imagePath);
        if (processResult.isSuccess() && processResult.hasResult()) {
            result.result = processResult.getResultString();
        } else {
            LOGD("processResult {} {}", int(processResult.error_code), processResult.error_message);
            if (processResult.error_code == utils::ErrorCode::OCR_RECOGNITION_FAILED) {
                result.errorCode = int(assistant::ErrorCode::ExtractImageContentsFailed);
            } else {
                result.errorCode = int(assistant::ErrorCode::UnknownError);
            }
            result.errorMessage = "Extract image contents failed.";
        }
        return result;
    } catch (std::exception &exception) {
        LongChatImageParserResult result{};
        result.errorCode = int(assistant::ErrorCode::UnknownError);
        result.errorMessage = "Extract image contents exception.";
        LOGE("Extract image contents exception: {}", exception.what());
        return result;
    }
}
