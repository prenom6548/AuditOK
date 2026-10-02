use axum::{
    extract::{Query, State},
    http::{HeaderMap, StatusCode},
    response::{IntoResponse, Json},
};
use serde::{Deserialize, Serialize};

use super::AppState;
use super::api::require_auth;

#[derive(Deserialize)]
pub struct SearchQuery {
    pub q: String,
    pub limit: Option<u32>,
}

#[derive(Deserialize)]
pub struct ShowcaseQuery {
    pub cat: String,
}

#[derive(Deserialize)]
pub struct InstallBody {
    pub source: String,
    pub force: Option<bool>,
    pub timeout: Option<u64>,
}

#[derive(Deserialize)]
pub struct UpgradeBody {
    pub slug: Option<String>,
    pub check_only: Option<bool>,
    pub timeout: Option<u64>,
}

#[derive(Deserialize)]
pub struct UninstallBody {
    pub slug: String,
}

#[derive(Deserialize)]
pub struct DetailQuery {
    pub slug: String,
}

#[derive(Serialize)]
struct SkillhubError {
    success: bool,
    message: String,
}

#[derive(Serialize)]
struct SkillhubSuccess {
    success: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    slug: Option<String>,
    message: String,
}

pub async fn handle_skillhub_search(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<SearchQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let query = params.q.trim().to_string();
    if query.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-query-param-required",
                    &[("param", "q")],
                ),
            }),
        )
            .into_response();
    }

    let api = zeroclaw_skillhub::ApiClient::new();
    let limit = params.limit;
    match api.search(&query, limit).await {
        Ok(results) => Json(serde_json::json!({ "results": results })).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-search-failed",
                    &[("error", &format!("{e}"))],
                ),
            }),
        )
            .into_response(),
    }
}

pub async fn handle_skillhub_showcase(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<ShowcaseQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let category = params.cat.trim().to_string();
    if category.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-query-param-required",
                    &[("param", "cat")],
                ),
            }),
        )
            .into_response();
    }

    let api = zeroclaw_skillhub::ApiClient::new();
    match api.showcase(&category).await {
        Ok(resp) => Json(resp).into_response(),
        Err(e) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-showcase-failed",
                    &[("error", &format!("{e}"))],
                ),
            }),
        )
            .into_response(),
    }
}

pub async fn handle_skillhub_install(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<InstallBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let source = body.source.trim().to_string();
    if source.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-field-required",
                    &[("field", "source")],
                ),
            }),
        )
            .into_response();
    }

    let force = body.force.unwrap_or(false);
    let timeout = body.timeout.unwrap_or(30);
    let workspace_dir = state.config.lock().workspace_dir.clone();
    let api = zeroclaw_skillhub::ApiClient::new();
    let install_source = source.clone();

    let result = tokio::task::spawn(async move {
        let workspace = std::path::Path::new(&workspace_dir);
        zeroclaw_skillhub::commands::install::run(&api, workspace, &install_source, force, timeout)
            .await
    })
    .await;

    match result {
        Ok(Ok(())) => {
            let slug = source
                .strip_prefix("clawhub:")
                .unwrap_or(&source)
                .trim()
                .to_string();
            (
                StatusCode::OK,
                Json(SkillhubSuccess {
                    success: true,
                    slug: Some(slug),
                    message: kylinbot_runtime::i18n::get_required_cli_string(
                        "skillhub-installed-successfully",
                    ),
                }),
            )
                .into_response()
        }
        Ok(Err(e)) => {
            let msg = format!("{e}");
            let status = if msg.contains("already exists") || msg.contains("Already installed") {
                StatusCode::CONFLICT
            } else {
                StatusCode::INTERNAL_SERVER_ERROR
            };
            (
                status,
                Json(SkillhubError {
                    success: false,
                    message: msg,
                }),
            )
                .into_response()
        }
        Err(_) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string(
                    "skillhub-install-task-failed",
                ),
            }),
        )
            .into_response(),
    }
}

pub async fn handle_skillhub_list(
    State(state): State<AppState>,
    headers: HeaderMap,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let workspace_dir = &state.config.lock().workspace_dir;
    let skills_dir = zeroclaw_skillhub::config::skills_install_dir(workspace_dir);
    let mut lock = zeroclaw_skillhub::lockfile::load_lockfile(&skills_dir);

    let stale: Vec<String> = lock
        .skills
        .keys()
        .filter(|slug| !skills_dir.join(slug).exists())
        .cloned()
        .collect();

    if !stale.is_empty() {
        for slug in &stale {
            lock.skills.remove(slug);
        }
        let _ = zeroclaw_skillhub::lockfile::save_lockfile(&skills_dir, &lock);
    }

    let mut skills: Vec<_> = lock.skills.into_values().collect();
    skills.sort_by(|a, b| a.slug.cmp(&b.slug));

    Json(serde_json::json!({ "skills": skills })).into_response()
}

