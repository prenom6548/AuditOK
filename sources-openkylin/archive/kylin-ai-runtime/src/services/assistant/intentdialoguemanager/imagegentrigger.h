/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef SERVICES_ASSISTANT_IMAGE_GEN_TRIGGER_H
#define SERVICES_ASSISTANT_IMAGE_GEN_TRIGGER_H

#include <string>
#include <vector>

class ImageGenTrigger {
public:
    ImageGenTrigger() = default;

    ~ImageGenTrigger() = default;

    /**
     * @brief 判断是否触发文生图逻辑 (优化版：按句检测)
     */
    bool shouldTriggerImageGeneration(const std::string& input);

private:
    // --- 核心辅助函数：单句检测 ---
    bool checkSingleSentence(const std::string& sentence) const;

    // --- 核心辅助函数：按标点分句 ---
    std::vector<std::string> splitIntoSentences(const std::string& text);

private:
    const std::vector<std::string> wordJudgment_{"绘制", "创造", "生成", "打造", "制作", "创作", "显现",
                                                 "呈现", "产生", "输出", "创建", "设计", "描绘", "展示",
                                                 "需要", "想要", "画",   "构思", "表现", "开发", "可视化",
                                                 "模拟", "想看", "形成", "渲染", "合成", "图示", "绘画"};

    const std::vector<std::string> mainFont_{"图片", "图像", "插图", "照片", "画作", "海报", "草图"};
};

#endif  // SERVICES_ASSISTANT_IMAGE_GEN_TRIGGER_H
