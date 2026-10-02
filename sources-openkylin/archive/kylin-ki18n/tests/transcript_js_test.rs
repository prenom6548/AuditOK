// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};

use ki18n_rs::paths::ki18n_autotests_dir;
use ki18n_rs::transcript::eval_transcript_js_call;

fn test_module_path(rel: &str) -> PathBuf {
    ki18n_autotests_dir()
        .join(rel)
        .canonicalize()
        .unwrap_or_else(|_| ki18n_autotests_dir().join(rel))
}

#[test]
fn test_ts_api_compat_with_ki18n_test_js() {
    let language = "fr";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";

    let mut dynctxt = HashMap::new();
    dynctxt.insert("origin".to_string(), "neverwhere".to_string());

    let subs = vec!["10".to_string(), "qwyx".to_string()];
    let vals = subs.clone();

    let test_js = test_module_path("test.js");
    let module_files = [test_js.as_path()];
    let autotests_dir = test_js.parent().unwrap().to_path_buf();
    let generated_dir = autotests_dir.join("generated");
    fs::create_dir_all(&generated_dir).unwrap();

    // Ensure a compiled props map (.pmapc) is available and preferred over .pmap.
    // We create both .pmap (wrong value) and .pmapc (correct value) for the same relpath.
    let relpath = "generated/testmap";
    let pmap_path = autotests_dir.join(format!("{}.pmap", relpath));
    let pmapc_path = autotests_dir.join(format!("{}.pmapc", relpath));
    fs::write(
        &pmap_path,
        b"=:Athens:Atina:nom=Atina:gen=WRONG:dat=Atini:acc=Atinu::\n",
    )
    .unwrap();

    // TSPMAP00 binary with 1 entry:
    //   ekey:  "athens"
    //   prop:  "gen" -> "Atine"
    fn push_u32_be(out: &mut Vec<u8>, v: u32) {
        out.extend_from_slice(&v.to_be_bytes());
    }
    fn push_string(out: &mut Vec<u8>, s: &str) {
        let bytes = s.as_bytes();
        push_u32_be(out, bytes.len() as u32);
        out.extend_from_slice(bytes);
    }

    let mut bin: Vec<u8> = Vec::new();
    bin.extend_from_slice(b"TSPMAP00");
    push_u32_be(&mut bin, 1); // nentries
    push_u32_be(&mut bin, 1); // nekeys
    push_string(&mut bin, "athens"); // ekey
    push_u32_be(&mut bin, 1); // nprops
    push_string(&mut bin, "gen"); // pkey
    push_string(&mut bin, "Atine"); // pval
    fs::write(&pmapc_path, &bin).unwrap();

    // (argv, expected_result, expected_fallback)
    let cases: &[(&[&str], &str, bool)] = &[
        (&["test_basic", "foo"], "foo bar", false),
        (&["test_unicode", "čгσィ九"], "čгσィ九 фу", false),
        (&["test_hascall", "test_basic"], "yes", false),
        (&["test_acall", "test_basic", "qwyx"], "qwyx bar", false),
        (&["test_load"], "foo blurb", false),
        (&["test_fallback"], "", true),
        (&["test_msgid"], "source-text", false),
        (&["test_msgtrf"], "translated-text", false),
        (&["test_msgctxt"], "a-context", false),
        (&["test_msgkey"], "a-context|source-text", false),
        (&["test_nsubs"], "2", false),
        (&["test_subs", "1"], "qwyx", false),
        (&["test_vals", "0", "5"], "50", false),
        (&["test_dynctxt", "origin"], "neverwhere", false),
        (&["test_dbgputs"], "debugged", false),
        (&["test_warnputs"], "warned", false),
        (&["test_setcallForall"], "done", false),
        (&["test_toUpperFirst", "...123 foo"], "...123 Foo", false),
        (&["test_toUpperFirst", "...123 фу"], "...123 Фу", false),
        (&["test_toLowerFirst", "...123 FOO"], "...123 fOO", false),
        (&["test_loadProps", "cities"], "loaded", false),
        (&["test_getProp", "cities", "Athens", "gen"], "Atine", false),
        (&["test_getProp", "generated/testmap", "Athens", "gen"], "Atine", false),
        (&["test_setProp", "Oslo", "dat", "Oslou"], "Oslou", false),
        (&["test_normKey", "Some &Thing"], "something", false),
        (&["test_getConfString", "StringKey"], "StringValue", false),
        (
            &["test_getConfStringWithDefault", "NoSuchKey", "DefaultValue"],
            "DefaultValue",
            false,
        ),
        (&["test_getConfBool", "BoolKey"], "true", false),
        (&["test_getConfBoolWithDefault", "NoSuchKey", "true"], "true", false),
        (&["test_getConfNumber", "NumberKey"], "12345", false),
        (&["test_getConfNumberWithDefault", "NoSuchKey", "54321"], "54321", false),
    ];

    for (argv, expected, expected_fallback) in cases {
        let argv_s: Vec<String> = argv.iter().map(|s| s.to_string()).collect();

        let (result, fallback) = eval_transcript_js_call(
            &argv_s,
            language,
            msgctxt,
            &dynctxt,
            msgid,
            &subs,
            &vals,
            msgstrf,
            &module_files,
        )
        .unwrap_or_else(|e| panic!("case argv={:?} failed: {}", argv, e));

        assert_eq!(
            fallback, *expected_fallback,
            "case argv={:?}",
            argv
        );
        assert_eq!(result, *expected, "case argv={:?}", argv);
    }
}

