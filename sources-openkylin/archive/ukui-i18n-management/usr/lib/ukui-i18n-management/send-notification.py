#!/usr/bin/env python3
# UKUI 通知发送与 ActionInvoked 监听
#
# freedesktop 规范要求：ActionInvoked 信号只发送给 Notify 的调用方。
# 因此必须在同一 D-Bus 连接中发送通知并监听信号，不能由独立 dbus-monitor 进程接收。

import argparse
import os
import subprocess
import sys
import syslog
import time

import dbus
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

LOG_TAG = "ukui-i18n-management"
DEFAULT_CONTROL_CENTER = "ukui-control-center"
DEFAULT_TIMEOUT_SEC = 86400

DBUS_NOTIFY_SERVICE = "org.freedesktop.Notifications"
DBUS_NOTIFY_PATH = "/org/freedesktop/Notifications"
DBUS_NOTIFY_INTERFACE = "org.freedesktop.Notifications"

syslog.openlog(LOG_TAG, syslog.LOG_PID, syslog.LOG_USER)


def log(msg):
    syslog.syslog(syslog.LOG_INFO, msg)


def launch_control_center(control_center):
    # 继承当前进程环境（DISPLAY / WAYLAND_DISPLAY 等），
    # 且不要 start_new_session，否则会脱离图形会话导致无法显示
    env = os.environ.copy()
    try:
        proc = subprocess.Popen(
            [control_center],
            env=env,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            start_new_session=False,
        )
        time.sleep(0.5)
        if proc.poll() is not None:
            err = proc.stderr.read().decode("utf-8", errors="replace").strip()
            log(
                f"{control_center} 启动后立即退出 (code={proc.returncode})"
                f"{': ' + err if err else ''}"
            )
            return
        log(
            f"已启动 {control_center} (pid={proc.pid}, "
            f"DISPLAY={env.get('DISPLAY', '')}, "
            f"WAYLAND={env.get('WAYLAND_DISPLAY', '')})"
        )
    except OSError as exc:
        log(f"启动 {control_center} 失败: {exc}")


def main():
    parser = argparse.ArgumentParser(description="发送 UKUI 通知并监听 ActionInvoked")
    parser.add_argument("title", help="通知标题")
    parser.add_argument("body", help="通知正文")
    parser.add_argument("--app-name", default=LOG_TAG, help="通知应用名")
    parser.add_argument(
        "--control-center",
        default=DEFAULT_CONTROL_CENTER,
        help="点击通知后启动的控制面板",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=DEFAULT_TIMEOUT_SEC,
        help="监听超时（秒）",
    )
    args = parser.parse_args()

    DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    notify_obj = bus.get_object(DBUS_NOTIFY_SERVICE, DBUS_NOTIFY_PATH)
    notify_iface = dbus.Interface(notify_obj, DBUS_NOTIFY_INTERFACE)

    hints = dbus.Dictionary(
        {"desktop-entry": dbus.String(args.control_center)},
        signature=dbus.Signature("sv"),
    )
    actions = ["default", "打开控制面板"]

    notify_id = int(
        notify_iface.Notify(
            args.app_name,
            dbus.UInt32(0),
            "",
            args.title,
            args.body,
            actions,
            hints,
            dbus.Int32(0),
        )
    )
    log(f"通知已发送 (ID: {notify_id})，开始监听 ActionInvoked")

    loop = GLib.MainLoop()
    handled = {"done": False}

    def on_action_invoked(invoked_id, action_key):
        invoked_id = int(invoked_id)
        action_key = str(action_key)
        log(f"收到 ActionInvoked (id={invoked_id}, action={action_key})")

        if invoked_id == notify_id and action_key == "default" and not handled["done"]:
            handled["done"] = True
            launch_control_center(args.control_center)
            loop.quit()

    def on_notification_closed(closed_id, reason):
        closed_id = int(closed_id)
        log(f"收到 NotificationClosed (id={closed_id}, reason={reason})")
        if closed_id == notify_id and not handled["done"]:
            loop.quit()

    def on_timeout():
        if not handled["done"]:
            log(f"ActionInvoked 监听超时 (通知 ID: {notify_id})")
            loop.quit()
        return False

    bus.add_signal_receiver(
        on_action_invoked,
        dbus_interface=DBUS_NOTIFY_INTERFACE,
        signal_name="ActionInvoked",
        bus_name=DBUS_NOTIFY_SERVICE,
        path=DBUS_NOTIFY_PATH,
    )
    bus.add_signal_receiver(
        on_notification_closed,
        dbus_interface=DBUS_NOTIFY_INTERFACE,
        signal_name="NotificationClosed",
        bus_name=DBUS_NOTIFY_SERVICE,
        path=DBUS_NOTIFY_PATH,
    )

    GLib.timeout_add_seconds(args.timeout, on_timeout)
    loop.run()


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        log(f"send-notification 异常: {exc}")
        sys.exit(1)
