use anyhow::Result;
use chrono::Utc;
use std::collections::HashMap;
use std::future::Future;
use std::path::PathBuf;
use std::sync::Arc;
use tokio::task::JoinHandle;
use tokio::time::Duration;
use tokio_util::sync::CancellationToken;
use kylinbot_config::schema::Config;

pub mod reload;
pub mod worker_pool;

pub use reload::{ReloadRequest, Subsystem, diff_subsystems};
pub use worker_pool::ChannelWorkerPool;

const STATUS_FLUSH_SECONDS: u64 = 5;
/// Maximum time we wait for a subsystem to exit gracefully after cancellation
/// before aborting its task handle.
const SUBSYSTEM_SHUTDOWN_GRACE: Duration = Duration::from_secs(5);
/// Bound on the daemon-level worker pool shared across reloads. Bounded only
/// for memory safety; actual rate-limiting happens in the channel dispatcher.
const DEFAULT_WORKER_POOL_CAPACITY: usize = 256;
/// Maximum time we wait for in-flight channel workers to finish at shutdown
/// before forcibly exiting the daemon process.
const WORKER_POOL_DRAIN_TIMEOUT: Duration = Duration::from_secs(30);
/// Capacity of the reload request channel. Reloads coalesce naturally because
/// each `ReloadRequest` carries a full `affected` set; a small buffer is enough
/// to absorb back-to-back saves without dropping requests.
const RELOAD_CHANNEL_CAPACITY: usize = 16;

/// Why the daemon's main loop returned.
///
/// Only `Shutdown` is produced now — reloads are handled entirely inside
/// `daemon::run` via the [`ReloadRequest`] mpsc channel, with in-flight
/// agent inference preserved via the daemon-level [`ChannelWorkerPool`].
/// The `Reload` variant is retained for source-compatibility with callers
/// that pattern-match on it, but the runtime never returns it.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DaemonExit {
    Shutdown,
    Reload,
}

/// Arguments passed to each subsystem start function.
///
/// `cancel_token` is honored by subsystems that need to perform explicit
/// cleanup before exit (e.g. closing channel WebSocket listeners). Subsystems
/// that don't need cleanup may ignore it — the supervisor will still cancel
/// the task handle and respawn with the new config.
pub struct SubsystemSpawnArgs {
    pub config: Config,
    pub host: String,
    pub port: u16,
    pub worker_pool: Arc<ChannelWorkerPool>,
    pub cancel_token: CancellationToken,
    pub event_tx: tokio::sync::broadcast::Sender<serde_json::Value>,
    pub reload_tx: tokio::sync::mpsc::Sender<ReloadRequest>,
}

#[allow(clippy::type_complexity)]
pub type SubsystemStartFn = Arc<
    dyn Fn(SubsystemSpawnArgs) -> std::pin::Pin<Box<dyn Future<Output = Result<()>> + Send>>
        + Send
        + Sync,
>;

/// Wait for an OS shutdown signal (SIGINT / SIGTERM / Ctrl+C). SIGHUP is
/// ignored on Unix so the daemon survives terminal / SSH disconnects.
///
/// Returns when one of the shutdown signals is observed; never returns for
/// reload — reloads are signaled via the `mpsc<ReloadRequest>` channel
/// consumed inside [`run`].
async fn wait_for_shutdown_signal() -> Result<()> {
    #[cfg(unix)]
    {
        use tokio::signal::unix::{SignalKind, signal};

        let mut sigint = signal(SignalKind::interrupt())?;
        let mut sigterm = signal(SignalKind::terminate())?;
        let mut sighup = signal(SignalKind::hangup())?;

        loop {
            tokio::select! {
                _ = sigint.recv() => {
                    tracing::info!("Received SIGINT, shutting down...");
                    return Ok(());
                }
                _ = sigterm.recv() => {
                    tracing::info!("Received SIGTERM, shutting down...");
                    return Ok(());
                }
                _ = sighup.recv() => {
                    tracing::info!("Received SIGHUP, ignoring (daemon stays running)");
                }
            }
        }
    }

    #[cfg(not(unix))]
    {
        tokio::signal::ctrl_c().await?;
        tracing::info!("Received Ctrl+C, shutting down...");
        Ok(())
    }
}

/// Optional subsystem start functions injected by the binary crate.
/// This allows the daemon to spawn subsystems without depending on their crates.
///
/// Each start fn accepts a [`SubsystemSpawnArgs`] bundle. The bundle carries
/// the cancel token the subsystem should observe to exit gracefully — listeners
/// closed, sockets shut down — without disturbing the shared
/// [`ChannelWorkerPool`].
#[allow(clippy::type_complexity)]
#[derive(Default)]
pub struct DaemonSubsystems {
    pub gateway_start: Option<SubsystemStartFn>,
    pub channels_start: Option<SubsystemStartFn>,
    pub mqtt_start: Option<SubsystemStartFn>,
}