/// 与 `ki18n/autotests/cities.pmap`（仅文本、无 .pmapc）+ `ktranscripttest::test_getProp` 路径一致。
#[test]
fn test_upstream_cities_pmap_text_only_fixture() {
    let language = "fr";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";
    let dynctxt: HashMap<String, String> = HashMap::new();
    let subs = vec!["10".to_string(), "qwyx".to_string()];
    let vals = subs.clone();

    let autotests = ki18n_autotests_dir();
    let pmap_path = autotests.join("cities.pmap");
    assert!(
        pmap_path.is_file(),
        "missing bundled/synced cities.pmap at {:?}",
        pmap_path
    );
    let raw = fs::read_to_string(&pmap_path).expect("cities.pmap");
    assert!(
        raw.contains("Athens") && raw.contains("gen=Atine"),
        "unexpected cities.pmap content"
    );
    assert!(
        !autotests.join("cities.pmapc").exists(),
        "cities.pmapc would shadow text pmap unlike upstream autotests"
    );

    let test_js = test_module_path("test.js");
    let module_files = [test_js.as_path()];
    let argv = vec![
        "test_getProp".to_string(),
        "cities".to_string(),
        "Athens".to_string(),
        "gen".to_string(),
    ];
    let (result, fallback) = eval_transcript_js_call(
        &argv,
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files,
    )
    .expect("eval test_getProp cities");
    assert!(!fallback);
    assert_eq!(result, "Atine");
}

