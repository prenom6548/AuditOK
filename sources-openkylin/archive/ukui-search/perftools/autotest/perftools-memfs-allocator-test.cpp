#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <sys/mman.h>
#include <sys/statfs.h>
#include <unistd.h>

#include "gperftools/malloc_extension.h"

namespace {

int failures = 0;

#define EXPECT_TRUE(condition)                                                                       \
  do {                                                                                               \
    if (!(condition)) {                                                                              \
      std::fprintf(stderr, "EXPECT_TRUE failed at %s:%d: %s\n", __FILE__, __LINE__, #condition);     \
      ++failures;                                                                                    \
    }                                                                                                \
  } while (0)

#define EXPECT_EQ(actual, expected)                                                                  \
  do {                                                                                               \
    auto actual_value = (actual);                                                                    \
    auto expected_value = (expected);                                                                \
    if (!(actual_value == expected_value)) {                                                         \
      std::fprintf(stderr, "EXPECT_EQ failed at %s:%d\n", __FILE__, __LINE__);                       \
      ++failures;                                                                                    \
    }                                                                                                \
  } while (0)

class FallbackAllocator : public SysAllocator {
 public:
  void* Alloc(size_t size, size_t* actual_size, size_t alignment) override {
    ++calls;
    last_size = size;
    last_alignment = alignment;
    if (actual_size != nullptr) {
      *actual_size = size;
    }
    if (return_null) {
      return nullptr;
    }
    void* ptr = nullptr;
    const size_t effective_alignment = alignment < sizeof(void*) ? sizeof(void*) : alignment;
    if (posix_memalign(&ptr, effective_alignment, size == 0 ? 1 : size) != 0) {
      return nullptr;
    }
    last_ptr = ptr;
    return ptr;
  }

  int calls = 0;
  size_t last_size = 0;
  size_t last_alignment = 0;
  bool return_null = false;
  void* last_ptr = nullptr;
};

class TestMallocExtension : public MallocExtension {
 public:
  SysAllocator* GetSystemAllocator() override { return system_allocator; }
  void SetSystemAllocator(SysAllocator* allocator) override { system_allocator = allocator; }

  SysAllocator* system_allocator = nullptr;
};

TestMallocExtension extension;
bool fail_mkstemp = false;
bool fail_ftruncate = false;
bool fail_mmap = false;
bool offset_mmap_result = false;
size_t forced_mmap_offset = 0;
int forced_errno = EIO;

std::string MakeTempPrefix()
{
  char pattern[] = "/tmp/perftools-memfs-test-XXXXXX";
  char* dir = mkdtemp(pattern);
  EXPECT_TRUE(dir != nullptr);
  return std::string(dir ? dir : "/tmp") + "/huge";
}

void FreeFallbackIfAny(FallbackAllocator& fallback)
{
  if (fallback.last_ptr != nullptr) {
    std::free(fallback.last_ptr);
    fallback.last_ptr = nullptr;
  }
}

int MemfsTestMkstemp(char* path)
{
  if (fail_mkstemp) {
    errno = forced_errno;
    return -1;
  }
  return ::mkstemp(path);
}

int MemfsTestFtruncate(int fd, off_t length)
{
  if (fail_ftruncate) {
    errno = forced_errno;
    return -1;
  }
  return ::ftruncate(fd, length);
}

void* MemfsTestMmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  if (fail_mmap) {
    errno = forced_errno;
    return MAP_FAILED;
  }
  void* result = ::mmap(addr, length, prot, flags, fd, offset);
  if (offset_mmap_result && result != MAP_FAILED) {
    return static_cast<char*>(result) + forced_mmap_offset;
  }
  return result;
}

}  // namespace

