use super::timezone::{
    cost_stats_local_date, cost_stats_local_year_month, current_cost_stats_period,
};
use super::types::{BudgetCheck, CostRecord, CostSummary, ModelStats, TokenUsage, UsagePeriod};
use crate::schema::CostConfig;
use anyhow::{Context, Result, anyhow};
use chrono::NaiveDate;
use parking_lot::{Mutex, MutexGuard};
use std::collections::{HashMap, HashSet};
use std::fs::{self, File, OpenOptions};
use std::io::{BufRead, BufReader, Write};
use std::path::{Path, PathBuf};
use std::sync::{Arc, OnceLock};

/// Cost tracker for API usage monitoring and budget enforcement.
pub struct CostTracker {
    config: CostConfig,
    storage: Arc<Mutex<CostStorage>>,
    session_id: String,
}

impl CostTracker {
    /// Create a new cost tracker.
    pub fn new(config: CostConfig, workspace_dir: &Path) -> Result<Self> {
        let storage_path = resolve_storage_path(workspace_dir)?;

        let storage = CostStorage::new(&storage_path).with_context(|| {
            format!("Failed to open cost storage at {}", storage_path.display())
        })?;

        let session_id = load_or_create_session_id(&storage_path, &storage)?;

        Ok(Self {
            config,
            storage: Arc::new(Mutex::new(storage)),
            session_id,
        })
    }

    /// Get the session ID.
    pub fn session_id(&self) -> &str {
        &self.session_id
    }

    fn lock_storage(&self) -> MutexGuard<'_, CostStorage> {
        self.storage.lock()
    }

    /// Check if a request is within budget.
    pub fn check_budget(&self, estimated_cost_usd: f64) -> Result<BudgetCheck> {
        if !self.config.enabled {
            return Ok(BudgetCheck::Allowed);
        }

        if !estimated_cost_usd.is_finite() || estimated_cost_usd < 0.0 {
            return Err(anyhow!(
                "Estimated cost must be a finite, non-negative value"
            ));
        }

        let mut storage = self.lock_storage();
        let (daily_cost, monthly_cost) = storage.get_aggregated_costs()?;

        // Check daily limit
        let projected_daily = daily_cost + estimated_cost_usd;
        if projected_daily > self.config.daily_limit_usd {
            return Ok(BudgetCheck::Exceeded {
                current_usd: daily_cost,
                limit_usd: self.config.daily_limit_usd,
                period: UsagePeriod::Day,
            });
        }

        // Check monthly limit
        let projected_monthly = monthly_cost + estimated_cost_usd;
        if projected_monthly > self.config.monthly_limit_usd {
            return Ok(BudgetCheck::Exceeded {
                current_usd: monthly_cost,
                limit_usd: self.config.monthly_limit_usd,
                period: UsagePeriod::Month,
            });
        }

        // Check warning thresholds
        let warn_threshold = f64::from(self.config.warn_at_percent.min(100)) / 100.0;
        let daily_warn_threshold = self.config.daily_limit_usd * warn_threshold;
        let monthly_warn_threshold = self.config.monthly_limit_usd * warn_threshold;

        if projected_daily >= daily_warn_threshold {
            return Ok(BudgetCheck::Warning {
                current_usd: daily_cost,
                limit_usd: self.config.daily_limit_usd,
                period: UsagePeriod::Day,
            });
        }

        if projected_monthly >= monthly_warn_threshold {
            return Ok(BudgetCheck::Warning {
                current_usd: monthly_cost,
                limit_usd: self.config.monthly_limit_usd,
                period: UsagePeriod::Month,
            });
        }

        Ok(BudgetCheck::Allowed)
    }

    /// Record a usage event.
    ///
    /// When `configured_models` is `Some`, usage for model ids that are not in the
    /// set is skipped (e.g. model removed from config while a turn is still running).
    pub fn record_usage(
        &self,
        usage: TokenUsage,
        configured_models: Option<&std::collections::HashSet<String>>,
    ) -> Result<()> {
        if !self.config.enabled {
            return Ok(());
        }

        if !usage.cost_usd.is_finite() || usage.cost_usd < 0.0 {
            return Err(anyhow!(
                "Token usage cost must be a finite, non-negative value"
            ));
        }

        if let Some(configured) = configured_models {
            if !configured.contains(&usage.model) {
                tracing::debug!(
                    model = %usage.model,
                    "Skipping cost record for model not in providers.models"
                );
                return Ok(());
            }
        }

        let record = CostRecord::new(&self.session_id, usage);

        let mut storage = self.lock_storage();
        storage.add_record(record)?;

        Ok(())
    }

    /// Get the current cost summary.
    pub fn get_summary(&self) -> Result<CostSummary> {
        let (session_cost, daily_cost, monthly_cost, total_tokens, request_count, by_model) = {
            let mut storage = self.lock_storage();
            let session_cost = storage.session_cost_for(&self.session_id)?;
            let (daily_cost, monthly_cost) = storage.get_aggregated_costs()?;
            let (total_tokens, request_count, by_model) = storage.get_all_time_stats()?;
            (
                session_cost,
                daily_cost,
                monthly_cost,
                total_tokens,
                request_count,
                by_model,
            )
        };

        Ok(CostSummary {
            session_cost_usd: session_cost,
            daily_cost_usd: daily_cost,
            monthly_cost_usd: monthly_cost,
            total_tokens,
            request_count,
            by_model,
        })
    }

    /// Get the daily cost for a specific calendar date in [`super::timezone::COST_STATS_TIMEZONE`].
    pub fn get_daily_cost(&self, date: NaiveDate) -> Result<f64> {
        let storage = self.lock_storage();
        storage.get_cost_for_date(date)
    }

    /// Get the monthly cost for a specific calendar month in [`super::timezone::COST_STATS_TIMEZONE`].
    pub fn get_monthly_cost(&self, year: i32, month: u32) -> Result<f64> {
        let storage = self.lock_storage();
        storage.get_cost_for_month(year, month)
    }

    /// Remove all persisted and in-memory cost records for the given model names.
    pub fn purge_models(&self, models: &[String]) -> Result<usize> {
        if models.is_empty() {
            return Ok(0);
        }

        let model_set: HashSet<&str> = models.iter().map(String::as_str).collect();

        let mut storage = self.lock_storage();
        storage.purge_models(&model_set)
    }
}

