//! REST API handlers for the web dashboard.
//!
//! All `/api/*` routes require bearer token authentication (PairingGuard).

use super::AppState;
use axum::{
    extract::{Multipart, Path, Query, State},
    http::{HeaderMap, StatusCode, header},
    response::{IntoResponse, Json},
};
use serde::Deserialize;
use kylinbot_config::schema::{self, Config};
use kylinbot_memory::MemoryCategory;
use kylinbot_runtime::{
    cron, doctor, health,
    integrations::{self, IntegrationStatus},
    skills,
    tools::agent_runtime,
};
use kylinbot_tools::{claude_code_runner, cli_discovery};

fn find_file_ci(dir: &std::path::Path, name: &str) -> Option<std::path::PathBuf> {
    let exact = dir.join(name);
    if exact.exists() {
        return Some(exact);
    }
    let lower = name.to_ascii_lowercase();
    let Ok(entries) = std::fs::read_dir(dir) else {
        return None;
    };
    for entry in entries.flatten() {
        let file_name = entry.file_name();
        if file_name.to_string_lossy().to_ascii_lowercase() == lower {
            return Some(entry.path());
        }
    }
    None
}

async fn add_skill_to_enabled(state: &AppState, name: &str) {
    let mut config = state.config.lock().clone();
    if !config
        .skills
        .enabled_skills
        .iter()
        .any(|s| s.eq_ignore_ascii_case(name))
    {
        config.skills.enabled_skills.push(name.to_string());
    }
    config
        .skills
        .default_skill
        .disabled_skills
        .retain(|s| !s.eq_ignore_ascii_case(name));
    if let Err(e) = config.save().await {
        tracing::warn!("Failed to save config after enabling skill '{name}': {e}");
    } else {
        *state.config.lock() = config;
    }
}

async fn reload_config_and_daemon(state: &AppState) -> Result<(), &'static str> {
    // Pull the fresh config off disk and compute a diff against the current
    // in-memory copy. Only the subsystems whose effective config changed are
    // restarted — the gateway itself stays up if `[gateway]` did not change,
    // the channels supervisor only respawns if `[channels.*]` changed, and so
    // on. The daemon-level `ChannelWorkerPool` is shared across reloads so
    // in-flight agent inference is never interrupted.
    let new_config = match Config::load_or_init().await {
        Ok(mut c) => {
            c.apply_env_overrides();
            c
        }
        Err(e) => {
            tracing::warn!("Failed to reload config into gateway memory: {e}");
            return Err("config load failed");
        }
    };

    let old_config = state.config.lock().clone();
    let affected = kylinbot_runtime::daemon::diff_subsystems(&old_config, &new_config);

    // Update in-memory config unconditionally — every gateway handler reads
    // through `state.config.lock()`, so this is what makes "save took effect"
    // visible even when no subsystem needs to restart.
    *state.config.lock() = new_config.clone();

    if affected.is_empty() {
        tracing::info!("config reload: no subsystem affected, in-memory swap only");
        return Ok(());
    }

    let Some(reload_tx) = state.reload_tx.clone() else {
        return Err("no daemon supervisor");
    };

    let req = kylinbot_runtime::daemon::ReloadRequest {
        new_config,
        affected,
    };
    tokio::spawn(async move {
        // Brief delay so the HTTP response flushes before reload begins.
        tokio::time::sleep(std::time::Duration::from_millis(200)).await;
        if let Err(e) = reload_tx.send(req).await {
            tracing::warn!("config reload: failed to enqueue request: {e}");
        }
    });

    Ok(())
}

async fn save_config_and_reload(state: &AppState, new_config: Config) -> Result<(), String> {
    new_config
        .validate()
        .map_err(|e| format!("Invalid config: {e}"))?;
    new_config
        .save()
        .await
        .map_err(|e| format!("Failed to save config: {e}"))?;
    if let Err(e) = reload_config_and_daemon(state).await {
        tracing::warn!("Config saved but daemon reload was not triggered: {e}");
    }
    Ok(())
}

fn wechat_state_dir_from_config(config: &Config) -> std::path::PathBuf {
    let config_path = config.config_path.clone();
    config
        .channels
        .wechat
        .as_ref()
        .and_then(|w| w.state_dir.clone())
        .map(std::path::PathBuf::from)
        .or_else(|| config_path.parent().map(|p| p.join("wechat")))
        .unwrap_or_else(|| std::path::PathBuf::from(".kylinbot/wechat"))
}

async fn remove_wechat_state_dir(config: &Config) {
    let state_dir = wechat_state_dir_from_config(config);
    if state_dir.exists()
        && let Err(e) = tokio::fs::remove_dir_all(&state_dir).await
    {
        tracing::warn!("WeChat: failed to remove state dir {state_dir:?}: {e}");
    }
}

// ── Bearer token auth extractor ─────────────────────────────────

/// Extract and validate bearer token from Authorization header.
fn extract_bearer_token(headers: &HeaderMap) -> Option<&str> {
    headers
        .get(header::AUTHORIZATION)
        .and_then(|v| v.to_str().ok())
        .and_then(|auth| auth.strip_prefix("Bearer "))
}

/// Verify bearer token against PairingGuard. Returns error response if unauthorized.
pub(super) fn require_auth(
    state: &AppState,
    headers: &HeaderMap,
) -> Result<(), (StatusCode, Json<serde_json::Value>)> {
    if !state.pairing.require_pairing() {
        return Ok(());
    }

    let token = extract_bearer_token(headers).unwrap_or("");
    if state.pairing.is_authenticated(token) {
        Ok(())
    } else {
        Err((
            StatusCode::UNAUTHORIZED,
            Json(serde_json::json!({
                "error": "Unauthorized — pair first via POST /pair, then send Authorization: Bearer <token>"
            })),
        ))
    }
}

// ── Query parameters ─────────────────────────────────────────────

#[derive(Deserialize)]
pub struct MemoryQuery {
    pub query: Option<String>,
    pub category: Option<String>,
    /// Filter memories created at or after (RFC 3339 / ISO 8601)
    pub since: Option<String>,
    /// Filter memories created at or before (RFC 3339 / ISO 8601)
    pub until: Option<String>,
}

#[derive(Deserialize)]
pub struct MemoryStoreBody {
    pub key: String,
    pub content: String,
    pub category: Option<String>,
}

#[derive(Deserialize)]
pub struct CostPurgeBody {
    pub models: Vec<String>,
}

#[derive(Deserialize)]
pub struct CronRunsQuery {
    pub limit: Option<u32>,
}

#[derive(Deserialize)]
pub struct CronRecentRunsQuery {
    pub limit: Option<u32>,
    pub job_type: Option<String>,
}

#[derive(Deserialize)]
pub struct CronAddBody {
    pub name: Option<String>,
    pub schedule: serde_json::Value,
    pub command: Option<String>,
    pub job_type: Option<String>,
    pub prompt: Option<String>,
    pub enabled: Option<bool>,
    pub delivery: Option<cron::DeliveryConfig>,
    pub session_target: Option<String>,
    pub model: Option<String>,
    pub allowed_tools: Option<Vec<String>>,
    pub delete_after_run: Option<bool>,
}

#[derive(Deserialize)]
pub struct CronPatchBody {
    pub name: Option<String>,
    pub schedule: Option<serde_json::Value>,
    pub command: Option<String>,
    pub prompt: Option<String>,
    pub enabled: Option<bool>,
    pub delivery: Option<cron::DeliveryConfig>,
    pub model: Option<String>,
    pub session_target: Option<String>,
    pub delete_after_run: Option<bool>,
    pub allowed_tools: Option<Vec<String>>,
    pub uses_memory: Option<bool>,
}

#[derive(Deserialize)]
pub struct CronRunNowBody {
    pub approved: Option<bool>,
}

fn parse_schedule_input(value: &serde_json::Value) -> Result<cron::Schedule, String> {
    if let Some(expr) = value.as_str() {
        let trimmed = expr.trim();
        if trimmed.is_empty() {
            return Err("schedule cannot be empty".to_string());
        }
        return Ok(cron::Schedule::Cron {
            expr: trimmed.to_string(),
            tz: None,
        });
    }
    cron::deserialize_maybe_stringified::<cron::Schedule>(value)
        .map_err(|e| format!("invalid schedule payload: {e}"))
}

// ── Handlers ────────────────────────────────────────────────────

/// GET /api/status — system status overview
pub async fn handle_api_status(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let health = health::snapshot();

    let mut channels = serde_json::Map::new();

    for (channel, present) in config.channels.channels() {
        channels.insert(channel.name().to_string(), serde_json::Value::Bool(present));
    }

    let (active_provider, active_model) = super::resolve_fallback_model_route(&config);
    let body = serde_json::json!({
        "provider": active_provider,
        "model": active_model,
        "temperature": state.temperature,
        "uptime_seconds": health.uptime_seconds,
        "gateway_port": config.gateway.port,
        "version": env!("CARGO_PKG_VERSION"),
        "locale": config.locale.as_deref().unwrap_or_else(|| {
            std::env::var("LANG")
                .or_else(|_| std::env::var("LC_ALL"))
                .or_else(|_| std::env::var("KYLINBOT_LOCALE"))
                .map(|v| {
                    let lower = v.to_lowercase();
                    if lower.starts_with("zh") { "zh" }
                    else if lower.starts_with("tr") { "tr" }
                    else { "en" }
                })
                .unwrap_or("en")
        }),
        "memory_backend": state.mem.name(),
        "paired": state.pairing.is_paired(),
        "channels": channels,
        "health": health,
    });

    Json(body).into_response()
}

/// GET /api/config — current config (api_key masked)
pub async fn handle_api_config_get(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();

    // Serialize to TOML after masking sensitive fields.
    let masked_config = mask_sensitive_fields(&config);
    let toml_str = match toml::to_string_pretty(&masked_config) {
        Ok(s) => s,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to serialize config: {e}")})),
            )
                .into_response();
        }
    };

    Json(serde_json::json!({
        "format": "toml",
        "content": toml_str,
    }))
    .into_response()
}

/// PUT /api/config — update config from TOML body
pub async fn handle_api_config_put(
    State(state): State<AppState>,
    headers: HeaderMap,
    body: String,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    // Parse the incoming TOML
    let incoming: Config = match toml::from_str(&body) {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({"error": format!("Invalid TOML: {e}")})),
            )
                .into_response();
        }
    };

    let current_config = state.config.lock().clone();
    let new_config = hydrate_config_for_save(incoming, &current_config);

    if let Err(e) = save_config_and_reload(&state, new_config).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": e})),
        )
            .into_response();
    }

    Json(serde_json::json!({"status": "ok"})).into_response()
}

/// POST /api/channels/reload — authenticated dashboard/CLI entrypoint.
pub async fn handle_api_channels_reload(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    match reload_config_and_daemon(&state).await {
        Ok(()) => Json(serde_json::json!({
            "success": true,
            "message": "Daemon reload initiated. Channels will restart with the latest configuration."
        }))
        .into_response(),
        Err(e) => (
            StatusCode::SERVICE_UNAVAILABLE,
            Json(serde_json::json!({
                "success": false,
                "error": e,
                "message": "Running as standalone gateway. Restart the process to pick up channel changes."
            })),
        )
            .into_response(),
    }
}

/// GET /api/tools — list registered tool specs
pub async fn handle_api_tools(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let tools: Vec<serde_json::Value> = state
        .tools_registry
        .iter()
        .map(|spec| {
            serde_json::json!({
                "name": spec.name,
                "description": spec.description,
                "parameters": spec.parameters,
            })
        })
        .collect();

    Json(serde_json::json!({"tools": tools})).into_response()
}

/// GET /api/channels — list configured channel details for the dashboard
pub async fn handle_api_channels(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let health = health::snapshot();

    let channels: Vec<serde_json::Value> = config
        .channels
        .channels()
        .into_iter()
        .map(|(channel, enabled)| {
            let name = channel.name().to_string();
            let component = health.components.get(&name);
            let component_status = component.map(|c| c.status.as_str()).unwrap_or("unknown");
            let identifier = match name.as_str() {
                "WeChat" => {
                    let state_dir = wechat_state_dir_from_config(&config);
                    std::fs::read_to_string(state_dir.join("account.json"))
                        .ok()
                        .and_then(|content| serde_json::from_str::<serde_json::Value>(&content).ok())
                        .and_then(|json| {
                            json.get("account_id")
                                .and_then(|v| v.as_str())
                                .map(|value| serde_json::json!({
                                    "label": "Account ID",
                                    "value": value.split('@').next().unwrap_or(value),
                                }))
                        })
                }
                "WeCom" => config.channels.wecom.as_ref().map(|wecom| {
                    serde_json::json!({
                        "label": "Bot ID",
                        "value": wecom.bot_id,
                    })
                }),
                "DingTalk" => config.channels.dingtalk.as_ref().map(|dingtalk| {
                    serde_json::json!({
                        "label": "Client ID",
                        "value": dingtalk.client_id,
                    })
                }),
                "Feishu" => config.channels.feishu.as_ref().map(|feishu| {
                    serde_json::json!({
                        "label": "App ID",
                        "value": feishu.app_id,
                    })
                }),
                "Lansenger" => config.channels.lansenger.as_ref().map(|lansenger| {
                    serde_json::json!({
                        "label": "App ID",
                        "value": lansenger.app_id,
                    })
                }),
                _ => None,
            };

            let (status, health_label) = if !enabled {
                ("inactive", "down")
            } else if component_status.eq_ignore_ascii_case("ok")
                || component_status.eq_ignore_ascii_case("healthy")
            {
                ("active", "healthy")
            } else if component.is_some() {
                ("error", "degraded")
            } else {
                ("active", "healthy")
            };

            serde_json::json!({
                "name": name,
                "type": channel.name(),
                "enabled": enabled,
                "status": status,
                "message_count": 0,
                "last_message_at": serde_json::Value::Null,
                "health": health_label,
                "identifier": identifier,
            })
        })
        .collect();

    Json(serde_json::json!({ "channels": channels })).into_response()
}

/// DELETE /api/channels/{channel} — remove a channel config and reload.
pub async fn handle_api_channel_disconnect(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(channel): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let normalized = channel.to_ascii_lowercase().replace('-', "_");
    let mut new_config = state.config.lock().clone();
    let was_configured = match normalized.as_str() {
        "wechat" => new_config.channels.wechat.take().is_some(),
        "wecom" => new_config.channels.wecom.take().is_some(),
        "dingtalk" => new_config.channels.dingtalk.take().is_some(),
        "feishu" => new_config.channels.feishu.take().is_some(),
        "lansenger" => new_config.channels.lansenger.take().is_some(),
        _ => {
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({
                    "error": format!("Unsupported channel '{channel}'")
                })),
            )
                .into_response();
        }
    };

    if let Err(e) = save_config_and_reload(&state, new_config.clone()).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": e})),
        )
            .into_response();
    }

    if normalized == "wechat" {
        remove_wechat_state_dir(&new_config).await;
    }

    Json(serde_json::json!({
        "status": "ok",
        "channel": normalized,
        "disconnected": was_configured,
    }))
    .into_response()
}

/// GET /api/skills — list installed skills with enable status
pub async fn handle_api_skills(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let skills = skills::load_skills_with_config(&config.workspace_dir, &config);

    let enabled_skills = config.skills.enabled_skills.clone();
    let default_skills = config.skills.default_skill.default_skills.clone();
    let disabled_skills = config.skills.default_skill.disabled_skills.clone();

    let skills_json: Vec<serde_json::Value> = skills
        .iter()
        .map(|skill| {
            let slug = skill
                .location
                .as_ref()
                .and_then(|p| p.parent())
                .and_then(|p| p.file_name())
                .and_then(|n| n.to_str())
                .unwrap_or(&skill.name)
                .to_string();
            let is_explicitly_enabled = enabled_skills
                .iter()
                .any(|s| s.eq_ignore_ascii_case(&slug));
            let is_default = default_skills
                .iter()
                .any(|s| s.eq_ignore_ascii_case(&slug));
            let is_disabled = disabled_skills
                .iter()
                .any(|s| s.eq_ignore_ascii_case(&slug));
            let is_enabled = (is_explicitly_enabled || is_default) && !is_disabled;
            serde_json::json!({
                "name": skill.name,
                "slug": slug,
                "description": skill.description,
                "version": skill.version,
                "author": skill.author,
                "tags": skill.tags,
                "enabled": is_enabled,
                "is_default": is_default,
            })
        })
        .collect();

    Json(serde_json::json!({ "skills": skills_json })).into_response()
}

/// PUT /api/skills/{name}/toggle — toggle skill enabled state
pub async fn handle_api_skills_toggle(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(name): Path<String>,
    body: String,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let payload: serde_json::Value = match serde_json::from_str(&body) {
        Ok(p) => p,
        Err(e) => {
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({ "error": format!("Invalid JSON: {e}") })),
            )
                .into_response();
        }
    };

    let enabled = match payload.get("enabled").and_then(|v| v.as_bool()) {
        Some(e) => e,
        None => {
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({ "error": "Missing or invalid 'enabled' field" })),
            )
                .into_response();
        }
    };

    let mut config = state.config.lock().clone();

    if enabled {
        config
            .skills
            .default_skill
            .disabled_skills
            .retain(|s| !s.eq_ignore_ascii_case(&name));
        if !config
            .skills
            .enabled_skills
            .iter()
            .any(|s| s.eq_ignore_ascii_case(&name))
        {
            config.skills.enabled_skills.push(name.clone());
        }
    } else {
        config
            .skills
            .enabled_skills
            .retain(|s| !s.eq_ignore_ascii_case(&name));
        if !config
            .skills
            .default_skill
            .disabled_skills
            .iter()
            .any(|s| s.eq_ignore_ascii_case(&name))
        {
            config
                .skills
                .default_skill
                .disabled_skills
                .push(name.clone());
        }
    }

    if let Err(e) = config.save().await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to save config: {e}") })),
        )
            .into_response();
    }

    *state.config.lock() = config;

    Json(serde_json::json!({ "status": "ok", "enabled": enabled })).into_response()
}

