use chrono::{DateTime, Utc};
use std::collections::HashMap;
use std::sync::{Mutex, OnceLock};

#[derive(Debug, Clone)]
struct AgentRuntimeState {
    active_tasks: u32,
    status: String,
    stage: String,
    current_task: Option<String>,
    started_at: Option<DateTime<Utc>>,
    last_started_at: Option<DateTime<Utc>>,
    last_finished_at: Option<DateTime<Utc>>,
    last_error: Option<String>,
}

#[derive(Debug, Clone)]
pub struct AgentRuntimeSnapshot {
    pub status: String,
    pub stage: String,
    pub active_tasks: u32,
    pub current_task: Option<String>,
    pub started_at: Option<String>,
    pub last_started_at: Option<String>,
    pub last_finished_at: Option<String>,
    pub last_error: Option<String>,
}

fn registry() -> &'static Mutex<HashMap<String, AgentRuntimeState>> {
    static REGISTRY: OnceLock<Mutex<HashMap<String, AgentRuntimeState>>> = OnceLock::new();
    REGISTRY.get_or_init(|| Mutex::new(HashMap::new()))
}

pub fn mark_started(agent_name: &str, task_summary: impl Into<String>, stage: &str) {
    let now = Utc::now();
    let summary = task_summary.into();
    let mut guard = registry().lock().unwrap_or_else(|e| e.into_inner());
    let entry = guard
        .entry(agent_name.to_string())
        .or_insert_with(|| AgentRuntimeState {
            active_tasks: 0,
            status: "idle".to_string(),
            stage: "idle".to_string(),
            current_task: None,
            started_at: None,
            last_started_at: None,
            last_finished_at: None,
            last_error: None,
        });

    entry.active_tasks = entry.active_tasks.saturating_add(1);
    entry.status = "running".to_string();
    entry.stage = stage.to_string();
    entry.current_task = Some(summary);
    entry.started_at.get_or_insert(now);
    entry.last_started_at = Some(now);
    entry.last_error = None;
}

pub fn mark_finished(agent_name: &str, success: bool, error: Option<String>) {
    let now = Utc::now();
    let mut guard = registry().lock().unwrap_or_else(|e| e.into_inner());
    let Some(entry) = guard.get_mut(agent_name) else {
        return;
    };

    entry.active_tasks = entry.active_tasks.saturating_sub(1);
    entry.last_finished_at = Some(now);

    if let Some(err) = error {
        entry.last_error = Some(err);
    } else if success {
        entry.last_error = None;
    }

    if entry.active_tasks == 0 {
        entry.status = if success { "idle" } else { "error" }.to_string();
        entry.stage = if success { "completed" } else { "failed" }.to_string();
        entry.current_task = None;
        entry.started_at = None;
    } else {
        entry.status = "running".to_string();
        entry.stage = "running".to_string();
    }
}

pub fn snapshot(agent_name: &str) -> AgentRuntimeSnapshot {
    let guard = registry().lock().unwrap_or_else(|e| e.into_inner());
    let state = guard.get(agent_name);

    AgentRuntimeSnapshot {
        status: state
            .map(|s| s.status.clone())
            .unwrap_or_else(|| "idle".to_string()),
        stage: state
            .map(|s| s.stage.clone())
            .unwrap_or_else(|| "idle".to_string()),
        active_tasks: state.map(|s| s.active_tasks).unwrap_or(0),
        current_task: state.and_then(|s| s.current_task.clone()),
        started_at: state.and_then(|s| s.started_at.map(|v| v.to_rfc3339())),
        last_started_at: state.and_then(|s| s.last_started_at.map(|v| v.to_rfc3339())),
        last_finished_at: state.and_then(|s| s.last_finished_at.map(|v| v.to_rfc3339())),
        last_error: state.and_then(|s| s.last_error.clone()),
    }
}

#[cfg(test)]
pub fn reset_for_tests() {
    let mut guard = registry().lock().unwrap_or_else(|e| e.into_inner());
    guard.clear();
}

#[cfg(test)]
mod tests {
    use super::{mark_finished, mark_started, reset_for_tests, snapshot};

    #[test]
    fn runtime_snapshot_tracks_running_and_idle_states() {
        reset_for_tests();
        mark_started("researcher", "inspect logs", "running");

        let running = snapshot("researcher");
        assert_eq!(running.status, "running");
        assert_eq!(running.stage, "running");
        assert_eq!(running.active_tasks, 1);
        assert_eq!(running.current_task.as_deref(), Some("inspect logs"));
        assert!(running.started_at.is_some());

        mark_finished("researcher", true, None);

        let idle = snapshot("researcher");
        assert_eq!(idle.status, "idle");
        assert_eq!(idle.stage, "completed");
        assert_eq!(idle.active_tasks, 0);
        assert!(idle.current_task.is_none());
        assert!(idle.last_finished_at.is_some());
    }
}
