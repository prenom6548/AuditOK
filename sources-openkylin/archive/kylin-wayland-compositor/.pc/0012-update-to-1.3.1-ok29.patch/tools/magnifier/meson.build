if not add_languages('cpp', native: false, required: false)
  subdir_done()
endif

qt5 = import('qt5')
qt5dep = dependency('qt5', modules: ['Core', 'DBus', 'Widgets', 'Gui'], private_headers: true, required: false)
gsettings_dep = dependency('gsettings-qt')
kysdk_ukuiwindowhelper = dependency('kysdk-ukuiwindowhelper')

subdir('translations')

build_dir = meson.current_build_dir()
mag_sources = []

qdbusxml2cpp = find_program('qdbusxml2cpp')

xml_file = {
  'org_ukui_Magnifier': 'org.ukui.Magnifier.xml',
  'com_kylin_Wlcom_Effect': 'com.kylin.Wlcom.Effect.xml',
}

foreach name, path : xml_file
  base_name = name.underscorify()
  gen_result = run_command([qdbusxml2cpp, '-p', join_paths(build_dir, base_name), path, '-N', '-i', 'dbus_interface_date.h'], check: true)
  mag_sources += qt5.preprocess(moc_headers: [join_paths(build_dir, base_name + '.h')], sources: [join_paths(build_dir, base_name + '.cpp')])
endforeach

res_processed = qt5.preprocess(
  moc_headers: 'magnifier.h',
  qresources: files('svg.qrc'),
)

mag_sources += [
  res_processed,
  'magnifier.cpp',
  'main.cpp',
]

executable('kylin-magnifier',
  sources: [mag_sources],
  dependencies: [qt5dep, gsettings_dep, wayland_client, kysdk_ukuiwindowhelper],
  build_by_default: true,
  install: true,
)
