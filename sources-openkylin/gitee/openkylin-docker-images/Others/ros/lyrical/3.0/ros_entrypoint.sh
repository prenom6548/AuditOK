#!/bin/sh
set -e

# One entrypoint for every Lyrical image; Dockerfiles select the expected variant.
: "${ROS_DISTRO:=lyrical}"
export ROS_DISTRO
_lyrical_variant=${ROS_IMAGE_VARIANT:-auto}
if [ "$_lyrical_variant" = auto ]; then
  if [ -d "/opt/ros-simulation/$ROS_DISTRO" ]; then
    if [ -d "/opt/ros-desktop/$ROS_DISTRO" ]; then
      _lyrical_variant=desktop_full
    else
      _lyrical_variant=simulation
    fi
  elif [ -d "/opt/ros-desktop/$ROS_DISTRO" ]; then
    _lyrical_variant=desktop
  elif [ -d "/opt/ros-perception/$ROS_DISTRO" ]; then
    _lyrical_variant=perception
  else
    _lyrical_variant=ros_base
  fi
fi
case "$_lyrical_variant" in
  ros_core|ros_base) _lyrical_prefixes='/opt/ros' ;;
  perception) _lyrical_prefixes='/opt/ros /opt/ros-perception' ;;
  desktop) _lyrical_prefixes='/opt/ros /opt/ros-desktop' ;;
  simulation) _lyrical_prefixes='/opt/ros /opt/ros-simulation' ;;
  desktop_full) _lyrical_prefixes='/opt/ros /opt/ros-desktop /opt/ros-simulation' ;;
  *) printf 'Unsupported ROS_IMAGE_VARIANT: %s\n' "$_lyrical_variant" >&2; exit 1 ;;
esac

for _lyrical_prefix in $_lyrical_prefixes; do
  _lyrical_setup="$_lyrical_prefix/$ROS_DISTRO/setup.sh"
  if [ ! -r "$_lyrical_setup" ]; then
    printf 'Missing required ROS setup for %s: %s\n' "$_lyrical_variant" "$_lyrical_setup" >&2
    exit 1
  fi
done
# The highest overlay's generated setup.sh also loads its recorded underlays.
. "$_lyrical_setup"

# Ogre Plugin_* SONAMEs are not indexed by ldconfig like lib* SONAMEs.
if [ -r /etc/ld.so.conf.d/rviz-ogre-plugins.conf ]; then
  _lyrical_ogre=
  IFS= read -r _lyrical_ogre < /etc/ld.so.conf.d/rviz-ogre-plugins.conf || [ -n "$_lyrical_ogre" ]
  if [ ! -d "$_lyrical_ogre" ]; then
    printf 'Missing Ogre plugin directory: %s\n' "$_lyrical_ogre" >&2
    exit 1
  fi
  case ":${LD_LIBRARY_PATH:-}:" in
    *:"$_lyrical_ogre":*) ;;
    *) export LD_LIBRARY_PATH="$_lyrical_ogre${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" ;;
  esac
fi
unset _lyrical_variant _lyrical_prefixes _lyrical_prefix _lyrical_setup _lyrical_ogre
exec "$@"
