if not add_languages('cpp', native: false, required: false)
  subdir_done()
endif

qt5 = import('qt5')
qt5dep = dependency(
  'qt5',
  modules: ['Core', 'Widgets', 'Gui', 'Network', 'Quick', 'QuickControls2', 'Qml'],
  private_headers: true,
  required: false,
)

protocols_client = [
  protocols_code['xdg-dialog'],
  protocols_code['xdg-foreign-unstable-v2'],
  protocols_client_header['xdg-foreign-unstable-v2'],
  protocols_client_header['xdg-dialog'],
  protocols_code['xdg-shell'],
  protocols_client_header['xdg-shell'],
]

moc_sources = qt5.preprocess(
  moc_headers: [
    'DynamicModel.h',
    'Gsettings.h',
    'MainWindow.h',
    'WaylandClient.h',
    'XdgDialog.h',
    'XdgForeign.h',
  ],
  qresources: ['Resources.qrc'],
)

dialog_sources = []
dialog_sources += protocols_client
dialog_sources += moc_sources
dialog_sources += files(
  'DynamicModel.cpp',
  'Gsettings.cpp',
  'MainWindow.cpp',
  'WaylandClient.cpp',
  'XdgDialog.cpp',
  'XdgForeign.cpp',
  'Main.cpp',
)

cpp_args = ['-Wno-unused-parameter']
if kysdk_ukuiwindowhelper.found()
  cpp_args += '-DKYSDK_IS_SUPPORT'
endif

executable(
  'wldialog',
  sources: dialog_sources,
  dependencies: [qt5dep, wayland_client, gsettings_dep, kysdk_ukuiwindowhelper, util],
  cpp_args: cpp_args,
  include_directories: wlcom_inc,
  build_by_default: true,
  install: true,
)
