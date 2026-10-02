#include "embeddingplugin.h"

#include <iostream>

int main()
{
    EmbeddingEnginePlugin p;
    std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> engine = p.createEmbeddingEngine();
    // 由于plugin只是起一个中介的作用，所以这里测试代码较少
    std::string info = engine->modelInfo();
    std::cout << "modelInfo:" << info << std::endl;
    std::cout << "Hello world!" << std::endl;
    return 0;
}