# Channels 优雅热重载设计

> 日期：2026-05-20
> 状态：设计已确认，待实施
> 范围：daemon、kylinbot-runtime、kylinbot-channels、kylinbot-gateway、kylinbot 二进制入口
> 关键词：hot reload、graceful restart、worker survival、subsystem diff

---

## 1. 背景与问题陈述

### 1.1 现状

KylinBot 的配置热重载链路（截至本设计稿撰写时）：

```
[前端] POST /api/lansenger/config（或任何其它配置写入端点）
   │
   ▼
[gateway] save_config_and_reload()  →  reload_config_and_daemon()
   │
   ├── shutdown_tx.send(true)   → axum::serve.graceful_shutdown 退出 → 所有 HTTP/WS 全断
   └── reload_tx.send(true)     → daemon::wait_for_exit_signal 返回 Reload
                                 │
                                 ▼
                       daemon::run 把所有 supervisor handle abort
                                 │
                                 ▼
                       [main.rs 外层 loop] 重新读 config → 重新调 daemon::run
                                 │
                                 ▼
                       全部子系统 from scratch
```

### 1.2 问题

1. **WebSocket 连接被中断**：dashboard、ACP、所有 channel 的 WS 长连接全部断掉。前端依赖自动重连兜底，但断连瞬间正在传输的数据会丢失。
2. **正在运行的 agent 推理被打断**：worker 任务挂在 `run_message_dispatch_loop` 内的本地 `JoinSet`，supervisor 一被 abort，JoinSet 跟着 drop，**所有 in-flight 推理立刻取消**。如果用户正在等 LLM 回复或工具调用结果，会直接丢失。
3. **不相关子系统受牵连**：改个纯前端字段（如显示语言）也会触发 cron、heartbeat、所有 channel 长连接重启。
4. **触发频率高**：每次在 Web 上点"连接 XX channel"都会触发一次全量重启。频繁配置变更场景下体感很差。

### 1.3 用户期望

引用对话中的明确要求：

> 我想要的是不能把推理终端

也就是：**reload 必须保证正在跑的 agent 推理任务跑完，不能被打断。**

### 1.4 设计目标

| 优先级 | 目标 | 度量 |
|---|---|---|
| P0 | reload 时**正在跑的 agent 推理不被打断** | 推理完成率 = 100% |
| P0 | reload 时 gateway HTTP/WS 不重启（除非 gateway 自身配置改了） | dashboard WS 维持连接 |
| P1 | 只有受配置变化影响的子系统重启 | 改 cron 不会断 channel 连接 |
| P1 | 纯前端字段变更不触发任何子系统重启 | diff 为空时直接 no-op |
| P2 | 改动量可控、可独立 PR、可回滚 | <600 行核心代码、有 feature flag 兜底 |

非目标：

- 不做"零停机的 gateway 重启"（需要 SO_REUSEPORT + 双进程接力，超出 Phase 1+ 范围）
- 不做"单 channel 重启"（改 lansenger 不影响 lark），仍是 channels-as-a-whole 重启，留给 Phase 2

---

## 2. 当前架构分析

### 2.1 进程与任务拓扑

```
kylinbot daemon (PID 不变)
│
├── [main.rs:1771] loop {
│       daemon::run(config, ...)
│   }
│
└── daemon::run (kylinbot-runtime/src/daemon/mod.rs:136)
    │
    ├── (reload_tx, reload_rx) = watch::channel::<bool>(false)
    │
    ├── handles: Vec<JoinHandle<()>> = [
    │       spawn_state_writer,
    │       spawn_component_supervisor("gateway",   |..| gateway_start(...)),
    │       spawn_component_supervisor("channels",  |..| channels_start(...)),
    │       spawn_component_supervisor("heartbeat", ...),
    │       spawn_component_supervisor("scheduler", ...),
    │       spawn_component_supervisor("mqtt",      ...),
    │   ]
    │
    ├── wait_for_exit_signal(reload_rx)  ← block 在 SIGINT/SIGTERM/Ctrl+C 或 reload_rx.changed()
    │
    └── 退出时：
        for handle in &handles { handle.abort(); }
        // 注：abort 只取消 supervisor 自身的 future；
        // supervisor future 内部用 tokio::spawn 创建的子任务，
        // 如果用 JoinSet 包裹 → drop 时全 abort
        // 如果用裸 JoinHandle 存在 Vec → drop 时 detach 继续跑（这是 bug 来源之一）
```

### 2.2 channels supervisor 内部结构

`kylinbot_channels::orchestrator::start_channels`（crates/kylinbot-channels/src/orchestrator/mod.rs:5473）：

