wayland_client = dependency('wayland-client')

subdir('lib')

examples = {}

if have_kde_output
  examples += {
    'output-doctor': {
    'src': 'output_doctor.c',
    'proto': ['kde-output-management-v2', 'kde-output-device-v2',
              'kde-primary-output-v1', 'org-kde-kwin-dpms-manager',
              'xdg-output-unstable-v1'],
    },
  }
endif

if have_kde_virtual_desktop
  examples += {
    'virtual-desktop': {
    'src': 'virtual_desktop.c',
    'proto': ['org-kde-plasma-virtual-desktop-management'],
    },
  }
endif

examples += {
  'monitor': {
  'src': 'monitor.c',
  'dep': libkywc,
  },
}

if have_ukui_effect
examples += {
  'ukui-effect-test': {
  'src': 'ukui-effect-test.c',
  'proto': ['ukui-effect-v1', 'xdg-shell', 'xdg-decoration'],
  'dep': [rt],
  },
}
endif

examples += {
  'thumbnail': {
  'src': 'thumbnail.c',
  'dep': [libkywc, kywc_buffer, libwlu],
  },
}

if have_kde_keystate
  examples += {
    'kde_keystate': {
    'src': 'kde_keystate.c',
    'proto': ['org-kde-kwin-keystate-manager'],
    },
  }
endif

if have_alpha_modifier
  examples += {
    'alpha_modifier': {
    'src': 'alpha_modifier.c',
    'proto': ['alpha-modifier-v1'],
    'dep': [libwlu, cairo],
    },
  }
endif

examples += {
  'pipewire': {
  'src': 'pipewire.c',
  'dep': [libkywc, kywc_buffer, pipewire],
  },
}

foreach name, info : examples
  extra_src = []
  foreach p : info.get('proto', [])
    extra_src += protocols_code[p]
    extra_src += protocols_client_header[p]
  endforeach

  executable(
    name,
    [info.get('src'), extra_src],
    dependencies: [wayland_client, info.get('dep', [])],
    include_directories : wlcom_inc,
    build_by_default: true,
  )
endforeach

subdir('gui')

shared_module(
  'simple',
  'simple_plugin.c',
  name_prefix: '',

  include_directories : wlcom_inc,
  install_dir: plugindir,
  install: false
)
