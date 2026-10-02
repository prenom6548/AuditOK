#ifndef AIENGINERESULT_H
#define AIENGINERESULT_H

#include <string.h>
#include "aiengineerror.h"

namespace ai_engine {
namespace result {

struct AiEngineBaseResult {
    bool success;
    ai_engine::EngineError error;
};

struct AiEngineModelResult : public AiEngineBaseResult {
    std::string modelName;
};

}

}

#endif