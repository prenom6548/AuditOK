// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashSet;
use std::env;
use std::fs;
use std::path::{Path, PathBuf};

use ki18n_rs::country::Country;
use ki18n_rs::timezone;
use serde_json::Value;

fn data_dir() -> PathBuf {
    if let Ok(p) = env::var("KI18N_RS_DATA_DIR") {
        PathBuf::from(p)
    } else {
        PathBuf::from("data")
    }
}

fn read_json_array(path: &Path) -> Vec<Value> {
    let text = fs::read_to_string(path).expect("read json file");
    serde_json::from_str::<Vec<Value>>(&text).expect("parse json array")
}

#[test]
fn test_parity_data_files_exist() {
    let base = data_dir();
    assert!(base.join("iso_3166_1.json").exists());
    assert!(base.join("iso_3166_2.json").exists());
    assert!(base.join("timezone_coords.json").exists());
}

#[test]
fn test_parity_timezone_query_on_generated_points() {
    let base = data_dir();
    let tz_points = read_json_array(&base.join("timezone_coords.json"));
    assert!(!tz_points.is_empty(), "timezone_coords.json must not be empty");

    let known_tz: std::collections::HashSet<String> = tz_points
        .iter()
        .filter_map(|p| p.get("timezone").and_then(Value::as_str))
        .map(ToOwned::to_owned)
        .collect();
    assert!(!known_tz.is_empty(), "timezone_coords.json must define timezones");

    // Use a bounded sample so the test stays fast but still meaningful.
    for point in tz_points.iter().take(80) {
        let lat = point
            .get("lat")
            .and_then(Value::as_f64)
            .expect("lat must be f64") as f32;
        let lon = point
            .get("lon")
            .and_then(Value::as_f64)
            .expect("lon must be f64") as f32;
        let tz_id = point
            .get("timezone")
            .and_then(Value::as_str)
            .expect("timezone must be string");

        let got = timezone::from_location(&base, lat, lon);
        // Note:
        // - zone.tab representative coordinates can land in border tiles, so ki18n may
        //   return an alternative equivalent timezone.
        // - some tiles can be marked undefined (empty return in ki18n), which maps to None here.
        if let Some(g) = got {
            assert!(
                known_tz.contains(&g),
                "unexpected timezone id returned for ({lat}, {lon}): got={g}, expected_from_coords={tz_id}"
            );
        }
    }
}

#[test]
fn test_parity_country_alpha2_alpha3_roundtrip() {
    let base = data_dir();
    let countries = read_json_array(&base.join("iso_3166_1.json"));
    assert!(!countries.is_empty(), "iso_3166_1.json must not be empty");

    for item in countries.iter().take(120) {
        let alpha2 = item
            .get("alpha2")
            .and_then(Value::as_str)
            .expect("alpha2 missing");
        let alpha3 = item
            .get("alpha3")
            .and_then(Value::as_str)
            .expect("alpha3 missing");

        let c2 = Country::from_alpha2(&base, alpha2).expect("country by alpha2");
        assert_eq!(c2.alpha2(&base), alpha2);
        assert_eq!(c2.alpha3(&base), alpha3);

        let c3 = Country::from_alpha3(&base, alpha3).expect("country by alpha3");
        assert_eq!(c3.alpha2(&base), alpha2);
    }
}

fn has_iso3166_translation_catalogs() -> bool {
    for root in ["/usr/share/locale", "/usr/local/share/locale"] {
        let Ok(rd) = fs::read_dir(root) else {
            continue;
        };
        for entry in rd.flatten() {
            let lang = entry.file_name();
            let lang = lang.to_string_lossy();
            let mo = Path::new(root)
                .join(lang.as_ref())
                .join("LC_MESSAGES")
                .join("iso_3166-1.mo");
            if mo.is_file() {
                return true;
            }
        }
    }
    false
}

