#!/usr/bin/env bash
set -eo pipefail
. /opt/ros-simulation/lyrical/setup.sh
export CMAKE_PREFIX_PATH="/opt/ros-python/pybind11:/opt/poco:/opt/simulation-deps:/opt/assimp:${CMAKE_PREFIX_PATH:-}"
for prefix in /opt/ros-desktop/lyrical/opt/* /opt/ros-simulation/lyrical/opt/*; do
  [ ! -d "$prefix" ] || export CMAKE_PREFIX_PATH="$prefix:$CMAKE_PREFIX_PATH"
done
export LD_BIND_NOW=1
export PYTHONPATH="/opt/gazebo-python/lib/python:${PYTHONPATH:-}"
package="$1"
if [ "$package" = check ]; then
  export GZ_IP=127.0.0.1 GZ_TRANSPORT_IMPLEMENTATION=zeromq
  python3 /tmp/gz-python/math/src/python_pybind11/test/Vector3_TEST.py
  python3 /tmp/gz-python/sdformat/python/test/pyBox_TEST.py
  python3 /tmp/gz-python/transport/python/test/pubSub_TEST.py PubSubTEST.test_msg_callback PubSubTEST.test_publish_msg
  python3 /tmp/gz-python/sim/python/test/testFixture_TEST.py
  exit 0
fi
case "$package" in
 math) source_path=/tmp/gz-python/math/src/python_pybind11 ;;
 transport) source_path=/tmp/gz-python/transport/python ;;
 sdformat) source_path=/tmp/gz-python/sdformat/python ;;
 sim) source_path=/tmp/gz-python/sim ;;
 *) exit 2 ;;
esac
cmake -S "$source_path" -B "/tmp/gz-python/build-$package"  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF  -DCMAKE_INSTALL_PREFIX=/opt/gazebo-python -DCMAKE_INSTALL_LIBDIR=lib  -DCMAKE_CXX_STANDARD=17 -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF  -Dpybind11_DIR=/opt/ros-python/pybind11/share/cmake/pybind11  -DPython3_EXECUTABLE=/usr/bin/python3 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build "/tmp/gz-python/build-$package" --parallel "${BUILD_JOBS:-4}"
cmake --install "/tmp/gz-python/build-$package"
