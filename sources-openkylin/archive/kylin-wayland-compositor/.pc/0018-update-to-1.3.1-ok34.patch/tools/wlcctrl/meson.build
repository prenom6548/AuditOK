wlcctrl_sources = []

wlcctrl_sources += files(
  'main.c', 'seat.c', 'output.c',
  'wlcctrl.c', 'cursor.c', 'keyboard.c',
)

protos = [
  'virtual-keyboard-unstable-v1',
  'wlr-virtual-pointer-unstable-v1',
]

foreach p : protos
  wlcctrl_sources += protocols_code[p]
  wlcctrl_sources += protocols_client_header[p]
endforeach

executable(
  'wlcctrl',
  wlcctrl_sources,
  include_directories: wlcom_inc,
  dependencies: [wayland_client, libkywc, xkbcommon, kywc_buffer, util],
  build_by_default: true,
  install: true,
)

scdoc = find_program('scdoc')
if scdoc.found()
  mandir = get_option('mandir')

  custom_target(
    'wlcctrl.1',
    input: 'wlcctrl.1.scd',
    output: 'wlcctrl.1',
    command: scdoc,
    feed: true,
    capture: true,
    install: true,
    install_dir: '@0@/man1'.format(mandir)
  )
endif
