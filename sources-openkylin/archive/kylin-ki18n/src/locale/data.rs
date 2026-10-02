// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::fs;
use std::io;
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use once_cell::sync::Lazy;
use parking_lot::RwLock;
use serde::Deserialize;
use thiserror::Error;

#[derive(Debug, Error)]
pub enum DataError {
    #[error("I/O error: {0}")]
    Io(#[from] io::Error),
    #[error("JSON parse error: {0}")]
    Json(#[from] serde_json::Error),
}

#[derive(Debug, Clone, Deserialize)]
pub struct CountryData {
    pub alpha2: String,
    pub alpha3: String,
    pub name: String,
    /// ISO 3166-1 numeric code (可选，取自 IsoCodes 数据)
    pub numeric: Option<u16>,
    pub currency: Option<String>,
    pub emoji: Option<String>,
    pub timezones: Vec<String>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct SubdivisionData {
    pub code: String,
    pub name: String,
    pub country_alpha2: String,
    pub parent_code: Option<String>,
    pub timezone_ids: Vec<String>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct TimezoneCoord {
    pub lat: f32,
    pub lon: f32,
    pub timezone: String,
}

#[derive(Debug, Clone)]
pub struct IsoData {
    pub countries: Vec<CountryData>,
    pub subdivisions: Vec<SubdivisionData>,
}

#[derive(Debug, Clone)]
pub struct TimezoneData {
    pub coords: Vec<TimezoneCoord>,
}

pub type TimezoneCountryMap = HashMap<String, String>;
pub type CountryTimezoneMap = HashMap<String, String>;
pub type CountryTimezonesFromSubdivMap = HashMap<String, Vec<String>>;
/// ISO 3166-2 代码 → IANA 时区列表（与上游 `subdivision_timezone_map.cpp` 一致，由脚本自 ki18n 生成）。
pub type SubdivisionTimezoneMap = HashMap<String, Vec<String>>;

#[derive(Debug, Clone)]
pub struct SpatialIndexLookupData {
    entries_z: Vec<u32>,
    entries_property: Vec<u16>,
    tz_by_property: Vec<Option<String>>,
    country_by_property: Vec<Option<String>>, // alpha2
    /// R04: optional polygon rings for precise point-in-polygon check.
    /// Index = property index; value = exterior ring [(lat,lon), ...].
    polygon_rings: Option<Vec<Option<Vec<(f32, f32)>>>>,
    /// 与上游 `SpatialIndexProperty::m_subdiv` 对齐：可选，ISO 3166-2 编码（如 US-CA），与 property 表同索引。
    subdivision_by_property: Option<Vec<Option<String>>>,
}

#[derive(Debug, Clone)]
pub struct LoadedData {
    pub iso: IsoData,
    pub tz: TimezoneData,
}

static ISO_CACHE: Lazy<RwLock<Option<(PathBuf, IsoData)>>> = Lazy::new(|| RwLock::new(None));
static TZ_CACHE: Lazy<RwLock<Option<(PathBuf, TimezoneData)>>> = Lazy::new(|| RwLock::new(None));
static TZ_COUNTRY_MAP_CACHE: Lazy<RwLock<Option<(PathBuf, TimezoneCountryMap)>>> =
    Lazy::new(|| RwLock::new(None));
static COUNTRY_TIMEZONE_MAP_CACHE: Lazy<RwLock<Option<(PathBuf, CountryTimezoneMap)>>> =
    Lazy::new(|| RwLock::new(None));
static COUNTRY_TIMEZONES_FROM_SUBDIV_CACHE: Lazy<RwLock<Option<(PathBuf, CountryTimezonesFromSubdivMap)>>> =
    Lazy::new(|| RwLock::new(None));
static SUBDIVISION_TIMEZONE_MAP_CACHE: Lazy<RwLock<Option<(PathBuf, SubdivisionTimezoneMap)>>> =
    Lazy::new(|| RwLock::new(None));
static SPATIAL_INDEX_CACHE: Lazy<RwLock<Option<(PathBuf, Arc<SpatialIndexLookupData>)>>> =
    Lazy::new(|| RwLock::new(None));

fn normalized_base(base: &Path) -> PathBuf {
    std::fs::canonicalize(base).unwrap_or_else(|_| base.to_path_buf())
}

/// iso-codes format: {"3166-1": [{"alpha_2":"XX","alpha_3":"XXX","name":"...","numeric":"xxx","flag":"..."}]}
#[derive(Deserialize)]
struct IsoCodes3166_1Entry {
    #[serde(rename = "alpha_2")]
    alpha_2: String,
    #[serde(rename = "alpha_3")]
    alpha_3: String,
    name: String,
    #[serde(rename = "numeric", default)]
    numeric: Option<String>,
}

#[derive(Deserialize)]
struct IsoCodes3166_1Wrapper {
    #[serde(rename = "3166-1")]
    entries: Vec<IsoCodes3166_1Entry>,
}

#[derive(Deserialize)]
struct IsoCodes3166_2Entry {
    code: String,
    name: String,
    #[serde(default)]
    parent: Option<String>,
}

#[derive(Deserialize)]
struct IsoCodes3166_2Wrapper {
    #[serde(rename = "3166-2")]
    entries: Vec<IsoCodes3166_2Entry>,
}

fn country_alpha2_from_iso3166_2_code(code: &str) -> Option<String> {
    let upper = code.to_ascii_uppercase();
    let (a2, _) = upper.split_once('-')?;
    if a2.len() == 2 {
        Some(a2.to_string())
    } else {
        None
    }
}

fn load_subdivisions_from_iso_codes_dir(iso_codes_dir: &Path) -> Result<Vec<SubdivisionData>, DataError> {
    let path = iso_codes_dir.join("iso_3166-2.json");
    if !path.is_file() {
        return Ok(Vec::new());
    }
    let json = fs::read_to_string(&path)?;
    let wrapper: IsoCodes3166_2Wrapper = serde_json::from_str(&json)?;
    let mut out = Vec::with_capacity(wrapper.entries.len());
    for e in wrapper.entries {
        let Some(country_alpha2) = country_alpha2_from_iso3166_2_code(&e.code) else {
            continue;
        };
        let parent_code = e
            .parent
            .as_ref()
            .filter(|p| !p.is_empty())
            .map(|p| p.to_ascii_uppercase());
        out.push(SubdivisionData {
            code: e.code.to_ascii_uppercase(),
            name: e.name,
            country_alpha2,
            parent_code,
            timezone_ids: Vec::new(),
        });
    }
    Ok(out)
}

/// Load ISO 3166-1 from iso-codes format (e.g. /usr/share/iso-codes/json/iso_3166-1.json).
/// When `iso_3166-2.json` exists beside it, subdivisions are merged (no timezone enrichment).
fn load_iso_data_from_iso_codes(path: &Path) -> Result<IsoData, DataError> {
    let json = fs::read_to_string(path)?;
    let wrapper: IsoCodes3166_1Wrapper = serde_json::from_str(&json)?;
    let countries: Vec<CountryData> = wrapper
        .entries
        .into_iter()
        .filter(|e| e.alpha_2.len() == 2 && e.alpha_3.len() == 3)
        .map(|e| {
            let numeric = e
                .numeric
                .as_ref()
                .and_then(|s| s.trim().parse::<u16>().ok());
            CountryData {
                alpha2: e.alpha_2,
                alpha3: e.alpha_3,
                name: e.name,
                numeric,
                currency: None,
                emoji: None,
                timezones: Vec::new(),
            }
        })
        .collect();
    let iso_dir = path
        .parent()
        .map(Path::to_path_buf)
        .unwrap_or_else(|| PathBuf::from("."));
    let subdivisions = load_subdivisions_from_iso_codes_dir(&iso_dir)?;
    Ok(IsoData {
        countries,
        subdivisions,
    })
}

const ISO_CODES_FALLBACK_DIRS: &[&str] = &["/usr/share/iso-codes/json", "/usr/local/share/iso-codes/json"];

pub fn load_iso_data(base: &Path) -> Result<IsoData, DataError> {
    let countries_path = base.join("iso_3166_1.json");
    let subdivisions_path = base.join("iso_3166_2.json");

    // Try our format first
    if countries_path.exists() && subdivisions_path.exists() {
        let countries_json = fs::read_to_string(&countries_path)?;
        let subdivisions_json = fs::read_to_string(&subdivisions_path)?;

        let countries: Vec<CountryData> = serde_json::from_str(&countries_json)?;
        let subdivisions: Vec<SubdivisionData> = serde_json::from_str(&subdivisions_json)?;

        return Ok(IsoData {
            countries,
            subdivisions,
        });
    }

    // R03: Fallback to iso-codes runtime data (countries only)
    if countries_path.exists() {
        let countries_json = fs::read_to_string(&countries_path)?;
        let countries: Vec<CountryData> = serde_json::from_str(&countries_json)?;
        let subdivisions = if subdivisions_path.exists() {
            let sub_json = fs::read_to_string(&subdivisions_path)?;
            serde_json::from_str(&sub_json).unwrap_or_default()
        } else {
            Vec::new()
        };
        return Ok(IsoData { countries, subdivisions });
    }

    for iso_dir in ISO_CODES_FALLBACK_DIRS {
        let path = Path::new(iso_dir).join("iso_3166-1.json");
        if path.exists() {
            return load_iso_data_from_iso_codes(&path);
        }
    }

    Err(DataError::Io(io::Error::new(
        io::ErrorKind::NotFound,
        "iso_3166_1.json not found in data dir or /usr/share/iso-codes/json",
    )))
}

pub fn load_timezone_data(base: &Path) -> Result<TimezoneData, DataError> {
    let tz_path = base.join("timezone_coords.json");
    let tz_json = fs::read_to_string(tz_path)?;
    let coords: Vec<TimezoneCoord> = serde_json::from_str(&tz_json)?;
    Ok(TimezoneData { coords })
}

pub fn load_timezone_country_map(base: &Path) -> Result<TimezoneCountryMap, DataError> {
    let path = base.join("timezone_country_map.json");
    let text = fs::read_to_string(path)?;
    // JSON object: { "Asia/Shanghai": "CN", ... }
    let map: TimezoneCountryMap = serde_json::from_str(&text)?;
    Ok(map)
}

pub fn load_iso_data_cached(base: &Path) -> Option<IsoData> {
    let key = normalized_base(base);
    {
        let guard = ISO_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }
    let loaded = load_iso_data(base).ok()?;
    let mut guard = ISO_CACHE.write();
    *guard = Some((key, loaded.clone()));
    Some(loaded)
}

pub fn load_timezone_data_cached(base: &Path) -> Option<TimezoneData> {
    let key = normalized_base(base);
    {
        let guard = TZ_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }
    let loaded = load_timezone_data(base).ok()?;
    let mut guard = TZ_CACHE.write();
    *guard = Some((key, loaded.clone()));
    Some(loaded)
}

pub fn load_timezone_country_map_cached(base: &Path) -> Option<TimezoneCountryMap> {
    let key = normalized_base(base);
    {
        let guard = TZ_COUNTRY_MAP_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }

    let loaded = load_timezone_country_map(base).ok()?;
    let mut guard = TZ_COUNTRY_MAP_CACHE.write();
    *guard = Some((key, loaded.clone()));
    Some(loaded)
}

pub fn load_country_timezone_map(base: &Path) -> Result<CountryTimezoneMap, DataError> {
    let path = base.join("country_timezone_map.json");
    let text = fs::read_to_string(path)?;
    let map: CountryTimezoneMap = serde_json::from_str(&text)?;
    Ok(map)
}

pub fn load_country_timezones_from_subdiv_map(
    base: &Path,
) -> Result<CountryTimezonesFromSubdivMap, DataError> {
    let path = base.join("country_timezones_from_subdiv.json");
    let text = fs::read_to_string(path)?;
    let map: CountryTimezonesFromSubdivMap = serde_json::from_str(&text)?;
    Ok(map)
}

pub fn load_subdivision_timezone_map(base: &Path) -> Result<SubdivisionTimezoneMap, DataError> {
    let path = base.join("subdivision_timezone_map.json");
    let text = fs::read_to_string(path)?;
    let map: SubdivisionTimezoneMap = serde_json::from_str(&text)?;
    Ok(map)
}

pub fn load_subdivision_timezone_map_cached(base: &Path) -> Option<SubdivisionTimezoneMap> {
    let key = normalized_base(base);
    {
        let guard = SUBDIVISION_TIMEZONE_MAP_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }
    let loaded = load_subdivision_timezone_map(base).ok()?;
    let mut guard = SUBDIVISION_TIMEZONE_MAP_CACHE.write();
    *guard = Some((key, loaded.clone()));
    Some(loaded)
}

pub fn load_country_timezone_map_cached(base: &Path) -> Option<CountryTimezoneMap> {
    let key = normalized_base(base);
    {
        let guard = COUNTRY_TIMEZONE_MAP_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }
    let loaded = load_country_timezone_map(base).ok()?;
    let mut guard = COUNTRY_TIMEZONE_MAP_CACHE.write();
    *guard = Some((key, loaded.clone()));
    Some(loaded)
}

pub fn load_country_timezones_from_subdiv_map_cached(
    base: &Path,
) -> Option<CountryTimezonesFromSubdivMap> {
    let key = normalized_base(base);
    {
        let guard = COUNTRY_TIMEZONES_FROM_SUBDIV_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }
    let loaded = load_country_timezones_from_subdiv_map(base).ok()?;
    let mut guard = COUNTRY_TIMEZONES_FROM_SUBDIV_CACHE.write();
    *guard = Some((key, loaded.clone()));
    Some(loaded)
}

fn load_spatial_index_entries(base: &Path) -> Result<(Vec<u32>, Vec<u16>), DataError> {
    let path = base.join("spatial_index_entries.bin");
    let bytes = fs::read(path)?;
    // Stored as repeated (z:u32 LE, prop:u16 LE)
    if bytes.len() % 6 != 0 {
        // Keep it forgiving: if mismatch, treat as missing data.
        return Err(DataError::Io(io::Error::new(
            io::ErrorKind::InvalidData,
            "spatial_index_entries.bin length not multiple of 6",
        )));
    }
    let mut entries_z = Vec::with_capacity(bytes.len() / 6);
    let mut entries_prop = Vec::with_capacity(bytes.len() / 6);
    for chunk in bytes.chunks_exact(6) {
        let z = u32::from_le_bytes([chunk[0], chunk[1], chunk[2], chunk[3]]);
        let prop = u16::from_le_bytes([chunk[4], chunk[5]]);
        entries_z.push(z);
        entries_prop.push(prop);
    }
    Ok((entries_z, entries_prop))
}

#[derive(Deserialize)]
struct SpatialPropsJson {
    tz_by_property: Vec<Option<String>>,
    country_by_property: Vec<Option<String>>,
    /// R04: optional. Each element: null or [[lon,lat],...] (GeoJSON order) or [[lat,lon],...]
    #[serde(default)]
    polygon_rings: Option<Vec<Option<Vec<Vec<f32>>>>>,
    /// 与 ki18n `spatial_index_properties` 中行政区列一致；缺省则 `from_location` 用时区代表点包围盒启发式。
    #[serde(default)]
    subdivision_by_property: Option<Vec<Option<String>>>,
}

fn load_spatial_index_properties(
    base: &Path,
) -> Result<
    (
        Vec<Option<String>>,
        Vec<Option<String>>,
        Option<Vec<Option<Vec<(f32, f32)>>>>,
        Option<Vec<Option<String>>>,
    ),
    DataError,
> {
    let path = base.join("spatial_index_properties.json");
    let text = fs::read_to_string(path)?;
    let props: SpatialPropsJson = serde_json::from_str(&text)?;
    let polygon_rings = props.polygon_rings.map(|rings| {
        rings
            .into_iter()
            .map(|r| {
                r.map(|pts| {
                    pts.into_iter()
                        .map(|p| {
                            if p.len() >= 2 {
                                (p[0], p[1])
                            } else {
                                (0.0f32, 0.0f32)
                            }
                        })
                        .collect()
                })
            })
            .collect()
    });
    Ok((
        props.tz_by_property,
        props.country_by_property,
        polygon_rings,
        props.subdivision_by_property,
    ))
}

pub fn load_spatial_index_lookup_cached(base: &Path) -> Option<Arc<SpatialIndexLookupData>> {
    let key = normalized_base(base);
    {
        let guard = SPATIAL_INDEX_CACHE.read();
        if let Some((cached_key, data)) = &*guard {
            if *cached_key == key {
                return Some(data.clone());
            }
        }
    }

    let loaded_entries = load_spatial_index_entries(base).ok()?;
    let loaded_props = load_spatial_index_properties(base).ok()?;
    let (tz_by_property, country_by_property, polygon_rings, subdivision_by_property) = loaded_props;
    let data = Arc::new(SpatialIndexLookupData {
        entries_z: loaded_entries.0,
        entries_property: loaded_entries.1,
        tz_by_property,
        country_by_property,
        polygon_rings,
        subdivision_by_property,
    });
    let mut guard = SPATIAL_INDEX_CACHE.write();
    *guard = Some((key, data.clone()));
    Some(data)
}

// ---------------------------------------------------------------------------
// R04: Point-in-polygon (ray casting)
// ---------------------------------------------------------------------------

/// Ray casting algorithm: odd crossings = inside. Ring is closed (first==last implicitly).
pub fn point_in_polygon(lat: f32, lon: f32, ring: &[(f32, f32)]) -> bool {
    if ring.len() < 3 {
        return false;
    }
    let n = ring.len();
    let mut inside = false;
    let mut j = n - 1;
    for i in 0..n {
        let (xi, yi) = ring[i];
        let (xj, yj) = ring[j];
        let intersect = if (yi - yj).abs() < 1e-9 {
            false
        } else {
            ((yi > lat) != (yj > lat))
                && (lon < (xj - xi) * (lat - yi) / (yj - yi) + xi)
        };
        if intersect {
            inside = !inside;
        }
        j = i;
    }
    inside
}

const X_START: f32 = -180.0;
const X_RANGE: f32 = 360.0;
const Y_START: f32 = -60.0;
const Y_RANGE: f32 = 140.0;
const Z_DEPTH: u32 = 11;

fn latlon_to_z(lat: f32, lon: f32) -> u32 {
    let x = (((lon - X_START) / X_RANGE) * (1u32 << Z_DEPTH) as f32) as u32;
    let y = (((lat - Y_START) / Y_RANGE) * (1u32 << Z_DEPTH) as f32) as u32;
    let mut z: u32 = 0;
    for i in (0..Z_DEPTH).rev() {
        z <<= 1;
        z |= (y >> i) & 1;
        z <<= 1;
        z |= (x >> i) & 1;
    }
    z
}

fn upper_bound_last_le(z_values: &[u32], z: u32) -> Option<usize> {
    // return last idx where z_values[idx] <= z
    let mut lo = 0usize;
    let mut hi = z_values.len();
    while lo < hi {
        let mid = (lo + hi) / 2;
        if z_values[mid] <= z {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    if lo == 0 {
        None
    } else {
        Some(lo - 1)
    }
}

pub fn spatial_index_timezone_from_location(base: &Path, lat: f32, lon: f32) -> Option<String> {
    let si = load_spatial_index_lookup_cached(base)?;
    if lat.is_nan() || lon.is_nan() || lon < X_START || lon >= X_START + X_RANGE || lat < Y_START || lat >= Y_START + Y_RANGE {
        return None;
    }
    let z = latlon_to_z(lat, lon);
    let idx = upper_bound_last_le(&si.entries_z, z)?;
    let prop_idx = si.entries_property[idx] as usize;
    let tz = si.tz_by_property.get(prop_idx).and_then(|o| o.clone())?;
    // R04: when polygon data exists, verify point is inside before returning
    if let Some(ref rings) = si.polygon_rings {
        if let Some(Some(ring)) = rings.get(prop_idx) {
            if !point_in_polygon(lat, lon, ring) {
                return None; // Point outside polygon, no match
            }
        }
    }
    Some(tz)
}

pub fn spatial_index_country_alpha2_from_location(base: &Path, lat: f32, lon: f32) -> Option<String> {
    let si = load_spatial_index_lookup_cached(base)?;
    if lat.is_nan() || lon.is_nan() || lon < X_START || lon >= X_START + X_RANGE || lat < Y_START || lat >= Y_START + Y_RANGE {
        return None;
    }
    let z = latlon_to_z(lat, lon);
    let idx = upper_bound_last_le(&si.entries_z, z)?;
    let prop_idx = si.entries_property[idx] as usize;
    let country = si.country_by_property.get(prop_idx).and_then(|o| o.clone())?;
    // R04: when polygon data exists, verify point is inside before returning
    if let Some(ref rings) = si.polygon_rings {
        if let Some(Some(ring)) = rings.get(prop_idx) {
            if !point_in_polygon(lat, lon, ring) {
                return None; // Point outside polygon, no match
            }
        }
    }
    Some(country)
}

/// `KCountrySubdivision::fromLocation` 与 ki18n 空间属性表对齐时的查询结果。
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum SpatialSubdivisionLookup {
    /// 数据未提供 `subdivision_by_property`（或无法加载空间索引）：调用方可用启发式索引。
    DatasetHasNoSubdivisionColumn,
    /// 该瓦片无下级行政区（对应上游 `m_subdiv` 低位为 0）。
    NoMatch,
    /// ISO 3166-2 编码。
    Found(String),
}

/// 使用与 `spatial_index_timezone_from_location` 相同的 Z-order 与可选 polygon 精化。
pub fn spatial_index_subdivision_lookup(base: &Path, lat: f32, lon: f32) -> SpatialSubdivisionLookup {
    if !base.join("spatial_index_entries.bin").is_file()
        || !base.join("spatial_index_properties.json").is_file()
    {
        return SpatialSubdivisionLookup::DatasetHasNoSubdivisionColumn;
    }
    let Some(si) = load_spatial_index_lookup_cached(base) else {
        return SpatialSubdivisionLookup::DatasetHasNoSubdivisionColumn;
    };
    let Some(ref codes) = si.subdivision_by_property else {
        return SpatialSubdivisionLookup::DatasetHasNoSubdivisionColumn;
    };
    if lat.is_nan()
        || lon.is_nan()
        || lon < X_START
        || lon >= X_START + X_RANGE
        || lat < Y_START
        || lat >= Y_START + Y_RANGE
    {
        return SpatialSubdivisionLookup::NoMatch;
    }
    let z = latlon_to_z(lat, lon);
    let Some(idx) = upper_bound_last_le(&si.entries_z, z) else {
        return SpatialSubdivisionLookup::NoMatch;
    };
    let prop_idx = si.entries_property[idx] as usize;
    let code_opt = codes.get(prop_idx).cloned().flatten();
    let Some(code) = code_opt.filter(|s| !s.is_empty()) else {
        return SpatialSubdivisionLookup::NoMatch;
    };
    if let Some(ref rings) = si.polygon_rings {
        if let Some(Some(ring)) = rings.get(prop_idx) {
            if !point_in_polygon(lat, lon, ring) {
                return SpatialSubdivisionLookup::NoMatch;
            }
        }
    }
    SpatialSubdivisionLookup::Found(code)
}

pub fn get_country_by_alpha2<'a>(
    iso: &'a IsoData,
    alpha2: &str,
) -> Option<&'a CountryData> {
    let code = alpha2.to_ascii_uppercase();
    iso.countries
        .iter()
        .find(|c| c.alpha2.eq_ignore_ascii_case(&code))
}

pub fn get_country_by_alpha3<'a>(
    iso: &'a IsoData,
    alpha3: &str,
) -> Option<&'a CountryData> {
    let code = alpha3.to_ascii_uppercase();
    iso.countries
        .iter()
        .find(|c| c.alpha3.eq_ignore_ascii_case(&code))
}

pub fn get_subdivisions<'a>(
    iso: &'a IsoData,
    country_alpha2: &str,
) -> Vec<&'a SubdivisionData> {
    let code = country_alpha2.to_ascii_uppercase();
    iso.subdivisions
        .iter()
        .filter(|s| s.country_alpha2.eq_ignore_ascii_case(&code))
        .collect()
}

