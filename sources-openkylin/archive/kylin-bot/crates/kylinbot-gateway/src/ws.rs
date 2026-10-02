//! WebSocket agent chat handler.
//!
//! Connect: `ws://host:port/ws/chat?session_id=ID&name=My+Session`
//!
//! Protocol:
//! ```text
//! Server -> Client: {"type":"session_start","session_id":"...","name":"...","resumed":true,"message_count":42}
//! Client -> Server: {"type":"message","content":"Hello"}
//! Server -> Client: {"type":"chunk","content":"Hi! "}
//! Server -> Client: {"type":"tool_call","name":"shell","args":{...}}
//! Server -> Client: {"type":"tool_result","name":"shell","output":"..."}
//! Server -> Client: {"type":"done","full_response":"..."}
//! ```
//!
//! ## Tool approvals
//!
//! When supervised-mode tool calls hit the `ApprovalManager`, the server
//! emits an `approval_request` and pauses the tool loop until the client
//! responds. Mirrors the Telegram inline-keyboard / CLI Y/N/A pattern,
//! over the WS frame transport.
//!
//! ```text
//! Server -> Client: {
//!     "type": "approval_request",
//!     "request_id": "<uuid>",
//!     "tool": "shell",
//!     "arguments_summary": "command: git status",
//!     "timeout_secs": 120
//! }
//! Client -> Server: {
//!     "type": "approval_response",
//!     "request_id": "<uuid>",
//!     "decision": "approve" | "deny" | "always"
//! }
//! ```
//!
//! `approve` runs the tool once, `always` adds the tool to the session
//! allowlist for the rest of the conversation, `deny` returns a structured
//! error to the model. When no client is connected, or the client
//! disconnects mid-prompt, the tool call is auto-denied after `timeout_secs`.
//!
//! ### `arguments_summary` security boundary
//!
//! `arguments_summary` is a human-readable string the runtime synthesises
//! for the operator (e.g. `"command: git status"`, `"path: /etc/hosts"`).
//! It is render-only; the operator's approve/deny choice attaches to the
//! `request_id`, never to the summary string. The runtime must not echo
//! any `#[secret]` or `#[derived_from_secret]` field (auth tokens, API
//! keys, OAuth secrets) into the summary. The agent's tool loop runs
//! tool args through `kylinbot_runtime::approval::summarize_args` before
//! the request reaches this transport; do not stringify raw args here.
//!
//! Query params:
//! - `session_id` — resume or create a session (default: new UUID)
//! - `name` — optional human-readable label for the session
//! - `token` — bearer auth token (alternative to Authorization header)

use super::AppState;
use super::resolve_fallback_model_route;
use crate::MAX_WS_MESSAGE_SIZE;
use crate::ws_approval::{PendingApprovals, WsApprovalChannel, new_pending_approvals};
use axum::{
    extract::{
        Query, State, WebSocketUpgrade,
        ws::{Message, WebSocket},
    },
    http::{HeaderMap, header},
    response::IntoResponse,
};
use base64::{Engine as _, engine::general_purpose::STANDARD};
use futures_util::{SinkExt, StreamExt};
use kylinbot_api::channel::ChannelApprovalResponse;
use kylinbot_api::media::MediaAttachment;
use kylinbot_api::provider::ConversationMessage;
use kylinbot_config::schema::Config;
use serde::Deserialize;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::time::Duration;
use tokio_util::sync::CancellationToken;
use tracing::debug;

const WS_APPROVAL_TIMEOUT_SECS: u64 = 120;

const MAX_WS_PAYLOAD_BYTES: usize = 80 * 1024 * 1024;

fn load_persisted_session_messages(
    backend: &dyn kylinbot_infra::session_backend::SessionBackend,
    session_key: &str,
) -> Vec<kylinbot_providers::ChatMessage> {
    let mut messages = backend.load(session_key);
    let healed =
        kylinbot_runtime::agent::history_pruner::remove_orphaned_tool_messages_quiet(&mut messages);
    if healed > 0 {
        if let Err(e) = backend.replace_messages(session_key, &messages) {
            tracing::warn!(
                session_key,
                count = healed,
                "Failed to persist healed WS session history: {e}"
            );
        } else {
            tracing::info!(
                session_key,
                count = healed,
                "Healed and rewrote orphaned tool message(s) in WS session history"
            );
        }
    }
    messages
}

/// Optional connection parameters sent as the first WebSocket message.
///
/// If the first message after upgrade is `{"type":"connect",...}`, these
/// parameters are extracted and an acknowledgement is sent back. Old clients
/// that send `{"type":"message",...}` as the first frame still work — the
/// message is processed normally (backward-compatible).
#[derive(Debug, Deserialize)]
struct ConnectParams {
    #[serde(rename = "type")]
    msg_type: String,
    /// Client-chosen session ID for memory persistence
    #[serde(default)]
    session_id: Option<String>,
    /// Device name for device registry tracking
    #[serde(default)]
    device_name: Option<String>,
    /// Client capabilities
    #[serde(default)]
    capabilities: Vec<String>,
    /// Project root / working directory for this session.
    #[serde(default, alias = "workspaceDir", alias = "workspace_dir")]
    cwd: Option<String>,
}

/// Supported sub-protocols for the chat WebSocket (primary + legacy).
const WS_PROTOCOLS: &[&str] = &["kylinbot.v1", "kylinbot.v1"];

/// Prefix used in `Sec-WebSocket-Protocol` to carry a bearer token.
const BEARER_SUBPROTO_PREFIX: &str = "bearer.";

#[derive(Deserialize)]
pub struct WsQuery {
    pub token: Option<String>,
    pub session_id: Option<String>,
    /// Optional human-readable name for the session.
    pub name: Option<String>,
    /// Project root / working directory for this session.
    #[serde(default)]
    pub cwd: Option<String>,
    #[serde(default, alias = "workspaceDir", alias = "workspace_dir")]
    pub workspace_dir: Option<String>,
}