```rust
async fn start_channels(config, canvas_store) -> Result<()> {
    // ① 构建 provider, runtime_ctx (含 channels_by_name, tools, ...)
    // ② 为每个 channel 起独立 listen 任务
    let mut handles: Vec<JoinHandle<()>> = vec![];
    for ch in channels {
        handles.push(spawn_supervised_listener(ch, tx.clone(), ...));
    }
    // ③ 跑消息分发循环
    run_message_dispatch_loop(rx, runtime_ctx, max_in_flight).await;
    // ④ drain listener handles（注：abort 路径下不会执行到这里）
    for h in handles { let _ = h.await; }
    Ok(())
}
```

`run_message_dispatch_loop`（同文件 3929）：

```rust
async fn run_message_dispatch_loop(mut rx, ctx, max_in_flight) {
    let sem = Arc::new(Semaphore::new(max_in_flight));
    let mut workers = tokio::task::JoinSet::new();       // ← 关键
    let in_flight_by_sender = Arc::new(Mutex::new(HashMap::new()));
    let task_seq = Arc::new(AtomicU64::new(1));

    while let Some(msg) = rx.recv().await {
        // /stop 短路、debounce 略
        let permit = sem.clone().acquire_owned().await.unwrap();
        workers.spawn(async move {                       // ← 关键
            dispatch_worker(ctx, msg, in_flight, task_seq, permit).await;
        });
    }
    while let Some(r) = workers.join_next().await { /* log */ }
}
```

**问题归纳**：

- `workers: JoinSet<()>` 是 dispatch loop 的本地变量。dispatch_loop 退出（如被 abort）时 JoinSet 析构 → **JoinSet 的 Drop 实现会对所有 task 调用 abort()**。这就是推理被打断的根因。
- `handles: Vec<JoinHandle<()>>` 存 listener。supervisor 被外部 abort 时 future 在某个 await 点取消，Vec 析构 → **JoinHandle 的 Drop 不会 abort task，只是 detach** → 旧 listener 残留继续跑（之前分析过的 ghost listener）。

### 2.3 gateway shutdown 链路

`crates/kylinbot-gateway/src/lib.rs:1402`：

```rust
axum::serve(listener, app...)
    .with_graceful_shutdown(async move {
        let _ = shutdown_rx.changed().await;
        tracing::info!("KylinBot Gateway shutting down...");
    })
    .await?;
```

`AppState.shutdown_tx` 控制这条 graceful shutdown 通道。今天的 reload 路径同时发送 `shutdown_tx + reload_tx`，所以 gateway 也被关掉。

### 2.4 触发点清单（reload_config_and_daemon 被谁调用）

| 文件:行 | 触发场景 |
|---|---|
| `api.rs:56` | `/admin/reload` 手动触发 |
| `api.rs:314` | 通用 `save_config_and_reload`，被 `/api/config/save`、`/api/providers/...`、`/api/channels/...` 等多个端点用 |
| `api.rs:3079` | DingTalk QR 绑定成功后写 config |
| `api.rs:3350` | Feishu/Lark 绑定 |
| `api.rs:3671` | WeChat 绑定 |
| `api.rs:3866` | Lansenger 绑定 |
| `api.rs:3955`（Lansenger config 直接写） | 同上 |

**几乎所有写配置的端点都会触发整体 reload。**

---

## 3. 设计方案

### 3.1 整体思路

把 reload 从「整体重启」改成「按需重启」，且让 worker 任务跨 reload 生存。具体来说：

1. **Worker 解耦**：worker spawn 到 daemon 级别的共享 `ChannelWorkerPool`，pool 生命周期 = 进程生命周期，不绑 supervisor。supervisor reload 时 pool 不动，里面的 worker 自然跑完。
2. **Subsystem diff**：reload 触发前算出新旧 config 的 diff，得出 `affected: HashSet<Subsystem>`。只重启受影响的 subsystem，其它原地不动。
3. **优雅退出代替 abort**：supervisor 用 `CancellationToken` 通知，自己在 cancel 后做显式清理（abort listener、不动 worker pool），返回。daemon 不再用 `handle.abort()`。
4. **gateway 不再为 reload 关闭**：`shutdown_tx` 只用于真正的进程关闭信号，reload 路径不再触发它（除非 gateway 自身配置改了）。

### 3.2 数据结构

#### 3.2.1 Subsystem（新）

```rust
// crates/kylinbot-runtime/src/daemon/reload.rs（新文件）

#[derive(Debug, Clone, Copy, Hash, PartialEq, Eq)]
pub enum Subsystem {
    Gateway,
    Channels,
    Cron,
    Heartbeat,
    Mqtt,
}
```

#### 3.2.2 ReloadRequest（新）

