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
  man_files = [
    'wlcctrl.1.scd',
  ]
  foreach filename : man_files
    topic = filename.split('.')[-3].split('/')[-1]
    section = filename.split('.')[-2]
    output = '@0@.@1@'.format(topic, section)

    custom_target(
      output,
      input: filename,
      output: output,
      command: scdoc,
      feed: true,
      capture: true,
      install: true,
      install_dir: '@0@/man@1@'.format(mandir, section)
    )
  endforeach
endif