#[derive(Debug, PartialEq, Eq)]
enum WsInboundAction {
    Message {
        content: String,
        attachments: Vec<MediaAttachment>,
    },
    Cancel,
    SetIdentity {
        name: Option<String>,
        role: Option<String>,
    },
    Ignore,
    Error(&'static str),
}

/// Extract a bearer token from WebSocket-compatible sources.
///
/// Precedence (first non-empty wins):
/// 1. `Authorization: Bearer <token>` header
/// 2. `Sec-WebSocket-Protocol: bearer.<token>` subprotocol
/// 3. `?token=<token>` query parameter
///
/// Browsers cannot set custom headers on `new WebSocket(url)`, so the query
/// parameter and subprotocol paths are required for browser-based clients.
fn extract_ws_token<'a>(headers: &'a HeaderMap, query_token: Option<&'a str>) -> Option<&'a str> {
    // 1. Authorization header
    if let Some(t) = headers
        .get(header::AUTHORIZATION)
        .and_then(|v| v.to_str().ok())
        .and_then(|auth| auth.strip_prefix("Bearer "))
        && !t.is_empty()
    {
        return Some(t);
    }

    // 2. Sec-WebSocket-Protocol: bearer.<token>
    if let Some(t) = headers
        .get("sec-websocket-protocol")
        .and_then(|v| v.to_str().ok())
        .and_then(|protos| {
            protos
                .split(',')
                .map(|p| p.trim())
                .find_map(|p| p.strip_prefix(BEARER_SUBPROTO_PREFIX))
        })
        && !t.is_empty()
    {
        return Some(t);
    }

    // 3. ?token= query parameter
    if let Some(t) = query_token
        && !t.is_empty()
    {
        return Some(t);
    }

    None
}

fn parse_ws_inbound_action(raw: &str) -> WsInboundAction {
    let parsed: serde_json::Value = match serde_json::from_str(raw) {
        Ok(v) => v,
        Err(_) => return WsInboundAction::Error("Invalid JSON"),
    };

    match parsed["type"].as_str().unwrap_or("") {
        "message" => {
            let content = parsed["content"].as_str().unwrap_or("").trim().to_string();
            let attachments = match parse_inbound_attachments(&parsed) {
                Ok(attachments) => attachments,
                Err(message) => return WsInboundAction::Error(message),
            };
            if content.is_empty() && attachments.is_empty() {
                WsInboundAction::Error("Message content cannot be empty")
            } else {
                WsInboundAction::Message {
                    content,
                    attachments,
                }
            }
        }
        "cancel" => WsInboundAction::Cancel,
        "set_identity" => WsInboundAction::SetIdentity {
            name: parsed["name"]
                .as_str()
                .map(str::trim)
                .filter(|value| !value.is_empty())
                .map(ToOwned::to_owned),
            role: parsed["role"]
                .as_str()
                .map(str::trim)
                .filter(|value| !value.is_empty())
                .map(ToOwned::to_owned),
        },
        _ => WsInboundAction::Ignore,
    }
}

fn build_identity_markdown(name: Option<&str>, role: Option<&str>) -> Option<String> {
    if name.is_none() && role.is_none() {
        return None;
    }

    let mut content = String::from("# IDENTITY.md — Who Am I?\n\n");
    if let Some(name) = name {
        content.push_str(&format!("I am {name}.\n\n"));
    }
    if let Some(role) = role {
        content.push_str("## Role\n");
        content.push_str(role);
        content.push('\n');
    }
    Some(content)
}

fn build_soul_identity_section(name: &str, role: Option<&str>) -> String {
    let role = role.unwrap_or("帮忙做很多事情");
    format!(
        "## Identity\n\n你的名字是**{name}**。你是一个智能助手。\n你可以**{role}**\n当被问到你叫什么名字时，回答“{name}”。"
    )
}

fn replace_identity_section(soul: &str, new_identity: &str) -> String {
    let mut result = String::new();
    let mut in_identity = false;
    let mut found_identity = false;

    for line in soul.lines() {
        if line.starts_with("## Identity") {
            in_identity = true;
            found_identity = true;
            result.push_str(new_identity);
            result.push('\n');
            continue;
        }
        if in_identity {
            if line.starts_with("## ") {
                in_identity = false;
            } else {
                continue;
            }
        }
        result.push_str(line);
        result.push('\n');
    }

    if !found_identity {
        if let Some(pos) = soul.find("## Core Truths") {
            let mut new_content = soul[..pos].to_string();
            new_content.push_str(new_identity);
            new_content.push_str("\n\n");
            new_content.push_str(&soul[pos..]);
            return new_content;
        }
        let mut new_content = soul.to_string();
        if !new_content.ends_with('\n') {
            new_content.push('\n');
        }
        new_content.push('\n');
        new_content.push_str(new_identity);
        return new_content;
    }

    result.trim_end().to_string()
}

fn parse_inbound_attachments(
    parsed: &serde_json::Value,
) -> Result<Vec<MediaAttachment>, &'static str> {
    let Some(raw_attachments) = parsed.get("attachments") else {
        return Ok(vec![]);
    };
    let Some(array) = raw_attachments.as_array() else {
        return Err("Invalid attachments payload");
    };

    let mut attachments = Vec::with_capacity(array.len());
    for item in array {
        let file_name = item
            .get("file_name")
            .and_then(serde_json::Value::as_str)
            .map(str::trim)
            .filter(|value| !value.is_empty())
            .ok_or("Attachment file_name is required")?;
        let data_base64 = item
            .get("data_base64")
            .and_then(serde_json::Value::as_str)
            .map(str::trim)
            .filter(|value| !value.is_empty())
            .ok_or("Attachment data_base64 is required")?;
        let data = STANDARD
            .decode(data_base64)
            .map_err(|_| "Attachment data_base64 is invalid")?;
        let mime_type = item
            .get("mime_type")
            .and_then(serde_json::Value::as_str)
            .map(str::trim)
            .filter(|value| !value.is_empty())
            .map(ToOwned::to_owned);

        attachments.push(MediaAttachment {
            file_name: file_name.to_string(),
            data,
            mime_type,
        });
    }

    Ok(attachments)
}

/// Enrich the user message with textual descriptions of any attached media.
///
/// Uses the configured [`MediaPipeline`] to transcribe audio, describe images,
/// and summarise video. When the pipeline is disabled or there are no
/// attachments the original `content` is returned unchanged.
///
/// Additionally, image attachments are converted to `[IMAGE:data:...]` markers
/// so the vision pipeline can process them.
async fn enrich_with_attachments(
    state: &AppState,
    content: &str,
    attachments: Vec<MediaAttachment>,
) -> String {
    eprintln!(
        "[WS-DEBUG] enrich_with_attachments: {} attachments",
        attachments.len()
    );
    if attachments.is_empty() {
        return content.to_string();
    }
    let config = state.config.lock().clone();
    eprintln!(
        "[WS-DEBUG] enrich: media_pipeline enabled={}",
        config.media_pipeline.enabled
    );
    let pipeline = kylinbot_channels::orchestrator::media_pipeline::MediaPipeline::new(
        &config.media_pipeline,
        &config.transcription,
        false,
    );
    let mut enriched = pipeline.process(content, &attachments).await;

    // Inject [IMAGE:data:...] markers for image attachments so the
    // multimodal pipeline normalizes (and potentially resizes) them
    // before sending to the vision-capable provider.
    for att in &attachments {
        if let Some(ref mime) = att.mime_type {
            if mime.starts_with("image/") {
                let b64 = STANDARD.encode(&att.data);
                enriched.push_str(&format!("\n[IMAGE:data:{mime};base64,{b64}]"));
            }
        }
    }

    enriched
}

/// GET /ws/chat — WebSocket upgrade for agent chat
pub async fn handle_ws_chat(
    State(state): State<AppState>,
    Query(params): Query<WsQuery>,
    headers: HeaderMap,
    ws: WebSocketUpgrade,
) -> impl IntoResponse {
    // Auth: check header, subprotocol, then query param (precedence order)
    if state.pairing.require_pairing() {
        let token = extract_ws_token(&headers, params.token.as_deref()).unwrap_or("");
        if !state.pairing.is_authenticated(token) {
            return (
                axum::http::StatusCode::UNAUTHORIZED,
                "Unauthorized — provide Authorization header, Sec-WebSocket-Protocol bearer, or ?token= query param",
            )
                .into_response();
        }
    }

    // Echo Sec-WebSocket-Protocol if the client requests one of our supported sub-protocols.
    let client_protos: Vec<&str> = headers
        .get("sec-websocket-protocol")
        .and_then(|v| v.to_str().ok())
        .map(|s| s.split(',').map(|p| p.trim()).collect())
        .unwrap_or_default();
    let matched = client_protos
        .iter()
        .find_map(|cp| WS_PROTOCOLS.iter().find(|sp| **sp == *cp).copied());
    let ws = if let Some(proto) = matched {
        ws.protocols([proto])
    } else {
        ws
    };

    let session_id = params.session_id;
    let session_name = params.name;
    let session_cwd = params.cwd.or(params.workspace_dir);
    ws.max_message_size(MAX_WS_MESSAGE_SIZE)
        .max_frame_size(MAX_WS_MESSAGE_SIZE)
        .on_upgrade(move |socket| {
            handle_socket(socket, state, session_id, session_name, session_cwd)
        })
        .into_response()
}

