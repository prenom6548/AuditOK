#include <iostream>

#include "audioplayer.h"
#include "mellotts.h"
#include "utils.h"

using namespace ai_engine::core_ai::speech;

int main() {
    std::unique_ptr<MelloTTSClient> client = MelloTTSClient::create();
    if (client == nullptr) {
        std::cerr << "\033[31mFailed to create MelloTTSClient\033[0m" << std::endl;
        return 1;
    }

    AudioPlayer audioPlayer;

    std::cout << "Test case 1:" << std::endl;
    std::string text1 = "今天天气真好！我们去公园散步吧！";
    std::string result1 = client->textToSpeech(text1);

    if (result1.empty() == false) {
        std::cout << "\033[32mTest case 1 passed!\033[0m" << std::endl;
        std::string audioDecoded = Utils::base64Decode(result1);
        std::vector<uint8_t> audio{audioDecoded.begin(), audioDecoded.end()};
        audioPlayer.playing(audio);
    } else {
        std::cout << "\033[31mTest case 1 failed!\033[0m" << std::endl;
    }

    std::cout << std::endl;

    // 测试用例2: 空文本
    std::cout << "Test case 2:" << std::endl;
    std::string text2 = "";
    std::string result2 = client->textToSpeech(text2);

    std::cout << "  Origin text: " << text2 << std::endl;
    std::cout << "  Result text: " << result2 << std::endl;
    if (result2 == "") {
        std::cout << "\033[32mTest case 2 passed!\033[0m" << std::endl;
    } else {
        std::cout << "\033[31mTest case 2 failed!\033[0m" << std::endl;
    }

    return 0;
}
