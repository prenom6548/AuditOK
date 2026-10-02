NAME = libutils
SOURCES = CallStack.cpp \
        FileMap.cpp \
        JenkinsHash.cpp \
        NativeHandle.cpp \
        Printer.cpp \
        PropertyMap.cpp \
        RefBase.cpp \
        SharedBuffer.cpp \
        Static.cpp \
        StopWatch.cpp \
        String8.cpp \
        String16.cpp \
        StrongPointer.cpp \
        SystemClock.cpp \
        Threads.cpp \
        Timers.cpp \
        Tokenizer.cpp \
        Unicode.cpp \
        VectorImpl.cpp \
        misc.cpp \
        Looper.cpp \
        ProcessCallStack.cpp \

SOURCES := $(foreach source, $(SOURCES), libutils/$(source))
CXXFLAGS += -std=gnu++11
CPPFLAGS += -Iinclude -DLIBUTILS_NATIVE=1
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 \
           -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
           -lpthread -L. -llog -lcutils -lbacktrace

build: $(SOURCES)
	$(CXX) $^ -o $(NAME).so.0 $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(NAME).so*