pub async fn handle_skillhub_upgrade(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<UpgradeBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let check_only = body.check_only.unwrap_or(false);
    let timeout = body.timeout.unwrap_or(30);
    let workspace_dir = state.config.lock().workspace_dir.clone();
    let api = zeroclaw_skillhub::ApiClient::new();

    let result = tokio::task::spawn(async move {
        let workspace = std::path::Path::new(&workspace_dir);
        zeroclaw_skillhub::commands::upgrade::run(
            &api,
            workspace,
            body.slug.as_deref(),
            check_only,
            timeout,
        )
        .await
    })
    .await;

    match result {
        Ok(Ok(())) => (
            StatusCode::OK,
            Json(SkillhubSuccess {
                success: true,
                slug: None,
                message: if check_only {
                    kylinbot_runtime::i18n::get_required_cli_string("skillhub-check-complete")
                } else {
                    kylinbot_runtime::i18n::get_required_cli_string("skillhub-upgrade-complete")
                },
            }),
        )
            .into_response(),
        Ok(Err(e)) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-upgrade-failed",
                    &[("error", &format!("{e}"))],
                ),
            }),
        )
            .into_response(),
        Err(_) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string(
                    "skillhub-upgrade-task-failed",
                ),
            }),
        )
            .into_response(),
    }
}

pub async fn handle_skillhub_detail(
    State(state): State<AppState>,
    headers: HeaderMap,
    Query(params): Query<DetailQuery>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let slug = params.slug.trim().to_string();
    if slug.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-query-param-required",
                    &[("param", "slug")],
                ),
            }),
        )
            .into_response();
    }

    let api = zeroclaw_skillhub::ApiClient::new();
    match api.detail(&slug).await {
        Ok(detail) => Json(detail).into_response(),
        Err(e) => {
            let msg = format!("{e}");
            let status = if msg.contains("404") {
                StatusCode::NOT_FOUND
            } else {
                StatusCode::INTERNAL_SERVER_ERROR
            };
            (
                status,
                Json(SkillhubError {
                    success: false,
                    message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                        "skillhub-detail-failed",
                        &[("error", &format!("{e}"))],
                    ),
                }),
            )
                .into_response()
        }
    }
}

pub async fn handle_skillhub_uninstall(
    State(state): State<AppState>,
    headers: HeaderMap,
    Json(body): Json<UninstallBody>,
) -> impl IntoResponse {
    if let Err(e) = require_auth(&state, &headers) {
        return e.into_response();
    }

    let slug = body.slug.trim().to_string();
    if slug.is_empty() {
        return (
            StatusCode::BAD_REQUEST,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string_with_args(
                    "skillhub-field-required",
                    &[("field", "slug")],
                ),
            }),
        )
            .into_response();
    }

    if slug.contains("..") || slug.contains('/') || slug.contains('\\') {
        return (
            StatusCode::BAD_REQUEST,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string(
                    "skillhub-invalid-skill-slug",
                ),
            }),
        )
            .into_response();
    }

    let workspace_dir = state.config.lock().workspace_dir.clone();
    let slug_clone = slug.clone();
    let result = tokio::task::spawn_blocking(move || {
        zeroclaw_skillhub::commands::uninstall::run(
            std::path::Path::new(&workspace_dir),
            &slug_clone,
        )
    })
    .await;

    match result {
        Ok(Ok(())) => (
            StatusCode::OK,
            Json(SkillhubSuccess {
                success: true,
                slug: Some(slug),
                message: kylinbot_runtime::i18n::get_required_cli_string(
                    "skillhub-uninstalled-successfully",
                ),
            }),
        )
            .into_response(),
        Ok(Err(e)) => {
            let msg = format!("{e}");
            let status = if msg.contains("not found") {
                StatusCode::NOT_FOUND
            } else {
                StatusCode::INTERNAL_SERVER_ERROR
            };
            (
                status,
                Json(SkillhubError {
                    success: false,
                    message: msg,
                }),
            )
                .into_response()
        }
        Err(_) => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(SkillhubError {
                success: false,
                message: kylinbot_runtime::i18n::get_required_cli_string(
                    "skillhub-uninstall-task-failed",
                ),
            }),
        )
            .into_response(),
    }
}
