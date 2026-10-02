NAME = libnativebridge
SOURCES = native_bridge.cc
SOURCES := $(foreach source, $(SOURCES), libnativebridge/$(source))
CPPFLAGS += \
  -I/usr/include/android/nativehelper \
  -Iinclude \
  -Ilibnativebridge/include
LDFLAGS += \
  -shared -Wl,-soname,$(NAME).so.0 \
  -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
  -ldl \
  -L. \
  -llog

build: $(SOURCES)
	$(CXX) $^ -o $(NAME).so.0 $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(NAME).so*