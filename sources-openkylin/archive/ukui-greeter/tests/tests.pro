TEMPLATE = subdirs

SUBDIRS = unit_test_service_manager \
          unit_test_security_user   \
          unit_test_uniauth_service \
          unit_test_common \
          unit_test_inputswitch_event \
          unit_test_monitor_watcher \
          unit_test_touch_screen \
          unit_test_auto_screen \
          unit_test_display_service \
          unit_test_engine_device \
          unit_test_biometric-proxy \
#          unit_test_service_interface \
          unit_test_pam_tally \
          unit_test_greeter_wrapper \
          unit_test_plugins_loader \
          unit_test_screen_model \
          unit_test_greeter_service \
          unit_test_user_model \
          unit_test_lock_info \
          unit_test_ktooltip

# 打桩工具
# Define paths
TEST_UTILS_PATH = $$PWD/kt-test-utils

# Header files (if needed)
HEADERS += $$files($$TEST_UTILS_PATH/cpp-stub/*.h $$TEST_UTILS_PATH/cpp-stub/*.hpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.h)

# Gather source files
SOURCES += $$files($$TEST_UTILS_PATH/cpp-stub/*.cpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.cpp)

