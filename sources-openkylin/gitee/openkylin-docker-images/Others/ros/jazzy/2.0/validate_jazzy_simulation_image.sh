#!/bin/bash
set -Eeo pipefail
trap 'rc=$?; printf "Validation failed at line %s: %s (exit %s)\n" "$LINENO" "$BASH_COMMAND" "$rc" >&2' ERR

mode="${1:---runtime}"
case "${mode}" in --build|--runtime|--gui) ;; *) echo 'usage: validate_jazzy_simulation_image.sh [--build|--runtime|--gui]' >&2; exit 2 ;; esac
test "${ROS_DISTRO}" = jazzy
source /opt/ros/jazzy/setup.bash
source /opt/ros-simulation/jazzy/setup.bash

packages="$(ros2 pkg list)"
for package in ros_base simulation ros_gz_bridge ros_gz_sim ros_gz_image \
    ros_gz_interfaces gz_sim_vendor gz_gui_vendor gz_rendering_vendor \
    gz_sensors_vendor gz_physics_vendor gz_dartsim_vendor gz_ogre_next_vendor; do
  grep -qx "${package}" <<<"${packages}" || { echo "Missing ROS package: ${package}" >&2; exit 1; }
done
while IFS= read -r package; do
  # The closure also contains plain CMake packages such as urdfdom. They
  # install colcon dependency markers without registering an ament ROS index.
  test -f "/opt/ros-simulation/jazzy/share/colcon-core/packages/${package}" || {
    echo "Missing installed closure package: ${package}" >&2; exit 1;
  }
done < /opt/jazzy-build-info/simulation-packages.txt
python3 -c 'import rclpy, PyKDL; from rosgraph_msgs.msg import Clock; from sensor_msgs.msg import Image'
python3 -c 'from gz.transport13 import Node; import gz.math7; import sdformat14; import gz.sim8'
versions="$(gz sim --versions)"
grep -Eq '(^|[^0-9])8\.[0-9]+' <<<"${versions}"

# A successful vendor build can still omit optional physics/rendering plugins.
# Require the full backends here instead of declaring such an image complete.
prefix=/opt/ros-simulation/jazzy
for pattern in 'libgz-physics-dartsim-plugin.so*' \
    'libgz-physics-bullet-plugin.so*' 'libgz-physics-bullet-featherstone-plugin.so*' \
    'libgz-rendering8-ogre.so*' 'libgz-rendering8-ogre2.so*' 'libgz-gui8.so*'; do
  match="$(find "${prefix}" -name "${pattern}" -print -quit)"
  test -n "${match}" || { echo "missing required plugin: ${pattern}" >&2; exit 1; }
done

while IFS= read -r -d '' library; do
  output="$(ldd "${library}" 2>&1 || true)"
  if grep -q 'not found' <<<"${output}"; then
    printf 'Unresolved shared library: %s\n%s\n' "${library}" "${output}" >&2
    exit 1
  fi
done < <(find /opt/ros/jazzy "${prefix}" /opt/assimp /opt/simulation-deps \
    /opt/poco /opt/ffmpeg /usr/local/lib -type f -name '*.so*' -print0)

echo "jazzy-simulation-build-check=ok gazebo=${versions}"
if [ "${mode}" = --runtime ]; then
  python3 /opt/jazzy-simulation/tests/validate_jazzy_simulation_clock.py
  echo 'jazzy-simulation-runtime-check=ok (headless physics and Gazebo-to-ROS clock)'
fi
if [ "${mode}" = --gui ]; then
  # A window that stays alive can still display blank geometry. Verify real
  # OGRE2 camera pixels crossing ros_gz_image before the GUI lifetime check.
  python3 /opt/jazzy-simulation/tests/validate_jazzy_simulation_clock.py --camera
  gui_log="$(mktemp /tmp/jazzy-gui-XXXXXX.log)"
  result=0
  timeout --signal=INT --kill-after=10s 45s \
    xvfb-run -a -s '-screen 0 1280x800x24' \
    gz sim -r -v 4 --render-engine-gui "${SIMULATION_GUI_RENDER_ENGINE:-ogre}" \
    /opt/jazzy-simulation/worlds/demo.sdf >"${gui_log}" 2>&1 || result=$?
  cat "${gui_log}"
  # gz's Ruby launcher can return zero when its GUI child crashes. Only a
  # sustained run ending at our timeout is accepted, and crash logs fail too.
  test "${result}" -eq 124
  if grep -Eq '\[Err\]|Segmentation fault|LLVM ERROR|terminate called|Aborted' "${gui_log}"; then
    echo "GUI reported an error; log: ${gui_log}" >&2
    exit 1
  fi
  grep -Eiq 'ogre' "${gui_log}"
  echo 'jazzy-simulation-gui-check=ok (45s Xvfb, Qt/OGRE GUI and OGRE2 camera)'
fi
