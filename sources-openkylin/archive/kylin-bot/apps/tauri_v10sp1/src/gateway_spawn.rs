//! Spawn `kylinbot gateway` when the desktop starts if nothing is listening yet, and stop the
//! child process on application exit when we started it (do not kill an independently launched
//! gateway).

use crate::gateway_client::GatewayClient;
use std::process::{Child, Command, Stdio};
use std::sync::{Mutex, OnceLock};
use std::time::Duration;

const DEFAULT_GATEWAY_URL: &str = "http://127.0.0.1:42617";

static GATEWAY_CHILD: OnceLock<Mutex<Option<Child>>> = OnceLock::new();

fn gateway_child_slot() -> &'static Mutex<Option<Child>> {
    GATEWAY_CHILD.get_or_init(|| Mutex::new(None))
}

fn kylinbot_binary_path() -> std::path::PathBuf {
    if let Ok(exe) = std::env::current_exe() {
        if let Some(dir) = exe.parent() {
            #[cfg(windows)]
            let candidate = dir.join("kylinbot.exe");
            #[cfg(not(windows))]
            let candidate = dir.join("kylinbot");
            if candidate.is_file() {
                return candidate;
            }
        }
    }
    #[cfg(windows)]
    {
        std::path::PathBuf::from("kylinbot.exe")
    }
    #[cfg(not(windows))]
    {
        std::path::PathBuf::from("kylinbot")
    }
}

/// If the gateway is not reachable, start `kylinbot gateway start` as a child process and
/// poll `/health` until it succeeds or a timeout elapses.
///
/// The child is stored so [`stop_local_gateway_if_spawned`] can terminate it.
pub async fn ensure_local_gateway_running() {
    let client = GatewayClient::new(DEFAULT_GATEWAY_URL, None);
    if client.get_health().await.unwrap_or(false) {
        return;
    }

    println!(
        "kylinbot-desktop: gateway not reachable at {}; spawning `kylinbot daemon`",
        DEFAULT_GATEWAY_URL
    );

    let bin = kylinbot_binary_path();
    let bin_display = bin.display().to_string();

    let child = match Command::new(&bin)
        .args(["daemon"])
        .stdin(Stdio::null())
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
    {
        Ok(c) => c,
        Err(e) => {
            eprintln!(
                "kylinbot-desktop: failed to spawn `{} daemon`: {}",
                bin_display, e
            );
            return;
        }
    };

    {
        let mut slot = gateway_child_slot()
            .lock()
            .expect("gateway child mutex poisoned");
        *slot = Some(child);
    }

    for _ in 0..60 {
        tokio::time::sleep(Duration::from_millis(500)).await;
        if GatewayClient::new(DEFAULT_GATEWAY_URL, None)
            .get_health()
            .await
            .unwrap_or(false)
        {
            println!(
                "kylinbot-desktop: gateway is healthy at {}",
                DEFAULT_GATEWAY_URL
            );
            return;
        }
    }

    eprintln!(
        "kylinbot-desktop: gateway did not become healthy within 30s after spawn (binary={})",
        bin_display
    );
}

/// Kill the gateway process if this desktop session started it.
pub fn stop_local_gateway_if_spawned() {
    let mut slot = match gateway_child_slot().lock() {
        Ok(g) => g,
        Err(e) => {
            eprintln!("kylinbot-desktop: gateway child mutex poisoned: {}", e);
            return;
        }
    };

    if let Some(mut child) = slot.take() {
        if let Err(e) = child.kill() {
            eprintln!("kylinbot-desktop: failed to stop gateway child: {}", e);
        } else {
            let _ = child.wait();
            println!("kylinbot-desktop: stopped locally spawned gateway");
        }
    }
}
