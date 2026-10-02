wlcom_sources += files(
  'action.c',
  'binding.c',
  'config.c',
  'cursor.c',
  'data_device.c',
  'event.c',
  'function.c',
  'gesture.c',
  'global_shortcuts.c',
  'global_gestures.c',
  'idle.c',
  'idle_inhibit.c',
  'input.c',
  'text_input.c',
  'keyboard.c',
  'keyboard_group.c',
  'libinput.c',
  'monitor.c',
  'primary_selection.c',
  'seat.c',
  'selection.c',
  'tablet.c',
  'text_input_v1.c',
  'text_input_v2.c',
  'text_input_v3.c',
  'toplevel_drag.c',
  'touch.c',
  'transient_seat.c',
  'wlr_data_control.c',
)

if have_kde_keystate
  wlcom_sources += files(
    'kde_keystate.c',
  )
endif

if have_kde_clipboard
  wlcom_sources += files(
    'kde_clipboard.c',
  )
endif
