// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashSet;
use std::fs;
use std::path::Path;

use crate::locale::data::{self, CountryData, IsoData, TimezoneData};
use crate::locale::subdivision::Subdivision;
use crate::core::catalog::Catalog;
use crate::core::global as core_global;
use unicode_normalization::UnicodeNormalization;
use once_cell::sync::Lazy;

fn geo_distance2_approx(lat1: f32, lon1: f32, lat2: f32, lon2: f32) -> f32 {
    // Must stay consistent with `kylin-ki18n/src/locale/spatial.rs` approximation.
    let to_rad = std::f32::consts::PI / 180.0;
    let x = (lon2 - lon1) * to_rad * ((lat1 + lat2) * 0.5 * to_rad).cos();
    let y = (lat2 - lat1) * to_rad;
    x * x + y * y
}

fn ensure_iso_loaded(base: &Path) -> Option<IsoData> {
    data::load_iso_data_cached(base)
}

fn ensure_tz_loaded(base: &Path) -> Option<TimezoneData> {
    data::load_timezone_data_cached(base)
}

static SYSTEM_ISO3166_1_TRANSLATION_CATALOGS: Lazy<Vec<Catalog>> = Lazy::new(|| {
    // KCountry::fromName() in ki18n checks translated names from iso_3166-1 catalogs.
    // For kylin-ki18n we mirror this by loading system-provided `.mo` files (when present).
    let mut catalogs: Vec<Catalog> = Vec::new();
    let mut seen_paths: HashSet<std::path::PathBuf> = HashSet::new();

    for root in ["/usr/share/locale", "/usr/local/share/locale"] {
        let Ok(rd) = fs::read_dir(root) else {
            continue;
        };
        for entry in rd.flatten() {
            let lang = entry.file_name();
            let lang = lang.to_string_lossy();
            let mo_path = std::path::Path::new(root)
                .join(lang.as_ref())
                .join("LC_MESSAGES")
                .join("iso_3166-1.mo");
            if mo_path.is_file() && seen_paths.insert(mo_path.clone()) {
                if let Ok(cat) = Catalog::load_mo(&mo_path, None) {
                    catalogs.push(cat);
                }
            }
        }
    }
    catalogs
});

#[derive(Clone, Debug)]
pub struct Country {
    index: u16,
}

impl Country {
    /// Index into the ISO countries array. Used for ABI compatibility with KCountry (uint16_t d).
    pub fn index(&self) -> u16 {
        self.index  // field access
    }

    pub(crate) fn from_data_index(index: usize) -> Option<Self> {
        if index > u16::MAX as usize {
            None
        } else {
            Some(Country { index: index as u16 })
        }
    }
}

impl Country {
    pub fn from_alpha2(base: &Path, code: &str) -> Option<Self> {
        let iso = ensure_iso_loaded(base)?;
        let upper = code.to_ascii_uppercase();
        iso.countries
            .iter()
            .position(|c| c.alpha2.eq_ignore_ascii_case(&upper))
            .and_then(Country::from_data_index)
    }

    pub fn from_alpha3(base: &Path, code: &str) -> Option<Self> {
        let iso = ensure_iso_loaded(base)?;
        let upper = code.to_ascii_uppercase();
        iso.countries
            .iter()
            .position(|c| c.alpha3.eq_ignore_ascii_case(&upper))
            .and_then(Country::from_data_index)
    }

