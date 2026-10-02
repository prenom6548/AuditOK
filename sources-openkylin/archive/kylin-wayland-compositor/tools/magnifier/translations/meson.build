ts_files = [
  'de_DE.ts',
  'bo_CN.ts',
  'es_ES.ts',
  'kk.ts',
  'mn.ts',
  'zh_CN.ts',
  'zh_TW.ts',
  'fr_FR.ts',
  'ky.ts',
  'ug.ts',
  'zh_HK.ts',
  'ar.ts',
  'vi.ts',
  'ms.ts',
  'th.ts',
  'zh_Hant.ts',
]

translations = qt5.compile_translations(
  ts_files: ts_files,
  build_by_default: true,
)

fs = import('fs')

qm_files_xml = ''
foreach qm_target : translations
  qm_file = qm_target.full_path()
  qm_files_xml += '        <file>' + fs.name(qm_file) + '</file>\n'
endforeach

translations_qrc = configure_file(
  input: 'translations.qrc.in',
  output: 'translations.qrc',
  configuration: {'QM_FILES': qm_files_xml},
)

mag_sources += qt5.preprocess(
  qresources: translations_qrc,
)
