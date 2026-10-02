DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_MESSAGELOGCONTEXT
DEFINES += QT_NO_DEBUG_OUTPUT


TEMPLATE = app
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control 


QT += testlib
LIBS +=  -lgtest -lgtest_main -lpthread -lgcov

# 取消任何优化选项
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O3
QMAKE_CXXFLAGS_RELEASE -= -Os
QMAKE_CXXFLAGS_RELEASE -= -Ofast
QMAKE_CXXFLAGS_RELEASE -= -O1

# 确保没有其他优化选项
QMAKE_CXXFLAGS += -O0

QMAKE_LFLAGS -= -Wl,-O1