#[test]
fn test_parity_country_from_name_ki18n_autotest_data() {
    let base = data_dir();
    let has_translations = has_iso3166_translation_catalogs();

    // invalid
    assert!(Country::from_name(&base, "").is_none());
    assert!(Country::from_name(&base, "Disneyland").is_none());

    // English names always work (matched against iso_3166_1.json directly)
    assert_eq!(
        Country::from_name(&base, "new zealand").unwrap().alpha2(&base),
        "NZ"
    );

    // diacritic normalization + whitespace/punctuation stripping (English)
    assert_eq!(
        Country::from_name(&base, "Ünīted  States\nOf Amérìcâ")
            .unwrap()
            .alpha2(&base),
        "US"
    );

    // unique prefix/suffix vs ambiguity (English)
    assert!(Country::from_name(&base, "United").is_none());
    assert_eq!(
        Country::from_name(&base, "Arab Emirates").unwrap().alpha2(&base),
        "AE"
    );
    assert_eq!(
        Country::from_name(&base, "United States").unwrap().alpha2(&base),
        "US"
    );

    // code fallback
    assert_eq!(
        Country::from_name(&base, "USA").unwrap().alpha2(&base),
        "US"
    );

    // Translated-name lookups require system iso_3166-1.mo catalogs.
    if !has_translations {
        eprintln!("SKIP: translated country name tests (no iso_3166-1.mo catalogs found)");
        return;
    }

    assert_eq!(
        Country::from_name(&base, "Nouvelle-Zélande").unwrap().alpha2(&base),
        "NZ"
    );
    assert_eq!(
        Country::from_name(&base, "NEUSEELAND").unwrap().alpha2(&base),
        "NZ"
    );
    assert_eq!(
        Country::from_name(&base, "Österreich").unwrap().alpha2(&base),
        "AT"
    );
    assert_eq!(
        Country::from_name(&base, "Østrig").unwrap().alpha2(&base),
        "AT"
    );
    assert_eq!(
        Country::from_name(&base, "osterreich").unwrap().alpha2(&base),
        "AT"
    );
    assert_eq!(
        Country::from_name(&base, "대한민국").unwrap().alpha2(&base),
        "KR"
    );
    assert_eq!(
        Country::from_name(&base, "Bundesrepuplik Deutschland")
            .unwrap()
            .alpha2(&base),
        "DE"
    );
}

#[test]
fn test_parity_country_timezones_are_known() {
    let base = data_dir();
    let tz_points = read_json_array(&base.join("timezone_coords.json"));
    let mut known_tz: HashSet<String> = tz_points
        .iter()
        .filter_map(|v| v.get("timezone").and_then(Value::as_str))
        .map(ToOwned::to_owned)
        .collect();

    // ki18n's KCountry::timeZoneIds() can return timezone IDs even when we don't
    // have corresponding coordinate anchors in timezone_coords.json.
    // Use spatial_index_properties.json + timezone_country_map.json as the
    // canonical timezone ID universe.
    let props_path = base.join("spatial_index_properties.json");
    if props_path.exists() {
        let text = fs::read_to_string(&props_path).expect("read spatial_index_properties.json");
        let v: Value = serde_json::from_str(&text).expect("parse spatial_index_properties.json");
        if let Some(arr) = v.get("tz_by_property").and_then(Value::as_array) {
            for maybe in arr {
                if let Some(s) = maybe.as_str() {
                    known_tz.insert(s.to_string());
                }
            }
        }
    }

    let tz_country_map_path = base.join("timezone_country_map.json");
    if tz_country_map_path.exists() {
        let text = fs::read_to_string(&tz_country_map_path).expect("read timezone_country_map.json");
        let v: serde_json::Map<String, Value> = serde_json::from_str(&text).expect("parse timezone_country_map.json");
        for k in v.keys() {
            known_tz.insert(k.to_string());
        }
    }

    assert!(!known_tz.is_empty());

    let countries = Country::all(&base);
    assert!(!countries.is_empty());

    // Check the first group only to keep test time bounded.
    for c in countries.iter().take(120) {
        for tz in c.timezones(&base) {
            assert!(
                known_tz.contains(&tz),
                "country timezone not found in generated timezone set: {tz}"
            );
        }
    }
}

#[test]
fn test_parity_country_timezone_population_is_not_mostly_empty() {
    let base = data_dir();
    let countries = Country::all(&base);
    assert!(!countries.is_empty());

    let with_tz = countries
        .iter()
        .filter(|c| !c.timezones(&base).is_empty())
        .count();
    // With zone.tab-derived mapping we expect most countries to have at least one timezone.
    assert!(
        with_tz >= 180,
        "too many countries without timezone mapping: with_tz={with_tz}"
    );
}