SysAllocator::~SysAllocator() = default;
MallocExtension::~MallocExtension() = default;
void MallocExtension::Initialize() {}
bool MallocExtension::VerifyAllMemory() { return true; }
bool MallocExtension::VerifyNewMemory(const void* p) { return p != nullptr; }
bool MallocExtension::VerifyArrayNewMemory(const void* p) { return p != nullptr; }
bool MallocExtension::VerifyMallocMemory(const void* p) { return p != nullptr; }
bool MallocExtension::MallocMemoryStats(int* blocks, size_t* total, int histogram[kMallocHistogramSize])
{
  if (blocks) *blocks = 0;
  if (total) *total = 0;
  if (histogram) std::memset(histogram, 0, sizeof(*histogram) * kMallocHistogramSize);
  return true;
}
bool MallocExtension::GetNumericProperty(const char* property, size_t* value)
{
  (void)property;
  if (value) *value = 0;
  return false;
}
bool MallocExtension::SetNumericProperty(const char* property, size_t value)
{
  (void)property;
  (void)value;
  return false;
}
void MallocExtension::GetStats(char* buffer, int length)
{
  if (buffer && length > 0) buffer[0] = '\0';
}
void MallocExtension::GetHeapSample(MallocExtensionWriter* writer)
{
  if (writer) writer->clear();
}
void MallocExtension::GetHeapGrowthStacks(MallocExtensionWriter* writer)
{
  if (writer) writer->clear();
}
void MallocExtension::Ranges(void* arg, RangeFunction func)
{
  (void)arg;
  (void)func;
}
void** MallocExtension::ReadStackTraces(int* sample_period)
{
  if (sample_period) *sample_period = 0;
  return nullptr;
}
void** MallocExtension::ReadHeapGrowthStackTraces() { return nullptr; }
void MallocExtension::MarkThreadIdle() {}
void MallocExtension::MarkThreadBusy() {}
SysAllocator* MallocExtension::GetSystemAllocator() { return nullptr; }
void MallocExtension::SetSystemAllocator(SysAllocator* a) { (void)a; }
void MallocExtension::ReleaseToSystem(size_t num_bytes) { (void)num_bytes; }
void MallocExtension::ReleaseFreeMemory() {}
void MallocExtension::SetMemoryReleaseRate(double rate) { (void)rate; }
double MallocExtension::GetMemoryReleaseRate() { return 0.0; }
size_t MallocExtension::GetEstimatedAllocatedSize(size_t size) { return size; }
size_t MallocExtension::GetAllocatedSize(const void* p) { return p ? 1 : 0; }
MallocExtension::Ownership MallocExtension::GetOwnership(const void* p)
{
  return p ? kUnknownOwnership : kNotOwned;
}
void MallocExtension::GetFreeListSizes(std::vector<MallocExtension::FreeListInfo>* v)
{
  if (v) v->clear();
}
size_t MallocExtension::GetThreadCacheSize() { return 0; }
void MallocExtension::MarkThreadTemporarilyIdle() {}
MallocExtension* MallocExtension::instance() { return &extension; }

#define mkstemp MemfsTestMkstemp
#define ftruncate MemfsTestFtruncate
#define mmap MemfsTestMmap
#define private public
#include "memfs_malloc.cc"
#undef private
#undef mmap
#undef ftruncate
#undef mkstemp

namespace {

void ResetFlags()
{
  FLAGS_memfs_malloc_path.clear();
  FLAGS_memfs_malloc_limit_mb = 0;
  FLAGS_memfs_malloc_abort_on_fail = false;
  FLAGS_memfs_malloc_ignore_mmap_fail = false;
  FLAGS_memfs_malloc_map_private = false;
  FLAGS_memfs_malloc_disable_fallback = false;
  fail_mkstemp = false;
  fail_ftruncate = false;
  fail_mmap = false;
  offset_mmap_result = false;
  forced_mmap_offset = 0;
  forced_errno = EIO;
}

void TestInitializeFailureAndFallback()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = "/tmp/path-that-does-not-exist/perftools";

  EXPECT_TRUE(!allocator.Initialize());
  EXPECT_TRUE(allocator.failed_);

  size_t actual = 0;
  void* ptr = allocator.Alloc(128, &actual, 16);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_EQ(actual, static_cast<size_t>(128));
  EXPECT_EQ(fallback.calls, 1);
  EXPECT_EQ(fallback.last_size, static_cast<size_t>(128));
  EXPECT_EQ(fallback.last_alignment, static_cast<size_t>(16));
  FreeFallbackIfAny(fallback);

