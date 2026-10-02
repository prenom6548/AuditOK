#include <iostream>

#include "punctuation.h"

using namespace ai_engine::core_ai::speech;

int main() {
    std::unique_ptr<PunctuationClient> client = PunctuationClient::create();
    if (client == nullptr) {
        std::cerr << "\033[31mFailed to create PunctuationClient\033[0m" << std::endl;
        return 1;
    }

    // 测试用例1: 普通中文文本标点符号添加
    std::string text1 = "今天天气真好啊我们去公园散步吧";
    std::string result1 = client->addPunctuation(text1);

    std::cout << "Test case 1:" << std::endl;
    std::cout << "  Origin text: " << text1 << std::endl;
    std::cout << "  Result text: " << result1 << std::endl;
    std::string expectedResult1 = "今天天气真好啊，我们去公园散步吧。";
    if (result1 == expectedResult1) {
        std::cout << "\033[32mTest case 1 passed!\033[0m" << std::endl;
    } else {
        std::cout << "\033[31mTest case 1 failed!\033[0m" << std::endl;
        std::cout << "Expected: " << expectedResult1 << std::endl;
    }

    // 测试用例2: 空文本
    std::string text2 = "";
    std::string result2 = client->addPunctuation(text2);

    std::cout << "Test case 2:" << std::endl;
    std::cout << "  Origin text: " << text2 << std::endl;
    std::cout << "  Result text: " << result2 << std::endl;
    if (result2 == "") {
        std::cout << "\033[32mTest case 2 passed!\033[0m" << std::endl;
    } else {
        std::cout << "\033[31mTest case 2 failed!\033[0m" << std::endl;
    }

    // 测试用例3: 英文文本，当前无法通过，但也不急支持
    std::string text3 = "hello world how are you today";
    std::string result3 = client->addPunctuation(text3);

    std::cout << "Test case 3:" << std::endl;
    std::cout << "  Origin text: " << text3 << std::endl;
    std::cout << "  Result text: " << result3 << std::endl;
    std::string expectedResult3 = "hello world, how are you today?";
    if (result3 == expectedResult3) {
        std::cout << "\033[32mTest case 3 passed!\033[0m" << std::endl;
    } else {
        std::cout << "\033[33mTest case 3 failed!\033[0m" << std::endl;
        std::cout << "Expected: " << expectedResult3 << std::endl;
    }

    // 测试用例4: 包含标点的文本，当前无法通过，但也不急支持
    std::string text4 = "你好吗？我想我们应该去吃饭了。";
    std::string result4 = client->addPunctuation(text4);

    std::cout << "Test case 4:" << std::endl;
    std::cout << "  Origin text: " << text4 << std::endl;
    std::cout << "  Result text: " << result4 << std::endl;
    std::string expectedResult4 = "你好吗？我想我们应该去吃饭了。";
    if (result4 == expectedResult4) {
        std::cout << "\033[32mTest case 4 passed!\033[0m" << std::endl;
    } else {
        std::cout << "\033[33mTest case 4 failed!\033[0m" << std::endl;
        std::cout << "Expected: " << expectedResult4 << std::endl;
    }

    return 0;
}
