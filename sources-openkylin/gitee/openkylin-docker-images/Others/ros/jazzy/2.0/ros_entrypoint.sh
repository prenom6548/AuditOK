#!/bin/bash
set -e

# Shared by all Jazzy recipes. Dockerfiles select their installation layout.
: "${ROS_DISTRO:=jazzy}"
export ROS_DISTRO
_jazzy_variant=${ROS_IMAGE_VARIANT:-auto}
if [ "$_jazzy_variant" = auto ]; then
  if [ -f "/opt/ros/$ROS_DISTRO/share/ament_index/resource_index/packages/desktop_full" ]; then
    _jazzy_variant=desktop_full
  elif [ -d "/opt/ros-simulation/$ROS_DISTRO" ]; then
    _jazzy_variant=simulation
  elif [ -d "/opt/ros-desktop/$ROS_DISTRO" ]; then
    _jazzy_variant=desktop
  elif [ -d "/opt/ros-perception/$ROS_DISTRO" ]; then
    _jazzy_variant=perception
  else
    _jazzy_variant=ros_base
  fi
fi
_jazzy_setup_name=setup.sh
case "$_jazzy_variant" in
  ros_core|ros_base) _jazzy_prefixes='/opt/ros' ;;
  perception) _jazzy_prefixes='/opt/ros /opt/ros-perception' ;;
  desktop) _jazzy_prefixes='/opt/ros /opt/ros-perception /opt/ros-desktop' ;;
  simulation) _jazzy_prefixes='/opt/ros /opt/ros-simulation'; _jazzy_setup_name=setup.bash ;;
  desktop_full) _jazzy_prefixes='/opt/ros'; _jazzy_setup_name=setup.bash ;;
  *) printf 'Unsupported ROS_IMAGE_VARIANT: %s\n' "$_jazzy_variant" >&2; exit 1 ;;
esac
for _jazzy_prefix in $_jazzy_prefixes; do
  _jazzy_setup="$_jazzy_prefix/$ROS_DISTRO/$_jazzy_setup_name"
  if [ ! -r "$_jazzy_setup" ]; then
    printf 'Missing required ROS setup for %s: %s\n' "$_jazzy_variant" "$_jazzy_setup" >&2
    exit 1
  fi
  source "$_jazzy_setup"
done

if [ "$_jazzy_variant" = desktop_full ]; then
  export LD_LIBRARY_PATH="/opt/ros/$ROS_DISTRO/opt/rviz_ogre_vendor/lib/OGRE${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
fi

if [ "$_jazzy_variant" = simulation ]; then
  # Gazebo installs Python bindings under GNUInstallDirs lib64/python here,
  # while some upstream vendor hooks cover only lib/python.
  for vendor_python in /opt/ros-simulation/${ROS_DISTRO}/opt/*/lib/python \
      /opt/ros-simulation/${ROS_DISTRO}/opt/*/lib64/python \
      /opt/ros-simulation/${ROS_DISTRO}/opt/*/lib/python3*/site-packages \
      /opt/ros-simulation/${ROS_DISTRO}/opt/*/lib64/python3*/site-packages; do
    if [ -d "${vendor_python}" ]; then
      export PYTHONPATH="${vendor_python}${PYTHONPATH:+:${PYTHONPATH}}"
    fi
  done
  unset vendor_python

  # openKylin's Mesa/LLVM 22 llvmpipe can crash or produce blank geometry.
  # Use the verified non-LLVM path only when software rendering is requested.
  if [ "${LIBGL_ALWAYS_SOFTWARE:-}" = 1 ] && [ "$(uname -m)" = x86_64 ]; then
    export GALLIUM_DRIVER="${GALLIUM_DRIVER:-softpipe}"
    # OGRE2 offscreen cameras work, but its Qt texture sharing is unreliable
    # with Mesa software rendering. Use OGRE for a fresh GUI configuration;
    # preserve existing user configurations and explicit CLI engine choices.
    software_gui_config="${HOME}/.gz/sim/8/gui.config"
    if [ ! -e "${software_gui_config}" ]; then
      mkdir -p "$(dirname "${software_gui_config}")"
      sed 's#<engine>ogre2</engine>#<engine>ogre</engine>#' \
        /opt/ros-simulation/${ROS_DISTRO}/opt/gz_sim_vendor/share/gz/gz-sim8/gui/gui.config \
        > "${software_gui_config}"
    fi
    unset software_gui_config
  fi

  # Keep SDK prefixes available when building user simulation plugins.
  export CMAKE_PREFIX_PATH="/opt/jazzy-tools/lib/python3.12/site-packages/pybind11:/opt/ffmpeg:/opt/poco:/opt/assimp:/opt/simulation-deps:${CMAKE_PREFIX_PATH:-}"
  export PKG_CONFIG_PATH="/opt/ffmpeg/lib/pkgconfig:/opt/assimp/lib/pkgconfig:/opt/simulation-deps/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
  export LD_LIBRARY_PATH="/opt/ffmpeg/lib:/opt/assimp/lib:/opt/simulation-deps/lib:/opt/simulation-deps/lib64:/opt/poco/lib:${LD_LIBRARY_PATH:-}"

  if [ "$#" -eq 0 ]; then set -- bash; fi
fi

unset _jazzy_variant _jazzy_prefixes _jazzy_prefix _jazzy_setup _jazzy_setup_name
exec "$@"
