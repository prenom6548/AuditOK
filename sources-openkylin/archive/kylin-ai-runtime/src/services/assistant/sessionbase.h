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

#ifndef SERVICES_ASSISTANT_SESSION_BASE_H
#define SERVICES_ASSISTANT_SESSION_BASE_H

#include <kylin-ai/ai-engine/imagegenerationengine.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

namespace assistant {
using TextGenerationEngine = ai_engine::gen_ai::nlp::AbstractTextGenerationEngine;
using ImageGenerationEngine = ai_engine::gen_ai::vision::AbstractImageGenerationEngine;
using ImageProcessEngine = ai_engine::core_ai::vision::AbstractImageProcessEngine;
}  // namespace assistant

#endif  // SERVICES_ASSISTANT_SESSION_BASE_H
