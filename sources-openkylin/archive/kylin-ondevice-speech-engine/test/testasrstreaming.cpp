#include <iostream>
#include <vector>

#include "asrstreaming.h"
#include "audiorecorder.h"

using namespace ai_engine::core_ai::speech;

int main() {
    std::unique_ptr<ASRStreamingClient> client = ASRStreamingClient::create();
    if (client == nullptr) {
        std::cerr << "\033[31mFailed to create ASRStreamingClient\033[0m" << std::endl;
        return 1;
    }

    client->setRecognitionResultCallback(
        [](const std::string& result, size_t index) { std::cout << "\033[32m" << result << "\033[0m" << std::flush; });

    AudioRecorder audioRecorder{};
    audioRecorder.setSamplingTime(std::chrono::milliseconds(60));

    while (true) {
        std::vector<uint8_t> audioData;
        audioRecorder.recording(audioData);

        bool ret = client->automaticSpeechRecognitionAsync(audioData, 1);
        if (!ret) {
            std::cerr << "Error infer." << std::endl;
            return 1;
        }
    }

    return 0;
}