// ── Process-global singleton ────────────────────────────────────────
// Both the gateway and the channels supervisor share a single CostTracker
// so that budget enforcement is consistent across all paths.

static GLOBAL_COST_TRACKER: OnceLock<Option<Arc<CostTracker>>> = OnceLock::new();

impl CostTracker {
    /// Return the process-global `CostTracker`, creating it on first call.
    /// Subsequent calls (from gateway or channels, whichever starts second)
    /// receive the same `Arc`.  Returns `None` when cost tracking is disabled
    /// or initialisation fails.
    pub fn get_or_init_global(config: CostConfig, workspace_dir: &Path) -> Option<Arc<Self>> {
        GLOBAL_COST_TRACKER
            .get_or_init(|| {
                if !config.enabled {
                    return None;
                }
                match Self::new(config, workspace_dir) {
                    Ok(ct) => Some(Arc::new(ct)),
                    Err(e) => {
                        tracing::warn!("Failed to initialize global cost tracker: {e}");
                        None
                    }
                }
            })
            .clone()
    }
}

fn resolve_session_id_path(storage_path: &Path) -> PathBuf {
    storage_path
        .parent()
        .map(|dir| dir.join("cost_session_id"))
        .unwrap_or_else(|| PathBuf::from("cost_session_id"))
}

fn persist_session_id(storage_path: &Path, session_id: &str) -> Result<()> {
    let path = resolve_session_id_path(storage_path);
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)
            .with_context(|| format!("Failed to create directory {}", parent.display()))?;
    }
    fs::write(&path, format!("{session_id}\n"))
        .with_context(|| format!("Failed to write cost session id to {}", path.display()))
}

fn load_or_create_session_id(
    storage_path: &Path,
    storage: &CostStorage,
) -> Result<String> {
    let path = resolve_session_id_path(storage_path);
    if path.is_file() {
        let raw = fs::read_to_string(&path)
            .with_context(|| format!("Failed to read cost session id from {}", path.display()))?;
        let id = raw.trim();
        if !id.is_empty() {
            return Ok(id.to_string());
        }
    }

    let session_id = storage
        .latest_session_id()?
        .unwrap_or_else(|| uuid::Uuid::new_v4().to_string());
    persist_session_id(storage_path, &session_id)?;
    Ok(session_id)
}