/// GET /api/skills/{name} — get single skill details
pub async fn handle_api_skill_get(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(name): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let all_skills = skills::load_skills_with_config(&config.workspace_dir, &config);

    // Find the skill
    let skill = all_skills.iter().find(|s| s.name.eq_ignore_ascii_case(&name));

    match skill {
        Some(s) => {
            let is_enabled = config
                .skills
                .enabled_skills
                .iter()
                .any(|e| e.eq_ignore_ascii_case(&s.name));

            let tools_json: Vec<serde_json::Value> = s
                .tools
                .iter()
                .map(|t| {
                    serde_json::json!({
                        "name": t.name,
                        "description": t.description,
                        "kind": t.kind,
                        "command": t.command,
                    })
                })
                .collect();

            Json(serde_json::json!({
                "name": s.name,
                "description": s.description,
                "version": s.version,
                "author": s.author,
                "tags": s.tags,
                "enabled": is_enabled,
                "tools": tools_json,
            }))
            .into_response()
        }
        None => (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({ "error": format!("Skill '{}' not found", name) })),
        )
            .into_response(),
    }
}

#[derive(Deserialize)]
pub struct SkillCreateBody {
    #[serde(default)]
    pub description: Option<String>,
    #[serde(default)]
    pub author: Option<String>,
    #[serde(default)]
    pub tags: Option<Vec<String>>,
    #[serde(default)]
    pub tools: Option<Vec<serde_json::Value>>,
    #[serde(default)]
    pub prompts: Option<Vec<String>>,
}

/// PUT /api/skills/{name} — create or update skill
pub async fn handle_api_skill_put(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(name): Path<String>,
    Json(body): Json<SkillCreateBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let skill_name = name.trim();
    if skill_name.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({ "error": "Skill name cannot be empty" })),
        )
            .into_response();
    }

    let config = state.config.lock().clone();
    let skills_dir = config.workspace_dir.join("skills").join(&name);

    // Create skill directory
    if let Err(e) = tokio::fs::create_dir_all(&skills_dir).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to create skill directory: {e}") })),
        )
            .into_response();
    }

    // Build SKILL.md content
    let mut content = String::new();
    content.push_str("---\n");
    content.push_str(&format!("name: {}\n", skill_name));
    if let Some(desc) = &body.description {
        content.push_str(&format!("description: {}\n", desc));
    }
    content.push_str("---\n");

    if let Some(prompts) = &body.prompts {
        for prompt in prompts {
            content.push_str(&format!("{}\n", prompt));
        }
    }

    // Build tool definitions
    if let Some(tools) = &body.tools {
        if !tools.is_empty() {
            content.push_str("## Tools\n\n");
            for tool in tools {
                let tool_name = tool.get("name").and_then(|v| v.as_str()).unwrap_or("tool");
                let desc = tool.get("description").and_then(|v| v.as_str()).unwrap_or("");
                let kind = tool.get("kind").and_then(|v| v.as_str()).unwrap_or("shell");
                let cmd = tool.get("command").and_then(|v| v.as_str()).unwrap_or("");

                content.push_str(&format!("### {}\n\n", tool_name));
                content.push_str(&format!("{}\n\n", desc));
                content.push_str("```toml\n");
                content.push_str(&format!("[tool.{}]\n", tool_name));
                content.push_str(&format!("kind = \"{}\"\n", kind));
                content.push_str(&format!("command = \"{}\"\n", cmd));
                content.push_str("```\n\n");
            }
        }
    }

    let skill_md_path = skills_dir.join("SKILL.md");
    if let Err(e) = tokio::fs::write(&skill_md_path, &content).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to write skill file: {e}") })),
        )
            .into_response();
    }

    add_skill_to_enabled(&state, &name).await;

    Json(serde_json::json!({
        "status": "ok",
        "name": name,
        "message": "Skill created/updated successfully"
    }))
    .into_response()
}

/// DELETE /api/skills/{name} — delete skill
pub async fn handle_api_skill_delete(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(name): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let skill_name = name.trim();
    if skill_name.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({ "error": "Skill name cannot be empty" })),
        )
            .into_response();
    }

    let config = state.config.lock().clone();
    let skills_dir = config.workspace_dir.join("skills").join(&name);

    // Read display name from SKILL.md BEFORE deleting the directory
    let skill_md_path = find_file_ci(&skills_dir, "SKILL.md");
    let display_name = match skill_md_path.as_ref() {
        Some(p) => match tokio::fs::read_to_string(p).await {
            Ok(content) => content
                .lines()
                .find_map(|line| {
                    let t = line.trim();
                    if t.starts_with("name:") || t.starts_with("name :") {
                        Some(
                            t.trim_start_matches("name")
                                .trim_start_matches(':')
                                .trim_start_matches(' ')
                                .trim()
                                .to_string(),
                        )
                    } else {
                        None
                    }
                })
                .unwrap_or_else(|| name.clone()),
            Err(_) => name.clone(),
        },
        None => name.clone(),
    };

    // Delete skill directory if it exists
    if skills_dir.exists() {
        if let Err(e) = tokio::fs::remove_dir_all(&skills_dir).await {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({ "error": format!("Failed to delete skill: {e}") })),
            )
                .into_response();
        }
    }

    // Remove from enabled and disabled lists (match both slug and display name)
    let mut updated_config = state.config.lock().clone();
    let before_remove = updated_config.skills.enabled_skills.clone();
    updated_config
        .skills
        .enabled_skills
        .retain(|s| !s.eq_ignore_ascii_case(&name) && !s.eq_ignore_ascii_case(&display_name));
    updated_config
        .skills
        .default_skill
        .disabled_skills
        .retain(|s| !s.eq_ignore_ascii_case(&name) && !s.eq_ignore_ascii_case(&display_name));
    let after_remove = updated_config.skills.enabled_skills.clone();
    tracing::warn!(
        "DELETE skill: name={name:?}, display_name={display_name:?}, enabled_skills before={before_remove:?}, after={after_remove:?}"
    );

    if let Err(e) = updated_config.save().await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to save config: {e}") })),
        )
            .into_response();
    }
    *state.config.lock() = updated_config;

    Json(serde_json::json!({
        "status": "ok",
        "name": name,
        "message": "Skill deleted successfully"
    }))
    .into_response()
}

/// POST /api/skills/import — import a skill from .zip, .md, or folder upload
pub async fn handle_api_skill_import(
    State(state): State<AppState>,
    headers: HeaderMap,
    mut multipart: Multipart,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let skills_dir = config.workspace_dir.join("skills");
    if let Err(e) = tokio::fs::create_dir_all(&skills_dir).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to create skills dir: {e}") })),
        )
            .into_response();
    }

    while let Ok(Some(field)) = multipart.next_field().await {
        let field_name = field.name().unwrap_or("").to_string();
        let filename = field.file_name().unwrap_or("").to_string();
        let data = match field.bytes().await {
            Ok(d) => d,
            Err(e) => {
                return (
                    StatusCode::BAD_REQUEST,
                    Json(serde_json::json!({ "error": format!("Failed to read upload: {e}") })),
                )
                    .into_response();
            }
        };

        if field_name != "file" || data.is_empty() {
            continue;
        }

        if filename.to_lowercase().ends_with(".zip") {
            let reader = std::io::Cursor::new(&data[..]);
            let mut archive = match zip::ZipArchive::new(reader) {
                Ok(a) => a,
                Err(e) => {
                    return (
                        StatusCode::BAD_REQUEST,
                        Json(serde_json::json!({ "error": format!("Invalid zip file: {e}") })),
                    )
                        .into_response();
                }
            };

            let mut skill_name: Option<String> = None;
            let mut has_skill_md = false;
            let mut has_subdir = false;
            let mut entries: Vec<(String, bool, Vec<u8>)> = Vec::new();
            for i in 0..archive.len() {
                let mut file = match archive.by_index(i) {
                    Ok(f) => f,
                    Err(_) => continue,
                };
                let path_str = file.name().to_string();
                if path_str.contains("..") {
                    continue;
                }

                let has_slash = path_str.contains('/');
                if has_slash {
                    has_subdir = true;
                }

                let file_name = path_str.rsplit('/').next().unwrap_or(&path_str);
                if file_name.eq_ignore_ascii_case("SKILL.md") {
                    has_skill_md = true;
                }

                let top = path_str.split('/').next().unwrap_or(&path_str);
                if skill_name.is_none() && has_slash {
                    skill_name = Some(top.to_string());
                }

                if file.is_dir() {
                    entries.push((path_str, true, Vec::new()));
                } else {
                    let mut buf = Vec::new();
                    use std::io::Read;
                    let _ = file.read_to_end(&mut buf);
                    entries.push((path_str, false, buf));
                }
            }

            if !has_skill_md {
                return (
                    StatusCode::BAD_REQUEST,
                    Json(serde_json::json!({ "error": "技能添加失败" })),
                )
                    .into_response();
            }

            if !has_subdir {
                if let Some((_, _, data)) = entries.iter().find(|(p, _, _)| {
                    p.rsplit('/').next().unwrap_or(p).eq_ignore_ascii_case("SKILL.md")
                }) {
                    if let Ok(content) = String::from_utf8(data.clone()) {
                        skill_name = content
                            .lines()
                            .find_map(|line| {
                                let t = line.trim();
                                if t.starts_with("name:") || t.starts_with("name :") {
                                    Some(
                                        t.trim_start_matches("name")
                                            .trim_start_matches(':')
                                            .trim_start_matches(' ')
                                            .trim()
                                            .to_string(),
                                    )
                                } else {
                                    None
                                }
                            });
                    }
                }
                skill_name = skill_name.filter(|s| !s.is_empty())
                    .or_else(|| {
                        Some(filename.trim_end_matches(".zip").trim_end_matches(".ZIP").to_string())
                    });
            }

            let resolved_name = skill_name.clone().unwrap_or_else(|| "imported-skill".into());
            if skills_dir.join(&resolved_name).exists() {
                return (
                    StatusCode::CONFLICT,
                    Json(serde_json::json!({ "error": "技能已存在", "duplicate": true, "name": resolved_name })),
                )
                    .into_response();
            }

            for (path_str, is_dir, data) in &entries {
                let relative = if has_subdir {
                    path_str.clone()
                } else {
                    let fname = path_str.rsplit('/').next().unwrap_or(path_str);
                    format!("{}/{}", resolved_name, fname)
                };
                let dest = skills_dir.join(&relative);
                if *is_dir {
                    let _ = tokio::fs::create_dir_all(&dest).await;
                } else {
                    if let Some(parent) = dest.parent() {
                        let _ = tokio::fs::create_dir_all(parent).await;
                    }
                    let _ = tokio::fs::write(&dest, data).await;
                }
            }
            let name = skill_name.unwrap_or_else(|| "imported-skill".into());
            add_skill_to_enabled(&state, &name).await;
            return Json(serde_json::json!({
                "status": "ok",
                "name": name,
                "message": "Skill imported from zip successfully"
            }))
            .into_response();
        }

        if filename.to_lowercase().ends_with(".md") {
            let content = match String::from_utf8(data.to_vec()) {
                Ok(c) => c,
                Err(e) => {
                    return (
                        StatusCode::BAD_REQUEST,
                        Json(serde_json::json!({ "error": format!("Invalid text file: {e}") })),
                    )
                        .into_response();
                }
            };
            let skill_name = content
                .lines()
                .find_map(|line| {
                    let trimmed = line.trim();
                    if trimmed.starts_with("name:") || trimmed.starts_with("name :") {
                        Some(trimmed.trim_start_matches("name").trim_start_matches(':').trim_start_matches(' ').trim().to_string())
                    } else {
                        None
                    }
                })
                .unwrap_or_else(|| {
                    filename
                        .trim_end_matches(".md")
                        .trim_end_matches(".MD")
                        .to_string()
                });
            let skill_dir = skills_dir.join(&skill_name);
            if skill_dir.exists() {
                return (
                    StatusCode::CONFLICT,
                    Json(serde_json::json!({ "error": "技能已存在", "duplicate": true, "name": skill_name })),
                )
                    .into_response();
            }
            if let Err(e) = tokio::fs::create_dir_all(&skill_dir).await {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("Failed to create skill dir: {e}") })),
                )
                    .into_response();
            }
            if let Err(e) = tokio::fs::write(skill_dir.join("SKILL.md"), &content).await {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("Failed to write SKILL.md: {e}") })),
                )
                    .into_response();
            }
            add_skill_to_enabled(&state, &skill_name).await;
            return Json(serde_json::json!({
                "status": "ok",
                "name": skill_name,
                "message": "Skill imported from md successfully"
            }))
            .into_response();
        }

        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({ "error": "Unsupported file type. Use .zip or .md" })),
        )
            .into_response();
    }

    (
        StatusCode::BAD_REQUEST,
        Json(serde_json::json!({ "error": "No file uploaded" })),
    )
        .into_response()
}

/// POST /api/skills/import-folder — import a skill from multiple files (folder upload)
pub async fn handle_api_skill_import_folder(
    State(state): State<AppState>,
    headers: HeaderMap,
    mut multipart: Multipart,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let skills_dir = config.workspace_dir.join("skills");
    if let Err(e) = tokio::fs::create_dir_all(&skills_dir).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to create skills dir: {e}") })),
        )
            .into_response();
    }

    let mut top_folder: Option<String> = None;
    let mut file_count: u32 = 0;

    while let Ok(Some(field)) = multipart.next_field().await {
        let field_name = field.name().unwrap_or("").to_string();
        if field_name != "files" {
            continue;
        }
        let filename = field.file_name().unwrap_or("").to_string();
        let data = match field.bytes().await {
            Ok(d) => d,
            Err(_) => continue,
        };

        if filename.is_empty() || filename.contains("..") {
            continue;
        }
        if top_folder.is_none() {
            let parts: Vec<&str> = filename.split('/').collect();
            if !parts.is_empty() {
                top_folder = Some(parts[0].to_string());
            }
        }

        let dest = skills_dir.join(&filename);
        if let Some(parent) = dest.parent() {
            let _ = tokio::fs::create_dir_all(parent).await;
        }
        let _ = tokio::fs::write(&dest, &data).await;
        file_count += 1;
    }

    let name = top_folder.unwrap_or_else(|| "imported-skill".into());
    if file_count == 0 {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({ "error": "No files found in folder" })),
        )
            .into_response();
    }

    add_skill_to_enabled(&state, &name).await;

    Json(serde_json::json!({
        "status": "ok",
        "name": name,
        "message": format!("Skill imported from folder ({} files)", file_count)
    }))
    .into_response()
}

/// POST /api/skills/open-skillhub — open skillhub in system browser
pub async fn handle_api_skills_open_skillhub(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let base_url = "http://localhost:42617/skillhub";

    let token = read_external_gateway_token();
    let url = match token {
        Some(t) if !t.is_empty() => format!("{base_url}?token={t}"),
        _ => base_url.to_string(),
    };

    #[cfg(target_os = "linux")]
    {
        let mut last_error = String::new();
        for cmd in ["xdg-open", "gio", "sensible-browser"] {
            let mut command = tokio::process::Command::new(cmd);
            if cmd == "gio" {
                command.arg("open");
            }
            command.arg(&url);
            match command.status().await {
                Ok(status) if status.success() => {
                    return Json(serde_json::json!({ "status": "ok", "message": "Opened skillhub in system browser" })).into_response();
                }
                Ok(status) => {
                    last_error = format!("{cmd} exited with status {status}");
                }
                Err(error) => {
                    last_error = format!("{cmd} not runnable: {error}");
                }
            }
        }
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to open browser: {last_error}") })),
        )
            .into_response();
    }

    #[cfg(target_os = "macos")]
    {
        match tokio::process::Command::new("open").arg(&url).status().await {
            Ok(status) if status.success() => {
                return Json(serde_json::json!({ "status": "ok", "message": "Opened skillhub in system browser" })).into_response();
            }
            Ok(status) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(
                        serde_json::json!({ "error": format!("open exited with status {status}") }),
                    ),
                )
                    .into_response();
            }
            Err(error) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("open not runnable: {error}") })),
                )
                    .into_response();
            }
        }
    }

    #[cfg(target_os = "windows")]
    {
        match tokio::process::Command::new("rundll32")
            .arg("url.dll,FileProtocolHandler")
            .arg(&url)
            .status()
            .await
        {
            Ok(status) if status.success() => {
                return Json(serde_json::json!({ "status": "ok", "message": "Opened skillhub in system browser" })).into_response();
            }
            Ok(status) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("rundll32 exited with status {status}") })),
                )
                    .into_response();
            }
            Err(error) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("rundll32 not runnable: {error}") })),
                )
                    .into_response();
            }
        }
    }

    #[cfg(not(any(target_os = "linux", target_os = "macos", target_os = "windows")))]
    {
        let _ = url;
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": "Opening browser is not supported on this OS" })),
        )
            .into_response();
    }
}

/// POST /api/skills/{name}/open-folder — open skill folder in system file manager
pub async fn handle_api_skill_open_folder(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(name): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let skill_dir = config.workspace_dir.join("skills").join(&name);

    if !skill_dir.exists() {
        return (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({ "error": format!("Skill folder not found: {}", skill_dir.display()) })),
        )
            .into_response();
    }

    let path_str = skill_dir.to_string_lossy().to_string();

    #[cfg(target_os = "linux")]
    {
        let mut last_error = String::new();
        for cmd in ["xdg-open", "gio"] {
            let mut command = tokio::process::Command::new(cmd);
            if cmd == "gio" {
                command.arg("open");
            }
            command.arg(&path_str);
            match command.status().await {
                Ok(status) if status.success() => {
                    return Json(serde_json::json!({ "status": "ok", "message": "Opened skill folder" })).into_response();
                }
                Ok(status) => {
                    last_error = format!("{cmd} exited with status {status}");
                }
                Err(error) => {
                    last_error = format!("{cmd} not runnable: {error}");
                }
            }
        }
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": format!("Failed to open folder: {last_error}") })),
        )
            .into_response();
    }

    #[cfg(target_os = "macos")]
    {
        match tokio::process::Command::new("open").arg(&path_str).status().await {
            Ok(status) if status.success() => {
                return Json(serde_json::json!({ "status": "ok", "message": "Opened skill folder" })).into_response();
            }
            Ok(status) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("open exited with status {status}") })),
                )
                    .into_response();
            }
            Err(error) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("open not runnable: {error}") })),
                )
                    .into_response();
            }
        }
    }

    #[cfg(target_os = "windows")]
    {
        match tokio::process::Command::new("explorer").arg(&path_str).status().await {
            Ok(status) if status.success() => {
                return Json(serde_json::json!({ "status": "ok", "message": "Opened skill folder" })).into_response();
            }
            Ok(status) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("explorer exited with status {status}") })),
                )
                    .into_response();
            }
            Err(error) => {
                return (
                    StatusCode::INTERNAL_SERVER_ERROR,
                    Json(serde_json::json!({ "error": format!("explorer not runnable: {error}") })),
                )
                    .into_response();
            }
        }
    }

    #[cfg(not(any(target_os = "linux", target_os = "macos", target_os = "windows")))]
    {
        let _ = path_str;
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({ "error": "Opening folder is not supported on this OS" })),
        )
            .into_response();
    }
}

