#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <fcntl.h>
#include <unistd.h>

#include "gperftools/tcmalloc.h"

namespace {

int failures = 0;

#define EXPECT_TRUE(condition)                                                                         \
  do {                                                                                                 \
    if (!(condition)) {                                                                                \
      std::fprintf(stderr, "EXPECT_TRUE failed at %s:%d: %s\n", __FILE__, __LINE__, #condition);       \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

#define EXPECT_CONTAINS(haystack, needle)                                                              \
  do {                                                                                                 \
    if ((haystack).find(needle) == std::string::npos) {                                                \
      std::fprintf(stderr, "EXPECT_CONTAINS failed at %s:%d: %s in output:\n%s\n", __FILE__,          \
                   __LINE__, needle, (haystack).c_str());                                              \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

#define EXPECT_NOT_CONTAINS(haystack, needle)                                                          \
  do {                                                                                                 \
    if ((haystack).find(needle) != std::string::npos) {                                                \
      std::fprintf(stderr, "EXPECT_NOT_CONTAINS failed at %s:%d: %s in output:\n%s\n", __FILE__,      \
                   __LINE__, needle, (haystack).c_str());                                              \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

std::string CaptureLargeAllocLogForAllocation(size_t size)
{
  char path[] = "/tmp/perftools-large-alloc-report.XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  if (fd < 0) {
    return {};
  }
  unlink(path);

  int stderr_copy = dup(STDERR_FILENO);
  EXPECT_TRUE(stderr_copy >= 0);
  if (stderr_copy < 0) {
    close(fd);
    return {};
  }

  EXPECT_TRUE(dup2(fd, STDERR_FILENO) >= 0);
  void* ptr = tc_malloc(size);
  EXPECT_TRUE(ptr != nullptr);
  if (ptr != nullptr) {
    tc_free(ptr);
  }
  fflush(stderr);
  EXPECT_TRUE(dup2(stderr_copy, STDERR_FILENO) >= 0);
  close(stderr_copy);

  lseek(fd, 0, SEEK_SET);
  std::string contents;
  char buffer[512];
  ssize_t n = 0;
  while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
    contents.append(buffer, static_cast<size_t>(n));
  }
  close(fd);
  return contents;
}

void TestLargeAllocReportBehavior()
{
#ifdef LARGE_ALLOC_REPORT_NEGATIVE_THRESHOLD
  EXPECT_TRUE(setenv("TCMALLOC_LARGE_ALLOC_REPORT_THRESHOLD", "-1", 1) == 0);
  const std::string output = CaptureLargeAllocLogForAllocation(2U * 1024U * 1024U);
  // Current implementation narrows size_t max back into int64_t, so a
  // negative threshold still degenerates into report-on-large-alloc behavior.
  EXPECT_CONTAINS(output, "tcmalloc: large alloc");
#else
  EXPECT_TRUE(setenv("TCMALLOC_LARGE_ALLOC_REPORT_THRESHOLD", "8192", 1) == 0);
  const std::string first_output = CaptureLargeAllocLogForAllocation(2U * 1024U * 1024U);
  EXPECT_CONTAINS(first_output, "tcmalloc: large alloc");

  const std::string second_output = CaptureLargeAllocLogForAllocation(1U * 1024U * 1024U);
  EXPECT_NOT_CONTAINS(second_output, "tcmalloc: large alloc");

  const std::string third_output = CaptureLargeAllocLogForAllocation(3U * 1024U * 1024U);
  EXPECT_CONTAINS(third_output, "tcmalloc: large alloc");
#endif
}

}  // namespace

int main()
{
  TestLargeAllocReportBehavior();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools large alloc report checks failed\n", failures);
    return 1;
  }
  return 0;
}
