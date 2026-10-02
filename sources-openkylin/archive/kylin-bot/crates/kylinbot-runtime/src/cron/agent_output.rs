use crate::agent::AgentRunTrace;
use serde::{Deserialize, Serialize};

const CRON_OUTPUT_VERSION: &str = "kylin_cron_v1";

#[derive(Debug, Clone, Serialize, Deserialize)]
struct CronAgentOutputV1 {
    message: String,
    #[serde(default, skip_serializing_if = "String::is_empty")]
    thinking: String,
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    tool_calls: Vec<serde_json::Value>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ParsedCronAgentOutput {
    pub message: String,
    pub thinking: String,
    pub tool_calls: Vec<serde_json::Value>,
}

/// Serialize cron agent output. Plain text is used when there is no trace data
/// so existing consumers keep working unchanged.
pub fn format_agent_output(text: &str, trace: &AgentRunTrace) -> String {
    if trace.thinking.is_empty() && trace.tool_calls.is_empty() {
        return text.to_string();
    }

    let payload = CronAgentOutputV1 {
        message: text.to_string(),
        thinking: trace.thinking.clone(),
        tool_calls: trace.tool_calls.clone(),
    };

    serde_json::json!({ CRON_OUTPUT_VERSION: payload }).to_string()
}

/// Parse cron agent output, falling back to plain text for legacy runs.
pub fn parse_agent_output(raw: &str) -> ParsedCronAgentOutput {
    let trimmed = raw.trim();
    if trimmed.is_empty() {
        return ParsedCronAgentOutput {
            message: String::new(),
            thinking: String::new(),
            tool_calls: Vec::new(),
        };
    }

    let Ok(value) = serde_json::from_str::<serde_json::Value>(trimmed) else {
        return ParsedCronAgentOutput {
            message: raw.to_string(),
            thinking: String::new(),
            tool_calls: Vec::new(),
        };
    };

    let Some(obj) = value.get(CRON_OUTPUT_VERSION) else {
        return ParsedCronAgentOutput {
            message: raw.to_string(),
            thinking: String::new(),
            tool_calls: Vec::new(),
        };
    };

    match serde_json::from_value::<CronAgentOutputV1>(obj.clone()) {
        Ok(parsed) => ParsedCronAgentOutput {
            message: parsed.message,
            thinking: parsed.thinking,
            tool_calls: parsed.tool_calls,
        },
        Err(_) => ParsedCronAgentOutput {
            message: raw.to_string(),
            thinking: String::new(),
            tool_calls: Vec::new(),
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::agent::AgentRunTrace;

    #[test]
    fn format_agent_output_uses_plain_text_without_trace() {
        let trace = AgentRunTrace::default();
        assert_eq!(format_agent_output("hello", &trace), "hello");
    }

    #[test]
    fn format_and_parse_agent_output_roundtrip() {
        let trace = AgentRunTrace {
            thinking: "need weather tool".into(),
            tool_calls: vec![serde_json::json!({
                "id": "call_1",
                "name": "weather",
                "args": "{\"city\":\"Tianjin\"}",
                "output": "sunny"
            })],
        };
        let raw = format_agent_output("tomorrow is sunny", &trace);
        let parsed = parse_agent_output(&raw);
        assert_eq!(parsed.message, "tomorrow is sunny");
        assert_eq!(parsed.thinking, "need weather tool");
        assert_eq!(parsed.tool_calls.len(), 1);
    }

    #[test]
    fn parse_agent_output_falls_back_to_plain_text() {
        let parsed = parse_agent_output("legacy output");
        assert_eq!(parsed.message, "legacy output");
        assert!(parsed.thinking.is_empty());
        assert!(parsed.tool_calls.is_empty());
    }
}
