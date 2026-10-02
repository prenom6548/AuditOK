# 供 tests 下各单测工程统一引入：覆盖率与 gtest 链接
CONFIG += c++17
QMAKE_CXXFLAGS += --coverage -g
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
LIBS += -lgcov

unix:!macx {
    # Debian/Ubuntu/Kylin：libgtest-dev
    exists(/usr/lib/x86_64-linux-gnu/libgtest.a) {
        LIBS += /usr/lib/x86_64-linux-gnu/libgtest.a -lpthread
    } else:exists(/usr/lib/aarch64-linux-gnu/libgtest.a) {
        LIBS += /usr/lib/aarch64-linux-gnu/libgtest.a -lpthread
    } else {
        LIBS += -lgtest -lpthread
    }
}