/// Read `gateway-token` from the external desktop app's settings.ini.
///
/// Looks for `~/.config/kylin-aiassistant/settings.ini` and parses the
/// `gateway-token` key under the `[General]` section. Returns `None` if
/// the file doesn't exist or the key is absent.
fn read_external_gateway_token() -> Option<String> {
    let home = directories::UserDirs::new()?.home_dir().to_path_buf();
    let path = home.join(".config/kylin-aiassistant/settings.ini");
    let content = std::fs::read_to_string(&path).ok()?;
    for line in content.lines() {
        if let Some((key, value)) = line.split_once('=') {
            if key.trim() == "gateway-token" {
                let token = value.trim().to_string();
                if !token.is_empty() {
                    return Some(token);
                }
            }
        }
    }
    None
}

/// GET /api/agents — list configured delegate agent profiles and live runtime state
pub async fn handle_api_agents(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let agents: Vec<serde_json::Value> = config
        .agents
        .iter()
        .map(|(name, agent)| {
            let runtime = agent_runtime::snapshot(name);
            serde_json::json!({
                "name": name,
                "provider": agent.provider,
                "model": agent.model,
                "system_prompt": agent.system_prompt,
                "temperature": agent.temperature,
                "max_depth": agent.max_depth,
                "agentic": agent.agentic,
                "allowed_tools": agent.allowed_tools,
                "max_iterations": agent.max_iterations,
                "skills_directory": agent.skills_directory,
                "runtime": {
                    "status": runtime.status,
                    "stage": runtime.stage,
                    "active_tasks": runtime.active_tasks,
                    "current_task": runtime.current_task,
                    "started_at": runtime.started_at,
                    "last_started_at": runtime.last_started_at,
                    "last_finished_at": runtime.last_finished_at,
                    "last_error": runtime.last_error,
                }
            })
        })
        .collect();

    Json(serde_json::json!({ "agents": agents })).into_response()
}

/// GET /api/cron — list cron jobs
pub async fn handle_api_cron_list(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    match cron::list_jobs(&config) {
        Ok(jobs) => Json(serde_json::json!({"jobs": jobs})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to list cron jobs: {e}")})),
        )
            .into_response(),
    }
}

/// POST /api/cron — add a new cron job
pub async fn handle_api_cron_add(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<CronAddBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let CronAddBody {
        name,
        schedule,
        command,
        job_type,
        prompt,
        enabled,
        delivery,
        session_target,
        model,
        allowed_tools,
        delete_after_run,
    } = body;

    let config = state.config.lock().clone();
    let schedule = match parse_schedule_input(&schedule) {
        Ok(s) => s,
        Err(e) => {
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({"error": format!("Failed to add cron job: {e}")})),
            )
                .into_response();
        }
    };
    if let Err(e) = cron::validate_delivery_config(delivery.as_ref()) {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": format!("Failed to add cron job: {e}")})),
        )
            .into_response();
    }

    // Determine job type: explicit field, or infer "agent" when prompt is provided.
    let is_agent =
        matches!(job_type.as_deref(), Some("agent")) || (job_type.is_none() && prompt.is_some());

    let result = if is_agent {
        let prompt = match prompt.as_deref() {
            Some(p) if !p.trim().is_empty() => p,
            _ => {
                return (
                    StatusCode::BAD_REQUEST,
                    Json(serde_json::json!({"error": "Missing 'prompt' for agent job"})),
                )
                    .into_response();
            }
        };

        let session_target = session_target
            .as_deref()
            .map(cron::SessionTarget::parse)
            .unwrap_or_default();

        let default_delete = matches!(schedule, cron::Schedule::At { .. });
        let delete_after_run = delete_after_run.unwrap_or(default_delete);

        cron::add_agent_job(
            &config,
            name,
            schedule,
            prompt,
            session_target,
            model,
            delivery,
            delete_after_run,
            allowed_tools,
        )
    } else {
        let command = match command.as_deref() {
            Some(c) if !c.trim().is_empty() => c,
            _ => {
                return (
                    StatusCode::BAD_REQUEST,
                    Json(serde_json::json!({"error": "Missing 'command' for shell job"})),
                )
                    .into_response();
            }
        };

        cron::add_shell_job_with_approval(&config, name, schedule, command, delivery, false)
    };

    match result {
        Ok(job) => {
            if enabled == Some(false) {
                match cron::pause_job(&config, &job.id) {
                    Ok(paused) => {
                        Json(serde_json::json!({"status": "ok", "job": paused})).into_response()
                    }
                    Err(e) => (
                        StatusCode::INTERNAL_SERVER_ERROR,
                        Json(
                            serde_json::json!({"error": format!("Failed to set enabled state: {e}")}),
                        ),
                    )
                        .into_response(),
                }
            } else {
                Json(serde_json::json!({"status": "ok", "job": job})).into_response()
            }
        }
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to add cron job: {e}")})),
        )
            .into_response(),
    }
}

fn cron_run_to_json(r: &cron::CronRun) -> serde_json::Value {
    serde_json::json!({
        "id": r.id,
        "job_id": r.job_id,
        "job_name": r.job_name,
        "job_prompt": r.job_prompt,
        "job_type": r.job_type,
        "started_at": r.started_at.to_rfc3339(),
        "finished_at": r.finished_at.to_rfc3339(),
        "status": r.status,
        "output": r.output,
        "duration_ms": r.duration_ms,
    })
}

/// GET /api/cron/runs/recent — list recent runs across all jobs (incl. deleted one-shots)
pub async fn handle_api_cron_runs_recent(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<CronRecentRunsQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let limit = params.limit.unwrap_or(50).clamp(1, 200) as usize;
    let job_type = params.job_type.as_deref();
    let config = state.config.lock().clone();

    match cron::list_recent_runs(&config, limit, job_type) {
        Ok(runs) => {
            let runs_json: Vec<serde_json::Value> = runs.iter().map(cron_run_to_json).collect();
            Json(serde_json::json!({"runs": runs_json})).into_response()
        }
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to list recent cron runs: {e}")})),
        )
            .into_response(),
    }
}

/// GET /api/cron/:id/runs — list recent runs for a cron job
pub async fn handle_api_cron_runs(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
    Query(params): Query<CronRunsQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let limit = params.limit.unwrap_or(20).clamp(1, 100) as usize;
    let config = state.config.lock().clone();

    match cron::list_runs(&config, &id, limit) {
        Ok(runs) => {
            let runs_json: Vec<serde_json::Value> = runs.iter().map(cron_run_to_json).collect();
            Json(serde_json::json!({"runs": runs_json})).into_response()
        }
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to list cron runs: {e}")})),
        )
            .into_response(),
    }
}

/// POST /api/cron/:id/run — trigger a cron job manually
pub async fn handle_api_cron_run(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();

    let job = match kylinbot_runtime::cron::get_job(&config, &id) {
        Ok(job) => job,
        Err(e) => {
            return (
                StatusCode::NOT_FOUND,
                Json(serde_json::json!({"error": format!("Cron job not found: {e}")})),
            )
                .into_response();
        }
    };

    let started_at = chrono::Utc::now();
    let (mut success, output) =
        kylinbot_runtime::cron::scheduler::execute_job_now(&config, &job).await;
    let finished_at = chrono::Utc::now();
    let duration_ms = (finished_at - started_at).num_milliseconds();

    if job.delivery.mode.eq_ignore_ascii_case("announce")
        && let (Some(channel), Some(target)) =
            (job.delivery.channel.as_deref(), job.delivery.to.as_deref())
        && let Err(e) = kylinbot_runtime::cron::scheduler::deliver_announcement(
            &config, channel, target, &output,
        )
        .await
    {
        if job.delivery.best_effort {
            tracing::warn!(
                job_id = %job.id,
                error = %e,
                "manual cron trigger delivery failed (best_effort)"
            );
        } else {
            tracing::warn!(
                job_id = %job.id,
                error = %e,
                "manual cron trigger delivery failed"
            );
            success = false;
        }
    }

    let status = if success { "ok" } else { "error" };
    if let Err(e) = kylinbot_runtime::cron::record_run(
        &config,
        &job.id,
        started_at,
        finished_at,
        status,
        Some(&output),
        duration_ms,
        Some(&job),
    ) {
        tracing::warn!(
            job_id = %job.id,
            error = %e,
            "manual cron trigger: failed to persist run history",
        );
    }
    if let Err(e) =
        kylinbot_runtime::cron::record_last_run(&config, &job.id, finished_at, success, &output)
    {
        tracing::warn!(
            job_id = %job.id,
            error = %e,
            "manual cron trigger: failed to update last_run state",
        );
    }

    // Broadcast the result so dashboard/SSE clients refresh in real time,
    // matching the scheduler's automatic-execution behavior.
    let _ = state.event_tx.send(serde_json::json!({
        "type": "cron_result",
        "job_id": job.id,
        "success": success,
        "output": output,
        "manual": true,
        "timestamp": finished_at.to_rfc3339(),
    }));

    Json(serde_json::json!({
        "status": status,
        "job_id": job.id,
        "success": success,
        "output": output,
        "duration_ms": duration_ms,
        "started_at": started_at.to_rfc3339(),
        "finished_at": finished_at.to_rfc3339(),
    }))
    .into_response()
}

/// PATCH /api/cron/:id — update an existing cron job
pub async fn handle_api_cron_patch(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
    Json(body): Json<CronPatchBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();

    // Build the schedule from the provided value (string or structured object).
    let schedule = match body.schedule.as_ref() {
        Some(value) => match parse_schedule_input(value) {
            Ok(s) => Some(s),
            Err(e) => {
                return (
                    StatusCode::BAD_REQUEST,
                    Json(serde_json::json!({"error": format!("Failed to update cron job: {e}")})),
                )
                    .into_response();
            }
        },
        None => None,
    };

    // Route the edited text to the correct field based on the job's stored type.
    // The frontend sends a single textarea value; for agent jobs it is the prompt,
    // for shell jobs it is the command.
    let existing = match cron::get_job(&config, &id) {
        Ok(j) => j,
        Err(e) => {
            return (
                StatusCode::NOT_FOUND,
                Json(serde_json::json!({"error": format!("Cron job not found: {e}")})),
            )
                .into_response();
        }
    };
    let is_agent = matches!(existing.job_type, cron::JobType::Agent);
    let (patch_command, patch_prompt) = if is_agent {
        (None, body.command.or(body.prompt))
    } else {
        (body.command.or(body.prompt), None)
    };

    let patch = cron::CronJobPatch {
        name: body.name,
        schedule,
        command: patch_command,
        prompt: patch_prompt,
        enabled: body.enabled,
        delivery: body.delivery,
        model: body.model,
        session_target: body
            .session_target
            .as_deref()
            .map(cron::SessionTarget::parse),
        delete_after_run: body.delete_after_run,
        allowed_tools: body.allowed_tools,
        uses_memory: body.uses_memory,
    };

    match cron::update_shell_job_with_approval(&config, &id, patch, false) {
        Ok(job) => Json(serde_json::json!({"status": "ok", "job": job})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to update cron job: {e}")})),
        )
            .into_response(),
    }
}

/// POST /api/cron/:id/run — force-run a cron job immediately
pub async fn handle_api_cron_run_now(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
    Json(body): Json<CronRunNowBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let job = match cron::get_job(&config, &id) {
        Ok(job) => job,
        Err(e) => {
            return (
                StatusCode::NOT_FOUND,
                Json(serde_json::json!({"error": format!("Cron job not found: {e}")})),
            )
                .into_response();
        }
    };

    if matches!(job.job_type, cron::JobType::Shell) {
        let approved = body.approved.unwrap_or(false);
        if let Err(e) = cron::validate_shell_command(&config, &job.command, approved) {
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({"error": format!("Failed to run cron job: {e}")})),
            )
                .into_response();
        }
    }

    let started_at = chrono::Utc::now();
    let (success, output) = cron::scheduler::execute_job_now(&config, &job).await;
    let finished_at = chrono::Utc::now();
    let duration_ms = (finished_at - started_at).num_milliseconds();
    let status = if success { "ok" } else { "error" };

    let _ = cron::record_run(
        &config,
        &job.id,
        started_at,
        finished_at,
        status,
        Some(&output),
        duration_ms,
        Some(&job),
    );
    let _ = cron::record_last_run(&config, &job.id, finished_at, success, &output);

    Json(serde_json::json!({
        "status": status,
        "ok": success,
        "job_id": job.id,
        "duration_ms": duration_ms,
        "output": output
    }))
    .into_response()
}

/// POST /api/cron/:id/pause — disable a cron job
pub async fn handle_api_cron_pause(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    match cron::pause_job(&config, &id) {
        Ok(job) => Json(serde_json::json!({"status": "ok", "job": job})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to pause cron job: {e}")})),
        )
            .into_response(),
    }
}

/// POST /api/cron/:id/resume — enable a cron job
pub async fn handle_api_cron_resume(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    match cron::resume_job(&config, &id) {
        Ok(job) => Json(serde_json::json!({"status": "ok", "job": job})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to resume cron job: {e}")})),
        )
            .into_response(),
    }
}

/// DELETE /api/cron/:id — remove a cron job
pub async fn handle_api_cron_delete(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    match cron::remove_job(&config, &id) {
        Ok(()) => Json(serde_json::json!({"status": "ok"})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to remove cron job: {e}")})),
        )
            .into_response(),
    }
}

/// GET /api/cron/settings — return cron subsystem settings
pub async fn handle_api_cron_settings_get(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    Json(serde_json::json!({
        "enabled": config.cron.enabled,
        "catch_up_on_startup": config.cron.catch_up_on_startup,
        "max_run_history": config.cron.max_run_history,
    }))
    .into_response()
}

/// PATCH /api/cron/settings — update cron subsystem settings
pub async fn handle_api_cron_settings_patch(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<serde_json::Value>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let mut config = state.config.lock().clone();

    if let Some(v) = body.get("enabled").and_then(|v| v.as_bool()) {
        config.cron.enabled = v;
    }
    if let Some(v) = body.get("catch_up_on_startup").and_then(|v| v.as_bool()) {
        config.cron.catch_up_on_startup = v;
    }
    if let Some(v) = body.get("max_run_history").and_then(|v| v.as_u64()) {
        config.cron.max_run_history = u32::try_from(v).unwrap_or(u32::MAX);
    }

    if let Err(e) = config.save().await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to save config: {e}")})),
        )
            .into_response();
    }

    *state.config.lock() = config.clone();

    Json(serde_json::json!({
        "status": "ok",
        "enabled": config.cron.enabled,
        "catch_up_on_startup": config.cron.catch_up_on_startup,
        "max_run_history": config.cron.max_run_history,
    }))
    .into_response()
}

/// GET /api/integrations — list all integrations with status
pub async fn handle_api_integrations(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let entries = integrations::registry::all_integrations(&config);

    let integrations: Vec<serde_json::Value> = entries
        .iter()
        .map(|entry| {
            serde_json::json!({
                "name": entry.name,
                "description": entry.description,
                "category": entry.category,
                "status": entry.status,
            })
        })
        .collect();

    Json(serde_json::json!({"integrations": integrations})).into_response()
}

/// GET /api/integrations/settings — return per-integration settings (enabled + category)
pub async fn handle_api_integrations_settings(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let entries = integrations::registry::all_integrations(&config);

    let mut settings = serde_json::Map::new();
    for entry in &entries {
        let enabled = matches!(entry.status, IntegrationStatus::Active);
        settings.insert(
            entry.name.to_string(),
            serde_json::json!({
                "enabled": enabled,
                "category": entry.category,
                "status": entry.status,
            }),
        );
    }

    Json(serde_json::json!({"settings": settings})).into_response()
}

/// POST /api/doctor — run diagnostics
pub async fn handle_api_doctor(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    let results = doctor::diagnose(&config);

    let ok_count = results
        .iter()
        .filter(|r| r.severity == doctor::Severity::Ok)
        .count();
    let warn_count = results
        .iter()
        .filter(|r| r.severity == doctor::Severity::Warn)
        .count();
    let error_count = results
        .iter()
        .filter(|r| r.severity == doctor::Severity::Error)
        .count();

    Json(serde_json::json!({
        "results": results,
        "summary": {
            "ok": ok_count,
            "warnings": warn_count,
            "errors": error_count,
        }
    }))
    .into_response()
}

/// GET /api/memory — list or search memory entries
pub async fn handle_api_memory_list(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<MemoryQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    // Use recall when query or time range is provided
    if params.query.is_some() || params.since.is_some() || params.until.is_some() {
        let query = params.query.as_deref().unwrap_or("");
        let since = params.since.as_deref();
        let until = params.until.as_deref();
        match state.mem.recall(query, 50, None, since, until).await {
            Ok(entries) => Json(serde_json::json!({"entries": entries})).into_response(),
            Err(e) => (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Memory recall failed: {e}")})),
            )
                .into_response(),
        }
    } else {
        // List mode
        let category = params.category.as_deref().map(|cat| match cat {
            "core" => MemoryCategory::Core,
            "daily" => MemoryCategory::Daily,
            "conversation" => MemoryCategory::Conversation,
            other => MemoryCategory::Custom(other.to_string()),
        });

        match state.mem.list(category.as_ref(), None).await {
            Ok(entries) => Json(serde_json::json!({"entries": entries})).into_response(),
            Err(e) => (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Memory list failed: {e}")})),
            )
                .into_response(),
        }
    }
}

