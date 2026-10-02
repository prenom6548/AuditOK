util_sources = files(
  'file.c',
  'hash_table.c',
  'identifier.c',
  'limit.c',
  'logger.c',
  'queue.c',
  'spawn.c',
  'string.c',
  'sysfs.c',
  'time.c',
)

util_static = static_library(
  'util_static',
  util_sources,
  include_directories: wlcom_inc,
  c_args: '-DUTIL_USED_IN_CLIENT',
  build_by_default : false,
)

util = declare_dependency(
  link_with: util_static,
)

crypto = dependency('openssl')
wlcom_deps += crypto

wlcom_sources += util_sources
wlcom_sources += files(
  'dbus.c',
  'debug.c',
  'matrix.c',
  'quirks.c',
  'region.c',
  'vec2.c',
  'wayland.c',
)