/// Gateway session key prefix to avoid collisions with channel sessions.
const GW_SESSION_PREFIX: &str = "gw_";

#[derive(Debug, Clone, PartialEq, Eq)]
struct WsModelRoute {
    provider: String,
    model: String,
    /// Fallback profile credentials — compared so api-key edits take effect on
    /// the next message without forcing a manual WebSocket reconnect.
    api_key: Option<String>,
    base_url: Option<String>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
struct WsAgentConfigSnapshot {
    route: WsModelRoute,
    /// Broad fingerprint for long-lived WS sessions. If a saved config change
    /// can affect prompt/tool availability, rebuild the session Agent before
    /// the next turn instead of waiting for a manual reconnect.
    fingerprint: String,
}

fn optional_trimmed(value: Option<&str>) -> Option<String> {
    value
        .map(str::trim)
        .filter(|s| !s.is_empty())
        .map(ToString::to_string)
}

fn ws_model_route_from_config(config: &Config) -> WsModelRoute {
    let (provider, model) = resolve_fallback_model_route(config);
    let fallback = config.providers.fallback_provider();
    WsModelRoute {
        provider,
        model,
        api_key: fallback.and_then(|e| optional_trimmed(e.api_key.as_deref())),
        base_url: fallback.and_then(|e| optional_trimmed(e.base_url.as_deref())),
    }
}

fn ws_agent_config_snapshot(config: &Config) -> WsAgentConfigSnapshot {
    let fingerprint = serde_json::to_string(config).unwrap_or_else(|err| {
        tracing::warn!("Failed to serialize WS agent config snapshot: {err}");
        String::new()
    });
    WsAgentConfigSnapshot {
        route: ws_model_route_from_config(config),
        fingerprint,
    }
}

/// Rebuild the per-connection Agent when saved config changes can affect the
/// live prompt/tool/runtime state, so long-lived WebSocket sessions pick up
/// skill toggles and model switches without reconnecting.
async fn ensure_agent_matches_config(
    agent: &mut kylinbot_runtime::agent::Agent,
    state: &AppState,
    session_cwd: &Path,
    session_key: &str,
    active_snapshot: &mut WsAgentConfigSnapshot,
    approval_channel: Arc<WsApprovalChannel>,
) -> Result<(), anyhow::Error> {
    let config = state.config.lock().clone();
    let new_snapshot = ws_agent_config_snapshot(&config);
    if *active_snapshot == new_snapshot {
        return Ok(());
    }

    let route_changed = active_snapshot.route != new_snapshot.route;
    tracing::info!(
        old_provider = %active_snapshot.route.provider,
        old_model = %active_snapshot.route.model,
        new_provider = %new_snapshot.route.provider,
        new_model = %new_snapshot.route.model,
        route_changed,
        credentials_changed = (active_snapshot.route.api_key != new_snapshot.route.api_key
            || active_snapshot.route.base_url != new_snapshot.route.base_url),
        session_key = %session_key,
        "WebSocket session reloading agent after config change"
    );

    let memory_session_id = agent.memory_session_id().map(str::to_string);
    let messages = if let Some(ref backend) = state.session_backend {
        load_persisted_session_messages(backend.as_ref(), session_key)
    } else {
        agent
            .history()
            .iter()
            .filter_map(|m| match m {
                ConversationMessage::Chat(c) => Some(c.clone()),
                _ => None,
            })
            .collect()
    };

    let mut new_agent =
        kylinbot_runtime::agent::Agent::from_config_with_session_cwd(&config, Some(session_cwd))
            .await?;
    if let Some(sid) = memory_session_id {
        new_agent.set_memory_session_id(Some(sid));
    }
    if !messages.is_empty() {
        new_agent.seed_history(&messages);
    }
    new_agent
        .channel_handles()
        .register_channel("ws", approval_channel);
    *agent = new_agent;
    *active_snapshot = new_snapshot;
    Ok(())
}

async fn handle_socket(
    socket: WebSocket,
    state: AppState,
    session_id: Option<String>,
    session_name: Option<String>,
    session_cwd: Option<String>,
) {
    eprintln!("[WS-DEBUG] handle_socket: connection accepted, entering handler");
    let (mut sender, mut receiver) = socket.split();

    // Resolve session ID: use provided or generate a new UUID
    let session_id = session_id.unwrap_or_else(|| uuid::Uuid::new_v4().to_string());
    let session_key = format!("{GW_SESSION_PREFIX}{session_id}");
    let mut memory_session_id = session_id.clone();
    eprintln!("[WS-DEBUG] session_key={session_key}, loading state...");

    // Hydrate session metadata from persistence (if available). Agent
    // construction is deferred until after the optional `connect` frame so the
    // client can provide a per-session cwd for the security sandbox root.
    let config = state.config.lock().clone();
    let mut resumed = false;
    let mut message_count: usize = 0;
    let mut effective_name: Option<String> = None;
    let mut stored_messages = Vec::new();
    if let Some(ref backend) = state.session_backend {
        let messages = load_persisted_session_messages(backend.as_ref(), &session_key);
        if !messages.is_empty() {
            message_count = messages.len();
            stored_messages = messages;
            resumed = true;
        }
        // Set session name if provided (non-empty) on connect
        if let Some(ref name) = session_name
            && !name.is_empty()
        {
            let _ = backend.set_session_name(&session_key, name);
            effective_name = Some(name.clone());
        }
        // If no name was provided via query param, load the stored name
        if effective_name.is_none() {
            effective_name = backend.get_session_name(&session_key).unwrap_or(None);
        }
    }

    // Send session_start message to client
    let mut session_start = serde_json::json!({
        "type": "session_start",
        "session_id": session_id,
        "resumed": resumed,
        "message_count": message_count,
    });
    if let Some(ref name) = effective_name {
        session_start["name"] = serde_json::Value::String(name.clone());
    }
    eprintln!("[WS-DEBUG] sending session_start (session_id={session_id}, resumed={resumed})");
    let _ = sender
        .send(Message::Text(session_start.to_string().into()))
        .await;
    eprintln!("[WS-DEBUG] session_start sent, waiting for first message...");

    // ── Optional connect handshake ──────────────────────────────────
    // The first message may be a `{"type":"connect",...}` frame carrying
    // connection parameters.  If it is, we extract the params, send an
    // ack, and proceed to the normal message loop.  If the first message
    // is a regular `{"type":"message",...}` frame, we fall through and
    // process it immediately (backward-compatible).
    let mut first_msg_fallback: Option<String> = None;
    let mut requested_cwd = session_cwd;

    if let Some(first) = receiver.next().await {
        match first {
            Ok(Message::Text(text)) => {
                if text.len() > MAX_WS_PAYLOAD_BYTES {
                    let err = serde_json::json!({
                        "type": "error",
                        "message": "Message payload too large",
                        "code": "PAYLOAD_TOO_LARGE"
                    });
                    let _ = sender.send(Message::Text(err.to_string().into())).await;
                    return;
                }
                if let Ok(cp) = serde_json::from_str::<ConnectParams>(&text) {
                    if cp.msg_type == "connect" {
                        debug!(
                            session_id = ?cp.session_id,
                            device_name = ?cp.device_name,
                            capabilities = ?cp.capabilities,
                            cwd = ?cp.cwd,
                            "WebSocket connect params received"
                        );
                        if let Some(sid) = &cp.session_id {
                            memory_session_id = sid.clone();
                            debug!(
                                session_id = sid,
                                "WebSocket connect session override received"
                            );
                        }
                        if cp.cwd.is_some() {
                            requested_cwd = cp.cwd;
                        }
                        let ack = serde_json::json!({
                            "type": "connected",
                            "message": "Connection established"
                        });
                        let _ = sender.send(Message::Text(ack.to_string().into())).await;
                    } else {
                        // Not a connect message — fall through to normal processing
                        first_msg_fallback = Some(text.to_string());
                    }
                } else {
                    // Not parseable as ConnectParams — fall through
                    first_msg_fallback = Some(text.to_string());
                }
            }
            Ok(Message::Close(frame)) => {
                let code = frame.as_ref().map(|f| f.code);
                let reason = frame
                    .as_ref()
                    .map(|f| String::from_utf8_lossy(f.reason.as_ref()).to_string());
                eprintln!(
                    "[WS-DEBUG] first message was Close: code={:?} reason={:?}",
                    code, reason
                );
                return;
            }
            Err(e) => {
                eprintln!("[WS-DEBUG] first message read ERROR: {e}");
                return;
            }
            Ok(Message::Ping(_)) | Ok(Message::Pong(_)) => {
                eprintln!("[WS-DEBUG] first message was Ping/Pong (auto-handled)");
            }
            Ok(Message::Binary(b)) => {
                eprintln!("[WS-DEBUG] first message was Binary (len={})", b.len());
            }
        }
    } else {
        eprintln!(
            "[WS-DEBUG] first message receiver returned None (stream ended before any message)"
        );
        return;
    }

    let session_cwd = match resolve_session_cwd(requested_cwd.as_deref(), &config.workspace_dir) {
        Ok(cwd) => cwd,
        Err(e) => {
            let err = serde_json::json!({
                "type": "error",
                "message": e.to_string(),
                "code": "INVALID_CWD"
            });
            let _ = sender.send(Message::Text(err.to_string().into())).await;
            return;
        }
    };

    eprintln!(
        "[WS-DEBUG] cwd resolved, building Agent from config (workspace_dir={})...",
        config.workspace_dir.display()
    );
    // Build a persistent Agent for this connection so history is maintained
    // across turns. The session cwd becomes the security sandbox root; config
    // workspace remains the daemon data directory.
    let mut agent = match kylinbot_runtime::agent::Agent::from_config_with_session_cwd(
        &config,
        Some(&session_cwd),
    )
    .await
    {
        Ok(a) => {
            eprintln!("[WS-DEBUG] Agent built successfully");
            a
        }
        Err(e) => {
            eprintln!("[WS-DEBUG] Agent::from_config_with_session_cwd FAILED: {e}");
            tracing::error!(error = %e, "Agent initialization failed");
            let err = serde_json::json!({
                "type": "error",
                "message": format!("Failed to initialise agent: {e}"),
                "code": "AGENT_INIT_FAILED"
            });
            let _ = sender.send(Message::Text(err.to_string().into())).await;
            let _ = sender
                .send(Message::Close(Some(axum::extract::ws::CloseFrame {
                    code: 1011,
                    reason: axum::extract::ws::Utf8Bytes::from_static(
                        "Agent initialization failed",
                    ),
                })))
                .await;
            eprintln!("[WS-DEBUG] agent init failed, sent close frame, exiting handle_socket");
            return;
        }
    };
    eprintln!("[WS-DEBUG] registering approval channel...");
    agent.set_memory_session_id(Some(memory_session_id));
    if !stored_messages.is_empty() {
        agent.seed_history(&stored_messages);
    }

    let (approval_event_tx, mut approval_event_rx) =
        tokio::sync::mpsc::channel::<kylinbot_api::agent::TurnEvent>(8);
    let pending_approvals: PendingApprovals = new_pending_approvals();
    let approval_channel = Arc::new(WsApprovalChannel::new(
        approval_event_tx,
        pending_approvals.clone(),
        Duration::from_secs(WS_APPROVAL_TIMEOUT_SECS),
    ));
    agent
        .channel_handles()
        .register_channel("ws", approval_channel.clone());

    let mut active_snapshot = ws_agent_config_snapshot(&config);

    // Process the first message if it was not a connect frame
    if let Some(ref text) = first_msg_fallback {
        eprintln!(
            "[WS-DEBUG] processing first message (len={})...",
            text.len()
        );
        match parse_ws_inbound_action(text) {
            WsInboundAction::Message {
                content,
                attachments,
            } => {
                let content = enrich_with_attachments(&state, &content, attachments).await;

                if let Err(e) = ensure_agent_matches_config(
                    &mut agent,
                    &state,
                    &session_cwd,
                    &session_key,
                    &mut active_snapshot,
                    Arc::clone(&approval_channel),
                )
                .await
                {
                    tracing::error!(error = %e, "Agent refresh after config change failed");
                    let err = serde_json::json!({
                        "type": "error",
                        "message": format!("Failed to apply model change: {e}"),
                        "code": "AGENT_RELOAD_FAILED"
                    });
                    let _ = sender.send(Message::Text(err.to_string().into())).await;
                } else {
                    if let Some(ref backend) = state.session_backend {
                        let user_msg = kylinbot_providers::ChatMessage::user(&content);
                        let _ = backend.append(&session_key, &user_msg);
                    }

                    process_chat_message(
                        &state,
                        &mut agent,
                        &mut sender,
                        &mut receiver,
                        &mut approval_event_rx,
                        &pending_approvals,
                        &content,
                        &session_key,
                        &active_snapshot.route.provider,
                        &active_snapshot.route.model,
                    )
                    .await;
                }
            }
            WsInboundAction::Cancel => {
                let cancelled = serde_json::json!({
                    "type": "message_cancelled",
                    "session_id": session_id,
                });
                let _ = sender
                    .send(Message::Text(cancelled.to_string().into()))
                    .await;
            }
            WsInboundAction::SetIdentity { name, role } => {
                match persist_identity_update(&state, name, role).await {
                    Ok(()) => {
                        let ack =
                            serde_json::json!({ "type": "identity_set", "session_id": session_id });
                        let _ = sender.send(Message::Text(ack.to_string().into())).await;
                    }
                    Err(e) => {
                        let err = serde_json::json!({
                            "type": "error",
                            "message": format!("Failed to save identity: {e}"),
                            "code": "IDENTITY_SAVE_FAILED"
                        });
                        let _ = sender.send(Message::Text(err.to_string().into())).await;
                    }
                }
            }
            WsInboundAction::Ignore => {
                let err = serde_json::json!({
                    "type": "error",
                    "message": "Unsupported message type. Send {\"type\":\"message\",\"content\":\"your text\"}"
                });
                let _ = sender.send(Message::Text(err.to_string().into())).await;
            }
            WsInboundAction::Error(message) => {
                let err = serde_json::json!({ "type": "error", "message": message });
                let _ = sender.send(Message::Text(err.to_string().into())).await;
            }
        }
    }

    // Subscribe to the shared broadcast channel so cron/heartbeat events
    // are forwarded to this WebSocket client.
    let mut broadcast_rx = state.event_tx.subscribe();

    eprintln!("[WS-DEBUG] entering main message loop");

    loop {
        tokio::select! {
            // ── Client message ────────────────────────────────────────
            client_msg = receiver.next() => {
                let Some(msg) = client_msg else { break };
                let msg = match msg {
                    Ok(Message::Text(text)) => {
                        if text.len() > MAX_WS_PAYLOAD_BYTES {
                            let err = serde_json::json!({
                                "type": "error",
                                "message": "Message payload too large",
                                "code": "PAYLOAD_TOO_LARGE"
                            });
                            let _ = sender.send(Message::Text(err.to_string().into())).await;
                            continue;
                        }
                        text
                    }
                    Ok(Message::Close(_)) | Err(_) => break,
                    _ => continue,
                };

                // Voice duplex text frames (gated by feature + runtime config).
                #[cfg(feature = "gateway-voice-duplex")]
                {
                    let duplex_enabled = state
                        .config
                        .lock()
                        .channels
                        .voice_duplex
                        .as_ref()
                        .is_some_and(|v| v.enabled);
                    if duplex_enabled {
                        if let Some(voice_event) = crate::voice_duplex::try_parse_voice_event(&msg)
                        {
                            if let Some(error_frame) =
                                crate::voice_duplex::handle_voice_event(voice_event)
                            {
                                let _ = sender
                                    .send(Message::Text(error_frame.to_string().into()))
                                    .await;
                            }
                            continue;
                        }
                    }
                }

                if let Ok(parsed) = serde_json::from_str::<serde_json::Value>(&msg)
                    && parsed["type"].as_str() == Some("approval_response")
                {
                    let request_id = parsed["request_id"].as_str().unwrap_or("");
                    let decision = match parsed["decision"].as_str().unwrap_or("") {
                        "approve" => Some(ChannelApprovalResponse::Approve),
                        "always" => Some(ChannelApprovalResponse::AlwaysApprove),
                        "deny" => Some(ChannelApprovalResponse::Deny),
                        _ => None,
                    };
                    if request_id.is_empty() || decision.is_none() {
                        let err = serde_json::json!({
                            "type": "error",
                            "message": "approval_response requires request_id and decision in {approve,deny,always}",
                            "code": "INVALID_APPROVAL_RESPONSE"
                        });
                        let _ = sender.send(Message::Text(err.to_string().into())).await;
                        continue;
                    }
                    if let Some(tx) = pending_approvals.lock().remove(request_id) {
                        let _ = tx.send(decision.expect("checked above"));
                    } else {
                        debug!(%request_id, "approval_response with no matching pending request");
                    }
                    continue;
                }

                match parse_ws_inbound_action(&msg) {
                    WsInboundAction::Message { content, attachments } => {
                        let content = enrich_with_attachments(&state, &content, attachments).await;

                        let _session_guard = match state.session_queue.acquire(&session_key).await {
                            Ok(guard) => guard,
                            Err(e) => {
                                let err = serde_json::json!({
                                    "type": "error",
                                    "message": e.to_string(),
                                    "code": "SESSION_BUSY"
                                });
                                let _ = sender.send(Message::Text(err.to_string().into())).await;
                                continue;
                            }
                        };

                        if let Err(e) = ensure_agent_matches_config(
                            &mut agent,
                            &state,
                            &session_cwd,
                            &session_key,
                            &mut active_snapshot,
                            Arc::clone(&approval_channel),
                        )
                        .await
                        {
                            tracing::error!(error = %e, "Agent refresh after config change failed");
                            let err = serde_json::json!({
                                "type": "error",
                                "message": format!("Failed to apply model change: {e}"),
                                "code": "AGENT_RELOAD_FAILED"
                            });
                            let _ = sender.send(Message::Text(err.to_string().into())).await;
                            continue;
                        }

                        if let Some(ref backend) = state.session_backend {
                            let user_msg = kylinbot_providers::ChatMessage::user(&content);
                            let _ = backend.append(&session_key, &user_msg);
                        }

                        process_chat_message(
                            &state,
                            &mut agent,
                            &mut sender,
                            &mut receiver,
                            &mut approval_event_rx,
                            &pending_approvals,
                            &content,
                            &session_key,
                            &active_snapshot.route.provider,
                            &active_snapshot.route.model,
                        )
                        .await;
                    }
                    WsInboundAction::Cancel => {
                        let cancelled = serde_json::json!({
                            "type": "message_cancelled",
                            "session_id": session_id
                        });
                        let _ = sender.send(Message::Text(cancelled.to_string().into())).await;
                    }
                    WsInboundAction::SetIdentity { name, role } => {
                        match persist_identity_update(&state, name, role).await {
                            Ok(()) => {
                                let ack = serde_json::json!({
                                    "type": "identity_set",
                                    "session_id": session_id
                                });
                                let _ = sender.send(Message::Text(ack.to_string().into())).await;
                            }
                            Err(e) => {
                                let err = serde_json::json!({
                                    "type": "error",
                                    "message": format!("Failed to save identity: {e}"),
                                    "code": "IDENTITY_SAVE_FAILED"
                                });
                                let _ = sender.send(Message::Text(err.to_string().into())).await;
                            }
                        }
                    }
                    WsInboundAction::Ignore => {
                        let err = serde_json::json!({
                            "type": "error",
                            "message": "Unsupported message type. Send {\"type\":\"message\",\"content\":\"your text\"}",
                            "code": "UNKNOWN_MESSAGE_TYPE"
                        });
                        let _ = sender.send(Message::Text(err.to_string().into())).await;
                    }
                    WsInboundAction::Error(message) => {
                        let err = serde_json::json!({
                            "type": "error",
                            "message": message,
                            "code": if message == "Invalid JSON" { "INVALID_JSON" } else { "EMPTY_CONTENT" }
                        });
                        let _ = sender.send(Message::Text(err.to_string().into())).await;
                    }
                }
            }

            // ── Broadcast event (cron/heartbeat results) ──────────────
            event = broadcast_rx.recv() => {
                if let Ok(event) = event {
                    let _ = sender.send(Message::Text(event.to_string().into())).await;
                }
            }

            // ── Approval request from the agent's tool loop ────────────
            // The WsApprovalChannel emits these whenever a supervised tool
            // call needs operator consent. Forwarded out the same socket
            // as the regular streaming events; the matching response
            // arrives via the `approval_response` arm above and resolves
            // the channel's pending oneshot.
            approval_event = approval_event_rx.recv() => {
                let Some(event) = approval_event else { break };
                let frame = match event {
                    kylinbot_api::agent::TurnEvent::ApprovalRequest {
                        request_id,
                        tool_name,
                        arguments_summary,
                        timeout_secs,
                    } => serde_json::json!({
                        "type": "approval_request",
                        "request_id": request_id,
                        "tool": tool_name,
                        "arguments_summary": arguments_summary,
                        "timeout_secs": timeout_secs,
                    }),
                    other => {
                        tracing::warn!(
                            kind = ?other,
                            "non-ApprovalRequest event leaked into approval channel"
                        );
                        continue;
                    }
                };
                let _ = sender.send(Message::Text(frame.to_string().into())).await;
            }
        }
    }
    eprintln!("[WS-DEBUG] handle_socket: exiting (loop ended)");
}

async fn persist_identity_update(
    state: &AppState,
    name: Option<String>,
    role: Option<String>,
) -> anyhow::Result<()> {
    let mut config = state.config.lock().clone();
    config.identity.name = name.clone();
    config.identity.role = role.clone();
    let workspace_dir = config.workspace_dir.clone();
    config.save().await?;
    *state.config.lock() = config;

    if let Some(identity_markdown) = build_identity_markdown(name.as_deref(), role.as_deref()) {
        tokio::fs::write(workspace_dir.join("IDENTITY.md"), identity_markdown).await?;
    }
    if let Some(name) = name.as_deref() {
        let soul_path = workspace_dir.join("SOUL.md");
        if let Ok(soul_content) = tokio::fs::read_to_string(&soul_path).await {
            let next_identity = build_soul_identity_section(name, role.as_deref());
            let next_soul = replace_identity_section(&soul_content, &next_identity);
            tokio::fs::write(soul_path, next_soul).await?;
        }
    }

    Ok(())
}

fn resolve_session_cwd(
    requested_cwd: Option<&str>,
    default_workspace: &Path,
) -> anyhow::Result<PathBuf> {
    let cwd = requested_cwd
        .map(PathBuf::from)
        .unwrap_or_else(|| default_workspace.to_path_buf());
    std::fs::canonicalize(&cwd)
        .map_err(|e| anyhow::anyhow!("cwd is not a usable directory ({}): {e}", cwd.display()))
}

/// Process a single chat message through the agent and send the response.
///
/// Uses [`Agent::turn_streamed`] so that intermediate text chunks, tool calls,
/// and tool results are forwarded to the WebSocket client in real time.
async fn process_chat_message(
    state: &AppState,
    agent: &mut kylinbot_runtime::agent::Agent,
    sender: &mut futures_util::stream::SplitSink<WebSocket, Message>,
    receiver: &mut futures_util::stream::SplitStream<WebSocket>,
    approval_event_rx: &mut tokio::sync::mpsc::Receiver<kylinbot_api::agent::TurnEvent>,
    pending_approvals: &PendingApprovals,
    content: &str,
    session_key: &str,
    provider_label: &str,
    model_label: &str,
) {
    eprintln!(
        "[WS-DEBUG] process_chat_message: starting turn (content_len={}, provider={provider_label}, model={model_label})",
        content.len()
    );
    use kylinbot_runtime::agent::TurnEvent;

    // Broadcast agent_start event
    let _ = state.event_tx.send(serde_json::json!({
        "type": "agent_start",
        "provider": provider_label,
        "model": model_label,
    }));

    // Set session state to running
    let turn_id = uuid::Uuid::new_v4().to_string();
    if let Some(ref backend) = state.session_backend {
        let _ = backend.set_session_state(session_key, "running", Some(&turn_id));
    }

    // ── Cancellation token lifecycle ─────────────────────────────
    // Create a token before the turn starts so the abort endpoint
    // can cancel it. Remove it after the turn completes regardless
    // of outcome (normal, error, or cancelled).
    let cancel_token = CancellationToken::new();
    {
        state
            .cancel_tokens
            .lock()
            .expect("cancel_tokens lock poisoned")
            .insert(session_key.to_string(), cancel_token.clone());
    }

    // Channel for streaming turn events from the agent.
    let (event_tx, mut event_rx) = tokio::sync::mpsc::channel::<TurnEvent>(64);

    let content_owned = content.to_string();
    let session_key_owned = session_key.to_string();
    let turn_fut = async {
        kylinbot_runtime::agent::loop_::scope_session_key(
            Some(session_key_owned),
            agent.turn_streamed(&content_owned, event_tx, Some(cancel_token.clone())),
        )
        .await
    };
    tokio::pin!(turn_fut);

    let mut accumulated_text = String::new();
    let mut partial_saved = false;
    let mut last_partial_save_for_persist = std::time::Instant::now();
    let partial_save_interval = std::time::Duration::from_millis(500);
    let mut total_input_tokens: Option<u64> = None;
    let mut total_output_tokens: Option<u64> = None;

    let result = loop {
        tokio::select! {
            result = &mut turn_fut => break result,
            maybe_event = event_rx.recv() => {
                let Some(event) = maybe_event else {
                    continue;
                };

                let ws_msg = match event {
                    TurnEvent::Usage {
                        input_tokens,
                        output_tokens,
                        cost_usd: _,
                    } => {
                        if let Some(it) = input_tokens {
                            total_input_tokens = Some(total_input_tokens.unwrap_or(0) + it);
                        }
                        if let Some(ot) = output_tokens {
                            total_output_tokens = Some(total_output_tokens.unwrap_or(0) + ot);
                        }
                        continue;
                    }
                    TurnEvent::Chunk { delta } => {
                        accumulated_text.push_str(&delta);

                        if last_partial_save_for_persist.elapsed() >= partial_save_interval {
                            if let Some(ref backend) = state.session_backend {
                                let partial =
                                    kylinbot_providers::ChatMessage::assistant(&accumulated_text);
                                if partial_saved {
                                    let _ = backend.update_last(session_key, &partial);
                                } else {
                                    let _ = backend.append(session_key, &partial);
                                    partial_saved = true;
                                }
                            }
                            last_partial_save_for_persist = std::time::Instant::now();
                        }

                        serde_json::json!({ "type": "chunk", "content": delta })
                    }
                    TurnEvent::Thinking { delta } => {
                        serde_json::json!({ "type": "thinking", "content": delta })
                    }
                    TurnEvent::ToolCall { id, name, args } => {
                        serde_json::json!({ "type": "tool_call", "id": id, "name": name, "args": args })
                    }
                    TurnEvent::ToolResult { id, name, output } => {
                        serde_json::json!({ "type": "tool_result", "id": id, "name": name, "output": output })
                    }
                    TurnEvent::ApprovalRequest {
                        request_id,
                        tool_name,
                        arguments_summary,
                        timeout_secs,
                    } => serde_json::json!({
                        "type": "approval_request",
                        "request_id": request_id,
                        "tool": tool_name,
                        "arguments_summary": arguments_summary,
                        "timeout_secs": timeout_secs,
                    }),
                };
                let _ = sender.send(Message::Text(ws_msg.to_string().into())).await;
            }
            approval_event = approval_event_rx.recv() => {
                let Some(event) = approval_event else {
                    continue;
                };
                if let TurnEvent::ApprovalRequest {
                    request_id,
                    tool_name,
                    arguments_summary,
                    timeout_secs,
                } = event {
                    let frame = serde_json::json!({
                        "type": "approval_request",
                        "request_id": request_id,
                        "tool": tool_name,
                        "arguments_summary": arguments_summary,
                        "timeout_secs": timeout_secs,
                    });
                    let _ = sender.send(Message::Text(frame.to_string().into())).await;
                }
            }
            inbound = receiver.next() => {
                let Some(inbound) = inbound else {
                    cancel_token.cancel();
                    break Err(kylinbot_runtime::agent::loop_::ToolLoopCancelled.into());
                };

                match inbound {
                    Ok(Message::Text(text)) => {
                        if let Ok(parsed) = serde_json::from_str::<serde_json::Value>(&text)
                            && parsed["type"].as_str() == Some("approval_response")
                        {
                            let request_id = parsed["request_id"].as_str().unwrap_or("");
                            let decision = match parsed["decision"].as_str().unwrap_or("") {
                                "approve" => Some(ChannelApprovalResponse::Approve),
                                "always" => Some(ChannelApprovalResponse::AlwaysApprove),
                                "deny" => Some(ChannelApprovalResponse::Deny),
                                _ => None,
                            };
                            if let (false, Some(decision)) = (request_id.is_empty(), decision) {
                                if let Some(tx) = pending_approvals.lock().remove(request_id) {
                                    let _ = tx.send(decision);
                                } else {
                                    debug!(%request_id, "approval_response with no matching pending request (mid-turn)");
                                }
                            }
                            continue;
                        }
                        match parse_ws_inbound_action(&text) {
                        WsInboundAction::Cancel => {
                            cancel_token.cancel();
                        }
                        WsInboundAction::Ignore => {}
                        WsInboundAction::Error(message) => {
                            let err = serde_json::json!({
                                "type": "error",
                                "message": message,
                                "code": if message == "Invalid JSON" { "INVALID_JSON" } else { "EMPTY_CONTENT" }
                            });
                            let _ = sender.send(Message::Text(err.to_string().into())).await;
                        }
                        WsInboundAction::Message { .. }
                        | WsInboundAction::SetIdentity { .. } => {
                            let err = serde_json::json!({
                                "type": "error",
                                "message": "A turn is already in progress",
                                "code": "SESSION_BUSY"
                            });
                            let _ = sender.send(Message::Text(err.to_string().into())).await;
                        }
                        }
                    }
                    Ok(Message::Close(_)) | Err(_) => {
                        cancel_token.cancel();
                        break Err(kylinbot_runtime::agent::loop_::ToolLoopCancelled.into());
                    }
                    _ => {}
                }
            }
        }
    };

    // After `turn_fut` completes, `tokio::select!` may have resolved the finished
    // future in the same tick as a pending `TurnEvent::Usage`, dropping that
    // usage update. Upstream uses `join!(turn_fut, forward_fut)` so the channel is
    // drained until closed; mirror that by draining remaining events here.
    while let Some(event) = event_rx.recv().await {
        if let TurnEvent::Usage {
            input_tokens,
            output_tokens,
            cost_usd: _,
        } = event
        {
            if let Some(it) = input_tokens {
                total_input_tokens = Some(total_input_tokens.unwrap_or(0) + it);
            }
            if let Some(ot) = output_tokens {
                total_output_tokens = Some(total_output_tokens.unwrap_or(0) + ot);
            }
        }
    }

    // ── Remove cancel token (turn finished) ──────────────────────
    {
        state
            .cancel_tokens
            .lock()
            .expect("cancel_tokens lock poisoned")
            .remove(session_key);
    }

    let was_cancelled = match &result {
        Err(e) => kylinbot_runtime::agent::loop_::is_tool_loop_cancelled(e),
        Ok(_) => false,
    };

    if was_cancelled {
        let truncated = if accumulated_text.is_empty() {
            "[interrupted by user]".to_string()
        } else {
            format!("{accumulated_text}\n\n[interrupted by user]")
        };

        if let Some(ref backend) = state.session_backend {
            let assistant_msg = kylinbot_providers::ChatMessage::assistant(&truncated);
            if partial_saved {
                let _ = backend.update_last(session_key, &assistant_msg);
            } else {
                let _ = backend.append(session_key, &assistant_msg);
            }
        }

        let aborted = serde_json::json!({ "type": "aborted" });
        let _ = sender.send(Message::Text(aborted.to_string().into())).await;

        if let Some(ref backend) = state.session_backend {
            let _ = backend.set_session_state(session_key, "idle", None);
        }

        let _ = state.event_tx.send(serde_json::json!({
            "type": "agent_end",
            "provider": provider_label,
            "model": model_label,
        }));

        kylinbot_runtime::observability::runtime_trace::record_event(
            "gateway_ws_turn",
            Some("ws"),
            Some(&provider_label),
            Some(model_label),
            Some(&turn_id),
            Some(false),
            Some("interrupted by user"),
            serde_json::json!({ "session_key": session_key, "cancelled": true }),
        );

        return;
    }

    match result {
        Ok(response) => {
            // Persist final assistant response. If we saved partial content
            // during streaming, update it in-place; otherwise append fresh.
            if let Some(ref backend) = state.session_backend {
                let assistant_msg = kylinbot_providers::ChatMessage::assistant(&response);
                if partial_saved {
                    let _ = backend.update_last(session_key, &assistant_msg);
                } else {
                    let _ = backend.append(session_key, &assistant_msg);
                }
            }

            // Fire-and-forget memory consolidation so facts from WS sessions
            // are extracted to long-term memory (Daily + Core categories).
            if state.auto_save {
                let mem = state.mem.clone();
                let provider = state.provider.clone();
                let model = model_label.to_string();
                let user_msg = content.to_string();
                let assistant_resp = response.clone();
                tokio::spawn(async move {
                    if let Err(e) = kylinbot_memory::consolidation::consolidate_turn(
                        provider.as_ref(),
                        &model,
                        mem.as_ref(),
                        &user_msg,
                        &assistant_resp,
                    )
                    .await
                    {
                        tracing::debug!("WS memory consolidation skipped: {e}");
                    }
                });
            }

            // Send chunk_reset so the client clears any accumulated draft
            // before the authoritative done message.
            let reset = serde_json::json!({ "type": "chunk_reset" });
            let _ = sender.send(Message::Text(reset.to_string().into())).await;

            // Compute cost from accumulated tokens + configured pricing,
            // then write the cost record so /api/cost and costs.jsonl reflect
            // this turn. Done before the done frame so cost_usd can ride along.
            let total_tokens = match (total_input_tokens, total_output_tokens) {
                (Some(i), Some(o)) => Some(i.saturating_add(o)),
                (Some(i), None) => Some(i),
                (None, Some(o)) => Some(o),
                (None, None) => None,
            };
            let cost_usd = record_turn_cost(
                state,
                &provider_label,
                &model_label,
                total_input_tokens,
                total_output_tokens,
            );

            let done = serde_json::json!({
                "type": "done",
                "full_response": response,
                "input_tokens": total_input_tokens,
                "output_tokens": total_output_tokens,
                "tokens_used": total_tokens,
                "cost_usd": cost_usd,
                "model": model_label,
                "provider": provider_label,
            });
            let _ = sender.send(Message::Text(done.to_string().into())).await;

            // Set session state to idle
            if let Some(ref backend) = state.session_backend {
                let _ = backend.set_session_state(session_key, "idle", None);
            }

            // Broadcast agent_end event
            let _ = state.event_tx.send(serde_json::json!({
                "type": "agent_end",
                "provider": provider_label,
                "model": model_label,
            }));

            kylinbot_runtime::observability::runtime_trace::record_event(
                "gateway_ws_turn",
                Some("ws"),
                Some(&provider_label),
                Some(model_label),
                Some(&turn_id),
                Some(true),
                None,
                serde_json::json!({
                    "session_key": session_key,
                    "input_tokens": total_input_tokens,
                    "output_tokens": total_output_tokens,
                    "tokens_used": total_tokens,
                    "cost_usd": cost_usd,
                }),
            );
        }
        Err(e) => {
            if kylinbot_runtime::agent::loop_::is_tool_loop_cancelled(&e) {
                if let Some(ref backend) = state.session_backend {
                    let _ = backend.set_session_state(session_key, "idle", None);
                }
                return;
            }

            // Set session state to error
            if let Some(ref backend) = state.session_backend {
                let _ = backend.set_session_state(session_key, "error", Some(&turn_id));
            }

            eprintln!("[WS-DEBUG] process_chat_message: Agent turn FAILED: {e}");
            tracing::error!(error = %e, "Agent turn failed");
            let sanitized = kylinbot_providers::sanitize_api_error(&e.to_string());
            let error_code = if sanitized.to_lowercase().contains("api key")
                || sanitized.to_lowercase().contains("authentication")
                || sanitized.to_lowercase().contains("unauthorized")
            {
                "AUTH_ERROR"
            } else if sanitized.to_lowercase().contains("provider")
                || sanitized.to_lowercase().contains("model")
            {
                "PROVIDER_ERROR"
            } else {
                "AGENT_ERROR"
            };
            let err = serde_json::json!({
                "type": "error",
                "message": sanitized,
                "code": error_code,
            });
            let _ = sender.send(Message::Text(err.to_string().into())).await;

            // Broadcast error event
            let _ = state.event_tx.send(serde_json::json!({
                "type": "error",
                "component": "ws_chat",
                "message": sanitized,
            }));

            // Trace the failed turn so the doctor / replay tool sees the
            // failure mode and the turn_id can be cross-referenced with
            // costs.jsonl. #6001 follow-through.
            kylinbot_runtime::observability::runtime_trace::record_event(
                "gateway_ws_turn",
                Some("ws"),
                Some(&provider_label),
                Some(model_label),
                Some(&turn_id),
                Some(false),
                Some(&sanitized),
                serde_json::json!({ "session_key": session_key, "error_code": error_code }),
            );
        }
    }
}

/// Record token usage for the just-completed turn against the gateway's
/// cost tracker, returning the computed cost in USD (or `None` when no
/// tracker is configured or no usage was reported).
fn record_turn_cost(
    state: &AppState,
    provider_name: &str,
    model: &str,
    input_tokens: Option<u64>,
    output_tokens: Option<u64>,
) -> Option<f64> {
    let tracker = state.cost_tracker.as_ref()?;
    if input_tokens.is_none() && output_tokens.is_none() {
        return None;
    }
    let input = input_tokens.unwrap_or(0);
    let output = output_tokens.unwrap_or(0);
    if input == 0 && output == 0 {
        return None;
    }
    let prices = state.config.lock().combined_pricing();
    // 3-tier model pricing lookup mirrors record_tool_loop_cost_usage so
    // streaming and non-streaming paths derive identical costs.
    let pricing = prices
        .get(&format!("{provider_name}/{model}"))
        .or_else(|| prices.get(model))
        .or_else(|| {
            model
                .rsplit_once('/')
                .and_then(|(_, suffix)| prices.get(suffix))
        });
    let usage = kylinbot_runtime::cost::types::TokenUsage::new(
        model,
        input,
        output,
        pricing.map_or(0.0, |entry| entry.input),
        pricing.map_or(0.0, |entry| entry.output),
    );
    let cost_usd = usage.cost_usd;
    let configured_models = state.config.lock().configured_model_names();
    if let Err(error) = tracker.record_usage(usage, Some(&configured_models)) {
        tracing::warn!(
            provider = provider_name,
            model,
            "Failed to record gateway turn cost: {error}"
        );
    }
    Some(cost_usd)
}

#[cfg(test)]
mod tests {
    use super::*;
    use axum::http::HeaderMap;

