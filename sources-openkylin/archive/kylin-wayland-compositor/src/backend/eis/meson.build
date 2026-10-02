eis = dependency('libeis-1.0')
if not eis.found()
  subdir_done()
endif

wlcom_deps += eis

wlcom_sources += files(
  'backend.c',
  'client.c',
  'context.c',
  'event.c',
  'filter.c',
)
