#!/usr/bin/env python3
import argparse
import socket
import threading
from typing import Dict


def _sendline(conn: socket.socket, s: str) -> None:
    conn.sendall((s + "\r\n").encode("utf-8"))


class FtpSession:
    def __init__(self, conn: socket.socket, addr, root_files: Dict[str, bytes], user: str, password: str):
        self.conn = conn
        self.addr = addr
        self.root_files = root_files
        self.want_user = user
        self.want_pass = password
        self.authed = False
        self.user = ""
        self.type = "I"
        self.cwd = "/"
        self.pasv_listener = None
        self.pasv_addr = None

    def _open_pasv(self):
        if self.pasv_listener is not None:
            try:
                self.pasv_listener.close()
            except Exception:
                pass
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(("127.0.0.1", 0))
        s.listen(1)
        host, port = s.getsockname()
        self.pasv_listener = s
        self.pasv_addr = (host, port)
        h1, h2, h3, h4 = host.split(".")
        p1 = port // 256
        p2 = port % 256
        _sendline(self.conn, f"227 Entering Passive Mode ({h1},{h2},{h3},{h4},{p1},{p2}).")

    def _accept_data(self) -> socket.socket:
        assert self.pasv_listener is not None
        self.pasv_listener.settimeout(5.0)
        d, _ = self.pasv_listener.accept()
        try:
            self.pasv_listener.close()
        except Exception:
            pass
        self.pasv_listener = None
        return d

    def handle(self):
        _sendline(self.conn, "220 ftp-test ready")
        buf = b""
        while True:
            data = self.conn.recv(4096)
            if not data:
                break
            buf += data
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                line = line.strip(b"\r").decode("utf-8", "replace")
                if not line:
                    continue
                parts = line.split(" ", 1)
                cmd = parts[0].upper()
                arg = parts[1] if len(parts) > 1 else ""

                if cmd == "USER":
                    self.user = arg.strip()
                    _sendline(self.conn, "331 Password required")
                    continue
                if cmd == "PASS":
                    pw = arg.strip()
                    # allow anonymous always; or exact match of configured user/pass
                    if self.user.lower() == "anonymous" or (self.user == self.want_user and pw == self.want_pass):
                        self.authed = True
                        _sendline(self.conn, "230 Login ok")
                    else:
                        _sendline(self.conn, "530 Login incorrect")
                    continue
                if cmd == "SYST":
                    _sendline(self.conn, "215 UNIX Type: L8")
                    continue
                if cmd == "PWD":
                    _sendline(self.conn, f'257 "{self.cwd}" is current directory')
                    continue
                if cmd == "CWD":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    p = arg.strip() or "/"
                    if not p.startswith("/"):
                        if self.cwd.endswith("/"):
                            p = self.cwd + p
                        else:
                            p = self.cwd + "/" + p
                    # support /, /dir, /auth (for netrc tests)
                    if p in ("/", "./", ""):
                        self.cwd = "/"
                        _sendline(self.conn, "250 Directory successfully changed")
                    elif p == "/dir" or p == "/dir/":
                        self.cwd = "/dir"
                        _sendline(self.conn, "250 Directory successfully changed")
                    elif p == "/auth" or p == "/auth/":
                        self.cwd = "/auth"
                        _sendline(self.conn, "250 Directory successfully changed")
                    else:
                        _sendline(self.conn, "550 Failed to change directory")
                    continue
                if cmd == "TYPE":
                    self.type = (arg.strip() or "I").upper()
                    _sendline(self.conn, "200 Type set")
                    continue
                if cmd == "PASV":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    self._open_pasv()
                    continue
                if cmd == "EPSV":
                    # treat like PASV (only IPv4 in this test server)
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    self._open_pasv()
                    continue
                if cmd == "SIZE":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    path = (arg.strip() or "").strip()
                    if not path.startswith("/"):
                        path = (self.cwd.rstrip("/") + "/" + path).lstrip("/")
                    else:
                        path = path.lstrip("/")
                    content = self.root_files.get("/" + path)
                    if content is None:
                        _sendline(self.conn, "550 Not found")
                    else:
                        _sendline(self.conn, f"213 {len(content)}")
                    continue
                if cmd == "MDTM":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    # Return a fixed timestamp
                    _sendline(self.conn, "213 20260101000000")
                    continue
                if cmd == "LIST":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    if self.pasv_listener is None:
                        _sendline(self.conn, "425 Use PASV first")
                        continue
                    _sendline(self.conn, "150 Opening ASCII mode data connection for file list")
                    d = self._accept_data()
                    # Minimal unix-ish listing with a directory and a file.
                    # Serve different listing depending on path.
                    raw = (arg.strip() or "").strip()
                    # wget may send: "LIST -a" or "LIST -la /path"
                    toks = raw.split()
                    p = ""
                    if len(toks) >= 1:
                        # choose last token that doesn't start with '-'
                        for t in reversed(toks):
                            if not t.startswith("-"):
                                p = t
                                break
                    if p == "":
                        p = self.cwd
                    if not p.startswith("/"):
                        if self.cwd.endswith("/"):
                            p = self.cwd + p
                        else:
                            p = self.cwd + "/" + p
                    if p == "" or p == "/" or p == "./":
                        listing = (
                            "drwxr-xr-x 1 owner group 0 Jan 01 00:00 dir\r\n"
                            "-rw-r--r-- 1 owner group 4 Jan 01 00:00 file.txt\r\n"
                            "-rw-r--r-- 1 owner group 4 Jan 01 00:00 file.bin\r\n"
                            "-rw-r--r-- 1 owner group 4 Jan 01 00:00 /abs.txt\r\n"
                            "-rw-r--r-- 1 owner group 4 Jan 01 00:00 ../evil.txt\r\n"
                            "-rw-r--r-- 1 owner group 4 Jan 01 00:00 file.txt\r\n"
                        )
                    elif p.startswith("/dir"):
                        listing = (
                            "-rw-r--r-- 1 owner group 7 Jan 01 00:00 sub.txt\r\n"
                            "lrwxrwxrwx 1 owner group 3 Jan 01 00:00 link.txt -> ok.txt\r\n"
                        )
                    else:
                        listing = ""
                    d.sendall(listing.encode("utf-8"))
                    d.close()
                    _sendline(self.conn, "226 Transfer complete")
                    continue
                if cmd == "NLST":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    if self.pasv_listener is None:
                        _sendline(self.conn, "425 Use PASV first")
                        continue
                    _sendline(self.conn, "150 Opening ASCII mode data connection for name list")
                    d = self._accept_data()
                    p = (arg.strip() or "").strip() or self.cwd
                    if p == "/" or p == "./":
                        listing = "dir\r\nfile.txt\r\n"
                    elif p.startswith("/dir"):
                        listing = "sub.txt\r\n"
                    else:
                        listing = ""
                    d.sendall(listing.encode("utf-8"))
                    d.close()
                    _sendline(self.conn, "226 Transfer complete")
                    continue
                if cmd == "RETR":
                    if not self.authed:
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    if self.pasv_listener is None:
                        _sendline(self.conn, "425 Use PASV first")
                        continue
                    path = (arg.strip() or "").strip()
                    if path == "":
                        _sendline(self.conn, "550 Not found")
                        continue
                    if not path.startswith("/"):
                        if self.cwd.endswith("/"):
                            path = self.cwd + path
                        else:
                            path = self.cwd + "/" + path
                    path = path.lstrip("/")
                    # Require real credentials for /auth/* paths (to test .netrc / --no-netrc).
                    if path.startswith("auth/") and self.user.lower() == "anonymous":
                        _sendline(self.conn, "530 Not logged in")
                        continue
                    if path.endswith("/"):
                        _sendline(self.conn, "550 Not a regular file")
                        continue
                    content = self.root_files.get("/" + path)
                    if content is None:
                        _sendline(self.conn, "550 Not found")
                        continue
                    _sendline(self.conn, "150 Opening BINARY mode data connection")
                    d = self._accept_data()
                    d.sendall(content)
                    d.close()
                    _sendline(self.conn, "226 Transfer complete")
                    continue
                if cmd == "QUIT":
                    _sendline(self.conn, "221 Bye")
                    return

                _sendline(self.conn, "502 Command not implemented")


def serve(port: int, user: str, password: str):
    files = {
        "/ok.txt": b"ok\n",
        "/auth/secret.txt": b"secret\n",
        "/file.txt": b"data",
        "/file.bin": b"BIN!",
        "/dir/sub.txt": b"subtxt\n",
        "/dir/link.txt": b"lnk",
    }
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("127.0.0.1", port))
    s.listen(20)
    while True:
        c, addr = s.accept()
        sess = FtpSession(c, addr, files, user, password)
        t = threading.Thread(target=lambda: sess.handle(), daemon=True)
        t.start()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=0)
    ap.add_argument("--user", type=str, default="u")
    ap.add_argument("--password", type=str, default="p")
    args = ap.parse_args()

    port = args.port
    if port == 0:
        tmp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        tmp.bind(("127.0.0.1", 0))
        port = tmp.getsockname()[1]
        tmp.close()

    print(f"ftp://127.0.0.1:{port}", flush=True)
    serve(port, args.user, args.password)


if __name__ == "__main__":
    main()

