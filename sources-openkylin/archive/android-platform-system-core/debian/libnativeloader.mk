NAME = libnativeloader
SOURCES = native_loader.cpp
SOURCES := $(foreach source, $(SOURCES), libnativeloader/$(source))
CPPFLAGS += \
  -I/usr/include/android/nativehelper \
  -Iinclude \
  -Ibase/include \
  -Ilibnativebridge/include \
  -Ilibnativeloader/include
LDFLAGS += \
  -shared -Wl,-soname,$(NAME).so.0 \
  -ldl \
  -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
  -L. \
  -lnativebridge

build: $(SOURCES)
	$(CXX) $^ -o $(NAME).so.0 $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(NAME).so*