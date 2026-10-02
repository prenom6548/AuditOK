#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include "base/dynamic_annotations.h"
#include "base/generic_writer.h"
#include "base/proc_maps_iterator.h"
#include "base/sysinfo.h"
#include "internal_logging.h"
#include "safe_strerror.h"

namespace {

int failures = 0;

#define EXPECT_TRUE(condition)                                                                         \
  do {                                                                                                 \
    if (!(condition)) {                                                                                \
      fprintf(stderr, "EXPECT_TRUE failed at %s:%d: %s\n", __FILE__, __LINE__, #condition);            \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

#define EXPECT_EQ(actual, expected)                                                                    \
  do {                                                                                                 \
    auto actual_value = (actual);                                                                      \
    auto expected_value = (expected);                                                                  \
    if (!(actual_value == expected_value)) {                                                           \
      fprintf(stderr, "EXPECT_EQ failed at %s:%d\n", __FILE__, __LINE__);                              \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

void TestSafeStrError()
{
  tcmalloc::SafeStrError known(EINVAL);
  EXPECT_EQ(std::string(known.c_str()), std::string("EINVAL"));

  struct ErrnoCase {
    int errnum;
    const char* expected;
  };

  const ErrnoCase known_cases[] = {
#ifdef EACCES
      {EACCES, "EACCES"},
#endif
#ifdef EADDRINUSE
      {EADDRINUSE, "EADDRINUSE"},
#endif
#ifdef EADDRNOTAVAIL
      {EADDRNOTAVAIL, "EADDRNOTAVAIL"},
#endif
#ifdef EAFNOSUPPORT
      {EAFNOSUPPORT, "EAFNOSUPPORT"},
#endif
#ifdef EAGAIN
      {EAGAIN, "EAGAIN"},
#endif
#ifdef EALREADY
      {EALREADY, "EALREADY"},
#endif
#ifdef EBADF
      {EBADF, "EBADF"},
#endif
#ifdef EBADMSG
      {EBADMSG, "EBADMSG"},
#endif
#ifdef EBUSY
      {EBUSY, "EBUSY"},
#endif
#ifdef ECANCELED
      {ECANCELED, "ECANCELED"},
#endif
#ifdef ECHILD
      {ECHILD, "ECHILD"},
#endif
#ifdef ECONNABORTED
      {ECONNABORTED, "ECONNABORTED"},
#endif
#ifdef ECONNREFUSED
      {ECONNREFUSED, "ECONNREFUSED"},
#endif
#ifdef ECONNRESET
      {ECONNRESET, "ECONNRESET"},
#endif
#ifdef EDEADLK
      {EDEADLK, "EDEADLK"},
#endif
#ifdef EDESTADDRREQ
      {EDESTADDRREQ, "EDESTADDRREQ"},
#endif
#ifdef EDOM
      {EDOM, "EDOM"},
#endif
#ifdef EEXIST
      {EEXIST, "EEXIST"},
#endif
#ifdef EFAULT
      {EFAULT, "EFAULT"},
#endif
#ifdef EFBIG
      {EFBIG, "EFBIG"},
#endif
#ifdef EHOSTUNREACH
      {EHOSTUNREACH, "EHOSTUNREACH"},
#endif
#ifdef EILSEQ
      {EILSEQ, "EILSEQ"},
#endif
#ifdef EINPROGRESS
      {EINPROGRESS, "EINPROGRESS"},
#endif
#ifdef EINTR
      {EINTR, "EINTR"},
#endif
#ifdef EISCONN
      {EISCONN, "EISCONN"},
#endif
#ifdef EIO
      {EIO, "EIO"},
#endif
#ifdef EISDIR
      {EISDIR, "EISDIR"},
#endif
#ifdef ELOOP
      {ELOOP, "ELOOP"},
#endif
#ifdef EMFILE
      {EMFILE, "EMFILE"},
#endif
#ifdef EMLINK
      {EMLINK, "EMLINK"},
#endif
#ifdef EMSGSIZE
      {EMSGSIZE, "EMSGSIZE"},
#endif
#ifdef ENAMETOOLONG
      {ENAMETOOLONG, "ENAMETOOLONG"},
#endif
#ifdef ENETDOWN
      {ENETDOWN, "ENETDOWN"},
#endif
#ifdef ENETRESET
      {ENETRESET, "ENETRESET"},
#endif
#ifdef ENETUNREACH
      {ENETUNREACH, "ENETUNREACH"},
#endif
#ifdef ENFILE
      {ENFILE, "ENFILE"},
#endif
#ifdef ENOBUFS
      {ENOBUFS, "ENOBUFS"},
#endif
#ifdef ENODATA
      {ENODATA, "ENODATA"},
#endif
#ifdef ENODEV
      {ENODEV, "ENODEV"},
#endif
#ifdef ENOENT
      {ENOENT, "ENOENT"},
#endif
#ifdef ENOEXEC
      {ENOEXEC, "ENOEXEC"},
#endif
#ifdef ENOLCK
      {ENOLCK, "ENOLCK"},
#endif
#ifdef ENOLINK
      {ENOLINK, "ENOLINK"},
#endif
#ifdef ENOMEM
      {ENOMEM, "ENOMEM"},
#endif
#ifdef ENOMSG
      {ENOMSG, "ENOMSG"},
#endif
#ifdef ENOPROTOOPT
      {ENOPROTOOPT, "ENOPROTOOPT"},
#endif
#ifdef ENOSPC
      {ENOSPC, "ENOSPC"},
#endif
#ifdef ENOSR
      {ENOSR, "ENOSR"},
#endif
#ifdef ENOSTR
      {ENOSTR, "ENOSTR"},
#endif
#ifdef ENOSYS
      {ENOSYS, "ENOSYS"},
#endif
#ifdef ENOTCONN
      {ENOTCONN, "ENOTCONN"},
#endif
#ifdef ENOTDIR
      {ENOTDIR, "ENOTDIR"},
#endif
#ifdef ENOTEMPTY
      {ENOTEMPTY, "ENOTEMPTY"},
#endif
#ifdef ENOTSOCK
      {ENOTSOCK, "ENOTSOCK"},
#endif
#ifdef ENOTSUP
      {ENOTSUP, "ENOTSUP"},
#endif
#ifdef ENOTTY
      {ENOTTY, "ENOTTY"},
#endif
#ifdef ENXIO
      {ENXIO, "ENXIO"},
#endif
#ifdef EOPNOTSUPP
      {EOPNOTSUPP, "EOPNOTSUPP"},
#endif
#ifdef EOVERFLOW
      {EOVERFLOW, "EOVERFLOW"},
#endif
#ifdef EPERM
      {EPERM, "EPERM"},
#endif
#ifdef EPIPE
      {EPIPE, "EPIPE"},
#endif
#ifdef EPROTO
      {EPROTO, "EPROTO"},
#endif
#ifdef EPROTONOSUPPORT
      {EPROTONOSUPPORT, "EPROTONOSUPPORT"},
#endif
#ifdef EPROTOTYPE
      {EPROTOTYPE, "EPROTOTYPE"},
#endif
#ifdef ERANGE
      {ERANGE, "ERANGE"},
#endif
#ifdef EROFS
      {EROFS, "EROFS"},
#endif
#ifdef ESPIPE
      {ESPIPE, "ESPIPE"},
#endif
#ifdef ESRCH
      {ESRCH, "ESRCH"},
#endif
#ifdef ESTALE
      {ESTALE, "ESTALE"},
#endif
#ifdef ETIMEDOUT
      {ETIMEDOUT, "ETIMEDOUT"},
#endif
#ifdef ETXTBSY
      {ETXTBSY, "ETXTBSY"},
#endif
#ifdef EWOULDBLOCK
      {EWOULDBLOCK, "EWOULDBLOCK"},
#endif
#ifdef EXDEV
      {EXDEV, "EXDEV"},
#endif
  };

  for (const auto& known_case : known_cases) {
    tcmalloc::SafeStrError value(known_case.errnum);
    const std::string actual(value.c_str());
    bool matched = (actual == known_case.expected);
    if (!matched) {
      for (const auto& alias_case : known_cases) {
        if (alias_case.errnum == known_case.errnum && actual == alias_case.expected) {
          matched = true;
          break;
        }
      }
    }
    EXPECT_TRUE(matched);
  }

  tcmalloc::SafeStrError unknown(1234567);
  EXPECT_EQ(std::string(unknown.c_str()), std::string("errno 1234567"));
}

void TestStringGenericWriter()
{
  std::string output;
  {
    tcmalloc::StringGenericWriter writer(&output);
    writer.AppendStr("hello");
    writer.AppendMem(" world", 6);
    writer.AppendF(" %d", 42);

    std::string large(9000, 'x');
    writer.AppendMem(large.data(), large.size());
  }

  EXPECT_TRUE(output.rfind("hello world 42", 0) == 0);
  EXPECT_TRUE(output.size() > 9000);
  EXPECT_EQ(output.back(), 'x');
}

void TestChunkedWriter()
{
  tcmalloc::ChunkedWriterConfig config(std::malloc, std::free, 16);
  char* result = tcmalloc::WithWriterToStrDup(config, [](tcmalloc::GenericWriter* writer) {
    writer->AppendStr("alpha");
    writer->AppendMem(":", 1);
    writer->AppendF("%s-%d", "beta", 7);
    writer->AppendMem("01234567890123456789", 20);
  });

  EXPECT_EQ(std::string(result), std::string("alpha:beta-701234567890123456789"));
  std::free(result);
}

void TestRawWriter()
{
  char path[] = "/tmp/perftools-core-test-XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  if (fd >= 0) {
    close(fd);
  }

  RawFD raw = RawOpenForWriting(path);
  EXPECT_TRUE(raw != kIllegalRawFD);
  RawWrite(raw, "raw-data", 8);
  RawClose(raw);

  int read_fd = open(path, O_RDONLY);
  EXPECT_TRUE(read_fd >= 0);
  char buf[16] = {};
  ssize_t read_count = read(read_fd, buf, sizeof(buf));
  close(read_fd);
  unlink(path);

  EXPECT_EQ(read_count, static_cast<ssize_t>(8));
  EXPECT_EQ(std::string(buf, 8), std::string("raw-data"));
}

void TestProcMaps()
{
  int mapping_count = 0;
  bool saw_current_exe_or_library = false;
  bool ok = tcmalloc::ForEachProcMapping([&](const tcmalloc::ProcMapping& mapping) {
    ++mapping_count;
    EXPECT_TRUE(mapping.end >= mapping.start);
    EXPECT_TRUE(mapping.flags != nullptr);
    EXPECT_TRUE(mapping.filename != nullptr);
    if (std::strstr(mapping.filename, "perftoolsCoreTest") != nullptr ||
        std::strstr(mapping.filename, ".so") != nullptr) {
      saw_current_exe_or_library = true;
    }
  });

  EXPECT_TRUE(ok);
  EXPECT_TRUE(mapping_count > 0);
  EXPECT_TRUE(saw_current_exe_or_library);

  std::string maps_text;
  {
    tcmalloc::StringGenericWriter writer(&maps_text);
    tcmalloc::SaveProcSelfMaps(&writer);
  }
  EXPECT_TRUE(maps_text.find(" r") != std::string::npos);
  EXPECT_TRUE(maps_text.find('\n') != std::string::npos);

  char path[] = "/tmp/perftools-maps-test-XXXXXX";
  int fd = mkstemp(path);
  EXPECT_TRUE(fd >= 0);
  if (fd >= 0) {
    tcmalloc::SaveProcSelfMapsToRawFD(fd);
    close(fd);
    int read_fd = open(path, O_RDONLY);
    EXPECT_TRUE(read_fd >= 0);
    char buf[64] = {};
    ssize_t count = read(read_fd, buf, sizeof(buf));
    close(read_fd);
    EXPECT_TRUE(count > 0);
  }
  unlink(path);
}

void TestProcMapsFormattedOutputAndCallbackShape()
{
  struct MappingStats {
    int executable = 0;
    int readable = 0;
    int named = 0;
    uint64_t previous_start = 0;
  } stats;

  bool ok = tcmalloc::ForEachProcMapping([&](const tcmalloc::ProcMapping& mapping) {
    EXPECT_TRUE(mapping.end >= mapping.start);
    EXPECT_TRUE(mapping.flags != nullptr);
    EXPECT_TRUE(std::strlen(mapping.flags) >= 3);
    if (mapping.flags[0] == 'r') {
      ++stats.readable;
    }
    if (mapping.flags[2] == 'x') {
      ++stats.executable;
    }
    if (mapping.filename != nullptr && mapping.filename[0] != '\0') {
      ++stats.named;
    }
    stats.previous_start = mapping.start;
  });

  EXPECT_TRUE(ok);
  EXPECT_TRUE(stats.readable > 0);
  EXPECT_TRUE(stats.named > 0);
  EXPECT_TRUE(stats.previous_start != 0);

  std::string maps_text;
  tcmalloc::StringGenericWriter writer(&maps_text);
  tcmalloc::SaveProcSelfMaps(&writer);
  EXPECT_TRUE(maps_text.find(" r--p ") != std::string::npos ||
              maps_text.find(" r-xp ") != std::string::npos ||
              maps_text.find(" rw-p ") != std::string::npos);
  EXPECT_TRUE(maps_text.find(" 00:00 ") != std::string::npos);
}

void TestSysInfoAndEnvironment()
{
  setenv("PERFTOOLS_TEST_PATH", "/tmp/perf-path", 1);
  setenv("PERFTOOLS_PMIX_PATH", "/tmp/pmix-path", 1);
  setenv("PERFTOOLS_SLURM_PATH", "/tmp/slurm-path", 1);
  setenv("PERFTOOLS_PMI_PATH", "/tmp/pmi-path", 1);
  unsetenv("PERFTOOLS_TEST_PATH_USE_PID");
  unsetenv("PERFTOOLS_PMIX_PATH_USE_PID");
  unsetenv("PERFTOOLS_SLURM_PATH_USE_PID");
  unsetenv("PERFTOOLS_PMI_PATH_USE_PID");
  unsetenv("PMIX_RANK");
  unsetenv("SLURM_JOB_ID");
  unsetenv("SLURM_PROCID");
  unsetenv("OMPI_HOME");
  unsetenv("PMI_RANK");

  EXPECT_EQ(std::string(GetenvBeforeMain("PERFTOOLS_TEST_PATH")), std::string("/tmp/perf-path"));

  char path[PATH_MAX] = {};
  EXPECT_TRUE(GetUniquePathFromEnv("PERFTOOLS_TEST_PATH", path));
  EXPECT_EQ(std::string(path), std::string("/tmp/perf-path"));

  EXPECT_TRUE(GetenvBeforeMain("PERFTOOLS_TEST_PATH_USE_PID") != nullptr);
  EXPECT_TRUE(GetUniquePathFromEnv("PERFTOOLS_TEST_PATH", path));
  EXPECT_TRUE(std::string(path).find("/tmp/perf-path_") == 0);

  setenv("PMIX_RANK", "9", 1);
  EXPECT_TRUE(GetUniquePathFromEnv("PERFTOOLS_PMIX_PATH", path));
  EXPECT_EQ(std::string(path), std::string("/tmp/pmix-path.rank-9"));

  unsetenv("PMIX_RANK");
  setenv("SLURM_JOB_ID", "job", 1);
  setenv("SLURM_PROCID", "3", 1);
  EXPECT_TRUE(GetUniquePathFromEnv("PERFTOOLS_SLURM_PATH", path));
  EXPECT_EQ(std::string(path), std::string("/tmp/slurm-path.slurmid-3"));

  unsetenv("SLURM_PROCID");
  EXPECT_TRUE(GetUniquePathFromEnv("PERFTOOLS_SLURM_PATH", path));
  EXPECT_TRUE(std::string(path).find("/tmp/slurm-path_") == 0);

  unsetenv("SLURM_JOB_ID");
  setenv("PMI_RANK", "5", 1);
  EXPECT_TRUE(GetUniquePathFromEnv("PERFTOOLS_PMI_PATH", path));
  EXPECT_EQ(std::string(path), std::string("/tmp/pmi-path.rank-5"));

  EXPECT_TRUE(GetSystemCPUsCount() >= 1);
  EXPECT_TRUE(tcmalloc::GetProgramInvocationName() != nullptr);

  tcmalloc::SafeSetEnv("PERFTOOLS_SAFE_SET_ENV_TEST", "ok");
  EXPECT_EQ(std::string(getenv("PERFTOOLS_SAFE_SET_ENV_TEST")), std::string("ok"));
}

void TestDynamicAnnotations()
{
  EXPECT_TRUE(RunningOnValgrind() == 0 || RunningOnValgrind() == 1);
}

void TestInternalPrinterAndLogging()
{
  char buffer[16] = {};
  TCMalloc_Printer printer(buffer, sizeof(buffer));
  printer.printf("%s:%d", "abc", 7);
  EXPECT_EQ(std::string(buffer), std::string("abc:7"));

  printer.printf("-%s", "0123456789abcdef");
  EXPECT_TRUE(std::string(buffer).rfind("abc:7-", 0) == 0);

  char tiny[4] = {};
  TCMalloc_Printer tiny_printer(tiny, sizeof(tiny));
  tiny_printer.printf("%s", "abcdef");
  EXPECT_TRUE(tiny[0] != '\0');

  tcmalloc::Log(tcmalloc::kLog,
                "perftools-core-test",
                123,
                "signed",
                -42,
                "ptr",
                reinterpret_cast<void*>(0x1234));

  const char* long_text =
      "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
      "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
  tcmalloc::Log(tcmalloc::kLog, "perftools-core-test", 124, long_text, 123456789ULL, -1LL);
}

}  // namespace

int main()
{
  TestSafeStrError();
  TestStringGenericWriter();
  TestChunkedWriter();
  TestRawWriter();
  TestProcMaps();
  TestProcMapsFormattedOutputAndCallbackShape();
  TestSysInfoAndEnvironment();
  TestDynamicAnnotations();
  TestInternalPrinterAndLogging();

  if (failures != 0) {
    fprintf(stderr, "%d perftools core test checks failed\n", failures);
    return 1;
  }
  return 0;
}
