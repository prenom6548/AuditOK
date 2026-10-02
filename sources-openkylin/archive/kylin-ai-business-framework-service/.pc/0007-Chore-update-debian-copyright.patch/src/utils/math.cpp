#include "math.h"

#include <cmath>
#include <iostream>

// 将向量归一化
void Math::normalize(std::vector<float> &vec)
{
    if (vec.empty()) {
        return;
    }

    float normNum = norm(vec);
    if (normNum == 0.0) {
        return;
    }

    for (int i = 0; i < vec.size(); ++i) {
        vec[i] = vec[i] / normNum;
    }
}

// 向量取模
float Math::norm(const std::vector<float>& vec)
{
    float norm = 0.0f;
    for (const float& element : vec) {
        norm += element * element;
    }
    norm = std::sqrt(norm);

    return norm;
}

void Math::normalizeTokens(std::vector<int64_t> &tokens, int length)
{
    if (tokens.size() == length) {
        return;
    } else if (tokens.size() > length) {
        tokens[length - 1] = tokens.back();
        tokens.resize(length);
        std::cerr << "Input tokens exceed " << length
                  << ", the tokens will be cropped" << std::endl;
    } else {
        tokens.resize(52, 0);
    }
}
