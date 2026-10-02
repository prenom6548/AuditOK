// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::path::{Path, PathBuf};

use crate::locale::country::Country;
use crate::locale::data::{
    self, IsoData, SpatialSubdivisionLookup, SubdivisionData, TimezoneCoord, TimezoneData,
};
use crate::locale::spatial::{SpatialBounds, SpatialIndex};
use once_cell::sync::Lazy;
use parking_lot::RwLock;

static SUB_SPATIAL: Lazy<RwLock<Option<(PathBuf, SpatialIndex<SubdivisionBounds, String>)>>> =
    Lazy::new(|| RwLock::new(None));

fn ensure_iso_loaded(base: &Path) -> Option<IsoData> {
    data::load_iso_data_cached(base)
}

fn ensure_tz_loaded(base: &Path) -> Option<TimezoneData> {
    data::load_timezone_data_cached(base)
}

#[derive(Clone)]
struct SubdivisionBounds {
    min_lat: f32,
    min_lon: f32,
    max_lat: f32,
    max_lon: f32,
}

impl SpatialBounds for SubdivisionBounds {
    fn contains(&self, lat: f32, lon: f32) -> bool {
        lat >= self.min_lat
            && lat <= self.max_lat
            && lon >= self.min_lon
            && lon <= self.max_lon
    }

    fn bounds(&self) -> (f32, f32, f32, f32) {
        (self.min_lat, self.min_lon, self.max_lat, self.max_lon)
    }
}

fn build_subdivision_spatial_index(
    iso: &IsoData,
    tz: &TimezoneData,
) -> SpatialIndex<SubdivisionBounds, String> {
    let mut index: SpatialIndex<SubdivisionBounds, String> = SpatialIndex::new();

    // 无 `subdivision_by_property` 时的后备：各 subdivision 其时区代表点坐标的 AABB（与上游空间表不完全一致）
    for sub in &iso.subdivisions {
        let mut coords: Vec<&TimezoneCoord> = Vec::new();
        for tz_id in &sub.timezone_ids {
            coords.extend(tz.coords.iter().filter(|c| &c.timezone == tz_id));
        }
        if coords.is_empty() {
            continue;
        }
        let mut min_lat = f32::MAX;
        let mut max_lat = f32::MIN;
        let mut min_lon = f32::MAX;
        let mut max_lon = f32::MIN;
        for c in coords {
            if c.lat < min_lat {
                min_lat = c.lat;
            }
            if c.lat > max_lat {
                max_lat = c.lat;
            }
            if c.lon < min_lon {
                min_lon = c.lon;
            }
            if c.lon > max_lon {
                max_lon = c.lon;
            }
        }
        let bounds = SubdivisionBounds {
            min_lat,
            min_lon,
            max_lat,
            max_lon,
        };
        index.insert(bounds, sub.code.clone());
    }

    index
}

#[derive(Clone, Debug)]
pub struct Subdivision {
    index: u32,
    code: String,
    name: String,
    country_code: String,
    parent_code: Option<String>,
    timezone_ids: Vec<String>,
}

impl Subdivision {
    fn from_data(index: usize, data: &SubdivisionData) -> Self {
        Subdivision {
            index: index as u32,
            code: data.code.clone(),
            name: data.name.clone(),
            country_code: data.country_alpha2.clone(),
            parent_code: data.parent_code.clone(),
            timezone_ids: data.timezone_ids.clone(),
        }
    }

    /// Index into iso.subdivisions. For C++ KCountrySubdivision uint32_t d (1-based: 0=invalid).
    pub fn index(&self) -> u32 {
        self.index
    }

    pub fn from_data_index(base: &Path, index: usize) -> Option<Self> {
        let iso = ensure_iso_loaded(base)?;
        data::get_subdivision_data_by_index(&iso, index).map(|d| Subdivision::from_data(index, d))
    }

    pub fn from_code(base: &Path, code: &str) -> Option<Self> {
        let iso = ensure_iso_loaded(base)?;
        data::get_subdivision_by_code_with_index(&iso, code)
            .map(|(i, d)| Subdivision::from_data(i, d))
    }

    pub fn from_location(base: &Path, lat: f32, lon: f32) -> Option<Self> {
        let iso = ensure_iso_loaded(base)?;

        match data::spatial_index_subdivision_lookup(base, lat, lon) {
            SpatialSubdivisionLookup::Found(code) => {
                return data::get_subdivision_by_code_with_index(&iso, &code)
                    .map(|(i, d)| Subdivision::from_data(i, d));
            }
            SpatialSubdivisionLookup::NoMatch => return None,
            SpatialSubdivisionLookup::DatasetHasNoSubdivisionColumn => {}
        }

        let tz = ensure_tz_loaded(base)?;
        let key = std::fs::canonicalize(base).unwrap_or_else(|_| base.to_path_buf());

        // 构建或获取 subdivision 空间索引
        {
            let guard = SUB_SPATIAL.read();
            let need_rebuild = guard
                .as_ref()
                .map(|(cached_key, _)| *cached_key != key)
                .unwrap_or(true);
            if need_rebuild {
                drop(guard);
                let mut w = SUB_SPATIAL.write();
                let need_rebuild_w = w
                    .as_ref()
                    .map(|(cached_key, _)| *cached_key != key)
                    .unwrap_or(true);
                if need_rebuild_w {
                    *w = Some((key, build_subdivision_spatial_index(&iso, &tz)));
                }
            }
        }

        let index_guard = SUB_SPATIAL.read();
        let spatial = index_guard.as_ref().map(|(_, i)| i)?;
        if let Some(code) = spatial.nearest(lat, lon).cloned() {
            return data::get_subdivision_by_code_with_index(&iso, &code)
                .map(|(i, d)| Subdivision::from_data(i, d));
        }
        None
    }

    pub fn code(&self) -> &str {
        &self.code
    }

    pub fn name(&self) -> String {
        crate::locale::iso_translate::translate_iso_message("iso_3166-2", self.name.as_str())
    }

    pub fn country(&self, base: &Path) -> Country {
        Country::from_alpha2(base, &self.country_code)
            .expect("ISO subdivision row must reference a valid alpha2 country")
    }

    pub fn parent(&self, base: &Path) -> Option<Subdivision> {
        self.parent_code
            .as_ref()
            .and_then(|c| Subdivision::from_code(base, c))
    }

    pub fn subdivisions(&self, base: &Path) -> Vec<Subdivision> {
        let iso = match ensure_iso_loaded(base) {
            Some(d) => d,
            None => return Vec::new(),
        };
        iso.subdivisions
            .iter()
            .enumerate()
            .filter(|(_, s)| s.parent_code.as_ref().map(|p| p == &self.code).unwrap_or(false))
            .map(|(i, d)| Subdivision::from_data(i, d))
            .collect()
    }

    /// 与上游 `KCountrySubdivision::timeZoneIds` 一致：先 `subdivision_timezone_map`，再国家级单一时区表。
    pub fn timezones(&self, base: &std::path::Path) -> Vec<String> {
        if !self.timezone_ids.is_empty() {
            return self.timezone_ids.clone();
        }
        if let Some(m) = data::load_subdivision_timezone_map_cached(base) {
            if let Some(v) = m.get(self.code()) {
                if !v.is_empty() {
                    return v.clone();
                }
            }
        }
        if let Some(map) = data::load_country_timezone_map_cached(base) {
            if let Some(tz) = map.get(self.country_code.as_str()) {
                return vec![tz.clone()];
            }
        }
        Vec::new()
    }
}

