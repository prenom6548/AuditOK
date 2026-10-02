/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <filesystem>
#include <fstream>
#include <utility>

#include "WebSocketClient.h"
#include "logger.h"
#include "util.h"

class BaiduContinuousAsrTool : public hv::WebSocketClient {
public:
    explicit BaiduContinuousAsrTool() : hv::WebSocketClient(loop_ = std::make_shared<hv::EventLoop>()) {}

    ~BaiduContinuousAsrTool() override = default;

    bool create() {
        reconn_setting_t reconn;
        reconn_setting_init(&reconn);
        reconn.min_delay = 100;
        reconn.max_delay = 1000;
        reconn.delay_policy = 1;

        std::string appId_{""};
        std::string apiKey_{""};

        authorizationJson_["type"] = "START";
        authorizationJson_["data"]["appid"] = atoi(appId_.c_str());
        authorizationJson_["data"]["appkey"] = apiKey_;
        authorizationJson_["data"]["dev_pid"] = 15372;
        authorizationJson_["data"]["cuid"] = "kylin-ai-subsystem";
        authorizationJson_["data"]["format"] = "pcm";
        authorizationJson_["data"]["sample"] = 16000;

        hv::WebSocketClient::close();
        connected_ = false;
        authorized_ = false;

        hv::WebSocketClient::onopen = [this]() {
            LOGI("onopen");
            connected_ = true;
        };
        hv::WebSocketClient::onmessage = [this](const std::string &message) { LOGI("onmessage:", message); };
        hv::WebSocketClient::onclose = [this]() {
            LOGI("onclose");
            connected_ = false;
            authorized_ = false;
        };

        hv::WebSocketClient::setReconnect(&reconn);
        hv::WebSocketClient::setPingInterval(10000);
        int ret = hv::WebSocketClient::open("wss://vop.baidu.com/realtime_asr?sn=kylin-ai-subsystem");
        if (ret != 0) {
            LOGD("open failed", ret);
            return ret;
        }
        return true;
    }

    bool destroy() {
        hv::WebSocketClient::close();
        connected_ = false;
        authorized_ = false;
        return true;
    }

    bool asr(const char *data, size_t size) {
        if (connected_) {
            if (!authorized_) {
                hv::WebSocketClient::send(authorizationJson_.dump());
                // 认为发送完成就授权成功，除非接到云端鉴权失败的消息
                authorized_ = true;
            }
            int ret = hv::WebSocketClient::send(data, size, WS_OPCODE_BINARY);
            if (ret < 0) {
                LOGD("send failed", ret, size);
                return false;
            }
        }
        return true;
    }

    bool asr(const std::vector<char> &audioData) { return asr(audioData.data(), audioData.size()); }

    void setAsrTimeoutSec(int sec) const {
        if (channel) {
            channel->setWriteTimeout(sec * 1000);
        }
    }

private:
    hv::EventLoopPtr loop_{std::make_shared<hv::EventLoop>()};
    hv::Json authorizationJson_;
    std::atomic_bool connected_{false};
    std::atomic_bool authorized_{false};
};

std::vector<char> readAudioData(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file:", filePath);
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> audioData(fileSize);
    file.read(audioData.data(), fileSize);
    return audioData;
}

void testContinuousAsrTool() {

    std::atomic_bool shouldContinue = true;

    std::thread([&shouldContinue] {
        while (std::getchar() == '\n') {
            shouldContinue = false;
            break;
        }
    }).detach();

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "16k.pcm";
    if (not fs::exists(path)) {
        LOGE("error");
        return;
    }
    std::vector<char> audioData = readAudioData(path.string());

    BaiduContinuousAsrTool tool;
    bool success = tool.create();
    if (!success) {
        LOGD("create failed");
        return;
    }
    // 超时时间测试结果记录(单位：秒)：
    // 设置时间：20，实际时间：25
    //           20            24
    //           20            23
    //           20            24
    //           20            23

    // 设置时间：10，实际时间：12
    //           10            12
    //           10            14
    //           10            13
    //           10            13

    // 设置时间： 5，实际时间： 7
    //            5             7
    //            5             8
    //            5             7
    //            5             8

    tool.setAsrTimeoutSec(5);

    while (shouldContinue) {
        char *data = audioData.data();
        size_t size = audioData.size();
        for (size_t i = 0; i < size; i += 1280) {
            // 余下数据未处理
            tool.asr(data + i, 1280);
            baidu_speech_util::sleepForMs(40);
        }
    }

    tool.destroy();
}

int main() {
    testContinuousAsrTool();

    return 0;
}