/// POST /api/memory — store a memory entry
pub async fn handle_api_memory_store(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<MemoryStoreBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let category = body
        .category
        .as_deref()
        .map(|cat| match cat {
            "core" => MemoryCategory::Core,
            "daily" => MemoryCategory::Daily,
            "conversation" => MemoryCategory::Conversation,
            other => MemoryCategory::Custom(other.to_string()),
        })
        .unwrap_or(MemoryCategory::Core);

    match state
        .mem
        .store(&body.key, &body.content, category, None)
        .await
    {
        Ok(()) => Json(serde_json::json!({"status": "ok"})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Memory store failed: {e}")})),
        )
            .into_response(),
    }
}

/// DELETE /api/memory/:key — delete a memory entry
pub async fn handle_api_memory_delete(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(key): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    match state.mem.forget(&key).await {
        Ok(deleted) => {
            Json(serde_json::json!({"status": "ok", "deleted": deleted})).into_response()
        }
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Memory forget failed: {e}")})),
        )
            .into_response(),
    }
}

/// GET /api/cost — cost summary
pub async fn handle_api_cost(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    if let Some(ref tracker) = state.cost_tracker {
        match tracker.get_summary() {
            Ok(summary) => Json(serde_json::json!({"cost": summary})).into_response(),
            Err(e) => (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Cost summary failed: {e}")})),
            )
                .into_response(),
        }
    } else {
        Json(serde_json::json!({
            "cost": {
                "session_cost_usd": 0.0,
                "daily_cost_usd": 0.0,
                "monthly_cost_usd": 0.0,
                "total_tokens": 0,
                "request_count": 0,
                "by_model": {},
            }
        }))
        .into_response()
    }
}

/// POST /api/cost/purge — remove cost records for model names no longer configured
pub async fn handle_api_cost_purge(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<CostPurgeBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let models: Vec<String> = body
        .models
        .into_iter()
        .map(|model| model.trim().to_string())
        .filter(|model| !model.is_empty())
        .collect();

    if models.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "models must be a non-empty array of model names"})),
        )
            .into_response();
    }

    if let Some(ref tracker) = state.cost_tracker {
        match tracker.purge_models(&models) {
            Ok(removed_records) => Json(serde_json::json!({
                "status": "ok",
                "removed_records": removed_records,
            }))
            .into_response(),
            Err(e) => (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Cost purge failed: {e}")})),
            )
                .into_response(),
        }
    } else {
        Json(serde_json::json!({
            "status": "ok",
            "removed_records": 0,
        }))
        .into_response()
    }
}

/// GET /api/cli-tools — discovered CLI tools
pub async fn handle_api_cli_tools(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let tools = cli_discovery::discover_cli_tools(&[], &[]);

    Json(serde_json::json!({"cli_tools": tools})).into_response()
}

/// GET /api/health — component health snapshot
pub async fn handle_api_health(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let snapshot = health::snapshot();
    Json(serde_json::json!({"health": snapshot})).into_response()
}

// ── Helpers ─────────────────────────────────────────────────────

const MASKED_SECRET: &str = "****";

fn is_masked_secret(value: &str) -> bool {
    value == MASKED_SECRET
}

fn mask_optional_secret(value: &mut Option<String>) {
    if value.is_some() {
        *value = Some(MASKED_SECRET.to_string());
    }
}

fn mask_required_secret(value: &mut String) {
    if !value.is_empty() {
        *value = MASKED_SECRET.to_string();
    }
}

fn mask_vec_secrets(values: &mut [String]) {
    for value in values.iter_mut() {
        if !value.is_empty() {
            *value = MASKED_SECRET.to_string();
        }
    }
}

#[allow(clippy::ref_option)]
fn restore_optional_secret(value: &mut Option<String>, current: &Option<String>) {
    if value.as_deref().is_some_and(is_masked_secret) {
        *value = current.clone();
    }
}

fn restore_required_secret(value: &mut String, current: &str) {
    if is_masked_secret(value) {
        *value = current.to_string();
    }
}

fn restore_vec_secrets(values: &mut [String], current: &[String]) {
    for (idx, value) in values.iter_mut().enumerate() {
        if is_masked_secret(value)
            && let Some(existing) = current.get(idx)
        {
            *value = existing.clone();
        }
    }
}

fn normalize_route_field(value: &str) -> String {
    value.trim().to_ascii_lowercase()
}

fn model_route_identity_matches(
    incoming: &schema::ModelRouteConfig,
    current: &schema::ModelRouteConfig,
) -> bool {
    normalize_route_field(&incoming.hint) == normalize_route_field(&current.hint)
        && normalize_route_field(&incoming.provider) == normalize_route_field(&current.provider)
        && normalize_route_field(&incoming.model) == normalize_route_field(&current.model)
}

fn model_route_provider_model_matches(
    incoming: &schema::ModelRouteConfig,
    current: &schema::ModelRouteConfig,
) -> bool {
    normalize_route_field(&incoming.provider) == normalize_route_field(&current.provider)
        && normalize_route_field(&incoming.model) == normalize_route_field(&current.model)
}

fn embedding_route_identity_matches(
    incoming: &schema::EmbeddingRouteConfig,
    current: &schema::EmbeddingRouteConfig,
) -> bool {
    normalize_route_field(&incoming.hint) == normalize_route_field(&current.hint)
        && normalize_route_field(&incoming.provider) == normalize_route_field(&current.provider)
        && normalize_route_field(&incoming.model) == normalize_route_field(&current.model)
}

fn embedding_route_provider_model_matches(
    incoming: &schema::EmbeddingRouteConfig,
    current: &schema::EmbeddingRouteConfig,
) -> bool {
    normalize_route_field(&incoming.provider) == normalize_route_field(&current.provider)
        && normalize_route_field(&incoming.model) == normalize_route_field(&current.model)
}

fn restore_model_route_api_keys(
    incoming: &mut [schema::ModelRouteConfig],
    current: &[schema::ModelRouteConfig],
) {
    let mut used_current = vec![false; current.len()];
    for incoming_route in incoming {
        if !incoming_route
            .api_key
            .as_deref()
            .is_some_and(is_masked_secret)
        {
            continue;
        }

        let exact_match_idx = current
            .iter()
            .enumerate()
            .find(|(idx, current_route)| {
                !used_current[*idx] && model_route_identity_matches(incoming_route, current_route)
            })
            .map(|(idx, _)| idx);

        let match_idx = exact_match_idx.or_else(|| {
            current
                .iter()
                .enumerate()
                .find(|(idx, current_route)| {
                    !used_current[*idx]
                        && model_route_provider_model_matches(incoming_route, current_route)
                })
                .map(|(idx, _)| idx)
        });

        if let Some(idx) = match_idx {
            used_current[idx] = true;
            incoming_route.api_key = current[idx].api_key.clone();
        } else {
            // Never persist UI placeholders to disk when no safe restore target exists.
            incoming_route.api_key = None;
        }
    }
}

fn restore_embedding_route_api_keys(
    incoming: &mut [schema::EmbeddingRouteConfig],
    current: &[schema::EmbeddingRouteConfig],
) {
    let mut used_current = vec![false; current.len()];
    for incoming_route in incoming {
        if !incoming_route
            .api_key
            .as_deref()
            .is_some_and(is_masked_secret)
        {
            continue;
        }

        let exact_match_idx = current
            .iter()
            .enumerate()
            .find(|(idx, current_route)| {
                !used_current[*idx]
                    && embedding_route_identity_matches(incoming_route, current_route)
            })
            .map(|(idx, _)| idx);

        let match_idx = exact_match_idx.or_else(|| {
            current
                .iter()
                .enumerate()
                .find(|(idx, current_route)| {
                    !used_current[*idx]
                        && embedding_route_provider_model_matches(incoming_route, current_route)
                })
                .map(|(idx, _)| idx)
        });

        if let Some(idx) = match_idx {
            used_current[idx] = true;
            incoming_route.api_key = current[idx].api_key.clone();
        } else {
            // Never persist UI placeholders to disk when no safe restore target exists.
            incoming_route.api_key = None;
        }
    }
}

fn mask_sensitive_fields(config: &Config) -> Config {
    let mut masked = config.clone();

    mask_vec_secrets(&mut masked.reliability.api_keys);
    mask_vec_secrets(&mut masked.gateway.paired_tokens);
    mask_optional_secret(&mut masked.composio.api_key);
    mask_optional_secret(&mut masked.browser.computer_use.api_key);
    mask_optional_secret(&mut masked.web_search.brave_api_key);
    mask_optional_secret(&mut masked.storage.provider.config.db_url);
    mask_optional_secret(&mut masked.memory.qdrant.api_key);
    if let Some(cloudflare) = masked.tunnel.cloudflare.as_mut() {
        mask_required_secret(&mut cloudflare.token);
    }
    if let Some(ngrok) = masked.tunnel.ngrok.as_mut() {
        mask_required_secret(&mut ngrok.auth_token);
    }

    for agent in masked.agents.values_mut() {
        mask_optional_secret(&mut agent.api_key);
    }
    for provider in masked.providers.models.values_mut() {
        mask_optional_secret(&mut provider.api_key);
    }
    for route in &mut masked.providers.model_routes {
        mask_optional_secret(&mut route.api_key);
    }
    for route in &mut masked.providers.embedding_routes {
        mask_optional_secret(&mut route.api_key);
    }

    if let Some(telegram) = masked.channels.telegram.as_mut() {
        mask_required_secret(&mut telegram.bot_token);
    }
    if let Some(discord) = masked.channels.discord.as_mut() {
        mask_required_secret(&mut discord.bot_token);
    }
    if let Some(slack) = masked.channels.slack.as_mut() {
        mask_required_secret(&mut slack.bot_token);
        mask_optional_secret(&mut slack.app_token);
    }
    if let Some(mattermost) = masked.channels.mattermost.as_mut() {
        mask_required_secret(&mut mattermost.bot_token);
    }
    if let Some(webhook) = masked.channels.webhook.as_mut() {
        mask_optional_secret(&mut webhook.secret);
    }
    if let Some(matrix) = masked.channels.matrix.as_mut() {
        mask_required_secret(&mut matrix.access_token);
    }
    if let Some(whatsapp) = masked.channels.whatsapp.as_mut() {
        mask_optional_secret(&mut whatsapp.access_token);
        mask_optional_secret(&mut whatsapp.app_secret);
        mask_optional_secret(&mut whatsapp.verify_token);
    }
    if let Some(linq) = masked.channels.linq.as_mut() {
        mask_required_secret(&mut linq.api_token);
        mask_optional_secret(&mut linq.signing_secret);
    }
    if let Some(nextcloud) = masked.channels.nextcloud_talk.as_mut() {
        mask_required_secret(&mut nextcloud.app_token);
        mask_optional_secret(&mut nextcloud.webhook_secret);
    }
    if let Some(wati) = masked.channels.wati.as_mut() {
        mask_required_secret(&mut wati.api_token);
    }
    if let Some(irc) = masked.channels.irc.as_mut() {
        mask_optional_secret(&mut irc.server_password);
        mask_optional_secret(&mut irc.nickserv_password);
        mask_optional_secret(&mut irc.sasl_password);
    }
    if let Some(lark) = masked.channels.lark.as_mut() {
        mask_required_secret(&mut lark.app_secret);
        mask_optional_secret(&mut lark.encrypt_key);
        mask_optional_secret(&mut lark.verification_token);
    }
    if let Some(feishu) = masked.channels.feishu.as_mut() {
        mask_required_secret(&mut feishu.app_secret);
        mask_optional_secret(&mut feishu.encrypt_key);
        mask_optional_secret(&mut feishu.verification_token);
    }
    if let Some(lansenger) = masked.channels.lansenger.as_mut() {
        mask_required_secret(&mut lansenger.app_secret);
    }
    if let Some(dingtalk) = masked.channels.dingtalk.as_mut() {
        mask_required_secret(&mut dingtalk.client_secret);
    }
    if let Some(qq) = masked.channels.qq.as_mut() {
        mask_required_secret(&mut qq.app_secret);
    }
    #[cfg(feature = "channel-nostr")]
    if let Some(nostr) = masked.channels.nostr.as_mut() {
        mask_required_secret(&mut nostr.private_key);
    }
    if let Some(clawdtalk) = masked.channels.clawdtalk.as_mut() {
        mask_required_secret(&mut clawdtalk.api_key);
        mask_optional_secret(&mut clawdtalk.webhook_secret);
    }
    if let Some(email) = masked.channels.email.as_mut() {
        mask_required_secret(&mut email.password);
    }
    mask_optional_secret(&mut masked.transcription.api_key);
    masked
}

fn restore_masked_sensitive_fields(incoming: &mut Config, current: &Config) {
    restore_vec_secrets(
        &mut incoming.gateway.paired_tokens,
        &current.gateway.paired_tokens,
    );
    restore_vec_secrets(
        &mut incoming.reliability.api_keys,
        &current.reliability.api_keys,
    );
    restore_optional_secret(&mut incoming.composio.api_key, &current.composio.api_key);
    restore_optional_secret(
        &mut incoming.browser.computer_use.api_key,
        &current.browser.computer_use.api_key,
    );
    restore_optional_secret(
        &mut incoming.web_search.brave_api_key,
        &current.web_search.brave_api_key,
    );
    restore_optional_secret(
        &mut incoming.storage.provider.config.db_url,
        &current.storage.provider.config.db_url,
    );
    restore_optional_secret(
        &mut incoming.memory.qdrant.api_key,
        &current.memory.qdrant.api_key,
    );
    if let (Some(incoming_tunnel), Some(current_tunnel)) = (
        incoming.tunnel.cloudflare.as_mut(),
        current.tunnel.cloudflare.as_ref(),
    ) {
        restore_required_secret(&mut incoming_tunnel.token, &current_tunnel.token);
    }
    if let (Some(incoming_tunnel), Some(current_tunnel)) = (
        incoming.tunnel.ngrok.as_mut(),
        current.tunnel.ngrok.as_ref(),
    ) {
        restore_required_secret(&mut incoming_tunnel.auth_token, &current_tunnel.auth_token);
    }

    for (name, agent) in &mut incoming.agents {
        if let Some(current_agent) = current.agents.get(name) {
            restore_optional_secret(&mut agent.api_key, &current_agent.api_key);
        }
    }
    for (name, provider) in &mut incoming.providers.models {
        if let Some(current_provider) = current.providers.models.get(name) {
            restore_optional_secret(&mut provider.api_key, &current_provider.api_key);
        }
    }
    restore_model_route_api_keys(
        &mut incoming.providers.model_routes,
        &current.providers.model_routes,
    );
    restore_embedding_route_api_keys(
        &mut incoming.providers.embedding_routes,
        &current.providers.embedding_routes,
    );

    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.telegram.as_mut(),
        current.channels.telegram.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.bot_token, &current_ch.bot_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.discord.as_mut(),
        current.channels.discord.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.bot_token, &current_ch.bot_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.slack.as_mut(),
        current.channels.slack.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.bot_token, &current_ch.bot_token);
        restore_optional_secret(&mut incoming_ch.app_token, &current_ch.app_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.mattermost.as_mut(),
        current.channels.mattermost.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.bot_token, &current_ch.bot_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.webhook.as_mut(),
        current.channels.webhook.as_ref(),
    ) {
        restore_optional_secret(&mut incoming_ch.secret, &current_ch.secret);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.matrix.as_mut(),
        current.channels.matrix.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.access_token, &current_ch.access_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.whatsapp.as_mut(),
        current.channels.whatsapp.as_ref(),
    ) {
        restore_optional_secret(&mut incoming_ch.access_token, &current_ch.access_token);
        restore_optional_secret(&mut incoming_ch.app_secret, &current_ch.app_secret);
        restore_optional_secret(&mut incoming_ch.verify_token, &current_ch.verify_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.linq.as_mut(),
        current.channels.linq.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.api_token, &current_ch.api_token);
        restore_optional_secret(&mut incoming_ch.signing_secret, &current_ch.signing_secret);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.nextcloud_talk.as_mut(),
        current.channels.nextcloud_talk.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.app_token, &current_ch.app_token);
        restore_optional_secret(&mut incoming_ch.webhook_secret, &current_ch.webhook_secret);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.wati.as_mut(),
        current.channels.wati.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.api_token, &current_ch.api_token);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.irc.as_mut(),
        current.channels.irc.as_ref(),
    ) {
        restore_optional_secret(
            &mut incoming_ch.server_password,
            &current_ch.server_password,
        );
        restore_optional_secret(
            &mut incoming_ch.nickserv_password,
            &current_ch.nickserv_password,
        );
        restore_optional_secret(&mut incoming_ch.sasl_password, &current_ch.sasl_password);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.lark.as_mut(),
        current.channels.lark.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.app_secret, &current_ch.app_secret);
        restore_optional_secret(&mut incoming_ch.encrypt_key, &current_ch.encrypt_key);
        restore_optional_secret(
            &mut incoming_ch.verification_token,
            &current_ch.verification_token,
        );
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.feishu.as_mut(),
        current.channels.feishu.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.app_secret, &current_ch.app_secret);
        restore_optional_secret(&mut incoming_ch.encrypt_key, &current_ch.encrypt_key);
        restore_optional_secret(
            &mut incoming_ch.verification_token,
            &current_ch.verification_token,
        );
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.lansenger.as_mut(),
        current.channels.lansenger.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.app_secret, &current_ch.app_secret);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.dingtalk.as_mut(),
        current.channels.dingtalk.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.client_secret, &current_ch.client_secret);
    }
    if let (Some(incoming_ch), Some(current_ch)) =
        (incoming.channels.qq.as_mut(), current.channels.qq.as_ref())
    {
        restore_required_secret(&mut incoming_ch.app_secret, &current_ch.app_secret);
    }
    #[cfg(feature = "channel-nostr")]
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.nostr.as_mut(),
        current.channels.nostr.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.private_key, &current_ch.private_key);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.clawdtalk.as_mut(),
        current.channels.clawdtalk.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.api_key, &current_ch.api_key);
        restore_optional_secret(&mut incoming_ch.webhook_secret, &current_ch.webhook_secret);
    }
    if let (Some(incoming_ch), Some(current_ch)) = (
        incoming.channels.email.as_mut(),
        current.channels.email.as_ref(),
    ) {
        restore_required_secret(&mut incoming_ch.password, &current_ch.password);
    }
    restore_optional_secret(
        &mut incoming.transcription.api_key,
        &current.transcription.api_key,
    );
}

