r = run_command('arch')
arch = r.stdout().strip()
pkg = import('pkgconfig')

gtk3_dep = dependency('gtk+-3.0')
glib2_dep = dependency('glib-2.0')

gtkmodule_dir = join_paths('/usr/lib/'+ arch +'-linux-gnu/gtk-3.0/modules')

lib = shared_module(
  'ukuireload-gtk-module',
  sources:'ukuireload.c',
  dependencies: [gtk3_dep,glib2_dep],
  install: true,
  install_dir: gtkmodule_dir
)

pkg.generate(lib)
