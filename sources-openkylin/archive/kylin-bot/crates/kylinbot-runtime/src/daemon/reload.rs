//! Subsystem-level reload protocol.
//!
//! `daemon::run` listens for `ReloadRequest`s on an `mpsc` channel and only
//! restarts the subsystems listed in `affected`. The gateway computes the
//! affected set from a config diff before sending the request, so a typical
//! channel-binding save no longer disturbs the gateway HTTP/WS server, cron,
//! or heartbeat — and in-flight agent inference (which lives in the daemon
//! level [`ChannelWorkerPool`](super::worker_pool::ChannelWorkerPool))
//! continues uninterrupted.
//!
//! Diffing uses `serde_json::Value` rather than a `PartialEq` derive cascade
//! on `Config`, so adding a new field to any sub-config doesn't require
//! propagating `PartialEq` through dozens of nested structs.

use kylinbot_config::schema::Config;
use serde::Serialize;
use std::collections::HashSet;

/// One of the daemon-supervised subsystems that can be reloaded independently.
#[derive(Debug, Clone, Copy, Hash, PartialEq, Eq)]
pub enum Subsystem {
    /// HTTP / WebSocket API server.
    Gateway,
    /// All chat / messaging channels (Telegram, Lark, Lansenger, …).
    Channels,
    /// Scheduled tasks.
    Cron,
    /// Periodic health pings.
    Heartbeat,
    /// MQTT SOP listener.
    Mqtt,
}

impl Subsystem {
    pub fn as_str(self) -> &'static str {
        match self {
            Subsystem::Gateway => "gateway",
            Subsystem::Channels => "channels",
            Subsystem::Cron => "cron",
            Subsystem::Heartbeat => "heartbeat",
            Subsystem::Mqtt => "mqtt",
        }
    }
}

/// A reload request handed to the daemon supervisor.
///
/// `new_config` is the already-validated, env-overlaid `Config` to use when
/// (re)spawning each subsystem in `affected`. Subsystems that are not in
/// `affected` are left running untouched.
#[derive(Debug, Clone)]
pub struct ReloadRequest {
    pub new_config: Config,
    pub affected: HashSet<Subsystem>,
}

/// Compute the set of subsystems whose effective configuration changed
/// between `old` and `new`.
///
/// The comparison happens via `serde_json::to_value` to avoid forcing a
/// `PartialEq` derive cascade through every nested config struct.
/// Serialization is only invoked on reload (rare), so the cost is negligible.
pub fn diff_subsystems(old: &Config, new: &Config) -> HashSet<Subsystem> {
    let mut out = HashSet::new();

    if !json_eq(&old.channels, &new.channels) {
        out.insert(Subsystem::Channels);
    }
    if !json_eq(&old.gateway, &new.gateway) {
        out.insert(Subsystem::Gateway);
    }
    if !json_eq(&old.cron, &new.cron) {
        out.insert(Subsystem::Cron);
    }
    if !json_eq(&old.heartbeat, &new.heartbeat) {
        out.insert(Subsystem::Heartbeat);
    }
    if !json_eq(&old.channels.mqtt, &new.channels.mqtt) {
        out.insert(Subsystem::Mqtt);
    }

    out
}

fn json_eq<T: Serialize>(a: &T, b: &T) -> bool {
    match (serde_json::to_value(a), serde_json::to_value(b)) {
        (Ok(av), Ok(bv)) => av == bv,
        // If serialization fails for either side, fall back to "they differ"
        // so the subsystem reloads. Better to over-reload than to miss a real
        // change, since serialization failure is unexpected for a Config.
        _ => true,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn base_config() -> Config {
        Config::default()
    }

    #[test]
    fn diff_empty_when_configs_identical() {
        let a = base_config();
        let b = a.clone();
        assert!(diff_subsystems(&a, &b).is_empty());
    }

    #[test]
    fn diff_detects_channels_change() {
        let a = base_config();
        let mut b = a.clone();
        // ChannelsConfig has no top-level `enabled`; toggle `cli` which is a
        // pure bool field that does not implicate any nested mqtt structure.
        b.channels.cli = !a.channels.cli;
        let affected = diff_subsystems(&a, &b);
        assert!(affected.contains(&Subsystem::Channels));
        assert!(!affected.contains(&Subsystem::Mqtt));
    }

    #[test]
    fn diff_detects_cron_change() {
        let a = base_config();
        let mut b = a.clone();
        b.cron.enabled = !a.cron.enabled;
        let affected = diff_subsystems(&a, &b);
        assert_eq!(
            affected,
            HashSet::from_iter([Subsystem::Cron].into_iter())
        );
    }

    #[test]
    fn diff_detects_gateway_change() {
        let a = base_config();
        let mut b = a.clone();
        b.gateway.port = a.gateway.port.wrapping_add(1);
        let affected = diff_subsystems(&a, &b);
        assert!(affected.contains(&Subsystem::Gateway));
    }

    #[test]
    fn diff_detects_heartbeat_change() {
        let a = base_config();
        let mut b = a.clone();
        b.heartbeat.enabled = !a.heartbeat.enabled;
        let affected = diff_subsystems(&a, &b);
        assert!(affected.contains(&Subsystem::Heartbeat));
    }

    #[test]
    fn diff_detects_mqtt_change_via_channels() {
        let a = base_config();
        let mut b = a.clone();
        // Mqtt lives under channels. Toggling it must surface both flags.
        let mut mqtt = b.channels.mqtt.clone().unwrap_or_default();
        mqtt.enabled = !mqtt.enabled;
        b.channels.mqtt = Some(mqtt);
        let affected = diff_subsystems(&a, &b);
        assert!(affected.contains(&Subsystem::Channels));
        assert!(affected.contains(&Subsystem::Mqtt));
    }
}
