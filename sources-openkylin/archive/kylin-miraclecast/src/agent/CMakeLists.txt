find_package(PkgConfig)
pkg_check_modules (GLIB2 REQUIRED glib-2.0)
pkg_check_modules (GIO2 REQUIRED gio-2.0)
pkg_check_modules (DBUS1 REQUIRED dbus-1)
pkg_check_modules (DBUSGLIB1 REQUIRED dbus-glib-1)
link_directories( ${GLIB2_LIBRARY_DIRS} ${DBUS1_LIBRARY_DIRS} ${DBUSGLIB1_LIBRARY_DIRS})
include_directories( ${GLIB2_INCLUDE_DIRS})

########### next target ###############

set(miracle-agent_SRCS miracle-agent.c
                        ui-agent.c
                        wfd_r2_agent.c
                        agent.h)

add_executable(miracle-agent ${miracle-agent_SRCS})

target_link_libraries(miracle-agent miracle-shared)
target_link_libraries(miracle-agent ${GLIB2_LIBRARIES} ${GIO2_LIBRARIES} ${DBUS1_LIBRARIES} ${DBUSGLIB1_LIBRARIES} gdk-3)

install(TARGETS miracle-agent DESTINATION bin)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src/shared ${DBUS1_INCLUDE_DIRS} /usr/include/gtk-3.0/ /usr/include/pango-1.0/ /usr/include/harfbuzz/ /usr/include/cairo/ /usr/include/gdk-pixbuf-2.0/ /usr/include/atk-1.0/)

########### install files ###############

#original Makefile.am contents follow:

#include $(top_srcdir)/common.am
#bin_PROGRAMS = miracle-agent miracle-sinkctl
#
#miracle_agent_SOURCES = \
#	ctl.h \
#	ctl-cli.c \
#	ctl-wifi.c \
#	agent.c
#miracle_agent_CPPFLAGS = \
#	$(AM_CPPFLAGS) \
#	$(DEPS_CFLAGS)
#miracle_agent_LDADD = \
#	../shared/libmiracle-shared.la \
#	-lreadline \
#	$(DEPS_LIBS)
#
#miracle_sinkctl_SOURCES = \
#	ctl.h \
#	ctl-cli.c \
#	ctl-sink.c \
#	ctl-wifi.c \
#	sinkctl.c
#miracle_sinkctl_CPPFLAGS = \
#	$(AM_CPPFLAGS) \
#	$(DEPS_CFLAGS)
#miracle_sinkctl_LDADD = \
#	../shared/libmiracle-shared.la \
#	-lreadline \
#	$(DEPS_LIBS)
#
#
