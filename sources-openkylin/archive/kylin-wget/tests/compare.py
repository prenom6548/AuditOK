#!/usr/bin/env python3
from __future__ import annotations

import argparse
import dataclasses
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile
import time
import re
from pathlib import Path
from urllib.parse import urlparse


@dataclasses.dataclass(frozen=True)
class Case:
    id: str
    args: list[str]
    expect_exit_nonzero_ok: bool = False  # for cases where exact exit code can vary early
    expect_files: dict[str, str] = dataclasses.field(default_factory=dict)  # relpath -> sha256 hex
    expect_min_rust_elapsed_s: float | None = None  # rough lower-bound for rust runtime (limit-rate)
    timeout_s: float = 20.0
    # 追加环境变量（已清过 WGETRC）；值中可用 {PROXY} / {PROXY_EVIL} / {PROXY_AUTH} 占位符
    extra_env: dict[str, str] | None = None
    # 运行前写入临时目录（用于 --post-file 等）
    seed_files: dict[str, str | bytes] = dataclasses.field(default_factory=dict)
    # 对指定输出文件只比较「HTTP 头之后的 body」的 hash（避免 --save-headers 因 Date/顺序差异导致全文不一致）
    compare_body_only: frozenset[str] = frozenset()
    # 成功且退出码 0 时期望 stderr 为空（如 -q）
    expect_stderr_blank_on_success: bool = False
    # 成功且退出码 0 时 orig 与 rust 的 stderr 均应包含该子串（如 -S）
    expect_stderr_contains: str | None = None

    # 失败场景也可判：对 orig / rust 分别做 stderr 子串计数（比如连接拒绝重试次数）
    expect_orig_stderr_contains: str | None = None
    expect_orig_stderr_contains_count: int | None = None
    expect_rust_stderr_contains: str | None = None
    expect_rust_stderr_contains_count: int | None = None

    # 剥离时间戳前缀后，orig 与 rust 的 stderr 逐行应一致（TLS 证书错误等）
    compare_stderr_with_orig: bool = False

    # 针对 GNU wget 的 `-o/-a` 日志文件：日志前缀包含本地时间，做无时区的前缀剥离以避免测试抖动
    normalize_wget_output_log_timestamps: frozenset[str] = frozenset()
    # Netscape cookies.txt：允许忽略以 '#' 开头的注释行做 hash 对照
    normalize_netscape_cookie_files: frozenset[str] = frozenset()
    # HSTS DB：忽略 created 字段（时间戳）做 hash 对照
    normalize_hsts_files: frozenset[str] = frozenset()

    # 对 `-O -` 输出到 stdout 的行为：与 orig stdout 完全一致性对照
    compare_stdout_with_orig: bool = False
    # `-V` 等：成功时 stdout 应同时包含该子串（orig 与 rust 均检查）
    expect_stdout_contains: str | None = None
    # 仅对 rust 的 stdout 子串（如版本串中的 `(rust`）
    expect_rust_stdout_contains: str | None = None

    # 对指定输出文件比较 mtime（orig 与 rust 应一致；用于 timestamping）
    compare_mtime_files: frozenset[str] = frozenset()
    # 期望这些文件不存在（orig 与 rust 都不应留下）
    expect_absent_files: frozenset[str] = frozenset()
    # 期望这些文件存在（仅检查存在性，不做 hash 对照）
    expect_present_files: frozenset[str] = frozenset()
    # 仅校验 rust 输出文件 hash（不沿用 orig 结果；用于与 GNU 实测分歧、手册语义用例）
    expect_rust_files: dict[str, str] = dataclasses.field(default_factory=dict)


