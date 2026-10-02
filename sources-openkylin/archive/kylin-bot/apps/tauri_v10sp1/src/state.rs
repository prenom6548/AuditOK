//! Shared application state for Tauri.

use std::sync::Arc;
use tokio::sync::RwLock;

/// High-level agent activity used by the tray icon state.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum AgentStatus {
    #[default]
    Idle,
    Working,
    Error,
}

/// Shared application state behind an `Arc<RwLock<_>>`.
#[derive(Debug, Clone)]
pub struct AppState {
    pub gateway_url: String,
    pub token: Option<String>,
    pub connected: bool,
    pub agent_status: AgentStatus,
}

impl Default for AppState {
    fn default() -> Self {
        Self {
            gateway_url: "http://127.0.0.1:42617".to_string(),
            token: None,
            connected: false,
            agent_status: AgentStatus::Idle,
        }
    }
}

/// Thread-safe wrapper around `AppState`.
pub type SharedState = Arc<RwLock<AppState>>;

/// Create the default shared state.
pub fn shared_state() -> SharedState {
    Arc::new(RwLock::new(AppState::default()))
}