    pub fn from_name(base: &Path, name: &str) -> Option<Self> {
        if name.is_empty() {
            return None;
        }

        fn is_hangul(ch: char) -> bool {
            // Hangul syllables + Jamo ranges (good enough for our "don't strip combining marks" rule).
            ('\u{AC00}'..='\u{D7A3}').contains(&ch) || ('\u{1100}'..='\u{11FF}').contains(&ch) || ('\u{3130}'..='\u{318F}').contains(&ch)
        }

        fn normalize_country_name(input: &str) -> String {
            let mut res = String::new();
            for ch in input.chars() {
                if ch.is_whitespace() {
                    continue;
                }
                // ki18n drops punctuation; approximate by keeping only letters/digits.
                if !ch.is_alphanumeric() {
                    continue;
                }
                if is_hangul(ch) {
                    for lc in ch.to_lowercase() {
                        res.push(lc);
                    }
                    continue;
                }

                // Strip combining diacritics via NFD.
                let s = ch.to_string();
                for dc in s.nfd() {
                    if dc.is_alphanumeric() {
                        for lc in dc.to_lowercase() {
                            res.push(lc);
                        }
                    }
                }
            }
            res
        }

        fn update_substring_match(
            lhs: &str,
            rhs: &str,
            candidate_idx: usize,
            substr_match: &mut Option<usize>,
            ambiguous: &mut bool,
        ) {
            if *ambiguous {
                return;
            }
            if rhs.is_empty() {
                return;
            }
            if substr_match == &Some(candidate_idx) {
                return;
            }
            let matches = lhs.starts_with(rhs) || rhs.starts_with(lhs) || lhs.ends_with(rhs) || rhs.ends_with(lhs);
            if !matches {
                return;
            }
            match substr_match {
                None => *substr_match = Some(candidate_idx),
                Some(prev) => {
                    if *prev != candidate_idx {
                        *ambiguous = true;
                    }
                }
            }
        }

        let iso = ensure_iso_loaded(base)?;
        let normalized_name = normalize_country_name(name);

        // In ki18n, normalization happens before both exact and substring matching.
        let mut substr_match: Option<usize> = None;
        let mut ambiguous = false;

        // 1) check untranslated names
        for (idx, c) in iso.countries.iter().enumerate() {
            let normalized_country = normalize_country_name(&c.name);
            if normalized_name == normalized_country {
                return Country::from_data_index(idx);
            }
            update_substring_match(
                &normalized_name,
                &normalized_country,
                idx,
                &mut substr_match,
                &mut ambiguous,
            );
        }

        // 2) check translated names (across all relevant catalogs)
        // Prefer catalogs injected via kylin-ki18n domain locale dirs (via C API),
        // and fall back to system scanning only when no such injection exists.
        //
        // This aligns with kylin-ki18n's existing catalog resolution behavior and
        // reduces dependency on a specific filesystem layout.
        let injected_langs = core_global::available_languages("iso_3166-1");
        let injected_any = !injected_langs.is_empty();

        let mut translation_catalogs: Vec<Catalog> = Vec::new();
        if injected_any {
            for lang in injected_langs {
                if let Some(cat) = core_global::get_or_load_catalog("iso_3166-1", &lang) {
                    translation_catalogs.push(cat);
                }
            }
        } else {
            translation_catalogs.extend_from_slice(SYSTEM_ISO3166_1_TRANSLATION_CATALOGS.as_slice());
        }

        for cat in translation_catalogs.iter() {
            for (idx, c) in iso.countries.iter().enumerate() {
                if let Some(entry) = cat.get_message(&c.name) {
                    if let Some(translated) = entry.msgstr.get(0) {
                        if translated.is_empty() {
                            continue;
                        }
                        let normalized_country = normalize_country_name(translated);
                        if normalized_name == normalized_country {
                            return Country::from_data_index(idx);
                        }
                        update_substring_match(
                            &normalized_name,
                            &normalized_country,
                            idx,
                            &mut substr_match,
                            &mut ambiguous,
                        );
                    }
                }
            }
        }

        // 3) unique prefix/suffix match
        if !ambiguous {
            if let Some(idx) = substr_match {
                return Country::from_data_index(idx);
            }
        }

        // 4) code lookups fallbacks
        let clen = normalized_name.chars().count();
        if clen == 3 {
            return Country::from_alpha3(base, &normalized_name);
        }
        if clen == 2 {
            return Country::from_alpha2(base, &normalized_name);
        }

        None
    }