```rust
#[derive(Debug, Clone)]
pub struct ReloadRequest {
    pub new_config: Config,
    pub affected: HashSet<Subsystem>,
}
```

#### 3.2.3 diff_subsystems（新）

```rust
pub fn diff_subsystems(old: &Config, new: &Config) -> HashSet<Subsystem> {
    let mut out = HashSet::new();
    if old.channels != new.channels {
        out.insert(Subsystem::Channels);
    }
    if old.gateway != new.gateway {
        out.insert(Subsystem::Gateway);
    }
    if old.cron != new.cron {
        out.insert(Subsystem::Cron);
    }
    if old.heartbeat != new.heartbeat {
        out.insert(Subsystem::Heartbeat);
    }
    if old.channels.mqtt != new.channels.mqtt {
        out.insert(Subsystem::Mqtt);
    }
    out
}
```

**前置条件**：`Config` 内相关字段（`ChannelsConfig`、`GatewayConfig`、`CronConfig`、`HeartbeatConfig`、`MqttConfig`）需要 `#[derive(PartialEq, Eq)]`。多数已有，不全的补上。**这是一个 schema-level 的小改动**，影响所有引用这些 type 的地方（仅 derive，不改字段，向下兼容）。

#### 3.2.4 ChannelWorkerPool（新）

```rust
// crates/kylinbot-runtime/src/daemon/worker_pool.rs（新文件）

use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::{Duration, Instant};

#[derive(Clone)]
pub struct ChannelWorkerPool {
    semaphore: Arc<tokio::sync::Semaphore>,
    in_flight: Arc<AtomicUsize>,
}

impl ChannelWorkerPool {
    pub fn new(max_in_flight: usize) -> Self {
        Self {
            semaphore: Arc::new(tokio::sync::Semaphore::new(max_in_flight.max(1))),
            in_flight: Arc::new(AtomicUsize::new(0)),
        }
    }

    /// Spawn a worker that **survives supervisor restarts**.
    ///
    /// The future is detached (no JoinHandle is retained), so cancelling the
    /// supervisor — or dropping any local task set — has no effect on it.
    /// The semaphore guarantees `max_in_flight` global concurrency across
    /// reloads.
    pub fn spawn_worker<F>(&self, fut: F)
    where
        F: std::future::Future<Output = ()> + Send + 'static,
    {
        let sem = self.semaphore.clone();
        let counter = self.in_flight.clone();
        tokio::spawn(async move {
            let _permit = sem.acquire_owned().await.expect("worker semaphore closed");
            counter.fetch_add(1, Ordering::Relaxed);
            fut.await;
            counter.fetch_sub(1, Ordering::Relaxed);
        });
    }

    pub fn active(&self) -> usize {
        self.in_flight.load(Ordering::Relaxed)
    }

    /// Poll-wait until all workers complete, bounded by `timeout`.
    /// Returns `true` if the pool fully drained, `false` if timed out.
    pub async fn drain_with_timeout(&self, timeout: Duration) -> bool {
        let start = Instant::now();
        while self.active() > 0 && start.elapsed() < timeout {
            tokio::time::sleep(Duration::from_millis(100)).await;
        }
        self.active() == 0
    }
}
```

### 3.3 改造点详细描述

#### 改动 A：worker 解耦（`crates/kylinbot-channels/src/orchestrator/mod.rs`）

**A1**：`start_channels` 函数签名扩展：

```rust
// before
pub async fn start_channels(
    config: Config,
    canvas_store: Option<CanvasStore>,
) -> Result<()>

// after
pub async fn start_channels(
    config: Config,
    canvas_store: Option<CanvasStore>,
    worker_pool: Arc<ChannelWorkerPool>,           // 新
    cancel_token: CancellationToken,               // 新
) -> Result<()>
```

**A2**：`run_message_dispatch_loop` 签名扩展，去掉本地 JoinSet：

```rust
// before
async fn run_message_dispatch_loop(
    mut rx: Receiver<ChannelMessage>,
    ctx: Arc<ChannelRuntimeContext>,
    max_in_flight_messages: usize,
) {
    let semaphore = Arc::new(Semaphore::new(max_in_flight_messages));
    let mut workers = tokio::task::JoinSet::new();
    let in_flight_by_sender = ...;
    while let Some(msg) = rx.recv().await {
        // ...
        let permit = semaphore.clone().acquire_owned().await.unwrap();
        workers.spawn(async move { dispatch_worker(...).await; });
    }
    while let Some(r) = workers.join_next().await { ... }
}

// after
async fn run_message_dispatch_loop(
    mut rx: Receiver<ChannelMessage>,
    ctx: Arc<ChannelRuntimeContext>,
    worker_pool: Arc<ChannelWorkerPool>,           // 新
) {
    // 不再创建本地 Semaphore（已挪到 pool 内）
    // 不再创建本地 JoinSet
    let in_flight_by_sender = ...;
    while let Some(msg) = rx.recv().await {
        // /stop 短路、debounce 不变
        let worker_ctx = ctx.clone();
        let in_flight = in_flight_by_sender.clone();
        let task_seq = task_seq.clone();
        worker_pool.spawn_worker(async move {
            dispatch_worker(worker_ctx, msg, in_flight, task_seq).await;
        });
    }
    // 不再 drain workers —— supervisor 退出时 worker 继续在 pool 里跑
}
```

