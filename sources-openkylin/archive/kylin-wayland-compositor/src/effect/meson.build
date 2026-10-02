wlcom_sources += files(
  'action.c',
  'animator.c',
  'blur.c',
  'blurrer.c',
  'capture.c',
  'circle_progressbar.c',
  'color_filter.c',
  'color_picker.c',
  'config.c',
  'effect.c',
  'fade.c',
  'locate_pointer.c',
  'magic_lamp.c',
  'mouse_click.c',
  'mouse_trail.c',
  'move.c',
  'output_transform.c',
  'showfps.c',
  'showkey.c',
  'scale.c',
  'shake_cursor.c',
  'shake_view.c',
  'snapshot.c',
  'stream.c',
  'tap_ripple.c',
  'touch_click.c',
  'touch_long.c',
  'touch_trail.c',
  'trail.c',
  'translation.c',
  'transform.c',
  'wallpaper.c',
  'wlr_screencopy.c',
  'zoom.c',
)

wlcom_sources += files(
  'ky_capture.c',
  'ky_stream.c',
)

if have_ukui_screenshot
  wlcom_sources += files(
    'screenshot.c',
  )
endif

if have_ukui_watermark
  wlcom_sources += files(
    'watermark.c',
  )
endif

if have_kde_slide
  wlcom_sources += files(
    'slide.c',
  )
endif

if have_ukui_effect
  wlcom_sources += files(
    'ukui_effect.c',
  )
endif

subdir('shaders')
