#install_subdir(
 # 'ukui-xunguang',
  #install_dir: '/usr/share/sounds/ukui-xunguang',
  #strip_directory: true
#)

sounds_dir= join_paths(get_option('prefix'), 'share/sounds')

sounds_f=['Light-Seeking','HeYin','Classic']

foreach sounds: sounds_f
  if get_option(sounds)
    install_subdir(
      sounds,
      install_dir: sounds_dir/sounds,
      strip_directory: true
    )
  endif
endforeach