pub async fn run(
    config: Config,
    host: String,
    port: u16,
    subsystems: DaemonSubsystems,
) -> Result<DaemonExit> {
    let initial_backoff = config.reliability.channel_initial_backoff_secs.max(1);
    let max_backoff = config
        .reliability
        .channel_max_backoff_secs
        .max(initial_backoff);

    crate::health::mark_component_ok("daemon");

    // Shared broadcast channel so all daemon components (gateway, cron,
    // heartbeat) can publish real-time events to dashboard clients.
    let (event_tx, _rx) = tokio::sync::broadcast::channel::<serde_json::Value>(256);

    if config.heartbeat.enabled {
        let _ =
            crate::heartbeat::engine::HeartbeatEngine::ensure_heartbeat_file(&config.workspace_dir)
                .await;
    }

    // Daemon-level worker pool. Lives for the entire `run()` invocation and
    // is shared with every channels supervisor (including respawns), so
    // in-flight agent inference survives channel reloads untouched.
    let worker_pool = Arc::new(ChannelWorkerPool::new(DEFAULT_WORKER_POOL_CAPACITY));

    // Reload channel: gateway endpoints push `ReloadRequest`s here after
    // computing the diff. We consume them in the main select loop below and
    // only restart the subsystems listed in `affected`.
    let (reload_tx, mut reload_rx) =
        tokio::sync::mpsc::channel::<ReloadRequest>(RELOAD_CHANNEL_CAPACITY);

    // Tasks not subject to selective reload (state writer survives forever).
    let singleton_handles: Vec<JoinHandle<()>> = vec![spawn_state_writer(config.clone())];

    // Swappable subsystems, keyed by Subsystem so reload can replace exactly
    // the ones whose config changed.
    let mut subsystem_handles: HashMap<Subsystem, (CancellationToken, JoinHandle<()>)> =
        HashMap::new();
    let subsystems = Arc::new(subsystems);
    let mut current_config = config;

    // Spawn each initially-enabled subsystem.
    spawn_initial_subsystems(
        &mut subsystem_handles,
        &subsystems,
        &current_config,
        &host,
        port,
        worker_pool.clone(),
        event_tx.clone(),
        reload_tx.clone(),
        initial_backoff,
        max_backoff,
    );

    println!("🧠 KylinBot daemon started");
    println!("   Gateway:  http://{host}:{port}");
    println!("   Components: gateway, channels, heartbeat, scheduler");
    if current_config.gateway.require_pairing {
        println!("   Pairing:    enabled (code appears in gateway output above)");
    }
    println!("   Ctrl+C or SIGTERM to stop");

    // Main loop: serve OS shutdown signals and reload requests concurrently.
    // Reloads only restart `affected` subsystems and leave the worker pool
    // (and hence in-flight inference) running.
    loop {
        tokio::select! {
            _ = wait_for_shutdown_signal() => {
                break;
            }
            Some(req) = reload_rx.recv() => {
                tracing::info!(
                    "daemon reload requested: affected = {:?}",
                    req.affected.iter().map(|s| s.as_str()).collect::<Vec<_>>()
                );
                apply_reload(
                    &mut subsystem_handles,
                    &subsystems,
                    &req,
                    &host,
                    port,
                    worker_pool.clone(),
                    event_tx.clone(),
                    reload_tx.clone(),
                    initial_backoff,
                    max_backoff,
                ).await;
                current_config = req.new_config.clone();
                tracing::info!("daemon reload applied; current_config refreshed");
                // Suppress unused warning when reload payload didn't change
                // anything we surface elsewhere.
                let _ = &current_config;
            }
        }
    }

    crate::health::mark_component_error("daemon", "shutdown requested");

    // Graceful shutdown: cancel every subsystem, wait briefly for cleanup,
    // then drain in-flight workers.
    for (_, (token, _)) in &subsystem_handles {
        token.cancel();
    }
    for (subsystem, (_token, handle)) in subsystem_handles.drain() {
        match tokio::time::timeout(SUBSYSTEM_SHUTDOWN_GRACE, handle).await {
            Ok(Ok(())) | Ok(Err(_)) => {}
            Err(_) => tracing::warn!(
                "subsystem '{}' did not exit within grace period; abandoning task",
                subsystem.as_str()
            ),
        }
    }
    for h in &singleton_handles {
        h.abort();
    }
    for h in singleton_handles {
        let _ = h.await;
    }
    if !worker_pool.drain_with_timeout(WORKER_POOL_DRAIN_TIMEOUT).await {
        tracing::warn!(
            "{} in-flight channel worker(s) did not finish within {:?}; exiting anyway",
            worker_pool.active(),
            WORKER_POOL_DRAIN_TIMEOUT
        );
    }

    Ok(DaemonExit::Shutdown)
}

/// Subsystems that should be spawned for `config`.
fn enabled_subsystems(config: &Config, subsystems: &DaemonSubsystems) -> Vec<Subsystem> {
    let mut out = Vec::new();
    if subsystems.gateway_start.is_some() {
        out.push(Subsystem::Gateway);
    }
    if subsystems.channels_start.is_some() && has_supervised_channels(config) {
        out.push(Subsystem::Channels);
    }
    if let Some(mqtt) = config.channels.mqtt.as_ref() {
        if mqtt.enabled && subsystems.mqtt_start.is_some() {
            out.push(Subsystem::Mqtt);
        }
    }
    if config.heartbeat.enabled {
        out.push(Subsystem::Heartbeat);
    }
    if config.cron.enabled {
        out.push(Subsystem::Cron);
    }
    out
}

#[allow(clippy::too_many_arguments)]
fn spawn_initial_subsystems(
    handles: &mut HashMap<Subsystem, (CancellationToken, JoinHandle<()>)>,
    subsystems: &Arc<DaemonSubsystems>,
    config: &Config,
    host: &str,
    port: u16,
    worker_pool: Arc<ChannelWorkerPool>,
    event_tx: tokio::sync::broadcast::Sender<serde_json::Value>,
    reload_tx: tokio::sync::mpsc::Sender<ReloadRequest>,
    initial_backoff: u64,
    max_backoff: u64,
) {
    for subsystem in enabled_subsystems(config, subsystems) {
        let token = CancellationToken::new();
        if let Some(handle) = spawn_subsystem(
            subsystem,
            subsystems,
            config.clone(),
            host.to_string(),
            port,
            worker_pool.clone(),
            event_tx.clone(),
            reload_tx.clone(),
            token.clone(),
            initial_backoff,
            max_backoff,
        ) {
            handles.insert(subsystem, (token, handle));
        }
    }
    // Mark not-enabled subsystems as healthy-but-disabled for observability.
    for subsystem in [
        Subsystem::Channels,
        Subsystem::Mqtt,
        Subsystem::Heartbeat,
        Subsystem::Cron,
    ] {
        if !handles.contains_key(&subsystem) {
            crate::health::mark_component_ok(subsystem.as_str());
        }
    }
}

#[allow(clippy::too_many_arguments)]
async fn apply_reload(
    handles: &mut HashMap<Subsystem, (CancellationToken, JoinHandle<()>)>,
    subsystems: &Arc<DaemonSubsystems>,
    req: &ReloadRequest,
    host: &str,
    port: u16,
    worker_pool: Arc<ChannelWorkerPool>,
    event_tx: tokio::sync::broadcast::Sender<serde_json::Value>,
    reload_tx: tokio::sync::mpsc::Sender<ReloadRequest>,
    initial_backoff: u64,
    max_backoff: u64,
) {
    // Compute the post-reload enabled set so we can also stop subsystems that
    // got disabled (e.g. cron.enabled flipped off).
    let new_enabled: std::collections::HashSet<Subsystem> = enabled_subsystems(&req.new_config, subsystems)
        .into_iter()
        .collect();

    for subsystem in &req.affected {
        let subsystem = *subsystem;
        if let Some((token, handle)) = handles.remove(&subsystem) {
            token.cancel();
            match tokio::time::timeout(SUBSYSTEM_SHUTDOWN_GRACE, handle).await {
                Ok(_) => {}
                Err(_) => tracing::warn!(
                    "subsystem '{}' did not exit within grace; respawning anyway",
                    subsystem.as_str()
                ),
            }
        }
        if new_enabled.contains(&subsystem) {
            let new_token = CancellationToken::new();
            if let Some(new_handle) = spawn_subsystem(
                subsystem,
                subsystems,
                req.new_config.clone(),
                host.to_string(),
                port,
                worker_pool.clone(),
                event_tx.clone(),
                reload_tx.clone(),
                new_token.clone(),
                initial_backoff,
                max_backoff,
            ) {
                handles.insert(subsystem, (new_token, new_handle));
                tracing::info!("subsystem '{}' respawned after reload", subsystem.as_str());
            }
        } else {
            tracing::info!(
                "subsystem '{}' disabled by new config; not respawning",
                subsystem.as_str()
            );
            crate::health::mark_component_ok(subsystem.as_str());
        }
    }
}

