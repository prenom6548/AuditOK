// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::fs;
use std::io::Write;
use std::path::PathBuf;
use std::sync::{Mutex, MutexGuard, OnceLock};
use std::time::{SystemTime, UNIX_EPOCH};
use std::time::Instant;

/// 串行化会修改 ki18n 全局状态（语言/domain/catalog）的集成测试，避免 `cargo test` 并行竞态。
#[allow(dead_code)]
pub fn global_state_test_lock() -> MutexGuard<'static, ()> {
    static LOCK: OnceLock<Mutex<()>> = OnceLock::new();
    LOCK.get_or_init(|| Mutex::new(()))
        .lock()
        .unwrap_or_else(|e| e.into_inner())
}

/// 串行化会修改进程环境变量的测试。
#[allow(dead_code)]
pub fn env_test_lock() -> MutexGuard<'static, ()> {
    static LOCK: OnceLock<Mutex<()>> = OnceLock::new();
    LOCK.get_or_init(|| Mutex::new(()))
        .lock()
        .unwrap_or_else(|e| e.into_inner())
}

/// 返回 tests/data 目录路径，如不存在则创建。
#[allow(dead_code)]
pub fn ensure_data_dir() -> PathBuf {
    let mut dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    dir.push("tests");
    dir.push("data");
    if !dir.exists() {
        fs::create_dir_all(&dir).expect("create tests/data failed");
    }
    dir
}

/// 构建最小 GNU gettext `.mo` 二进制（revision 0）。
#[allow(dead_code)]
pub fn build_test_mo_bytes(
    plural_header: &str,
    entries: &[(&str, &str)],
    big_endian: bool,
    revision: u32,
) -> Vec<u8> {
    let mut orig_strings: Vec<Vec<u8>> = Vec::new();
    let mut trans_strings: Vec<Vec<u8>> = Vec::new();

    let mut header = Vec::new();
    header.extend_from_slice(b"Content-Type: text/plain; charset=UTF-8\n");
    header.extend_from_slice(b"Plural-Forms: ");
    header.extend_from_slice(plural_header.as_bytes());
    header.extend_from_slice(b"\n");
    orig_strings.push(Vec::new());
    trans_strings.push(header);

    for (msgid, msgstr) in entries {
        orig_strings.push(msgid.as_bytes().to_vec());
        trans_strings.push(msgstr.as_bytes().to_vec());
    }

    let nstrings = orig_strings.len() as u32;
    let header_size: u32 = 28;
    let table_size: u32 = nstrings * 8 * 2;
    let orig_offset: u32 = header_size + table_size;
    let trans_offset: u32 =
        orig_offset + orig_strings.iter().map(|s| s.len() as u32 + 1).sum::<u32>();

    let write_u32 = |file: &mut Vec<u8>, v: u32| {
        if big_endian {
            file.extend_from_slice(&v.to_be_bytes());
        } else {
            file.extend_from_slice(&v.to_le_bytes());
        }
    };

    let mut file = Vec::new();
    // GNU .mo magic：磁盘上按 LE uint32 读取后应为 0x950412de（LE 文件）或 0xde120495（BE 文件）。
    let magic = if big_endian {
        0xde12_0495u32
    } else {
        0x9504_12deu32
    };
    file.extend_from_slice(&magic.to_le_bytes());

    write_u32(&mut file, revision);
    write_u32(&mut file, nstrings);
    write_u32(&mut file, header_size);
    write_u32(&mut file, header_size + (nstrings * 8));
    write_u32(&mut file, 0);
    write_u32(&mut file, 0);

    let mut cur_offset = orig_offset;
    for s in &orig_strings {
        let len = s.len() as u32;
        write_u32(&mut file, len);
        write_u32(&mut file, cur_offset);
        cur_offset += len + 1;
    }

    cur_offset = trans_offset;
    for s in &trans_strings {
        let len = s.len() as u32;
        write_u32(&mut file, len);
        write_u32(&mut file, cur_offset);
        cur_offset += len + 1;
    }

    for s in &orig_strings {
        file.extend_from_slice(s);
        file.push(0);
    }
    for s in &trans_strings {
        file.extend_from_slice(s);
        file.push(0);
    }
    file
}

/// 在 tests/data 下创建最小 .mo 文件，包含给定语言的 header 及少量测试条目。
#[allow(dead_code)]
pub fn write_test_mo(
    name: &str,
    plural_header: &str,
    entries: &[(&str, &str)],
) -> PathBuf {
    let dir = ensure_data_dir();
    let path = dir.join(name);
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_nanos())
        .unwrap_or(0);
    let tmp_path = dir.join(format!("{name}.{unique}.tmp"));

    let bytes = build_test_mo_bytes(plural_header, entries, false, 0);
    let mut file = fs::File::create(&tmp_path).expect("create mo");
    file.write_all(&bytes).expect("write mo");
    file.flush().expect("flush mo");
    drop(file);
    fs::rename(&tmp_path, &path).expect("rename mo");

    path
}

/// 性能测试计时辅助函数
#[allow(dead_code)]
pub fn measure<F: FnOnce() -> R, R>(label: &str, f: F) -> (R, u128) {
    let start = Instant::now();
    let result = f();
    let elapsed = start.elapsed().as_micros();
    println!("{} took {} µs", label, elapsed);
    (result, elapsed)
}

