"""
Copyright (C) 2026, KylinSoft Co., Ltd.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
import glob
import os
import shutil
import subprocess
import time
from pathlib import Path

import cv2

# 与 ensure-vcam 中 card_label 保持一致
VIRTUAL_CAMERA_LABEL = "UKUI-Air-Gesture-Camera"
ENSURE_VCAM_HELPER = "/usr/libexec/ukui-air-gesture/ensure-vcam"
REAL_CAM_STATE = "/run/ukui-air-gesture/real-camera"


def _sysfs_text(node_path, field):
    path = Path("/sys/class/video4linux") / Path(node_path).name / field
    if not path.is_file():
        return ""
    return path.read_text(encoding="utf-8").strip()


def is_v4l2loopback(node_path):
    driver = Path("/sys/class/video4linux") / Path(node_path).name / "device" / "driver"
    if driver.is_symlink() and "v4l2loopback" in os.readlink(driver):
        return True
    name = _sysfs_text(node_path, "name").lower()
    return name == VIRTUAL_CAMERA_LABEL.lower() or "loopback" in name


def list_video_nodes():
    def sort_key(path):
        suffix = Path(path).name[5:]
        return int(suffix) if suffix.isdigit() else 999

    return sorted(glob.glob("/dev/video*"), key=sort_key)


def find_real_camera_node():
    env_path = os.environ.get("AIR_GESTURE_CAMERA", "").strip()
    if env_path:
        return env_path if os.path.exists(env_path) else None

    state_file = Path(REAL_CAM_STATE)
    if state_file.is_file():
        state_path = state_file.read_text(encoding="utf-8").strip()
        if state_path and os.path.exists(state_path) and not is_v4l2loopback(state_path):
            return state_path

    for node in list_video_nodes():
        if is_v4l2loopback(node):
            continue
        index = _sysfs_text(node, "index")
        if index == "" or index == "0":
            return node
    return None


def find_virtual_camera_node():
    env_path = os.environ.get("AIR_GESTURE_VIRTUAL_CAMERA", "").strip()
    if env_path:
        return env_path if os.path.exists(env_path) else None

    label = VIRTUAL_CAMERA_LABEL.lower()
    for node in list_video_nodes():
        if is_v4l2loopback(node) and _sysfs_text(node, "name").lower() == label:
            return node
    return None


def _run_ensure_vcam():
    if not os.path.isfile(ENSURE_VCAM_HELPER):
        print(f"ensure-vcam helper missing: {ENSURE_VCAM_HELPER}")
        return False

    commands = []
    if os.geteuid() != 0:
        pkexec = shutil.which("pkexec")
        if pkexec:
            commands.append([pkexec, ENSURE_VCAM_HELPER])
    commands.append([ENSURE_VCAM_HELPER])

    for cmd in commands:
        result = subprocess.run(cmd, capture_output=True, text=True, check=False)
        out = (result.stdout or "").strip()
        err = (result.stderr or "").strip()
        print(f"ensure-vcam ({cmd[0]}) rc={result.returncode} out={out} err={err}")
        if result.returncode == 0:
            return True
    return False


def activate_shared_camera():
    virtual = find_virtual_camera_node()
    real = find_real_camera_node()
    if virtual == "/dev/video0" and real:
        print(f"Shared camera ready: virtual=/dev/video0 real={real}")
        return True

    if not _run_ensure_vcam():
        print(
            "Failed to put virtual camera on /dev/video0. "
            "Other apps that open video0 will still conflict with this process."
        )
        return False

    virtual = find_virtual_camera_node()
    real = find_real_camera_node()
    if virtual != "/dev/video0" or not real:
        print(f"Shared camera setup incomplete: virtual={virtual} real={real}")
        return False

    print(f"Shared camera ready: virtual={virtual} real={real}")
    return True


class LoopbackWriter:

    def __init__(self, device, width, height, fps):
        self.device = device
        self.width = int(width)
        self.height = int(height)
        self.fps = max(float(fps), 1.0)
        self._proc = None

        ffmpeg = shutil.which("ffmpeg")
        if not ffmpeg:
            raise RuntimeError("ffmpeg not found")

        self._proc = subprocess.Popen(
            [
                ffmpeg,
                "-y",
                "-loglevel", "error",
                "-f", "rawvideo",
                "-pix_fmt", "bgr24",
                "-s", f"{self.width}x{self.height}",
                "-r", str(self.fps),
                "-i", "-",
                "-an",
                "-f", "v4l2",
                "-pix_fmt", "yuyv422",
                self.device,
            ],
            stdin=subprocess.PIPE,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        print(
            f"Forwarding frames to virtual camera: {self.device} "
            f"({self.width}x{self.height} @ {self.fps:.1f}fps)"
        )

    def write(self, frame):
        proc = self._proc
        if proc is None or proc.stdin is None or proc.poll() is not None:
            if proc is not None and proc.poll() is not None:
                err = proc.stderr.read() if proc.stderr else b""
                print(
                    "Virtual camera writer exited: "
                    f"{err.decode(errors='ignore').strip()}"
                )
                self._proc = None
            return False

        if frame.shape[1] != self.width or frame.shape[0] != self.height:
            frame = cv2.resize(frame, (self.width, self.height))

        try:
            proc.stdin.write(frame.tobytes())
        except BrokenPipeError:
            print("Virtual camera writer pipe broken.")
            self.close()
            return False
        return True

    def close(self):
        proc = self._proc
        self._proc = None
        if proc is None:
            return

        if proc.stdin and not proc.stdin.closed:
            proc.stdin.close()

        if proc.poll() is not None:
            return

        proc.terminate()
        for _ in range(20):
            if proc.poll() is not None:
                return
            time.sleep(0.1)
        proc.kill()
        proc.wait()


def open_loopback_writer(width, height, fps):
    device = find_virtual_camera_node()
    if not device:
        print("Virtual camera node not found")
        return None
    if not shutil.which("ffmpeg"):
        print("ffmpeg not found")
        return None
    return LoopbackWriter(device, width, height, fps)