#[allow(clippy::too_many_arguments)]
fn spawn_subsystem(
    subsystem: Subsystem,
    subsystems: &Arc<DaemonSubsystems>,
    config: Config,
    host: String,
    port: u16,
    worker_pool: Arc<ChannelWorkerPool>,
    event_tx: tokio::sync::broadcast::Sender<serde_json::Value>,
    reload_tx: tokio::sync::mpsc::Sender<ReloadRequest>,
    cancel_token: CancellationToken,
    initial_backoff: u64,
    max_backoff: u64,
) -> Option<JoinHandle<()>> {
    let name = subsystem.as_str();
    let make_args = move |config: Config,
                          cancel_token: CancellationToken,
                          host: String,
                          worker_pool: Arc<ChannelWorkerPool>,
                          event_tx: tokio::sync::broadcast::Sender<serde_json::Value>,
                          reload_tx: tokio::sync::mpsc::Sender<ReloadRequest>|
          -> SubsystemSpawnArgs {
        SubsystemSpawnArgs {
            config,
            host,
            port,
            worker_pool,
            cancel_token,
            event_tx,
            reload_tx,
        }
    };

    match subsystem {
        Subsystem::Gateway => {
            let start = subsystems.gateway_start.clone()?;
            let cfg = config.clone();
            let token = cancel_token.clone();
            let host_c = host.clone();
            let worker_pool_c = worker_pool.clone();
            let event_tx_c = event_tx.clone();
            let reload_tx_c = reload_tx.clone();
            let make_args = make_args.clone();
            Some(spawn_component_supervisor(
                name,
                initial_backoff,
                max_backoff,
                cancel_token,
                move || {
                    let args = make_args(
                        cfg.clone(),
                        token.clone(),
                        host_c.clone(),
                        worker_pool_c.clone(),
                        event_tx_c.clone(),
                        reload_tx_c.clone(),
                    );
                    let start = start.clone();
                    async move { start(args).await }
                },
            ))
        }
        Subsystem::Channels => {
            let start = subsystems.channels_start.clone()?;
            let cfg = config.clone();
            let token = cancel_token.clone();
            let host_c = host.clone();
            let worker_pool_c = worker_pool.clone();
            let event_tx_c = event_tx.clone();
            let reload_tx_c = reload_tx.clone();
            let make_args = make_args.clone();
            Some(spawn_component_supervisor(
                name,
                initial_backoff,
                max_backoff,
                cancel_token,
                move || {
                    let args = make_args(
                        cfg.clone(),
                        token.clone(),
                        host_c.clone(),
                        worker_pool_c.clone(),
                        event_tx_c.clone(),
                        reload_tx_c.clone(),
                    );
                    let start = start.clone();
                    async move { start(args).await }
                },
            ))
        }
        Subsystem::Mqtt => {
            let start = subsystems.mqtt_start.clone()?;
            let cfg = config.clone();
            let token = cancel_token.clone();
            let host_c = host.clone();
            let worker_pool_c = worker_pool.clone();
            let event_tx_c = event_tx.clone();
            let reload_tx_c = reload_tx.clone();
            let make_args = make_args.clone();
            Some(spawn_component_supervisor(
                name,
                initial_backoff,
                max_backoff,
                cancel_token,
                move || {
                    let args = make_args(
                        cfg.clone(),
                        token.clone(),
                        host_c.clone(),
                        worker_pool_c.clone(),
                        event_tx_c.clone(),
                        reload_tx_c.clone(),
                    );
                    let start = start.clone();
                    async move { start(args).await }
                },
            ))
        }
        Subsystem::Heartbeat => {
            let cfg = config.clone();
            Some(spawn_component_supervisor(
                name,
                initial_backoff,
                max_backoff,
                cancel_token,
                move || {
                    let cfg = cfg.clone();
                    async move { Box::pin(run_heartbeat_worker(cfg)).await }
                },
            ))
        }
        Subsystem::Cron => {
            let cfg = config.clone();
            let tx = event_tx.clone();
            Some(spawn_component_supervisor(
                name,
                initial_backoff,
                max_backoff,
                cancel_token,
                move || {
                    let cfg = cfg.clone();
                    let tx = tx.clone();
                    async move { Box::pin(crate::cron::scheduler::run(cfg, Some(tx))).await }
                },
            ))
        }
    }
}

pub fn state_file_path(config: &Config) -> PathBuf {
    config
        .config_path
        .parent()
        .map_or_else(|| PathBuf::from("."), PathBuf::from)
        .join("daemon_state.json")
}

fn spawn_state_writer(config: Config) -> JoinHandle<()> {
    tokio::spawn(async move {
        let path = state_file_path(&config);
        if let Some(parent) = path.parent() {
            let _ = tokio::fs::create_dir_all(parent).await;
        }

        let mut interval = tokio::time::interval(Duration::from_secs(STATUS_FLUSH_SECONDS));
        loop {
            interval.tick().await;
            let mut json = crate::health::snapshot_json();
            if let Some(obj) = json.as_object_mut() {
                obj.insert(
                    "written_at".into(),
                    serde_json::json!(Utc::now().to_rfc3339()),
                );
            }
            let data = serde_json::to_vec_pretty(&json).unwrap_or_else(|_| b"{}".to_vec());
            let _ = tokio::fs::write(&path, data).await;
        }
    })
}