fn hydrate_config_for_save(mut incoming: Config, current: &Config) -> Config {
    restore_masked_sensitive_fields(&mut incoming, current);
    // These are runtime-computed fields skipped from TOML serialization.
    incoming.config_path = current.config_path.clone();
    incoming.workspace_dir = current.workspace_dir.clone();
    incoming
}

// ── Session API handlers ─────────────────────────────────────────

/// GET /api/sessions — list gateway sessions
pub async fn handle_api_sessions_list(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let Some(ref backend) = state.session_backend else {
        return Json(serde_json::json!({
            "sessions": [],
            "message": "Session persistence is disabled"
        }))
        .into_response();
    };

    let all_metadata = backend.list_sessions_with_metadata();
    let gw_sessions: Vec<serde_json::Value> = all_metadata
        .into_iter()
        .filter_map(|meta| {
            let session_id = meta.key.strip_prefix("gw_")?;
            let mut entry = serde_json::json!({
                "session_id": session_id,
                "created_at": meta.created_at.to_rfc3339(),
                "last_activity": meta.last_activity.to_rfc3339(),
                "message_count": meta.message_count,
            });
            if let Some(name) = meta.name {
                entry["name"] = serde_json::Value::String(name);
            }
            Some(entry)
        })
        .collect();

    Json(serde_json::json!({ "sessions": gw_sessions })).into_response()
}

/// GET /api/sessions/{id}/messages — load persisted gateway WebSocket chat transcript
pub async fn handle_api_session_messages(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let Some(ref backend) = state.session_backend else {
        return Json(serde_json::json!({
            "session_id": id,
            "messages": [],
            "session_persistence": false,
        }))
        .into_response();
    };

    let session_key = format!("gw_{id}");
    let msgs = backend.load(&session_key);
    let messages: Vec<serde_json::Value> = msgs
        .into_iter()
        .map(|m| serde_json::json!({ "role": m.role, "content": m.content }))
        .collect();

    Json(serde_json::json!({
        "session_id": id,
        "messages": messages,
        "session_persistence": true,
    }))
    .into_response()
}

/// DELETE /api/sessions/{id} — delete a gateway session
pub async fn handle_api_session_delete(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let Some(ref backend) = state.session_backend else {
        return (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({"error": "Session persistence is disabled"})),
        )
            .into_response();
    };

    let session_key = format!("gw_{id}");

    // If a turn is in flight for this session, cancel it and evict the entry
    // from `cancel_tokens` here rather than leaving the WebSocket handler's
    // post-`tokio::join!` cleanup (`ws.rs:535`) as the only path. Without
    // this, deleting a session mid-turn leaks the map entry until the
    // streaming task happens to wake up — and on a process crash the
    // entry is lost entirely (#5835).
    let token = state
        .cancel_tokens
        .lock()
        .expect("cancel_tokens lock poisoned")
        .remove(&session_key);
    if let Some(token) = token {
        token.cancel();
        tracing::info!(session_key, "cancelled in-flight turn for deleted session");
    }

    match backend.delete_session(&session_key) {
        Ok(true) => Json(serde_json::json!({"deleted": true, "session_id": id})).into_response(),
        Ok(false) => (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({"error": "Session not found"})),
        )
            .into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to delete session: {e}")})),
        )
            .into_response(),
    }
}

/// PUT /api/sessions/{id} — rename a gateway session
pub async fn handle_api_session_rename(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
    Json(body): Json<serde_json::Value>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let Some(ref backend) = state.session_backend else {
        return (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({"error": "Session persistence is disabled"})),
        )
            .into_response();
    };

    let name = body["name"].as_str().unwrap_or("").trim();
    if name.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "name is required"})),
        )
            .into_response();
    }

    let session_key = format!("gw_{id}");

    // Verify the session exists before renaming
    let sessions = backend.list_sessions();
    if !sessions.contains(&session_key) {
        return (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({"error": "Session not found"})),
        )
            .into_response();
    }

    match backend.set_session_name(&session_key, name) {
        Ok(()) => Json(serde_json::json!({"session_id": id, "name": name})).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to rename session: {e}")})),
        )
            .into_response(),
    }
}

/// GET /api/sessions/running — list sessions currently in "running" state
pub async fn handle_api_sessions_running(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let Some(ref backend) = state.session_backend else {
        return Json(serde_json::json!({
            "sessions": [],
            "message": "Session persistence is disabled"
        }))
        .into_response();
    };

    let running = backend.list_running_sessions();
    let sessions: Vec<serde_json::Value> = running
        .into_iter()
        .filter_map(|meta| {
            let session_id = meta.key.strip_prefix("gw_")?;
            Some(serde_json::json!({
                "session_id": session_id,
                "created_at": meta.created_at.to_rfc3339(),
                "last_activity": meta.last_activity.to_rfc3339(),
                "message_count": meta.message_count,
            }))
        })
        .collect();

    Json(serde_json::json!({ "sessions": sessions })).into_response()
}

/// GET /api/sessions/{id}/state — get session state
pub async fn handle_api_session_state(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let Some(ref backend) = state.session_backend else {
        return (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({"error": "Session persistence is disabled"})),
        )
            .into_response();
    };

    let session_key = format!("gw_{id}");
    match backend.get_session_state(&session_key) {
        Ok(Some(ss)) => {
            let mut resp = serde_json::json!({
                "session_id": id,
                "state": ss.state,
            });
            if let Some(turn_id) = ss.turn_id {
                resp["turn_id"] = serde_json::Value::String(turn_id);
            }
            if let Some(started) = ss.turn_started_at {
                resp["turn_started_at"] = serde_json::Value::String(started.to_rfc3339());
            }
            Json(resp).into_response()
        }
        Ok(None) => (
            StatusCode::NOT_FOUND,
            Json(serde_json::json!({"error": "Session not found"})),
        )
            .into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": format!("Failed to get session state: {e}")})),
        )
            .into_response(),
    }
}

// ── Session abort endpoint ────────────────────────────────────────

/// POST /api/sessions/{id}/abort — cancel an in-flight agent response.
///
/// Looks up the cancellation token for the given session. If a turn is
/// currently running the token is cancelled, which causes the agent's
/// streaming loop and tool-call loop to exit early. The WebSocket handler
/// is responsible for cleaning up partial state and sending the abort
/// frame to the client.
///
/// Returns 200 with `{"status": "aborted"}` if a running turn was found,
/// or `{"status": "no_active_response"}` if the session was idle (no
/// token present). Both are success — abort is idempotent.
pub async fn handle_api_session_abort(
    State(state): State<AppState>,
    headers: HeaderMap,
    Path(id): Path<String>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let session_key = format!("gw_{id}");

    // Look up and cancel the token. Hold the lock only long enough to
    // clone the token — cancellation itself does not need the lock.
    let token = state
        .cancel_tokens
        .lock()
        .expect("cancel_tokens lock poisoned")
        .get(&session_key)
        .cloned();

    if let Some(token) = token {
        token.cancel();
        tracing::info!(session_key, "session abort requested");
        Json(serde_json::json!({ "status": "aborted" })).into_response()
    } else {
        Json(serde_json::json!({ "status": "no_active_response" })).into_response()
    }
}

// ── Claude Code hook endpoint ────────────────────────────────────

/// POST /hooks/claude-code — receives HTTP hook events from Claude Code
/// sessions spawned by `ClaudeCodeRunnerTool`.
///
/// Claude Code posts structured JSON describing tool executions, completions,
/// and errors. This handler logs the event and (when a Slack channel is
/// configured) could be wired to update a Slack message in-place.
pub async fn handle_claude_code_hook(
    State(state): State<AppState>,
    Json(payload): Json<claude_code_runner::ClaudeCodeHookEvent>,
) -> impl IntoResponse {
    // Do not require bearer-token auth: Claude Code subprocesses cannot easily
    // obtain a pairing token, and the hook carries a session_id that ties it
    // back to a session we spawned.
    let _ = &state; // retained for future Slack update wiring

    tracing::info!(
        session_id = %payload.session_id,
        event_type = %payload.event_type,
        tool_name = ?payload.tool_name,
        summary = ?payload.summary,
        "Claude Code hook event received"
    );

    Json(serde_json::json!({ "ok": true }))
}

// ── WeChat iLink Bot QR binding ────────────────────────────────

const WECHAT_API_BASE: &str = "https://ilinkai.weixin.qq.com";
const WECHAT_QR_TIMEOUT_SECS: u64 = 480;

static WECHAT_CONFIRMED_QRCODES: std::sync::OnceLock<
    std::sync::Mutex<std::collections::HashSet<String>>,
> = std::sync::OnceLock::new();

fn wechat_confirmed_qrcodes() -> &'static std::sync::Mutex<std::collections::HashSet<String>> {
    WECHAT_CONFIRMED_QRCODES.get_or_init(|| std::sync::Mutex::new(std::collections::HashSet::new()))
}

/// Execute HTTP GET via system curl. Used for iLink API because:
///   1. iLink CDN requires SNI routing; direct IP connections are rejected
///   2. reqwest defaults to IPv6 which is unreachable for this host
///   3. reqwest .resolve() override does not propagate to native-tls TCP
///   4. curl handles IPv4-fallback + SNI correctly out of the box
async fn curl_get(
    url: &str,
    extra_headers: &[(&str, &str)],
    timeout_secs: u64,
) -> anyhow::Result<serde_json::Value> {
    let mut args: Vec<String> = vec![
        "-4".into(),
        "-s".into(),
        "--connect-timeout".into(),
        "10".into(),
        "-m".into(),
        timeout_secs.to_string(),
    ];
    for (k, v) in extra_headers {
        args.push("-H".into());
        args.push(format!("{k}: {v}"));
    }
    args.push(url.to_string());
    tracing::debug!(url, "curl_get: spawning curl subprocess");
    let output = match tokio::process::Command::new("curl")
        .args(&args)
        .output()
        .await
    {
        Ok(o) => o,
        Err(e) => {
            tracing::error!(url, error = %e, "curl_get: failed to spawn curl");
            return Err(e.into());
        }
    };
    if !output.status.success() || output.stdout.is_empty() {
        let stderr = String::from_utf8_lossy(&output.stderr);
        anyhow::bail!(
            "curl failed (exit={}): {}",
            output.status.code().unwrap_or(-1),
            stderr
        );
    }
    Ok(serde_json::from_slice(&output.stdout)?)
}

pub async fn handle_wechat_qrcode() -> impl IntoResponse {
    let url = format!("{WECHAT_API_BASE}/ilink/bot/get_bot_qrcode?bot_type=3");
    let data = match curl_get(&url, &[], 15).await {
        Ok(d) => d,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("iLink API unreachable: {e}")})),
            )
                .into_response();
        }
    };

    let qrcode = data.get("qrcode").and_then(|v| v.as_str()).unwrap_or("");
    let qrcode_img_url = data
        .get("qrcode_img_content")
        .and_then(|v| v.as_str())
        .unwrap_or("");

    if qrcode.is_empty() {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": "No qrcode in iLink response"})),
        )
            .into_response();
    }

    Json(serde_json::json!({
        "qrcode": qrcode,
        "qrcode_img_url": qrcode_img_url,
        "poll_url": format!("{WECHAT_API_BASE}/ilink/bot/get_qrcode_status"),
        "expires_in": WECHAT_QR_TIMEOUT_SECS,
    }))
    .into_response()
}

/// Query params for QR status polling.
#[derive(Deserialize)]
pub struct WechatQrStatusQuery {
    pub qrcode: String,
}

/// GET /api/wechat/status — poll QR code scan status from iLink API.
///
/// Returns `{ status, bot_token?, ilink_bot_id?, ilink_user_id? }`.
/// No auth required (public endpoint for binding flow).
pub async fn handle_wechat_status(
    State(state): State<AppState>,
    Query(params): Query<WechatQrStatusQuery>,
) -> impl IntoResponse {
    if params.qrcode.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "Missing 'qrcode' query parameter"})),
        )
            .into_response();
    }

    let status_url = format!(
        "{WECHAT_API_BASE}/ilink/bot/get_qrcode_status?qrcode={}",
        urlencoding::encode(&params.qrcode)
    );

    let data = match curl_get(&status_url, &[("iLink-App-ClientVersion", "1")], 40).await {
        Ok(d) => d,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("iLink status check failed: {e}")})),
            )
                .into_response();
        }
    };

    let status_str = data
        .get("status")
        .and_then(|v| v.as_str())
        .unwrap_or("wait");

    let mut body = serde_json::json!({
        "status": status_str,
    });

    if status_str == "confirmed" {
        let already_confirmed = {
            let mut set = wechat_confirmed_qrcodes()
                .lock()
                .unwrap_or_else(|e| e.into_inner());
            !set.insert(params.qrcode.clone())
        };

        let token = data.get("bot_token").and_then(|v| v.as_str());
        let bot_id = data.get("ilink_bot_id").and_then(|v| v.as_str());
        let user_id = data.get("ilink_user_id").and_then(|v| v.as_str());

        if let (Some(t), Some(b)) = (token, bot_id) {
            body["bot_token"] = serde_json::json!(t);
            body["ilink_bot_id"] = serde_json::json!(b);
            if let Some(u) = user_id {
                body["ilink_user_id"] = serde_json::json!(u);
            }

            if already_confirmed {
                tracing::debug!("WeChat: qrcode already confirmed, skipping save");
                return Json(body).into_response();
            }

            let (config_path, state_dir) = {
                let config = state.config.lock();
                let config_path = config.config_path.clone();
                let state_dir = config
                    .channels
                    .wechat
                    .as_ref()
                    .and_then(|c| c.state_dir.clone())
                    .map(|s| std::path::PathBuf::from(&s))
                    .or_else(|| config_path.parent().map(|p| p.join("wechat")))
                    .unwrap_or_else(|| std::path::PathBuf::from(".kylinbot/wechat"));
                (config_path, state_dir)
            };
            let path = state_dir.join("account.json");

            if let Err(e) = tokio::fs::create_dir_all(&state_dir).await {
                tracing::warn!("WeChat: failed to create state dir {state_dir:?}: {e}");
            } else {
                let account_data = serde_json::json!({
                    "token": t,
                    "account_id": b,
                    "base_url": WECHAT_API_BASE,
                    "user_id": user_id,
                    "saved_at": chrono::Utc::now().to_rfc3339(),
                });
                match serde_json::to_string_pretty(&account_data) {
                    Ok(json) => {
                        if let Err(e) = tokio::fs::write(&path, json.as_bytes()).await {
                            tracing::warn!("WeChat: failed to write account.json: {e}");
                        } else {
                            tracing::info!("WeChat: account data saved to {path:?}");
                        }
                    }
                    Err(e) => tracing::warn!("WeChat: failed to serialize account data: {e}"),
                }

                let wechat_section = "[channels.wechat]\nenabled = true\nallowed_users = [\"*\"]";
                match tokio::fs::read_to_string(&config_path).await {
                    Ok(toml_content) => {
                        if toml_content.contains("[channels.wechat]") {
                            tracing::debug!("WeChat: [channels.wechat] already in config.toml");
                        } else {
                            let new_toml =
                                insert_channel_section(&toml_content, "wechat", wechat_section);
                            if let Err(e) =
                                tokio::fs::write(&config_path, new_toml.as_bytes()).await
                            {
                                tracing::warn!("WeChat: failed to update config.toml: {e}");
                            } else {
                                tracing::info!(
                                    "WeChat: wrote [channels.wechat] to {config_path:?}"
                                );
                            }
                        }
                    }
                    Err(e) => tracing::warn!("WeChat: failed to read config.toml: {e}"),
                }
                if let Err(e) = reload_config_and_daemon(&state).await {
                    tracing::warn!("WeChat: config saved but daemon reload was not triggered: {e}");
                }
            }
        }
    }

    Json(body).into_response()
}

pub async fn handle_wechat_account(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let state_dir = {
        let config = state.config.lock();
        wechat_state_dir_from_config(&config)
    };
    let path = state_dir.join("account.json");

    match tokio::fs::read_to_string(&path).await {
        Ok(content) => match serde_json::from_str::<serde_json::Value>(&content) {
            Ok(json) => Json(json).into_response(),
            Err(_) => Json(serde_json::json!({})).into_response(),
        },
        Err(_) => Json(serde_json::json!({})).into_response(),
    }
}

pub async fn handle_wechat_account_delete(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let config = state.config.lock().clone();
    remove_wechat_state_dir(&config).await;

    Json(serde_json::json!({ "status": "ok" })).into_response()
}

// ---------------------------------------------------------------------------
// Feishu (Lark) Device Authorization Grant (RFC 8628)
// ---------------------------------------------------------------------------

const FEISHU_REGISTER_URL: &str = "https://accounts.feishu.cn/oauth/v1/app/registration";

/// POST /api/feishu/register/begin — initiate device authorization flow.
///
/// Returns `{ device_code, verification_uri_complete, expires_in, interval }`.
pub async fn handle_feishu_register_begin() -> impl IntoResponse {
    let params = [
        ("action", "begin"),
        ("archetype", "PersonalAgent"),
        ("auth_method", "client_secret"),
        ("request_user_info", "open_id"),
    ];

    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()
    {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to create HTTP client: {e}")})),
            )
                .into_response();
        }
    };

    let resp = match client
        .post(FEISHU_REGISTER_URL)
        .form(&params)
        .header("Content-Type", "application/x-www-form-urlencoded")
        .send()
        .await
    {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Feishu registration service unreachable: {e}")})),
            )
                .into_response();
        }
    };

    let status = resp.status();
    let body: serde_json::Value = match resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Failed to parse response: {e}")})),
            )
                .into_response();
        }
    };

    if !status.is_success() && status != StatusCode::BAD_REQUEST {
        return (
            StatusCode::from_u16(status.as_u16()).unwrap_or(StatusCode::BAD_GATEWAY),
            Json(serde_json::json!({
                "error": body.get("error").and_then(|v| v.as_str()).unwrap_or("Unknown error"),
                "error_description": body.get("error_description").and_then(|v| v.as_str()),
            })),
        )
            .into_response();
    }

    let device_code = body
        .get("device_code")
        .and_then(|v| v.as_str())
        .unwrap_or("");
    let verification_uri = body
        .get("verification_uri_complete")
        .and_then(|v| v.as_str())
        .unwrap_or("");

    if device_code.is_empty() || verification_uri.is_empty() {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": "Invalid response from Feishu registration service"})),
        )
            .into_response();
    }

    let expires_in = body
        .get("expires_in")
        .and_then(|v| v.as_u64())
        .unwrap_or(600);
    let interval = body.get("interval").and_then(|v| v.as_u64()).unwrap_or(5);

    Json(serde_json::json!({
        "device_code": device_code,
        "verification_uri_complete": verification_uri,
        "expires_in": expires_in,
        "interval": interval,
    }))
    .into_response()
}

