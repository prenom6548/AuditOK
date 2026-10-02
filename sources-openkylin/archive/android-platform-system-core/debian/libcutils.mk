NAME = libcutils

# copied from libcutils/Android.bp
libcutils_nonwindows_sources = \
    android_get_control_file.cpp \
    fs.c \
    multiuser.c \
    socket_inaddr_any_server_unix.c \
    socket_local_client_unix.c \
    socket_local_server_unix.c \
    socket_network_client_unix.c \
    sockets_unix.cpp \
    str_parms.c \

# copied from libcutils/Android.bp
cc_library_srcs = \
        config_utils.c \
        fs_config.cpp \
        canned_fs_config.c \
        hashmap.c \
        iosched_policy.c \
        load_file.c \
        native_handle.c \
        open_memstream.c \
        record_stream.c \
        sched_policy.cpp \
        sockets.cpp \
        strdup16to8.c \
        strdup8to16.c \
        strlcpy.c \
        threads.c \

# copied from libcutils/Android.bp
cc_library_target_host_srcs = \
                dlmalloc_stubs.c

# copied from libcutils/Android.bp
cc_library_target_not_windows_srcs = \
                ashmem-host.c \
                trace-host.c \

SOURCES = \
  $(libcutils_nonwindows_sources) \
  $(cc_library_srcs) \
  $(cc_library_target_host_srcs) \
  $(cc_library_target_not_windows_srcs)


CSOURCES := $(foreach source, $(filter %.c, $(SOURCES)), libcutils/$(source))
CXXSOURCES := $(foreach source, $(filter %.cpp, $(SOURCES)), libcutils/$(source))
COBJECTS := $(CSOURCES:.c=.o)
CXXOBJECTS := $(CXXSOURCES:.cpp=.o)
CFLAGS += -c
CXXFLAGS += -c
CPPFLAGS += -Iinclude
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 \
           -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android -lpthread -L. -llog

build: $(COBJECTS) $(CXXOBJECTS)
	$(CXX) $^ -o $(NAME).so.0 $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(CXXOBJECTS) $(COBJECTS) $(NAME).so*

$(COBJECTS): %.o: %.c
	$(CC) $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)
