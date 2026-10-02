//! KylinBot Desktop — main entry point.

#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod commands;
mod gateway_client;
mod gateway_spawn;
mod health;
mod state;

use commands::{agent, channels, gateway, pairing};
use gateway_client::GatewayClient;
use state::shared_state;
use tauri::Manager;

const DEFAULT_GATEWAY_URL: &str = "http://127.0.0.1:42617";

/// Escape a string for safe injection into a JavaScript single-quoted literal.
fn js_escape(s: &str) -> String {
    s.replace('\\', "\\\\").replace('\'', "\\'")
}

/// Inject a bearer token into the WebView's localStorage so the React frontend
/// skips the pairing dialog. Uses the standard Tauri Webview scripting API.
fn inject_token_into_webview(window: &tauri::Window, token: &str) {
    let escaped = js_escape(token);
    let script = format!("localStorage.setItem('kylinbot_token', '{}')", escaped);
    let _ = window.eval(&script);
}

fn main() {
    // Disable GPU acceleration for systems with WebView/display driver incompatibilities
    // (e.g. Huawei Kylin OS with certain GPU drivers).
    std::env::set_var("LIBGL_ALWAYS_SOFTWARE", "1");
    std::env::set_var("WEBKIT_DISABLE_COMPOSITING_MODE", "1");

    let state = shared_state();
    let app = tauri::Builder::default()
        .manage(state.clone())
        .invoke_handler(tauri::generate_handler![
            gateway::get_app_token,
            gateway::get_status,
            gateway::get_health,
            agent::send_message,
            channels::list_channels,
            pairing::initiate_pairing,
            pairing::get_devices,
        ])
        .setup(move |app_handle| {
            let pair_state = state.clone();

            // Run auto-pairing synchronously in setup so `app_handle` (non-Send)
            // never escapes this closure. Use Tauri's async runtime because this
            // async flow relies on Tokio primitives (sleep/RwLock/reqwest).
            let pairing_result = tauri::async_runtime::block_on(async {
                gateway_spawn::ensure_local_gateway_running().await;

                let client = GatewayClient::new(DEFAULT_GATEWAY_URL, None);

                let require_pair = match client.requires_pairing().await {
                    Ok(v) => {
                        println!(
                            "kylinbot-desktop: requires_pairing = {}",
                            v
                        );
                        v
                    }
                    Err(e) => {
                        eprintln!(
                            "kylinbot-desktop: requires_pairing check failed: {}",
                            e
                        );
                        false
                    }
                };

                if require_pair {
                    println!("kylinbot-desktop: auto-pairing starting...");
                    match client.auto_pair().await {
                        Ok(token) => {
                            println!(
                                "kylinbot-desktop: auto-pairing succeeded, token prefix: {}",
                                &token[..20.min(token.len())]
                            );
                            {
                                let mut s = pair_state.write().await;
                                s.token = Some(token.clone());
                            }
                            // Inject the token into the WebView so the frontend skips the
                            // pairing dialog. This must happen here (not in a spawned task)
                            // because `app_handle` is only valid within this closure.
                            if let Some(window) = app_handle.get_window("main") {
                                inject_token_into_webview(&window, &token);
                            }
                            Ok(())
                        }
                        Err(e) => {
                            eprintln!(
                                "kylinbot-desktop: auto-pairing failed: {}",
                                e
                            );
                            Err(e)
                        }
                    }
                } else {
                    println!(
                        "kylinbot-desktop: require_pairing is false, skipping auto-pair"
                    );
                    Ok(())
                }
            });

            if let Err(e) = pairing_result {
                eprintln!(
                    "kylinbot-desktop: pairing setup failed (continuing anyway): {}",
                    e
                );
            }

            // Health poller can run in a spawned task — `pair_state` is Arc<RwLock> (Send).
            tauri::async_runtime::spawn(async move {
                health::spawn_health_poller(pair_state);
            });

            Ok(())
        })
        .build(tauri::generate_context!())
        .expect("error while building tauri application");

    app.run(|_app_handle, _event| {});
}
