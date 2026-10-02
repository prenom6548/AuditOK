sass = find_program('sass')
custom_target(
  'ukui-black-gtk-3',
  input: 'gtk.scss',
  output: 'gtk.css',
  command: [sass, '--style', 'nested','@INPUT@' ,'@OUTPUT@'],
  install: true,
  install_dir: white_dir/'gtk-3.0'
)

install_subdir(
  'assets',
  install_dir: white_dir/'gtk-3.0/assets',
  strip_directory: true
)
