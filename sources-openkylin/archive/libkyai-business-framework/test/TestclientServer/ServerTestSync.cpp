#include <gtest/gtest.h>

#include "scene.h"

Response handleSyncMethod(const Request& request) {
    PRINT_DEBUG("method1 request: %s\n", request.toString().c_str());
    const std::string& input1 = request.getString("input1");
    PRINT_INFO("method1 request input1: %s\n", input1.c_str());

    return Response::success(nlohmann::json{
        {"aa", 11},
        {"bb", "asdfg"},
        {"cc", 33},
    });
}

REGISTER_SERVICE_HANDLER("getSync", handleSyncMethod)