fs = import('fs')

languages = []
foreach line : fs.read(meson.current_source_dir() / 'LINGUAS').strip().split('\n')
  if line != ''
    languages += line
  endif
endforeach

locales_table = ''
mo_h_files = []
foreach lang : languages
  mo = custom_target(
    lang + '.mo',
    input: lang + '.po',
    output: lang + '.mo',
    command: [msgfmt, '-o', '@OUTPUT@', '@INPUT@'],
  )

  mo_h_files += custom_target(
    lang + '_mo.h',
    input: mo,
    output: lang + '_mo.h',
    command: [embed, lang + '_mo'],
    feed: true,
    capture: true,
  )

  locales_table += '#include "@0@_mo.h"\n'.format(lang)
endforeach

locales_table += '''
static const struct locale locales[] = {
'''
foreach lang : languages
  locales_table += '    { "@0@", @0@_mo, sizeof(@0@_mo) - 1 },\n'.format(lang)
endforeach
locales_table += '};'

locales_h = configure_file(
  input: 'locales.h.in',
  output: 'locales.h',
  configuration: {'LOCALES': locales_table},
)

wlcom_sources += mo_h_files
wlcom_sources += locales_h