    #[test]
    fn extract_ws_token_from_authorization_header() {
        let mut headers = HeaderMap::new();
        headers.insert("authorization", "Bearer zc_test123".parse().unwrap());
        assert_eq!(extract_ws_token(&headers, None), Some("zc_test123"));
    }

    #[test]
    fn extract_ws_token_from_subprotocol() {
        let mut headers = HeaderMap::new();
        headers.insert(
            "sec-websocket-protocol",
            "kylinbot.v1, bearer.zc_sub456".parse().unwrap(),
        );
        assert_eq!(extract_ws_token(&headers, None), Some("zc_sub456"));
    }

    #[test]
    fn extract_ws_token_from_query_param() {
        let headers = HeaderMap::new();
        assert_eq!(
            extract_ws_token(&headers, Some("zc_query789")),
            Some("zc_query789")
        );
    }

    #[test]
    fn extract_ws_token_precedence_header_over_subprotocol() {
        let mut headers = HeaderMap::new();
        headers.insert("authorization", "Bearer zc_header".parse().unwrap());
        headers.insert("sec-websocket-protocol", "bearer.zc_sub".parse().unwrap());
        assert_eq!(
            extract_ws_token(&headers, Some("zc_query")),
            Some("zc_header")
        );
    }

    #[test]
    fn extract_ws_token_precedence_subprotocol_over_query() {
        let mut headers = HeaderMap::new();
        headers.insert("sec-websocket-protocol", "bearer.zc_sub".parse().unwrap());
        assert_eq!(extract_ws_token(&headers, Some("zc_query")), Some("zc_sub"));
    }

