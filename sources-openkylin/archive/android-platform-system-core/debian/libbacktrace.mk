NAME = libbacktrace
SOURCES = Backtrace.cpp \
          BacktraceCurrent.cpp \
          BacktraceMap.cpp \
          BacktracePtrace.cpp \
          thread_utils.c \
          ThreadEntry.cpp \
          UnwindCurrent.cpp \
          UnwindMap.cpp \
          UnwindPtrace.cpp

demangle_srcs = demangle/Demangler.cpp

CSOURCES = $(foreach source, $(filter %.c, $(SOURCES)), libbacktrace/$(source))
CXXSOURCES = $(foreach source, $(filter %.cpp, $(SOURCES)), libbacktrace/$(source)) $(demangle_srcs)
COBJECTS = $(CSOURCES:.c=.o)
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)
CFLAGS += -c -fno-omit-frame-pointer
CXXFLAGS += -c -std=gnu++11 -fno-omit-frame-pointer
CPPFLAGS += -Iinclude -Ibase/include -Idemangle/include -I/usr/include/android/unwind
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 \
           -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
           -L/usr/lib/$(DEB_HOST_MULTIARCH)/android -lunwind \
           -L. -lbase -llog -lcutils -lpthread

build: $(COBJECTS) $(CXXOBJECTS)
	$(CXX) $^ -o $(NAME).so.0 $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(COBJECTS) $(CXXOBJECTS) $(NAME).so*

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) $(CPPFLAGS)

$(COBJECTS): %.o: %.c
	$(CC) $< -o $@ $(CFLAGS) $(CPPFLAGS)
