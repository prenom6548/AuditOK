TEMPLATE = subdirs

SUBDIRS = unit_test_keyboard \
          unit_test_mouse \
          unit_test_about \
          unit_test_autoboot \
          unit_test_datetime \
          unit_test_default \
          unit_test_area \
          unit_test_screenlock \
          unit_test_screensaver \
          unit_test_shortcut \
          unit_test_wallpaper \
          unit_test_font \
          unit_test_vino \
          unit_test_theme \
	  unit_test_accessibility \
          unit_test_boot \

# 打桩工具
# Define paths
TEST_UTILS_PATH = $$PWD/kt-test-utils

# Header files (if needed)
HEADERS += $$files($$TEST_UTILS_PATH/cpp-stub/*.h $$TEST_UTILS_PATH/cpp-stub/*.hpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.h)

# Gather source files
SOURCES += $$files($$TEST_UTILS_PATH/cpp-stub/*.cpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.cpp)

