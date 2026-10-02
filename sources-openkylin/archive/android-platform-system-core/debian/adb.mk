NAME = adb

SOURCES = adb_client.cpp \
	bugreport.cpp \
	client/main.cpp \
	console.cpp \
	commandline.cpp \
	file_sync_client.cpp \
	line_printer.cpp \
	services.cpp \
	shell_service_protocol.cpp

SOURCES := $(foreach source, $(SOURCES), adb/$(source))
CXXFLAGS += -std=c++14 -fpermissive
CPPFLAGS += -Iinclude -Iadb -Ibase/include -I/usr/include/android \
            -DADB_VERSION='"$(DEB_VERSION)"' -DADB_HOST=1 -D_GNU_SOURCE
LDFLAGS += -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android -Wl,-rpath-link=. \
           -lpthread -L. -ladb -lbase

build: $(SOURCES)
	$(CXX) $^ -o adb/$(NAME) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)

clean:
	$(RM) adb/$(NAME)
