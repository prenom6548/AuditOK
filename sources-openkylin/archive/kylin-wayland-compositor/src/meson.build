wlcom_sources += files(
  'main.c',
  'server.c',
)

# util first
subdir('util')

subdir('backend')
subdir('config')
subdir('effect')
subdir('input')
subdir('output')
subdir('painter')
subdir('plugin')
subdir('render')
subdir('scene')
subdir('security')
subdir('theme')
subdir('view')
subdir('widget')

if have_xwayland
  subdir('xwayland')
endif
