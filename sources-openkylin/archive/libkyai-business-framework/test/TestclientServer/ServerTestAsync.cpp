#include <unistd.h>

#include <thread>

#include "scene.h"

void workMethod2(const std::string& taskId) {
    PRINT_INFO("workMethod2 start, input: %s.\n", taskId.c_str());
    sleep(2);
    Response output = Response::success();
    output.add("p1", 11);
    output.add("p2", "hehe");
    ASYNC_RESPONSE(taskId, output);
    PRINT_INFO("workMethod2 end.\n");
}

Response handleAsyncMethod(const std::string& taskId, const Request& request) {
    PRINT_DEBUG("method2 taskId: %s, request: %s\n", taskId.c_str(), request.toString().c_str());

    // 异步处理
    std::thread t(workMethod2, taskId);
    t.detach();

    return Response::success();
}

REGISTER_SERVICE_ASYNC_HANDLER("getAsync", handleAsyncMethod)
