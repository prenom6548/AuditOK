TEMPLATE = subdirs

SUBDIRS = unit_test_common \
          unit_test_log4qt


# 打桩工具
# Define paths
TEST_UTILS_PATH = $$PWD/kt-test-utils

# Header files (if needed)
HEADERS += $$files($$TEST_UTILS_PATH/cpp-stub/*.h $$TEST_UTILS_PATH/cpp-stub/*.hpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.h)

# Gather source files
SOURCES += $$files($$TEST_UTILS_PATH/cpp-stub/*.cpp) \
           $$files($$TEST_UTILS_PATH/cpp-stub-ext/*.cpp)

