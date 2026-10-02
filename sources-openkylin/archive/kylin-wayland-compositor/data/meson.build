install_data(
  'udev-rules/90-framebuffer.rules',
  install_dir: '/usr/lib/udev/rules.d'
)

install_data(
  'kylin-wlcom.desktop',
  install_dir: '/usr/share/wayland-sessions'
)

install_data(
  ['kylin-wlcom.service', 'kylin-wlcom.target'],
  install_dir: '/usr/lib/systemd/user'
)

install_data(
  'config.json',
  install_dir: '/etc/kylin-wlcom/'
)

embed = executable('embed', 'embed.c')

executable(
  'kylin-wlcom-wrapper',
  'wrapper.c',
  dependencies: sdbus,
  build_by_default: true,
  install: true,
)

scdoc = find_program('scdoc')
if scdoc.found()
  mandir = get_option('mandir')

  custom_target(
    'kylin-wlcom.1',
    input: 'kylin-wlcom.1.scd',
    output: 'kylin-wlcom.1',
    command: scdoc,
    feed: true,
    capture: true,
    install: true,
    install_dir: '@0@/man1'.format(mandir)
  )
  custom_target(
    'kylin-wlcom-wrapper.1',
    input: 'kylin-wlcom-wrapper.1.scd',
    output: 'kylin-wlcom-wrapper.1',
    command: scdoc,
    feed: true,
    capture: true,
    install: true,
    install_dir: '@0@/man1'.format(mandir)
  )
endif
