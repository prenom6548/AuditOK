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

#ifndef __KYLIN_AI_GEN_AI_TEXT_TOOL_CHOICE_H__
#define __KYLIN_AI_GEN_AI_TEXT_TOOL_CHOICE_H__

#include <common/macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Tool Choice interface suite.
 *
 * Modern LLM platforms such as OpenAI promote the Function Calling concept into a broader Tool Choice abstraction. The
 * change reflects richer capabilities:
 * 1. Wider tool coverage: not only structured function calls but also code interpreters, file retrieval, web search,
 *    and more.
 * 2. Precise control: developers can steer whether tools are optional, disabled, or strictly required.
 * 3. Clear semantics: "Tool Choice" better expresses that the model selects from multiple registered tools at runtime.
 *
 * The SDK follows OpenAI's Tool Choice specification to stay compatible with common AI backends. A model may request
 * one of the tools you registered ahead of time. Each tool declares its parameters via JSON Schema. When the model
 * decides to call a tool, the SDK triggers your callback, you execute the business logic, and then submit the tool
 * result for the model to continue the conversation.
 *
 * Prototype example:
 * {"name":"getWeather","parameters":{"type":"object","properties":{"city":{"type":"string"}}}}
 *
 * Call flow overview:
 *
 * 1. Register tools
 *    genai_text_register_tool() ----------------------+
 *                                                     |
 * 2. Configure tool choice                            |
 *    genai_text_create_tool_choice_config()           |
 *    genai_text_tool_choice_config_set_specific_tool()|
 *    genai_text_set_tool_choice_config()              |
 *                                                     |
 * 3. Install the handler                              |
 *    genai_text_set_tool_handler()                    |
 *                                                     |
 * 4. Run chat                                         |
 *    genai_text_chat_async()                          |
 *         |                                           |
 *         | (model decides to call a tool?)           |
 *         v                                           |
 *    GenAiToolHandler callback <---------------------+
 *         | (app executes business logic)
 *         v
 *    genai_text_submit_tool_result()
 *         |
 *         | (model resumes generation)
 *         v
 *    Final response
 *
 * Usage examples:
 *
 * Example 1: Register a tool and allow automatic invocation
 * ```
 * const char* weather_schema = "{\"type\":\"object\",\"properties\":{\"location\":{\"type\":\"string\"}}}";
 * genai_text_register_tool(session, "get_current_weather",
 * GENAI_TEXT_TOOL_TYPE_FUNCTION, weather_schema);
 *
 * GenAiToolChoiceConfig* config = genai_text_create_tool_choice_config(GENAI_TEXT_TOOL_CHOICE_AUTO);
 * genai_text_set_tool_choice_config(session, config);
 * genai_text_destroy_tool_choice_config(config);
 * genai_text_set_tool_handler(session, tool_callback, user_data);
 * ```
 *
 * Example 2: Force a specific tool
 * ```
 * genai_text_register_tool(session, "get_current_weather", GENAI_TEXT_TOOL_TYPE_FUNCTION, weather_schema);
 * genai_text_register_tool(session, "search_web", GENAI_TEXT_TOOL_TYPE_FUNCTION, search_schema);
 *
 * GenAiToolChoiceConfig* config =
 * genai_text_create_tool_choice_config(GENAI_TEXT_TOOL_CHOICE_AUTO);
 * genai_text_tool_choice_config_set_specific_tool(config, "get_current_weather");
 * genai_text_set_tool_choice_config(session, config);
 * genai_text_destroy_tool_choice_config(config);
 * ```
 *
 * Example 3: Disable tool calls entirely
 * ```
 * GenAiToolChoiceConfig* config = genai_text_create_tool_choice_config(GENAI_TEXT_TOOL_CHOICE_NONE);
 * genai_text_set_tool_choice_config(session, config);
 * genai_text_destroy_tool_choice_config(config);
 * ```
 *
 * Example 4: Require at least one tool invocation
 * ```
 * GenAiToolChoiceConfig* config = genai_text_create_tool_choice_config(GENAI_TEXT_TOOL_CHOICE_REQUIRED);
 * genai_text_set_tool_choice_config(session, config);
 * genai_text_destroy_tool_choice_config(config);
 * ```
 *
 * Example 5: Implement the handler
 * ```
 * void tool_callback(const char *tool_name, const char *arguments_json, void *user_data) {
 *     if (strcmp(tool_name, "get_current_weather") == 0) { *     const char* weather_result = "{\"temperature\": 25,
 * \"condition\": \"sunny\"}"; genai_text_submit_tool_result(session, weather_result); } else if (strcmp(tool_name,
 * "search_web") == 0) { const char* search_result = "{\"results\": [\"result1\", \"result2\"]}";
 *         genai_text_submit_tool_result(session, search_result);
 *     }
 * }
 * ```
 */

typedef struct _GenAiTextSession GenAiTextSession;

/**
 * Callback prototype invoked when the model requests one or more tools.
 *
 * @param tools_json     JSON array string containing tool call requests.
 *                       Each element contains:
 *                       - "name": Tool name registered earlier via @ref genai_text_register_tool.
 *                       - "call_id": Unique identifier for this tool call, used when submitting results via @ref
 *                         genai_text_submit_tool_result.
 *                       - "arguments": JSON object representing input arguments validated against the tool's schema.
 *                       Example:
 *                       [
 *                         {
 *                           "name": "get_current_weather",
 *                           "call_id": "call_abc123",
 *                           "arguments": "{\"location\": \"Beijing\"}"
 *                         }
 *                       ]
 * @param user_data      Opaque pointer you supplied via @ref genai_text_set_tool_handler.
 */