#[test]
fn test_parity_country_timezones_order_and_exclusions() {
    let base = data_dir();

    // KCountryTest::testTimezone expectations:
    // - DE should only contain Europe/Berlin (no Europe/Busingen)
    // - ES should be ordered: Europe/Madrid, Africa/Ceuta, Atlantic/Canary
    let be = Country::from_alpha2(&base, "BE").expect("BE country");
    assert_eq!(be.timezones(&base), vec!["Europe/Brussels"]);

    let de = Country::from_alpha2(&base, "DE").expect("DE country");
    assert_eq!(de.timezones(&base), vec!["Europe/Berlin"]);

    let es = Country::from_alpha2(&base, "ES").expect("ES country");
    assert_eq!(
        es.timezones(&base),
        vec!["Europe/Madrid", "Africa/Ceuta", "Atlantic/Canary"]
    );
}

#[test]
fn test_parity_timezone_country_known_cases() {
    let base = data_dir();
    let cases = [
        ("Asia/Shanghai", "CN"),
        ("America/New_York", "US"),
        ("Europe/Berlin", "DE"),
        ("Asia/Tokyo", "JP"),
        ("Australia/Sydney", "AU"),
    ];
    for (tz_id, expected_country) in cases {
        let c = timezone::country(&base, tz_id).expect("country by timezone");
        assert_eq!(c.alpha2(&base), expected_country, "tz={tz_id}");
    }

    // invalid
    assert!(timezone::country(&base, "").is_none());
    assert!(timezone::country(&base, "Moon/Dark_Side").is_none());

    // ki18n/autotests/ktimezonetest.cpp::testCountry key assertions
    let more = [
        ("Europe/Brussels", "BE"),
        ("America/Argentina/Cordoba", "AR"),
        ("Europe/Busingen", "DE"),
        ("America/Toronto", "CA"),
        ("Atlantic/Canary", "ES"),
    ];
    for (tz_id, expected_country) in more {
        let c = timezone::country(&base, tz_id).expect("country by timezone");
        assert_eq!(c.alpha2(&base), expected_country, "tz={tz_id}");
    }
}

#[test]
fn test_parity_timezone_from_location_invalid_and_out_of_coverage() {
    let base = data_dir();

    // ki18n: invalid should return empty.
    assert!(timezone::from_location(&base, f32::NAN, f32::NAN).is_none());

    // ki18n: -90, 180 is explicitly marked as out-of-coverage.
    assert!(timezone::from_location(&base, -90.0, 180.0).is_none());
}

#[test]
fn test_parity_timezone_from_location_ki18n_autotest_data() {
    let base = data_dir();

    // Subset of ki18n/autotests/ktimezonetest.cpp::testFromLocation_data
    // where tzAlt is allowed in border regions.
    let cases = [
        (f32::NAN, f32::NAN, "", ""),
        (-90.0, 180.0, "", ""),
        (-8.0, -35.0, "America/Recife", ""),
        (44.0, -79.5, "America/Toronto", ""),
        (52.4, 13.1, "Europe/Berlin", ""),
        (-36.5, 175.0, "Pacific/Auckland", ""),
        (46.0998, 7.781469, "Europe/Zurich", ""),
        // Northern Vietnam has a Thai timezone
        (21.0, 106.0, "Asia/Bangkok", ""),
        // Eliat: either empty or the alt answer is acceptable
        (29.72530, 35.00598, "", "Asia/Jerusalem"),
        // Cordoba: Buenos_Aires or Cordoba are acceptable
        (-31.4, -64.2, "America/Argentina/Buenos_Aires", "America/Argentina/Cordoba"),
        // Maastricht: Amsterdam or Brussels
        (50.8505, 5.6881, "Europe/Amsterdam", "Europe/Brussels"),
        // Aachen: Berlin or Brussels
        (50.7717, 6.04235, "Europe/Berlin", "Europe/Brussels"),
        // Geneva: Zurich or Paris
        (46.23213, 6.10636, "Europe/Zurich", "Europe/Paris"),
        // Busingen: Zurich or Berlin
        (47.69947, 8.68833, "Europe/Zurich", "Europe/Berlin"),
        // Tijuana: Tijuana or Los_Angeles
        (32.54274, -116.97505, "America/Tijuana", "America/Los_Angeles"),
        // Baarle: Amsterdam or Brussels
        (51.44344, 4.93373, "Europe/Amsterdam", "Europe/Brussels"),
    ];

    for (lat, lon, tz, tz_alt) in cases {
        let got = timezone::from_location(&base, lat, lon);
        if tz_alt.is_empty() {
            let expected = if tz.is_empty() { None } else { Some(tz.to_string()) };
            assert_eq!(got, expected, "lat={lat} lon={lon}");
        } else {
            let ok = (tz.is_empty() && got.is_none()) || got == Some(tz.to_string())
                || got == Some(tz_alt.to_string());
            assert!(ok, "lat={lat} lon={lon} got={got:?} expected in ['{tz}', '{tz_alt}']");
        }
    }
}