fn spawn_component_supervisor<F, Fut>(
    name: &'static str,
    initial_backoff_secs: u64,
    max_backoff_secs: u64,
    cancel_token: CancellationToken,
    mut run_component: F,
) -> JoinHandle<()>
where
    F: FnMut() -> Fut + Send + 'static,
    Fut: Future<Output = Result<()>> + Send + 'static,
{
    tokio::spawn(async move {
        let mut backoff = initial_backoff_secs.max(1);
        let max_backoff = max_backoff_secs.max(backoff);

        loop {
            if cancel_token.is_cancelled() {
                tracing::debug!("Daemon component '{name}' supervisor: cancelled, exiting");
                return;
            }
            crate::health::mark_component_ok(name);
            let component_future = run_component();
            tokio::select! {
                _ = cancel_token.cancelled() => {
                    tracing::debug!(
                        "Daemon component '{name}' supervisor: cancellation received while running; exiting"
                    );
                    return;
                }
                outcome = component_future => match outcome {
                    Ok(()) => {
                        crate::health::mark_component_error(name, "component exited unexpectedly");
                        tracing::warn!("Daemon component '{name}' exited unexpectedly");
                        // Clean exit — reset backoff since the component ran successfully
                        backoff = initial_backoff_secs.max(1);
                    }
                    Err(e) => {
                        crate::health::mark_component_error(name, e.to_string());
                        tracing::error!("Daemon component '{name}' failed: {e}");
                    }
                }
            }

            crate::health::bump_component_restart(name);
            tokio::select! {
                _ = cancel_token.cancelled() => {
                    tracing::debug!("Daemon component '{name}' supervisor: cancelled during backoff");
                    return;
                }
                _ = tokio::time::sleep(Duration::from_secs(backoff)) => {}
            }
            // Double backoff AFTER sleeping so first error uses initial_backoff
            backoff = backoff.saturating_mul(2).min(max_backoff);
        }
    })
}