/// Request body for polling Feishu registration status.
#[derive(Deserialize)]
pub struct FeishuPollRequest {
    pub device_code: String,
}

/// POST /api/feishu/register/poll — poll device authorization status.
pub async fn handle_feishu_register_poll(
    State(state): State<AppState>,
    Json(body): Json<FeishuPollRequest>,
) -> impl IntoResponse {
    if body.device_code.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "Missing 'device_code' in request body"})),
        )
            .into_response();
    }

    let params = [("action", "poll"), ("device_code", &body.device_code)];

    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()
    {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to create HTTP client: {e}")})),
            )
                .into_response();
        }
    };

    let resp = match client
        .post(FEISHU_REGISTER_URL)
        .form(&params)
        .header("Content-Type", "application/x-www-form-urlencoded")
        .send()
        .await
    {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Feishu poll failed: {e}")})),
            )
                .into_response();
        }
    };

    let status = resp.status();
    let poll_body: serde_json::Value = match resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Failed to parse poll response: {e}")})),
            )
                .into_response();
        }
    };

    let mut result = serde_json::json!({});

    if let Some(client_id) = poll_body.get("client_id").and_then(|v| v.as_str()) {
        result["client_id"] = serde_json::json!(client_id);
    }
    if let Some(client_secret) = poll_body.get("client_secret").and_then(|v| v.as_str()) {
        result["client_secret"] = serde_json::json!(client_secret);
    }

    if let Some(user_info) = poll_body.get("user_info") {
        result["user_info"] = user_info.clone();
    }

    if let Some(error) = poll_body.get("error").and_then(|v| v.as_str()) {
        result["error"] = serde_json::json!(error);
    }
    if let Some(error_desc) = poll_body.get("error_description").and_then(|v| v.as_str()) {
        result["error_description"] = serde_json::json!(error_desc);
    }

    // Auto-save credentials to config when registration succeeds
    if result.get("client_id").is_some() && result.get("client_secret").is_some() {
        let cid = result
            .get("client_id")
            .and_then(|v| v.as_str())
            .unwrap_or("");
        let csecret = result
            .get("client_secret")
            .and_then(|v| v.as_str())
            .unwrap_or("");

        let config_path = {
            let config = state.config.lock();
            config.config_path.clone()
        };

        let feishu_section =
            "[channels.feishu]\nenabled = true\napp_id = \"APP_ID\"\napp_secret = \"APP_SECRET\"\nallowed_users = [\"*\"]\nreceive_mode = \"websocket\""
                .replace("APP_ID", cid)
                .replace("APP_SECRET", csecret);

        match tokio::fs::read_to_string(&config_path).await {
            Ok(toml_content) => {
                if toml_content.contains("[channels.feishu]") {
                    tracing::debug!("Feishu: [channels.feishu] already in config.toml");
                } else {
                    let new_toml = insert_channel_section(&toml_content, "feishu", &feishu_section);

                    if let Err(e) = tokio::fs::write(&config_path, new_toml.as_bytes()).await {
                        tracing::warn!("Feishu: failed to update config.toml: {e}");
                    } else {
                        tracing::info!("Feishu: wrote [channels.feishu] to {config_path:?}");
                    }
                }
            }
            Err(e) => tracing::warn!("Feishu: failed to read config.toml: {e}"),
        }
        if let Err(e) = reload_config_and_daemon(&state).await {
            tracing::warn!("Feishu: config saved but daemon reload was not triggered: {e}");
        }
    }

    if !status.is_success() && result.get("client_id").is_none() {
        return (StatusCode::OK, Json(result)).into_response();
    }

    Json(result).into_response()
}

// ---------------------------------------------------------------------------
// DingTalk Device Flow (scan QR code to register app)
// ---------------------------------------------------------------------------

const DINGTALK_API_BASE: &str = "https://oapi.dingtalk.com";

/// POST /api/dingtalk/register/begin — initiate DingTalk device authorization flow.
///
/// Step 1: POST /app/registration/init → nonce
/// Step 2: POST /app/registration/begin → device_code + QR URL
/// Returns `{ device_code, verification_uri_complete, expires_in, interval }`.
pub async fn handle_dingtalk_register_begin() -> impl IntoResponse {
    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()
    {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to create HTTP client: {e}")})),
            )
                .into_response();
        }
    };

    let init_body = serde_json::json!({});
    let init_resp = match client
        .post(format!("{DINGTALK_API_BASE}/app/registration/init"))
        .json(&init_body)
        .send()
        .await
    {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(
                    serde_json::json!({"error": format!("DingTalk init service unreachable: {e}")}),
                ),
            )
                .into_response();
        }
    };

    let init_status = init_resp.status();
    let init_data: serde_json::Value = match init_resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Failed to parse DingTalk init response: {e}")})),
            )
                .into_response();
        }
    };

    if !init_status.is_success() {
        return (
            StatusCode::from_u16(init_status.as_u16()).unwrap_or(StatusCode::BAD_GATEWAY),
            Json(serde_json::json!({
                "error": init_data.get("errmsg").and_then(|v| v.as_str()).unwrap_or("DingTalk init failed"),
            })),
        )
            .into_response();
    }

    let errcode = init_data
        .get("errcode")
        .and_then(|v| v.as_i64())
        .unwrap_or(-1);
    if errcode != 0 {
        return (
            StatusCode::BAD_GATEWAY,
            Json(serde_json::json!({
                "error": init_data.get("errmsg").and_then(|v| v.as_str()).unwrap_or("DingTalk init failed"),
            })),
        )
            .into_response();
    }

    let nonce = match init_data.get("nonce").and_then(|v| v.as_str()) {
        Some(n) => n.to_string(),
        None => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": "DingTalk init did not return nonce"})),
            )
                .into_response();
        }
    };

    let begin_body = serde_json::json!({ "nonce": nonce });
    let begin_resp = match client
        .post(format!("{DINGTALK_API_BASE}/app/registration/begin"))
        .json(&begin_body)
        .send()
        .await
    {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("DingTalk begin service unreachable: {e}")})),
            )
                .into_response();
        }
    };

    let begin_status = begin_resp.status();
    let begin_data: serde_json::Value = match begin_resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Failed to parse DingTalk begin response: {e}")})),
            )
                .into_response();
        }
    };

    if !begin_status.is_success() {
        return (
            StatusCode::from_u16(begin_status.as_u16()).unwrap_or(StatusCode::BAD_GATEWAY),
            Json(serde_json::json!({
                "error": begin_data.get("errmsg").and_then(|v| v.as_str()).unwrap_or("DingTalk begin failed"),
            })),
        )
            .into_response();
    }

    let begin_errcode = begin_data
        .get("errcode")
        .and_then(|v| v.as_i64())
        .unwrap_or(-1);
    if begin_errcode != 0 {
        return (
            StatusCode::BAD_GATEWAY,
            Json(serde_json::json!({
                "error": begin_data.get("errmsg").and_then(|v| v.as_str()).unwrap_or("DingTalk begin failed"),
            })),
        )
            .into_response();
    }

    let device_code = begin_data
        .get("device_code")
        .and_then(|v| v.as_str())
        .unwrap_or("");
    let verification_uri = begin_data
        .get("verification_uri_complete")
        .and_then(|v| v.as_str())
        .unwrap_or("");

    if device_code.is_empty() || verification_uri.is_empty() {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(
                serde_json::json!({"error": "Invalid response from DingTalk registration service"}),
            ),
        )
            .into_response();
    }

    let expires_in = begin_data
        .get("expires_in")
        .and_then(|v| v.as_u64())
        .unwrap_or(7200);
    let interval = begin_data
        .get("interval")
        .and_then(|v| v.as_u64())
        .unwrap_or(5);

    Json(serde_json::json!({
        "device_code": device_code,
        "verification_uri_complete": verification_uri,
        "expires_in": expires_in,
        "interval": interval,
    }))
    .into_response()
}

/// Request body for polling DingTalk registration status.
#[derive(Deserialize)]
pub struct DingtalkPollRequest {
    pub device_code: String,
}

/// POST /api/dingtalk/register/poll — poll DingTalk device authorization status.
pub async fn handle_dingtalk_register_poll(
    State(state): State<AppState>,
    Json(body): Json<DingtalkPollRequest>,
) -> impl IntoResponse {
    if body.device_code.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "Missing 'device_code' in request body"})),
        )
            .into_response();
    }

    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()
    {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to create HTTP client: {e}")})),
            )
                .into_response();
        }
    };

    let poll_body = serde_json::json!({ "device_code": body.device_code });
    let resp = match client
        .post(format!("{DINGTALK_API_BASE}/app/registration/poll"))
        .json(&poll_body)
        .send()
        .await
    {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("DingTalk poll failed: {e}")})),
            )
                .into_response();
        }
    };

    let status = resp.status();
    let poll_data: serde_json::Value = match resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Failed to parse DingTalk poll response: {e}")})),
            )
                .into_response();
        }
    };

    let mut result = serde_json::json!({});

    let poll_status = poll_data
        .get("status")
        .and_then(|v| v.as_str())
        .unwrap_or("");
    result["status"] = serde_json::json!(poll_status);

    if let Some(client_id) = poll_data.get("client_id").and_then(|v| v.as_str()) {
        result["client_id"] = serde_json::json!(client_id);
    }
    if let Some(client_secret) = poll_data.get("client_secret").and_then(|v| v.as_str()) {
        result["client_secret"] = serde_json::json!(client_secret);
    }
    if let Some(fail_reason) = poll_data.get("fail_reason").and_then(|v| v.as_str()) {
        result["fail_reason"] = serde_json::json!(fail_reason);
    }

    let errcode = poll_data
        .get("errcode")
        .and_then(|v| v.as_i64())
        .unwrap_or(0);
    if errcode != 0 {
        result["error"] = poll_data
            .get("errmsg")
            .and_then(|v| v.as_str())
            .unwrap_or("DingTalk poll failed")
            .into();
    }

    if result.get("client_id").is_some() && result.get("client_secret").is_some() {
        let cid = result
            .get("client_id")
            .and_then(|v| v.as_str())
            .unwrap_or("");
        let csecret = result
            .get("client_secret")
            .and_then(|v| v.as_str())
            .unwrap_or("");

        let config_path = {
            let config = state.config.lock();
            config.config_path.clone()
        };

        let dingtalk_section = format!(
            "[channels.dingtalk]\nenabled = true\nclient_id = \"{cid}\"\nclient_secret = \"{csecret}\"\nallowed_users = [\"*\"]"
        );

        match tokio::fs::read_to_string(&config_path).await {
            Ok(toml_content) => {
                if toml_content.contains("[channels.dingtalk]") {
                    tracing::debug!("DingTalk: [channels.dingtalk] already in config.toml");
                } else {
                    let new_toml =
                        insert_channel_section(&toml_content, "dingtalk", &dingtalk_section);
                    if let Err(e) = tokio::fs::write(&config_path, new_toml.as_bytes()).await {
                        tracing::warn!("DingTalk: failed to update config.toml: {e}");
                    } else {
                        tracing::info!("DingTalk: wrote [channels.dingtalk] to {config_path:?}");
                    }
                }
            }
            Err(e) => tracing::warn!("DingTalk: failed to read config.toml: {e}"),
        }
        if let Err(e) = reload_config_and_daemon(&state).await {
            tracing::warn!("DingTalk: config saved but daemon reload was not triggered: {e}");
        }
    }

    if !status.is_success() && result.get("client_id").is_none() {
        return (StatusCode::OK, Json(result)).into_response();
    }

    Json(result).into_response()
}

// ---------------------------------------------------------------------------
// WeCom AI Bot QR Code Binding
// ---------------------------------------------------------------------------

const WECOM_QR_GENERATE_URL: &str =
    "https://work.weixin.qq.com/ai/qc/generate?source=wecom-cli&plat=3";
const WECOM_QR_QUERY_URL: &str = "https://work.weixin.qq.com/ai/qc/query_result";

/// GET /api/wecom/qrcode — generate WeCom AI Bot QR code for scanning.
///
/// Returns `{ scode, auth_url }`.
pub async fn handle_wecom_qrcode() -> impl IntoResponse {
    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()
    {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to create HTTP client: {e}")})),
            )
                .into_response();
        }
    };

    let resp = match client.get(WECOM_QR_GENERATE_URL).send().await {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("WeCom QR service unreachable: {e}")})),
            )
                .into_response();
        }
    };

    let data: serde_json::Value = match resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(
                    serde_json::json!({"error": format!("Failed to parse WeCom QR response: {e}")}),
                ),
            )
                .into_response();
        }
    };

    let scode = data
        .pointer("/data/scode")
        .and_then(|v| v.as_str())
        .unwrap_or("");
    let auth_url = data
        .pointer("/data/auth_url")
        .and_then(|v| v.as_str())
        .unwrap_or("");

    if scode.is_empty() || auth_url.is_empty() {
        return (
            StatusCode::BAD_GATEWAY,
            Json(serde_json::json!({"error": "Invalid response from WeCom QR service"})),
        )
            .into_response();
    }

    Json(serde_json::json!({
        "scode": scode,
        "auth_url": auth_url,
    }))
    .into_response()
}

/// Query parameters for WeCom QR status polling.
#[derive(Deserialize)]
pub struct WecomQrStatusQuery {
    pub scode: String,
}

/// GET /api/wecom/status — poll WeCom QR code scan status.
///
/// Returns `{ status, bot_id?, secret? }`.
/// No auth required (public endpoint for binding flow).
pub async fn handle_wecom_status(
    State(state): State<AppState>,
    Query(params): Query<WecomQrStatusQuery>,
) -> impl IntoResponse {
    if params.scode.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "Missing 'scode' query parameter"})),
        )
            .into_response();
    }

    let url = format!(
        "{WECOM_QR_QUERY_URL}?scode={}",
        urlencoding::encode(&params.scode)
    );

    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()
    {
        Ok(c) => c,
        Err(e) => {
            return (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(serde_json::json!({"error": format!("Failed to create HTTP client: {e}")})),
            )
                .into_response();
        }
    };

    let resp = match client.get(&url).send().await {
        Ok(r) => r,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("WeCom status check failed: {e}")})),
            )
                .into_response();
        }
    };

    let data: serde_json::Value = match resp.json().await {
        Ok(b) => b,
        Err(e) => {
            return (
                StatusCode::BAD_GATEWAY,
                Json(serde_json::json!({"error": format!("Failed to parse WeCom status response: {e}")})),
            )
                .into_response();
        }
    };

    let status = data
        .pointer("/data/status")
        .and_then(|v| v.as_str())
        .unwrap_or("waiting");

    let mut body = serde_json::json!({
        "status": status,
    });

    if status == "success" {
        let bot_id = data
            .pointer("/data/bot_info/botid")
            .and_then(|v| v.as_str());
        let secret = data
            .pointer("/data/bot_info/secret")
            .and_then(|v| v.as_str());

        if let (Some(bid), Some(sec)) = (bot_id, secret) {
            body["bot_id"] = serde_json::json!(bid);
            body["secret"] = serde_json::json!(sec);

            let config_path = {
                let config = state.config.lock();
                config.config_path.clone()
            };

            let wecom_section = format!(
                "[channels.wecom]\nenabled = true\nbot_id = \"{bid}\"\nsecret = \"{sec}\"\nallowed_users = [\"*\"]"
            );

            match tokio::fs::read_to_string(&config_path).await {
                Ok(toml_content) => {
                    if toml_content.contains("[channels.wecom]") {
                        tracing::debug!("WeCom: [channels.wecom] already in config.toml");
                    } else {
                        let new_toml =
                            insert_channel_section(&toml_content, "wecom", &wecom_section);
                        if let Err(e) = tokio::fs::write(&config_path, new_toml.as_bytes()).await {
                            tracing::warn!("WeCom: failed to update config.toml: {e}");
                        } else {
                            tracing::info!("WeCom: wrote [channels.wecom] to {config_path:?}");
                        }
                    }
                }
                Err(e) => tracing::warn!("WeCom: failed to read config.toml: {e}"),
            }
            if let Err(e) = reload_config_and_daemon(&state).await {
                tracing::warn!("WeCom: config saved but daemon reload was not triggered: {e}");
            }
        }
    }

    Json(body).into_response()
}

// ---------------------------------------------------------------------------
// Channel manual binding
// ---------------------------------------------------------------------------

#[derive(Debug, Deserialize)]
pub struct FeishuConfigRequest {
    pub app_id: String,
    pub app_secret: String,
}

#[derive(Debug, Deserialize)]
pub struct DingTalkConfigRequest {
    pub client_id: String,
    pub client_secret: String,
}

#[derive(Debug, Deserialize)]
pub struct WeComConfigRequest {
    pub bot_id: String,
    pub secret: String,
}

pub async fn handle_feishu_config(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(req): Json<FeishuConfigRequest>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let app_id = req.app_id.trim().to_string();
    let app_secret = req.app_secret.trim().to_string();
    if app_id.is_empty() || app_secret.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "app_id and app_secret are required"})),
        )
            .into_response();
    }

    let mut new_config = state.config.lock().clone();
    new_config.channels.feishu = Some(schema::FeishuConfig {
        enabled: true,
        app_id,
        app_secret,
        encrypt_key: None,
        verification_token: None,
        allowed_users: vec!["*".to_string()],
        mention_only: false,
        receive_mode: schema::LarkReceiveMode::Websocket,
        port: None,
        proxy_url: None,
    });

    if let Err(e) = save_config_and_reload(&state, new_config).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": e})),
        )
            .into_response();
    }

    Json(serde_json::json!({"status": "ok"})).into_response()
}