**A3**：`start_channels` 内部用 `tokio::select!` 监听 cancel_token：

```rust
async fn start_channels(config, canvas_store, worker_pool, cancel_token) -> Result<()> {
    // ... 构建 provider, ctx, tx/rx ...

    let mut listener_handles: Vec<JoinHandle<()>> = vec![];
    for ch in channels {
        listener_handles.push(spawn_supervised_listener(ch, tx.clone(), ...));
    }

    tokio::select! {
        _ = cancel_token.cancelled() => {
            tracing::info!("channels supervisor: cancel received, stopping listeners");
            for h in &listener_handles { h.abort(); }
            for h in listener_handles { let _ = h.await; }
            // 不动 worker_pool —— worker 自己跑完
        }
        _ = run_message_dispatch_loop(rx, ctx, worker_pool.clone()) => {
            tracing::warn!("channels dispatch loop exited unexpectedly");
            for h in &listener_handles { h.abort(); }
        }
    }
    Ok(())
}
```

**A4**：`dispatch_worker` 签名同步去掉 `permit: OwnedSemaphorePermit` 入参（permit 现在由 pool 内部持有）。函数体不变。

**影响域**：仅 channels orchestrator 内部。`spawn_supervised_listener`、各个 channel 的 `listen()` 实现都不动。`process_channel_message` 不动。

#### 改动 B：Subsystem + diff（`crates/kylinbot-runtime/src/daemon/reload.rs`）

新文件，~80 行。提供：

- `Subsystem` 枚举
- `ReloadRequest` 结构
- `diff_subsystems(old, new)` 函数
- 单元测试覆盖空 diff、单字段 diff、跨子系统 diff

**前置 schema 改动**：

```rust
// kylinbot-config/src/schema.rs

#[derive(Debug, Clone, Default, Deserialize, Serialize, PartialEq, Eq)]  // 加 PartialEq, Eq
pub struct ChannelsConfig { ... }

// 同样地 GatewayConfig / CronConfig / HeartbeatConfig / MqttConfig 加 PartialEq, Eq
```

**注意**：`Config` 本身含 `PathBuf`、`String`、enum 等 → `PartialEq` 应可自动 derive。但其中含 `Arc<...>` 或带回调的字段时不能 derive，需要手写或 `#[derive(PartialEq)]` 时跳过。**实施时按编译错误逐个解决，可能需要 `#[serde(skip)] #[derivative(PartialEq="ignore")]` 之类的标记**。

#### 改动 C：daemon::run 改为选择性 reload（`crates/kylinbot-runtime/src/daemon/mod.rs`）

**C1**：把 reload 通道从 `watch<bool>` 换成 `mpsc<ReloadRequest>`：

```rust
// before
let (reload_tx, reload_rx) = tokio::sync::watch::channel::<bool>(false);

// after
let (reload_tx, mut reload_rx) = tokio::sync::mpsc::channel::<ReloadRequest>(8);
```

**C2**：handles 容器升级：

```rust
// before
let mut handles: Vec<JoinHandle<()>> = vec![spawn_state_writer(...)];

// after
let mut handles: HashMap<Subsystem, (CancellationToken, JoinHandle<()>)> = HashMap::new();
// state_writer 仍是 Vec<JoinHandle> 或单独存（它不参与 reload）
let mut singleton_handles: Vec<JoinHandle<()>> = vec![spawn_state_writer(config.clone())];
```

**C3**：每个 supervisor 启动时绑定自己的 token：

```rust
fn spawn_subsystem(
    subsystem: Subsystem,
    config: Config,
    worker_pool: Arc<ChannelWorkerPool>,
    cancel_token: CancellationToken,
    /* 其它共享资源 */
) -> JoinHandle<()> {
    match subsystem {
        Subsystem::Channels => tokio::spawn(start_channels(config, ..., worker_pool, cancel_token)),
        Subsystem::Gateway => tokio::spawn(start_gateway(..., cancel_token)),
        Subsystem::Cron => tokio::spawn(start_cron(config, cancel_token)),
        // ...
    }
}
```