async fn run_heartbeat_worker(config: Config) -> Result<()> {
    use crate::heartbeat::engine::{
        HeartbeatEngine, HeartbeatTask, TaskPriority, TaskStatus, compute_adaptive_interval,
    };
    use std::sync::Arc;

    let observer: std::sync::Arc<dyn crate::observability::Observer> =
        std::sync::Arc::from(crate::observability::create_observer(&config.observability));
    let engine = HeartbeatEngine::new(
        config.heartbeat.clone(),
        config.workspace_dir.clone(),
        observer,
    );
    let metrics = engine.metrics();
    let delivery = resolve_heartbeat_delivery(&config)?;
    let two_phase = config.heartbeat.two_phase;
    let adaptive = config.heartbeat.adaptive;
    let start_time = std::time::Instant::now();

    // ── Deadman watcher ──────────────────────────────────────────
    let deadman_timeout = config.heartbeat.deadman_timeout_minutes;
    if deadman_timeout > 0 {
        let dm_metrics = Arc::clone(&metrics);
        let dm_config = config.clone();
        let dm_delivery = delivery.clone();
        tokio::spawn(async move {
            let check_interval = Duration::from_secs(60);
            let timeout = chrono::Duration::minutes(i64::from(deadman_timeout));
            loop {
                tokio::time::sleep(check_interval).await;
                let last_tick = dm_metrics.lock().last_tick_at;
                if let Some(last) = last_tick
                    && chrono::Utc::now() - last > timeout
                {
                    let alert = format!(
                        "⚠️ Heartbeat dead-man's switch: no tick in {deadman_timeout} minutes"
                    );
                    let (channel, target) = if let Some(ch) = &dm_config.heartbeat.deadman_channel {
                        let to = dm_config
                            .heartbeat
                            .deadman_to
                            .as_deref()
                            .or(dm_config.heartbeat.to.as_deref())
                            .unwrap_or_default();
                        (ch.clone(), to.to_string())
                    } else if let Some((ch, to)) = &dm_delivery {
                        (ch.clone(), to.clone())
                    } else {
                        continue;
                    };
                    let delivery_fut = crate::cron::scheduler::deliver_announcement(
                        &dm_config, &channel, &target, &alert,
                    );
                    match tokio::time::timeout(Duration::from_secs(30), delivery_fut).await {
                        Ok(Err(e)) => {
                            tracing::warn!("Deadman alert delivery failed: {e}");
                        }
                        Err(_) => {
                            tracing::warn!("Deadman alert delivery timed out (30s)");
                        }
                        Ok(Ok(())) => {}
                    }
                }
            }
        });
    }

    let base_interval = config.heartbeat.interval_minutes.max(1);
    let mut sleep_mins = base_interval;

    loop {
        tokio::time::sleep(Duration::from_secs(u64::from(sleep_mins) * 60)).await;

        // Update uptime
        {
            let mut m = metrics.lock();
            m.uptime_secs = start_time.elapsed().as_secs();
        }

        let tick_start = std::time::Instant::now();

        // Collect runnable tasks (active only, sorted by priority)
        let mut tasks = engine.collect_runnable_tasks().await?;
        let has_high_priority = tasks.iter().any(|t| t.priority == TaskPriority::High);

        if tasks.is_empty() {
            if let Some(fallback) = config
                .heartbeat
                .message
                .as_deref()
                .map(str::trim)
                .filter(|m| !m.is_empty())
            {
                tasks.push(HeartbeatTask {
                    text: fallback.to_string(),
                    priority: TaskPriority::Medium,
                    status: TaskStatus::Active,
                });
            } else {
                #[allow(clippy::cast_precision_loss)]
                let elapsed = tick_start.elapsed().as_millis() as f64;
                metrics.lock().record_success(elapsed);
                continue;
            }
        }

        // ── Phase 1: LLM decision (two-phase mode) ──────────────
        let tasks_to_run = if two_phase {
            let decision_prompt = format!(
                "[Heartbeat Task | decision] {}",
                HeartbeatEngine::build_decision_prompt(&tasks),
            );
            let phase1_fut = Box::pin(crate::agent::run(
                config.clone(),
                Some(decision_prompt),
                None,
                None,
                0.0,
                vec![],
                false,
                None,
                None,
            ));
            let phase1_result = if config.heartbeat.task_timeout_secs > 0 {
                match tokio::time::timeout(
                    Duration::from_secs(config.heartbeat.task_timeout_secs),
                    phase1_fut,
                )
                .await
                {
                    Ok(r) => r,
                    Err(_) => Err(anyhow::anyhow!(
                        "Phase 1 decision timed out ({}s)",
                        config.heartbeat.task_timeout_secs
                    )),
                }
            } else {
                phase1_fut.await
            };
            match phase1_result {
                Ok(output) => {
                    let response = output.text;
                    let indices = HeartbeatEngine::parse_decision_response(&response, tasks.len());
                    if indices.is_empty() {
                        tracing::info!("💓 Heartbeat Phase 1: skip (nothing to do)");
                        crate::health::mark_component_ok("heartbeat");
                        #[allow(clippy::cast_precision_loss)]
                        let elapsed = tick_start.elapsed().as_millis() as f64;
                        metrics.lock().record_success(elapsed);
                        continue;
                    }
                    tracing::info!(
                        "💓 Heartbeat Phase 1: run {} of {} tasks",
                        indices.len(),
                        tasks.len()
                    );
                    indices
                        .into_iter()
                        .filter_map(|i| tasks.get(i).cloned())
                        .collect()
                }
                Err(e) => {
                    tracing::warn!("💓 Heartbeat Phase 1 failed, running all tasks: {e}");
                    tasks
                }
            }
        } else {
            tasks
        };

        // ── Phase 2: Execute selected tasks ─────────────────────
        // Re-read session context on every tick so we pick up messages
        // that arrived since the daemon started.
        let session_context = if config.heartbeat.load_session_context {
            load_heartbeat_session_context(&config)
        } else {
            None
        };

        // Create memory once per tick for recall + consolidation.
        let heartbeat_memory: Option<Box<dyn kylinbot_memory::Memory>> =
            kylinbot_memory::create_memory(
                &config.memory,
                &config.workspace_dir,
                config
                    .providers
                    .fallback_provider()
                    .and_then(|e| e.api_key.as_deref()),
            )
            .ok();

        let mut tick_had_error = false;
        for task in &tasks_to_run {
            let task_start = std::time::Instant::now();
            let task_prompt = format!("[Heartbeat Task | {}] {}", task.priority, task.text);

            // Recall relevant memories so heartbeat tasks have context awareness.
            // Exclude `Conversation` memories to prevent chat context from
            // leaking into scheduled executions (see #5415).
            let memory_context = if let Some(ref mem) = heartbeat_memory {
                match mem.recall(&task.text, 5, None, None, None).await {
                    Ok(entries) if !entries.is_empty() => {
                        let ctx: String = entries
                            .iter()
                            .filter(|e| {
                                !matches!(
                                    e.category,
                                    kylinbot_memory::traits::MemoryCategory::Conversation
                                )
                            })
                            .map(|e| format!("- {}: {}", e.key, e.content))
                            .collect::<Vec<_>>()
                            .join("\n");
                        if ctx.is_empty() {
                            None
                        } else {
                            Some(format!("[Memory context]\n{ctx}\n[/Memory context]\n\n"))
                        }
                    }
                    _ => None,
                }
            } else {
                None
            };

            let prompt = match (&session_context, &memory_context) {
                (Some(sc), Some(mc)) => format!("{mc}\n{sc}\n\n{task_prompt}"),
                (Some(sc), None) => format!("{sc}\n\n{task_prompt}"),
                (None, Some(mc)) => format!("{mc}\n\n{task_prompt}"),
                (None, None) => task_prompt,
            };
            let temp = config
                .providers
                .fallback_provider()
                .and_then(|e| e.temperature)
                .unwrap_or(0.7);
            let phase2_fut = Box::pin(crate::agent::run(
                config.clone(),
                Some(prompt),
                None,
                None,
                temp,
                vec![],
                false,
                None,
                None,
            ));
            let phase2_result = if config.heartbeat.task_timeout_secs > 0 {
                match tokio::time::timeout(
                    Duration::from_secs(config.heartbeat.task_timeout_secs),
                    phase2_fut,
                )
                .await
                {
                    Ok(r) => r,
                    Err(_) => Err(anyhow::anyhow!(
                        "Heartbeat task timed out ({}s)",
                        config.heartbeat.task_timeout_secs
                    )),
                }
            } else {
                phase2_fut.await
            };
            match phase2_result {
                Ok(output) => {
                    let response = output.text;
                    crate::health::mark_component_ok("heartbeat");
                    #[allow(clippy::cast_possible_truncation)]
                    let duration_ms = task_start.elapsed().as_millis() as i64;
                    let now = chrono::Utc::now();
                    let _ = crate::heartbeat::store::record_run(
                        &config.workspace_dir,
                        &task.text,
                        &task.priority.to_string(),
                        now - chrono::Duration::milliseconds(duration_ms),
                        now,
                        "ok",
                        Some(response.as_str()),
                        duration_ms,
                        config.heartbeat.max_run_history,
                    );
                    // Consolidate heartbeat output to memory for cross-session awareness.
                    if config.memory.auto_save
                        && response.chars().count() >= 50
                        && let Some(ref mem) = heartbeat_memory
                    {
                        let key = format!("heartbeat_{}", uuid::Uuid::new_v4());
                        let summary = if response.len() > 500 {
                            // Find a valid UTF-8 char boundary at or before 500.
                            let mut end = 500;
                            while end > 0 && !response.is_char_boundary(end) {
                                end -= 1;
                            }
                            &response[..end]
                        } else {
                            &response
                        };
                        let _ = mem
                            .store(
                                &key,
                                &format!("Heartbeat task '{}': {}", task.text, summary),
                                kylinbot_memory::MemoryCategory::Daily,
                                None,
                            )
                            .await;
                    }

                    let announcement = if response.trim().is_empty() {
                        format!("💓 heartbeat task completed: {}", task.text)
                    } else {
                        response
                    };
                    if let Some((channel, target)) = &delivery {
                        let delivery_result = tokio::time::timeout(
                            Duration::from_secs(30),
                            crate::cron::scheduler::deliver_announcement(
                                &config,
                                channel,
                                target,
                                &announcement,
                            ),
                        )
                        .await;
                        match delivery_result {
                            Ok(Err(e)) => {
                                crate::health::mark_component_error(
                                    "heartbeat",
                                    format!("delivery failed: {e}"),
                                );
                                tracing::warn!("Heartbeat delivery failed: {e}");
                            }
                            Err(_) => {
                                crate::health::mark_component_error(
                                    "heartbeat",
                                    "delivery timed out (30s)".to_string(),
                                );
                                tracing::warn!("Heartbeat delivery timed out (30s)");
                            }
                            Ok(Ok(())) => {}
                        }
                    }
                }
                Err(e) => {
                    tick_had_error = true;
                    #[allow(clippy::cast_possible_truncation)]
                    let duration_ms = task_start.elapsed().as_millis() as i64;
                    let now = chrono::Utc::now();
                    let _ = crate::heartbeat::store::record_run(
                        &config.workspace_dir,
                        &task.text,
                        &task.priority.to_string(),
                        now - chrono::Duration::milliseconds(duration_ms),
                        now,
                        "error",
                        Some(&e.to_string()),
                        duration_ms,
                        config.heartbeat.max_run_history,
                    );
                    crate::health::mark_component_error("heartbeat", e.to_string());
                    tracing::warn!("Heartbeat task failed: {e}");
                }
            }
        }

        // Update metrics
        #[allow(clippy::cast_precision_loss)]
        let tick_elapsed = tick_start.elapsed().as_millis() as f64;
        {
            let mut m = metrics.lock();
            if tick_had_error {
                m.record_failure(tick_elapsed);
            } else {
                m.record_success(tick_elapsed);
            }
        }

        // Compute next sleep interval
        if adaptive {
            let failures = metrics.lock().consecutive_failures;
            sleep_mins = compute_adaptive_interval(
                base_interval,
                config.heartbeat.min_interval_minutes,
                config.heartbeat.max_interval_minutes,
                failures,
                has_high_priority,
            );
        } else {
            sleep_mins = base_interval;
        }
    }
}