pub async fn handle_dingtalk_config(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(req): Json<DingTalkConfigRequest>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let client_id = req.client_id.trim().to_string();
    let client_secret = req.client_secret.trim().to_string();
    if client_id.is_empty() || client_secret.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "client_id and client_secret are required"})),
        )
            .into_response();
    }

    let mut new_config = state.config.lock().clone();
    new_config.channels.dingtalk = Some(schema::DingTalkConfig {
        enabled: true,
        client_id,
        client_secret,
        allowed_users: vec!["*".to_string()],
        proxy_url: None,
    });

    if let Err(e) = save_config_and_reload(&state, new_config).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": e})),
        )
            .into_response();
    }

    Json(serde_json::json!({"status": "ok"})).into_response()
}

pub async fn handle_wecom_config(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(req): Json<WeComConfigRequest>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let bot_id = req.bot_id.trim().to_string();
    let secret = req.secret.trim().to_string();
    if bot_id.is_empty() || secret.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "bot_id and secret are required"})),
        )
            .into_response();
    }

    let mut new_config = state.config.lock().clone();
    new_config.channels.wecom = Some(schema::WeComConfig {
        enabled: true,
        webhook_key: None,
        bot_id: Some(bot_id),
        secret: Some(secret),
        websocket_url: None,
        allowed_users: vec!["*".to_string()],
    });

    if let Err(e) = save_config_and_reload(&state, new_config).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": e})),
        )
            .into_response();
    }

    Json(serde_json::json!({"status": "ok"})).into_response()
}

// ---------------------------------------------------------------------------
// Lansenger manual binding
// ---------------------------------------------------------------------------

const LANSENGER_DEFAULT_API_GATEWAY_URL: &str = "https://open.e.lanxin.cn/open/apigw";

#[derive(Debug, Deserialize)]
pub struct LansengerConfigRequest {
    pub app_id: String,
    pub app_secret: String,
    #[serde(default)]
    pub api_gateway_url: Option<String>,
}

#[derive(Debug, Deserialize)]
struct LansengerProbeResponse {
    #[serde(rename = "errCode")]
    err_code: i64,
    #[serde(rename = "errMsg")]
    err_msg: Option<String>,
    data: Option<LansengerProbeData>,
}

#[derive(Debug, Deserialize)]
struct LansengerProbeData {
    #[serde(rename = "appToken")]
    app_token: Option<String>,
}

pub async fn handle_lansenger_probe(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(req): Json<LansengerConfigRequest>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let app_id = req.app_id.trim();
    let app_secret = req.app_secret.trim();
    if app_id.is_empty() || app_secret.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "app_id and app_secret are required"})),
        )
            .into_response();
    }

    match probe_lansenger_credentials(app_id, app_secret, req.api_gateway_url.as_deref()).await {
        Ok(()) => Json(serde_json::json!({"ok": true})).into_response(),
        Err(e) => (
            StatusCode::BAD_GATEWAY,
            Json(serde_json::json!({"ok": false, "error": e.to_string()})),
        )
            .into_response(),
    }
}

pub async fn handle_lansenger_config(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(req): Json<LansengerConfigRequest>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let app_id = req.app_id.trim().to_string();
    let app_secret = req.app_secret.trim().to_string();
    if app_id.is_empty() || app_secret.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(serde_json::json!({"error": "app_id and app_secret are required"})),
        )
            .into_response();
    }

    let api_gateway_url = normalize_lansenger_gateway_url(req.api_gateway_url.as_deref());

    let mut new_config = state.config.lock().clone();
    new_config.channels.lansenger = Some(schema::LansengerConfig {
        enabled: true,
        app_id,
        app_secret,
        api_gateway_url,
        allowed_users: vec!["*".to_string()],
        group_allow_from: Vec::new(),
        mention_only: false,
        proxy_url: None,
    });

    if let Err(e) = save_config_and_reload(&state, new_config).await {
        return (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(serde_json::json!({"error": e})),
        )
            .into_response();
    }

    Json(serde_json::json!({"status": "ok"})).into_response()
}

async fn probe_lansenger_credentials(
    app_id: &str,
    app_secret: &str,
    api_gateway_url: Option<&str>,
) -> anyhow::Result<()> {
    let gateway_url = normalize_lansenger_gateway_url(api_gateway_url);
    let url = format!(
        "{}/v1/apptoken/create?grant_type=client_credential&appid={}&secret={}",
        gateway_url,
        urlencoding::encode(app_id),
        urlencoding::encode(app_secret)
    );
    let client = reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(30))
        .build()?;
    let resp = client.get(url).send().await?;
    let status = resp.status();
    let text = resp.text().await.unwrap_or_default();
    if !status.is_success() {
        anyhow::bail!("Lansenger probe failed ({status}): {text}");
    }

    let parsed: LansengerProbeResponse = serde_json::from_str(&text)?;
    if parsed.err_code != 0 {
        anyhow::bail!(
            "Lansenger probe failed: errCode={} errMsg={}",
            parsed.err_code,
            parsed.err_msg.as_deref().unwrap_or("unknown")
        );
    }
    if parsed
        .data
        .and_then(|data| data.app_token)
        .is_none_or(|token| token.is_empty())
    {
        anyhow::bail!("Lansenger probe response missing appToken");
    }

    Ok(())
}

fn normalize_lansenger_gateway_url(api_gateway_url: Option<&str>) -> String {
    let trimmed = api_gateway_url
        .map(str::trim)
        .filter(|value| !value.is_empty())
        .unwrap_or(LANSENGER_DEFAULT_API_GATEWAY_URL);
    trimmed.trim_end_matches('/').to_string()
}

