/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

// std
#include <cstdio>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// glib
#include <gio/gio.h>

// private
#include "chat.h"
#include "config.h"
#include "error.h"
#include "modelinfo.h"
#include "result.h"

namespace {
std::vector<std::string> stringSplit(const std::string &str, const std::string &delim) {
    std::vector<std::string> result;
    size_t begin = 0;
    for (size_t end; end = str.find(delim, begin), end != std::string::npos; begin = end + delim.size()) {
        result.emplace_back(str.substr(begin, end - begin));
    }
    result.emplace_back(str.substr(begin));
    return result;
}
}  // namespace

class Chatbot {
public:
    Chatbot() {
        stdinChannel_ = g_io_channel_unix_new(STDIN_FILENO);
        g_io_channel_set_encoding(stdinChannel_, nullptr, nullptr);
        g_io_channel_set_buffered(stdinChannel_, TRUE);
        g_io_channel_set_flags(stdinChannel_, G_IO_FLAG_NONBLOCK, nullptr);
        g_io_add_watch(stdinChannel_, GIOCondition(G_IO_IN | G_IO_HUP | G_IO_ERR), onStdinEnter, this);

        mainLoop_ = g_main_loop_new(nullptr, false);
        session_ = genai_text_create_session();

        config_ = chat_model_config_create();

        // default
        genai_text_set_model_config(session_, config_);

        int errorCode = genai_text_init_session(session_);
        if (errorCode != AISDK_NO_ERROR) {
            std::string errorMessage =
                "Func genai_text_init_session init error, error code is: " + std::to_string(errorCode);
            throw std::runtime_error(errorMessage.c_str());
        }
        genai_text_result_set_callback(session_, callback, nullptr);
        chatMessage_ = chat_message_create();
    }

    ~Chatbot() {
        g_main_loop_unref(mainLoop_);
        g_io_channel_unref(stdinChannel_);

        chat_model_config_destroy(&config_);
        genai_text_destroy_session(&session_);
        chat_message_destroy(&chatMessage_);
    }

    void run() const { g_main_loop_run(mainLoop_); }

private:
    static gboolean onStdinEnter(GIOChannel *channel, GIOCondition condition, gpointer userData) {
        Chatbot *tool = static_cast<Chatbot *>(userData);
        gchar *line{nullptr};
        if (condition & G_IO_IN) {
            GIOStatus status = g_io_channel_read_line(channel, &line, nullptr, nullptr, nullptr);
            if (status == G_IO_STATUS_NORMAL) {
                std::string input(line);
                if (!input.empty() && input[input.size() - 1] == '\n') {
                    input.erase(input.size() - 1);
                }
                tool->control(input);
                g_free(line);
            }
        }
        return TRUE;
    }