/// Resolve delivery target: explicit config > auto-detect first configured channel.
fn resolve_heartbeat_delivery(config: &Config) -> Result<Option<(String, String)>> {
    let channel = config
        .heartbeat
        .target
        .as_deref()
        .map(str::trim)
        .filter(|value| !value.is_empty());
    let target = config
        .heartbeat
        .to
        .as_deref()
        .map(str::trim)
        .filter(|value| !value.is_empty());

    match (channel, target) {
        // Both explicitly set — validate and use.
        (Some(channel), Some(target)) => {
            validate_heartbeat_channel_config(config, channel)?;
            Ok(Some((channel.to_string(), target.to_string())))
        }
        // Only one set — error.
        (Some(_), None) => anyhow::bail!("heartbeat.to is required when heartbeat.target is set"),
        (None, Some(_)) => anyhow::bail!("heartbeat.target is required when heartbeat.to is set"),
        // Neither set — try auto-detect the first configured channel.
        (None, None) => Ok(auto_detect_heartbeat_channel(config)),
    }
}

/// Load recent conversation history for the heartbeat's delivery target and
/// format it as a text preamble to inject into the task prompt.
///
/// Scans `{workspace}/sessions/` for JSONL files whose name starts with
/// `{channel}_` and ends with `_{to}.jsonl` (or exactly `{channel}_{to}.jsonl`),
/// then picks the most recently modified match. This handles session key
/// formats such as `telegram_diskiller.jsonl` and
/// `telegram_5673725398_diskiller.jsonl`.
/// Returns `None` when `target`/`to` are not configured or no session exists.
const HEARTBEAT_SESSION_CONTEXT_MESSAGES: usize = 20;

fn load_heartbeat_session_context(config: &Config) -> Option<String> {
    use kylinbot_providers::traits::ChatMessage;

    let channel = config
        .heartbeat
        .target
        .as_deref()
        .map(str::trim)
        .filter(|v| !v.is_empty())?;
    let to = config
        .heartbeat
        .to
        .as_deref()
        .map(str::trim)
        .filter(|v| !v.is_empty())?;

    if channel.contains('/') || channel.contains('\\') || to.contains('/') || to.contains('\\') {
        tracing::warn!("heartbeat session context: channel/to contains path separators, skipping");
        return None;
    }

    let sessions_dir = config.workspace_dir.join("sessions");

    // Find the most recently modified JSONL file that belongs to this target.
    // Matches both `{channel}_{to}.jsonl` and `{channel}_{anything}_{to}.jsonl`.
    let prefix = format!("{channel}_");
    let suffix = format!("_{to}.jsonl");
    let exact = format!("{channel}_{to}.jsonl");
    let mid_prefix = format!("{channel}_{to}_");

    let path = std::fs::read_dir(&sessions_dir)
        .ok()?
        .filter_map(|e| e.ok())
        .filter(|e| {
            let name = e.file_name();
            let name = name.to_string_lossy();
            name.ends_with(".jsonl")
                && (name == exact
                    || (name.starts_with(&prefix) && name.ends_with(&suffix))
                    || name.starts_with(&mid_prefix))
        })
        .max_by_key(|e| {
            e.metadata()
                .and_then(|m| m.modified())
                .unwrap_or(std::time::SystemTime::UNIX_EPOCH)
        })
        .map(|e| e.path())?;

    if !path.exists() {
        tracing::debug!("💓 Heartbeat session context: no session file found for {channel}/{to}");
        return None;
    }

    let messages = load_jsonl_messages(&path);
    if messages.is_empty() {
        return None;
    }

    let recent: Vec<&ChatMessage> = messages
        .iter()
        .filter(|m| m.role == "user" || m.role == "assistant")
        .rev()
        .take(HEARTBEAT_SESSION_CONTEXT_MESSAGES)
        .collect::<Vec<_>>()
        .into_iter()
        .rev()
        .collect();

    // Only inject context if there is at least one real user message in the
    // window. If the JSONL contains only assistant messages (e.g. previous
    // heartbeat outputs with no reply yet), skip context to avoid feeding
    // Monika's own messages back to her in a loop.
    let has_user_message = recent.iter().any(|m| m.role == "user");
    if !has_user_message {
        tracing::debug!(
            "💓 Heartbeat session context: no user messages in recent history — skipping"
        );
        return None;
    }

    // Use the session file's mtime as a proxy for when the last message arrived.
    let last_message_age = std::fs::metadata(&path)
        .ok()
        .and_then(|m| m.modified().ok())
        .and_then(|mtime| mtime.elapsed().ok());

    let silence_note = match last_message_age {
        Some(age) => {
            let mins = age.as_secs() / 60;
            if mins < 60 {
                format!("(last message ~{mins} minutes ago)\n")
            } else {
                let hours = mins / 60;
                let rem = mins % 60;
                if rem == 0 {
                    format!("(last message ~{hours}h ago)\n")
                } else {
                    format!("(last message ~{hours}h {rem}m ago)\n")
                }
            }
        }
        None => String::new(),
    };

    tracing::debug!(
        "💓 Heartbeat session context: {} messages from {}, silence: {}",
        recent.len(),
        path.display(),
        silence_note.trim(),
    );

    let mut ctx = format!(
        "[Recent conversation history — use this for context when composing your message] {silence_note}",
    );
    for msg in &recent {
        let label = if msg.role == "user" { "User" } else { "You" };
        // Truncate very long messages to avoid bloating the prompt.
        // Use char_indices to avoid panicking on multi-byte UTF-8 characters.
        let content = if msg.content.len() > 500 {
            let truncate_at = msg
                .content
                .char_indices()
                .map(|(i, _)| i)
                .take_while(|&i| i <= 500)
                .last()
                .unwrap_or(0);
            format!("{}…", &msg.content[..truncate_at])
        } else {
            msg.content.clone()
        };
        ctx.push_str(label);
        ctx.push_str(": ");
        ctx.push_str(&content);
        ctx.push('\n');
    }

    Some(ctx)
}