/// Top-level subdivisions only (no parent). For KCountry::subdivisions().
pub fn get_top_level_subdivisions_for_country<'a>(
    iso: &'a IsoData,
    country_alpha2: &str,
) -> Vec<(usize, &'a SubdivisionData)> {
    let code = country_alpha2.to_ascii_uppercase();
    iso.subdivisions
        .iter()
        .enumerate()
        .filter(|(_, s)| {
            s.country_alpha2.eq_ignore_ascii_case(&code)
                && s.parent_code.as_ref().map_or(true, |p| p.is_empty())
        })
        .map(|(i, s)| (i, s))
        .collect()
}

pub fn get_subdivision_by_code<'a>(
    iso: &'a IsoData,
    code: &str,
) -> Option<&'a SubdivisionData> {
    let c = code.to_ascii_uppercase();
    iso.subdivisions
        .iter()
        .find(|s| s.code.eq_ignore_ascii_case(&c))
}

/// Returns (index, &SubdivisionData) for C++ KCountrySubdivision index storage.
pub fn get_subdivision_by_code_with_index<'a>(
    iso: &'a IsoData,
    code: &str,
) -> Option<(usize, &'a SubdivisionData)> {
    let c = code.to_ascii_uppercase();
    iso.subdivisions
        .iter()
        .enumerate()
        .find(|(_, s)| s.code.eq_ignore_ascii_case(&c))
        .map(|(i, s)| (i, s))
}

