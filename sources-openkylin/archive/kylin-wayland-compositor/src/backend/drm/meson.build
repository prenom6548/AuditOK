wlcom_sources += files(
  'backend.c',
  'drm.c',
  'fb.c',
  'kms.c',
  'monitor.c',
  'output.c',
  'properties.c',
  'renderer.c',
  'util.c'
)

if have_drm_lease_device
  wlcom_sources += files('lease.c')
endif

subdir('shaders')

hwdata = dependency ('hwdata')

hwdata_dir = hwdata.get_variable(pkgconfig:'pkgdatadir')
pnpids_c = custom_target(
  'pnpids.c',
  output: 'pnpids.c',
  input: files(hwdata_dir / 'pnp.ids'),
  feed: true,
  capture: true,
  command: files('gen_pnpids.sh'),
)

wlcom_sources += pnpids_c
