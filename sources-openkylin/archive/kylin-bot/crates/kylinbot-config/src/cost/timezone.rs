//! Cost aggregation calendar boundaries for the China-facing product default.
//!
//! `costs.jsonl` timestamps are serialized in `Asia/Shanghai` (`+08:00`). In memory they
//! remain [`DateTime<Utc>`]; daily/monthly rollups also use the China calendar.

use chrono::{DateTime, Datelike, NaiveDate, Utc};
use chrono_tz::Asia::Shanghai;
use serde::{Deserialize, Deserializer, Serialize, Serializer};

/// IANA timezone used for「今日 / 本月」用量统计边界。
pub const COST_STATS_TIMEZONE: &str = "Asia/Shanghai";

/// Current instant in the cost-stats timezone (China Standard Time).
pub fn cost_stats_now() -> DateTime<chrono_tz::Tz> {
    Utc::now().with_timezone(&Shanghai)
}

/// Calendar date of `time` in the cost-stats timezone.
pub fn cost_stats_local_date(time: DateTime<Utc>) -> NaiveDate {
    time.with_timezone(&Shanghai).date_naive()
}

/// Calendar year and month of `time` in the cost-stats timezone.
pub fn cost_stats_local_year_month(time: DateTime<Utc>) -> (i32, u32) {
    let local = time.with_timezone(&Shanghai);
    (local.year(), local.month())
}

/// Today's date and current month in the cost-stats timezone.
pub fn current_cost_stats_period() -> (NaiveDate, i32, u32) {
    let now = cost_stats_now();
    (now.date_naive(), now.year(), now.month())
}

/// Serde adapter: write/read `costs.jsonl` timestamps in China Standard Time.
pub mod china_timestamp {
    use super::{DateTime, Deserialize, Deserializer, Serializer, Shanghai, Utc};

    pub fn serialize<S>(dt: &DateTime<Utc>, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        let local = dt.with_timezone(&Shanghai);
        serializer.serialize_str(&local.to_rfc3339())
    }

    pub fn deserialize<'de, D>(deserializer: D) -> Result<DateTime<Utc>, D::Error>
    where
        D: Deserializer<'de>,
    {
        let raw = String::deserialize(deserializer)?;
        let parsed = DateTime::parse_from_rfc3339(&raw).map_err(serde::de::Error::custom)?;
        Ok(parsed.with_timezone(&Utc))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::TimeZone;

    #[test]
    fn utc_late_evening_counts_as_next_cst_day() {
        // 2026-05-25 20:00 UTC = 2026-05-26 04:00 CST
        let utc = Utc.with_ymd_and_hms(2026, 5, 25, 20, 0, 0).unwrap();
        assert_eq!(cost_stats_local_date(utc), NaiveDate::from_ymd_opt(2026, 5, 26).unwrap());
    }

    #[test]
    fn utc_month_boundary_for_cst() {
        // 2026-01-31 20:00 UTC = 2026-02-01 04:00 CST
        let utc = Utc.with_ymd_and_hms(2026, 1, 31, 20, 0, 0).unwrap();
        let (y, m) = cost_stats_local_year_month(utc);
        assert_eq!(y, 2026);
        assert_eq!(m, 2);
    }

    #[test]
    fn china_timestamp_serializes_with_plus_eight_offset() {
        #[derive(Serialize)]
        struct Wrapper {
            #[serde(with = "china_timestamp")]
            ts: DateTime<Utc>,
        }
        let utc = Utc.with_ymd_and_hms(2026, 5, 25, 14, 17, 9).unwrap();
        let line = serde_json::to_string(&Wrapper { ts: utc }).unwrap();
        assert!(line.contains("+08:00"));
        assert!(line.contains("22:17:09"));
        assert!(!line.contains('Z'));
    }

    #[test]
    fn china_timestamp_deserializes_legacy_utc_z() {
        #[derive(Deserialize)]
        struct Wrapper {
            #[serde(with = "china_timestamp")]
            ts: DateTime<Utc>,
        }
        let wrapper: Wrapper =
            serde_json::from_str(r#"{"ts":"2026-05-25T14:17:09.712371559Z"}"#).unwrap();
        let expected = Utc.with_ymd_and_hms(2026, 5, 25, 14, 17, 9).unwrap();
        assert_eq!(wrapper.ts.timestamp(), expected.timestamp());
    }

    #[test]
    fn china_timestamp_round_trips_cst_string() {
        #[derive(Serialize, Deserialize)]
        struct Wrapper {
            #[serde(with = "china_timestamp")]
            ts: DateTime<Utc>,
        }
        let utc = Utc.with_ymd_and_hms(2026, 5, 25, 14, 17, 9).unwrap();
        let json = serde_json::to_string(&Wrapper { ts: utc }).unwrap();
        let back: Wrapper = serde_json::from_str(&json).unwrap();
        assert_eq!(back.ts, utc);
    }
}
