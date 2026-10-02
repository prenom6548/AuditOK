subdir('cursors')

run_command('build.sh')
r=run_command('check.sh')
output = r.stdout().strip()
message('-------Please Check Warning Icons!!!-------')
message(output)
message('-------Please Check Warning Icons!!!-------')

icons_dir = join_paths(get_option('prefix'), 'share/icons')

icons_f = ['ukui-icon-theme-default', 'ukui-icon-theme-classical', 'ukui-icon-theme-fashion', 'ukui-icon-theme-dawnlight']

foreach icons: icons_f
  if get_option(icons)
    install_subdir(
      icons,
      install_dir: icons_dir/icons,
      strip_directory: true
    )
  endif
endforeach

#if get_option(icons_f[3])
#  custom_target('ln', '-s',icons_dir/icons_f[0] ,icons_dir/icons_f[3])
#endif

#if get_option(icons_f[4])
#  custom_target('ln', '-s',icons_dir/icons_f[1] ,icons_dir/icons_f[4])
#endif

#if get_option(icons_f[5])
#  custom_target('ln', '-s',icons_dir/icons_f[2] ,icons_dir/icons_f[5])
#endif
