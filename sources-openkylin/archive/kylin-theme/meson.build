project('openkylin-theme','c',
         default_options: ['prefix=/usr/'])

components_d = ['themes', 'icons', 'sounds', 'highlight']

foreach component: components_d
  if get_option(component)
    subdir(component)
  endif
endforeach

data_d = ['fcitx-qimpanel','grub','mime','plymouth','initramfs-tools']

foreach data: data_d
  if get_option(data)
    install_subdir(
      'data',
      install_dir: '/usr/share',
      strip_directory: true
    )
  endif
endforeach

configrue_d = ['glib', 'openkylin-grub-theme']

if get_option(configrue_d[0])
  install_subdir(
    'configrue/glib',
    install_dir: '/usr/share/',
    strip_directory: true
  )
endif

if get_option(configrue_d[1])
  install_subdir(
    'configrue/openkylin-grub-theme',
    install_dir: '/etc/grub.d/',
    strip_directory: true
  )
endif
