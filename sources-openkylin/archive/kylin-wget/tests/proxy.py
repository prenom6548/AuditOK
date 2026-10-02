#!/usr/bin/env python3
"""最小 HTTP 正向代理（仅供 wget-rust 对照测试；仅支持 GET 常见路径）。"""
from __future__ import annotations

import argparse
import base64
import binascii
import socketserver
import urllib.error
import urllib.request
from urllib.parse import urlsplit, urlunsplit
from http.server import BaseHTTPRequestHandler


def _parse_basic_proxy_auth(value: str | None) -> tuple[str, str] | None:
    if not value or not value.startswith("Basic "):
        return None
    try:
        raw = base64.b64decode(value[6:].strip(), validate=False)
    except (binascii.Error, ValueError):
        return None
    try:
        s = raw.decode("utf-8")
    except UnicodeDecodeError:
        s = raw.decode("latin-1")
    if ":" in s:
        u, _, p = s.partition(":")
        return (u, p)
    return (s, "")


class Handler(BaseHTTPRequestHandler):
    server_version = "wget-rust-test-proxy/0.1"

    def log_message(self, fmt: str, *args) -> None:
        if getattr(self.server, "quiet", False):
            return
        super().log_message(fmt, *args)

    def do_GET(self) -> None:
        mode = getattr(self.server, "mode", "forward")
        if mode == "evil":
            if self.path.startswith("http://") or self.path.startswith("https://"):
                self.send_response(418)
                self.send_header("Content-Length", "0")
                self.end_headers()
                return
            self.send_error(400, "expected absolute URL")
            return

        # forward
        if not (self.path.startswith("http://") or self.path.startswith("https://")):
            self.send_error(400, "bad proxy request")
            return

        auth_user = getattr(self.server, "proxy_auth_user", "") or ""
        if auth_user:
            exp_pass = getattr(self.server, "proxy_auth_password", "") or ""
            parsed = _parse_basic_proxy_auth(self.headers.get("Proxy-Authorization"))
            ok = parsed is not None and parsed[0] == auth_user and parsed[1] == exp_pass
            if not ok:
                self.send_response(407, "Proxy Authentication Required")
                self.send_header("Proxy-Authenticate", 'Basic realm="wget-rust-test"')
                self.send_header("Content-Type", "text/plain; charset=utf-8")
                msg = b"proxy auth required\n"
                self.send_header("Content-Length", str(len(msg)))
                self.end_headers()
                self.wfile.write(msg)
                return

        try:
            # IDN/IRI 测试：把任意目标 host 映射到 127.0.0.1，但保留原始 Host 头。
            sp = urlsplit(self.path)
            orig_hostport = sp.netloc
            if "@" in orig_hostport:
                orig_hostport = orig_hostport.rsplit("@", 1)[1]

            # Normalize Host header to ASCII (punycode) to mimic "request uses IDNA ASCII"
            # regardless of how the client encoded the proxy absolute URL.
            host = sp.hostname or ""
            port = sp.port
            host_ascii = host
            try:
                host_ascii = host.encode("idna").decode("ascii")
            except Exception:
                host_ascii = host
            if port:
                orig_hostport = f"{host_ascii}:{port}"
            else:
                orig_hostport = host_ascii
            mapped_netloc = "127.0.0.1"
            if sp.port:
                mapped_netloc = f"{mapped_netloc}:{sp.port}"
            mapped_url = urlunsplit((sp.scheme, mapped_netloc, sp.path, sp.query, sp.fragment))

            req = urllib.request.Request(mapped_url, method="GET")
            if orig_hostport:
                req.add_header("Host", orig_hostport)
            with urllib.request.urlopen(req, timeout=60) as resp:
                self.send_response(resp.status)
                skip = {"transfer-encoding", "connection"}
                for k, v in resp.headers.items():
                    if k.lower() in skip:
                        continue
                    self.send_header(k, v)
                self.end_headers()
                self.wfile.write(resp.read())
        except urllib.error.HTTPError as e:
            self.send_response(e.code)
            for k, v in e.headers.items():
                if k.lower() in {"transfer-encoding", "connection"}:
                    continue
                self.send_header(k, v)
            self.end_headers()
            self.wfile.write(e.read())
        except Exception:
            self.send_error(502, "bad gateway")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=0, help="0 = 随机端口")
    ap.add_argument("--quiet", action="store_true")
    ap.add_argument("--mode", choices=("forward", "evil"), default="forward")
    ap.add_argument(
        "--proxy-auth-user",
        default="",
        help="若设置则要求客户端提供 Proxy-Authorization: Basic（forward 模式）",
    )
    ap.add_argument(
        "--proxy-auth-password",
        default="",
        help="与 --proxy-auth-user 配套的密码（允许空字符串）",
    )
    args = ap.parse_args()

    class ReusableTCPServer(socketserver.TCPServer):
        allow_reuse_address = True

    with ReusableTCPServer((args.host, args.port), Handler) as httpd:
        httpd.quiet = args.quiet
        httpd.mode = args.mode
        httpd.proxy_auth_user = (args.proxy_auth_user or "").strip()
        httpd.proxy_auth_password = args.proxy_auth_password or ""
        host, port = httpd.server_address
        # 与 server.py 一致：首行打印可被脚本读取的 base URL
        print(f"http://{host}:{port}", flush=True)
        httpd.serve_forever()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
