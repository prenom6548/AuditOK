use crate::gateway_client::GatewayClient;
use crate::state::SharedState;
use tauri::State;

/// Return the token stored in Rust app state (set by auto-pairing on startup).
/// Returns null if no token is stored yet.
#[tauri::command]
pub async fn get_app_token(state: State<'_, SharedState>) -> Result<Option<String>, String> {
    let s = state.read().await;
    Ok(s.token.clone())
}

#[tauri::command]
pub async fn get_status(state: State<'_, SharedState>) -> Result<serde_json::Value, String> {
    let s = state.read().await;
    let client = GatewayClient::new(&s.gateway_url, s.token.as_deref());
    drop(s);
    client.get_status().await.map_err(|e| e.to_string())
}

#[tauri::command]
pub async fn get_health(state: State<'_, SharedState>) -> Result<bool, String> {
    let s = state.read().await;
    let client = GatewayClient::new(&s.gateway_url, s.token.as_deref());
    drop(s);
    client.get_health().await.map_err(|e| e.to_string())
}
