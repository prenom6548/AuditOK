#!/usr/bin/env python3
"""Validate this standalone Jazzy image; default includes live DDS/Gazebo tests."""
import argparse
import os
import platform
from pathlib import Path
import signal
import shutil
import subprocess
import tempfile
import time


PREFIX = Path('/opt/ros/jazzy')
# Cross-architecture validation under QEMU needs longer startup deadlines.
TIMEOUT_SCALE = float(os.environ.get('JAZZY_VALIDATION_TIMEOUT_SCALE', '10' if platform.machine() in ('aarch64', 'riscv64') else '1'))


def run(*command, **kwargs):
    return subprocess.run(command, check=True, text=True, **kwargs)


def static_checks():
    from ament_index_python.packages import get_packages_with_prefixes
    import cv2
    import PyKDL
    import matplotlib
    import pygraphviz
    import rclpy
    import rqt_gui
    from cv_bridge import CvBridge
    from python_qt_binding import QtCore

    installed = get_packages_with_prefixes()
    expected = set(Path('/opt/ros-build-manifests/packages.txt').read_text().split())
    required = {
        'desktop_full', 'desktop', 'perception', 'simulation', 'ros_base',
        'rviz2', 'rviz_default_plugins', 'rqt_common_plugins', 'turtlesim',
        'pcl_ros', 'cv_bridge', 'camera_calibration', 'ros_gz_bridge',
        'ros_gz_sim', 'ros_gz_image', 'ros_gz_interfaces', 'ros_gz_sim_demos',
        'gz_sim_vendor', 'gz_gui_vendor', 'gz_rendering_vendor',
    }
    # Plain CMake dependencies (e.g. fastcdr and urdfdom_headers) are indexed
    # by colcon, but are not required to register an ament resource marker.
    colcon_index = PREFIX / 'share' / 'colcon-core' / 'packages'
    colcon_installed = {path.name for path in colcon_index.iterdir() if path.is_file()}
    missing = (expected - colcon_installed) | (required - installed.keys())
    if missing:
        raise RuntimeError(f'Missing ROS packages: {sorted(missing)}')
    wrong = {p: installed[p] for p in expected & installed.keys()
             if installed[p] != str(PREFIX)}
    if wrong:
        raise RuntimeError(f'Packages outside the standalone prefix: {wrong}')

    # Check every ELF installed by ROS and the manually built dependencies,
    # including Gazebo plugin libraries; checking only *.so misses executables.
    failures = []
    elf_count = 0
    for root in (PREFIX, Path('/opt/pcl'), Path('/opt/flann'),
                 Path('/opt/assimp'), Path('/opt/simulation-deps'), Path('/opt/poco'), Path('/opt/jazzy-graphviz')):
        for path in root.rglob('*'):
            if not path.is_file() or path.is_symlink():
                continue
            with path.open('rb') as source:
                if source.read(4) != b'\x7fELF':
                    continue
            elf_count += 1
            result = subprocess.run(['ldd', str(path)], capture_output=True, text=True)
            if 'not found' in result.stdout + result.stderr:
                failures.append(f'{path}:\n{result.stdout}{result.stderr}')
    if failures:
        raise RuntimeError('Unresolved shared libraries:\n' + '\n'.join(failures))
    with tempfile.TemporaryDirectory() as directory:
        graph = pygraphviz.AGraph(directed=True)
        graph.add_edge('talker', 'listener')
        output = Path(directory) / 'graph.svg'
        graph.draw(str(output), prog='dot', format='svg')
        if not output.stat().st_size:
            raise RuntimeError('Graphviz produced an empty image')
    env = dict(os.environ, QT_QPA_PLATFORM='offscreen')
    run('ros2', 'run', 'rviz2', 'rviz2', '--help', env=env, timeout=60 * TIMEOUT_SCALE)
    result = run('gz', 'sim', '--versions', capture_output=True, timeout=60 * TIMEOUT_SCALE)
    if not any(line.strip().startswith('8.') for line in result.stdout.splitlines()):
        raise RuntimeError(f'Expected Gazebo Sim 8 (Harmonic): {result.stdout}')
    print(f'static=ok packages={len(installed)} elf_files={elf_count}', flush=True)


def dds_check():
    import rclpy
    from std_msgs.msg import String
    rclpy.init()
    node = rclpy.create_node('jazzy_full_dds_validation')
    received = []
    subscription = node.create_subscription(String, 'jazzy_full_check',
                                           lambda msg: received.append(msg.data), 10)
    publisher = node.create_publisher(String, 'jazzy_full_check', 10)
    message = String(data='jazzy-desktop-full')
    try:
        deadline = time.monotonic() + 20 * TIMEOUT_SCALE
        while not received and time.monotonic() < deadline:
            publisher.publish(message)
            rclpy.spin_once(node, timeout_sec=0.1)
        if message.data not in received:
            raise RuntimeError('DDS publish/subscribe timed out')
        print('dds-pubsub=ok', flush=True)
    finally:
        node.destroy_node()
        rclpy.shutdown()


