#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "voiceactivitydetection.h"

using namespace ai_engine::core_ai::speech;

struct VadResult {
    int chunkIndex;
    int startPoint;
    int endPoint;

    VadResult(int index, int start, int end) : chunkIndex(index), startPoint(start), endPoint(end) {}

    bool operator==(const VadResult &other) const {
        return chunkIndex == other.chunkIndex && startPoint == other.startPoint && endPoint == other.endPoint;
    }
};

std::ostream &operator<<(std::ostream &os, const VadResult &result) {
    os << "Chunk " << result.chunkIndex << ": [" << result.startPoint << ", " << result.endPoint << "]";
    return os;
}

static const std::vector<VadResult> expectedResults = {
    VadResult(2, 80, -1),      VadResult(14, -1, 2340),   VadResult(15, 2620, -1),   VadResult(33, -1, 6200),
    VadResult(34, 6480, -1),   VadResult(121, -1, 23670), VadResult(134, -1, 26260), VadResult(136, 26780, -1),
    VadResult(147, -1, 29000), VadResult(151, 29950, -1), VadResult(160, -1, 31430), VadResult(190, -1, 37600),
    VadResult(193, 38210, -1), VadResult(237, -1, 46900), VadResult(238, 47310, -1), VadResult(251, -1, 49630),
    VadResult(285, -1, 56460), VadResult(300, -1, 59550), VadResult(301, 59830, -1), VadResult(353, -1, 70660)};

int main() {
    std::unique_ptr<VoiceActivityDetectionClient> client = VoiceActivityDetectionClient::create();
    if (client == nullptr) {
        std::cerr << "\033[31mFailed to create VoiceActivityDetectionClient\033[0m" << std::endl;
        return 1;
    }

    // TODO YDS path
    std::ifstream file("../../test/vad_example.wav", std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "\033[31mFailed to open vad_example.wav file\033[0m" << std::endl;
        return 1;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> audioData(fileSize);
    if (!file.read(reinterpret_cast<char *>(audioData.data()), fileSize)) {
        std::cerr << "\033[31mFailed to read audio data from file\033[0m" << std::endl;
        return 1;
    }

    int chunk_size = 200;
    int sample_rate = 16000;
    int chunk_stride = chunk_size * sample_rate / 1000 * 2;
    int total_chunk_num = (audioData.size() - 1) / chunk_stride + 1;

    std::vector<VadResult> actualResults;

    for (int i = 0; i < total_chunk_num; ++i) {
        int start = i * chunk_stride;
        int end = (i + 1) * chunk_stride;
        if (end > static_cast<int>(audioData.size())) {
            end = audioData.size();
        }

        std::vector<uint8_t> audioChunk(audioData.begin() + start, audioData.begin() + end);

        bool isStart = (i == 0);
        bool isEnd = (i == total_chunk_num - 1);

        std::pair<int, int> result = client->voiceActivityDetect(audioChunk, isStart, isEnd);

        if (result.first != -1 || result.second != -1) {
            actualResults.emplace_back(i, result.first, result.second);
        }
    }

    for (const auto &result : actualResults) {
        std::cout << result << std::endl;
    }

    bool testPassed = (actualResults.size() == expectedResults.size());
    if (testPassed) {
        for (size_t i = 0; i < actualResults.size(); ++i) {
            if (!(actualResults[i] == expectedResults[i])) {
                testPassed = false;
                std::cout << "\033[31mMismatch at index " << i << ":\033[0m" << std::endl;
                std::cout << "  Expected: " << expectedResults[i] << std::endl;
                std::cout << "  Actual:   " << actualResults[i] << std::endl;
                break;
            }
        }
    }

    if (testPassed) {
        std::cout << "\033[32mVAD test completed! All results match expected "
                     "values.\033[0m"
                  << std::endl;
    } else {
        std::cout << "\033[31mVAD test failed! Results do not match expected "
                     "values.\033[0m"
                  << std::endl;
        std::cout << "Expected " << expectedResults.size() << " results, but got " << actualResults.size() << std::endl;
        return 1;
    }

    return 0;
}
