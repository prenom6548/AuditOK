wlcom_sources += files(
  'config.c',
  'drm_syncobj.c',
  'drm_syncobj_merger.c',
  'drm_syncobj_v1.c',
  'pass.c',
  'pixel_format.c',
  'renderer.c',
  'single_pixel.c',
  'wayland_drm.c',
)

if get_option('tracy')
  wlcom_sources += files('profile.c', 'opengl/profile.c')
endif

subdir('allocator')
subdir('opengl')
subdir('pixman')