fn insert_channel_section(toml_content: &str, section_name: &str, new_section: &str) -> String {
    let check_existing = format!("[channels.{}]", section_name);
    if toml_content.contains(&check_existing) {
        return toml_content.to_string();
    }

    let section = new_section.trim();

    if let Some(channels_pos) = toml_content.find("[channels]") {
        let after_channels = &toml_content[channels_pos..];

        let mut last_channels_subsection_end = channels_pos;
        let mut found_any_subsection = false;

        for (pos, ch) in after_channels.char_indices() {
            if ch == '[' {
                let rest = &after_channels[pos..];
                let trimmed = rest.trim_start();
                if trimmed.starts_with("[channels.") {
                    let absolute_pos = channels_pos + pos + (rest.len() - trimmed.len());
                    last_channels_subsection_end = absolute_pos;
                    found_any_subsection = true;
                }
            }
        }

        if found_any_subsection {
            let after_last_subsection = &toml_content[last_channels_subsection_end..];
            let lines: Vec<&str> = after_last_subsection.split('\n').collect();

            let mut content_end_line = 1;
            for (i, line) in lines.iter().enumerate().skip(1) {
                let trimmed = line.trim();
                if trimmed.is_empty() || trimmed.starts_with('[') {
                    content_end_line = i;
                    break;
                }
                content_end_line = i + 1;
            }

            let mut insert_byte_pos = last_channels_subsection_end;
            for line in lines.iter().take(content_end_line) {
                insert_byte_pos += line.len() + 1;
            }

            let remaining = &toml_content[insert_byte_pos..];
            let remaining_stripped = remaining.trim_start_matches('\n');
            let followed_by_section = remaining_stripped.starts_with('[');

            let after = if followed_by_section {
                format!("\n{}", remaining_stripped)
            } else {
                remaining_stripped.to_string()
            };

            format!(
                "{}\n{}\n{}",
                &toml_content[..insert_byte_pos],
                section,
                after
            )
        } else {
            let lines_after_channels: Vec<&str> = after_channels.split('\n').collect();

            let mut last_kv_line = 0;
            for (i, line) in lines_after_channels.iter().enumerate().skip(1) {
                let trimmed = line.trim();
                if trimmed.starts_with('[') {
                    break;
                }
                if trimmed.contains('=') {
                    last_kv_line = i;
                }
            }

            let mut insert_byte_pos = channels_pos;
            for line in lines_after_channels.iter().take(last_kv_line + 1) {
                insert_byte_pos += line.len() + 1;
            }

            let remaining = &toml_content[insert_byte_pos..];
            let remaining_stripped = remaining.trim_start_matches('\n');
            let followed_by_section = remaining_stripped.starts_with('[');

            let after = if followed_by_section {
                format!("\n{}", remaining_stripped)
            } else {
                remaining_stripped.to_string()
            };

            format!(
                "{}\n{}\n{}",
                &toml_content[..insert_byte_pos],
                section,
                after
            )
        }
    } else {
        format!("{}\n\n[channels]\n{}\n", toml_content.trim_end(), section)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::auth_rate_limit::AuthRateLimiter;
    use crate::nodes;
    use crate::session_queue::SessionActorQueue;
    use crate::sse::EventBuffer;
    use crate::{GatewayRateLimiter, IdempotencyStore};
    use async_trait::async_trait;
    use axum::http::StatusCode;
    use http_body_util::BodyExt;
    use parking_lot::Mutex;
    use kylinbot_channels::email_channel;
    use kylinbot_memory::{Memory, MemoryEntry};
    use kylinbot_providers::Provider;
    use kylinbot_runtime::observability::NoopObserver;
    use kylinbot_runtime::security::pairing::PairingGuard;
    use std::sync::Arc;
    use std::time::Duration;

    struct MockMemory;

    #[async_trait]
    impl Memory for MockMemory {
        fn name(&self) -> &str {
            "mock"
        }

        async fn store(
            &self,
            _key: &str,
            _content: &str,
            _category: MemoryCategory,
            _session_id: Option<&str>,
        ) -> anyhow::Result<()> {
            Ok(())
        }

        async fn recall(
            &self,
            _query: &str,
            _limit: usize,
            _session_id: Option<&str>,
            _since: Option<&str>,
            _until: Option<&str>,
        ) -> anyhow::Result<Vec<MemoryEntry>> {
            Ok(Vec::new())
        }

        async fn get(&self, _key: &str) -> anyhow::Result<Option<MemoryEntry>> {
            Ok(None)
        }

        async fn list(
            &self,
            _category: Option<&MemoryCategory>,
            _session_id: Option<&str>,
        ) -> anyhow::Result<Vec<MemoryEntry>> {
            Ok(Vec::new())
        }

        async fn forget(&self, _key: &str) -> anyhow::Result<bool> {
            Ok(false)
        }

        async fn count(&self) -> anyhow::Result<usize> {
            Ok(0)
        }

        async fn health_check(&self) -> bool {
            true
        }
    }

    struct MockProvider;

    #[async_trait]
    impl Provider for MockProvider {
        async fn chat_with_system(
            &self,
            _system_prompt: Option<&str>,
            _message: &str,
            _model: &str,
            _temperature: Option<f64>,
        ) -> anyhow::Result<String> {
            Ok("ok".to_string())
        }
    }

    fn test_state(config: Config) -> AppState {
        AppState {
            config: Arc::new(Mutex::new(config)),
            provider: Arc::new(MockProvider),
            model: "test-model".into(),
            temperature: 0.0,
            mem: Arc::new(MockMemory),
            auto_save: false,
            webhook_secret_hash: None,
            pairing: Arc::new(PairingGuard::new(false, &[])),
            trust_forwarded_headers: false,
            rate_limiter: Arc::new(GatewayRateLimiter::new(100, 100, 100)),
            auth_limiter: Arc::new(AuthRateLimiter::new()),
            idempotency_store: Arc::new(IdempotencyStore::new(Duration::from_secs(300), 1000)),
            whatsapp: None,
            whatsapp_app_secret: None,
            linq: None,
            linq_signing_secret: None,
            nextcloud_talk: None,
            nextcloud_talk_webhook_secret: None,
            wati: None,
            gmail_push: None,
            observer: Arc::new(NoopObserver),
            tools_registry: Arc::new(Vec::new()),
            cost_tracker: None,
            event_tx: tokio::sync::broadcast::channel(16).0,
            event_buffer: Arc::new(EventBuffer::new(16)),
            shutdown_tx: tokio::sync::watch::channel(false).0,
            node_registry: Arc::new(nodes::NodeRegistry::new(16)),
            web_dist_dir: None,
            session_backend: None,
            session_queue: Arc::new(SessionActorQueue::new(8, 30, 600)),
            device_registry: None,
            pending_pairings: None,
            path_prefix: String::new(),
            canvas_store: kylinbot_runtime::tools::CanvasStore::new(),
            cancel_tokens: Arc::new(std::sync::Mutex::new(std::collections::HashMap::new())),
            reload_tx: None,
            #[cfg(feature = "webauthn")]
            webauthn: None,
        }
    }

    async fn response_json(response: axum::response::Response) -> serde_json::Value {
        let body = response
            .into_body()
            .collect()
            .await
            .expect("response body")
            .to_bytes();
        serde_json::from_slice(&body).expect("valid json response")
    }

    #[test]
    fn masking_keeps_toml_valid_and_preserves_api_keys_type() {
        let mut cfg = Config::default();
        cfg.providers.fallback = Some("openrouter".to_string());
        cfg.providers.models.insert(
            "openrouter".to_string(),
            schema::ModelProviderConfig {
                api_key: Some("sk-live-123".to_string()),
                ..schema::ModelProviderConfig::default()
            },
        );
        cfg.reliability.api_keys = vec!["rk-1".to_string(), "rk-2".to_string()];
        cfg.gateway.paired_tokens = vec!["pair-token-1".to_string()];
        cfg.tunnel.cloudflare = Some(schema::CloudflareTunnelConfig {
            token: "cf-token".to_string(),
        });
        cfg.memory.qdrant.api_key = Some("qdrant-key".to_string());
        cfg.channels.wati = Some(schema::WatiConfig {
            enabled: true,
            api_token: "wati-token".to_string(),
            api_url: "https://live-mt-server.wati.io".to_string(),
            tenant_id: None,
            allowed_numbers: vec![],
            proxy_url: None,
        });
        cfg.channels.feishu = Some(schema::FeishuConfig {
            enabled: true,
            app_id: "cli_aabbcc".to_string(),
            app_secret: "feishu-secret".to_string(),
            encrypt_key: Some("feishu-encrypt".to_string()),
            verification_token: Some("feishu-verify".to_string()),
            allowed_users: vec!["*".to_string()],
            mention_only: false,
            receive_mode: schema::LarkReceiveMode::Websocket,
            port: None,
            proxy_url: None,
        });
        cfg.channels.email = Some(email_channel::EmailConfig {
            enabled: true,
            imap_host: "imap.example.com".to_string(),
            imap_port: 993,
            imap_folder: "INBOX".to_string(),
            smtp_host: "smtp.example.com".to_string(),
            smtp_port: 465,
            smtp_tls: true,
            username: "agent@example.com".to_string(),
            password: "email-password-secret".to_string(),
            from_address: "agent@example.com".to_string(),
            idle_timeout_secs: 1740,
            poll_interval_secs: 60,
            allowed_senders: vec!["*".to_string()],
            default_subject: "KylinBot Message".to_string(),
            max_attachment_bytes: 25 * 1024 * 1024,
        });
        cfg.providers.model_routes = vec![schema::ModelRouteConfig {
            hint: "reasoning".to_string(),
            provider: "openrouter".to_string(),
            model: "anthropic/claude-sonnet-4.6".to_string(),
            api_key: Some("route-model-key".to_string()),
        }];
        cfg.providers.embedding_routes = vec![schema::EmbeddingRouteConfig {
            hint: "semantic".to_string(),
            provider: "openai".to_string(),
            model: "text-embedding-3-small".to_string(),
            dimensions: Some(1536),
            api_key: Some("route-embed-key".to_string()),
        }];

        let masked = mask_sensitive_fields(&cfg);
        let toml = toml::to_string_pretty(&masked).expect("masked config should serialize");
        let parsed: Config =
            toml::from_str(&toml).expect("masked config should remain valid TOML for Config");
        assert_eq!(
            parsed
                .providers
                .models
                .get("openrouter")
                .and_then(|v| v.api_key.as_deref()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed.reliability.api_keys,
            vec![MASKED_SECRET.to_string(), MASKED_SECRET.to_string()]
        );
        assert_eq!(
            parsed.gateway.paired_tokens,
            vec![MASKED_SECRET.to_string()]
        );
        assert_eq!(
            parsed.tunnel.cloudflare.as_ref().map(|v| v.token.as_str()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed.channels.wati.as_ref().map(|v| v.api_token.as_str()),
            Some(MASKED_SECRET)
        );
        assert_eq!(parsed.memory.qdrant.api_key.as_deref(), Some(MASKED_SECRET));
        assert_eq!(
            parsed
                .channels
                .feishu
                .as_ref()
                .map(|v| v.app_secret.as_str()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed
                .channels
                .feishu
                .as_ref()
                .and_then(|v| v.encrypt_key.as_deref()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed
                .channels
                .feishu
                .as_ref()
                .and_then(|v| v.verification_token.as_deref()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed
                .providers
                .model_routes
                .first()
                .and_then(|v| v.api_key.as_deref()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed
                .providers
                .embedding_routes
                .first()
                .and_then(|v| v.api_key.as_deref()),
            Some(MASKED_SECRET)
        );
        assert_eq!(
            parsed.channels.email.as_ref().map(|v| v.password.as_str()),
            Some(MASKED_SECRET)
        );
    }

    #[test]
    fn hydrate_config_for_save_restores_masked_secrets_and_paths() {
        let mut current = Config::default();
        current.config_path = std::path::PathBuf::from("/tmp/current/config.toml");
        current.workspace_dir = std::path::PathBuf::from("/tmp/current/workspace");
        current.providers.fallback = Some("openrouter".to_string());
        current.providers.models.insert(
            "openrouter".to_string(),
            schema::ModelProviderConfig {
                api_key: Some("real-key".to_string()),
                ..schema::ModelProviderConfig::default()
            },
        );
        current.reliability.api_keys = vec!["r1".to_string(), "r2".to_string()];
        current.gateway.paired_tokens = vec!["pair-1".to_string(), "pair-2".to_string()];
        current.tunnel.cloudflare = Some(schema::CloudflareTunnelConfig {
            token: "cf-token-real".to_string(),
        });
        current.tunnel.ngrok = Some(schema::NgrokTunnelConfig {
            auth_token: "ngrok-token-real".to_string(),
            domain: None,
        });
        current.memory.qdrant.api_key = Some("qdrant-real".to_string());
        current.channels.wati = Some(schema::WatiConfig {
            enabled: true,
            api_token: "wati-real".to_string(),
            api_url: "https://live-mt-server.wati.io".to_string(),
            tenant_id: None,
            allowed_numbers: vec![],
            proxy_url: None,
        });
        current.channels.feishu = Some(schema::FeishuConfig {
            enabled: true,
            app_id: "cli_current".to_string(),
            app_secret: "feishu-secret-real".to_string(),
            encrypt_key: Some("feishu-encrypt-real".to_string()),
            verification_token: Some("feishu-verify-real".to_string()),
            allowed_users: vec!["*".to_string()],
            mention_only: false,
            receive_mode: schema::LarkReceiveMode::Websocket,
            port: None,
            proxy_url: None,
        });
        current.channels.email = Some(email_channel::EmailConfig {
            enabled: true,
            imap_host: "imap.example.com".to_string(),
            imap_port: 993,
            imap_folder: "INBOX".to_string(),
            smtp_host: "smtp.example.com".to_string(),
            smtp_port: 465,
            smtp_tls: true,
            username: "agent@example.com".to_string(),
            password: "email-password-real".to_string(),
            from_address: "agent@example.com".to_string(),
            idle_timeout_secs: 1740,
            poll_interval_secs: 60,
            allowed_senders: vec!["*".to_string()],
            default_subject: "KylinBot Message".to_string(),
            max_attachment_bytes: 25 * 1024 * 1024,
        });
        current.providers.model_routes = vec![
            schema::ModelRouteConfig {
                hint: "reasoning".to_string(),
                provider: "openrouter".to_string(),
                model: "anthropic/claude-sonnet-4.6".to_string(),
                api_key: Some("route-model-key-1".to_string()),
            },
            schema::ModelRouteConfig {
                hint: "fast".to_string(),
                provider: "openrouter".to_string(),
                model: "openai/gpt-4.1-mini".to_string(),
                api_key: Some("route-model-key-2".to_string()),
            },
        ];
        current.providers.embedding_routes = vec![
            schema::EmbeddingRouteConfig {
                hint: "semantic".to_string(),
                provider: "openai".to_string(),
                model: "text-embedding-3-small".to_string(),
                dimensions: Some(1536),
                api_key: Some("route-embed-key-1".to_string()),
            },
            schema::EmbeddingRouteConfig {
                hint: "archive".to_string(),
                provider: "custom:https://emb.example.com/v1".to_string(),
                model: "bge-m3".to_string(),
                dimensions: Some(1024),
                api_key: Some("route-embed-key-2".to_string()),
            },
        ];

        let mut incoming = mask_sensitive_fields(&current);
        incoming.image_gen.default_model = "gpt-4.1-mini".to_string();
        // Simulate UI changing only one key and keeping the first masked.
        incoming.reliability.api_keys = vec![MASKED_SECRET.to_string(), "r2-new".to_string()];
        incoming.gateway.paired_tokens = vec![MASKED_SECRET.to_string(), "pair-2-new".to_string()];
        if let Some(cloudflare) = incoming.tunnel.cloudflare.as_mut() {
            cloudflare.token = MASKED_SECRET.to_string();
        }
        if let Some(ngrok) = incoming.tunnel.ngrok.as_mut() {
            ngrok.auth_token = MASKED_SECRET.to_string();
        }
        incoming.memory.qdrant.api_key = Some(MASKED_SECRET.to_string());
        if let Some(wati) = incoming.channels.wati.as_mut() {
            wati.api_token = MASKED_SECRET.to_string();
        }
        if let Some(feishu) = incoming.channels.feishu.as_mut() {
            feishu.app_secret = MASKED_SECRET.to_string();
            feishu.encrypt_key = Some(MASKED_SECRET.to_string());
            feishu.verification_token = Some("feishu-verify-new".to_string());
        }
        if let Some(email) = incoming.channels.email.as_mut() {
            email.password = MASKED_SECRET.to_string();
        }
        incoming.providers.model_routes[1].api_key = Some("route-model-key-2-new".to_string());
        incoming.providers.embedding_routes[1].api_key = Some("route-embed-key-2-new".to_string());

        let hydrated = hydrate_config_for_save(incoming, &current);

        assert_eq!(hydrated.config_path, current.config_path);
        assert_eq!(hydrated.workspace_dir, current.workspace_dir);
        assert_eq!(
            hydrated
                .providers
                .models
                .get("openrouter")
                .and_then(|v| v.api_key.as_deref()),
            Some("real-key")
        );
        assert_eq!(hydrated.image_gen.default_model, "gpt-4.1-mini");
        assert_eq!(
            hydrated.reliability.api_keys,
            vec!["r1".to_string(), "r2-new".to_string()]
        );
        assert_eq!(
            hydrated.gateway.paired_tokens,
            vec!["pair-1".to_string(), "pair-2-new".to_string()]
        );
        assert_eq!(
            hydrated
                .tunnel
                .cloudflare
                .as_ref()
                .map(|v| v.token.as_str()),
            Some("cf-token-real")
        );
        assert_eq!(
            hydrated
                .tunnel
                .ngrok
                .as_ref()
                .map(|v| v.auth_token.as_str()),
            Some("ngrok-token-real")
        );
        assert_eq!(
            hydrated.memory.qdrant.api_key.as_deref(),
            Some("qdrant-real")
        );
        assert_eq!(
            hydrated
                .channels
                .wati
                .as_ref()
                .map(|v| v.api_token.as_str()),
            Some("wati-real")
        );
        assert_eq!(
            hydrated
                .channels
                .feishu
                .as_ref()
                .map(|v| v.app_secret.as_str()),
            Some("feishu-secret-real")
        );
        assert_eq!(
            hydrated
                .channels
                .feishu
                .as_ref()
                .and_then(|v| v.encrypt_key.as_deref()),
            Some("feishu-encrypt-real")
        );
        assert_eq!(
            hydrated
                .channels
                .feishu
                .as_ref()
                .and_then(|v| v.verification_token.as_deref()),
            Some("feishu-verify-new")
        );
        assert_eq!(
            hydrated.providers.model_routes[0].api_key.as_deref(),
            Some("route-model-key-1")
        );
        assert_eq!(
            hydrated.providers.model_routes[1].api_key.as_deref(),
            Some("route-model-key-2-new")
        );
        assert_eq!(
            hydrated.providers.embedding_routes[0].api_key.as_deref(),
            Some("route-embed-key-1")
        );
        assert_eq!(
            hydrated.providers.embedding_routes[1].api_key.as_deref(),
            Some("route-embed-key-2-new")
        );
        assert_eq!(
            hydrated
                .channels
                .email
                .as_ref()
                .map(|v| v.password.as_str()),
            Some("email-password-real")
        );
    }

    #[test]
    fn hydrate_config_for_save_restores_route_keys_by_identity_and_clears_unmatched_masks() {
        let mut current = Config::default();
        current.providers.model_routes = vec![
            schema::ModelRouteConfig {
                hint: "reasoning".to_string(),
                provider: "openrouter".to_string(),
                model: "anthropic/claude-sonnet-4.6".to_string(),
                api_key: Some("route-model-key-1".to_string()),
            },
            schema::ModelRouteConfig {
                hint: "fast".to_string(),
                provider: "openrouter".to_string(),
                model: "openai/gpt-4.1-mini".to_string(),
                api_key: Some("route-model-key-2".to_string()),
            },
        ];
        current.providers.embedding_routes = vec![
            schema::EmbeddingRouteConfig {
                hint: "semantic".to_string(),
                provider: "openai".to_string(),
                model: "text-embedding-3-small".to_string(),
                dimensions: Some(1536),
                api_key: Some("route-embed-key-1".to_string()),
            },
            schema::EmbeddingRouteConfig {
                hint: "archive".to_string(),
                provider: "custom:https://emb.example.com/v1".to_string(),
                model: "bge-m3".to_string(),
                dimensions: Some(1024),
                api_key: Some("route-embed-key-2".to_string()),
            },
        ];

        let mut incoming = mask_sensitive_fields(&current);
        incoming.providers.model_routes.swap(0, 1);
        incoming.providers.embedding_routes.swap(0, 1);
        incoming
            .providers
            .model_routes
            .push(schema::ModelRouteConfig {
                hint: "new".to_string(),
                provider: "openai".to_string(),
                model: "gpt-4.1".to_string(),
                api_key: Some(MASKED_SECRET.to_string()),
            });
        incoming
            .providers
            .embedding_routes
            .push(schema::EmbeddingRouteConfig {
                hint: "new-embed".to_string(),
                provider: "custom:https://emb2.example.com/v1".to_string(),
                model: "bge-small".to_string(),
                dimensions: Some(768),
                api_key: Some(MASKED_SECRET.to_string()),
            });

        let hydrated = hydrate_config_for_save(incoming, &current);

        assert_eq!(
            hydrated.providers.model_routes[0].api_key.as_deref(),
            Some("route-model-key-2")
        );
        assert_eq!(
            hydrated.providers.model_routes[1].api_key.as_deref(),
            Some("route-model-key-1")
        );
        assert_eq!(hydrated.providers.model_routes[2].api_key, None);
        assert_eq!(
            hydrated.providers.embedding_routes[0].api_key.as_deref(),
            Some("route-embed-key-2")
        );
        assert_eq!(
            hydrated.providers.embedding_routes[1].api_key.as_deref(),
            Some("route-embed-key-1")
        );
        assert_eq!(hydrated.providers.embedding_routes[2].api_key, None);
        assert!(
            hydrated
                .providers
                .model_routes
                .iter()
                .all(|route| route.api_key.as_deref() != Some(MASKED_SECRET))
        );
        assert!(
            hydrated
                .providers
                .embedding_routes
                .iter()
                .all(|route| route.api_key.as_deref() != Some(MASKED_SECRET))
        );
    }

    #[tokio::test]
    async fn cron_api_shell_roundtrip_includes_delivery() {
        let tmp = tempfile::TempDir::new().unwrap();
        let config = Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        let state = test_state(config);

        let add_response = handle_api_cron_add(
            State(state.clone()),
            HeaderMap::new(),
            Json(
                serde_json::from_value::<CronAddBody>(serde_json::json!({
                    "name": "test-job",
                    "schedule": "*/5 * * * *",
                    "command": "echo hello",
                    "delivery": {
                        "mode": "announce",
                        "channel": "discord",
                        "to": "1234567890",
                        "best_effort": true
                    }
                }))
                .expect("body should deserialize"),
            ),
        )
        .await
        .into_response();

        let add_json = response_json(add_response).await;
        assert_eq!(add_json["status"], "ok");
        assert_eq!(add_json["job"]["delivery"]["mode"], "announce");
        assert_eq!(add_json["job"]["delivery"]["channel"], "discord");
        assert_eq!(add_json["job"]["delivery"]["to"], "1234567890");

        let list_response = handle_api_cron_list(State(state), HeaderMap::new())
            .await
            .into_response();
        let list_json = response_json(list_response).await;
        let jobs = list_json["jobs"].as_array().expect("jobs array");
        assert_eq!(jobs.len(), 1);
        assert_eq!(jobs[0]["delivery"]["mode"], "announce");
        assert_eq!(jobs[0]["delivery"]["channel"], "discord");
        assert_eq!(jobs[0]["delivery"]["to"], "1234567890");
    }

    #[tokio::test]
    async fn cron_api_accepts_agent_jobs() {
        let tmp = tempfile::TempDir::new().unwrap();
        let config = Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        let state = test_state(config);

        let response = handle_api_cron_add(
            State(state.clone()),
            HeaderMap::new(),
            Json(
                serde_json::from_value::<CronAddBody>(serde_json::json!({
                    "name": "agent-job",
                    "schedule": "*/5 * * * *",
                    "job_type": "agent",
                    "command": "ignored shell command",
                    "prompt": "summarize the latest logs"
                }))
                .expect("body should deserialize"),
            ),
        )
        .await
        .into_response();

        let json = response_json(response).await;
        assert_eq!(json["status"], "ok");

        let config = state.config.lock().clone();
        let jobs = cron::list_jobs(&config).unwrap();
        assert_eq!(jobs.len(), 1);
        assert_eq!(jobs[0].job_type, cron::JobType::Agent);
        assert_eq!(jobs[0].prompt.as_deref(), Some("summarize the latest logs"));
    }

    #[tokio::test]
    async fn cron_api_rejects_announce_delivery_without_target() {
        let tmp = tempfile::TempDir::new().unwrap();
        let config = Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        let state = test_state(config);

        let response = handle_api_cron_add(
            State(state.clone()),
            HeaderMap::new(),
            Json(
                serde_json::from_value::<CronAddBody>(serde_json::json!({
                    "name": "invalid-delivery-job",
                    "schedule": "*/5 * * * *",
                    "command": "echo hello",
                    "delivery": {
                        "mode": "announce",
                        "channel": "discord"
                    }
                }))
                .expect("body should deserialize"),
            ),
        )
        .await
        .into_response();

        assert_eq!(response.status(), StatusCode::BAD_REQUEST);
        let json = response_json(response).await;
        assert!(
            json["error"]
                .as_str()
                .unwrap_or_default()
                .contains("delivery.to is required")
        );

        let config = state.config.lock().clone();
        assert!(cron::list_jobs(&config).unwrap().is_empty());
    }

    #[tokio::test]
    async fn cron_api_rejects_announce_delivery_with_unsupported_channel() {
        let tmp = tempfile::TempDir::new().unwrap();
        let config = Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        let state = test_state(config);

        let response = handle_api_cron_add(
            State(state.clone()),
            HeaderMap::new(),
            Json(
                serde_json::from_value::<CronAddBody>(serde_json::json!({
                    "name": "invalid-delivery-job",
                    "schedule": "*/5 * * * *",
                    "command": "echo hello",
                    "delivery": {
                        "mode": "announce",
                        "channel": "email",
                        "to": "alerts@example.com"
                    }
                }))
                .expect("body should deserialize"),
            ),
        )
        .await
        .into_response();

        assert_eq!(response.status(), StatusCode::BAD_REQUEST);
        let json = response_json(response).await;
        assert!(
            json["error"]
                .as_str()
                .unwrap_or_default()
                .contains("unsupported delivery channel")
        );

        let config = state.config.lock().clone();
        assert!(cron::list_jobs(&config).unwrap().is_empty());
    }

    #[tokio::test]
    async fn channels_endpoint_returns_dashboard_channel_rows() {
        let response = handle_api_channels(State(test_state(Config::default())), HeaderMap::new())
            .await
            .into_response();

        let json = response_json(response).await;
        let channels = json["channels"]
            .as_array()
            .expect("channels should be an array");

        assert!(!channels.is_empty(), "expected configured channel rows");
        let first = &channels[0];
        assert!(first["name"].is_string());
        assert!(first["type"].is_string());
        assert!(first["enabled"].is_boolean());
        assert!(first["message_count"].is_number());
        assert!(first["last_message_at"].is_null());
        assert!(first["status"].is_string());
        assert!(first["health"].is_string());
    }

    #[tokio::test]
    async fn agents_endpoint_returns_configured_agent_profiles() {
        let mut config = Config::default();
        config.agents.insert(
            "researcher".to_string(),
            schema::DelegateAgentConfig {
                provider: "openrouter".to_string(),
                model: "anthropic/claude-sonnet-4.6".to_string(),
                system_prompt: Some("You are a research assistant.".to_string()),
                api_key: None,
                temperature: Some(0.2),
                max_depth: 3,
                agentic: true,
                allowed_tools: vec!["web_search".to_string(), "file_read".to_string()],
                max_iterations: 8,
                timeout_secs: None,
                agentic_timeout_secs: None,
                skills_directory: Some("skills/research".to_string()),
                memory_namespace: None,
            },
        );

        agent_runtime::mark_started("researcher", "trace provider issue", "running");

        let response = handle_api_agents(State(test_state(config)), HeaderMap::new())
            .await
            .into_response();
        let json = response_json(response).await;
        let agents = json["agents"].as_array().expect("agents array");

        assert_eq!(agents.len(), 1);
        assert_eq!(agents[0]["name"], "researcher");
        assert_eq!(agents[0]["provider"], "openrouter");
        assert_eq!(agents[0]["model"], "anthropic/claude-sonnet-4.6");
        assert_eq!(agents[0]["agentic"], true);
        assert_eq!(agents[0]["runtime"]["status"], "running");
        assert_eq!(agents[0]["runtime"]["current_task"], "trace provider issue");
    }

    #[tokio::test]
    async fn cron_api_run_executes_shell_job_and_records_run() {
        let tmp = tempfile::TempDir::new().unwrap();
        let config = kylinbot_config::schema::Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..kylinbot_config::schema::Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        let state = test_state(config);

        let job = kylinbot_runtime::cron::add_shell_job_with_approval(
            &state.config.lock().clone(),
            None,
            kylinbot_runtime::cron::Schedule::Cron {
                expr: "*/5 * * * *".to_string(),
                tz: None,
            },
            "echo hello-from-manual-trigger",
            None,
            true,
        )
        .expect("job added");

        let response =
            handle_api_cron_run(State(state.clone()), HeaderMap::new(), Path(job.id.clone()))
                .await
                .into_response();

        assert_eq!(response.status(), StatusCode::OK);
        let json = response_json(response).await;
        assert_eq!(json["status"], "ok");
        assert_eq!(json["success"], true);
        assert_eq!(json["job_id"], job.id);
        assert!(
            json["output"]
                .as_str()
                .unwrap_or_default()
                .contains("hello-from-manual-trigger")
        );

        let runs = kylinbot_runtime::cron::list_runs(&state.config.lock().clone(), &job.id, 10)
            .expect("runs listed");
        assert_eq!(runs.len(), 1);
        assert_eq!(runs[0].status, "ok");
    }

    #[tokio::test]
    async fn cron_api_run_returns_not_found_for_unknown_job() {
        let tmp = tempfile::TempDir::new().unwrap();
        let config = kylinbot_config::schema::Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..kylinbot_config::schema::Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        let state = test_state(config);

        let response = handle_api_cron_run(
            State(state),
            HeaderMap::new(),
            Path("does-not-exist".to_string()),
        )
        .await
        .into_response();

        assert_eq!(response.status(), StatusCode::NOT_FOUND);
    }
}