    #[test]
    fn extract_ws_token_returns_none_when_empty() {
        let headers = HeaderMap::new();
        assert_eq!(extract_ws_token(&headers, None), None);
    }

    #[test]
    fn extract_ws_token_skips_empty_header_value() {
        let mut headers = HeaderMap::new();
        headers.insert("authorization", "Bearer ".parse().unwrap());
        assert_eq!(
            extract_ws_token(&headers, Some("zc_fallback")),
            Some("zc_fallback")
        );
    }

    #[test]
    fn extract_ws_token_skips_empty_query_param() {
        let headers = HeaderMap::new();
        assert_eq!(extract_ws_token(&headers, Some("")), None);
    }

    #[test]
    fn extract_ws_token_subprotocol_with_multiple_entries() {
        let mut headers = HeaderMap::new();
        headers.insert(
            "sec-websocket-protocol",
            "kylinbot.v1, bearer.zc_tok, other".parse().unwrap(),
        );
        assert_eq!(extract_ws_token(&headers, None), Some("zc_tok"));
    }

    #[test]
    fn parse_ws_inbound_action_parses_cancel() {
        assert_eq!(
            parse_ws_inbound_action(r#"{"type":"cancel"}"#),
            WsInboundAction::Cancel
        );
    }

    #[test]
    fn parse_ws_inbound_action_parses_message_attachments() {
        let action = parse_ws_inbound_action(
            r#"{"type":"message","content":"","attachments":[{"file_name":"a.png","data_base64":"YWI=","mime_type":"image/png"}]}"#,
        );
        match action {
            WsInboundAction::Message {
                content,
                attachments,
            } => {
                assert!(content.is_empty());
                assert_eq!(attachments.len(), 1);
                assert_eq!(attachments[0].file_name, "a.png");
                assert_eq!(attachments[0].data, b"ab");
            }
            _ => panic!("expected message action"),
        }
    }

    #[test]
    fn resolve_session_cwd_uses_requested_cwd() {
        let requested = tempfile::tempdir().unwrap();
        let fallback = tempfile::tempdir().unwrap();

        let resolved =
            resolve_session_cwd(Some(requested.path().to_str().unwrap()), fallback.path()).unwrap();

        assert_eq!(resolved, requested.path().canonicalize().unwrap());
    }

    #[test]
    fn resolve_session_cwd_uses_default_workspace_without_request() {
        let fallback = tempfile::tempdir().unwrap();

        let resolved = resolve_session_cwd(None, fallback.path()).unwrap();

        assert_eq!(resolved, fallback.path().canonicalize().unwrap());
    }

    #[test]
    fn resolve_session_cwd_rejects_missing_directory() {
        let fallback = tempfile::tempdir().unwrap();
        let missing = fallback.path().join("missing");

        let err = resolve_session_cwd(Some(missing.to_str().unwrap()), fallback.path())
            .expect_err("missing cwd should be rejected");

        assert!(err.to_string().contains("cwd is not a usable directory"));
    }

    #[test]
    fn ws_model_route_detects_api_key_change() {
        use kylinbot_config::schema::ModelProviderConfig;

        let mut config = Config::default();
        config.providers.fallback = Some("deepseek".to_string());
        config.providers.models.insert(
            "deepseek".to_string(),
            ModelProviderConfig {
                model: Some("deepseek-v4-pro".to_string()),
                api_key: Some("sk-old".to_string()),
                ..Default::default()
            },
        );

        let route_old = ws_model_route_from_config(&config);
        config.providers.models.get_mut("deepseek").unwrap().api_key = Some("sk-new".to_string());
        let route_new = ws_model_route_from_config(&config);

        assert_eq!(route_old.provider, route_new.provider);
        assert_eq!(route_old.model, route_new.model);
        assert_ne!(route_old, route_new);
    }

    #[test]
    fn ws_agent_config_snapshot_detects_skill_toggle() {
        let mut config = Config::default();

        let snapshot_before = ws_agent_config_snapshot(&config);
        config.skills.enabled_skills.push("battery".to_string());
        let snapshot_after = ws_agent_config_snapshot(&config);

        assert_ne!(snapshot_before, snapshot_after);
        assert_eq!(snapshot_before.route, snapshot_after.route);
    }
}
