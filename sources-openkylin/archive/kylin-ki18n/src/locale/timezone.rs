// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::path::{Path, PathBuf};

use once_cell::sync::Lazy;
use parking_lot::RwLock;

use crate::locale::country::Country;
use crate::locale::data::{self, IsoData, TimezoneData};
use crate::locale::spatial::{SpatialBounds, SpatialIndex};

static TZ_STATE: Lazy<RwLock<Option<(PathBuf, TimezoneState)>>> = Lazy::new(|| RwLock::new(None));

#[derive(Clone)]
struct TimezoneState {
    #[allow(dead_code)]
    data: TimezoneData,
    index: SpatialIndex<PointBounds, String>,
}

#[derive(Clone)]
struct PointBounds {
    lat: f32,
    lon: f32,
}

impl SpatialBounds for PointBounds {
    fn contains(&self, lat: f32, lon: f32) -> bool {
        (self.lat - lat).abs() < 0.0001 && (self.lon - lon).abs() < 0.0001
    }

    fn bounds(&self) -> (f32, f32, f32, f32) {
        (self.lat, self.lon, self.lat, self.lon)
    }
}

fn ensure_iso_loaded(base: &Path) -> Option<IsoData> {
    data::load_iso_data_cached(base)
}

fn ensure_tz_state(base: &Path) -> Option<TimezoneState> {
    let key = std::fs::canonicalize(base).unwrap_or_else(|_| base.to_path_buf());
    {
        let guard = TZ_STATE.read();
        if let Some((cached_key, state)) = &*guard {
            if *cached_key == key {
                return Some(state.clone());
            }
        }
    }
    let mut guard = TZ_STATE.write();
    let should_reload = guard
        .as_ref()
        .map(|(cached_key, _)| *cached_key != key)
        .unwrap_or(true);
    if should_reload {
        if let Some(data) = data::load_timezone_data_cached(base) {
            let mut index: SpatialIndex<PointBounds, String> = SpatialIndex::new();
            for coord in &data.coords {
                index.insert(
                    PointBounds {
                        lat: coord.lat,
                        lon: coord.lon,
                    },
                    coord.timezone.clone(),
                );
            }
            *guard = Some((key, TimezoneState { data, index }));
        }
    }
    guard.as_ref().map(|(_, s)| s.clone())
}

pub fn from_location(base: &Path, lat: f32, lon: f32) -> Option<String> {
    // ki18n SpatialIndex::lookup returns "empty" for NaN and for coordinates outside coverage
    // (it uses checks like `lon < XStart || lon >= XEnd` and `lat < YStart || lat >= YEnd`).
    if lat.is_nan() || lon.is_nan() {
        return None;
    }
    // ki18n's spatial grid uses slightly narrower coverage than geo bounds:
    // spatial_index_parameters_p.h: YStart=-60, YRange=140 => lat in [-60, 80), lon in [-180, 180).
    if lat < -60.0 || lat >= 80.0 || lon < -180.0 || lon >= 180.0 {
        return None;
    }

    // Prefer ki18n spatial index when present (for exact border behavior).
    let has_spatial_index = base.join("spatial_index_entries.bin").exists()
        && base.join("spatial_index_properties.json").exists();
    if has_spatial_index {
        if let Some(tz) = data::spatial_index_timezone_from_location(base, lat, lon) {
            return Some(tz);
        }
        // If the ki18n spatial index exists but returns empty, keep empty result.
        return None;
    }

    let state = ensure_tz_state(base)?;
    // Fallback: representative-point nearest neighbor.
    state.index.nearest(lat, lon).cloned()
}

pub fn country(base: &Path, iana_id: &str) -> Option<Country> {
    let iso = ensure_iso_loaded(base)?;
    // Prefer exact mapping table when present (ki18n's timezone_country_map has
    // holes for ambiguous timezones like Asia/Bangkok).
    if let Some(map) = data::load_timezone_country_map_cached(base) {
        if let Some(alpha2) = map.get(iana_id) {
            return super::country::Country::from_alpha2(base, alpha2);
        }
        return None;
    }

    // Fallback: derive from iso_3166_1.json timezones, and only return when it is unique.
    let mut matched: Option<Country> = None;
    let mut ambiguous = false;

    for c in &iso.countries {
        if c.timezones.iter().any(|tz| tz == iana_id) {
            let country = super::country::Country::from_alpha2(base, &c.alpha2)
                .or_else(|| super::country::Country::from_alpha3(base, &c.alpha3));
            if country.is_none() {
                continue;
            }
            if matched.is_none() {
                matched = country;
            } else {
                ambiguous = true;
                break;
            }
        }
    }

    if ambiguous {
        None
    } else {
        matched
    }
}

