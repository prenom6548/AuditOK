if not add_languages('cpp', native: false, required: false)
  subdir_done()
endif

qt5 = import('qt5')

qt5dep = dependency(
  'qt5',
  modules: ['Core', 'Gui', 'Widgets', 'Qml', 'Quick', 'QuickWidgets'],
  private_headers: true,
  required: false
)

if not qt5dep.found()
  subdir_done()
endif

egl = dependency('egl')
glesv2 = dependency('glesv2')
libdrm = dependency('libdrm')

rcc = find_program('rcc')
# Preprocess the .qrc file using the rcc command and generate a C++ source file
resources_file = custom_target(
  'resources',
  input : 'qml.qrc',
  output : 'resources.cpp',
  command : [rcc, '-name', 'resources', '-o', '@OUTPUT@', '@INPUT@']
)

generated_files = qt5.preprocess(
  moc_headers: ['context.h', 'widget.h', 'ThumbnailItem.h', 'ThumInfo.h'],
  dependencies: qt5dep,
)

executable(
  'kywc-gui',
  ['main.cpp', 'context.cpp', 'ui.cpp',
    'workspace.cpp', 'output.cpp', 'toplevel.cpp',
    'thumbnail.cpp', 'ThumbnailItem.cpp', 'ThumInfo.cpp',
    resources_file, generated_files
  ],
  dependencies: [qt5dep, libkywc, egl, glesv2, libdrm],
  cpp_args: ['-Wno-unused-parameter'],
  install: true
)

child_files = qt5.preprocess(
  moc_sources: ['children.cpp'],
  dependencies: qt5dep,
)

executable(
  'children',
  ['children.cpp', child_files],
  dependencies: [qt5dep],
  cpp_args: ['-Wno-unused-parameter'],
  install: false
)

thu_files = qt5.preprocess(
  moc_headers: ['context.h', 'ThumbnailItem.h', 'ThumInfo.h'],
  dependencies: qt5dep,
)

executable(
  'thumbnail',
  ['ThumMain.cpp', 'context.cpp', 'workspace.cpp',
    'output.cpp', 'toplevel.cpp', 'thumbnail.cpp',
    'ThumbnailItem.cpp', 'ThumInfo.cpp',
    resources_file, thu_files
  ],
  dependencies: [qt5dep, libkywc, egl, glesv2, libdrm],
  cpp_args: ['-Wno-unused-parameter'],
  install: true
)

blur_files = qt5.preprocess(
  moc_sources: ['ukui_blur.cpp'],
  dependencies: qt5dep,
)

executable(
  'ukui-blur',
  ['ukui_blur.cpp',
    blur_files, protocols_code['ukui-blur'],
    protocols_client_header['ukui-blur']
  ],
  dependencies: [qt5dep, wayland_client],
  cpp_args: ['-Wno-unused-parameter'],
  install: false
)

secu_files = qt5.preprocess(
  moc_sources: ['security.cpp'],
  dependencies: qt5dep,
)

executable(
  'security',
  ['security.cpp', secu_files,
    protocols_code['kywc-security-manager-v1'],
    protocols_client_header['kywc-security-manager-v1']
  ],
  dependencies: [qt5dep, wayland_client],
  cpp_args: ['-Wno-unused-parameter'],
  install: false
)

if have_ukui_background
  gbm = dependency('gbm', required: false)
  if not gbm.found()
    gbm = declare_dependency()
  endif

  background_files = qt5.preprocess(
    moc_sources: ['ukui_background.cpp'],
    dependencies: qt5dep,
  )

  executable(
    'ukui-background',
    ['ukui_background.cpp', background_files,
      protocols_code['ukui-background-v1'],
      protocols_client_header['ukui-background-v1'],
      protocols_code['linux-dmabuf-v1'],
      protocols_client_header['linux-dmabuf-v1']
    ],
    dependencies: [qt5dep, wayland_client, libdrm, gbm],
    cpp_args: ['-Wno-unused-parameter'],
    install: false
  )
endif