**C4**：主循环改为 `tokio::select!` 同时等 OS signal 与 reload mpsc：

```rust
let worker_pool = Arc::new(ChannelWorkerPool::new(max_in_flight));

// 初始拉起所有子系统
for subsystem in initial_subsystems(&config) {
    let token = CancellationToken::new();
    let handle = spawn_subsystem(subsystem, config.clone(), worker_pool.clone(), token.clone());
    handles.insert(subsystem, (token, handle));
}

loop {
    tokio::select! {
        _ = wait_os_signal() => {
            // Shutdown 路径
            for (_, (token, _)) in &handles { token.cancel(); }
            for (_, (_, h)) in handles.drain() { let _ = h.await; }
            worker_pool.drain_with_timeout(Duration::from_secs(30)).await;
            return Ok(DaemonExit::Shutdown);
        }
        Some(req) = reload_rx.recv() => {
            tracing::info!("daemon reload requested: affected = {:?}", req.affected);
            for subsystem in req.affected {
                if let Some((token, handle)) = handles.remove(&subsystem) {
                    token.cancel();
                    let _ = handle.await;
                }
                let new_token = CancellationToken::new();
                let new_handle = spawn_subsystem(
                    subsystem,
                    req.new_config.clone(),
                    worker_pool.clone(),
                    new_token.clone(),
                );
                handles.insert(subsystem, (new_token, new_handle));
            }
            // 不再 return Reload；daemon::run 是单次跑到底的
        }
    }
}
```

**C5**：`DaemonSubsystems` 结构调整：

```rust
// before：每个 subsystem 一个 fn，签名各异
pub struct DaemonSubsystems {
    pub gateway_start: Option<Box<dyn Fn(host, port, config, tx, reload_tx) -> Future>>,
    pub channels_start: Option<Box<dyn Fn(config) -> Future>>,
    pub mqtt_start: Option<Box<dyn Fn(mqtt_config) -> Future>>,
}

// after：每个 subsystem 接受 config + worker_pool + cancel_token，签名统一
pub struct DaemonSubsystems {
    pub gateway_start: Option<Box<dyn Fn(SpawnArgs) -> Future>>,
    pub channels_start: Option<Box<dyn Fn(SpawnArgs) -> Future>>,
    pub mqtt_start: Option<Box<dyn Fn(SpawnArgs) -> Future>>,
}

pub struct SpawnArgs {
    pub config: Config,
    pub worker_pool: Arc<ChannelWorkerPool>,
    pub cancel_token: CancellationToken,
    pub event_tx: tokio::sync::broadcast::Sender<serde_json::Value>,
    pub reload_tx: tokio::sync::mpsc::Sender<ReloadRequest>,    // gateway 需要
    pub host: String,
    pub port: u16,
}
```

**影响域**：
- `kylinbot-runtime/src/daemon/mod.rs`：大改 `run()` 与 `DaemonSubsystems` 结构
- `src/main.rs`：调用 daemon::run 的地方需要新构造 SpawnArgs 闭包
- `kylinbot-gateway/src/lib.rs`：`run_gateway` 签名加 `cancel_token` 入参，axum 的 graceful_shutdown 改听 cancel_token

#### 改动 D：gateway 改造（`crates/kylinbot-gateway/src/lib.rs`、`crates/kylinbot-gateway/src/api.rs`）

**D1**：`AppState.shutdown_tx` 语义收窄。它今天既被 reload 路径用又被进程退出用，混杂。Phase 1+ 后：

- 进程退出（SIGINT/SIGTERM）→ daemon::run 通过 `cancel_token.cancel()` 通知 gateway
- gateway 的 `axum::serve.graceful_shutdown` 监听 `cancel_token.cancelled()`，不再监听 `shutdown_tx`
- `AppState.shutdown_tx` 字段保留（向下兼容旧代码路径），但 reload 路径不再触发它

**D2**：`AppState.reload_tx` 类型升级：

```rust
// before
pub reload_tx: Option<tokio::sync::watch::Sender<bool>>,

// after
pub reload_tx: Option<tokio::sync::mpsc::Sender<ReloadRequest>>,
```

**D3**：`reload_config_and_daemon` 改造（`api.rs:22`）：

