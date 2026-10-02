if not add_languages('cpp', native: false, required: false)
  subdir_done()
endif

qt5 = import('qt5')
qt5dep = dependency(
  'qt5',
  modules: ['Core', 'DBus', 'Widgets', 'Gui'],
  required: false
)

if not qt5dep.found()
  subdir_done()
endif

gsettings_dep = dependency('gsettings-qt')
kysdk_ukuiwindowhelper = dependency('kysdk-ukuiwindowhelper', required: false)

build_dir = meson.current_build_dir()
mag_sources = []

qdbusxml2cpp = find_program('qdbusxml2cpp')

xml_file = {
  'org_ukui_Magnifier': 'org.ukui.Magnifier.xml',
  'com_kylin_Wlcom_Effect': 'com.kylin.Wlcom.Effect.xml',
}

foreach name, path : xml_file
  base_name = name.underscorify()
  gen_result = run_command(
    [
      qdbusxml2cpp,
      '-p', join_paths(build_dir, base_name),
      path,
      '-N',
      '-i', 'dbus_interface_date.h'
    ],
    check: true
  )
  mag_sources += join_paths(build_dir, base_name + '.cpp')
  mag_sources += qt5.preprocess(
    moc_headers: [join_paths(build_dir, base_name + '.h')]
  )
endforeach

mag_sources += qt5.preprocess(
  moc_headers: 'magnifier.h',
  qresources: 'svg.qrc',
)

cpp_args = ['-Wno-unused-parameter']
if kysdk_ukuiwindowhelper.found()
  cpp_args += '-DKYSDK_IS_SUPPORT'
endif

mag_sources += [
  'magnifier.cpp',
  'main.cpp',
]

subdir('translations')

executable('kylin-magnifier',
  sources: mag_sources,
  dependencies: [qt5dep, gsettings_dep, wayland_client, kysdk_ukuiwindowhelper],
  cpp_args: cpp_args,
  build_by_default: true,
  install: true,
)