#[test]
fn test_ts_load_submodule_relative_base_isolated_by_call_definition() {
    use std::collections::HashMap;

    let language = "en";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";

    let dynctxt = HashMap::new();
    let subs: Vec<String> = vec![];
    let vals: Vec<String> = vec![];

    // Reuse autotests/test.js directory as the base for generated modules.
    let test_js = test_module_path("test.js");
    let autotests_dir = test_js.parent().unwrap().to_path_buf();
    let generated_dir = autotests_dir.join("generated");
    fs::create_dir_all(&generated_dir).unwrap();

    let sub_a = generated_dir.join("subA");
    let sub_b = generated_dir.join("subB");
    let module_a = generated_dir.join("moduleA");
    let module_b = generated_dir.join("moduleB");

    fs::write(
        sub_a.with_extension("js"),
        r#"
        Ts.setcall("markA", function() { return "markA"; });
    "#,
    )
    .unwrap();
    fs::write(
        sub_b.with_extension("js"),
        r#"
        Ts.setcall("markB", function() { return "markB"; });
    "#,
    )
    .unwrap();

    // callA should load subA relative to the directory where moduleA is defined,
    // even if moduleB was loaded before/after it.
    fs::write(
        module_a.with_extension("js"),
        r#"
        Ts.setcall("callA", function() {
            Ts.load("subA");
            return Ts.acall("markA");
        });
    "#,
    )
    .unwrap();
    fs::write(
        module_b.with_extension("js"),
        r#"
        Ts.setcall("callB", function() {
            Ts.load("subB");
            return Ts.acall("markB");
        });
    "#,
    )
    .unwrap();

    let module_a_path = module_a.with_extension("js");
    let module_b_path = module_b.with_extension("js");

    // Order 1: moduleA then moduleB.
    let module_files1 = [&module_a_path as &Path, &module_b_path as &Path];
    let (res_a1, fb_a1) = eval_transcript_js_call(
        &["callA".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files1,
    )
    .unwrap();
    assert!(!fb_a1);
    assert_eq!(res_a1, "markA");

    // Order 2: moduleB then moduleA.
    let module_files2 = [&module_b_path as &Path, &module_a_path as &Path];
    let (res_a2, fb_a2) = eval_transcript_js_call(
        &["callA".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files2,
    )
    .unwrap();
    assert!(!fb_a2);
    assert_eq!(res_a2, "markA");

    // Control: callB should still work.
    let (res_b, fb_b) = eval_transcript_js_call(
        &["callB".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files2,
    )
    .unwrap();
    assert!(!fb_b);
    assert_eq!(res_b, "markB");
}

#[test]
fn test_ts_load_props_multiple_args_and_cumulative_merge() {
    use std::time::{SystemTime, UNIX_EPOCH};

    let language = "en";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";

    let dynctxt = HashMap::new();
    let subs: Vec<String> = vec![];
    let vals: Vec<String> = vec![];

    let test_js = test_module_path("test.js");
    let autotests_dir = test_js.parent().unwrap().to_path_buf();
    let generated_dir = autotests_dir.join("generated");
    fs::create_dir_all(&generated_dir).unwrap();

    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let unique = format!("lp_{nanos}");

    let map1_rel = format!("map1_{unique}");
    let map2_rel = format!("map2_{unique}");
    let map1_pmapc_path = generated_dir.join(format!("{}.pmapc", map1_rel));
    let map2_pmapc_path = generated_dir.join(format!("{}.pmapc", map2_rel));

    // Build TSPMAP00 with 1 entry, 1 ekey ("athens"), and 1 prop.
    fn push_u32_be(out: &mut Vec<u8>, v: u32) {
        out.extend_from_slice(&v.to_be_bytes());
    }
    fn push_string(out: &mut Vec<u8>, s: &str) {
        let bytes = s.as_bytes();
        push_u32_be(out, bytes.len() as u32);
        out.extend_from_slice(bytes);
    }
    fn write_tspmap00_one_entry_one_prop(path: &std::path::Path, prop_key: &str, prop_val: &str) {
        let mut bin: Vec<u8> = Vec::new();
        bin.extend_from_slice(b"TSPMAP00");
        push_u32_be(&mut bin, 1); // nentries
        push_u32_be(&mut bin, 1); // nekeys
        push_string(&mut bin, "athens"); // ekey
        push_u32_be(&mut bin, 1); // nprops
        push_string(&mut bin, prop_key); // pkey
        push_string(&mut bin, prop_val); // pval
        fs::write(path, &bin).unwrap();
    }

    write_tspmap00_one_entry_one_prop(&map1_pmapc_path, "gen", "Atine1");
    write_tspmap00_one_entry_one_prop(&map2_pmapc_path, "dat", "Atini2");

    let module_rel = format!("module_{unique}");
    let module_path = generated_dir.join(format!("{}.js", module_rel));
    fs::write(
        &module_path,
        format!(
            r#"
            Ts.setcall("loadTwoProps", function() {{
                Ts.loadProps("{map1_rel}");
                Ts.loadProps("{map2_rel}");
                return Ts.getProp("Athens","gen") + "|" + Ts.getProp("Athens","dat");
            }});

            Ts.setcall("loadPropsArray", function() {{
                Ts.loadProps(["{map1_rel}", "{map2_rel}"]);
                return Ts.getProp("Athens","gen") + "|" + Ts.getProp("Athens","dat");
            }});

            Ts.setcall("loadPropsVarargs", function() {{
                Ts.loadProps("{map1_rel}", "{map2_rel}");
                return Ts.getProp("Athens","gen") + "|" + Ts.getProp("Athens","dat");
            }});
        "#
        ),
    )
    .unwrap();

    let module_file = module_path.as_path();
    let module_files: [&Path; 1] = [module_file];

    for (call, expected) in [
        ("loadTwoProps", "Atine1|Atini2"),
        ("loadPropsArray", "Atine1|Atini2"),
        ("loadPropsVarargs", "Atine1|Atini2"),
    ] {
        let (result, fb) = eval_transcript_js_call(
            &[call.to_string()],
            language,
            msgctxt,
            &dynctxt,
            msgid,
            &subs,
            &vals,
            msgstrf,
            &module_files,
        )
        .unwrap();
        assert!(!fb);
        assert_eq!(result, expected);
    }
}

#[test]
fn test_ts_load_props_relative_base_isolated_by_call_definition() {
    use std::time::{SystemTime, UNIX_EPOCH};

    let language = "en";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";

    let dynctxt = HashMap::new();
    let subs: Vec<String> = vec![];
    let vals: Vec<String> = vec![];

    let test_js = test_module_path("test.js");
    let autotests_dir = test_js.parent().unwrap().to_path_buf();
    let generated_dir = autotests_dir.join("generated");
    fs::create_dir_all(&generated_dir).unwrap();

    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let unique = format!("lp_base_{nanos}");

    let dir_a = generated_dir.join(format!("{unique}_A"));
    let dir_b = generated_dir.join(format!("{unique}_B"));
    fs::create_dir_all(&dir_a).unwrap();
    fs::create_dir_all(&dir_b).unwrap();

    let pmap_rel = "mymap";
    let pmapc_a = dir_a.join(format!("{}.pmapc", pmap_rel));
    let pmapc_b = dir_b.join(format!("{}.pmapc", pmap_rel));

    // Helpers to create a minimal TSPMAP00 with 1 ekey and 1 prop.
    fn push_u32_be(out: &mut Vec<u8>, v: u32) {
        out.extend_from_slice(&v.to_be_bytes());
    }
    fn push_string(out: &mut Vec<u8>, s: &str) {
        let bytes = s.as_bytes();
        push_u32_be(out, bytes.len() as u32);
        out.extend_from_slice(bytes);
    }
    fn write_tspmap00_one_entry_one_prop(path: &std::path::Path, pval: &str) {
        let mut bin: Vec<u8> = Vec::new();
        bin.extend_from_slice(b"TSPMAP00");
        push_u32_be(&mut bin, 1); // nentries
        push_u32_be(&mut bin, 1); // nekeys
        push_string(&mut bin, "athens"); // ekey (normalized to "athens")
        push_u32_be(&mut bin, 1); // nprops
        push_string(&mut bin, "gen"); // pkey
        push_string(&mut bin, pval); // pval
        fs::write(path, &bin).unwrap();
    }

    write_tspmap00_one_entry_one_prop(&pmapc_a, "AtineA");
    write_tspmap00_one_entry_one_prop(&pmapc_b, "AtineB");

    let module_a = dir_a.join("moduleA.js");
    let module_b = dir_b.join("moduleB.js");

    fs::write(
        &module_a,
        r#"
        Ts.setcall("callA", function() {
            Ts.loadProps("mymap");
            return Ts.getProp("Athens","gen");
        });
    "#,
    )
    .unwrap();
    fs::write(
        &module_b,
        r#"
        Ts.setcall("callB", function() {
            Ts.loadProps("mymap");
            return Ts.getProp("Athens","gen");
        });
    "#,
    )
    .unwrap();

    let module_a_path = module_a.as_path();
    let module_b_path = module_b.as_path();

    // Order doesn't matter: callA should always load from moduleA's directory.
    let module_files1: [&Path; 2] = [&module_a_path, &module_b_path];
    let (res_a1, fb_a1) = eval_transcript_js_call(
        &["callA".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files1,
    )
    .unwrap();
    assert!(!fb_a1);
    assert_eq!(res_a1, "AtineA");

    let module_files2: [&Path; 2] = [&module_b_path, &module_a_path];
    let (res_a2, fb_a2) = eval_transcript_js_call(
        &["callA".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files2,
    )
    .unwrap();
    assert!(!fb_a2);
    assert_eq!(res_a2, "AtineA");

    // callB should load from moduleB's directory.
    let (res_b, fb_b) = eval_transcript_js_call(
        &["callB".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files2,
    )
    .unwrap();
    assert!(!fb_b);
    assert_eq!(res_b, "AtineB");
}

#[test]
fn test_ts_load_loadprops_errors_and_scriptfaceinternal_pins() {
    use std::time::{SystemTime, UNIX_EPOCH};

    let language = "en";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";
    let dynctxt = HashMap::new();
    let subs: Vec<String> = vec![];
    let vals: Vec<String> = vec![];

    let test_js = test_module_path("test.js");
    let autotests_dir = test_js.parent().unwrap().to_path_buf();
    let generated_dir = autotests_dir.join("generated");
    fs::create_dir_all(&generated_dir).unwrap();

    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let unique = format!("ts_up_err_{nanos}");
    let mod_path = generated_dir.join(format!("{unique}.js"));

    fs::write(
        &mod_path,
        r#"
        Ts.setcall("loadNoPath", function() {
            Ts._currentModulePath = "";
            Ts.load("x");
        });
        Ts.setcall("loadMissing", function() {
            Ts.load("___no_such_module_file___");
        });
        Ts.setcall("loadPropsNoPath", function() {
            Ts._currentModulePath = "";
            Ts.loadProps("m");
        });
        Ts.setcall("loadPropsMissing", function() {
            Ts.loadProps("___no_such_pmap___");
        });
        Ts.setcall("gcPinMark", function() {
            Ts.setcall("tempGc", function() { return "ok"; });
            var k = '#:f<' + 'tempGc' + '>';
            return typeof ScriptfaceInternal[k] === "function" ? "yes" : "no";
        });
    "#,
    )
    .unwrap();

    let module_files = [mod_path.as_path()];

    let e = eval_transcript_js_call(
        &["loadNoPath".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files,
    )
    .unwrap_err();
    assert!(
        e.contains("no current module path") && e.contains("Ts.load"),
        "got {e}"
    );

    let e = eval_transcript_js_call(
        &["loadMissing".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files,
    )
    .unwrap_err();
    assert!(
        e.contains("cannot read file") && e.contains("Ts.load"),
        "got {e}"
    );

    let e = eval_transcript_js_call(
        &["loadPropsNoPath".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files,
    )
    .unwrap_err();
    assert!(
        e.contains("no current module path") && e.contains("loadProps"),
        "got {e}"
    );

    let e = eval_transcript_js_call(
        &["loadPropsMissing".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files,
    )
    .unwrap_err();
    assert!(
        e.contains("cannot read map") && e.contains("loadProps"),
        "got {e}"
    );

    let (pin, fb) = eval_transcript_js_call(
        &["gcPinMark".to_string()],
        language,
        msgctxt,
        &dynctxt,
        msgid,
        &subs,
        &vals,
        msgstrf,
        &module_files,
    )
    .unwrap();
    assert!(!fb);
    assert_eq!(pin, "yes");
}

/// Deliberately invalid `.pmap` text must surface `parse_pmap_text` errors (aligned with
/// `Scriptface::loadProps_text`) through `Ts.loadProps`.
#[test]
fn test_ts_loadprops_malformed_pmap_text_errors() {
    use std::time::{SystemTime, UNIX_EPOCH};

    let language = "en";
    let msgctxt = "a-context";
    let msgid = "source-text";
    let msgstrf = "translated-text";
    let dynctxt = HashMap::new();
    let subs: Vec<String> = vec![];
    let vals: Vec<String> = vec![];

    let test_js = test_module_path("test.js");
    let autotests_dir = test_js.parent().unwrap().to_path_buf();
    let generated_dir = autotests_dir.join("generated");
    fs::create_dir_all(&generated_dir).unwrap();

    let nanos = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap()
        .as_nanos();
    let unique = format!("bad_pmap_{nanos}");
    let case_dir = generated_dir.join(&unique);
    fs::create_dir_all(&case_dir).unwrap();

    let mod_path = case_dir.join("loader.js");
    fs::write(
        &mod_path,
        r#"
        Ts.setcall("loadBadTextPmap", function (rel) {
            Ts.loadProps(rel);
        });
    "#,
    )
    .unwrap();

    let cases: &[(&str, &[&str])] = &[
        // Letter-like entry separators (must not be alphabetic).
        ("bad_sep", &["separator characters must not be letters", "Ts.loadProps_text"]),
        // Only one character after whitespace: cannot read key/prop separators.
        ("bad_eof1", &["unexpected end of file", "Ts.loadProps_text"]),
        // `=` inside a property value before the next `:` (prop_sep).
        ("bad_in_value", &["property separator inside property value", "Ts.loadProps_text"]),
        // `=::` — first entry key segment empty with no prior ekeys.
        ("bad_no_ekey", &["no entry key for entry ending at", "Ts.loadProps_text"]),
        // Truncated record: parser not back in NextEntry at EOF.
        ("bad_trunc", &["Ts.loadProps: unexpected end of file"]),
    ];

    for (rel_base, needles) in cases {
        let pmap_path = case_dir.join(format!("{rel_base}.pmap"));
        let content: &[u8] = match *rel_base {
            "bad_sep" => b"ab",
            "bad_eof1" => b"=",
            "bad_in_value" => b"=:Athens:nom=foo=bar::\n",
            "bad_no_ekey" => b"=::\n",
            "bad_trunc" => b"=:Athens:nom=foo",
            _ => unreachable!(),
        };
        fs::write(&pmap_path, content).unwrap();

        let argv = vec![
            "loadBadTextPmap".to_string(),
            (*rel_base).to_string(),
        ];
        let e = eval_transcript_js_call(
            &argv,
            language,
            msgctxt,
            &dynctxt,
            msgid,
            &subs,
            &vals,
            msgstrf,
            &[mod_path.as_path()],
        )
        .unwrap_err();

        let pmap_display = pmap_path.display().to_string();
        for needle in *needles {
            assert!(
                e.contains(needle),
                "rel={rel_base} expected substring {needle:?} in error, got: {e}\npath={pmap_display}"
            );
        }
    }
}