  FLAGS_memfs_malloc_disable_fallback = true;
  ptr = allocator.Alloc(128, &actual, 16);
  EXPECT_TRUE(ptr == nullptr);
}

void TestInitializeSuccessAndMappedAllocation()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();

  EXPECT_TRUE(allocator.Initialize());
  EXPECT_TRUE(!allocator.failed_);

  size_t actual = 0;
  void* ptr = allocator.Alloc(4096, &actual, 64);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(actual >= 4096);
  EXPECT_EQ(fallback.calls, 0);
  std::memset(ptr, 0x5a, 128);
  munmap(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) & ~(static_cast<uintptr_t>(getpagesize()) - 1)),
         actual);
}

void TestSmallExactRequestFallsBack()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  void* ptr = allocator.Alloc(1, nullptr, 8);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_EQ(fallback.calls, 1);
  FreeFallbackIfAny(fallback);
}

void TestLimitAndNoFallbackBranches()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  FLAGS_memfs_malloc_limit_mb = 1;
  EXPECT_TRUE(allocator.Initialize());

  FLAGS_memfs_malloc_disable_fallback = true;
  size_t actual = 0;
  void* too_large = allocator.Alloc(2U * 1024U * 1024U, &actual, 4096);
  EXPECT_TRUE(too_large == nullptr);
  EXPECT_EQ(fallback.calls, 0);

  FLAGS_memfs_malloc_disable_fallback = false;
  void* fallback_ptr = allocator.Alloc(2U * 1024U * 1024U, &actual, 4096);
  EXPECT_TRUE(fallback_ptr != nullptr);
  EXPECT_EQ(fallback.calls, 1);
  FreeFallbackIfAny(fallback);
}

void TestDisableFallbackAndOverflowFallbackBranches()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  allocator.failed_ = true;
  FLAGS_memfs_malloc_disable_fallback = true;
  close(allocator.hugetlb_fd_);
  allocator.hugetlb_fd_ = -1;
  size_t actual = 0;
  EXPECT_TRUE(allocator.Alloc(128, &actual, 16) == nullptr);
  EXPECT_EQ(fallback.calls, 0);

  EXPECT_TRUE(allocator.failed_);
  allocator.failed_ = false;
  FLAGS_memfs_malloc_disable_fallback = false;
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());
  const size_t huge = static_cast<size_t>(-1) - 4095;
  void* ptr = allocator.Alloc(huge, &actual, 4096);
  EXPECT_TRUE(ptr == nullptr || ptr == fallback.last_ptr);
  EXPECT_EQ(fallback.calls, 1);
  EXPECT_EQ(fallback.last_size, huge);
  EXPECT_EQ(fallback.last_alignment, static_cast<size_t>(4096));
  FreeFallbackIfAny(fallback);
}

void TestPrivateMappingAlignmentBranches()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  FLAGS_memfs_malloc_map_private = true;
  EXPECT_TRUE(allocator.Initialize());

  size_t actual = 0;
  const size_t page = static_cast<size_t>(getpagesize());
  void* mapped = allocator.Alloc(64U * 1024U, &actual, page * 2);
  EXPECT_TRUE(mapped != nullptr);
  EXPECT_TRUE(actual >= 64U * 1024U);
  EXPECT_TRUE((reinterpret_cast<uintptr_t>(mapped) & (page * 2 - 1)) == 0);
  if (mapped != nullptr) {
    std::memset(mapped, 0x3c, 64);
  }

  EXPECT_TRUE(!allocator.failed_);
  EXPECT_EQ(fallback.calls, 0);
}

void TestLargeRequestWithoutActualSizeBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  void* ptr = allocator.Alloc(static_cast<size_t>(allocator.big_page_size_), nullptr, 8);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_EQ(fallback.calls, 0);
}

void TestInitializeMkstempFailureBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  fail_mkstemp = true;
  forced_errno = EACCES;

  EXPECT_TRUE(!allocator.Initialize());
  EXPECT_TRUE(allocator.failed_);
  EXPECT_EQ(allocator.hugetlb_fd_, -1);
}

void TestFtruncateFailureBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  fail_ftruncate = true;
  forced_errno = EPERM;
  size_t actual = 0;
  void* ptr = allocator.Alloc(4096, &actual, 4096);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(ptr == fallback.last_ptr);
  EXPECT_TRUE(allocator.failed_);
  EXPECT_EQ(fallback.calls, 1);
  FreeFallbackIfAny(fallback);
}

void TestFtruncateEinvalBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  fail_ftruncate = true;
  forced_errno = EINVAL;
  size_t actual = 0;
  void* ptr = allocator.Alloc(4096, &actual, 4096);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(actual >= 4096);
  EXPECT_TRUE(!allocator.failed_);
  EXPECT_EQ(fallback.calls, 0);
}

void TestIgnoreMmapFailBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  fail_mmap = true;
  forced_errno = ENOMEM;
  FLAGS_memfs_malloc_ignore_mmap_fail = true;
  size_t actual = 0;
  void* ptr = allocator.Alloc(4096, &actual, 4096);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(ptr == fallback.last_ptr);
  EXPECT_TRUE(!allocator.failed_);
  EXPECT_EQ(fallback.calls, 1);
  FreeFallbackIfAny(fallback);

  FLAGS_memfs_malloc_disable_fallback = true;
  ptr = allocator.Alloc(4096, &actual, 4096);
  EXPECT_TRUE(ptr == nullptr);
  EXPECT_TRUE(!allocator.failed_);
}

void TestMmapAlignmentAdjustBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  EXPECT_TRUE(allocator.Initialize());

  offset_mmap_result = true;
  forced_mmap_offset = static_cast<size_t>(allocator.big_page_size_);
  size_t actual = 0;
  void* ptr = allocator.Alloc(64U * 1024U, &actual, static_cast<size_t>(allocator.big_page_size_) * 2U);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(actual >= 64U * 1024U);
  EXPECT_TRUE((reinterpret_cast<uintptr_t>(ptr) & ((static_cast<size_t>(allocator.big_page_size_) * 2U) - 1)) == 0);
  EXPECT_TRUE(!allocator.failed_);
  EXPECT_EQ(fallback.calls, 0);
}

void TestLimitMarksAllocatorFailedBranch()
{
  ResetFlags();
  FallbackAllocator fallback;
  HugetlbSysAllocator allocator(&fallback);
  FLAGS_memfs_malloc_path = MakeTempPrefix();
  FLAGS_memfs_malloc_limit_mb = 1;
  EXPECT_TRUE(allocator.Initialize());

  const size_t page = static_cast<size_t>(allocator.big_page_size_);
  const size_t limit = static_cast<size_t>(FLAGS_memfs_malloc_limit_mb) * 1024U * 1024U;
  size_t actual = 0;
  void* first = allocator.Alloc(limit - page / 2, &actual, page);
  EXPECT_TRUE(first != nullptr);
  EXPECT_TRUE(actual >= limit - page / 2);
  EXPECT_TRUE(!allocator.failed_);

  FLAGS_memfs_malloc_disable_fallback = true;
  void* second = allocator.Alloc(page, &actual, page);
  EXPECT_TRUE(second == nullptr);
  EXPECT_TRUE(allocator.failed_);
  EXPECT_EQ(fallback.calls, 0);
}

}  // namespace

int main()
{
  TestInitializeFailureAndFallback();
  TestInitializeSuccessAndMappedAllocation();
  TestSmallExactRequestFallsBack();
  TestLimitAndNoFallbackBranches();
  TestDisableFallbackAndOverflowFallbackBranches();
  TestPrivateMappingAlignmentBranches();
  TestLargeRequestWithoutActualSizeBranch();
  TestInitializeMkstempFailureBranch();
  TestFtruncateFailureBranch();
  TestFtruncateEinvalBranch();
  TestIgnoreMmapFailBranch();
  TestMmapAlignmentAdjustBranch();
  TestLimitMarksAllocatorFailedBranch();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools memfs allocator checks failed\n", failures);
    return 1;
  }
  return 0;
}