#[test]
fn test_parity_timezone_country_ambiguity_asia_bangkok() {
    let base = data_dir();

    // ki18n treats Asia/Bangkok as ambiguous -> empty country.
    assert!(timezone::country(&base, "Asia/Bangkok").is_none());
}

#[test]
fn test_parity_country_from_location_ki18n_sample_points() {
    let base = data_dir();

    // Subset of ki18n KCountryTest::testFromLocation_data.
    // For `canBeConflict=true` we only assert that if we return a valid country,
    // it must match the expected country.
    let cases = [
        (-8.0f32, -35.0f32, "BR", false),
        (44.0f32, -79.5f32, "CA", false),
        (52.4f32, 13.1f32, "DE", false),
        (-36.5f32, 175.0f32, "NZ", false),
        (46.0998f32, 7.781469f32, "CH", false),
        (50.8505f32, 5.6881f32, "NL", true),
        (50.7717f32, 6.04235f32, "DE", true),
        (46.23213f32, 6.10636f32, "CH", true),
    ];

    for (lat, lon, expected_alpha2, can_be_conflict) in cases {
        let got = ki18n_rs::country::Country::from_location(&base, lat, lon);
        if !can_be_conflict {
            let c = got.unwrap_or_else(|| panic!("expected country for lat={lat} lon={lon}"));
            assert_eq!(c.alpha2(&base), expected_alpha2, "lat={lat} lon={lon}");
        } else if let Some(c) = got {
            assert_eq!(c.alpha2(&base), expected_alpha2, "lat={lat} lon={lon}");
        }
    }
}

#[test]
fn test_parity_country_from_location_ki18n_autotest_data() {
    let base = data_dir();

    // Subset of ki18n/autotests/kcountrytest.cpp::testFromLocation_data
    // translated into Option semantics: invalid KCountry => None.
    let cases = [
        // invalid/out-of-coverage => empty country and canBeConflict=false => must be invalid.
        (f32::NAN, 25.0f32, None, false),
        (-90.0f32, 0.0f32, None, false),

        (-8.0f32, -35.0f32, Some("BR"), false),
        (44.0f32, -79.5f32, Some("CA"), false),
        (52.4f32, 13.1f32, Some("DE"), false),
        (-36.5f32, 175.0f32, Some("NZ"), false),

        (46.0998f32, 7.781469f32, Some("CH"), false),

        // canBeConflict=true: invalid is acceptable, but if we return valid it must match.
        (50.8505f32, 5.6881f32, Some("NL"), true),
        (50.7717f32, 6.04235f32, Some("DE"), true),
        (46.23213f32, 6.10636f32, Some("CH"), true),
        (47.69947f32, 8.68833f32, Some("DE"), true), // Busingen
        (32.54274f32, -116.97505f32, Some("MX"), true), // Tijuana

        // Baarle special case: house-by-house NL/BE differs, so empty country and canBeConflict=false.
        (51.44344f32, 4.93373f32, None, false),
    ];

    for (lat, lon, expected_alpha2_opt, can_be_conflict) in cases {
        let got = ki18n_rs::country::Country::from_location(&base, lat, lon);
        match (expected_alpha2_opt, can_be_conflict) {
            (None, false) => assert!(
                got.is_none(),
                "expected invalid country for lat={lat} lon={lon}, got={got:?}"
            ),
            (Some(expected), false) => {
                let c = got.expect(&format!("expected country {expected} for lat={lat} lon={lon}"));
                assert_eq!(c.alpha2(&base), expected, "lat={lat} lon={lon}");
            }
            (Some(expected), true) => {
                if let Some(c) = got {
                    assert_eq!(c.alpha2(&base), expected, "lat={lat} lon={lon}");
                }
            }
            (None, true) => {
                // not used currently
                assert!(got.is_none() || got.unwrap().alpha2(&base) != "".to_string());
            }
        }
    }
}
