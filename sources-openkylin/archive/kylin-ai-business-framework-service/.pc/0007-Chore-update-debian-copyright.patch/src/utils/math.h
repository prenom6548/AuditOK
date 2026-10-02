#ifndef MATH_H
#define MATH_H

#include <cstdint>
#include <vector>

class Math
{
public:
    static void normalize(std::vector<float> &vec);
    static float norm(const std::vector<float> &matrix);
    static void normalizeTokens(std::vector<int64_t> &tokens, int length);
};

#endif // MATH_H