    void control(const std::string &input) const {
        if (input.empty()) {
            std::fprintf(stdout, "input message is empty.\n");
        } else if (input == "quit") {
            g_main_loop_quit(mainLoop_);
        } else if (input.substr(0, 5) == "top_k") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: top_k <top_k>\n");
                return;
            }
            int topK = std::stoi(args[1]);
            chat_model_config_set_top_k(config_, topK);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 6) == "stream") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: stream <stream>\n");
                return;
            }
            bool stream = args[1] == "true";
            chat_model_config_set_stream(config_, stream);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 10) == "web_search") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: web_search <web_search>\n");
                return;
            }
            bool webSearch = args[1] == "true";
            chat_model_config_set_web_search_enabled(config_, webSearch, nullptr);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 8) == "thinking") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: thinking <thinking>\n");
                return;
            }
            bool thinking = args[1] == "true";
            chat_model_config_set_thinking_enabled(config_, thinking, nullptr);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 5) == "model") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: model <model>\n");
                return;
            }
            std::string newModel = args[1];
            if (newModel == "nullptr") {
                // do nothing
            } else if (newModel == "baidu") {
                newModel = "百度-ERNIE-Bot-4";
            } else if (newModel == "ds-v3") {
                newModel = "DeepSeek-V3";
            } else if (newModel == "ds-r1") {
                newModel = "DeepSeek-R1";
            } else if (newModel == "free-qwen") {
                newModel = "通义千问（Qwen-Plus）试用版";
            } else if (newModel == "free-32b") {
                newModel = "通义千问（QwQ-32B）试用版";
            } else if (newModel == "free-ds-r1") {
                newModel = "DeepSeek-R1 试用版";
            } else if (newModel == "free-ds-v3") {
                newModel = "DeepSeek-V3 试用版";
            } else if (newModel == "xunfei") {
                newModel = "讯飞-Spark Max";
            } else if (newModel == "local") {
                newModel = "Qwen-2.5-3b_1.0";
            } else {
                std::fprintf(stderr, "unknown model %s\n", newModel.c_str());
            }
            if (newModel == "nullptr") {
                genai_text_set_model_config(session_, nullptr);
            } else {
                chat_model_config_set_name(config_, newModel.c_str());
                genai_text_set_model_config(session_, config_);
            }
        } else if (input == "get") {
            std::shared_ptr<int> errorCode = std::make_shared<int>(AI_COMMON_SUCCESS);
            const TextModelList *modelList = genai_text_get_model_list(session_, errorCode.get());
            if (errorCode != nullptr && *errorCode != AI_COMMON_SUCCESS) {
                std::fprintf(stderr, "error code: %d\n", *errorCode);
                return;
            }
            for (int i = 0; i < genai_text_model_list_get_count(modelList); i++) {
                const TextModelInfo *modelInfo = genai_text_model_list_get_model_info(modelList, i, nullptr);
                std::fprintf(stdout, "model name: %s\n", genai_text_model_info_get_model_name(modelInfo, nullptr));
                std::fprintf(stdout, "display name: %s\n", genai_text_model_info_get_display_name(modelInfo, nullptr));
                std::fprintf(stdout, "deploy type: %d\n", genai_text_model_info_get_deploy_type(modelInfo, nullptr));
                std::fprintf(stdout, "supports web search: %d\n",
                             genai_text_model_info_supports_web_search(modelInfo, nullptr));
                std::fprintf(stdout, "supports thinking: %d\n",
                             genai_text_model_info_supports_thinking(modelInfo, nullptr));
                std::fprintf(stdout, "supports thinking toggle: %d\n",
                             genai_text_model_info_supports_thinking_toggle(modelInfo, nullptr));
                std::fprintf(stdout, "brand: %s\n", genai_text_model_info_get_brand(modelInfo, nullptr));
                std::fprintf(stdout, "\n");
            }
        } else if (input == "clear") {
            genai_text_clear_chat_history_messages(session_);
        } else if (input == "history") {
            chat_message_add_system_message(chatMessage_, "");
            chat_message_add_user_message(chatMessage_, "一加一等于几？");
            chat_message_add_assistant_message(chatMessage_, "一加一等于二");
            chat_message_add_user_message(chatMessage_, "你说的不对");
            genai_text_chat_with_history_messages_async(session_, chatMessage_);
        } else if (input.substr(0, 11) == "temperature") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: temperature <temperature>\n");
                return;
            }
            double temperature = std::stod(args[1]);
            chat_model_config_set_temperature(config_, temperature);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 5) == "top_p") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: top_p <top_p>\n");
                return;
            }
            double topP = std::stod(args[1]);
            chat_model_config_set_top_p(config_, topP);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 10) == "max_tokens") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: max_tokens <max_tokens>\n");
                return;
            }
            int maxTokens = std::stoi(args[1]);
            chat_model_config_set_max_completion_tokens(config_, maxTokens);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 16) == "presence_penalty") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: presence_penalty <presence_penalty>\n");
                return;
            }
            double presencePenalty = std::stod(args[1]);
            chat_model_config_set_presence_penalty(config_, presencePenalty);
            genai_text_set_model_config(session_, config_);
        } else if (input.substr(0, 1) == "n") {
            std ::vector<std::string> args = stringSplit(input, " ");
            if (args.size() != 2) {
                std::fprintf(stderr, "usage: n <n>\n");
                return;
            }
            int n = std::stoi(args[1]);
            chat_model_config_set_n(config_, n);
            genai_text_set_model_config(session_, config_);
        } else {
            std::fprintf(stdout, "----------------------------------------\n");
            std::fprintf(stdout, "question:\n");
            std::fprintf(stdout, "%s\n", input.c_str());
            std::fprintf(stdout, "answer:\n");
            genai_text_chat_async(session_, input.c_str());
        }
    }

    static void callback(ChatResult *result, void *user_data) {
        if (int errorCode = chat_result_get_error_code(result); errorCode != AISDK_NO_ERROR) {
            std::fprintf(stderr, "error code is %d\n", errorCode);
            std::fprintf(stderr, "error message is %s\n", chat_result_get_error_message(result));
            return;
        }
        std::fprintf(stdout, "%s: %s\n", chat_result_get_model_name(result), chat_result_get_reasoning_message(result));
        std::fprintf(stdout, "%s: %s\n", chat_result_get_model_name(result), chat_result_get_assistant_message(result));
        if (chat_result_get_is_end(result)) {
            std::fprintf(stdout, "\n");
            std::fprintf(stdout, "========================================\n");
        }
        const ChatResultSearchInfo *searchInfo = chat_result_get_search_info(result, nullptr);
        if (searchInfo == nullptr) {
            return;
        }
        size_t count = chat_result_search_info_get_count(searchInfo, nullptr);
        for (size_t i = 0; i < count; i++) {
            const ChatResultSearchItem *item = chat_result_search_info_get_item(searchInfo, i, nullptr);
            if (item == nullptr) {
                continue;
            }
            size_t index = chat_result_search_item_get_index(item, nullptr);
            std::string title = chat_result_search_item_get_title(item, nullptr);
            std::string url = chat_result_search_item_get_url(item, nullptr);
            std::fprintf(stdout, "search result %lu: %s %s\n", index, title.c_str(), url.c_str());
        }
    }

private:
    GIOChannel *stdinChannel_{nullptr};
    GMainLoop *mainLoop_{nullptr};

    ChatModelConfig *config_{nullptr};
    GenAiTextSession *session_{nullptr};
    ChatMessage *chatMessage_{nullptr};
};

int main() {
    try {
        Chatbot chatbot{};
        chatbot.run();
    } catch (std::exception &exception) {
        std::fprintf(stderr, "Exception: %s\n", exception.what());
        return -1;
    }
    return 0;
}