/// Read the last `HEARTBEAT_SESSION_CONTEXT_MESSAGES` `ChatMessage` lines from
/// a JSONL session file using a bounded rolling window so we never hold the
/// entire file in memory.
fn load_jsonl_messages(path: &std::path::Path) -> Vec<kylinbot_providers::traits::ChatMessage> {
    use std::collections::VecDeque;
    use std::io::BufRead;

    let file = match std::fs::File::open(path) {
        Ok(f) => f,
        Err(_) => return Vec::new(),
    };
    let reader = std::io::BufReader::new(file);
    let mut window: VecDeque<kylinbot_providers::traits::ChatMessage> =
        VecDeque::with_capacity(HEARTBEAT_SESSION_CONTEXT_MESSAGES + 1);
    for line in reader.lines() {
        let Ok(line) = line else { continue };
        let trimmed = line.trim();
        if trimmed.is_empty() {
            continue;
        }
        if let Ok(msg) = serde_json::from_str::<kylinbot_providers::traits::ChatMessage>(trimmed) {
            window.push_back(msg);
            if window.len() > HEARTBEAT_SESSION_CONTEXT_MESSAGES {
                window.pop_front();
            }
        }
    }
    window.into_iter().collect()
}

/// Auto-detect the best channel for heartbeat delivery by checking which
/// channels are configured. Returns the first match in priority order.
fn auto_detect_heartbeat_channel(config: &Config) -> Option<(String, String)> {
    // Priority order: telegram > discord > slack > mattermost
    if let Some(tg) = &config.channels.telegram {
        // Use the first allowed_user as target, or fall back to empty (broadcast)
        let target = tg.allowed_users.first().cloned().unwrap_or_default();
        if !target.is_empty() {
            return Some(("telegram".to_string(), target));
        }
    }
    if config.channels.discord.is_some() {
        // Discord requires explicit target — can't auto-detect
        return None;
    }
    if config.channels.slack.is_some() {
        // Slack requires explicit target
        return None;
    }
    if config.channels.mattermost.is_some() {
        // Mattermost requires explicit target
        return None;
    }
    None
}

fn validate_heartbeat_channel_config(config: &Config, channel: &str) -> Result<()> {
    match channel.to_ascii_lowercase().as_str() {
        "telegram" => {
            if config.channels.telegram.is_none() {
                anyhow::bail!(
                    "heartbeat.target is set to telegram but channels.telegram is not configured"
                );
            }
        }
        "discord" => {
            if config.channels.discord.is_none() {
                anyhow::bail!(
                    "heartbeat.target is set to discord but channels.discord is not configured"
                );
            }
        }
        "slack" => {
            if config.channels.slack.is_none() {
                anyhow::bail!(
                    "heartbeat.target is set to slack but channels.slack is not configured"
                );
            }
        }
        "mattermost" => {
            if config.channels.mattermost.is_none() {
                anyhow::bail!(
                    "heartbeat.target is set to mattermost but channels.mattermost is not configured"
                );
            }
        }
        other => anyhow::bail!("unsupported heartbeat.target channel: {other}"),
    }

    Ok(())
}

fn has_supervised_channels(config: &Config) -> bool {
    config.channels.channels().iter().any(|(_, ok)| *ok)
}