```rust
async fn reload_config_and_daemon(state: &AppState) -> Result<(), &'static str> {
    let new_config = Config::load_or_init().await
        .map_err(|_| "reload config failed")?;
    let mut new_config = new_config;
    new_config.apply_env_overrides();

    let old_config = state.config.lock().clone();
    let affected = diff_subsystems(&old_config, &new_config);

    // 更新内存中的 config —— gateway handler 立刻能读到新值
    *state.config.lock() = new_config.clone();

    if affected.is_empty() {
        tracing::info!("config reload: no subsystem affected, in-memory swap only");
        return Ok(());
    }

    let Some(tx) = state.reload_tx.clone() else {
        return Err("no daemon supervisor");
    };
    let req = ReloadRequest { new_config, affected };
    tx.try_send(req).map_err(|_| "reload channel full or closed")?;
    Ok(())
}
```

**影响域**：仅 gateway 内 reload 逻辑，触发它的 7 个端点（见 §2.4）行为自动跟随。

#### 改动 E：main.rs 简化（`src/main.rs`）

**E1**：删掉外层 reload loop：

```rust
// before
let mut current_config = config;
loop {
    let subsystems = DaemonSubsystems { ... };
    let exit = daemon::run(current_config.clone(), host.clone(), port, subsystems).await?;
    match exit {
        DaemonExit::Shutdown => break,
        DaemonExit::Reload => {
            current_config = Config::load_or_init().await?;
            current_config.apply_env_overrides();
        }
    }
}

// after
let subsystems = DaemonSubsystems { ... };  // SpawnArgs-based 新签名
daemon::run(config, host, port, subsystems).await?;
// daemon::run 不会返回 Reload；reload 在它内部循环里完成
```

**E2**：构造 SpawnArgs 闭包（每个 subsystem 一个），把 canvas_store 等共享资源 capture 进去。

**影响域**：仅 `main.rs` 启动路径。

---

## 4. 影响域汇总

### 4.1 文件级影响

| 文件 | 改动类型 | 行数估计 | 主要改动 |
|---|---|---|---|
| `crates/kylinbot-runtime/src/daemon/reload.rs` | **新增** | ~80 | Subsystem 枚举、ReloadRequest、diff_subsystems |
| `crates/kylinbot-runtime/src/daemon/worker_pool.rs` | **新增** | ~100 | ChannelWorkerPool 结构与方法 |
| `crates/kylinbot-runtime/src/daemon/mod.rs` | 大改 | ~120 | run() 改造、handles 改 HashMap、tokio::select! 主循环 |
| `crates/kylinbot-channels/src/orchestrator/mod.rs` | 中改 | ~80 | start_channels 加参数、dispatch_loop 去 JoinSet、tokio::select! 包 cancel_token |
| `crates/kylinbot-gateway/src/lib.rs` | 中改 | ~50 | AppState.reload_tx 类型升级、run_gateway 接 cancel_token |
| `crates/kylinbot-gateway/src/api.rs` | 小改 | ~30 | reload_config_and_daemon 算 diff |
| `crates/kylinbot-config/src/schema.rs` | 小改 | ~10 | 给若干 config 结构加 derive(PartialEq, Eq) |
| `src/main.rs` | 中改 | ~50 | DaemonSubsystems 新签名、删外层 loop |
| 测试代码 | 新增 | ~200 | worker pool 单测、diff 单测、daemon reload 集成测、orchestrator 集成测 |
| **合计** | | **~720** | |

### 4.2 行为影响

| 资源 | 改动前 | 改动后（Phase 1+） |
|---|---|---|
| 正在跑的 agent 推理 | reload 全部中断 | **跑完后才结束**（核心目标） |
| Channel WS（lansenger/lark/telegram…） | reload 全断 | 仅 channels diff 命中时才断，listener 显式 abort 而非 detach |
| Gateway HTTP/WS | reload 全部中断 | 仅 gateway diff 命中时才断 |
| Cron 任务 | reload 全部 abort | 仅 cron diff 命中时才动 |
| Heartbeat / MQTT | reload 全部重启 | 仅各自 diff 命中时才动 |
| 进程 PID | 不变 | 不变 |
| AppState.config 内存值 | reload 时替换 | **每次 reload_config_and_daemon 调用都立即替换**，与是否触发 subsystem 重启无关 |
| 旧 worker 持有的 Context Arc | （不存在 worker 跨 reload） | **使用旧 config 跑完**（接受这个语义，见 §5） |
| 用户 `/stop` 命令 | 影响所有 in-flight | 仅影响新 dispatch loop 的 in-flight；旧 worker 不可中断 |

### 4.3 接口兼容性

| 接口 | 兼容性 |
|---|---|
| HTTP `/api/*` 端点 | 完全兼容，前端无需改 |
| 配置文件 schema | 完全兼容，仅给 struct 加 derive |
| `AppState` 公开字段 | **不兼容**：reload_tx 类型变了。如果有第三方 crate 引用 AppState（仓库内搜），需要同步改 |
| `DaemonSubsystems` 结构 | **不兼容**：内部 API；只有 `src/main.rs` 用 |
| `start_channels` 函数签名 | **不兼容**：加了 2 个参数。仓库内搜确认仅 `main.rs` 调用 |

