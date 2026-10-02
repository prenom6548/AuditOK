#!/usr/bin/env python3
"""Verify a running Harmonic physics server and real Gazebo -> ROS clock traffic."""
import os
from pathlib import Path
import signal
import sys
import subprocess
import tempfile
import time
import uuid

from ament_index_python.packages import get_package_prefix
import rclpy
from rclpy.qos import qos_profile_sensor_data
from rosgraph_msgs.msg import Clock
from sensor_msgs.msg import Image


def main():
    camera = sys.argv[1:] == ["--camera"]
    if sys.argv[1:] and not camera:
        raise SystemExit("usage: validate_jazzy_simulation_clock.py [--camera]")
    # Isolate Gazebo discovery from other tests and worlds on this host.
    os.environ["GZ_PARTITION"] = "jazzy_validation_" + uuid.uuid4().hex
    processes = []
    handles = []
    stamps = []
    frames = []
    topic = "/jazzy_test/image" if camera else "/world/jazzy_validation/clock"
    rclpy.init()
    node = rclpy.create_node("jazzy_clock_validation")
    subscription = node.create_subscription(
        Image if camera else Clock, topic,
        (lambda msg: frames.append(msg)) if camera else
        (lambda msg: stamps.append(msg.clock.sec * 1_000_000_000 + msg.clock.nanosec)),
        qos_profile_sensor_data,
    )
    with tempfile.TemporaryDirectory(prefix="jazzy-simulation-") as directory:
        work = Path(directory)
        world = work / "world.sdf"
        world.write_text('''<?xml version="1.0"?>
<sdf version="1.10"><world name="jazzy_validation">
  <physics name="physics" type="ignored">
    <max_step_size>0.001</max_step_size><real_time_factor>1</real_time_factor>
  </physics>
  <plugin filename="gz-sim-physics-system" name="gz::sim::systems::Physics"/>
  <model name="falling_box"><pose>0 0 10 0 0 0</pose><link name="body">
    <inertial><mass>1</mass><inertia><ixx>0.1</ixx><iyy>0.1</iyy><izz>0.1</izz></inertia></inertial>
    <collision name="box"><geometry><box><size>1 1 1</size></box></geometry></collision>
  </link></model>
</world></sdf>
''', encoding="utf-8")
        if camera:
            scene = Path('/opt/jazzy-simulation/worlds/demo.sdf').read_text()
            scene = scene.replace('</world>', '''
<model name="camera"><static>true</static><pose>-4 0 3 0 0.5 0</pose><link name="body">
  <sensor name="camera" type="camera"><always_on>true</always_on><update_rate>5</update_rate>
    <topic>/jazzy_test/image</topic><camera><horizontal_fov>1.047</horizontal_fov>
      <image><width>160</width><height>120</height><format>R8G8B8</format></image>
      <clip><near>0.1</near><far>50</far></clip>
    </camera>
  </sensor>
</link></model></world>''')
            world.write_text(scene, encoding='utf-8')
        bridge = str(Path(get_package_prefix("ros_gz_bridge")) / "lib/ros_gz_bridge/parameter_bridge")
        commands = (
            ("bridge", [bridge, topic + "@rosgraph_msgs/msg/Clock[gz.msgs.Clock"]),
            ("simulation", ["gz", "sim", "-s", "-r", "-v", "4", str(world)]),
        )
        if camera:
            bridge = str(Path(get_package_prefix('ros_gz_image')) / 'lib/ros_gz_image/image_bridge')
            commands = (
                ('image_bridge', [bridge, topic]),
                ('simulation', ['gz', 'sim', '-s', '-r', '--headless-rendering', '-v', '4', str(world)]),
            )
        try:
            for name, command in commands:
                handle = (work / (name + ".log")).open("w", encoding="utf-8")
                handles.append(handle)
                processes.append(subprocess.Popen(
                    command, stdout=handle, stderr=subprocess.STDOUT, start_new_session=True,
                ))
            deadline = time.monotonic() + float(os.environ.get("SIMULATION_TEST_TIMEOUT", "180"))
            while time.monotonic() < deadline:
                rclpy.spin_once(node, timeout_sec=0.5)
                if any(process.poll() is not None for process in processes):
                    raise RuntimeError("Gazebo or bridge exited before verification")
                if camera and len(frames) >= 3 and len(set(frames[-1].data)) > 4:
                    msg = frames[-1]
                    if (msg.width, msg.height, msg.encoding) != (160, 120, 'rgb8') or len(msg.data) != msg.step * msg.height:
                        raise RuntimeError('unexpected camera image shape/encoding')
                    print(f'gazebo-to-ros-image=ok frames={len(frames)} width={msg.width} height={msg.height} encoding={msg.encoding} distinct_bytes={len(set(msg.data))}')
                    break
                if len(stamps) >= 5 and stamps[-1] > stamps[0] >= 0:
                    if any(right < left for left, right in zip(stamps, stamps[1:])):
                        raise RuntimeError("simulation clock moved backwards")
                    for log in work.glob("*.log"):
                        if "[Err]" in log.read_text(errors="replace"):
                            raise RuntimeError(f"Gazebo reported an error: {log.name}")
                    print(f"gazebo-to-ros-clock=ok messages={len(stamps)} first_ns={stamps[0]} last_ns={stamps[-1]}")
                    break
            else:
                raise RuntimeError(f"no valid camera image; frames={len(frames)}" if camera else f"no advancing simulation clock; samples={stamps}")
        except Exception:
            for log in work.glob("*.log"):
                print(f"{log.name}:\n{log.read_text(errors='replace')}")
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
            for handle in handles:
                handle.close()
            node.destroy_subscription(subscription)
            node.destroy_node()
            rclpy.shutdown()


if __name__ == "__main__":
    main()
