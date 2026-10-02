TEMPLATE = subdirs

SUBDIRS = unit_test_manager_uniauth \
          unit_test_manager_service \
          unit_test_plugin_service \
          unit_test_plugin_uniauth \
          unit_test_manager_configuration \
          unit_test_manager_treeitem \
          unit_test_manager_treemodel \
          unit_test_plugin_pwdutil \
          unit_test_plugin_bioproxy \
          unit_test_manager_treeview \
          unit_test_plugin_enrollbtn \
          unit_test_manager_kalbel \
          unit_test_manager_kbutton \
          unit_test_plugin_klabel


# 打桩工具
# Define paths
TEST_UTILS_PATH = $$PWD/kt-test-utils

# Header files (if needed)
HEADERS += $$files($$TEST_UTILS_PATH/cpp-stub/*.h $$TEST_UTILS_PATH/cpp-stub/*.hpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.h)

# Gather source files
SOURCES += $$files($$TEST_UTILS_PATH/cpp-stub/*.cpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.cpp)