---

## 5. 边界情况与已知取舍

### 5.1 旧 worker 持旧凭证 / 旧 channel Arc

**现象**：reload 后，正在跑的 worker 仍持有 reload 前构建的 `Arc<ChannelRuntimeContext>`，其中包含旧 channel 实例。worker 完成推理后调 `channel.send()` 时，用的是旧凭证 / 旧 HTTP client。

**影响场景**：

- ✅ 改了 model：worker 用旧 model 完成推理 —— **符合用户期望**（不打断 = 用旧的跑完）
- ✅ 改了 channel.lark 凭证：worker 走旧 channel 实例 send，用旧 access_token。可能因为 access_token 还在有效期内成功；可能失败。
- ⚠️ 删除了 channel：worker 仍能 send，远端可能因为 secret 错误拒绝。

**对策**：接受这个语义。文档明确说明。必要时给 `channel.send()` 加超时（防止旧凭证导致卡死）。

### 5.2 WeCom Bot 模式的 outgoing WS

**现象**：WeCom Bot 模式 inbound + outbound 共享一条 WS（`outgoing_tx` 写到 channel listener 持有的 WS 上）。listener 被 cancel 后 WS 关闭，旧 worker send 时 `outgoing_tx.send()` 返回错误（receiver 已 drop）。

**影响**：reload 瞬间 1-N 个回复可能丢失。

**对策**：Phase 1+ 接受这个损失，给用户提示。Phase 2 时可以：

- 把 outgoing WS 提到 daemon 级 pool
- 或者让 channel 在 listen 退出时不立即关闭底层 WS，给 send 留窗口期

### 5.3 /stop 跨 reload 失效

**现象**：`in_flight_by_sender: Arc<Mutex<HashMap>>` 生在 dispatch loop 内，dispatch loop 被 cancel 后 map 丢。新 dispatch loop 是新 map，看不到旧 worker。

**影响**：用户 reload 后立刻发 `/stop`，旧 worker 不会被取消。

**对策**：可接受（旧 worker 反正快跑完了）。Phase 2 可以把这个 map 也提到 daemon 级。

### 5.4 并发上限 max_in_flight 跨 reload

**现象**：semaphore 提到 ChannelWorkerPool 内后，所有 worker（新旧）共享同一个 semaphore，全局上限是配置值。

**影响**：

- ✅ 不会因为新旧 supervisor 各持 sem 导致短暂翻倍
- ⚠️ 如果用户在 reload 时改了 `runtime.max_in_flight_messages`，**改动不会立即生效**（pool 用的是初始化时的值）。Phase 2 可以让 pool 支持 dynamic resize（用 `tokio::sync::Semaphore::add_permits`，但减少 permit 需要自己实现），暂时记为 known issue。

### 5.5 schema 加 PartialEq 的兼容性

**风险**：某些字段可能不支持 `PartialEq` derive（如 `Arc<dyn Trait>`、回调闭包）。

**对策**：实施时按编译错误一个一个看。多数 channel/gateway/cron config 都是纯数据结构，应该 OK。实在不行就 `#[derivative(PartialEq="ignore")]` 跳过那个字段（用 derivative crate 或手写 PartialEq）。

### 5.6 reload 失败的回滚

**风险**：spawn 新 supervisor 时启动失败（端口冲突、新 channel 凭证错误等），系统进入降级状态。

**对策**：Phase 1+ 选择 fail-soft —— spawn 新 supervisor 后不立即 await，让 supervisor 自己 retry（已有的 spawn_component_supervisor 退避重试逻辑保留）。旧 supervisor 已被 cancel 不可恢复 —— 这是必然代价。

### 5.7 reload mpsc 满载

**现象**：`mpsc<ReloadRequest>` 容量 8，连续触发 9+ 次 reload 时 `try_send` 返回 Err。

**对策**：让 try_send 失败时返回 503 给前端（前端 retry）。或者把 channel 大小调大（如 32）。

---

## 6. 测试策略

### 6.1 单元测试

| 测试目标 | 位置 |
|---|---|
| `diff_subsystems` 各种场景（空、单字段、跨子系统） | `daemon/reload.rs` |
| `ChannelWorkerPool::spawn_worker` 正常 spawn / drain / active 计数 | `daemon/worker_pool.rs` |
| `ChannelWorkerPool::drain_with_timeout` 超时与成功路径 | 同上 |
| schema 各 config 结构 PartialEq 行为 | `kylinbot-config/src/schema.rs` |

