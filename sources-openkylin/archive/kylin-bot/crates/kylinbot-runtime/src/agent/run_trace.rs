use kylinbot_providers::ChatMessage;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Debug, Clone, Default, Serialize, Deserialize, PartialEq, Eq)]
pub struct AgentRunTrace {
    pub thinking: String,
    pub tool_calls: Vec<serde_json::Value>,
}

/// Extract thinking and tool-call snapshots from a completed agent history.
/// The output format matches kylin-aiassistant task-mode `taskToolCalls`.
pub fn extract_agent_run_trace(history: &[ChatMessage]) -> AgentRunTrace {
    let mut thinking_parts = Vec::new();
    let mut call_order: Vec<String> = Vec::new();
    let mut calls: HashMap<String, (String, String, String)> = HashMap::new();

    for msg in history {
        match msg.role.as_str() {
            "assistant" => {
                let Ok(value) = serde_json::from_str::<serde_json::Value>(&msg.content) else {
                    continue;
                };

                if let Some(reasoning) = value
                    .get("reasoning_content")
                    .and_then(|item| item.as_str())
                    .filter(|text| !text.is_empty())
                {
                    thinking_parts.push(reasoning.to_string());
                }

                let Some(tool_calls) = value.get("tool_calls").and_then(|item| item.as_array()) else {
                    continue;
                };

                for call in tool_calls {
                    let id = call
                        .get("id")
                        .and_then(|item| item.as_str())
                        .unwrap_or_default()
                        .to_string();
                    if id.is_empty() {
                        continue;
                    }
                    let name = call
                        .get("name")
                        .and_then(|item| item.as_str())
                        .unwrap_or_default()
                        .to_string();
                    let args = match call.get("arguments") {
                        Some(serde_json::Value::String(text)) => text.clone(),
                        Some(other) => other.to_string(),
                        None => String::new(),
                    };
                    call_order.push(id.clone());
                    calls.insert(id, (name, args, String::new()));
                }
            }
            "tool" => {
                let Ok(value) = serde_json::from_str::<serde_json::Value>(&msg.content) else {
                    continue;
                };
                let id = value
                    .get("tool_call_id")
                    .and_then(|item| item.as_str())
                    .unwrap_or_default()
                    .to_string();
                if id.is_empty() {
                    continue;
                }
                let output = value
                    .get("content")
                    .and_then(|item| item.as_str())
                    .unwrap_or_default()
                    .to_string();
                if let Some(entry) = calls.get_mut(&id) {
                    entry.2 = output;
                }
            }
            _ => {}
        }
    }

    let tool_calls = call_order
        .into_iter()
        .filter_map(|id| {
            calls.get(&id).map(|(name, args, output)| {
                serde_json::json!({
                    "id": id,
                    "name": name,
                    "args": args,
                    "output": output,
                })
            })
        })
        .collect();

    AgentRunTrace {
        thinking: thinking_parts.join("\n"),
        tool_calls,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn extract_agent_run_trace_collects_thinking_and_tool_calls() {
        let history = vec![
            ChatMessage::assistant(
                serde_json::json!({
                    "content": "checking weather",
                    "reasoning_content": "need weather lookup",
                    "tool_calls": [{
                        "id": "call_1",
                        "name": "weather",
                        "arguments": "{\"city\":\"Tianjin\"}"
                    }]
                })
                .to_string(),
            ),
            ChatMessage::tool(
                serde_json::json!({
                    "tool_call_id": "call_1",
                    "content": "sunny"
                })
                .to_string(),
            ),
            ChatMessage::assistant("Tomorrow Tianjin will be sunny."),
        ];

        let trace = extract_agent_run_trace(&history);
        assert_eq!(trace.thinking, "need weather lookup");
        assert_eq!(trace.tool_calls.len(), 1);
        assert_eq!(trace.tool_calls[0]["name"], "weather");
        assert_eq!(trace.tool_calls[0]["output"], "sunny");
    }
}
