black_dir = join_paths(get_option('prefix'), 'share/themes/ukui-black')

conf_data = configuration_data()
configure_file(input : 'index.theme.in',
               output : 'index.theme',
               configuration : conf_data,
               install_dir: black_dir)

install_subdir(
  'gtk-2.0',
  install_dir: black_dir/'gtk-2.0',
  strip_directory: true
)

subdir('gtk-3.0')