### 6.2 集成测试（新增 `crates/kylinbot-runtime/tests/reload_integration.rs`）

| 测试场景 | 期望 |
|---|---|
| **核心**：reload 触发 Channels diff，且 worker 正在跑 | worker 跑完后 future 返回 Ok，未被 abort |
| reload 触发 Cron diff | channels handle 与 worker_pool 都不动 |
| reload 触发空 diff（仅 UI 字段） | 所有 handle 不动 |
| reload 触发 Gateway diff | gateway 重启，channels 不动 |
| SIGINT 进程退出 | 所有 subsystem 收到 cancel + worker pool drain |
| 旧 worker 用旧 Provider Arc 跑完后 send | send 调用成功（用 mock channel 验证） |

### 6.3 手工验证清单（在真实环境跑）

1. ✅ 启动 daemon，dashboard 打开
2. ✅ 用 channel 触发一个长时间任务（如 LLM 推理 10s+）
3. ✅ 推理进行中，在 Web 上保存一次 channel 绑定（任意 channel）
4. ✅ 观察：
   - dashboard WS 不断（前端不刷新页面）
   - 推理完成，回复正常送达
   - 日志能看到 "channels supervisor: cancel received" 与 "supervisor restarted"
5. ✅ 改 cron 配置保存，channel WS 不断
6. ✅ 改 gateway.port 保存，gateway 重启（WS 必然断），channels 不动
7. ✅ Ctrl+C 关闭，观察 worker drain 日志

---

## 7. 回滚策略

如果 Phase 1+ 出现严重问题（例如旧 worker 内存泄漏、cancel_token 在某些 channel 不生效导致 hang），有两级回滚：

### 7.1 配置开关（最快）

引入 `[daemon] graceful_reload = true`（默认 true）。设为 false 时退化为今天的行为：

- diff 函数始终返回 `{Gateway, Channels, Cron, Heartbeat, Mqtt}` 全集
- ChannelWorkerPool 替换为本地 JoinSet（在 dispatch_loop 内部 fallback）

这个开关把"细粒度"和"worker 解耦"两个特性合在一起退化，是最简单的紧急按钮。

### 7.2 完整回滚（git revert）

由于改动横跨多个 crate，且 schema 加了 derive，纯 revert 一个 commit 不一定干净。建议：

- 实施时拆成 3-4 个独立 commit（schema derive → worker_pool → daemon refactor → orchestrator refactor → main.rs/gateway 串通）
- 每个 commit 单独可 revert
- 紧急情况按 7.1 走开关，不走 revert

---

## 8. 实施顺序

| 阶段 | 内容 | 估计 |
|---|---|---|
| 0 | 写设计文档（本文）+ team review | 0.5d |
| 1 | schema 加 PartialEq derive，编译通过 | 0.5d |
| 2 | 新增 `daemon/reload.rs` + `daemon/worker_pool.rs`，含单测 | 0.5d |
| 3 | 改造 `daemon::run`，更新 DaemonSubsystems 签名 | 1d |
| 4 | 改造 `orchestrator::start_channels` 与 `run_message_dispatch_loop` | 0.5d |
| 5 | 改造 `gateway::api::reload_config_and_daemon` + AppState | 0.5d |
| 6 | 改造 `src/main.rs` 删外层 loop | 0.5d |
| 7 | 集成测试 + 手工验证 + 修 bug | 1d |
| **合计** | | **4.5d** |

---

## 9. 后续工作（不在本次范围）

Phase 2 候选项（按优先级）：

1. **单 channel reload**：`channels.lark` 改动不重启 `channels.lansenger`。需要 channels orchestrator 内维护 per-channel handle map，按 channel name 算 diff。
2. **outgoing WS 共享**：把 WeCom Bot 等需要 WS 出站的 channel 的 WS 提到 daemon 级，listen 重启时 send 通道不断。
3. **动态 max_in_flight**：ChannelWorkerPool semaphore 支持运行时调整 permit 数。
4. **零停机 gateway 重启**：SO_REUSEPORT + 双进程接力，仅在 gateway 自身配置改时使用。
5. **reload 速率限制 / debounce**：多次连续保存合并为一次 reload。

---

## 10. 验收标准

Phase 1+ 视为完成，当且仅当：

1. ✅ 所有改动落到上述 8 个文件，编译通过 (`cargo build --workspace --all-features`)
2. ✅ 所有单测 + 集成测试通过 (`cargo test --workspace`)
3. ✅ 手工验证清单（§6.3）全部通过
4. ✅ 文档（本文）记录所有取舍与已知问题
5. ✅ commit 拆分清晰，每个 commit 可独立 revert
6. ✅ 至少 1 人 review 通过