def clock_check():
    import rclpy
    from rclpy.qos import qos_profile_sensor_data
    from rosgraph_msgs.msg import Clock
    rclpy.init()
    node = rclpy.create_node('jazzy_full_clock_validation')
    stamps = []
    topic = '/world/jazzy_full_check/clock'
    subscription = node.create_subscription(
        Clock, topic,
        lambda msg: stamps.append(msg.clock.sec * 1_000_000_000 + msg.clock.nanosec),
        qos_profile_sensor_data)
    processes = []
    logs = []
    try:
        with tempfile.TemporaryDirectory(prefix='jazzy-full-') as directory:
            work = Path(directory)
            world = work / 'world.sdf'
            world.write_text('''<?xml version="1.0"?>
<sdf version="1.9"><world name="jazzy_full_check">
  <physics name="default" type="ignored">
    <max_step_size>0.001</max_step_size><real_time_factor>1</real_time_factor>
  </physics>
  <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
  <model name="box"><pose>0 0 10 0 0 0</pose><link name="body">
    <inertial><mass>1</mass><inertia><ixx>0.1</ixx><iyy>0.1</iyy><izz>0.1</izz></inertia></inertial>
    <collision name="box"><geometry><box><size>1 1 1</size></box></geometry></collision>
  </link></model>
</world></sdf>''')
            # Both Gazebo processes run locally; avoid QEMU/container interface discovery.
            env = dict(os.environ, GZ_PARTITION=f'jazzy-full-{os.getpid()}',
                       GZ_IP='127.0.0.1')
            commands = (
                ('bridge', ['ros2', 'run', 'ros_gz_bridge', 'parameter_bridge',
                            topic + '@rosgraph_msgs/msg/Clock[gz.msgs.Clock']),
                ('simulation', ['gz', 'sim', '-s', '-r', '-v', '4', str(world)]),
            )
            emulator = os.environ.get('JAZZY_VALIDATION_QEMU')
            if emulator:
                # Explicit execution keeps Gazebo multicast syscalls on the pinned
                # emulator without changing the host binfmt registration.
                commands = (
                    ('bridge', [emulator, str(PREFIX / 'lib/ros_gz_bridge/parameter_bridge'),
                                str(PREFIX / 'lib/ros_gz_bridge/parameter_bridge'),
                                topic + '@rosgraph_msgs/msg/Clock[gz.msgs.Clock']),
                    ('simulation', [emulator, shutil.which('ruby'), shutil.which('ruby'), shutil.which('gz'),
                                    'sim', '-s', '-r', '-v', '4', str(world)]),
                )
            try:
                for name, command in commands:
                    log = (work / f'{name}.log').open('w')
                    logs.append(log)
                    child_env = dict(env, LD_DEBUG='libs') if name == 'simulation' and emulator else env
                    processes.append(subprocess.Popen(command, env=child_env, stdout=log,
                                     stderr=subprocess.STDOUT, start_new_session=True))
                deadline = time.monotonic() + 120 * TIMEOUT_SCALE
                while time.monotonic() < deadline:
                    rclpy.spin_once(node, timeout_sec=0.5)
                    if any(p.poll() is not None for p in processes):
                        raise RuntimeError('Gazebo or bridge exited early')
                    if len(stamps) >= 5 and stamps[-1] > stamps[0] >= 0:
                        print('gazebo-to-ros-clock=ok', flush=True)
                        break
                else:
                    raise RuntimeError(f'No advancing Gazebo clock: {stamps[:5]}')
            except Exception:
                for path in work.glob('*.log'):
                    print(f'{path.name}:\n{path.read_text(errors="replace")}', flush=True)
                raise
            finally:
                for process in processes:
                    try:
                        os.killpg(process.pid, signal.SIGTERM)
                    except ProcessLookupError:
                        pass
                for process in processes:
                    try:
                        process.wait(timeout=10)
                    except subprocess.TimeoutExpired:
                        os.killpg(process.pid, signal.SIGKILL)
                        process.wait()
                for log in logs:
                    log.close()
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--static', action='store_true', help='Skip live DDS/Gazebo tests')
    args = parser.parse_args()
    if not args.static:
        clock_check()
        dds_check()
    static_checks()
    print('jazzy-desktop-full-validation=ok', flush=True)
