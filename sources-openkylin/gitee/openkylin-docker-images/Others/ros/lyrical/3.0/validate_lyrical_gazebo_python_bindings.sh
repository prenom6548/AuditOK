#!/usr/bin/env bash
# Run upstream tests against the installed bindings. Keep host binfmt unchanged.
set -eo pipefail
. /opt/ros-simulation/lyrical/setup.sh
export LD_BIND_NOW=1
export PYTHONPATH="/opt/gazebo-python/lib/python:/opt/ros-simulation/lyrical/opt/gz_msgs_vendor/lib64/python:${PYTHONPATH:-}"
export GZ_IP=127.0.0.1 GZ_TRANSPORT_IMPLEMENTATION=zeromq
python_runner=(/usr/bin/python3)
case "$(dpkg --print-architecture)" in
  amd64) ;;
  arm64) python_runner=(/checks/emulators/qemu-aarch64 /usr/bin/python3 /usr/bin/python3) ;;
  riscv64) python_runner=(/checks/emulators/qemu-riscv64 /usr/bin/python3 /usr/bin/python3) ;;
  *) echo 'unsupported architecture' >&2; exit 2 ;;
esac
"${python_runner[@]}" /tmp/gz-python/math/src/python_pybind11/test/Vector3_TEST.py
"${python_runner[@]}" /tmp/gz-python/sdformat/python/test/pyBox_TEST.py
"${python_runner[@]}" /tmp/gz-python/transport/python/test/pubSub_TEST.py PubSubTEST.test_msg_callback PubSubTEST.test_publish_msg
"${python_runner[@]}" /tmp/gz-python/sim/python/test/testFixture_TEST.py
echo 'gazebo-python-bindings-runtime=ok'