// run_mqtt_sop_listener has been moved to kylinbot-channels::orchestrator::mqtt.
// The daemon now receives it as a callback via DaemonSubsystems::mqtt_start.

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;

    fn test_config(tmp: &TempDir) -> Config {
        let config = Config {
            workspace_dir: tmp.path().join("workspace"),
            config_path: tmp.path().join("config.toml"),
            ..Config::default()
        };
        std::fs::create_dir_all(&config.workspace_dir).unwrap();
        config
    }

    #[test]
    fn state_file_path_uses_config_directory() {
        let tmp = TempDir::new().unwrap();
        let config = test_config(&tmp);

        let path = state_file_path(&config);
        assert_eq!(path, tmp.path().join("daemon_state.json"));
    }

    #[tokio::test]
    async fn supervisor_marks_error_and_restart_on_failure() {
        let token = CancellationToken::new();
        let handle = spawn_component_supervisor(
            "daemon-test-fail",
            1,
            1,
            token.clone(),
            || async { anyhow::bail!("boom") },
        );

        tokio::time::sleep(Duration::from_millis(50)).await;
        token.cancel();
        let _ = handle.await;

        let snapshot = crate::health::snapshot_json();
        let component = &snapshot["components"]["daemon-test-fail"];
        assert_eq!(component["status"], "error");
        assert!(component["restart_count"].as_u64().unwrap_or(0) >= 1);
        assert!(
            component["last_error"]
                .as_str()
                .unwrap_or("")
                .contains("boom")
        );
    }

    #[tokio::test]
    async fn supervisor_marks_unexpected_exit_as_error() {
        let token = CancellationToken::new();
        let handle = spawn_component_supervisor(
            "daemon-test-exit",
            1,
            1,
            token.clone(),
            || async { Ok(()) },
        );

        tokio::time::sleep(Duration::from_millis(50)).await;
        token.cancel();
        let _ = handle.await;

        let snapshot = crate::health::snapshot_json();
        let component = &snapshot["components"]["daemon-test-exit"];
        assert_eq!(component["status"], "error");
        assert!(component["restart_count"].as_u64().unwrap_or(0) >= 1);
        assert!(
            component["last_error"]
                .as_str()
                .unwrap_or("")
                .contains("component exited unexpectedly")
        );
    }

    #[test]
    fn detects_no_supervised_channels() {
        let config = Config::default();
        assert!(!has_supervised_channels(&config));
    }

    #[test]
    fn detects_supervised_channels_present() {
        let mut config = Config::default();
        config.channels.telegram = Some(kylinbot_config::schema::TelegramConfig {
            enabled: true,
            bot_token: "token".into(),
            allowed_users: vec![],
            stream_mode: kylinbot_config::schema::StreamMode::default(),
            draft_update_interval_ms: 1000,
            interrupt_on_new_message: false,
            mention_only: false,
            ack_reactions: None,
            proxy_url: None,
            approval_timeout_secs: 120,
        });
        assert!(has_supervised_channels(&config));
    }

    #[test]
    fn detects_dingtalk_as_supervised_channel() {
        let mut config = Config::default();
        config.channels.dingtalk = Some(kylinbot_config::schema::DingTalkConfig {
            enabled: true,
            client_id: "client_id".into(),
            client_secret: "client_secret".into(),
            allowed_users: vec!["*".into()],
            proxy_url: None,
        });
        assert!(has_supervised_channels(&config));
    }

    #[test]
    fn detects_mattermost_as_supervised_channel() {
        let mut config = Config::default();
        config.channels.mattermost = Some(kylinbot_config::schema::MattermostConfig {
            enabled: true,
            url: "https://mattermost.example.com".into(),
            bot_token: "token".into(),
            channel_id: Some("channel-id".into()),
            allowed_users: vec!["*".into()],
            thread_replies: Some(true),
            mention_only: Some(false),
            interrupt_on_new_message: false,
            proxy_url: None,
        });
        assert!(has_supervised_channels(&config));
    }

    #[test]
    fn detects_qq_as_supervised_channel() {
        let mut config = Config::default();
        config.channels.qq = Some(kylinbot_config::schema::QQConfig {
            enabled: true,
            app_id: "app-id".into(),
            app_secret: "app-secret".into(),
            allowed_users: vec!["*".into()],
            proxy_url: None,
        });
        assert!(has_supervised_channels(&config));
    }

    #[test]
    fn detects_nextcloud_talk_as_supervised_channel() {
        let mut config = Config::default();
        config.channels.nextcloud_talk = Some(kylinbot_config::schema::NextcloudTalkConfig {
            enabled: true,
            base_url: "https://cloud.example.com".into(),
            app_token: "app-token".into(),
            webhook_secret: None,
            allowed_users: vec!["*".into()],
            proxy_url: None,
            bot_name: None,
        });
        assert!(has_supervised_channels(&config));
    }

    #[test]
    fn webhook_only_config_is_supervised() {
        let mut config = Config::default();
        config.channels.webhook = Some(kylinbot_config::schema::WebhookConfig {
            enabled: true,
            port: 8080,
            listen_path: None,
            send_url: None,
            send_method: None,
            auth_header: None,
            secret: None,
        });
        assert!(has_supervised_channels(&config));
    }

    #[test]
    fn resolve_delivery_none_when_unset() {
        let config = Config::default();
        let target = resolve_heartbeat_delivery(&config).unwrap();
        assert!(target.is_none());
    }

    #[test]
    fn resolve_delivery_requires_to_field() {
        let mut config = Config::default();
        config.heartbeat.target = Some("telegram".into());
        let err = resolve_heartbeat_delivery(&config).unwrap_err();
        assert!(
            err.to_string()
                .contains("heartbeat.to is required when heartbeat.target is set")
        );
    }

    #[test]
    fn resolve_delivery_requires_target_field() {
        let mut config = Config::default();
        config.heartbeat.to = Some("123456".into());
        let err = resolve_heartbeat_delivery(&config).unwrap_err();
        assert!(
            err.to_string()
                .contains("heartbeat.target is required when heartbeat.to is set")
        );
    }

    #[test]
    fn resolve_delivery_rejects_unsupported_channel() {
        let mut config = Config::default();
        config.heartbeat.target = Some("email".into());
        config.heartbeat.to = Some("ops@example.com".into());
        let err = resolve_heartbeat_delivery(&config).unwrap_err();
        assert!(
            err.to_string()
                .contains("unsupported heartbeat.target channel")
        );
    }

    #[test]
    fn resolve_delivery_requires_channel_configuration() {
        let mut config = Config::default();
        config.heartbeat.target = Some("telegram".into());
        config.heartbeat.to = Some("123456".into());
        let err = resolve_heartbeat_delivery(&config).unwrap_err();
        assert!(
            err.to_string()
                .contains("channels.telegram is not configured")
        );
    }

    #[test]
    fn resolve_delivery_accepts_telegram_configuration() {
        let mut config = Config::default();
        config.heartbeat.target = Some("telegram".into());
        config.heartbeat.to = Some("123456".into());
        config.channels.telegram = Some(kylinbot_config::schema::TelegramConfig {
            enabled: true,
            bot_token: "bot-token".into(),
            allowed_users: vec![],
            stream_mode: kylinbot_config::schema::StreamMode::default(),
            draft_update_interval_ms: 1000,
            interrupt_on_new_message: false,
            mention_only: false,
            ack_reactions: None,
            proxy_url: None,
            approval_timeout_secs: 120,
        });

        let target = resolve_heartbeat_delivery(&config).unwrap();
        assert_eq!(target, Some(("telegram".to_string(), "123456".to_string())));
    }

    #[test]
    fn auto_detect_telegram_when_configured() {
        let mut config = Config::default();
        config.channels.telegram = Some(kylinbot_config::schema::TelegramConfig {
            enabled: true,
            bot_token: "bot-token".into(),
            allowed_users: vec!["user123".into()],
            stream_mode: kylinbot_config::schema::StreamMode::default(),
            draft_update_interval_ms: 1000,
            interrupt_on_new_message: false,
            mention_only: false,
            ack_reactions: None,
            proxy_url: None,
            approval_timeout_secs: 120,
        });

        let target = resolve_heartbeat_delivery(&config).unwrap();
        assert_eq!(
            target,
            Some(("telegram".to_string(), "user123".to_string()))
        );
    }

    #[test]
    fn auto_detect_none_when_no_channels() {
        let config = Config::default();
        let target = auto_detect_heartbeat_channel(&config);
        assert!(target.is_none());
    }

    /// Verify that SIGHUP does not cause shutdown — the daemon should ignore it
    /// and only terminate on SIGINT or SIGTERM.
    #[cfg(unix)]
    #[tokio::test]
    async fn sighup_does_not_shut_down_daemon() {
        use libc;
        use tokio::time::{Duration, timeout};

        let handle = tokio::spawn(wait_for_shutdown_signal());

        // Give the signal handler time to register
        tokio::time::sleep(Duration::from_millis(50)).await;

        // Send SIGHUP to ourselves — should be ignored by the handler
        unsafe { libc::raise(libc::SIGHUP) };

        // The future should NOT complete within a short window
        let result = timeout(Duration::from_millis(200), handle).await;
        assert!(
            result.is_err(),
            "wait_for_shutdown_signal should not return after SIGHUP"
        );
    }

    /// Reload requests now flow through an mpsc channel rather than waking
    /// `wait_for_shutdown_signal`, so this test exercises the channel directly:
    /// receivers should see the payload's `affected` set intact.
    #[tokio::test]
    async fn reload_channel_propagates_affected_set() {
        use std::collections::HashSet;
        use tokio::sync::mpsc;

        let (tx, mut rx) = mpsc::channel::<ReloadRequest>(4);
        let mut affected = HashSet::new();
        affected.insert(Subsystem::Channels);
        let req = ReloadRequest {
            new_config: Config::default(),
            affected: affected.clone(),
        };
        tx.send(req).await.expect("send reload");
        let got = rx.recv().await.expect("recv reload");
        assert_eq!(got.affected, affected);
    }
}
