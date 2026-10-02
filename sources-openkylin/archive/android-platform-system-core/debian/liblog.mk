NAME = liblog

# copied from liblog/Android.bp
liblog_sources = \
    config_read.c \
    config_write.c \
    local_logger.c \
    log_event_list.c \
    log_event_write.c \
    log_ratelimit.cpp \
    logger_lock.c \
    logger_name.c \
    logger_read.c \
    logger_write.c \
    logprint.c \
    stderr_write.c

# copied from liblog/Android.bp
liblog_host_sources = \
    fake_log_device.c \
    fake_writer.c

# copied from liblog/Android.bp
not_windows_sources = \
  event_tag_map.cpp \

SOURCES = $(liblog_sources) $(liblog_host_sources) $(not_windows_sources)
CSOURCES := $(foreach source, $(filter %.c, $(SOURCES)), liblog/$(source))
CXXSOURCES := $(foreach source, $(filter %.cpp, $(SOURCES)), liblog/$(source))
COBJECTS := $(CSOURCES:.c=.o)
CXXOBJECTS := $(CXXSOURCES:.cpp=.o)
CFLAGS += -c -fvisibility=hidden -fPIC
CXXFLAGS += -c -fpermissive
CPPFLAGS += -Iinclude \
            -DLIBLOG_LOG_TAG=1006 \
            -DFAKE_LOG_DEVICE=1 \
            -DSNET_EVENT_LOG_TAG=1397638484
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 -lpthread

build: $(COBJECTS) $(CXXOBJECTS)
	$(CXX) $^ -o $(NAME).so.0 $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(COBJECTS) $(CXXOBJECTS) $(NAME).so*

$(COBJECTS): %.o: %.c
	$(CC) $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)
