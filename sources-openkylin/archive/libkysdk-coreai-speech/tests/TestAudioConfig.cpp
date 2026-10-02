#include <cassert>
#include <string>

#define CATCH_CONFIG_MAIN  // 提供默认main函数
#include "catch2/catch_amalgamated.hpp"
//
#include "_audioconfig.h"
#include "kylin-ai/coreai/speech/audioconfig.h"
#include "logger.h"

TEST_CASE("audio_config_create_once_audio_input_from_pcm_file", "[audio_config]") {
    AudioConfig *audioConfig{nullptr};
    {
        std::string path = "xxx.pcm";  // 小于16字节，把数据存到栈里
        audioConfig = audio_config_create_once_audio_input_from_pcm_file(path.c_str());
    }
    REQUIRE(audioConfig->inputAudioInfo.inputFilePath == "xxx.pcm");
}

TEST_CASE("audio_config_create_audio_output_from_pcm_file_name", "[audio_config]") {
    AudioConfig *audioConfig{nullptr};
    {
        std::string path = "xxx.pcm";  // 小于16字节，把数据存到栈里
        audioConfig = audio_config_create_audio_output_from_pcm_file_name(path.c_str());
    }
    REQUIRE(audioConfig->outputAudioInfo.outputFilePath == "xxx.pcm");
}

TEST_CASE("audio_config_create_once_audio_input_from_pcm_data", "[audio_config]") {
    AudioConfig *audioConfig{nullptr};
    {
        std::array<uint8_t, 4> data = {0x01, 0x02, 0x03, 0x04};
        audioConfig = audio_config_create_once_audio_input_from_pcm_data(data.data(), data.size());
    }
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    REQUIRE(audioConfig->inputAudioInfo.audioData == data);
}