pub fn get_subdivision_data_by_index<'a>(
    iso: &'a IsoData,
    index: usize,
) -> Option<&'a SubdivisionData> {
    iso.subdivisions.get(index)
}

/// 根据 ISO 3166-1 numeric 代码进行二进制查找，返回在 countries 向量中的索引。
pub fn get_country_index_by_numeric(iso: &IsoData, numeric: u16) -> Option<usize> {
    // 构建索引数组并按 numeric 排序，然后二分查找。
    // 即使数据量较大，此操作仍然是 O(n log n)，且只在需要时调用。
    let mut indices: Vec<usize> = (0..iso.countries.len()).collect();
    indices.sort_by_key(|&i| iso.countries[i].numeric.unwrap_or(0));

    indices
        .binary_search_by_key(&numeric, |&i| iso.countries[i].numeric.unwrap_or(0))
        .ok()
}

pub fn find_timezone(tz: &TimezoneData, lat: f32, lon: f32) -> Option<String> {
    // 简单最近邻查找；空间索引会在 spatial.rs 中实现更高效版本
    let mut best: Option<(&TimezoneCoord, f32)> = None;
    for coord in &tz.coords {
        let dlat = coord.lat - lat;
        let dlon = coord.lon - lon;
        let dist2 = dlat * dlat + dlon * dlon;
        match best {
            None => best = Some((coord, dist2)),
            Some((_, best_d2)) if dist2 < best_d2 => best = Some((coord, dist2)),
            _ => {}
        }
    }
    best.map(|(c, _)| c.timezone.clone())
}