fn resolve_storage_path(workspace_dir: &Path) -> Result<PathBuf> {
    let storage_path = workspace_dir.join("state").join("costs.jsonl");
    let legacy_path = workspace_dir.join(".kylinbot").join("costs.db");

    if !storage_path.exists() && legacy_path.exists() {
        if let Some(parent) = storage_path.parent() {
            fs::create_dir_all(parent)
                .with_context(|| format!("Failed to create directory {}", parent.display()))?;
        }

        if let Err(error) = fs::rename(&legacy_path, &storage_path) {
            tracing::warn!(
                "Failed to move legacy cost storage from {} to {}: {error}; falling back to copy",
                legacy_path.display(),
                storage_path.display()
            );
            fs::copy(&legacy_path, &storage_path).with_context(|| {
                format!(
                    "Failed to copy legacy cost storage from {} to {}",
                    legacy_path.display(),
                    storage_path.display()
                )
            })?;
        }
    }

    Ok(storage_path)
}

/// Persistent storage for cost records.
struct CostStorage {
    path: PathBuf,
    daily_cost_usd: f64,
    monthly_cost_usd: f64,
    cached_day: NaiveDate,
    cached_year: i32,
    cached_month: u32,
}

impl CostStorage {
    /// Create or open cost storage.
    fn new(path: &Path) -> Result<Self> {
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent)
                .with_context(|| format!("Failed to create directory {}", parent.display()))?;
        }

        let (day, year, month) = current_cost_stats_period();
        let mut storage = Self {
            path: path.to_path_buf(),
            daily_cost_usd: 0.0,
            monthly_cost_usd: 0.0,
            cached_day: day,
            cached_year: year,
            cached_month: month,
        };

        storage.rebuild_aggregates(
            storage.cached_day,
            storage.cached_year,
            storage.cached_month,
        )?;

        Ok(storage)
    }

    fn for_each_record<F>(&self, mut on_record: F) -> Result<()>
    where
        F: FnMut(CostRecord),
    {
        if !self.path.exists() {
            return Ok(());
        }

        let file = File::open(&self.path)
            .with_context(|| format!("Failed to read cost storage from {}", self.path.display()))?;
        let reader = BufReader::new(file);

        for (line_number, line) in reader.lines().enumerate() {
            let raw_line = line.with_context(|| {
                format!(
                    "Failed to read line {} from cost storage {}",
                    line_number + 1,
                    self.path.display()
                )
            })?;

            let trimmed = raw_line.trim();
            if trimmed.is_empty() {
                continue;
            }

            match serde_json::from_str::<CostRecord>(trimmed) {
                Ok(record) => on_record(record),
                Err(error) => {
                    tracing::warn!(
                        "Skipping malformed cost record at {}:{}: {error}",
                        self.path.display(),
                        line_number + 1
                    );
                }
            }
        }

        Ok(())
    }

    fn rebuild_aggregates(&mut self, day: NaiveDate, year: i32, month: u32) -> Result<()> {
        let mut daily_cost = 0.0;
        let mut monthly_cost = 0.0;

        self.for_each_record(|record| {
            let ts = record.usage.timestamp;

            if cost_stats_local_date(ts) == day {
                daily_cost += record.usage.cost_usd;
            }

            let (record_year, record_month) = cost_stats_local_year_month(ts);
            if record_year == year && record_month == month {
                monthly_cost += record.usage.cost_usd;
            }
        })?;

        self.daily_cost_usd = daily_cost;
        self.monthly_cost_usd = monthly_cost;
        self.cached_day = day;
        self.cached_year = year;
        self.cached_month = month;

        Ok(())
    }

    fn ensure_period_cache_current(&mut self) -> Result<()> {
        let (day, year, month) = current_cost_stats_period();

        if day != self.cached_day || year != self.cached_year || month != self.cached_month {
            self.rebuild_aggregates(day, year, month)?;
        }

        Ok(())
    }

    /// Add a new record.
    fn add_record(&mut self, record: CostRecord) -> Result<()> {
        let mut file = OpenOptions::new()
            .create(true)
            .append(true)
            .open(&self.path)
            .with_context(|| format!("Failed to open cost storage at {}", self.path.display()))?;

        writeln!(file, "{}", serde_json::to_string(&record)?)
            .with_context(|| format!("Failed to write cost record to {}", self.path.display()))?;
        file.sync_all()
            .with_context(|| format!("Failed to sync cost storage at {}", self.path.display()))?;

        self.ensure_period_cache_current()?;

        let ts = record.usage.timestamp;
        if cost_stats_local_date(ts) == self.cached_day {
            self.daily_cost_usd += record.usage.cost_usd;
        }
        let (year, month) = cost_stats_local_year_month(ts);
        if year == self.cached_year && month == self.cached_month {
            self.monthly_cost_usd += record.usage.cost_usd;
        }

        Ok(())
    }

    /// Get aggregated costs for current day and month.
    fn get_aggregated_costs(&mut self) -> Result<(f64, f64)> {
        self.ensure_period_cache_current()?;
        Ok((self.daily_cost_usd, self.monthly_cost_usd))
    }

    /// Sum persisted cost for a single usage session id.
    fn session_cost_for(&self, session_id: &str) -> Result<f64> {
        let mut cost = 0.0;

        self.for_each_record(|record| {
            if record.session_id == session_id {
                cost += record.usage.cost_usd;
            }
        })?;

        Ok(cost)
    }

    /// Return the session id from the most recent persisted record, if any.
    fn latest_session_id(&self) -> Result<Option<String>> {
        let mut latest: Option<(chrono::DateTime<chrono::Utc>, String)> = None;

        self.for_each_record(|record| {
            let ts = record.usage.timestamp;
            match &latest {
                Some((best_ts, _)) if ts <= *best_ts => {}
                _ => latest = Some((ts, record.session_id.clone())),
            }
        })?;

        Ok(latest.map(|(_, session_id)| session_id))
    }

    /// Aggregate token/request totals and per-model stats from all persisted records.
    fn get_all_time_stats(&self) -> Result<(u64, usize, HashMap<String, ModelStats>)> {
        let mut total_tokens = 0u64;
        let mut request_count = 0usize;
        let mut by_model: HashMap<String, ModelStats> = HashMap::new();

        self.for_each_record(|record| {
            total_tokens = total_tokens.saturating_add(record.usage.total_tokens);
            request_count += 1;

            let entry = by_model
                .entry(record.usage.model.clone())
                .or_insert_with(|| ModelStats {
                    model: record.usage.model.clone(),
                    cost_usd: 0.0,
                    total_tokens: 0,
                    request_count: 0,
                });

            entry.cost_usd += record.usage.cost_usd;
            entry.total_tokens += record.usage.total_tokens;
            entry.request_count += 1;
        })?;

        Ok((total_tokens, request_count, by_model))
    }

    /// Get cost for a specific date.
    fn get_cost_for_date(&self, date: NaiveDate) -> Result<f64> {
        let mut cost = 0.0;

        self.for_each_record(|record| {
            if cost_stats_local_date(record.usage.timestamp) == date {
                cost += record.usage.cost_usd;
            }
        })?;

        Ok(cost)
    }

    /// Get cost for a specific month.
    fn get_cost_for_month(&self, year: i32, month: u32) -> Result<f64> {
        let mut cost = 0.0;

        self.for_each_record(|record| {
            let (record_year, record_month) = cost_stats_local_year_month(record.usage.timestamp);
            if record_year == year && record_month == month {
                cost += record.usage.cost_usd;
            }
        })?;

        Ok(cost)
    }

    /// Remove all records whose `usage.model` matches one of `models`.
    fn purge_models(&mut self, models: &HashSet<&str>) -> Result<usize> {
        if models.is_empty() {
            return Ok(0);
        }

        let mut kept_lines: Vec<String> = Vec::new();
        let mut removed = 0usize;

        self.for_each_record(|record| {
            if models.contains(record.usage.model.as_str()) {
                removed += 1;
            } else {
                kept_lines.push(serde_json::to_string(&record).expect("CostRecord serializes"));
            }
        })?;

        if removed == 0 {
            return Ok(0);
        }

        let parent = self.path.parent().ok_or_else(|| {
            anyhow!(
                "Cost storage path {} has no parent directory",
                self.path.display()
            )
        })?;
        fs::create_dir_all(parent)
            .with_context(|| format!("Failed to create directory {}", parent.display()))?;

        let temp_path = self.path.with_extension("jsonl.tmp");
        {
            let mut file = File::create(&temp_path).with_context(|| {
                format!(
                    "Failed to create temporary cost storage at {}",
                    temp_path.display()
                )
            })?;
            for line in &kept_lines {
                writeln!(file, "{line}").with_context(|| {
                    format!(
                        "Failed to write temporary cost storage at {}",
                        temp_path.display()
                    )
                })?;
            }
            file.sync_all().with_context(|| {
                format!(
                    "Failed to sync temporary cost storage at {}",
                    temp_path.display()
                )
            })?;
        }

        fs::rename(&temp_path, &self.path).with_context(|| {
            format!(
                "Failed to replace cost storage at {} with {}",
                self.path.display(),
                temp_path.display()
            )
        })?;

        let (day, year, month) = current_cost_stats_period();
        self.rebuild_aggregates(day, year, month)?;

        Ok(removed)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::Utc;
    use tempfile::TempDir;

    fn enabled_config() -> CostConfig {
        CostConfig {
            enabled: true,
            ..Default::default()
        }
    }

    #[test]
    fn cost_tracker_initialization() {
        let tmp = TempDir::new().unwrap();
        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();
        assert!(!tracker.session_id().is_empty());
    }

    #[test]
    fn budget_check_when_disabled() {
        let tmp = TempDir::new().unwrap();
        let config = CostConfig {
            enabled: false,
            ..Default::default()
        };

        let tracker = CostTracker::new(config, tmp.path()).unwrap();
        let check = tracker.check_budget(1000.0).unwrap();
        assert!(matches!(check, BudgetCheck::Allowed));
    }

    #[test]
    fn record_usage_and_get_summary() {
        let tmp = TempDir::new().unwrap();
        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();

        let usage = TokenUsage::new("test/model", 1000, 500, 1.0, 2.0);
        tracker.record_usage(usage, None).unwrap();

        let summary = tracker.get_summary().unwrap();
        assert_eq!(summary.request_count, 1);
        assert!(summary.session_cost_usd > 0.0);
        assert_eq!(summary.by_model.len(), 1);
    }

    #[test]
    fn budget_exceeded_daily_limit() {
        let tmp = TempDir::new().unwrap();
        let config = CostConfig {
            enabled: true,
            daily_limit_usd: 0.01, // Very low limit
            ..Default::default()
        };

        let tracker = CostTracker::new(config, tmp.path()).unwrap();

        // Record a usage that exceeds the limit
        let usage = TokenUsage::new("test/model", 10000, 5000, 1.0, 2.0); // ~0.02 USD
        tracker.record_usage(usage, None).unwrap();

        let check = tracker.check_budget(0.01).unwrap();
        assert!(matches!(check, BudgetCheck::Exceeded { .. }));
    }

    #[test]
    fn summary_includes_persisted_records_after_restart() {
        let tmp = TempDir::new().unwrap();
        let storage_path = resolve_storage_path(tmp.path()).unwrap();
        if let Some(parent) = storage_path.parent() {
            fs::create_dir_all(parent).unwrap();
        }

        let old_session = "old-session";
        let old_record = CostRecord::new(
            old_session,
            TokenUsage::new("legacy/model", 500, 500, 1.0, 1.0),
        );
        let mut file = OpenOptions::new()
            .create(true)
            .append(true)
            .open(&storage_path)
            .unwrap();
        writeln!(file, "{}", serde_json::to_string(&old_record).unwrap()).unwrap();
        file.sync_all().unwrap();
        persist_session_id(&storage_path, old_session).unwrap();

        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();
        assert_eq!(tracker.session_id(), old_session);
        tracker
            .record_usage(
                TokenUsage::new("session/model", 1000, 1000, 1.0, 1.0),
                None,
            )
            .unwrap();

        let summary = tracker.get_summary().unwrap();
        assert_eq!(summary.request_count, 2);
        assert_eq!(summary.total_tokens, 3000);
        assert_eq!(summary.by_model.len(), 2);
        assert!(summary.by_model.contains_key("session/model"));
        assert!(summary.by_model.contains_key("legacy/model"));
        assert!(summary.session_cost_usd > 0.0);
    }

    #[test]
    fn session_cost_survives_tracker_reinit() {
        let tmp = TempDir::new().unwrap();
        let storage_path = resolve_storage_path(tmp.path()).unwrap();
        if let Some(parent) = storage_path.parent() {
            fs::create_dir_all(parent).unwrap();
        }

        let session_id = "persisted-session";
        persist_session_id(&storage_path, session_id).unwrap();

        let usage = TokenUsage::new("test/model", 1000, 500, 1.0, 2.0);
        let record = CostRecord::new(session_id, usage.clone());
        let mut file = OpenOptions::new()
            .create(true)
            .append(true)
            .open(&storage_path)
            .unwrap();
        writeln!(file, "{}", serde_json::to_string(&record).unwrap()).unwrap();
        file.sync_all().unwrap();

        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();
        let summary = tracker.get_summary().unwrap();
        assert!((summary.session_cost_usd - usage.cost_usd).abs() < f64::EPSILON);
    }

    #[test]
    fn malformed_lines_are_ignored_while_loading() {
        let tmp = TempDir::new().unwrap();
        let storage_path = resolve_storage_path(tmp.path()).unwrap();
        if let Some(parent) = storage_path.parent() {
            fs::create_dir_all(parent).unwrap();
        }

        let valid_usage = TokenUsage::new("test/model", 1000, 0, 1.0, 1.0);
        let valid_record = CostRecord::new("session-a", valid_usage.clone());

        let mut file = OpenOptions::new()
            .create(true)
            .append(true)
            .open(storage_path)
            .unwrap();
        writeln!(file, "{}", serde_json::to_string(&valid_record).unwrap()).unwrap();
        writeln!(file, "not-a-json-line").unwrap();
        writeln!(file).unwrap();
        file.sync_all().unwrap();

        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();
        let today_cost = tracker
            .get_daily_cost(cost_stats_local_date(Utc::now()))
            .unwrap();
        assert!((today_cost - valid_usage.cost_usd).abs() < f64::EPSILON);
    }

    #[test]
    fn invalid_budget_estimate_is_rejected() {
        let tmp = TempDir::new().unwrap();
        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();

        let err = tracker.check_budget(f64::NAN).unwrap_err();
        assert!(
            err.to_string()
                .contains("Estimated cost must be a finite, non-negative value")
        );
    }

    #[test]
    fn purge_models_removes_persisted_and_session_records() {
        let tmp = TempDir::new().unwrap();
        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();

        tracker
            .record_usage(TokenUsage::new("keep/model", 1000, 0, 1.0, 1.0), None)
            .unwrap();
        tracker
            .record_usage(TokenUsage::new("remove/model", 2000, 0, 1.0, 1.0), None)
            .unwrap();

        let before = tracker.get_summary().unwrap();
        assert_eq!(before.request_count, 2);
        assert_eq!(before.by_model.len(), 2);
        assert!(before.by_model.contains_key("remove/model"));

        let removed = tracker
            .purge_models(&["remove/model".to_string()])
            .unwrap();
        assert_eq!(removed, 1);

        let after = tracker.get_summary().unwrap();
        assert_eq!(after.request_count, 1);
        assert_eq!(after.total_tokens, 1000);
        assert_eq!(after.by_model.len(), 1);
        assert!(after.by_model.contains_key("keep/model"));
        assert!(!after.by_model.contains_key("remove/model"));
        assert!(after.session_cost_usd <= before.session_cost_usd);
        assert!(after.session_cost_usd >= 0.0);
    }

    #[test]
    fn record_usage_skips_model_not_in_configured_set() {
        let tmp = TempDir::new().unwrap();
        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();
        let configured = std::collections::HashSet::from(["keep/model".to_string()]);

        tracker
            .record_usage(TokenUsage::new("keep/model", 1000, 0, 1.0, 1.0), Some(&configured))
            .unwrap();
        tracker
            .record_usage(
                TokenUsage::new("deleted/model", 2000, 0, 1.0, 1.0),
                Some(&configured),
            )
            .unwrap();

        let summary = tracker.get_summary().unwrap();
        assert_eq!(summary.request_count, 1);
        assert_eq!(summary.total_tokens, 1000);
        assert!(!summary.by_model.contains_key("deleted/model"));
    }

    #[test]
    fn purge_models_is_noop_when_model_not_present() {
        let tmp = TempDir::new().unwrap();
        let tracker = CostTracker::new(enabled_config(), tmp.path()).unwrap();

        tracker
            .record_usage(TokenUsage::new("keep/model", 1000, 0, 1.0, 1.0), None)
            .unwrap();

        let removed = tracker
            .purge_models(&["missing/model".to_string()])
            .unwrap();
        assert_eq!(removed, 0);

        let summary = tracker.get_summary().unwrap();
        assert_eq!(summary.request_count, 1);
        assert!(summary.by_model.contains_key("keep/model"));
    }
}
