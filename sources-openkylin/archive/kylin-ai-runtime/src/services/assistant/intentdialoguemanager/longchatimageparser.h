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

#ifndef SERVICES_ASSISTANT_INTENT_DIALOGUE_MANAGER_LONG_CHAT_IMAGE_PARSER_H
#define SERVICES_ASSISTANT_INTENT_DIALOGUE_MANAGER_LONG_CHAT_IMAGE_PARSER_H

#include <kylin-ai/common/error.h>

#include "longchatimageparser/processer/longimageparser.h"
#include "longchatimageparser/utils/processresult.h"

class LongChatImageParser {
public:
    struct LongChatImageParserResult {
        int errorCode{AI_COMMON_SUCCESS};
        std::string errorMessage;
        std::string result;
    };

public:
    LongChatImageParser();

    ~LongChatImageParser();

    bool init();

    LongChatImageParserResult extractImageContents(const std::string &imagePath) const;

private:
    std::shared_ptr<parser::LongImageParser> parser_{nullptr};
};

#endif  // SERVICES_ASSISTANT_INTENT_DIALOGUE_MANAGER_LONG_CHAT_IMAGE_PARSER_H
