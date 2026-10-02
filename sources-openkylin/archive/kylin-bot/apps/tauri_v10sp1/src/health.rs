//! Background health polling for the KylinBot gateway.

use crate::gateway_client::GatewayClient;
use crate::state::SharedState;
use std::time::Duration;

const POLL_INTERVAL: Duration = Duration::from_secs(5);

/// Spawn a background task that polls gateway health and updates state.
pub fn spawn_health_poller(state: SharedState) {
    tauri::async_runtime::spawn(async move {
        loop {
            let (url, token) = {
                let s = state.read().await;
                (s.gateway_url.clone(), s.token.clone())
            };

            let client = GatewayClient::new(&url, token.as_deref());
            let healthy = client.get_health().await.unwrap_or(false);

            {
                let mut s = state.write().await;
                s.connected = healthy;
            }

            tokio::time::sleep(POLL_INTERVAL).await;
        }
    });
}
