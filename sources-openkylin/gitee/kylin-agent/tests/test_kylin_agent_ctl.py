"""Client transport tests, using real Unix sockets without a running desktop."""

from contextlib import contextmanager
import json
from pathlib import Path
import socket
import sys
import tempfile
import threading
import time
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from kylin_agent_ctl import ControlError, KylinAgentClient


@contextmanager
def server(handler):
    with tempfile.TemporaryDirectory() as directory:
        path = str(Path(directory) / "control.socket")
        failures = []
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as listener:
            listener.bind(path)
            listener.listen(1)
            listener.settimeout(2)

            def serve():
                try:
                    connection, _ = listener.accept()
                    with connection:
                        connection.settimeout(2)
                        data = bytearray()
                        while b"\n" not in data:
                            part = connection.recv(4096)
                            if not part:
                                raise RuntimeError("Incomplete request")
                            data.extend(part)
                        handler(connection, json.loads(data))
                except Exception as exc:
                    failures.append(exc)

            worker = threading.Thread(target=serve, daemon=True)
            worker.start()
            try:
                yield KylinAgentClient(path, timeout=1)
            finally:
                worker.join(3)
                if worker.is_alive():
                    raise RuntimeError("Server thread did not finish")
                if failures:
                    raise failures[0]


class ClientTests(unittest.TestCase):
    def test_running_count(self):
        def handle(connection, request):
            self.assertEqual(request["command"], "running_count")
            connection.sendall(json.dumps({"ok": True, "id": request["id"],
                                           "result": {"running_count": 2,
                                                      "running_session_ids": ["a", "b"]}}).encode() + b"\n")

        with server(handle) as client:
            self.assertEqual(client.running_count(), 2)

    def test_missing_socket_has_actionable_error(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(ControlError) as caught:
                KylinAgentClient(Path(directory) / "missing.socket").status()
        self.assertEqual(caught.exception.code, "control_unavailable")
        self.assertIn("完全退出旧版", str(caught.exception))

    def test_unicode_and_fragmented_response(self):
        text = '中文\n"quoted" ✅'

        def handle(connection, request):
            self.assertEqual(request["command"], "set_input")
            self.assertEqual(request["text"], text)
            self.assertEqual(request["session_id"], "session-1")
            response = json.dumps({"ok": True, "id": request["id"], "result": {"input": text}},
                                  ensure_ascii=False).encode() + b"\n"
            for byte in response:
                connection.sendall(bytes([byte]))

        with server(handle) as client:
            self.assertEqual(client.set_input(text, "session-1")["input"], text)

    def test_error(self):
        def handle(connection, request):
            connection.sendall(json.dumps({"ok": False, "id": request["id"], "error": {
                "code": "task_running", "message": "Already running"
            }}).encode() + b"\n")

        with server(handle) as client, self.assertRaises(ControlError) as caught:
            client.send("hello")
        self.assertEqual(caught.exception.code, "task_running")

    def test_bad_response(self):
        for payload, code in [(b"not json\n", "invalid_response"),
                              (b"[]\n", "invalid_response"),
                              (b'{"ok":true,"id":"wrong","result":{}}\n', "invalid_response"),
                              (b"{", "incomplete_response")]:
            with self.subTest(payload=payload):
                with server(lambda connection, _: connection.sendall(payload)) as client:
                    with self.assertRaises(ControlError) as caught:
                        client.status()
                    self.assertEqual(caught.exception.code, code)

    def test_timeout(self):
        with server(lambda *_: time.sleep(0.1)) as client:
            client.timeout = 0.02
            with self.assertRaises(TimeoutError):
                client.status()

    def test_oversized_request_rejected_before_connect(self):
        with self.assertRaises(ControlError) as caught:
            KylinAgentClient("/does/not/exist").send("中" * 400000)
        self.assertEqual(caught.exception.code, "request_too_large")

    def test_send_without_text_uses_current_input(self):
        def handle(connection, request):
            self.assertNotIn("text", request)
            connection.sendall(json.dumps({"ok": True, "id": request["id"],
                                           "result": {"accepted": True}}).encode() + b"\n")

        with server(handle) as client:
            self.assertTrue(client.send()["accepted"])


if __name__ == "__main__":
    unittest.main()
