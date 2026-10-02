#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <fcntl.h>
#include <unistd.h>

#include "base/generic_writer.h"
#include "base/proc_maps_iterator.h"

#include "../src/base/proc_maps_iterator.cc"

namespace {

int failures = 0;

#define EXPECT_TRUE(condition)                                                                         \
  do {                                                                                                 \
    if (!(condition)) {                                                                                \
      std::fprintf(stderr, "EXPECT_TRUE failed at %s:%d: %s\n", __FILE__, __LINE__, #condition);       \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

#define EXPECT_EQ(actual, expected)                                                                    \
  do {                                                                                                 \
    auto actual_value = (actual);                                                                      \
    auto expected_value = (expected);                                                                  \
    if (!(actual_value == expected_value)) {                                                           \
      std::fprintf(stderr, "EXPECT_EQ failed at %s:%d\n", __FILE__, __LINE__);                         \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

void TestHelperParsersAndFormatting()
{
  using namespace tcmalloc;

  char* endptr = nullptr;
  char text_with_colon[] = "abc:def";
  EXPECT_TRUE(ExtractUntilChar(text_with_colon, ':', &endptr));
  EXPECT_EQ(std::string(text_with_colon), std::string("abc"));
  EXPECT_TRUE(endptr != nullptr);

  char text_without_colon[] = "abcdef";
  endptr = reinterpret_cast<char*>(0x1);
  EXPECT_TRUE(!ExtractUntilChar(text_without_colon, ':', &endptr));
  EXPECT_TRUE(endptr == nullptr);

  char whitespace[] = "   x";
  char* whitespace_ptr = whitespace;
  SkipWhileWhitespace(&whitespace_ptr, ' ');
  EXPECT_TRUE(*whitespace_ptr == ' ');
  EXPECT_TRUE(*(whitespace_ptr + 1) == 'x');

  char ok_number[] = "1a tail";
  endptr = nullptr;
  EXPECT_EQ(StringToIntegerUntilChar(ok_number, 16, ' ', &endptr), uint64_t{0x1a});
  EXPECT_TRUE(endptr != nullptr);
  EXPECT_TRUE(*endptr == ' ');

  char bad_number[] = "1g tail";
  endptr = nullptr;
  EXPECT_EQ(StringToIntegerUntilChar(bad_number, 16, ' ', &endptr), uint64_t{0});
  EXPECT_TRUE(endptr == nullptr);

  char copy_text[] = "name tail";
  char out[8] = {};
  endptr = CopyStringUntilChar(copy_text, sizeof(out), ' ', out);
  EXPECT_TRUE(endptr != nullptr);
  EXPECT_EQ(std::string(out), std::string("name"));

  char no_delimiter[] = "77";
  char* parse_ptr = no_delimiter;
  uint64_t out_value = 123;
  EXPECT_TRUE(!StringToIntegerUntilCharWithCheck(&out_value, no_delimiter, 10, ' ', &parse_ptr));

  char valid_line[] = "00400000-00452000 r-xp 00000000 08:02 173521 /usr/bin/cat";
  uint64_t start = 0;
  uint64_t end = 0;
  uint64_t offset = 0;
  uint64_t inode = 0;
  unsigned filename_offset = 0;
  char flags[8] = {};
  EXPECT_TRUE(ParseProcMapsLine(valid_line, &start, &end, flags, &offset, &inode, &filename_offset));
  EXPECT_EQ(start, uint64_t{0x00400000});
  EXPECT_EQ(end, uint64_t{0x00452000});
  EXPECT_EQ(std::string(flags), std::string("r-xp"));
  EXPECT_EQ(offset, uint64_t{0});
  EXPECT_EQ(inode, uint64_t{173521});
  EXPECT_EQ(std::string(valid_line + filename_offset), std::string("/usr/bin/cat"));

  char invalid_line_1[] = "";
  EXPECT_TRUE(!ParseProcMapsLine(invalid_line_1, &start, &end, flags, &offset, &inode, &filename_offset));
  char invalid_line_2[] = "00400000 00452000 r-xp 00000000 08:02 173521 /usr/bin/cat";
  EXPECT_TRUE(!ParseProcMapsLine(invalid_line_2, &start, &end, flags, &offset, &inode, &filename_offset));
  char invalid_line_3[] = "00400000-00452000 r-xp 00000000 08 173521 /usr/bin/cat";
  EXPECT_TRUE(!ParseProcMapsLine(invalid_line_3, &start, &end, flags, &offset, &inode, &filename_offset));

  std::string formatted;
  {
    StringGenericWriter writer(&formatted);
    FormatLine(&writer, 1, 2, nullptr, 3, 4, "/tmp/a", 0);
  }
  EXPECT_TRUE(formatted.find("---p") != std::string::npos);
  EXPECT_TRUE(formatted.find("/tmp/a") != std::string::npos);

  formatted.clear();
  {
    StringGenericWriter writer(&formatted);
    FormatLine(&writer, 1, 2, "rwxs", 3, 4, "/tmp/b", 0x0102);
  }
  EXPECT_TRUE(formatted.find("rwx-") != std::string::npos);
  EXPECT_TRUE(formatted.find("/tmp/b") != std::string::npos);
}

void TestIteratorsOnMissingAndMalformedInputs()
{
  using namespace tcmalloc;

  EXPECT_TRUE(!DoForEachLine("/definitely/missing/proc-maps", [](char*, char*) { return true; }));

  char path[] = "/tmp/perftools-proc-maps-XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  if (fd < 0) {
    return;
  }

  const char* content =
      "00400000-00452000 r-xp 00000000 08:02 173521 /usr/bin/cat\n"
      "broken-line-without-fields\n";
  EXPECT_TRUE(write(fd, content, std::strlen(content)) == static_cast<ssize_t>(std::strlen(content)));
  close(fd);

  int callback_count = 0;
  bool ok = DoIterateLinux(
      path,
      [](const ProcMapping& mapping, void* arg) {
        int* count = static_cast<int*>(arg);
        ++(*count);
        EXPECT_TRUE(mapping.end >= mapping.start);
      },
      &callback_count);
  unlink(path);

  EXPECT_TRUE(ok);
  EXPECT_EQ(callback_count, 1);
}

void TestPublicProcMapApis()
{
  int mapping_count = 0;
  EXPECT_TRUE(tcmalloc::DoForEachProcMapping(
      [](const tcmalloc::ProcMapping& mapping, void* arg) {
        int* count = static_cast<int*>(arg);
        ++(*count);
        EXPECT_TRUE(mapping.flags != nullptr);
        EXPECT_TRUE(mapping.filename != nullptr);
      },
      &mapping_count));
  EXPECT_TRUE(mapping_count > 0);

  std::string maps_text;
  tcmalloc::StringGenericWriter writer(&maps_text);
  tcmalloc::SaveProcSelfMaps(&writer);
  EXPECT_TRUE(!maps_text.empty());

  char path[] = "/tmp/perftools-proc-maps-raw-XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  if (fd >= 0) {
    tcmalloc::SaveProcSelfMapsToRawFD(fd);
    close(fd);
    int read_fd = open(path, O_RDONLY);
    EXPECT_TRUE(read_fd >= 0);
    char buf[64] = {};
    ssize_t count = read(read_fd, buf, sizeof(buf));
    EXPECT_TRUE(count > 0);
    close(read_fd);
    unlink(path);
  }
}

}  // namespace

int main()
{
  TestHelperParsersAndFormatting();
  TestIteratorsOnMissingAndMalformedInputs();
  TestPublicProcMapApis();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools proc maps whitebox checks failed\n", failures);
    return 1;
  }
  return 0;
}
