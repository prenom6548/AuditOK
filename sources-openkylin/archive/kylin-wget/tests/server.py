#!/usr/bin/env python3
from __future__ import annotations

import argparse
import base64
import os
import socketserver
import ssl
import time
from pathlib import Path
from http.server import BaseHTTPRequestHandler
from urllib.parse import parse_qs, urlparse
from email.utils import parsedate_to_datetime


def _http_date_ge(ims: str, last_modified: str) -> bool:
    """比较 HTTP-date（RFC 7231），避免对日期字符串做字典序比较。"""
    try:
        return parsedate_to_datetime(ims.strip()) >= parsedate_to_datetime(
            last_modified.strip()
        )
    except (TypeError, ValueError, OverflowError):
        return False


def _bytes(n: int, pattern: bytes = b"0123456789abcdef") -> bytes:
    if n <= 0:
        return b""
    out = bytearray()
    while len(out) < n:
        out.extend(pattern)
    return bytes(out[:n])


class Handler(BaseHTTPRequestHandler):
    server_version = "wget-rust-test-server/0.1"

    def log_message(self, fmt: str, *args) -> None:
        if getattr(self.server, "quiet", False):
            return
        super().log_message(fmt, *args)

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        path = parsed.path
        if path != "/echo":
            body = b"not found\n"
            self.send_response(404)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        try:
            length = int(self.headers.get("Content-Length", "0") or 0)
        except ValueError:
            length = 0
        raw = self.rfile.read(length) if length > 0 else b""
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Length", str(len(raw)))
        self.end_headers()
        self.wfile.write(raw)

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        path = parsed.path
        q = parse_qs(parsed.query)
        state_dir = getattr(self.server, "state_dir", None)

        if path == "/robots.txt":
            # robots 深水区：覆盖 UA 组选择、allow/disallow 顺序优先（first match wins）、以及 %XX 解码（不解 '/'）。
            body = (
                # legacy: forbid /recur/sub/ for TC105
                b"User-agent: *\n"
                b"Disallow: /recur/sub/\n"
                b"\n"
                # deep: Wget-specific record overrides "*" (GNU prune_non_exact)
                b"User-agent: wget\n"
                b"Allow: /rb/public\n"
                b"Disallow: /rb/\n"
                b"Allow: /rb/%7Euser\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ts.txt":
            # timestamping 测试：固定 Last-Modified；若 If-Modified-Since >= Last-Modified 则返回 304。
            # 这样只要本地文件 mtime 是“现在”，客户端就会跳过下载并保留原文件内容。
            last_modified = "Wed, 01 Jan 2020 00:00:00 GMT"
            ims = self.headers.get("If-Modified-Since", "")
            if ims and _http_date_ge(ims, last_modified):
                self.send_response(304)
                self.send_header("Last-Modified", last_modified)
                self.end_headers()
                return
            body = b"remote-ts\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Last-Modified", last_modified)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ts_ignore_ims.txt":
            # timestamping 深水区：服务端忽略 If-Modified-Since，仍返回 200 + Last-Modified。
            # body/长度与 /ts.txt 保持一致，便于 size 对齐检查。
            last_modified = "Wed, 01 Jan 2020 00:00:00 GMT"
            body = b"remote-ts\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Last-Modified", last_modified)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ok.txt":
            body = b"hello wget\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # restrict-file-names: serve any /rfn/* path with stable content.
        if path.startswith("/rfn/"):
            body = b"rfn\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path.startswith("/iri/"):
            body = b"iri\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path.startswith("/q/"):
            body = b"q\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/tsnq/start":
            # trust-server-names + query: redirect target contains query, should be appended to filename.
            self.send_response(302)
            self.send_header("Location", "/q/final.bin?sig=1")
            self.end_headers()
            return

        # trust-server-names: redirect decides output name by final URL last component
        if path == "/tsn/start":
            self.send_response(302)
            self.send_header("Location", "/tsn/final.bin")
            self.end_headers()
            return
        if path == "/tsn/final.bin":
            body = b"TSN-FINAL\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # no-directories deep cases: redirect to directory URL
        if path == "/ndredir/to_dir":
            self.send_response(302)
            self.send_header("Location", "/dp/")
            self.end_headers()
            return

        # trust-server-names + -nd deep case:
        # start is a non-directory URL, but redirects to a directory URL (/dp/).
        # With --trust-server-names, output name should follow the *effective* URL (directory -> default-page).
        if path == "/tsn_dir/start":
            self.send_response(302)
            self.send_header("Location", "/dp/")
            self.end_headers()
            return

        if path == "/tsnr/index.html":
            self.send_response(302)
            self.send_header("Location", "/tsnr/home.html")
            self.end_headers()
            return
        if path == "/tsnr/home.html":
            body = b"<html><body><a href='/tsnr/a.html'>A</a></body></html>"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/tsnr/a.html":
            body = b"<html><body>OK</body></html>"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # cut-dirs: build a deeper directory structure for recursive save paths
        if path == "/cut/one/two/index.html":
            body = b"<html><body><a href='/cut/one/two/a.txt'>A</a></body></html>"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/cut/one/two/a.txt":
            body = b"CUT-A\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # default-page: directory URL should map to default index filename
        if path == "/dp/":
            body = b"DP-DIR\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/echo_user_agent":
            # 用于验证 `-U/--user-agent` 是否正确注入请求头。
            ua = self.headers.get("User-Agent", "")
            if not ua:
                ua = "MISSING"
            body = (ua + "\n").encode("utf-8", "replace")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/is_wget_user_agent":
            # 判断 User-Agent 是否为类似 GNU wget 的格式，用于回归校验默认 UA 行为。
            ua = (self.headers.get("User-Agent", "") or "").strip()
            ok = ua.lower().startswith("wget/")
            body = (("YES" if ok else "NO") + "\n").encode("utf-8", "replace")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/echo_referer":
            # 用于验证 `--referer` 是否正确注入 Referer 请求头。
            ref = self.headers.get("Referer", "")
            if not ref:
                ref = "MISSING"
            body = (ref + "\n").encode("utf-8", "replace")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/echo_x_test":
            # 用于验证 `--header X-Test:...` 是否正确注入自定义请求头。
            v = self.headers.get("X-Test", "")
            if not v:
                v = "MISSING"
            body = (v + "\n").encode("utf-8", "replace")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/set_cookie":
            # 用于验证 cookie 保存：固定 Expires，保证对照测试可稳定比较。
            body = b"set\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Set-Cookie", "a=1; Path=/; Expires=Wed, 01 Jan 2030 00:00:00 GMT")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/echo_cookie":
            # 回显请求的 Cookie 头（无则 MISSING）
            ck = self.headers.get("Cookie", "") or "MISSING"
            body = (ck + "\n").encode("utf-8", "replace")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/hsts_set":
            body = b"hsts\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            # 固定值，避免测试抖动；includeSubDomains 用于 superdomain 行为（后续可扩展用例）。
            self.send_header("Strict-Transport-Security", "max-age=3600; includeSubDomains")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # 递归测试：最小站点结构（index -> a -> sub/b；index 还链到 /other/c 用于 no-parent）
        # 也提供目录 URL `/recur/` 用于验证 --default-page 行为。
        if path == "/recur/" or path == "/recur/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/recur/a.html">A</a>\n'
                b'<a href="/other/c.html">C</a>\n'
                + (b'<a href="http://localhost:%d/recur/foreign.html">F</a>\n' % self.server.server_address[1])
                + b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # no-directories deep cases: stable tiny site without dynamic port in HTML.
        if path == "/ndsite/" or path == "/ndsite/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/ndsite/a.html">A</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/ndsite/a.html":
            body = b"<!doctype html><html><body>OK</body></html>\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # relative-only deep cases: cover relative, root-relative, protocol-relative, absolute.
        if path == "/rel/index.html":
            port = self.server.server_address[1]
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="a.html">A</a>\n'
                b'<a href="/rel/root.html">ROOT</a>\n'
                + (b'<a href="http://localhost:%d/rel/abs1.html">ABS1</a>\n' % port)
                + (b'<a href="//localhost:%d/rel/abs2.html">ABS2</a>\n' % port)
                + b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rel/a.html":
            body = b"REL-A\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rel/root.html":
            body = b"REL-ROOT\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rel/abs1.html":
            body = b"REL-ABS1\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rel/abs2.html":
            body = b"REL-ABS2\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recur/a.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="sub/b.html">B</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/nofollow/index.html":
            body = (
                b"<!doctype html><html><head>\n"
                b'<meta name="robots" content="index,nofollow">\n'
                b"</head><body>\n"
                b'<a href="/recur/a.html">A</a>\n'
                b'<img src="/recurp/img.png">\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recur/sub/b.html":
            body = b"b\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recur/foreign.html":
            body = b"foreign\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # 递归策略测试：accept/reject 扩展名
        if path == "/acc/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/acc/a.txt">A</a>\n'
                b'<a href="/acc/b.bin">B</a>\n'
                b'<a href="/acc/c.html">C</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/acc/a.txt":
            body = b"a\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/acc/b.bin":
            body = b"BIN\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/acc/c.html":
            body = b"<!doctype html><html><body>c</body></html>\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # 递归策略测试：include/exclude directories
        if path == "/dirs/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/dirs/inc/keep.html">KEEP</a>\n'
                b'<a href="/dirs/exc/drop.html">DROP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/dirs/inc/keep.html":
            body = b"keep\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/dirs/exc/drop.html":
            body = b"drop\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # include/exclude 深水区：通配与 URL 编码路径（space -> %20）
        if path == "/dirs/wc/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/dirs/inc/keep.html">KEEP</a>\n'
                b'<a href="/dirs/inc%20x/keep.html">KEEPSPACE</a>\n'
                b'<a href="/dirs/exc/drop.html">DROP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/dirs/inc%20x/keep.html":
            body = b"keep space\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # accept/reject 深水区：通配、逗号多后缀、无扩展名、目录 URL + default-page 交互
        if path == "/accar/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/accar/file.html">HTML</a>\n'
                b'<a href="/accar/file.txt">TXT</a>\n'
                b'<a href="/accar/noext">NOEXT</a>\n'
                b'<a href="/accar/sub/">SUBDIR</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/accar/file.html":
            body = b"<!doctype html><html><body>file.html</body></html>\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/accar/file.txt":
            body = b"file.txt\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/accar/noext":
            body = b"noext\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/accar/sub/":
            body = b"<!doctype html><html><body>subdir</body></html>\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # accept/reject-regex 深水区：重定向 + query + 组合过滤
        if path == "/rx/index.html":
            port = self.server.server_address[1]
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/rx/start?from=index">START</a>\n'
                + (b'<a href="http://localhost:%d/rx/foreign?x=1">FOREIGN</a>\n' % port)
                + b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path.startswith("/rx/start"):
            # stable 302: include query portion in Location
            self.send_response(302)
            self.send_header("Location", "/rx/final.bin?ok=1")
            self.end_headers()
            return

        if path.startswith("/rx/final.bin"):
            body = b"final\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path.startswith("/rx/foreign"):
            body = b"foreign\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # robots 深水区站点
        if path == "/rb/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/rb/public/ok.html">PUB</a>\n'
                b'<a href="/rb/private/deny.html">PRIV</a>\n'
                b'<a href="/rb/%7Euser">TILDE</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rb/public/ok.html":
            body = b"ok\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rb/private/deny.html":
            body = b"deny\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/rb/%7Euser":
            body = b"tilde\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # 同站点/端口 深水区：用于测试 --no-parent 与不同端口/https 的交互
        if path == "/hp/index.html":
            peer = getattr(self.server, "peer_base_url", "")
            peer_https = getattr(self.server, "peer_https_base_url", "")
            if not peer:
                peer = f"http://{self.server.server_address[0]}:{self.server.server_address[1]}"
            if not peer_https:
                peer_https = peer.replace("http://", "https://", 1)
            body = (
                b"<!doctype html><html><body>\n"
                + (b'<a href="%s/hp2/outside.html">PORT</a>\n' % peer.encode("utf-8"))
                + (b'<a href="%s/hp2/https_outside.html">HTTPS</a>\n' % peer_https.encode("utf-8"))
                + b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/hp2/outside.html":
            body = b"port-outside\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/hp2/https_outside.html":
            body = b"https-outside\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # descend_redirect 深水区：重定向到“按 LIST/REGEX 本应拒绝的目录”，但仍需继续抽链/入队
        if path.startswith("/dr/start_list"):
            self.send_response(302)
            self.send_header("Location", "/dr/exc/target.html")
            self.end_headers()
            return
        if path.startswith("/dr/start_regex"):
            self.send_response(302)
            self.send_header("Location", "/dr/regex/target.html")
            self.end_headers()
            return
        if path == "/dr/exc/target.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/dirs/inc/keep.html">KEEP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/dr/regex/target.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/dirs/inc/keep.html">KEEP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # https-only 深水区：在 HTTPS 页面内混合相对/协议相对/绝对 http 链接，并覆盖 https->http 重定向
        if path == "/ho/index.html":
            peer_http = getattr(self.server, "peer_base_url", "")
            peer_https = getattr(self.server, "peer_https_base_url", "")
            if not peer_http:
                peer_http = f"http://{self.server.server_address[0]}:{self.server.server_address[1]}"
            if not peer_https:
                peer_https = peer_http.replace("http://", "https://", 1)
            hostport = peer_https.split("://", 1)[1]
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/ho/a.html">REL</a>\n'
                + (b'<a href="%s/ok.txt">ABS_HTTP</a>\n' % peer_http.encode("utf-8"))
                + (b'<a href="//%s/ho/pr.html">PROTO_REL</a>\n' % hostport.encode("utf-8"))
                + b'<a href="/ho/rd_to_http">RD_HTTP</a>\n'
                + b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ho/a.html":
            body = b"a\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ho/pr.html":
            body = b"pr\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ho/rd_to_http":
            peer_http = getattr(self.server, "peer_base_url", "")
            if not peer_http:
                peer_http = f"http://{self.server.server_address[0]}:{self.server.server_address[1]}"
            self.send_response(302)
            self.send_header("Location", f"{peer_http}/ok.txt")
            self.end_headers()
            return

        # redirect filter points deep tests: redirect to other host then decide whether to descend
        if path == "/rf/start_span":
            port = self.server.server_address[1]
            self.send_response(302)
            self.send_header("Location", f"http://localhost:{port}/rf/target.html")
            self.end_headers()
            return

        if path == "/rf/start_dom":
            port = self.server.server_address[1]
            self.send_response(302)
            self.send_header("Location", f"http://localhost:{port}/rf/target.html")
            self.end_headers()
            return

        if path == "/rf/start_list":
            self.send_response(302)
            self.send_header("Location", "/rf/list/target.html")
            self.end_headers()
            return

        if path == "/rf/start_regex":
            port = self.server.server_address[1]
            self.send_response(302)
            self.send_header("Location", f"http://localhost:{port}/rf/regex/target.html")
            self.end_headers()
            return

        if path == "/rf/target.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/rf/keep.html">KEEP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/rf/list/target.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/rf/keep.html">KEEP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/rf/regex/target.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/rf/keep_regex.html">KEEP</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/rf/keep.html":
            body = b"keep\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/rf/keep_regex.html":
            body = b"keep_regex\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        # page-requisites 测试：页面包含 img/script/link 等资源
        if path == "/recurp/index.html":
            body = (
                b"<!doctype html><html><head>\n"
                b'<link rel="stylesheet" href="/recurp/style.css">\n'
                b"</head><body>\n"
                b'<img src="/recurp/img.png">\n'
                b'<script src="/recurp/app.js"></script>\n'
                b'<a href="/recurp/a.html">A</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/a.html":
            body = b"<!doctype html><html><body>a</body></html>\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/style.css":
            body = b"@import \"/recurp/imp.css\";\nbody{background:url(/recurp/bg.png);color:#fff}\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/css; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/imp.css":
            body = b".x{background-image:url(\"/recurp/bg2.png\")}\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/css; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/app.js":
            body = b"console.log('ok');\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/javascript; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/img.png":
            body = b"PNGDATA\n"
            self.send_response(200)
            self.send_header("Content-Type", "image/png")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/bg.png":
            body = b"BGP\n"
            self.send_response(200)
            self.send_header("Content-Type", "image/png")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/recurp/bg2.png":
            body = b"BG2\n"
            self.send_response(200)
            self.send_header("Content-Type", "image/png")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/other/c.html":
            body = b"c\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cd_ok":
            body = b"cd payload line\n"
            self.send_response(200)
            self.send_header("Content-Disposition", 'attachment; filename="fromcd.bin"')
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cd_star":
            body = b"cd star\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Disposition", "attachment; filename*=UTF-8''hello%20world.txt")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cd_cont":
            body = b"cd cont\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header(
                "Content-Disposition",
                'attachment; filename*0="hello"; filename*1="world.txt"',
            )
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cd_both":
            body = b"cd both\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header(
                "Content-Disposition",
                'attachment; filename="A.ext"; filename*="B.ext"',
            )
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cdredir/start":
            self.send_response(302)
            self.send_header("Location", "/cd_star")
            self.end_headers()
            return

        if path == "/cdbase/index.html":
            body = b'<!doctype html><html><body><a href="/cd_star">a</a></body></html>\n'
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cfo/index.html":
            body = (
                b"<!doctype html><html><body>\n"
                b'<a href="/cfo/cdfile">CD</a>\n'
                b"</body></html>\n"
            )
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/cfo/cdfile":
            body = b"X\n"
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Disposition", 'attachment; filename="renamed.bin"')
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ae/noext":
            body = b"<!doctype html><html><body>ae</body></html>\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/ae/style":
            body = b"body{color:#000}\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/css; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/basic_ok.txt":
            auth = self.headers.get("Authorization", "")
            if not auth.lower().startswith("basic "):
                self.send_response(401)
                self.send_header("WWW-Authenticate", 'Basic realm="wget-test"')
                self.end_headers()
                return
            try:
                raw = base64.b64decode(auth[6:].strip()).decode("utf-8", "replace")
                user, _, pw = raw.partition(":")
            except Exception:
                self.send_response(401)
                self.send_header("WWW-Authenticate", 'Basic realm="wget-test"')
                self.end_headers()
                return
            if user != "httpauth" or pw != "httppass":
                self.send_response(401)
                self.send_header("WWW-Authenticate", 'Basic realm="wget-test"')
                self.end_headers()
                return
            body = b"basic ok\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/multi_a.txt":
            body = b"alpha-multi\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/multi_b.txt":
            body = b"beta-multi\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/echo_host":
            body = (self.headers.get("Host", "") + "\n").encode("utf-8", "replace")
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/fail_close":
            # 无状态：每次请求都直接断连（用于 TC26：不依赖 flaky 计数与 orig/rust 先后次序）
            try:
                self.connection.shutdown(2)
            except Exception:
                pass
            try:
                self.connection.close()
            except Exception:
                pass
            return

        if path == "/flaky":
            # Flaky endpoint: fail first N times then succeed.
            # Query params:
            #   fail: number of failures before success (default 2)
            #   mode: "close" (default) closes connection to simulate network error;
            #         "status" returns an HTTP error status code.
            #   code: HTTP status code for failures when mode=status (default 500)
            fail = int(q.get("fail", ["2"])[0])
            mode = (q.get("mode", ["close"])[0] or "close").lower()
            code = int(q.get("code", ["500"])[0])
            # sid：隔离不同 TC 的计数器，避免 compare.py 先跑 orig 再跑 rust 时状态被 orig「用完」
            sid = (q.get("sid", [""])[0] or "").replace(os.sep, "_").replace("/", "_")[:64]
            key = f"flaky_{fail}_{code}" + (f"_{sid}" if sid else "")
            hits = 0
            if state_dir:
                p = os.path.join(state_dir, key)
                try:
                    with open(p, "r", encoding="utf-8") as f:
                        hits = int((f.read() or "0").strip())
                except FileNotFoundError:
                    hits = 0
                except Exception:
                    hits = 0
                try:
                    with open(p, "w", encoding="utf-8") as f:
                        f.write(str(hits + 1))
                except Exception:
                    pass
            else:
                hits = 0

            if hits < fail:
                if mode == "status":
                    body = b"flaky fail\n"
                    self.send_response(code)
                    self.send_header("Content-Type", "text/plain; charset=utf-8")
                    self.send_header("Content-Length", str(len(body)))
                    self.end_headers()
                    self.wfile.write(body)
                    return
                # mode=close: simulate network error by closing connection without response.
                try:
                    self.connection.shutdown(2)
                except Exception:
                    pass
                try:
                    self.connection.close()
                except Exception:
                    pass
                return

            body = b"flaky ok\n"
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/notfound":
            body = b"not found\n"
            self.send_response(404)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        if path == "/redirect":
            self.send_response(302)
            self.send_header("Location", "/ok.txt")
            self.end_headers()
            return

        if path == "/big.bin":
            size = int(q.get("size", ["1048576"])[0])
            data = _bytes(size)
            rng = self.headers.get("Range")
            if rng and rng.startswith("bytes=") and "-" in rng:
                start_s, end_s = rng[len("bytes=") :].split("-", 1)
                start = int(start_s) if start_s else 0
                end = int(end_s) if end_s else (size - 1)
                start = max(0, min(start, size))
                end = max(start, min(end, size - 1))
                part = data[start : end + 1]
                self.send_response(206)
                self.send_header("Content-Type", "application/octet-stream")
                self.send_header("Accept-Ranges", "bytes")
                self.send_header("Content-Range", f"bytes {start}-{end}/{size}")
                self.send_header("Content-Length", str(len(part)))
                self.end_headers()
                self.wfile.write(part)
                return

            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Accept-Ranges", "bytes")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
            return

        if path == "/nolength_big":
            # 没有 `Content-Length` 的大响应：用于验证 wget-rust 在未知长度场景下仍能输出进度文案。
            size = int(q.get("size", ["4194304"])[0])
            delay = float(q.get("delay", ["0.02"])[0])
            chunk_size = 64 * 1024
            pattern = _bytes(chunk_size)

            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.end_headers()

            remaining = size
            while remaining > 0:
                n = min(chunk_size, remaining)
                self.wfile.write(pattern[:n])
                self.wfile.flush()
                remaining -= n
                time.sleep(delay)
            return

        if path == "/slow":
            delay = float(q.get("delay", ["2.0"])[0])
            body = b"slow\n"
            time.sleep(delay)
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return

        body = b"unknown\n"
        self.send_response(404)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=0, help="0 means random free port")
    ap.add_argument("--quiet", action="store_true")
    ap.add_argument("--state-dir", default="", help="directory for storing server state")
    ap.add_argument("--peer-base-url", default="", help="optional peer base url for cross-port tests")
    ap.add_argument("--peer-https-base-url", default="", help="optional peer https base url for cross-scheme tests")
    ap.add_argument(
        "--https",
        action="store_true",
        help="serve HTTPS using PEM cert/key (defaults under tests/fixtures/tls/)",
    )
    ap.add_argument("--tls-cert", default="", help="server certificate PEM (default: fixtures/tls/server.pem)")
    ap.add_argument("--tls-key", default="", help="server private key PEM (default: fixtures/tls/server.key)")
    args = ap.parse_args()

    class ReusableTCPServer(socketserver.TCPServer):
        allow_reuse_address = True

    fixture_dir = Path(__file__).resolve().parent / "fixtures" / "tls"
    cert_path = Path(args.tls_cert) if args.tls_cert else fixture_dir / "server.pem"
    key_path = Path(args.tls_key) if args.tls_key else fixture_dir / "server.key"

    with ReusableTCPServer((args.host, args.port), Handler) as httpd:
        httpd.quiet = args.quiet
        httpd.state_dir = args.state_dir or ""
        httpd.peer_base_url = args.peer_base_url or ""
        httpd.peer_https_base_url = args.peer_https_base_url or ""
        host, port = httpd.server_address
        if args.https:
            if not cert_path.is_file() or not key_path.is_file():
                print(
                    f"ERROR: missing TLS files (cert={cert_path} key={key_path})",
                    flush=True,
                )
                return 2
            ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            ctx.load_cert_chain(str(cert_path), str(key_path))
            httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)
            scheme = "https"
        else:
            scheme = "http"
        print(f"{scheme}://{host}:{port}", flush=True)
        httpd.serve_forever()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

