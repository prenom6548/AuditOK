#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "asrnonstreaming.h"

using namespace ai_engine::core_ai::speech;

int main() {
    std::unique_ptr<ASRNonStreamingClient> client = ASRNonStreamingClient::create();
    if (client == nullptr) {
        std::cerr << "\033[31mFailed to create ASRNonStreamingClient\033[0m" << std::endl;
        return 1;
    }

    std::ifstream file("../../test/16k.pcm", std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "\033[31mFailed to open 16k.pcm file\033[0m" << std::endl;
        return 1;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> audioData(fileSize);
    if (!file.read(reinterpret_cast<char *>(audioData.data()), fileSize)) {
        std::cerr << "\033[31mFailed to read audio data from file\033[0m" << std::endl;
        return 1;
    }

    std::string result = client->automaticSpeechRecognition(audioData);
    if (result == "北京科技馆") {
        std::cout << "\033[32mTest passed\033[0m" << std::endl;
    } else {
        std::cout << "\033[31mTest failed\033[0m" << std::endl;
        std::cout << "Expected: 北京科技馆" << std::endl;
        std::cout << "Actual: " << result << std::endl;
    }

    return 0;
}
