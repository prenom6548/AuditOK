NAME = libziparchive
SOURCES = zip_archive.cc \
          zip_archive_stream_entry.cc\
          zip_writer.cc
SOURCES := $(foreach source, $(SOURCES), libziparchive/$(source))
CXXFLAGS += -std=gnu++11
CPPFLAGS += -DZLIB_CONST -D_FILE_OFFSET_BITS=64 \
            -Iinclude -Ibase/include -Ilibziparchive/include
LDFLAGS += -shared -Wl,-soname,$(NAME).so.0 \
           -Wl,-rpath=/usr/lib/$(DEB_HOST_MULTIARCH)/android \
           -lz -L. -lutils -llog -lbase

build: $(SOURCES)
	$(CXX) $^ -o $(NAME).so.0 $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
	ln -s $(NAME).so.0 $(NAME).so

clean:
	$(RM) $(NAME).so*