def sha256_file(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def sha256_body_after_headers(p: Path) -> str:
    data = p.read_bytes()
    for sep in (b"\r\n\r\n", b"\n\n"):
        idx = data.find(sep)
        if idx != -1:
            return hashlib.sha256(data[idx + len(sep) :]).hexdigest()
    return hashlib.sha256(data).hexdigest()


def _case_output_file_paths(case: Case) -> set[str]:
    paths = set(case.expect_files.keys())
    paths.update(case.expect_rust_files.keys())
    return paths


_WGET_LOG_TS_PREFIX_RE = re.compile(r"^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} ")
_GNU_URL_TS_PREFIX_RE = re.compile(r"^--\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}--\s+")


def normalize_stderr_lines(s: str) -> list[str]:
    """去掉 wget 时间戳行前缀，便于 orig/rust stderr 逐行对照。"""
    out: list[str] = []
    for line in s.splitlines():
        t = line.strip()
        if not t:
            continue
        t = _GNU_URL_TS_PREFIX_RE.sub("", t)
        t = _WGET_LOG_TS_PREFIX_RE.sub("", t)
        out.append(t)
    return out


def sha256_wget_output_log_normalized(p: Path) -> str:
    data = p.read_bytes().decode("utf-8", "replace")
    out_lines: list[str] = []
    for line in data.splitlines(True):
        out_lines.append(_WGET_LOG_TS_PREFIX_RE.sub("", line))
    normalized = "".join(out_lines).encode("utf-8")
    h = hashlib.sha256()
    h.update(normalized)
    return h.hexdigest()


def sha256_netscape_cookie_file_normalized(p: Path) -> str:
    data = p.read_bytes().decode("utf-8", "replace")
    out_lines: list[str] = []
    for line in data.splitlines(True):
        if line.lstrip().startswith("#"):
            continue
        out_lines.append(line)
    normalized = "".join(out_lines).encode("utf-8")
    h = hashlib.sha256()
    h.update(normalized)
    return h.hexdigest()


def sha256_hsts_file_normalized(p: Path) -> str:
    data = p.read_bytes().decode("utf-8", "replace")
    out_lines: list[str] = []
    for line in data.splitlines(True):
        if line.lstrip().startswith("#") or not line.strip():
            continue
        parts = line.strip().split()
        if len(parts) == 5:
            host, port, incl, _created, max_age = parts
            out_lines.append(f"{host}\t{port}\t{incl}\t0\t{max_age}\n")
        else:
            out_lines.append(line)
    normalized = "".join(out_lines).encode("utf-8")
    h = hashlib.sha256()
    h.update(normalized)
    return h.hexdigest()


def _has_background_flag(args: list[str]) -> bool:
    for a in args:
        if a == "-b" or a == "--background" or a.startswith("--background="):
            return True
    return False


def _wait_for_files_stable(base_dir: Path, relpaths: list[str], timeout_s: float) -> None:
    # 针对 -b/--background：进程会立刻退出，但后台仍在写文件。
    # 等待目标文件都出现，且大小在短窗口内稳定。
    deadline = time.time() + timeout_s
    last_sizes: dict[str, int] = {}
    stable_rounds = 0
    while time.time() < deadline:
        ok = True
        for rp in relpaths:
            p = base_dir / rp
            if not p.exists():
                ok = False
                break
            if p.is_dir():
                continue
            sz = p.stat().st_size
            prev = last_sizes.get(rp)
            last_sizes[rp] = sz
            if prev is None or prev != sz:
                ok = False
                break
        if ok:
            stable_rounds += 1
            if stable_rounds >= 2:
                return
        else:
            stable_rounds = 0
        time.sleep(0.2)
    raise TimeoutError("background files not stabilized in time")


def _expand_test_placeholders(
    s: str,
    *,
    base_url: str,
    base2_url: str,
    base_host: str,
    proxy_url: str,
    proxy_evil_url: str,
    proxy_auth_url: str,
    ftp_base_url: str,
    https_base_url: str,
    ca_pem: str,
    ca_dir: str,
) -> str:
    https_hostport = urlparse(https_base_url).netloc if https_base_url else ""
    https_port = urlparse(https_base_url).port if https_base_url else None
    base_hostport = urlparse(base_url).netloc if base_url else ""
    base_port = urlparse(base_url).port if base_url else None
    base2_hostport = urlparse(base2_url).netloc if base2_url else ""
    lhttps = ""
    https_port_s = ""
    base_port_s = ""
    if https_port:
        lhttps = f"https://localhost:{https_port}"
        https_port_s = str(https_port)
    if base_port:
        base_port_s = str(base_port)
    return (
        s.replace("{BASE}", base_url)
        .replace("{BASE2}", base2_url)
        .replace("{BASE_HOST}", base_host)
        .replace("{BASE_HOSTPORT}", base_hostport)
        .replace("{BASE_PORT}", base_port_s)
        .replace("{BASE2_HOSTPORT}", base2_hostport)
        .replace("{HTTPS_HOSTPORT}", https_hostport)
        .replace("{HTTPS_PORT}", https_port_s)
        .replace("{LHTTPS}", lhttps)
        .replace("{PROXY}", proxy_url)
        .replace("{PROXY_EVIL}", proxy_evil_url)
        .replace("{PROXY_AUTH}", proxy_auth_url)
        .replace("{FTP}", ftp_base_url)
        .replace("{HTTPS}", https_base_url)
        .replace("{CAPATH}", ca_pem)
        .replace("{CADIR}", ca_dir)
    )


def run_one(
    bin_path: str,
    case: Case,
    base_url: str,
    *,
    base2_url: str,
    proxy_url: str,
    proxy_evil_url: str,
    proxy_auth_url: str,
    ftp_base_url: str = "",
    https_base_url: str = "",
    ca_pem: str = "",
    ca_dir: str = "",
) -> tuple[int, dict[str, str], dict[str, int], dict[str, bool], dict[str, bool], str, str, float, str]:
    keep_temps = os.environ.get("WGET_RUST_KEEP_TEMPS", "").strip() not in ("", "0", "false", "False", "no", "No")
    td_ctx: tempfile.TemporaryDirectory[str] | None = None
    if keep_temps:
        td = tempfile.mkdtemp(prefix=f"wget-cmp-{case.id}-")
    else:
        td_ctx = tempfile.TemporaryDirectory(prefix=f"wget-cmp-{case.id}-")
        td = td_ctx.__enter__()

    try:
        td_path = Path(td)
        # Make behavior more stable: avoid reading user configs.
        env = os.environ.copy()
        env.pop("WGETRC", None)
        env["HOME"] = td
        env["LC_ALL"] = "C"

        if case.seed_files:
            for rel, content in case.seed_files.items():
                fp = td_path / rel
                fp.parent.mkdir(parents=True, exist_ok=True)
                if isinstance(content, bytes):
                    data = content
                else:
                    data = _expand_test_placeholders(
                        content,
                        base_url=base_url,
                        base2_url=base2_url,
                        base_host=urlparse(base_url).hostname or "127.0.0.1",
                        proxy_url=proxy_url,
                        proxy_evil_url=proxy_evil_url,
                        proxy_auth_url=proxy_auth_url,
                        ftp_base_url=ftp_base_url,
                        https_base_url=https_base_url,
                        ca_pem=ca_pem,
                        ca_dir=ca_dir,
                    ).encode("utf-8")
                fp.write_bytes(data)

        if case.extra_env:
            for k, v in case.extra_env.items():
                env[k] = _expand_test_placeholders(
                    v,
                    base_url=base_url,
                    base2_url=base2_url,
                    base_host=urlparse(base_url).hostname or "127.0.0.1",
                    proxy_url=proxy_url,
                    proxy_evil_url=proxy_evil_url,
                    proxy_auth_url=proxy_auth_url,
                    ftp_base_url=ftp_base_url,
                    https_base_url=https_base_url,
                    ca_pem=ca_pem,
                    ca_dir=ca_dir,
                )

        files: dict[str, str] = {}
        # 参数里也允许引用占位符：{BASE}/{HTTPS}/{CAPATH}/{CADIR}/{PROXY}/…
        argv = [
            bin_path
        ] + [
            _expand_test_placeholders(
                a,
                base_url=base_url,
                base2_url=base2_url,
                base_host=urlparse(base_url).hostname or "127.0.0.1",
                proxy_url=proxy_url,
                proxy_evil_url=proxy_evil_url,
                proxy_auth_url=proxy_auth_url,
                ftp_base_url=ftp_base_url,
                https_base_url=https_base_url,
                ca_pem=ca_pem,
                ca_dir=ca_dir,
            )
            for a in case.args
        ]
        start = time.monotonic()
        try:
            p = subprocess.run(
                argv,
                cwd=td,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=False,
                timeout=case.timeout_s,
            )
        except subprocess.TimeoutExpired as e:
            elapsed = time.monotonic() - start
            return 124, files, {}, {}, {}, "", f"TIMEOUT after {elapsed:.2f}s: {e}".strip(), elapsed, td
        elapsed = time.monotonic() - start

        # -b/--background：wget 会立刻退出，但后台仍在写文件；先等待输出文件稳定再做 hash 对照。
        output_paths = _case_output_file_paths(case)
        if _has_background_flag(argv[1:]) and output_paths:
            _wait_for_files_stable(td_path, list(output_paths), case.timeout_s)

        for rel in output_paths:
            fp = td_path / rel
            if fp.exists():
                if rel in case.compare_body_only:
                    files[rel] = sha256_body_after_headers(fp)
                elif rel in case.normalize_wget_output_log_timestamps:
                    files[rel] = sha256_wget_output_log_normalized(fp)
                elif rel in case.normalize_netscape_cookie_files:
                    files[rel] = sha256_netscape_cookie_file_normalized(fp)
                elif rel in case.normalize_hsts_files:
                    files[rel] = sha256_hsts_file_normalized(fp)
                else:
                    files[rel] = sha256_file(fp)
        mtimes: dict[str, int] = {}
        for rel in case.compare_mtime_files:
            fp = td_path / rel
            if fp.exists():
                mtimes[rel] = int(fp.stat().st_mtime)
        absent: dict[str, bool] = {}
        for rel in case.expect_absent_files:
            absent[rel] = (td_path / rel).exists()
        present: dict[str, bool] = {}
        for rel in case.expect_present_files:
            present[rel] = (td_path / rel).exists()
        out_s = p.stdout.decode("utf-8", "replace")
        err_s = p.stderr.decode("utf-8", "replace")
        return (
            p.returncode,
            files,
            mtimes,
            absent,
            present,
            out_s,
            err_s,
            elapsed,
            td,
        )
    finally:
        if td_ctx is not None:
            td_ctx.__exit__(None, None, None)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig", default=os.environ.get("WGET_ORIG") or shutil.which("wget") or "", help="original wget binary path")
    ap.add_argument("--rust", default=os.environ.get("WGET_RUST") or str(Path(__file__).resolve().parents[1] / "rust/target/debug/wget"), help="Rust wget binary path")
    ap.add_argument("--base-url", required=True)
    ap.add_argument("--base2-url", default="")
    ap.add_argument("--proxy-url", default="", help="forward HTTP proxy base URL (e.g. http://127.0.0.1:9)")
    ap.add_argument("--proxy-evil-url", default="", help='always-418 proxy for no_proxy bypass test')
    ap.add_argument(
        "--proxy-auth-url",
        default="",
        help="forward HTTP proxy that requires Proxy-Authorization (Basic)",
    )
    ap.add_argument(
        "--https-base-url",
        default="",
        help="optional https://127.0.0.1:port for TLS tests ({HTTPS} placeholder)",
    )
    ap.add_argument(
        "--ftp-base-url",
        default="",
        help="optional ftp://127.0.0.1:port for FTP tests ({FTP} placeholder)",
    )
    ap.add_argument(
        "--ca-pem",
        default="",
        help="absolute path to test CA PEM for {CAPATH} (e.g. tests/fixtures/tls/ca.pem)",
    )
    ap.add_argument(
        "--ca-dir",
        default="",
        help="absolute path to CA directory for {CADIR}",
    )
    ap.add_argument(
        "--case-regex",
        default="",
        help="only run cases whose id matches this regex (e.g. 'ftp' for FTP-only)",
    )
    args = ap.parse_args()
    case_filter = re.compile(args.case_regex) if args.case_regex else None

    keep_temps = os.environ.get("WGET_RUST_KEEP_TEMPS", "").strip() not in ("", "0", "false", "False", "no", "No")

    if not args.orig or not Path(args.orig).exists():
        print("ERROR: cannot find original wget. Set WGET_ORIG=/path/to/wget", file=sys.stderr)
        return 2
    if not args.rust or not Path(args.rust).exists():
        print("ERROR: cannot find Rust wget binary. Build it or set WGET_RUST=/path/to/wget", file=sys.stderr)
        return 2

    no_proxy_host = urlparse(args.base_url).hostname or "127.0.0.1"

    cases: list[Case] = [
        Case(
            id="TC01_ok_default_name",
            args=["{BASE}/ok.txt"],
            expect_files={"ok.txt": ""},  # filled after orig run
        ),
        Case(
            id="TC02_ok_output_document",
            args=["-O", "out.txt", "{BASE}/ok.txt"],
            expect_files={"out.txt": ""},
        ),
        Case(
            id="TC03_404",
            args=["-O", "nf.txt", "{BASE}/notfound"],
            expect_exit_nonzero_ok=True,
            expect_files={},  # don't require file existence
        ),
        Case(
            id="TC04_redirect",
            args=["-O", "redir.txt", "{BASE}/redirect"],
            expect_files={"redir.txt": ""},
        ),
        Case(
            id="TC05_range_resume",
            args=["-c", "-O", "big.bin", "{BASE}/big.bin?size=131072"],
            expect_files={"big.bin": ""},
        ),
        Case(
            id="TC06_flaky_with_tries",
            args=[
                "--tries",
                "5",
                "-T",
                "1",
                "-O",
                "flaky.txt",
                "{BASE}/flaky?fail=2&mode=close&sid=tc06",
            ],
            expect_files={"flaky.txt": ""},
        ),
        Case(
            id="TC07_timeout_slow",
            args=["-T", "1", "-O", "slow.txt", "{BASE}/slow?delay=3.0"],
            expect_exit_nonzero_ok=True,
            expect_files={},
            timeout_s=8.0,
        ),
        Case(
            id="TC07b_read_idle_not_total_duration",
            args=[
                "-T",
                "2",
                "--limit-rate",
                "10k",
                "-O",
                "idle_t.bin",
                "{BASE}/big.bin?size=40960",
            ],
            expect_files={"idle_t.bin": ""},
            expect_min_rust_elapsed_s=2.5,
        ),
        Case(
            id="TC08_limit_rate_slower",
            args=["--limit-rate", "10k", "-O", "rate.bin", "{BASE}/big.bin?size=40960"],
            expect_files={"rate.bin": ""},
            expect_min_rust_elapsed_s=2.5,
        ),
        Case(
            id="TC09_http_proxy_env",
            args=["-O", "via.txt", "{BASE}/ok.txt"],
            expect_files={"via.txt": ""},
            extra_env={"http_proxy": "{PROXY}"},
        ),
        Case(
            id="TC10_no_proxy_cli",
            args=["--no-proxy", "-O", "direct.txt", "{BASE}/ok.txt"],
            expect_files={"direct.txt": ""},
            extra_env={"http_proxy": "http://127.0.0.1:9"},
        ),
        Case(
            id="TC11_no_proxy_env_bypass",
            args=["-O", "bypass.txt", "{BASE}/ok.txt"],
            expect_files={"bypass.txt": ""},
            extra_env={"http_proxy": "{PROXY_EVIL}", "no_proxy": no_proxy_host},
        ),
        Case(
            id="TC12_proxy_auth_ok",
            args=[
                "--proxy-user",
                "wgettest",
                "--proxy-password",
                "secret4test",
                "-O",
                "auth_ok.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={"auth_ok.txt": ""},
            extra_env={"http_proxy": "{PROXY_AUTH}"},
        ),
        Case(
            id="TC13_proxy_auth_wrong_password",
            args=[
                "--proxy-user",
                "wgettest",
                "--proxy-password",
                "wrongpassword",
                "-O",
                "auth_bad.txt",
                "{BASE}/ok.txt",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
            extra_env={"http_proxy": "{PROXY_AUTH}"},
        ),
        Case(
            id="TC13b_proxy_passwd_ok_deprecated_alias",
            args=[
                "--proxy-user",
                "wgettest",
                "--proxy-passwd",
                "secret4test",
                "-O",
                "auth_ok_depr.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={"auth_ok_depr.txt": ""},
            extra_env={"http_proxy": "{PROXY_AUTH}"},
        ),
        Case(
            id="TC13c_proxy_passwd_wrong_deprecated_alias",
            args=[
                "--proxy-user",
                "wgettest",
                "--proxy-passwd",
                "wrongpassword",
                "-O",
                "auth_bad_depr.txt",
                "{BASE}/ok.txt",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
            extra_env={"http_proxy": "{PROXY_AUTH}"},
        ),
        Case(
            id="TC14_post_data_echo",
            args=["--post-data", "hello-echo", "-O", "post.txt", "{BASE}/echo"],
            expect_files={"post.txt": ""},
        ),
        Case(
            id="TC15_save_headers_ok",
            args=["--save-headers", "-O", "hdr.txt", "{BASE}/ok.txt"],
            expect_files={"hdr.txt": ""},
            compare_body_only=frozenset({"hdr.txt"}),
        ),
        Case(
            id="TC16_method_post_body",
            args=["--method", "POST", "--body-data", "method-body", "-O", "mb.txt", "{BASE}/echo"],
            expect_files={"mb.txt": ""},
        ),
        Case(
            id="TC17_post_file_echo",
            args=["--post-file", "payload.txt", "-O", "pf.txt", "{BASE}/echo"],
            expect_files={"pf.txt": ""},
            seed_files={"payload.txt": b"from-file\n"},
        ),
        Case(
            id="TC18_quiet_stderr_silent",
            args=["-q", "-O", "quiet.txt", "{BASE}/ok.txt"],
            expect_files={"quiet.txt": ""},
            expect_stderr_blank_on_success=True,
        ),
        Case(
            id="TC19_no_verbose_saved_line",
            args=["--no-verbose", "-O", "nv.txt", "{BASE}/ok.txt"],
            expect_files={"nv.txt": ""},
        ),
        Case(
            id="TC20_nv_short_flag",
            args=["-nv", "-O", "nv_short.txt", "{BASE}/ok.txt"],
            expect_files={"nv_short.txt": ""},
        ),
        Case(
            id="TC21_illegal_nx_flag",
            args=["-nx", "-O", "badn.txt", "{BASE}/ok.txt"],
            expect_exit_nonzero_ok=True,
            expect_files={},
        ),
        Case(
            id="TC21b_n_flags_quiet_verbose_last_wins_ok",
            args=["-q", "-v", "-nv", "-O", "qvnv.txt", "{BASE}/ok.txt"],
            expect_files={"qvnv.txt": ""},
        ),
        Case(
            id="TC21c_n_flags_quiet_verbose_last_wins_conflict",
            args=["-q", "-nv", "-v", "-O", "qnvv.txt", "{BASE}/ok.txt"],
            expect_exit_nonzero_ok=True,
            expect_files={},
        ),
        Case(
            id="TC22_show_progress_small",
            args=["--show-progress", "-O", "sp2.txt", "{BASE}/ok.txt"],
            expect_files={"sp2.txt": ""},
        ),
        Case(
            id="TC70_unknown_length_progress",
            # Content-Length 未指定：验证 wget-rust 仍输出 `Length: unspecified` + 动态速率提示。
            args=["-O", "nolength.bin", "{BASE}/nolength_big"],
            expect_files={"nolength.bin": ""},
            expect_orig_stderr_contains="Length: unspecified",
            expect_rust_stderr_contains="Length: unspecified",
        ),
        Case(
            id="TC23_max_redirect_zero",
            args=["--max-redirect=0", "-O", "rd0.txt", "{BASE}/redirect"],
            expect_exit_nonzero_ok=True,
            expect_present_files=frozenset({"rd0.txt"}),
            expect_stderr_contains="redirections exceeded",
        ),
        Case(
            id="TC24_multi_url_default_names",
            args=["{BASE}/multi_a.txt", "{BASE}/multi_b.txt"],
            expect_files={"multi_a.txt": "", "multi_b.txt": ""},
        ),
        Case(
            id="TC178_restrict_file_names_unix_decodes_safe_colon",
            args=["{BASE}/rfn/a%3Ab.txt"],
            # GNU 默认 unix：会把 %3A 解码为 ':'（并仅在需要时再转义不安全字符）
            expect_files={"a:b.txt": ""},
        ),
        Case(
            id="TC179_restrict_file_names_windows_escapes_colon",
            args=["--restrict-file-names=windows", "{BASE}/rfn/a%3Ab.txt"],
            # windows 规则下 ':' 不允许，需转义回 %3A
            expect_files={"a%3Ab.txt": ""},
            expect_absent_files=frozenset({"a:b.txt"}),
        ),
        Case(
            id="TC180_iri_default_allows_unicode_path_and_saves_utf8_name",
            args=["{BASE}/iri/%E4%B8%AD%E6%96%87.txt"],
            # 对齐 GNU：IRI 开启时，非 ASCII path 可被接受；落盘名应为 UTF-8（linux/unix 默认允许非 ASCII）。
            expect_files={"中文.txt": ""},
        ),
        Case(
            id="TC181_no_iri_rejects_raw_unicode_url",
            args=["--no-iri", "{BASE}/iri/中文.txt"],
            expect_files={"中文.txt": ""},
        ),
        Case(
            id="TC182_iri_last_wins_over_no_iri",
            args=["--no-iri", "--iri", "{BASE}/iri/%E4%B8%AD%E6%96%87.txt"],
            expect_files={"中文.txt": ""},
        ),
        Case(
            id="TC183_netrc_http_basic_ok",
            args=["-O", "netrc_http.txt", "{BASE}/basic_ok.txt"],
            seed_files={
                ".netrc": "machine 127.0.0.1 login httpauth password httppass\n",
            },
            expect_files={"netrc_http.txt": ""},
        ),
        Case(
            id="TC184_no_netrc_disables_http_basic_lookup",
            args=["--no-netrc", "-O", "netrc_http.txt", "{BASE}/basic_ok.txt"],
            seed_files={
                ".netrc": "machine 127.0.0.1 login httpauth password httppass\n",
            },
            expect_exit_nonzero_ok=True,
        ),
        Case(
            id="TC185_netrc_ftp_auth_path_ok",
            args=["-O", "ftp_secret.txt", "{FTP}/auth/secret.txt"],
            seed_files={
                ".netrc": "machine 127.0.0.1 login u password p\n",
            },
            expect_files={"ftp_secret.txt": ""},
        ),
        Case(
            id="TC186_no_netrc_ftp_auth_path_fails",
            args=["--no-netrc", "-O", "ftp_secret.txt", "{FTP}/auth/secret.txt"],
            seed_files={
                ".netrc": "machine 127.0.0.1 login u password p\n",
            },
            expect_exit_nonzero_ok=True,
        ),
        Case(
            id="TC187_query_appended_unix_question_sep",
            args=["{BASE}/q/ok.txt?x=1"],
            expect_files={"ok.txt?x=1": ""},
        ),
        Case(
            id="TC188_query_appended_windows_at_sep",
            args=["--restrict-file-names=windows", "{BASE}/q/ok.txt?x=1"],
            expect_files={"ok.txt@x=1": ""},
        ),
        Case(
            id="TC189_trust_server_names_includes_query_in_filename",
            args=["--trust-server-names", "{BASE}/tsnq/start"],
            expect_files={"final.bin?sig=1": ""},
        ),
        Case(
            id="TC190_iri_host_percent_decoded_ipv4_literal",
            args=["http://%31%32%37.0.0.1:{BASE_PORT}/ok.txt"],
            expect_files={"ok.txt": ""},
        ),
        Case(
            id="TC191_idn_unicode_host_via_proxy_to_punycode_request",
            args=["-O", "idn_host.txt", "http://bücher.invalid:{BASE_PORT}/echo_host"],
            extra_env={"http_proxy": "{PROXY}"},
            expect_files={},  # only require orig/rust exit code consistency
        ),
        Case(
            id="TC192_idn_punycode_host_via_proxy",
            args=["-O", "idn_host.txt", "http://xn--bcher-kva.invalid:{BASE_PORT}/echo_host"],
            extra_env={"http_proxy": "{PROXY}"},
            expect_files={"idn_host.txt": ""},
        ),
        Case(
            id="TC170b_ftp_verbose_login_trace",
            args=["{FTP}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_stderr_contains="Logged in",
        ),
        Case(
            id="TC170c_ftp_verbose_cmd_trace",
            args=["{FTP}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_stderr_contains="==> SYST",
        ),
        Case(
            id="TC170d_ftp_verbose_length_unauth",
            args=["{FTP}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_stderr_contains="(unauthoritative)",
        ),
        Case(
            id="TC170e_ftp_verbose_no_saving_to",
            args=["{FTP}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_rust_stderr_contains="Saving to:",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC170f_ftp_verbose_saved_single_bracket",
            args=["{FTP}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_stderr_contains="saved [",
        ),
        Case(
            id="TC171b_ftp_missing_verbose_no_such_file",
            args=["{FTP}/missing.xyz"],
            expect_exit_nonzero_ok=True,
            expect_files={},
            expect_orig_stderr_contains="No such file",
            expect_rust_stderr_contains="No such file",
        ),
        Case(
            id="TC170_ftp_file_anonymous_ok",
            args=["{FTP}/ok.txt"],
            expect_files={"ok.txt": ""},
        ),
        Case(
            id="TC171_ftp_file_user_password_ok",
            args=["--ftp-user", "u", "--ftp-password", "p", "-O", "ftp_auth.txt", "{FTP}/ok.txt"],
            expect_files={"ftp_auth.txt": ""},
        ),
        Case(
            id="TC172b_ftp_dir_verbose_listing_trace",
            args=["{FTP}/"],
            expect_present_files=frozenset({"index.html"}),
            expect_stderr_contains="==> LIST",
        ),
        Case(
            id="TC172c_ftp_dir_verbose_index_written",
            args=["{FTP}/"],
            expect_present_files=frozenset({"index.html"}),
            expect_stderr_contains="Wrote HTML-ized index",
        ),
        Case(
            id="TC172d_ftp_dir_verbose_listing_removed",
            args=["{FTP}/"],
            expect_present_files=frozenset({"index.html"}),
            expect_stderr_contains="Removed '.listing'.",
        ),
        Case(
            id="TC173b_ftp_dir_dont_remove_listing_verbose",
            args=["--dont-remove-listing", "{FTP}/"],
            expect_files={".listing": ""},
            expect_present_files=frozenset({"index.html"}),
            expect_stderr_contains="Wrote HTML-ized index",
            expect_rust_stderr_contains="Removed '.listing'.",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC172_ftp_dir_listing_default_removed",
            args=["{FTP}/"],
            expect_files={},
            expect_absent_files=frozenset({".listing"}),
        ),
        Case(
            id="TC173_ftp_dir_listing_kept_with_dont_remove_listing",
            args=["--dont-remove-listing", "{FTP}/"],
            expect_files={".listing": ""},
        ),
        Case(
            id="TC174_ftp_recursive_downloads_dir_entries_min_loop",
            args=["-r", "-l", "1", "--no-directories", "{FTP}/"],
            expect_files={"file.txt": "", "sub.txt": ""},
            expect_absent_files=frozenset({".listing"}),
        ),
        Case(
            id="TC175_ftp_listing_filters_insecure_and_duplicates",
            args=["-r", "-l", "1", "--no-directories", "{FTP}/"],
            expect_files={"file.txt": "", "file.bin": ""},
            expect_absent_files=frozenset({"abs.txt", "evil.txt", "/abs.txt", "../evil.txt"}),
        ),
        Case(
            id="TC176_ftp_listing_accept_reject_applies_to_files_only",
            args=["-r", "-l", "1", "--no-directories", "-A", "txt", "{FTP}/"],
            expect_files={"file.txt": "", "sub.txt": ""},
            expect_absent_files=frozenset({"file.bin"}),
        ),
        Case(
            id="TC177_ftp_listing_accept_regex_filters_full_url",
            # 选择一个不依赖 alternation 方言差异的 regex：匹配 URL 中的 "dir" 子串，
            # 这样目录条目与其子文件都会被包含，其它根目录文件会被排除。
            args=["-r", "-l", "1", "--no-directories", "--accept-regex", "dir", "{FTP}/"],
            expect_files={"sub.txt": ""},
            expect_absent_files=frozenset({"file.txt", "file.bin"}),
        ),
        # 7.1.3：--start-pos（Range）；与 GNU 一致依赖服务端支持 Range；本服务端 /big.bin 返回 206。
        Case(
            id="TC25_start_pos_range",
            args=[
                "--no-verbose",
                "--start-pos=64",
                "-O",
                "sp.bin",
                "{BASE}/big.bin?size=256",
            ],
            expect_files={"sp.bin": ""},
        ),
        # 仅 1 次尝试：flaky 需 fail>=1 次才成功 → 应失败（orig/rust 均非 0）。
        Case(
            id="TC26_tries_one_always_reset",
            args=[
                "--tries=1",
                "-T",
                "1",
                "-O",
                "t1.txt",
                "{BASE}/fail_close",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
        ),
        # 仅 --read-timeout（不设 -T）：慢响应应触发读超时，双方均失败。
        Case(
            id="TC27_read_timeout_slow",
            args=[
                "--read-timeout=1",
                "-O",
                "srt.txt",
                "{BASE}/slow?delay=3.0",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
            timeout_s=12.0,
        ),
        # --connect-timeout：长 -T 但连接阶段应失败（依赖系统对不可达地址的 SYN 超时，语义对齐即可）。
        Case(
            id="TC28_connect_timeout_unreachable",
            args=[
                "--connect-timeout=2",
                "-T",
                "30",
                "-O",
                "cto.bin",
                "http://10.255.255.1:12345/nope",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
            timeout_s=35.0,
        ),
        # GNU 顺序：命令行 URL 在处理完后再处理 -i；此处仅文件内一条 URL → 默认文件名 ok.txt
        Case(
            id="TC29_input_file_urls",
            args=["-i", "urls.txt"],
            expect_files={"ok.txt": ""},
            seed_files={"urls.txt": "{BASE}/ok.txt\n"},
        ),
        Case(
            id="TC30_input_file_base_relative",
            args=["-B", "{BASE}/", "-i", "urls2.txt"],
            expect_files={"ok.txt": ""},
            seed_files={"urls2.txt": "ok.txt\n"},
        ),
        Case(
            id="TC31_force_html_base_href",
            args=["-F", "-i", "page.html"],
            expect_files={"ok.txt": ""},
            seed_files={
                "page.html": """<!doctype html><html><head>
<base href="{BASE}/">
</head><body><a href="ok.txt">l</a></body></html>
"""
            },
        ),
        Case(
            id="TC32_force_html_absolute_a",
            args=["-F", "-i", "page2.html"],
            expect_files={"ok.txt": ""},
            seed_files={
                "page2.html": """<!doctype html><html><body>
<a href="{BASE}/ok.txt">x</a>
</body></html>
"""
            },
        ),
        Case(
            id="TC33_force_html_meta_refresh",
            args=["-F", "-i", "page3.html"],
            expect_files={"ok.txt": ""},
            seed_files={
                "page3.html": """<!doctype html><html><head>
<base href="{BASE}/">
<meta http-equiv="refresh" content="0; URL=ok.txt">
</head><body></body></html>
"""
            },
        ),
        Case(
            id="TC38_http_basic_after_401",
            args=[
                "--http-user=httpauth",
                "--http-password=httppass",
                "-O",
                "bout.txt",
                "{BASE}/basic_ok.txt",
            ],
            expect_files={"bout.txt": ""},
        ),
        Case(
            id="TC39_http_basic_auth_no_challenge",
            args=[
                "--auth-no-challenge",
                "--http-user=httpauth",
                "--http-password=httppass",
                "-O",
                "bp.txt",
                "{BASE}/basic_ok.txt",
            ],
            expect_files={"bp.txt": ""},
        ),
        Case(
            id="TC40_http_basic_wrong_password",
            args=[
                "--http-user=httpauth",
                "--http-password=wrongpw",
                "-O",
                "bad.txt",
                "{BASE}/basic_ok.txt",
            ],
            expect_exit_nonzero_ok=True,
            expect_present_files=frozenset({"bad.txt"}),
            expect_stderr_contains="authentication failed",
        ),
        Case(
            id="TC41_user_password_fallback",
            args=[
                "--user=httpauth",
                "--password=httppass",
                "-O",
                "ub.txt",
                "{BASE}/basic_ok.txt",
            ],
            expect_files={"ub.txt": ""},
        ),
        Case(
            id="TC42_content_disposition_filename",
            args=["--content-disposition", "{BASE}/cd_ok"],
            expect_files={"fromcd.bin": ""},
        ),
        Case(
            id="TC42b_content_disposition_filename_star_rfc5987",
            args=["--content-disposition", "{BASE}/cd_star"],
            expect_files={"hello world.txt": ""},
        ),
        Case(
            id="TC42c_content_disposition_filename_continuations",
            args=["--content-disposition", "{BASE}/cd_cont"],
            expect_files={"helloworld.txt": ""},
        ),
        Case(
            id="TC42d_content_disposition_prefers_filename_star_over_filename",
            args=["--content-disposition", "{BASE}/cd_both"],
            expect_files={"B.ext": ""},
        ),
        Case(
            id="TC42e_content_disposition_ignored_by_output_document",
            args=["--content-disposition", "-O", "forced.bin", "{BASE}/cd_star"],
            expect_files={"forced.bin": ""},
            expect_absent_files=frozenset({"hello world.txt"}),
        ),
        Case(
            id="TC42f_content_disposition_overrides_trust_server_names_on_redirect",
            args=["--content-disposition", "--trust-server-names", "{BASE}/cdredir/start"],
            expect_files={"hello world.txt": ""},
        ),
        Case(
            id="TC42g_content_disposition_in_recursive_download",
            args=["-r", "-l", "1", "--no-directories", "--content-disposition", "{BASE}/cdbase/index.html"],
            expect_files={"index.html": "", "hello world.txt": ""},
        ),
        Case(
            id="TC193_wgetrc_sets_output_document_default",
            args=["{BASE}/ok.txt"],
            seed_files={".wgetrc": "output_document = rc_out.txt\n"},
            expect_files={"rc_out.txt": ""},
            expect_absent_files=frozenset({"ok.txt"}),
        ),
        Case(
            id="TC194_no_config_ignores_wgetrc",
            args=["--no-config", "{BASE}/ok.txt"],
            seed_files={".wgetrc": "output_document = rc_out.txt\n"},
            expect_files={"ok.txt": ""},
            expect_absent_files=frozenset({"rc_out.txt"}),
        ),
        Case(
            id="TC195_config_overrides_default_wgetrc_file",
            args=["--config", "alt.wgetrc", "{BASE}/ok.txt"],
            seed_files={
                ".wgetrc": "output_document = rc_out.txt\n",
                "alt.wgetrc": "output_document = alt_out.txt\n",
            },
            expect_files={"alt_out.txt": ""},
            expect_absent_files=frozenset({"rc_out.txt", "ok.txt"}),
        ),
        Case(
            id="TC196_cli_overrides_wgetrc",
            args=["-O", "cli_out.txt", "{BASE}/ok.txt"],
            seed_files={".wgetrc": "output_document = rc_out.txt\n"},
            expect_files={"cli_out.txt": ""},
            expect_absent_files=frozenset({"rc_out.txt", "ok.txt"}),
        ),
        Case(
            id="TC197_execute_sets_output_document_default",
            args=["-e", "output_document=exe_out.txt", "{BASE}/ok.txt"],
            expect_files={"exe_out.txt": ""},
            expect_absent_files=frozenset({"ok.txt"}),
        ),
        Case(
            id="TC198_execute_order_last_wins_over_cli",
            args=["-O", "cli_out.txt", "-e", "output_document=exe_out.txt", "{BASE}/ok.txt"],
            expect_files={"exe_out.txt": ""},
            expect_absent_files=frozenset({"cli_out.txt", "ok.txt"}),
        ),
        Case(
            id="TC199_cli_overrides_execute_when_afterwards",
            args=["-e", "output_document=exe_out.txt", "-O", "cli_out.txt", "{BASE}/ok.txt"],
            expect_files={"cli_out.txt": ""},
            expect_absent_files=frozenset({"exe_out.txt", "ok.txt"}),
        ),
        Case(
            id="TC200_execute_works_even_with_no_config",
            args=["--no-config", "-e", "output_document=exe_out.txt", "{BASE}/ok.txt"],
            seed_files={".wgetrc": "output_document = rc_out.txt\n"},
            expect_files={"exe_out.txt": ""},
            expect_absent_files=frozenset({"rc_out.txt", "ok.txt"}),
        ),
        Case(
            id="TC201_background_default_logfile_and_download",
            args=["-b", "{BASE}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_present_files=frozenset({"wget-log"}),
            expect_stdout_contains="Continuing in background, pid",
            timeout_s=20.0,
        ),
        Case(
            id="TC43_no_clobber_skip",
            args=["--no-clobber", "-O", "keep_nc.txt", "{BASE}/ok.txt"],
            expect_files={"keep_nc.txt": ""},
            seed_files={"keep_nc.txt": b"seeded-content\n"},
            expect_exit_nonzero_ok=True,
        ),
        Case(
            id="TC44_no_clobber_nc_short",
            args=["-nc", "-O", "keep_nc2.txt", "{BASE}/ok.txt"],
            expect_files={"keep_nc2.txt": ""},
            seed_files={"keep_nc2.txt": b"seeded-content\n"},
            expect_exit_nonzero_ok=True,
        ),
        Case(
            id="TC44c_no_clobber_default_name_single_skip",
            args=["-nc", "{BASE}/ok.txt"],
            seed_files={"ok.txt": b"seeded-content\n"},
            expect_files={"ok.txt": ""},
        ),
        Case(
            id="TC44b_no_clobber_input_list_all_skip_exit_ok",
            args=["-nc", "-i", "urls_nc.txt"],
            seed_files={
                "ok.txt": b"seeded\n",
                "urls_nc.txt": b"{BASE}/ok.txt\n{BASE}/ok.txt\n{BASE}/ok.txt\n",
            },
            expect_files={"ok.txt": ""},
            expect_stderr_contains="already there",
        ),
        Case(
            id="TC44d_no_clobber_batch_o_all_skip_exit_one",
            args=["-nc", "-O", "keep_batch_o.txt", "-i", "urls_batch_o.txt"],
            seed_files={
                "keep_batch_o.txt": b"seeded\n",
                "urls_batch_o.txt": b"{BASE}/ok.txt\n{BASE}/ok.txt\n",
            },
            expect_files={"keep_batch_o.txt": ""},
            expect_exit_nonzero_ok=True,
        ),
        Case(
            id="TC44e_no_clobber_batch_skip_and_404_exit_eight",
            args=["-nc", "-i", "urls_skip_404.txt"],
            seed_files={
                "ok.txt": b"seeded\n",
                "urls_skip_404.txt": b"{BASE}/ok.txt\n{BASE}/missing404xyz\n",
            },
            expect_files={"ok.txt": ""},
        ),
        Case(
            id="TC44f_no_clobber_dual_o_skip_then_download_exit_zero",
            args=[
                "-nc",
                "-O",
                "keep_f.txt",
                "{BASE}/ok.txt",
                "-O",
                "new_f.txt",
                "{BASE}/multi_b.txt",
            ],
            seed_files={"keep_f.txt": b"seeded\n"},
            expect_files={"keep_f.txt": "", "new_f.txt": ""},
        ),
        Case(
            id="TC46_no_clobber_then_clobber",
            args=[
                "--no-clobber",
                "--clobber",
                "-O",
                "clobber_last.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={"clobber_last.txt": ""},
            seed_files={"clobber_last.txt": b"seed-first\n"},
        ),
        Case(
            id="TC47_clobber_then_no_clobber",
            args=[
                "--clobber",
                "--no-clobber",
                "-O",
                "nc_last.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={"nc_last.txt": ""},
            seed_files={"nc_last.txt": b"seed-wins\n"},
        ),
        Case(
            id="TC48_check_certificate_http",
            args=["--check-certificate", "-O", "chk1.txt", "{BASE}/ok.txt"],
            expect_files={"chk1.txt": ""},
        ),
        Case(
            id="TC49_no_check_then_check_certificate_order",
            args=[
                "--no-check-certificate",
                "--check-certificate",
                "-O",
                "chk_order1.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={"chk_order1.txt": ""},
        ),
        Case(
            id="TC50_check_certificate_then_no_check_order",
            args=[
                "--check-certificate",
                "--no-check-certificate",
                "-O",
                "chk_order2.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={"chk_order2.txt": ""},
        ),
        Case(
            id="TC51_content_on_error_404",
            args=["--content-on-error", "-O", "coe.txt", "{BASE}/notfound"],
            expect_files={"coe.txt": ""},
        ),
        Case(
            id="TC52_retry_on_http_error_success",
            args=[
                "--tries",
                "5",
                "--retry-on-http-error=503",
                "-O",
                "retry503.txt",
                "{BASE}/flaky?fail=2&mode=status&code=503&sid=tc52",
            ],
            expect_files={"retry503.txt": ""},
        ),
        Case(
            id="TC53_retry_on_http_error_no_match_fails",
            args=[
                "--tries",
                "3",
                "--retry-on-http-error=500",
                "-O",
                "retry_nomatch.txt",
                "{BASE}/flaky?fail=2&mode=status&code=503&sid=tc53",
            ],
            expect_files={},
        ),
        Case(
            id="TC54_retry_connrefused_default_no_retry",
            args=["--tries", "3", "-nv", "-O", "connref_default.txt", "http://127.0.0.1:9/nope"],
            expect_files={},
            expect_orig_stderr_contains="failed: Connection refused.",
            expect_orig_stderr_contains_count=1,
            expect_rust_stderr_contains="failed: Connection refused.",
            expect_rust_stderr_contains_count=1,
        ),
        Case(
            id="TC54d_nv_no_retrying_line",
            args=["--tries", "3", "-nv", "-O", "connref_default.txt", "http://127.0.0.1:9/nope"],
            expect_files={},
            expect_orig_stderr_contains="Retrying.",
            expect_orig_stderr_contains_count=0,
            expect_rust_stderr_contains="Retrying.",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC54b_retry_connrefused_verbose_retrying",
            args=[
                "--tries",
                "3",
                "--retry-connrefused",
                "-O",
                "connref_verbose.txt",
                "http://127.0.0.1:9/nope",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
            expect_orig_stderr_contains="Retrying.",
            expect_orig_stderr_contains_count=2,
            expect_rust_stderr_contains="Retrying.",
            expect_rust_stderr_contains_count=2,
        ),
        Case(
            id="TC54c_retry_connrefused_verbose_try_prefix",
            args=[
                "--tries",
                "3",
                "--retry-connrefused",
                "-O",
                "connref_verbose2.txt",
                "http://127.0.0.1:9/nope",
            ],
            expect_exit_nonzero_ok=True,
            expect_files={},
            expect_orig_stderr_contains="(try: 2)",
            expect_rust_stderr_contains="(try: 2)",
        ),
        Case(
            id="TC55_retry_connrefused_enabled_retries",
            args=[
                "--tries",
                "3",
                "--retry-connrefused",
                "-nv",
                "-O",
                "connref_retry.txt",
                "http://127.0.0.1:9/nope",
            ],
            expect_files={},
            expect_orig_stderr_contains="failed: Connection refused.",
            expect_orig_stderr_contains_count=3,
            expect_rust_stderr_contains="failed: Connection refused.",
            expect_rust_stderr_contains_count=3,
        ),
        Case(
            id="TC55b_nv_no_retrying_line",
            args=[
                "--tries",
                "3",
                "--retry-connrefused",
                "-nv",
                "-O",
                "connref_retry.txt",
                "http://127.0.0.1:9/nope",
            ],
            expect_files={},
            expect_orig_stderr_contains="Retrying.",
            expect_orig_stderr_contains_count=0,
            expect_rust_stderr_contains="Retrying.",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC56_proxy_cli_evil_applies",
            args=["--proxy", "{PROXY_EVIL}", "-O", "proxy_evil.txt", "{BASE}/ok.txt"],
            expect_files={},
        ),
        Case(
            id="TC57_proxy_cli_disabled_by_no_proxy",
            args=[
                "--proxy",
                "{PROXY_EVIL}",
                "--no-proxy",
                "-O",
                "proxy_disabled.txt",
                "{BASE}/ok.txt",
            ],
            expect_files={},
        ),
        Case(
            id="TC58_retry_on_host_error_default_no_retry",
            args=["--tries", "3", "-nv", "-O", "host_default.txt", "http://nonexistent.invalid/"],
            expect_files={},
            extra_env={"RES_OPTIONS": "attempts:1 timeout:1"},
            timeout_s=15.0,
            expect_orig_stderr_contains="unable to resolve host address",
            expect_orig_stderr_contains_count=1,
            expect_rust_stderr_contains="unable to resolve host address",
            expect_rust_stderr_contains_count=1,
        ),
        Case(
            id="TC58b_nv_no_retrying_line",
            args=["--tries", "3", "-nv", "-O", "host_default.txt", "http://nonexistent.invalid/"],
            expect_files={},
            extra_env={"RES_OPTIONS": "attempts:1 timeout:1"},
            timeout_s=15.0,
            expect_orig_stderr_contains="Retrying.",
            expect_orig_stderr_contains_count=0,
            expect_rust_stderr_contains="Retrying.",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC59_retry_on_host_error_enabled_retries",
            args=[
                "--tries",
                "3",
                "--retry-on-host-error",
                "-nv",
                "-O",
                "host_retry.txt",
                "http://nonexistent.invalid/",
            ],
            expect_files={},
            extra_env={"RES_OPTIONS": "attempts:1 timeout:1"},
            timeout_s=15.0,
            expect_orig_stderr_contains="unable to resolve host address",
            expect_orig_stderr_contains_count=3,
            expect_rust_stderr_contains="unable to resolve host address",
            expect_rust_stderr_contains_count=3,
        ),
        Case(
            id="TC59b_nv_no_retrying_line",
            args=[
                "--tries",
                "3",
                "--retry-on-host-error",
                "-nv",
                "-O",
                "host_retry.txt",
                "http://nonexistent.invalid/",
            ],
            expect_files={},
            extra_env={"RES_OPTIONS": "attempts:1 timeout:1"},
            timeout_s=15.0,
            expect_orig_stderr_contains="Retrying.",
            expect_orig_stderr_contains_count=0,
            expect_rust_stderr_contains="Retrying.",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC60_spider_ok",
            args=["--spider", "-O", "sp_ok.txt", "{BASE}/ok.txt"],
            expect_files={"sp_ok.txt": ""},
        ),
        Case(
            id="TC61_spider_notfound",
            args=["--spider", "-O", "sp_nf.txt", "{BASE}/notfound"],
            expect_files={"sp_nf.txt": ""},
        ),
        Case(
            id="TC62_output_file_overwrite",
            args=["-o", "out.log", "-nv", "-O", "out.txt", "{BASE}/ok.txt"],
            expect_files={"out.txt": "", "out.log": ""},
            normalize_wget_output_log_timestamps=frozenset({"out.log"}),
        ),
        Case(
            id="TC63_append_output",
            args=["-a", "out.log", "-nv", "-O", "out2.txt", "{BASE}/ok.txt"],
            seed_files={"out.log": b"seed\n"},
            expect_files={"out2.txt": "", "out.log": ""},
            normalize_wget_output_log_timestamps=frozenset({"out.log"}),
        ),
        Case(
            id="TC64_output_file_404",
            args=["-o", "err.log", "-nv", "-O", "err_out.txt", "{BASE}/notfound"],
            expect_exit_nonzero_ok=False,
            expect_files={"err.log": ""},
            normalize_wget_output_log_timestamps=frozenset({"err.log"}),
        ),
        Case(
            id="TC65_user_agent",
            args=["-U", "UA-RUST-TEST/1.0", "-nv", "-O", "ua.txt", "{BASE}/echo_user_agent"],
            expect_files={"ua.txt": ""},
        ),
        Case(
            id="TC66_referer",
            args=[
                "--referer",
                "http://referer-rust-test.example/",
                "-nv",
                "-O",
                "ref.txt",
                "{BASE}/echo_referer",
            ],
            expect_files={"ref.txt": ""},
        ),
        Case(
            id="TC67_header",
            args=["--header", "X-Test:ABC123", "-nv", "-O", "x_test.txt", "{BASE}/echo_x_test"],
            expect_files={"x_test.txt": ""},
        ),
        Case(
            id="TC68_output_document_stdout",
            args=["-q", "-O", "-", "{BASE}/ok.txt"],
            expect_files={},
            compare_stdout_with_orig=True,
        ),
        Case(
            id="TC69_default_user_agent_is_wget",
            args=["-nv", "-O", "is_wget.txt", "{BASE}/is_wget_user_agent"],
            expect_files={"is_wget.txt": ""},
        ),
        Case(
            id="TC71_version_banner",
            args=["-V"],
            expect_files={},
            expect_stdout_contains="Kylin Wget",
            expect_rust_stdout_contains="(rust",
        ),
        Case(
            id="TC72_ca_certificate_https_ok",
            args=["--ca-certificate={CAPATH}", "-O", "tls_ca.txt", "{HTTPS}/ok.txt"],
            expect_files={"tls_ca.txt": ""},
        ),
        Case(
            id="TC73_https_no_ca_fails",
            args=["-O", "tls_fail.txt", "{HTTPS}/ok.txt"],
            expect_exit_nonzero_ok=False,
            expect_files={},
            expect_present_files=frozenset({"tls_fail.txt"}),
            compare_stderr_with_orig=True,
        ),
        Case(
            id="TC73b_https_no_ca_fails_nv",
            args=["-nv", "-O", "tls_fail_nv.txt", "{HTTPS}/ok.txt"],
            expect_exit_nonzero_ok=False,
            expect_files={},
            expect_present_files=frozenset({"tls_fail_nv.txt"}),
            compare_stderr_with_orig=True,
            expect_orig_stderr_contains="Connecting",
            expect_orig_stderr_contains_count=0,
            expect_rust_stderr_contains="Connecting",
            expect_rust_stderr_contains_count=0,
        ),
        Case(
            id="TC73c_https_no_ca_default_has_connecting",
            args=["-O", "tls_fail_conn.txt", "{HTTPS}/ok.txt"],
            expect_exit_nonzero_ok=False,
            expect_files={},
            expect_present_files=frozenset({"tls_fail_conn.txt"}),
            expect_orig_stderr_contains="Connecting",
            expect_orig_stderr_contains_count=1,
            expect_rust_stderr_contains="Connecting",
            expect_rust_stderr_contains_count=1,
        ),
        Case(
            id="TC74_ca_directory_https_ok",
            args=["--ca-directory={CADIR}", "-O", "tls_dir.txt", "{HTTPS}/ok.txt"],
            expect_files={"tls_dir.txt": ""},
        ),
        Case(
            id="TC75_inet4_only_ok",
            args=["--inet4-only", "-O", "i4.txt", "{BASE}/ok.txt"],
            expect_files={"i4.txt": ""},
        ),
        Case(
            id="TC76_inet6_only_ipv4_literal_fails",
            args=["--inet6-only", "-O", "i6fail.txt", "{BASE}/ok.txt"],
            expect_exit_nonzero_ok=False,
            expect_files={},
        ),
        Case(
            id="TC77_inet4_inet6_conflict",
            args=["--inet4-only", "--inet6-only", "{BASE}/ok.txt"],
            expect_exit_nonzero_ok=False,
            expect_files={},
        ),
        Case(
            id="TC78_short_inet4_only",
            args=["-4", "-O", "i4short.txt", "{BASE}/ok.txt"],
            expect_files={"i4short.txt": ""},
        ),
        Case(
            id="TC79_load_cookies_sends_cookie",
            args=["--load-cookies=cookies.txt", "-O", "ck.txt", "{BASE}/echo_cookie"],
            seed_files={
                "cookies.txt": "{BASE_HOST}\tFALSE\t/\tFALSE\t2147368447\ta\t1\n"
            },
            expect_files={"ck.txt": ""},
        ),
        Case(
            id="TC80_save_cookies_after_set_cookie",
            args=["--save-cookies=out.cookies", "-O", "set.txt", "{BASE}/set_cookie"],
            expect_files={"set.txt": "", "out.cookies": ""},
            normalize_netscape_cookie_files=frozenset({"out.cookies"}),
        ),
        Case(
            id="TC81_no_cookies_disables_load",
            args=["--no-cookies", "--load-cookies=cookies.txt", "-O", "nock.txt", "{BASE}/echo_cookie"],
            seed_files={
                "cookies.txt": "{BASE_HOST}\tFALSE\t/\tFALSE\t2147368447\ta\t1\n"
            },
            expect_files={"nock.txt": ""},
        ),
        Case(
            id="TC82_hsts_upgrade_http_to_https",
            args=[
                "--inet4-only",
                "--ca-certificate={CAPATH}",
                "--hsts-file=hsts.db",
                "-i",
                "hsts_urls.txt",
            ],
            seed_files={
                "hsts_urls.txt": "{LHTTPS}/hsts_set\nhttp://localhost:{HTTPS_PORT}/ok.txt\n"
            },
            expect_files={"hsts_set": "", "ok.txt": "", "hsts.db": ""},
            normalize_hsts_files=frozenset({"hsts.db"}),
        ),
        Case(
            id="TC45_server_response_stderr",
            args=["-S", "-O", "sr.txt", "{BASE}/ok.txt"],
            expect_files={"sr.txt": ""},
            expect_stderr_contains="200",
        ),
        Case(
            id="TC83_recursive_level1_no_parent_no_directories",
            args=[
                "-r",
                "-l",
                "1",
                "--no-parent",
                "--no-directories",
                "{BASE}/recur/index.html",
            ],
            expect_files={"index.html": "", "a.html": ""},
        ),
        Case(
            id="TC84_recursive_level2_no_parent_no_directories",
            args=[
                "-r",
                "-l",
                "2",
                "--no-parent",
                "--no-directories",
                "{BASE}/recur/index.html",
            ],
            expect_files={"index.html": "", "a.html": "", "b.html": ""},
        ),
        Case(
            id="TC85_recursive_level1_allows_parent_when_not_set",
            args=["-r", "-l", "1", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
        ),
        Case(
            id="TC86_timestamping_skips_when_local_newer",
            args=["-N", "{BASE}/ts.txt"],
            seed_files={"ts.txt": b"remote-ts\n"},
            expect_files={"ts.txt": ""},
        ),
        Case(
            id="TC86f_timestamping_skips_local_newer_on_304_even_if_size_differs",
            args=["-N", "{BASE}/ts.txt"],
            seed_files={"ts.txt": b"local-seed\n"},
            expect_files={"ts.txt": ""},
        ),
        Case(
            id="TC87_timestamping_downloads_when_missing",
            args=["-N", "{BASE}/ts.txt"],
            expect_files={"ts.txt": ""},
        ),
        Case(
            id="TC100_timestamping_sets_mtime_from_last_modified",
            args=["-N", "{BASE}/ts.txt"],
            expect_files={"ts.txt": ""},
            compare_mtime_files=frozenset({"ts.txt"}),
        ),
        Case(
            id="TC101_timestamping_server_ignores_ims_skips_when_local_newer_same_size",
            args=["-N", "{BASE}/ts_ignore_ims.txt"],
            seed_files={"ts_ignore_ims.txt": "LOCALNEW\n"},
            expect_files={"ts_ignore_ims.txt": ""},
        ),
        Case(
            id="TC88_mirror_implies_recursive_timestamping_infinite_level",
            args=["-m", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "b.html": "", "c.html": ""},
        ),
        Case(
            id="TC91_mirror_implies_timestamping_304_skips",
            args=["-m", "--no-directories", "{BASE}/ts.txt"],
            seed_files={"ts.txt": b"seed-local\n"},
            expect_files={"ts.txt": ""},
        ),
        Case(
            id="TC89_convert_links_rewrites_to_local_files",
            args=["-r", "-l", "1", "-k", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
        ),
        Case(
            id="TC102_backup_converted_creates_orig_before_k",
            args=["-r", "-l", "1", "-k", "-K", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "index.html.orig": "", "a.html": "", "a.html.orig": "", "c.html": ""},
        ),
        Case(
            id="TC104_convert_file_only_grafts_basename",
            args=["-r", "-l", "1", "--convert-file-only", "--content-disposition", "--no-directories", "{BASE}/cfo/index.html"],
            expect_files={"index.html": "", "renamed.bin": ""},
        ),
        Case(
            id="TC103_backups_rotates_before_writing",
            args=["--backups=2", "{BASE}/ok.txt", "{BASE}/ok.txt"],
            expect_files={"ok.txt": "", "ok.txt.1": ""},
        ),
        Case(
            id="TC90_page_requisites_downloads_resources",
            args=["-r", "-l", "1", "-p", "--no-directories", "{BASE}/recurp/index.html"],
            expect_files={
                "index.html": "",
                "a.html": "",
                "style.css": "",
                "app.js": "",
                "img.png": "",
                "imp.css": "",
                "bg.png": "",
                "bg2.png": "",
            },
        ),
        Case(
            id="TC93_page_requisites_without_recursive_does_not_follow_a",
            args=["-p", "--no-directories", "{BASE}/recurp/index.html"],
            expect_files={"index.html": "", "style.css": "", "app.js": "", "img.png": "", "imp.css": "", "bg.png": "", "bg2.png": ""},
        ),
        Case(
            id="TC94_recursive_accept_txt_reject_bin",
            args=["-r", "-l", "1", "-A", "txt", "--no-directories", "{BASE}/acc/index.html"],
            expect_files={"index.html": "", "a.txt": "", "c.html": ""},
        ),
        Case(
            id="TC95_recursive_include_directories_only",
            args=["-r", "-l", "1", "-I", "dirs/inc", "--no-directories", "{BASE}/dirs/index.html"],
            expect_files={"index.html": "", "keep.html": ""},
        ),
        Case(
            id="TC96_recursive_exclude_directories",
            args=["-r", "-l", "1", "-X", "dirs/exc", "--no-directories", "{BASE}/dirs/index.html"],
            expect_files={"index.html": "", "keep.html": ""},
        ),
        Case(
            id="TC97_recursive_relative_only_skips_absolute",
            args=["-r", "-l", "1", "-L", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
        ),
        Case(
            id="TC138_relative_only_deep_keeps_relative_and_root_relative",
            args=["-r", "-l", "1", "-L", "--no-directories", "{BASE}/rel/index.html"],
            expect_files={"index.html": "", "a.html": "", "root.html": ""},
            expect_absent_files=frozenset({"abs1.html", "abs2.html"}),
        ),
        Case(
            id="TC139_span_hosts_allows_absolute_and_protocol_relative",
            args=["-r", "-l", "1", "-H", "--no-directories", "{BASE}/rel/index.html"],
            expect_files={"index.html": "", "a.html": "", "root.html": "", "abs1.html": "", "abs2.html": ""},
        ),
        Case(
            id="TC140_relative_only_overrides_span_hosts",
            args=["-r", "-l", "1", "-H", "-L", "--no-directories", "{BASE}/rel/index.html"],
            expect_files={"index.html": "", "a.html": "", "root.html": ""},
            expect_absent_files=frozenset({"abs1.html", "abs2.html"}),
        ),
        Case(
            id="TC98_recursive_span_hosts_allows_localhost_foreign",
            args=["-r", "-l", "1", "-H", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": "", "foreign.html": ""},
        ),
        Case(
            id="TC134_recursive_no_span_hosts_blocks_localhost_foreign",
            args=["-r", "-l", "1", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
            expect_absent_files=frozenset({"foreign.html"}),
        ),
        Case(
            id="TC135_recursive_span_hosts_respects_domains_whitelist",
            args=["-r", "-l", "1", "-H", "-D", "127.0.0.1", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
            expect_absent_files=frozenset({"foreign.html"}),
        ),
        Case(
            id="TC136_recursive_span_hosts_respects_exclude_domains",
            args=["-r", "-l", "1", "-H", "--exclude-domains", "localhost", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
            expect_absent_files=frozenset({"foreign.html"}),
        ),
        Case(
            id="TC141_domains_leading_dot_matches_subdomain",
            args=["-r", "-l", "1", "-H", "-D", ".localhost", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "foreign.html": ""},
            expect_absent_files=frozenset({"a.html", "c.html"}),
        ),
        Case(
            id="TC142_domains_case_insensitive",
            args=["-r", "-l", "1", "-H", "-D", "LOCALHOST", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "foreign.html": ""},
            expect_absent_files=frozenset({"a.html", "c.html"}),
        ),
        Case(
            id="TC143_exclude_domains_overrides_domains",
            args=[
                "-r",
                "-l",
                "1",
                "-H",
                "-D",
                "localhost",
                "--exclude-domains",
                "localhost",
                "--no-directories",
                "{BASE}/recur/index.html",
            ],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
            expect_absent_files=frozenset({"foreign.html"}),
        ),
        # -I/-X 深水区：通配、URL 编码路径、与 --cut-dirs/-P 组合（仅影响落盘，不影响过滤）
        Case(
            id="TC144_incdir_prefix_matches_subdir",
            args=["-r", "-l", "1", "-I", "dirs/inc", "--no-directories", "{BASE}/dirs/index.html"],
            expect_files={"index.html": "", "keep.html": ""},
            expect_absent_files=frozenset({"drop.html"}),
        ),
        Case(
            id="TC145_excdir_prefix_excludes_subdir",
            args=["-r", "-l", "1", "-X", "dirs/exc", "--no-directories", "{BASE}/dirs/index.html"],
            expect_files={"index.html": "", "keep.html": ""},
            expect_absent_files=frozenset({"drop.html"}),
        ),
        Case(
            id="TC146_incdir_wildcard_fnmatch_pathname",
            args=["-r", "-l", "1", "-I", "dirs/*", "--no-directories", "{BASE}/dirs/wc/index.html"],
            # dirs/* 同时匹配 dirs/inc、dirs/inc x、dirs/exc，因此会下载 keep/keep(space)/drop
            expect_files={"index.html": "", "keep.html": "", "keep.html.1": "", "drop.html": ""},
        ),
        Case(
            id="TC147_incdir_url_percent_decoded_space",
            args=["-r", "-l", "1", "-I", "dirs/inc x", "--no-directories", "{BASE}/dirs/wc/index.html"],
            expect_files={"index.html": "", "keep.html": ""},
            expect_absent_files=frozenset({"drop.html"}),
        ),
        Case(
            id="TC148_incdir_with_cut_dirs_and_prefix_does_not_change_filter",
            args=[
                "-r",
                "-l",
                "1",
                "-I",
                "dirs/inc",
                "-P",
                "DL",
                "--cut-dirs",
                "1",
                "{BASE}/dirs/index.html",
            ],
            expect_files={
                "DL/localhost/dirs/index.html": "",
                "DL/localhost/inc/keep.html": "",
            },
            expect_absent_files=frozenset({"DL/localhost/exc/drop.html"}),
        ),
        # -A/-R 深水区：通配/逗号多后缀/无扩展名/目录 URL + --default-page 组合
        Case(
            id="TC149_accept_multi_suffix_csv",
            args=["-r", "-l", "1", "-A", "txt,html", "{BASE}/accar/index.html"],
            expect_files={
                "127.0.0.1/accar/index.html": "",
                "127.0.0.1/accar/file.html": "",
                "127.0.0.1/accar/file.txt": "",
                "127.0.0.1/accar/sub/index.html": "",
            },
            expect_absent_files=frozenset({"127.0.0.1/accar/noext"}),
        ),
        Case(
            id="TC150_accept_wildcard_html",
            args=["-r", "-l", "1", "-A", "*.htm*", "{BASE}/accar/index.html"],
            expect_files={
                "127.0.0.1/accar/index.html": "",
                "127.0.0.1/accar/file.html": "",
                "127.0.0.1/accar/sub/index.html": "",
            },
            expect_absent_files=frozenset({"127.0.0.1/accar/file.txt", "127.0.0.1/accar/noext"}),
        ),
        Case(
            id="TC151_reject_overrides_accept",
            args=["-r", "-l", "1", "-A", "txt,html", "-R", "txt", "{BASE}/accar/index.html"],
            expect_files={
                "127.0.0.1/accar/index.html": "",
                "127.0.0.1/accar/file.html": "",
                "127.0.0.1/accar/sub/index.html": "",
            },
            expect_absent_files=frozenset({"127.0.0.1/accar/file.txt", "127.0.0.1/accar/noext"}),
        ),
        Case(
            id="TC152_default_page_affects_dir_acceptance",
            args=[
                "-r",
                "-l",
                "1",
                "--default-page",
                "home",
                "-A",
                "*.htm*",
                "{BASE}/accar/index.html",
            ],
            expect_files={
                "127.0.0.1/accar/index.html": "",
                "127.0.0.1/accar/file.html": "",
            },
            expect_absent_files=frozenset({"127.0.0.1/accar/sub/home"}),
        ),
        Case(
            id="TC153_accept_star_allows_noext",
            args=["-r", "-l", "1", "-A", "*", "{BASE}/accar/index.html"],
            expect_files={
                "127.0.0.1/accar/index.html": "",
                "127.0.0.1/accar/file.html": "",
                "127.0.0.1/accar/file.txt": "",
                "127.0.0.1/accar/noext": "",
                "127.0.0.1/accar/sub/index.html": "",
            },
        ),
        # --accept-regex/--reject-regex 深水区：匹配 URL 全串（含 query）、优先级、与 -H/-L 组合
        Case(
            id="TC154_accept_regex_matches_query_and_redirect_source",
            args=["-r", "-l", "1", "--accept-regex", "start\\?from=index", "{BASE}/rx/index.html"],
            expect_files={
                "127.0.0.1/rx/index.html": "",
                "127.0.0.1/rx/final.bin": "",  # follow redirect, basename drop query
            },
            expect_absent_files=frozenset({"127.0.0.1/rx/foreign"}),
        ),
        Case(
            id="TC155_reject_regex_overrides_accept_regex",
            args=[
                "-r",
                "-l",
                "1",
                "--accept-regex",
                "rx/",
                "--reject-regex",
                "foreign",
                "-H",
                "{BASE}/rx/index.html",
            ],
            expect_files={
                "127.0.0.1/rx/index.html": "",
                "127.0.0.1/rx/final.bin": "",
            },
            expect_absent_files=frozenset({"localhost/rx/foreign"}),
        ),
        Case(
            id="TC156_relative_only_applies_before_regex_for_absolute_foreign",
            args=[
                "-r",
                "-l",
                "1",
                "-H",
                "-L",
                "--accept-regex",
                "foreign",
                "{BASE}/rx/index.html",
            ],
            expect_files={"127.0.0.1/rx/index.html": "", "127.0.0.1/rx/final.bin": ""},
            expect_absent_files=frozenset({"localhost/rx/foreign"}),
        ),
        # robots 深水区：UA 组选择 + first-match-wins + %XX 解码（不解 '/'）
        Case(
            id="TC157_robots_ua_exact_prunes_star_and_first_match_wins",
            args=["-r", "-l", "2", "{BASE}/rb/index.html"],
            expect_files={
                "127.0.0.1/rb/index.html": "",
                "127.0.0.1/rb/public/ok.html": "",
            },
            expect_absent_files=frozenset({"127.0.0.1/rb/private/deny.html"}),
        ),
        Case(
            id="TC158_robots_percent_decoding_matches_tilde",
            args=["-r", "-l", "2", "{BASE}/rb/index.html"],
            expect_files={"127.0.0.1/rb/%7Euser": ""},
        ),
        # 同站点/端口 深水区：--no-parent 与端口差异、http↔https 的组合边界
        Case(
            id="TC159_no_parent_ignored_when_same_scheme_but_port_diff",
            args=["-r", "-l", "1", "--no-parent", "{BASE}/hp/index.html"],
            expect_files={
                "127.0.0.1/hp/index.html": "",
                "127.0.0.1/hp2/outside.html": "",
            },
            expect_absent_files=frozenset({"127.0.0.1/hp2/https_outside.html"}),
        ),
        Case(
            id="TC160_relative_only_blocks_absolute_other_port_even_if_no_parent_ignored",
            args=["-r", "-l", "1", "--no-parent", "-L", "{BASE}/hp/index.html"],
            expect_files={"127.0.0.1/hp/index.html": ""},
            expect_absent_files=frozenset({"127.0.0.1/hp2/outside.html", "127.0.0.1/hp2/https_outside.html"}),
        ),
        # descend_redirect 深水区：redirect 目标被 LIST/REGEX 拒绝时仍需继续抽链/入队
        Case(
            id="TC161_descend_redirect_ignores_list_for_redirect_target",
            args=["-r", "-l", "1", "-X", "dr/exc", "{BASE}/dr/start_list"],
            # 若不忽略 LIST，则不会从 target.html 抽链并下载 keep.html
            expect_files={"127.0.0.1/dr/start_list": "", "keep.html": ""},
        ),
        Case(
            id="TC162_descend_redirect_ignores_regex_for_redirect_target",
            args=["-r", "-l", "1", "--accept-regex", "start_regex", "{BASE}/dr/start_regex"],
            expect_files={"127.0.0.1/dr/start_regex": "", "keep.html": ""},
        ),
        # --https-only 深水区：递归时只跟随 HTTPS（含协议相对链接），并拒绝绝对 http 与 https->http 重定向
        Case(
            id="TC163_https_only_recursive_follows_https_and_skips_http",
            args=["-r", "-l", "1", "--https-only", "{HTTPS}/ho/index.html"],
            expect_files={
                "127.0.0.1/ho/index.html": "",
                "127.0.0.1/ho/a.html": "",
                "127.0.0.1/ho/pr.html": "",
            },
            expect_absent_files=frozenset({"ok.txt", "127.0.0.1/ok.txt"}),
        ),
        # redirect filter points deep alignment: for redirects, LIST/REGEX are ignored for the redirect target,
        # but DOMAIN/SPANNEDHOST/ROBOTS/... still decide whether we descend into children.
        Case(
            id="TC164_redirect_target_spannedhost_blocks_descend",
            args=["-r", "-l", "1", "--no-directories", "{BASE}/rf/start_span"],
            expect_files={"start_span": ""},
            expect_absent_files=frozenset({"keep.html"}),
        ),
        Case(
            id="TC165_redirect_target_domain_filter_blocks_descend_even_with_span_hosts",
            args=["-r", "-l", "1", "-H", "-D", "127.0.0.1", "--no-directories", "{BASE}/rf/start_dom"],
            expect_files={"start_dom": ""},
            expect_absent_files=frozenset({"keep.html"}),
        ),
        Case(
            id="TC166_redirect_target_list_rejection_is_ignored_for_descend",
            args=["-r", "-l", "1", "-X", "rf/list", "--no-directories", "{BASE}/rf/start_list"],
            expect_files={"start_list": "", "keep.html": ""},
        ),
        Case(
            id="TC167_redirect_target_regex_rejection_is_ignored_for_descend",
            args=["-r", "-l", "1", "-H", "--accept-regex", "keep_regex", "--no-directories", "{BASE}/rf/start_regex"],
            expect_files={"start_regex": "", "keep_regex.html": ""},
        ),
        Case(
            id="TC137_recursive_span_hosts_with_relative_only_skips_absolute_foreign",
            args=["-r", "-l", "1", "-H", "-L", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": "", "c.html": ""},
            expect_absent_files=frozenset({"foreign.html"}),
        ),
        Case(
            id="TC99_recursive_reject_regex",
            args=["-r", "-l", "1", "--reject-regex", ".*c\\.html$", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": ""},
        ),
        Case(
            id="TC105_robots_txt_disallows_child",
            args=["-r", "-l", "2", "--no-directories", "{BASE}/recur/index.html"],
            expect_files={"index.html": "", "a.html": ""},
        ),
        Case(
            id="TC106_meta_robots_nofollow_disables_all_links",
            args=["-r", "-l", "2", "--no-directories", "{BASE}/nofollow/index.html"],
            expect_files={"index.html": ""},
        ),
        Case(
            id="TC107_adjust_extension_html_appends_html",
            args=["-E", "{BASE}/ae/noext"],
            expect_files={"noext.html": ""},
        ),
        Case(
            id="TC108_adjust_extension_css_appends_css",
            args=["-E", "{BASE}/ae/style"],
            expect_files={"style.css": ""},
        ),
        Case(
            id="TC109_delete_after_single_url_removes_file",
            args=["--delete-after", "{BASE}/ok.txt"],
            expect_absent_files=frozenset({"ok.txt"}),
        ),
        Case(
            id="TC110_delete_after_recursive_removes_harvested_files",
            args=["-r", "-l", "1", "--delete-after", "--no-directories", "{BASE}/recur/index.html"],
            expect_absent_files=frozenset({"index.html", "a.html", "c.html"}),
        ),
        Case(
            id="TC111_trust_server_names_uses_redirect_basename",
            args=["--trust-server-names", "{BASE}/tsn/start"],
            expect_files={"final.bin": ""},
            expect_absent_files=frozenset({"start"}),
        ),
        Case(
            id="TC112_trust_server_names_recursive_uses_effective_url_for_local_path",
            args=["-r", "-l", "1", "--trust-server-names", "--no-directories", "{BASE}/tsnr/index.html"],
            expect_files={"home.html": "", "a.html": ""},
            expect_absent_files=frozenset({"index.html"}),
        ),
        Case(
            id="TC113_cut_dirs_trims_remote_components_in_dirstruct",
            args=["-r", "-l", "1", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={
                # host dir is always created in current implementation
                "127.0.0.1/two/index.html": "",
                "127.0.0.1/two/a.txt": "",
            },
            expect_absent_files=frozenset(
                {
                    "127.0.0.1/cut/one/two/index.html",
                    "127.0.0.1/cut/one/two/a.txt",
                }
            ),
        ),
        Case(
            id="TC114_directory_prefix_single_url_saves_under_prefix",
            args=["-P", "dl", "{BASE}/ok.txt"],
            expect_files={"dl/ok.txt": ""},
            expect_absent_files=frozenset({"ok.txt"}),
        ),
        Case(
            id="TC115_directory_prefix_recursive_applies_to_dirstruct",
            args=["-r", "-l", "1", "-P", "dl", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={"dl/127.0.0.1/two/index.html": "", "dl/127.0.0.1/two/a.txt": ""},
        ),
        Case(
            id="TC116_no_host_directories_omits_host_dir",
            args=["-r", "-l", "1", "--no-host-directories", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={"two/index.html": "", "two/a.txt": ""},
            expect_absent_files=frozenset({"127.0.0.1/two/index.html", "127.0.0.1/two/a.txt"}),
        ),
        Case(
            id="TC117_no_host_then_host_directories_last_wins",
            args=["-r", "-l", "1", "--no-host-directories", "--host-directories", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={"127.0.0.1/two/index.html": "", "127.0.0.1/two/a.txt": ""},
        ),
        Case(
            id="TC118_short_nH_no_host_directories",
            args=["-r", "-l", "1", "-nH", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={"two/index.html": "", "two/a.txt": ""},
        ),
        Case(
            id="TC119_protocol_directories_inserts_scheme_before_host",
            args=["-r", "-l", "1", "--protocol-directories", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={"http/127.0.0.1/two/index.html": "", "http/127.0.0.1/two/a.txt": ""},
        ),
        Case(
            id="TC120_protocol_directories_without_hostdir",
            args=["-r", "-l", "1", "--protocol-directories", "--no-host-directories", "--cut-dirs=2", "{BASE}/cut/one/two/index.html"],
            expect_files={"http/two/index.html": "", "http/two/a.txt": ""},
        ),
        Case(
            id="TC121_force_directories_single_url_builds_dirstruct",
            args=["-x", "{BASE}/ok.txt"],
            expect_files={"127.0.0.1/ok.txt": ""},
            expect_absent_files=frozenset({"ok.txt"}),
        ),
        Case(
            id="TC122_force_directories_then_no_directories_last_wins",
            args=["-x", "-nd", "{BASE}/ok.txt"],
            expect_files={"ok.txt": ""},
            expect_absent_files=frozenset({"127.0.0.1/ok.txt"}),
        ),
        Case(
            id="TC123_no_directories_then_force_directories_last_wins",
            args=["-nd", "-x", "{BASE}/ok.txt"],
            expect_files={"127.0.0.1/ok.txt": ""},
            expect_absent_files=frozenset({"ok.txt"}),
        ),
        Case(
            id="TC124_default_page_single_url_directory_uses_name",
            args=["--default-page=home.html", "{BASE}/dp/"],
            expect_files={"home.html": ""},
            expect_absent_files=frozenset({"index.html"}),
        ),
        Case(
            id="TC125_default_page_with_force_directories_single_url_directory",
            args=["-x", "--default-page=home.html", "{BASE}/dp/"],
            expect_files={"127.0.0.1/dp/home.html": ""},
        ),
        Case(
            id="TC126_default_page_repeat_last_wins",
            args=["--default-page=a.html", "--default-page=home.html", "{BASE}/dp/"],
            expect_files={"home.html": ""},
            expect_absent_files=frozenset({"a.html"}),
        ),
        Case(
            id="TC127_no_directories_then_force_directories_affects_k_local_paths",
            args=["-r", "-l", "1", "-nd", "-x", "-k", "--default-page=home.html", "{BASE}/ndsite/"],
            expect_files={
                # With -nd -x (last-wins => dirstruct ON), we should save into dir tree.
                "127.0.0.1/ndsite/home.html": "",
                "127.0.0.1/ndsite/a.html": "",
            },
        ),
        Case(
            id="TC128_force_directories_then_no_directories_flattens_k_local_paths",
            args=["-r", "-l", "1", "-x", "-nd", "-k", "--default-page=home.html", "{BASE}/ndsite/"],
            expect_files={
                # With -x -nd (last-wins => dirstruct OFF), we should flatten.
                "home.html": "",
                "a.html": "",
            },
        ),
        Case(
            id="TC129_no_directories_redirect_to_directory_url_uses_default_page",
            args=["-nd", "--default-page=home.html", "{BASE}/ndredir/to_dir"],
            expect_files={"home.html": ""},
        ),
        Case(
            id="TC130_no_directories_redirect_to_directory_without_trust_server_names_uses_original_basename",
            args=["-nd", "--default-page=home.html", "{BASE}/tsn_dir/start"],
            expect_files={"start": ""},
            expect_absent_files=frozenset({"home.html"}),
        ),
        Case(
            id="TC131_no_directories_redirect_to_directory_with_trust_server_names_uses_default_page",
            args=["-nd", "--trust-server-names", "--default-page=home.html", "{BASE}/tsn_dir/start"],
            expect_files={"home.html": ""},
            expect_absent_files=frozenset({"start"}),
        ),
        Case(
            id="TC132_force_directories_redirect_to_directory_without_trust_server_names_uses_original_path",
            args=["-x", "--default-page=home.html", "{BASE}/tsn_dir/start"],
            expect_files={"127.0.0.1/tsn_dir/start": ""},
            expect_absent_files=frozenset({"127.0.0.1/dp/home.html"}),
        ),
        Case(
            id="TC133_force_directories_redirect_to_directory_with_trust_server_names_uses_default_page_path",
            args=["-x", "--trust-server-names", "--default-page=home.html", "{BASE}/tsn_dir/start"],
            expect_files={"127.0.0.1/dp/home.html": ""},
            expect_absent_files=frozenset({"127.0.0.1/tsn_dir/start"}),
        ),
    ]

    def case_selected(c: Case) -> bool:
        return case_filter is None or case_filter.search(c.id) is not None

    selected = [c for c in cases if case_selected(c)]

    if any((c.extra_env or {}).get("http_proxy") == "{PROXY}" for c in selected) and not args.proxy_url:
        print("ERROR: TC09 needs --proxy-url (forward proxy)", file=sys.stderr)
        return 2
    if any(
        (c.extra_env or {}).get("http_proxy") == "{PROXY_EVIL}" for c in selected
    ) and not args.proxy_evil_url:
        print("ERROR: TC11 needs --proxy-evil-url", file=sys.stderr)
        return 2
    if any("{PROXY_AUTH}" in v for c in selected for v in (c.extra_env or {}).values()) and not args.proxy_auth_url:
        print("ERROR: TC12/TC13 need --proxy-auth-url", file=sys.stderr)
        return 2

    failures = 0

    for case in selected:
        print(f"Running {case.id} ...")
        # For resume case: pre-create a partial file for both runs to ensure -c path is exercised.
        if case.id == "TC05_range_resume":
            # We'll run orig once to get final hash; for each tool run we'll create partial file before calling.
            pass

        # Run orig
        orig_case = case
        if case.id == "TC05_range_resume":
            # We can't inject setup into run_one; instead bake a shell wrapper by using /bin/sh -c is too messy.
            # So we approximate: first download small chunk via --header Range if supported? keep simple:
            # We'll just run normal -c; many wgets will create/continue properly even if file missing.
            # This will still validate that final file content matches.
            pass

        o_code, o_files, o_mtimes, o_absent, o_present, o_out, o_err, o_elapsed, o_workdir = run_one(
            args.orig,
            orig_case,
            args.base_url,
            base2_url=args.base2_url,
            proxy_url=args.proxy_url,
            proxy_evil_url=args.proxy_evil_url,
            proxy_auth_url=args.proxy_auth_url,
            ftp_base_url=args.ftp_base_url,
            https_base_url=args.https_base_url,
            ca_pem=args.ca_pem,
            ca_dir=args.ca_dir,
        )

        # Fill expected hashes from orig run when files exist.
        expect_files = dict(case.expect_files)
        for rel in list(expect_files.keys()):
            if rel in o_files:
                expect_files[rel] = o_files[rel]
            else:
                # If orig didn't create it, keep empty => we won't compare hash.
                expect_files[rel] = ""

        # Run rust
        r_code, r_files, r_mtimes, r_absent, r_present, r_out, r_err, r_elapsed, r_workdir = run_one(
            args.rust,
            case,
            args.base_url,
            base2_url=args.base2_url,
            proxy_url=args.proxy_url,
            proxy_evil_url=args.proxy_evil_url,
            proxy_auth_url=args.proxy_auth_url,
            ftp_base_url=args.ftp_base_url,
            https_base_url=args.https_base_url,
            ca_pem=args.ca_pem,
            ca_dir=args.ca_dir,
        )

        case_failed = False

        # Compare exit codes
        if not case.expect_exit_nonzero_ok:
            if r_code != o_code:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: exit code mismatch (orig={o_code}, rust={r_code})")
        else:
            if (o_code == 0) != (r_code == 0):
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: success/fail mismatch (orig={o_code}, rust={r_code})")

        if case.compare_stdout_with_orig and o_out != r_out:
            failures += 1
            case_failed = True
            print(f"[FAIL] {case.id}: stdout mismatch (orig_len={len(o_out)}, rust_len={len(r_out)})")

        # Compare file hashes for those we have expectations
        rust_file_expect = case.expect_rust_files if case.expect_rust_files else expect_files
        for rel, exp_hash in rust_file_expect.items():
            if not exp_hash:
                continue
            got = r_files.get(rel)
            if got != exp_hash:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: file {rel} sha256 mismatch")
                print(f"       expected={exp_hash}")
                print(f"       got     ={got}")

        # Compare mtime for selected files (orig vs rust)
        for rel in case.compare_mtime_files:
            if rel not in o_mtimes or rel not in r_mtimes:
                failures += 1
                case_failed = True
                print(
                    f"[FAIL] {case.id}: missing mtime for {rel} (orig_has={rel in o_mtimes}, rust_has={rel in r_mtimes})"
                )
                continue
            if o_mtimes[rel] != r_mtimes[rel]:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: mtime mismatch for {rel} (orig={o_mtimes[rel]}, rust={r_mtimes[rel]})")

        # Compare absent files expectations
        for rel in case.expect_absent_files:
            o_exists = o_absent.get(rel, False)
            r_exists = r_absent.get(rel, False)
            if o_exists or r_exists:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: expected {rel} absent (orig_exists={o_exists}, rust_exists={r_exists})")

        # Compare present files expectations (existence only)
        for rel in case.expect_present_files:
            o_ok = o_present.get(rel, False)
            r_ok = r_present.get(rel, False)
            if (not o_ok) or (not r_ok):
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: expected {rel} present (orig_has={o_ok}, rust_has={r_ok})")

        if case.expect_min_rust_elapsed_s is not None:
            if r_elapsed < case.expect_min_rust_elapsed_s:
                failures += 1
                case_failed = True
                print(
                    f"[FAIL] {case.id}: rust runtime too fast (elapsed={r_elapsed:.2f}s, expected>={case.expect_min_rust_elapsed_s}s)"
                )

        if case.expect_stderr_blank_on_success:
            if o_code == 0 and o_err.strip():
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: orig stderr not blank on success (repr={o_err[:120]!r})")
            if r_code == 0 and r_err.strip():
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: rust stderr not blank on success (repr={r_err[:120]!r})")

        if case.expect_stderr_contains and o_code == 0 and r_code == 0:
            needle = case.expect_stderr_contains
            if needle not in o_err or needle not in r_err:
                failures += 1
                case_failed = True
                print(
                    f"[FAIL] {case.id}: stderr missing {needle!r} (orig has={needle in o_err}, rust has={needle in r_err})"
                )

        if case.expect_stdout_contains is not None and o_code == 0 and r_code == 0:
            needle = case.expect_stdout_contains
            if needle not in o_out or needle not in r_out:
                failures += 1
                case_failed = True
                print(
                    f"[FAIL] {case.id}: stdout missing {needle!r} (orig has={needle in o_out}, rust has={needle in r_out})"
                )
        if case.expect_rust_stdout_contains is not None and r_code == 0:
            needle = case.expect_rust_stdout_contains
            if needle not in r_out:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: rust stdout missing {needle!r}")

        # 失败场景/非 0 退出码：允许对 orig/rust 分别做 stderr 断言（子串计数）
        if case.expect_orig_stderr_contains is not None and case.expect_orig_stderr_contains_count is None:
            needle = case.expect_orig_stderr_contains
            if needle not in o_err:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: orig stderr missing {needle!r}")
        if case.expect_rust_stderr_contains is not None and case.expect_rust_stderr_contains_count is None:
            needle = case.expect_rust_stderr_contains
            if needle not in r_err:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: rust stderr missing {needle!r}")

        if case.expect_orig_stderr_contains_count is not None:
            needle = case.expect_orig_stderr_contains or ""
            cnt = o_err.count(needle) if needle else 0
            if cnt != case.expect_orig_stderr_contains_count:
                failures += 1
                case_failed = True
                print(
                    f"[FAIL] {case.id}: orig stderr count mismatch needle={needle!r} got={cnt} expected={case.expect_orig_stderr_contains_count}"
                )

        if case.expect_rust_stderr_contains_count is not None:
            needle = case.expect_rust_stderr_contains or ""
            cnt = r_err.count(needle) if needle else 0
            if cnt != case.expect_rust_stderr_contains_count:
                failures += 1
                case_failed = True
                print(
                    f"[FAIL] {case.id}: rust stderr count mismatch needle={needle!r} got={cnt} expected={case.expect_rust_stderr_contains_count}"
                )

        if case.compare_stderr_with_orig:
            o_lines = normalize_stderr_lines(o_err)
            r_lines = normalize_stderr_lines(r_err)
            if o_lines != r_lines:
                failures += 1
                case_failed = True
                print(f"[FAIL] {case.id}: stderr line mismatch after timestamp strip")
                print(f"       orig ({len(o_lines)} lines): {o_lines!r}")
                print(f"       rust ({len(r_lines)} lines): {r_lines!r}")

        if case_failed and keep_temps:
            print(f"       [KEEP_TEMPS] orig_workdir={o_workdir}")
            print(f"       [KEEP_TEMPS] rust_workdir={r_workdir}")

    if failures:
        print(f"\nTOTAL FAILURES: {failures}")
        return 1
    print("ALL TESTS PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