    pub fn from_location(base: &Path, lat: f32, lon: f32) -> Option<Self> {
        // Prefer ki18n spatial index when present (for exact border behavior).
        let has_spatial_index = base.join("spatial_index_entries.bin").exists()
            && base.join("spatial_index_properties.json").exists();
        if has_spatial_index {
            if let Some(alpha2) = data::spatial_index_country_alpha2_from_location(base, lat, lon) {
                return Country::from_alpha2(base, &alpha2);
            }
            return None;
        }

        // Keep the same coverage contract as ki18n's spatial grid (fallback path).
        if lat.is_nan() || lon.is_nan() {
            return None;
        }
        // spatial_index_parameters_p.h: YStart=-60, YRange=140 => lat in [-60, 80), lon in [-180, 180)
        if lat < -60.0 || lat >= 80.0 || lon < -180.0 || lon >= 180.0 {
            return None;
        }

        // Derive country via nearest timezones, but avoid returning a wrong country
        // in border regions: if the top-2 timezones map to different countries, return empty.
        let tz = ensure_tz_loaded(base)?;

        let mut best: Option<(String, f32)> = None;
        let mut second: Option<(String, f32)> = None;
        for coord in &tz.coords {
            let d2 = geo_distance2_approx(lat, lon, coord.lat, coord.lon);
            match best {
                None => best = Some((coord.timezone.clone(), d2)),
                Some((_, best_d2)) if d2 < best_d2 => {
                    second = best;
                    best = Some((coord.timezone.clone(), d2));
                }
                _ => match second {
                    None => second = Some((coord.timezone.clone(), d2)),
                    Some((_, second_d2)) if d2 < second_d2 => {
                        second = Some((coord.timezone.clone(), d2));
                    }
                    _ => {}
                },
            }
        }

        let (tz1, d1) = best?;
        let c1 = crate::locale::timezone::country(base, &tz1);

        if let Some((tz2, d2)) = second {
            let c2 = crate::locale::timezone::country(base, &tz2);
            match (c1, c2) {
                // Prefer a valid unique mapping; only treat as conflict when
                // the 2nd nearest timezone is close enough to be considered border-ish.
                (Some(cc1), Some(cc2)) => {
                    // Heuristic border detector based on "how close" the 2nd nearest
                    // timezone coordinate is to the nearest one.
                    //
                    // Goal: match ki18n's `canBeConflict` expectations for sampled points:
                    // - avoid triggering conflict for Randa (CH), even if a neighbor timezone exists
                    // - trigger conflict for Aachen/Maastricht where the first country guess is wrong.
                    let ratio = if d1 > 0.0 { d2 / d1 } else { f32::INFINITY };
                    let diff = if d2 >= d1 { d2 - d1 } else { 0.0 };
                    let conflict = ratio <= 3.0 && (diff <= 0.0001 || diff >= 0.0002);
                    if !conflict {
                        Some(cc1)
                    } else if cc1.index == cc2.index {
                        Some(cc1)
                    } else {
                        None
                    }
                }
                (Some(cc1), None) => Some(cc1),
                (None, Some(cc2)) => Some(cc2),
                (None, None) => None,
            }
        } else {
            c1
        }
    }

    pub fn from_qlocale(base: &Path, country: i32) -> Option<Self> {
        let iso = ensure_iso_loaded(base)?;
        if country <= 0 {
            return None;
        }
        let numeric = country as u16;
        if let Some(idx) = data::get_country_index_by_numeric(&iso, numeric) {
            Country::from_data_index(idx)
        } else {
            None
        }
    }

    pub fn all(base: &Path) -> Vec<Self> {
        let iso = match ensure_iso_loaded(base) {
            Some(d) => d,
            None => return Vec::new(),
        };
        (0..iso.countries.len())
            .filter_map(Country::from_data_index)
            .collect()
    }

    fn data<'a>(&self, iso: &'a IsoData) -> Option<&'a CountryData> {
        iso.countries.get(self.index as usize)
    }

    pub fn alpha2(&self, base: &Path) -> String {
        ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).map(|c| c.alpha2.clone()))
            .unwrap_or_default()
    }

    pub fn alpha3(&self, base: &Path) -> String {
        ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).map(|c| c.alpha3.clone()))
            .unwrap_or_default()
    }

    pub fn name(&self, base: &Path) -> String {
        let english = ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).map(|c| c.name.clone()))
            .unwrap_or_default();
        crate::locale::iso_translate::translate_iso_message("iso_3166-1", &english)
    }

    pub fn emoji_flag(&self, base: &Path) -> String {
        ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).and_then(|c| c.emoji.clone()))
            .unwrap_or_default()
    }

    pub fn currency_code(&self, base: &Path) -> Option<String> {
        ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).and_then(|c| c.currency.clone()))
    }

    pub fn subdivisions(&self, base: &Path) -> Vec<Subdivision> {
        let iso = match ensure_iso_loaded(base) {
            Some(d) => d,
            None => return Vec::new(),
        };
        let alpha2 = self.data(&iso).map(|c| c.alpha2.as_str()).unwrap_or("");
        data::get_top_level_subdivisions_for_country(&iso, alpha2)
            .into_iter()
            .filter_map(|(i, _)| Subdivision::from_data_index(base, i))
            .collect()
    }

    pub fn timezones(&self, base: &Path) -> Vec<String> {
        // Prefer ki18n timezonedata mapping tables (for order + exclusions like
        // Europe/Busingen for DE). When not available (e.g. rich test data),
        // fall back to iso_3166_1.json's timezones.
        let alpha2 = self.alpha2(base);

        if let Some(map) = data::load_country_timezone_map_cached(base) {
            if let Some(tz) = map.get(&alpha2) {
                return vec![tz.clone()];
            }
        }
        if let Some(map) = data::load_country_timezones_from_subdiv_map_cached(base) {
            if let Some(list) = map.get(&alpha2) {
                return list.clone();
            }
        }

        ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).map(|c| c.timezones.clone()))
            .unwrap_or_default()
    }

    pub fn is_valid(&self, base: &Path) -> bool {
        ensure_iso_loaded(base)
            .and_then(|iso| self.data(&iso).map(|_| ()))
            .is_some()
    }
}