typedef void (*GenAiToolHandler)(const char *tools_json, void *user_data);

/**
 * Tool type enumeration.
 */
typedef enum {
    /**
     * Traditional Function Calling tool (OpenAI `function` type).
     */
    GENAI_TEXT_TOOL_TYPE_FUNCTION = 0
} GenAiToolType;

/**
 * Tool-choice modes.
 */
typedef enum {
    /**
     * Automatic (default): the model decides when and which tool to invoke.
     * You may still force a specific tool via @ref genai_text_tool_choice_config_set_specific_tool. Mirrors OpenAI
     * "auto".
     */
    GENAI_TEXT_TOOL_CHOICE_AUTO = 0,

    /**
     * None: disable tool usage altogether. Matches OpenAI "none".
     */
    GENAI_TEXT_TOOL_CHOICE_NONE,

    /**
     * Required: the model must call at least one registered tool before responding. Matches OpenAI "required".
     */
    GENAI_TEXT_TOOL_CHOICE_REQUIRED
} GenAiToolChoiceMode;

// Forward declaration owned by the SDK.
typedef struct _GenAiToolChoiceConfig GenAiToolChoiceConfig;

/**
 * Create a tool-choice configuration object.
 *
 * @param mode Initial mode that governs how the model may call tools.
 * @return A new configuration that must be destroyed with @ref genai_text_destroy_tool_choice_config.
 */
AISDK_EXTERN
GenAiToolChoiceConfig *genai_text_create_tool_choice_config(GenAiToolChoiceMode mode);

/**
 * Force the model to call a specific tool.
 *
 * Equivalent to OpenAI's `{"type":"function","function":{"name":"tool_name"}}`.
 *
 * @param config    Tool-choice configuration.
 * @param tool_name Name of a previously registered tool; NULL or empty clears the rule.
 */
AISDK_EXTERN
void genai_text_tool_choice_config_set_specific_tool(GenAiToolChoiceConfig *config, const char *tool_name);

/**
 * Destroy a tool-choice configuration.
 *
 * @param config Configuration created earlier by @ref genai_text_create_tool_choice_config.
 */
AISDK_EXTERN
void genai_text_destroy_tool_choice_config(GenAiToolChoiceConfig **config);

/**
 * Register a tool so that the model can call it.
 *
 * Provide a unique name, a type, and a JSON Schema describing the parameters.
 * After registration the model may request the tool during generation and your handler will be invoked.
 *
 * Example:
 *  name: "getWeather"
 *  type: GENAI_TEXT_TOOL_TYPE_FUNCTION
 *  schema:
 * {"name":"getWeather","parameters":{"type":"object","properties":{"city":{"type":"string"}}}}
 *
 * JSON Schema highlights (compatible with OpenAI Function Tool):
 *  - name (string, required): Must match @p tool_name.
 *  - description (string, optional): Natural-language description that helps the model understand when to call the *    tool.
 *  - parameters (object, required): Schema describing inputs.
 *      * type: usually "object".
 *      * properties: object map where each key is a parameter and the value is its schema (type, description, enum,
 *        items, default, etc.).
 *      * required: string array for mandatory parameters.
 *      * additionalProperties: set to false to reject undeclared fields.
 *  - strict (boolean, optional): When true the backend enforces the schema strictly.
 *  - Any Draft 7 keywords (oneOf, allOf, format, minItems, ...) are passed through.
 *
 * @param session     Text session.
 * @param tool_name   Unique name (snake_case or camelCase recommended).
 * @param tool_type   Currently only @ref GENAI_TEXT_TOOL_TYPE_FUNCTION is supported.
 * @param json_schema JSON Schema string describing the tool parameters.
 * @return 0 on success, negative on error.
 */
AISDK_EXTERN
int genai_text_register_tool(GenAiTextSession *session, const char *tool_name, GenAiToolType tool_type,
                             const char *json_schema);

/**
 * Remove all tools registered on the session.
 *
 * Tool-choice configuration remains untouched, but no tools are available until
 * you register them again.
 *
 * @param session Text session.
 */
AISDK_EXTERN
int genai_text_clear_tools(GenAiTextSession *session);

/**
 * Install the handler invoked when the model requests a tool.
 *
 * Passing NULL removes the handler. Keep the callback lightweight; offload expensive work to your own threads if
 * needed.
 *
 * @param session   Text session.
 * @param handler   Callback pointer or NULL.
 * @param user_data User data forwarded to every invocation.
 */
AISDK_EXTERN
void genai_text_set_tool_handler(GenAiTextSession *session, GenAiToolHandler handler, void *user_data);

/**
 * Submit the tool execution result as JSON so the model can continue responding.
 *
 * Return objects that match your business contract, for example
 * `{"answer":"Cloudy","temperature":25}` or `{"error":{"code":"timeout","message":"..."} }`.
 *
 * @param session     Text session.
 * @param tool_calls_json UTF-8 JSON string.
 * @param result_json UTF-8 JSON string.
 */
AISDK_EXTERN
int genai_text_submit_tool_result(GenAiTextSession *session, const char *tool_calls_json, const char *result_json);

/**
 * Attach the tool-choice configuration to the session.
 *
 * @param session Text session.
 * @param config  Non-null configuration describing tool policies.
 */
AISDK_EXTERN
void genai_text_set_tool_choice_config(GenAiTextSession *session, const GenAiToolChoiceConfig *config);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_TOOL_CHOICE_H__
