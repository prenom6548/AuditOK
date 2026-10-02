run_command('cursors.sh')

cursors_dir = join_paths(get_option('prefix'), 'share/icons/dark-sense')

install_subdir(
  'cursors',
  install_dir: cursors_dir/'cursors',
  strip_directory: true
)

cursors_f = [
  'cursor.theme',
  'index.theme',
]
conf_data = configuration_data()
foreach cursors: cursors_f
  configure_file(input : cursors,
                 output : cursors,
                 configuration : conf_data,
                 install_dir: cursors_dir)
endforeach

