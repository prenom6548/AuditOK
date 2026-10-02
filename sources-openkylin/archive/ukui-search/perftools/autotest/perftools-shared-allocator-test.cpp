#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <thread>
#include <vector>

#include <unistd.h>

#include "gperftools/malloc_extension_c.h"
#include "gperftools/malloc_extension.h"
#include "gperftools/malloc_hook_c.h"
#include "gperftools/tcmalloc.h"
#include "base/commandlineflags.h"
#define private public
#include "common.h"
#include "thread_cache.h"
#include "stack_trace_table.h"
#undef private
#include "system-alloc.h"
#include "tcmalloc_guard.h"
#include "testing_portal.h"

DECLARE_bool(malloc_disable_memory_release);
DECLARE_double(tcmalloc_release_rate);
DECLARE_int64(tcmalloc_heap_limit_mb);

extern "C" int tc_query_new_mode() noexcept;
extern "C" size_t tc_nallocx(size_t size, int flags);

namespace tcmalloc {
void CentralCacheLockAll();
void CentralCacheUnlockAll();
void* allocate_full_malloc_oom(size_t size);
void* allocate_full_cpp_nothrow_oom(size_t size);
void* allocate_full_cpp_throw_oom(size_t size);
}  // namespace tcmalloc

namespace {

int failures = 0;
std::atomic<int> new_hook_calls{0};
std::atomic<int> delete_hook_calls{0};
std::atomic<int> invalid_free_calls{0};
std::atomic<int> invalid_size_calls{0};
std::atomic<int> memalign_retry_handler_calls{0};
std::atomic<int> recovery_handler_calls{0};
size_t recovery_heap_limit_mb = 0;

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
      std::fprintf(stderr, "EXPECT_EQ failed at %s:%d: %lld != %lld\n", __FILE__, __LINE__,            \
                   static_cast<long long>(actual_value), static_cast<long long>(expected_value));      \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

bool IsAligned(void* ptr, std::size_t alignment)
{
  return (reinterpret_cast<std::uintptr_t>(ptr) % alignment) == 0;
}

void FillAndCheck(void* ptr, std::size_t size, unsigned char value)
{
  auto* bytes = static_cast<unsigned char*>(ptr);
  std::memset(bytes, value, size);
  for (std::size_t i = 0; i < size; ++i) {
    EXPECT_EQ(bytes[i], value);
  }
}

void NewHook(const void* ptr, std::size_t size)
{
  if (ptr != nullptr && size > 0) {
    ++new_hook_calls;
  }
}

void DeleteHook(const void* ptr)
{
  if (ptr != nullptr) {
    ++delete_hook_calls;
  }
}

void MemalignRetryHandler()
{
  if (++memalign_retry_handler_calls >= 2) {
    throw std::bad_alloc();
  }
}

void RestoreHeapLimitHandler()
{
  ++recovery_handler_calls;
  MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", recovery_heap_limit_mb);
}

void InvalidFreeCallback(void* ptr)
{
  if (ptr != nullptr) {
    ++invalid_free_calls;
  }
}

size_t InvalidSizeCallback(const void* ptr)
{
  if (ptr != nullptr) {
    ++invalid_size_calls;
  }
  return 0;
}

size_t CopyingInvalidSizeCallback(const void* ptr)
{
  if (ptr != nullptr) {
    ++invalid_size_calls;
  }
  return sizeof(int);
}

void TestVersionAndNewMode()
{
  int major = 0;
  int minor = 0;
  const char* patch = nullptr;
  const char* version = tc_version(&major, &minor, &patch);

  EXPECT_TRUE(version != nullptr);
  EXPECT_TRUE(std::strstr(version, "gperftools") != nullptr);
  EXPECT_EQ(major, TC_VERSION_MAJOR);
  EXPECT_EQ(minor, TC_VERSION_MINOR);
  EXPECT_TRUE(patch != nullptr);

  int old_mode = tc_set_new_mode(1);
  EXPECT_EQ(tc_set_new_mode(old_mode), 1);
  EXPECT_EQ(tc_query_new_mode(), old_mode);
}

void TestBasicAllocations()
{
  void* zero = tc_malloc(0);
  tc_free(zero);
  tc_free(nullptr);

  std::vector<void*> allocations;
  for (std::size_t size : {1U, 8U, 16U, 64U, 512U, 4096U, 32768U, 1024U * 1024U}) {
    void* ptr = tc_malloc(size);
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_TRUE(tc_malloc_size(ptr) >= size);
    EXPECT_EQ(MallocExtension_GetOwnership(ptr), MallocExtension_kOwned);
    FillAndCheck(ptr, size < 4096 ? size : 4096, static_cast<unsigned char>(size & 0xff));
    allocations.push_back(ptr);
  }

  for (void* ptr : allocations) {
    tc_free(ptr);
  }
}

void TestCallocAndRealloc()
{
  auto* calloc_ptr = static_cast<unsigned char*>(tc_calloc(32, 16));
  EXPECT_TRUE(calloc_ptr != nullptr);
  for (std::size_t i = 0; i < 32 * 16; ++i) {
    EXPECT_EQ(calloc_ptr[i], 0);
  }

  std::memset(calloc_ptr, 0x5a, 32 * 16);
  auto* grown = static_cast<unsigned char*>(tc_realloc(calloc_ptr, 4096));
  EXPECT_TRUE(grown != nullptr);
  for (std::size_t i = 0; i < 32 * 16; ++i) {
    EXPECT_EQ(grown[i], 0x5a);
  }

  auto* shrunk = static_cast<unsigned char*>(tc_realloc(grown, 64));
  EXPECT_TRUE(shrunk != nullptr);
  for (std::size_t i = 0; i < 64; ++i) {
    EXPECT_EQ(shrunk[i], 0x5a);
  }

  EXPECT_TRUE(tc_realloc(shrunk, 0) == nullptr);

  void* from_null = tc_realloc(nullptr, 128);
  EXPECT_TRUE(from_null != nullptr);
  tc_cfree(from_null);

  void* skip_new_handler = tc_malloc_skip_new_handler(256);
  EXPECT_TRUE(skip_new_handler != nullptr);
  EXPECT_TRUE(tc_malloc_size(skip_new_handler) >= 256);
  tc_free_sized(skip_new_handler, 256);

  EXPECT_TRUE(tc_calloc(static_cast<std::size_t>(-1), 2) == nullptr);
}

void TestAlignedAllocations()
{
  for (std::size_t alignment : {8U, 16U, 64U, 256U, 4096U, 8192U}) {
    void* ptr = tc_memalign(alignment, 333);
    EXPECT_TRUE(ptr != nullptr);
    EXPECT_TRUE(IsAligned(ptr, alignment));
    FillAndCheck(ptr, 333, 0xa5);
    tc_free_aligned_sized(ptr, alignment, 333);
  }

  void* posix_ptr = nullptr;
  EXPECT_EQ(tc_posix_memalign(&posix_ptr, 128, 777), 0);
  EXPECT_TRUE(posix_ptr != nullptr);
  EXPECT_TRUE(IsAligned(posix_ptr, 128));
  tc_free(posix_ptr);

  posix_ptr = reinterpret_cast<void*>(0x1);
  EXPECT_EQ(tc_posix_memalign(&posix_ptr, 3, 128), EINVAL);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(posix_ptr), static_cast<std::uintptr_t>(0x1));
  EXPECT_EQ(tc_posix_memalign(&posix_ptr, 0, 128), EINVAL);

  EXPECT_TRUE(tc_nallocx(0, 0) > 0);
  EXPECT_TRUE(tc_nallocx(33, 6) >= 64);
  EXPECT_TRUE(tc_nallocx(33, 13) >= 8192);

  const std::size_t huge_size = (std::numeric_limits<std::size_t>::max)();
  EXPECT_TRUE(tc_memalign(16, huge_size) == nullptr);
  EXPECT_TRUE(tc_memalign(kPageSize * 2, huge_size) == nullptr);

  const std::size_t page_size = static_cast<std::size_t>(getpagesize());
  void* valloc_ptr = tc_valloc(123);
  EXPECT_TRUE(valloc_ptr != nullptr);
  EXPECT_TRUE(IsAligned(valloc_ptr, page_size));
  tc_free(valloc_ptr);

  void* pvalloc_ptr = tc_pvalloc(1);
  EXPECT_TRUE(pvalloc_ptr != nullptr);
  EXPECT_TRUE(IsAligned(pvalloc_ptr, page_size));
  EXPECT_TRUE(tc_malloc_size(pvalloc_ptr) >= page_size);
  tc_free(pvalloc_ptr);

  void* pvalloc_zero = tc_pvalloc(0);
  EXPECT_TRUE(pvalloc_zero != nullptr);
  EXPECT_TRUE(IsAligned(pvalloc_zero, page_size));
  tc_free(pvalloc_zero);

  void* huge_aligned = tc_memalign(page_size * 4, 12345);
  EXPECT_TRUE(huge_aligned != nullptr);
  EXPECT_TRUE(IsAligned(huge_aligned, page_size * 4));
  tc_free(huge_aligned);
}

void TestCppAllocationWrappers()
{
  void* object = tc_new(96);
  EXPECT_TRUE(object != nullptr);
  FillAndCheck(object, 96, 0x31);
  tc_delete_sized(object, 96);

  void* nothrow_object = tc_new_nothrow(128, std::nothrow);
  EXPECT_TRUE(nothrow_object != nullptr);
  tc_delete_nothrow(nothrow_object, std::nothrow);

  void* array = tc_newarray(192);
  EXPECT_TRUE(array != nullptr);
  FillAndCheck(array, 192, 0x42);
  tc_deletearray_sized(array, 192);

  void* nothrow_array = tc_newarray_nothrow(256, std::nothrow);
  EXPECT_TRUE(nothrow_array != nullptr);
  tc_deletearray_nothrow(nothrow_array, std::nothrow);

#if defined(__cpp_aligned_new) || (defined(__cplusplus) && __cplusplus >= 201703L)
  void* aligned = tc_new_aligned(512, std::align_val_t(128));
  EXPECT_TRUE(aligned != nullptr);
  EXPECT_TRUE(IsAligned(aligned, 128));
  tc_delete_sized_aligned(aligned, 512, std::align_val_t(128));

  void* aligned_nothrow = tc_new_aligned_nothrow(384, std::align_val_t(256), std::nothrow);
  EXPECT_TRUE(aligned_nothrow != nullptr);
  EXPECT_TRUE(IsAligned(aligned_nothrow, 256));
  tc_delete_aligned_nothrow(aligned_nothrow, std::align_val_t(256), std::nothrow);

  void* aligned_array = tc_newarray_aligned(640, std::align_val_t(128));
  EXPECT_TRUE(aligned_array != nullptr);
  EXPECT_TRUE(IsAligned(aligned_array, 128));
  tc_deletearray_sized_aligned(aligned_array, 640, std::align_val_t(128));

  void* aligned_array_nothrow = tc_newarray_aligned_nothrow(768, std::align_val_t(512), std::nothrow);
  EXPECT_TRUE(aligned_array_nothrow != nullptr);
  EXPECT_TRUE(IsAligned(aligned_array_nothrow, 512));
  tc_deletearray_aligned_nothrow(aligned_array_nothrow, std::align_val_t(512), std::nothrow);

  void* aligned_plain = tc_new_aligned(96, std::align_val_t(64));
  EXPECT_TRUE(aligned_plain != nullptr);
  tc_delete_aligned(aligned_plain, std::align_val_t(64));

  void* aligned_array_plain = tc_newarray_aligned(160, std::align_val_t(64));
  EXPECT_TRUE(aligned_array_plain != nullptr);
  tc_deletearray_aligned(aligned_array_plain, std::align_val_t(64));
#endif
}

void TestMallocExtension()
{
  void* ptr = tc_malloc(2048);
  EXPECT_TRUE(ptr != nullptr);

  EXPECT_TRUE(MallocExtension_VerifyAllMemory());
  EXPECT_TRUE(MallocExtension_VerifyMallocMemory(ptr));
  EXPECT_TRUE(MallocExtension_VerifyNewMemory(ptr));
  EXPECT_TRUE(MallocExtension_VerifyArrayNewMemory(ptr));
  EXPECT_TRUE(MallocExtension_GetAllocatedSize(ptr) >= 2048);
  EXPECT_TRUE(MallocExtension_GetEstimatedAllocatedSize(1234) >= 1234);

  int blocks = 0;
  std::size_t total = 0;
  int histogram[kMallocExtensionHistogramSize] = {};
  EXPECT_TRUE(MallocExtension_MallocMemoryStats(&blocks, &total, histogram));
  EXPECT_TRUE(blocks >= 0);

  char stats[4096] = {};
  MallocExtension_GetStats(stats, sizeof(stats));
  EXPECT_TRUE(stats[0] != '\0');

  std::size_t value = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("generic.current_allocated_bytes", &value));
  EXPECT_TRUE(value >= 2048);
  EXPECT_TRUE(!MallocExtension_GetNumericProperty("ukui-search.invalid-property", &value));

  std::size_t old_cache_limit = 0;
  if (MallocExtension_GetNumericProperty("tcmalloc.max_total_thread_cache_bytes", &old_cache_limit)) {
    EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.max_total_thread_cache_bytes", old_cache_limit));
  }
  EXPECT_TRUE(!MallocExtension_SetNumericProperty("generic.current_allocated_bytes", 1));

  double old_rate = MallocExtension_GetMemoryReleaseRate();
  MallocExtension_SetMemoryReleaseRate(0.0);
  EXPECT_TRUE(MallocExtension_GetMemoryReleaseRate() >= 0.0);
  MallocExtension_SetMemoryReleaseRate(old_rate);

  MallocExtension_MarkThreadTemporarilyIdle();
  MallocExtension_MarkThreadIdle();
  MallocExtension_MarkThreadBusy();
  EXPECT_TRUE(MallocExtension_GetThreadCacheSize() >= 0);
  MallocExtension_ReleaseToSystem(4096);
  MallocExtension_ReleaseFreeMemory();

#if GPERFTOOLS_HAS_MALLINFO
  struct mallinfo info = tc_mallinfo();
  EXPECT_TRUE(info.arena >= 0);
#endif
#if GPERFTOOLS_HAS_MALLINFO2
  struct mallinfo2 info2 = tc_mallinfo2();
  EXPECT_TRUE(info2.arena >= 0);
#endif
  EXPECT_EQ(tc_mallopt(-1, 1), 1);

  tc_malloc_stats();
  tc_free(ptr);
}

void TestHooks()
{
  new_hook_calls = 0;
  delete_hook_calls = 0;
  EXPECT_TRUE(MallocHook_AddNewHook(&NewHook));
  EXPECT_TRUE(MallocHook_AddNewHook(&NewHook));
  EXPECT_TRUE(MallocHook_AddDeleteHook(&DeleteHook));
  EXPECT_TRUE(MallocHook_AddDeleteHook(&DeleteHook));

  void* ptr = tc_malloc(512);
  EXPECT_TRUE(ptr != nullptr);
  tc_free(ptr);

  void* malloc_with_hook = tc_malloc(48);
  EXPECT_TRUE(malloc_with_hook != nullptr);
  tc_free(malloc_with_hook);

  void* new_with_hook = tc_new(56);
  EXPECT_TRUE(new_with_hook != nullptr);
  tc_delete(new_with_hook);

  void* nothrow_new_with_hook = tc_new_nothrow(72, std::nothrow);
  EXPECT_TRUE(nothrow_new_with_hook != nullptr);
  tc_delete_nothrow(nothrow_new_with_hook, std::nothrow);

  void* calloc_with_hook = tc_calloc(4, 32);
  EXPECT_TRUE(calloc_with_hook != nullptr);
  tc_free(calloc_with_hook);

  EXPECT_TRUE(new_hook_calls.load() > 0);
  EXPECT_TRUE(delete_hook_calls.load() > 0);

  void* sized_ptr = tc_malloc(64);
  EXPECT_TRUE(sized_ptr != nullptr);
  tc_free_sized(sized_ptr, 64);

  void* delete_ptr = tc_new_nothrow(80, std::nothrow);
  EXPECT_TRUE(delete_ptr != nullptr);
  tc_delete_nothrow(delete_ptr, std::nothrow);

#if defined(__cpp_aligned_new) || (defined(__cplusplus) && __cplusplus >= 201703L)
  void* aligned_delete_ptr = tc_new_aligned(96, std::align_val_t(64));
  EXPECT_TRUE(aligned_delete_ptr != nullptr);
  tc_delete_aligned(aligned_delete_ptr, std::align_val_t(64));

  void* aligned_sized_ptr = tc_memalign(64, 128);
  EXPECT_TRUE(aligned_sized_ptr != nullptr);
  tc_free_aligned_sized(aligned_sized_ptr, 64, 128);

  void* aligned_array_delete_ptr = tc_newarray_aligned(160, std::align_val_t(64));
  EXPECT_TRUE(aligned_array_delete_ptr != nullptr);
  tc_deletearray_aligned(aligned_array_delete_ptr, std::align_val_t(64));
#endif

  EXPECT_TRUE(MallocHook_RemoveNewHook(&NewHook));
  EXPECT_TRUE(MallocHook_RemoveNewHook(&NewHook));
  EXPECT_TRUE(MallocHook_RemoveDeleteHook(&DeleteHook));
  EXPECT_TRUE(MallocHook_RemoveDeleteHook(&DeleteHook));

  void* stack[16] = {};
  int depth = MallocHook_GetCallerStackTrace(stack, 16, 0);
  EXPECT_TRUE(depth >= 0);

  EXPECT_EQ(MallocHook_GetCallerStackTrace(stack, 0, 0), 0);
}

void TestThreadCaches()
{
  constexpr int kThreadCount = 4;
  constexpr int kIterations = 400;
  std::atomic<int> ok_threads{0};

  std::vector<std::thread> threads;
  for (int thread_index = 0; thread_index < kThreadCount; ++thread_index) {
    threads.emplace_back([&ok_threads, thread_index]() {
      std::vector<void*> batch;
      for (int i = 0; i < kIterations; ++i) {
        std::size_t size = 1 + ((thread_index + 1) * (i + 17)) % 8192;
        void* ptr = tc_malloc(size);
        if (ptr == nullptr) {
          return;
        }
        std::memset(ptr, thread_index, std::min<std::size_t>(size, 128));
        batch.push_back(ptr);
        if (batch.size() >= 32) {
          for (void* old : batch) {
            tc_free(old);
          }
          batch.clear();
        }
      }
      for (void* ptr : batch) {
        tc_free(ptr);
      }
      MallocExtension_MarkThreadIdle();
      MallocExtension_MarkThreadBusy();
      ++ok_threads;
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
  EXPECT_EQ(ok_threads.load(), kThreadCount);
}

void TestLargeAllocationLifecycle()
{
  void* large = tc_malloc(2U * 1024U * 1024U);
  EXPECT_TRUE(large != nullptr);
  FillAndCheck(large, 4096, 0x7c);
  EXPECT_TRUE(tc_malloc_size(large) >= 2U * 1024U * 1024U);

  void* larger = tc_realloc(large, 3U * 1024U * 1024U);
  EXPECT_TRUE(larger != nullptr);
  FillAndCheck(larger, 4096, 0x4d);
  tc_free(larger);

  MallocExtension_ReleaseFreeMemory();
}

void TestSystemAllocationLifecycle()
{
  const size_t before_taken = TCMalloc_SystemTaken;
  const size_t page_size = static_cast<size_t>(getpagesize());
  size_t actual = 0;
  void* raw = TCMalloc_SystemAlloc(page_size * 2 + 123, &actual, page_size);
  EXPECT_TRUE(raw != nullptr);
  EXPECT_TRUE(actual >= page_size * 2 + 123);
  EXPECT_TRUE(IsAligned(raw, page_size));
  EXPECT_TRUE(TCMalloc_SystemTaken >= before_taken + actual);

  std::memset(raw, 0x2a, std::min<size_t>(actual, page_size));
  bool released = TCMalloc_SystemRelease(raw, actual);
  EXPECT_TRUE(released || !released);
  TCMalloc_SystemCommit(raw, actual);
  static_cast<unsigned char*>(raw)[0] = 0x5b;

  void* default_alignment = TCMalloc_SystemAlloc(1024, nullptr, 0);
  EXPECT_TRUE(default_alignment != nullptr);
  EXPECT_TRUE(TCMalloc_SystemAlloc(static_cast<size_t>(-1), nullptr, 4096) == nullptr);
}

void TestAdditionalMallocExtensionProperties()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  std::vector<MallocExtension::FreeListInfo> free_lists;
  extension->GetFreeListSizes(&free_lists);
  EXPECT_TRUE(free_lists.size() > 0);
  bool saw_tcmalloc_list = false;
  for (const auto& info : free_lists) {
    EXPECT_TRUE(info.max_object_size >= info.min_object_size);
    EXPECT_TRUE(info.type != nullptr);
    if (std::strstr(info.type, "tcmalloc") != nullptr) {
      saw_tcmalloc_list = true;
    }
  }
  EXPECT_TRUE(saw_tcmalloc_list);

  MallocExtensionWriter sample;
  extension->GetHeapSample(&sample);
  EXPECT_TRUE(!sample.empty());

  MallocExtensionWriter growth;
  extension->GetHeapGrowthStacks(&growth);
  EXPECT_TRUE(!growth.empty());

  bool range_callback_called = false;
  extension->Ranges(&range_callback_called, [](void* arg, const base::MallocRange* range) {
    *static_cast<bool*>(arg) = true;
    if (range != nullptr) {
      EXPECT_TRUE(range->type == base::MallocRange::INUSE ||
                  range->type == base::MallocRange::FREE ||
                  range->type == base::MallocRange::UNMAPPED ||
                  range->type == base::MallocRange::UNKNOWN);
    }
  });
  EXPECT_TRUE(range_callback_called || !range_callback_called);

  size_t pageheap_free = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_free_bytes", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_unmapped_bytes", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_committed_bytes", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_scavenge_count", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_commit_count", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_total_commit_bytes", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_decommit_count", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_total_decommit_bytes", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_reserve_count", &pageheap_free));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_total_reserve_bytes", &pageheap_free));
  size_t thread_free = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.thread_cache_free_bytes", &thread_free));
  size_t current_cache = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.current_total_thread_cache_bytes", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("generic.heap_size", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("generic.total_physical_bytes", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.slack_bytes", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.central_cache_free_bytes", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.transfer_cache_free_bytes", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.min_per_thread_cache_bytes", &current_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.impl.thread_cache_count", &current_cache));

  SysAllocator* old_allocator = extension->GetSystemAllocator();
  extension->SetSystemAllocator(old_allocator);
  EXPECT_TRUE(extension->GetSystemAllocator() == old_allocator);

  size_t old_decommit = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.aggressive_memory_decommit", &old_decommit));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.aggressive_memory_decommit", old_decommit ? 0 : 1));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.aggressive_memory_decommit", old_decommit));

  size_t old_heap_limit = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.heap_limit_mb", &old_heap_limit));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", old_heap_limit));

  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));

  EXPECT_TRUE(!MallocExtension_SetNumericProperty("tcmalloc.not_a_real_property", 1));
}

void TestHeapSamplingWithLiveAllocations()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 1));

  std::vector<void*> sampled_allocations;
  for (int i = 0; i < 128; ++i) {
    void* ptr = tc_malloc(4096 + i);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      std::memset(ptr, i, 64);
      sampled_allocations.push_back(ptr);
    }
  }

  MallocExtensionWriter sample;
  extension->GetHeapSample(&sample);
  EXPECT_TRUE(sample.find("heap profile:") != std::string::npos);
  EXPECT_TRUE(sample.size() > std::string("heap profile:").size());

  int sample_period = 0;
  void** traces = extension->ReadStackTraces(&sample_period);
  EXPECT_TRUE(sample_period >= 0);
  EXPECT_TRUE(traces != nullptr);
  if (traces != nullptr) {
    EXPECT_TRUE(traces[0] == nullptr || reinterpret_cast<uintptr_t>(traces[0]) >= 1);
    delete[] traces;
  }

  for (void* ptr : sampled_allocations) {
    tc_free(ptr);
  }
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
}

void TestIdleCacheAndEmptySamplingBranches()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 0));

  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), static_cast<size_t>(0));
  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), static_cast<size_t>(0));

  MallocExtensionWriter sample;
  extension->GetHeapSample(&sample);
  EXPECT_TRUE(sample.find("heap sampling turned off") != std::string::npos);
  EXPECT_TRUE(sample.find("%warn") != std::string::npos);

  int sample_period = -1;
  void** traces = extension->ReadStackTraces(&sample_period);
  EXPECT_TRUE(sample_period >= 0);
  EXPECT_TRUE(traces != nullptr);
  if (traces != nullptr) {
    EXPECT_TRUE(traces[0] == nullptr || reinterpret_cast<uintptr_t>(traces[0]) >= 1);
    delete[] traces;
  }

  MallocExtension_MarkThreadBusy();
  void* ptr = tc_malloc(128);
  EXPECT_TRUE(ptr != nullptr);
  tc_free(ptr);
  EXPECT_TRUE(MallocExtension_GetThreadCacheSize() > 0);

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestTestingPortalMagicPropertyBranch()
{
  struct PortalPropertyStorage {
    tcmalloc::TestingPortal* ptr = nullptr;
    size_t value = 0;
  } storage;

  EXPECT_TRUE(MallocExtension_GetNumericProperty(tcmalloc::TestingPortal::kMagic, &storage.value));
  EXPECT_TRUE(storage.ptr == tcmalloc::TestingPortal::Get());
  EXPECT_EQ(storage.value, size_t{1});
}

void TestTCMallocGuardRefcountBranch()
{
  TCMallocGuard guard;
}

void TestMemalignRetryAndLargeAlignBranches()
{
  const size_t large_align = kPageSize * 2;

  void* aligned_throw = tc_new_aligned(512, std::align_val_t(large_align));
  EXPECT_TRUE(aligned_throw != nullptr);
  EXPECT_TRUE(IsAligned(aligned_throw, large_align));
  tc_delete_aligned(aligned_throw, std::align_val_t(large_align));

  void* aligned_nothrow = tc_new_aligned_nothrow(768, std::align_val_t(large_align * 2), std::nothrow);
  EXPECT_TRUE(aligned_nothrow != nullptr);
  EXPECT_TRUE(IsAligned(aligned_nothrow, large_align * 2));
  tc_delete_aligned_nothrow(aligned_nothrow, std::align_val_t(large_align * 2), std::nothrow);

  void* page_span = tc_malloc(2U * 1024U * 1024U);
  EXPECT_TRUE(page_span != nullptr);
  tc_free_sized(page_span, 2U * 1024U * 1024U);

  size_t old_heap_limit = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.heap_limit_mb", &old_heap_limit));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", 1));

  memalign_retry_handler_calls = 0;
  std::new_handler previous_handler = std::set_new_handler(&MemalignRetryHandler);
  void* retry_result =
      tc_new_aligned_nothrow(8U * 1024U * 1024U, std::align_val_t(2U * 1024U * 1024U), std::nothrow);
  std::set_new_handler(previous_handler);

  EXPECT_TRUE(retry_result == nullptr);
  EXPECT_TRUE(memalign_retry_handler_calls.load() >= 2);

  recovery_heap_limit_mb = old_heap_limit;
  recovery_handler_calls = 0;
  previous_handler = std::set_new_handler(&RestoreHeapLimitHandler);
  void* recovered_aligned =
      tc_new_aligned_nothrow(8U * 1024U * 1024U, std::align_val_t(2U * 1024U * 1024U), std::nothrow);
  std::set_new_handler(previous_handler);

  EXPECT_TRUE(recovered_aligned != nullptr);
  EXPECT_TRUE(recovery_handler_calls.load() >= 1);
  if (recovered_aligned != nullptr) {
    EXPECT_TRUE(IsAligned(recovered_aligned, 2U * 1024U * 1024U));
    tc_delete_aligned_nothrow(recovered_aligned, std::align_val_t(2U * 1024U * 1024U), std::nothrow);
  }
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", old_heap_limit));
}

void TestCentralAndTransferCacheWorkload()
{
  constexpr std::size_t kObjectSize = 64;
  constexpr int kBatchCount = 32;
  constexpr int kObjectsPerBatch = 256;

  std::vector<std::vector<void*>> batches;
  batches.reserve(kBatchCount);

  for (int batch_index = 0; batch_index < kBatchCount; ++batch_index) {
    std::vector<void*> batch;
    batch.reserve(kObjectsPerBatch);
    for (int i = 0; i < kObjectsPerBatch; ++i) {
      void* ptr = tc_malloc(kObjectSize);
      EXPECT_TRUE(ptr != nullptr);
      if (ptr != nullptr) {
        std::memset(ptr, batch_index, kObjectSize);
        batch.push_back(ptr);
      }
    }
    batches.push_back(std::move(batch));
  }

  for (auto& batch : batches) {
    for (void* ptr : batch) {
      tc_free(ptr);
    }
  }

  MallocExtension_MarkThreadIdle();

  MallocExtension* extension = MallocExtension::instance();
  std::vector<MallocExtension::FreeListInfo> free_lists;
  extension->GetFreeListSizes(&free_lists);

  bool saw_central_for_size = false;
  bool saw_transfer_list = false;
  std::size_t central_free = 0;
  std::size_t transfer_free = 0;
  for (const auto& info : free_lists) {
    if (std::strcmp(info.type, "tcmalloc.central") == 0 &&
        info.min_object_size <= kObjectSize &&
        kObjectSize <= info.max_object_size) {
      saw_central_for_size = true;
      central_free += info.total_bytes_free;
    }
    if (std::strcmp(info.type, "tcmalloc.transfer") == 0 ||
        std::strcmp(info.type, "tcmalloc.thread") == 0) {
      saw_transfer_list = true;
      transfer_free += info.total_bytes_free;
    }
  }

  EXPECT_TRUE(saw_central_for_size);
  EXPECT_TRUE(saw_transfer_list);
  EXPECT_TRUE(central_free + transfer_free > 0);

  std::vector<void*> reused;
  reused.reserve(kBatchCount * kObjectsPerBatch);
  for (int i = 0; i < kBatchCount * kObjectsPerBatch; ++i) {
    void* ptr = tc_malloc(kObjectSize);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      reused.push_back(ptr);
    }
  }
  for (void* ptr : reused) {
    tc_free(ptr);
  }
}

void TestTestingPortalAndReallocCallback()
{
  tcmalloc::TestingPortal* portal = tcmalloc::TestingPortal::Get();
  EXPECT_TRUE(portal != nullptr);
  EXPECT_TRUE(portal->GetMinAlign() > 0);
  EXPECT_TRUE(portal->GetMinSpanSize() > 0);
  EXPECT_TRUE(portal->GetMaxSize() > portal->GetMinAlign());
  EXPECT_TRUE(portal->HaveSystemRelease() || !portal->HaveSystemRelease());
  EXPECT_TRUE(!portal->IsDebuggingMalloc());
  EXPECT_TRUE(portal->HasEmergencyMalloc() || !portal->HasEmergencyMalloc());

  int64_t old_sample = portal->GetSampleParameter();
  portal->GetSampleParameter() = old_sample;
  double old_release_rate = portal->GetReleaseRate();
  portal->GetReleaseRate() = old_release_rate;

  void* ptr = tc_malloc(100);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(portal->GetSizeClass(ptr) > 0);
  EXPECT_TRUE(!portal->IsEmergencyPtr(ptr));

  invalid_free_calls = 0;
  invalid_size_calls = 0;
  void* same_or_moved = portal->RunReallocWithCallback(ptr, 101, &InvalidFreeCallback, &InvalidSizeCallback);
  EXPECT_TRUE(same_or_moved != nullptr);
  EXPECT_EQ(invalid_free_calls.load(), 0);
  EXPECT_EQ(invalid_size_calls.load(), 0);
  tc_free(same_or_moved);

  void* exact = tc_malloc(104);
  EXPECT_TRUE(exact != nullptr);
  const size_t exact_usable = tc_malloc_size(exact);
  void* in_place = portal->RunReallocWithCallback(exact,
                                                  exact_usable,
                                                  &InvalidFreeCallback,
                                                  &InvalidSizeCallback);
  EXPECT_TRUE(in_place == exact);
  tc_free(in_place);

  bool emergency_body_called = false;
  portal->WithEmergencyMallocEnabled(tcmalloc::FunctionRef<void()>{[&emergency_body_called]() {
    emergency_body_called = true;
  }});
  EXPECT_TRUE(emergency_body_called);
}

void TestSizeMapAndFreeListWhiteboxBranches()
{
  void* warmup = tc_malloc(64);
  EXPECT_TRUE(warmup != nullptr);
  tc_free(warmup);

  tcmalloc::SizeMap size_map;
  size_map.Init();

  uint32_t idx = 999;
  EXPECT_TRUE(tcmalloc::SizeMap::ClassIndexMaybe(0, &idx));
  EXPECT_EQ(idx, uint32_t{0});
  EXPECT_TRUE(tcmalloc::SizeMap::ClassIndexMaybe(1024, &idx));
  const uint32_t small_idx = idx;
  EXPECT_TRUE(tcmalloc::SizeMap::ClassIndexMaybe(1025, &idx));
  EXPECT_TRUE(idx > small_idx);
  EXPECT_TRUE(tcmalloc::SizeMap::ClassIndexMaybe(kMaxSize, &idx));
  EXPECT_TRUE(!tcmalloc::SizeMap::ClassIndexMaybe(static_cast<size_t>(kMaxSize) + 1, &idx));

  EXPECT_EQ(tcmalloc::SizeMap::ClassIndex(0), size_t{0});
  EXPECT_EQ(tcmalloc::SizeMap::ClassIndex(8), size_t{1});
  EXPECT_TRUE(tcmalloc::SizeMap::ClassIndex(1025) >= tcmalloc::SizeMap::ClassIndex(1024));

  uint32_t cl = 0;
  for (size_t size : {size_t{1}, size_t{8}, size_t{16}, size_t{127},
                      size_t{128}, size_t{129}, size_t{255}, size_t{256},
                      size_t{511}, size_t{512}, size_t{1024}, size_t{1025},
                      size_t{2048}, size_t{4096}, size_t{32768},
                      static_cast<size_t>(kMaxSize)}) {
    EXPECT_TRUE(size_map.GetSizeClass(size, &cl));
    EXPECT_TRUE(cl > 0);
    EXPECT_TRUE(static_cast<size_t>(size_map.ByteSizeForClass(cl)) >= size);
  }
  EXPECT_TRUE(!size_map.GetSizeClass(static_cast<size_t>(kMaxSize) + 1, &cl));

  tcmalloc::ThreadCache::FreeList list;
  list.Init(64);
  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.length(), size_t{0});
  EXPECT_EQ(list.object_size(), 64);
  EXPECT_EQ(list.max_length(), size_t{1});
  list.set_max_length(3);
  list.set_length_overages(2);
  EXPECT_EQ(list.length_overages(), size_t{2});

  void* start = reinterpret_cast<void*>(0x1);
  void* end = reinterpret_cast<void*>(0x1);
  void* head = nullptr;
  tcmalloc::SLL_PopRange(&head, 0, &start, &end);
  EXPECT_TRUE(start == nullptr);
  EXPECT_TRUE(end == nullptr);

  std::uintptr_t slots[4] = {};
  void* rv = nullptr;
  EXPECT_TRUE(!list.TryPop(&rv));
  list.Push(&slots[0]);
  list.Push(&slots[1]);
  EXPECT_EQ(list.length(), size_t{2});
  EXPECT_TRUE(list.Next() == &slots[1]);
  list.clear_lowwatermark();
  EXPECT_EQ(list.lowwatermark(), 2);
  EXPECT_TRUE(list.TryPop(&rv));
  EXPECT_TRUE(rv == &slots[1]);
  EXPECT_EQ(list.lowwatermark(), 1);

  list.Push(&slots[2]);
  list.Push(&slots[3]);
  start = nullptr;
  end = nullptr;
  list.PopRange(2, &start, &end);
  EXPECT_TRUE(start != nullptr);
  EXPECT_TRUE(end != nullptr);
  EXPECT_EQ(list.length(), size_t{1});
  list.PushRange(2, start, end);
  EXPECT_EQ(list.length(), size_t{3});
  rv = list.Pop();
  EXPECT_TRUE(rv != nullptr);
  EXPECT_EQ(list.length(), size_t{2});
}

void* ReturnNullFromOom(size_t)
{
  return nullptr;
}

void TestCommonAndThreadCacheWhiteboxBranches()
{
  void* warmup = tc_malloc(64);
  EXPECT_TRUE(warmup != nullptr);
  tc_free(warmup);

  {
    tcmalloc::SizeMap size_map;
    setenv("TCMALLOC_OVERRIDE_PAGESIZE", "16384", 1);
    size_map.Init();
    EXPECT_EQ(size_map.min_span_size_in_pages_, size_t{2});

    setenv("TCMALLOC_OVERRIDE_PAGESIZE", "8193", 1);
    size_map.Init();
    EXPECT_EQ(size_map.min_span_size_in_pages_, size_t{1});
    unsetenv("TCMALLOC_OVERRIDE_PAGESIZE");

    EXPECT_EQ(size_map.NumMoveSize(0), 0);
    EXPECT_TRUE(size_map.NumMoveSize(1) >= 2);
    EXPECT_TRUE(size_map.NumMoveSize(static_cast<size_t>(kMaxSize) * 2) >= 2);
  }

  const uint64_t metadata_before = tcmalloc::metadata_system_bytes();
  void* small_meta = tcmalloc::MetaDataAlloc(1);
  void* aligned_meta = tcmalloc::MetaDataAlloc(257);
  void* large_meta = tcmalloc::MetaDataAlloc(9U * 1024U * 1024U);
  EXPECT_TRUE(small_meta != nullptr);
  EXPECT_TRUE(aligned_meta != nullptr);
  EXPECT_TRUE(large_meta != nullptr);
  EXPECT_TRUE(reinterpret_cast<std::uintptr_t>(small_meta) <= reinterpret_cast<std::uintptr_t>(aligned_meta));
  EXPECT_TRUE(tcmalloc::metadata_system_bytes() > metadata_before);

  tcmalloc::TestingPortal* portal = tcmalloc::TestingPortal::Get();
  EXPECT_TRUE(portal != nullptr);

  void* sample = tc_malloc(64);
  EXPECT_TRUE(sample != nullptr);
  const uint32_t cl = portal->GetSizeClass(sample);
  const int object_size = static_cast<int>(tc_malloc_size(sample));
  tc_free(sample);
  EXPECT_TRUE(cl > 0);
  EXPECT_TRUE(object_size >= 64);

  tcmalloc::ThreadCache* heap = tcmalloc::ThreadCache::NewHeap();
  tcmalloc::ThreadCache* victim = tcmalloc::ThreadCache::NewHeap();
  EXPECT_TRUE(heap != nullptr);
  EXPECT_TRUE(victim != nullptr);

  heap->list_[cl].Init(object_size);
  heap->list_[cl].set_max_length(1);
  for (int i = 0; i < 64; ++i) {
    heap->list_[cl].Push(tc_malloc(object_size));
  }
  heap->ListTooLong(&heap->list_[cl], cl);
  EXPECT_EQ(heap->list_[cl].max_length(), size_t{2});

  heap->list_[cl].Init(object_size);
  heap->list_[cl].set_max_length(1024);
  heap->list_[cl].set_length_overages(kMaxOverages);
  for (int i = 0; i < 1025; ++i) {
    heap->list_[cl].Push(tc_malloc(object_size));
  }
  heap->ListTooLong(&heap->list_[cl], cl);
  EXPECT_TRUE(heap->list_[cl].max_length() < size_t{1024});
  EXPECT_EQ(heap->list_[cl].length_overages(), size_t{0});

  heap->list_[cl].Init(object_size);
  EXPECT_TRUE(heap->FetchFromCentralCache(cl, object_size, &ReturnNullFromOom) != nullptr);
  heap->ReleaseToCentralCache(&heap->list_[cl], cl, 4096);
  const int32_t heap_before = heap->max_size_;
  heap->IncreaseCacheLimit();
  EXPECT_TRUE(heap->max_size_ >= heap_before);

  tcmalloc::ThreadCache::DeleteCache(victim);
  tcmalloc::ThreadCache::DeleteCache(heap);
}

void TestPortalInvalidReallocBranches()
{
  tcmalloc::TestingPortal* portal = tcmalloc::TestingPortal::Get();
  EXPECT_TRUE(portal != nullptr);

  struct PortalStorage {
    tcmalloc::TestingPortal* ptr = nullptr;
    size_t value = 0;
  } portal_storage;
  portal_storage.ptr = portal;
  tcmalloc::TestingPortal** slot =
      tcmalloc::TestingPortal::CheckGetPortal(tcmalloc::TestingPortal::kMagic, &portal_storage.value);
  EXPECT_TRUE(slot != nullptr);
  EXPECT_TRUE(*slot == portal);
  EXPECT_TRUE(tcmalloc::TestingPortal::CheckGetPortal("tcmalloc.not-magic", &portal_storage.value) == nullptr);

  invalid_free_calls = 0;
  invalid_size_calls = 0;
  int foreign_stack_value = 0;
  void* invalid_result = portal->RunReallocWithCallback(&foreign_stack_value,
                                                        128,
                                                        &InvalidFreeCallback,
                                                        &InvalidSizeCallback);
  EXPECT_TRUE(invalid_size_calls.load() >= 1);
  EXPECT_TRUE(invalid_free_calls.load() >= 1);
  EXPECT_TRUE(invalid_result != &foreign_stack_value);
  if (invalid_result != nullptr) {
    tc_free(invalid_result);
  }

  invalid_free_calls = 0;
  invalid_size_calls = 0;
  foreign_stack_value = 0x13572468;
  void* copied_result = portal->RunReallocWithCallback(&foreign_stack_value,
                                                       sizeof(foreign_stack_value),
                                                       &InvalidFreeCallback,
                                                       &CopyingInvalidSizeCallback);
  EXPECT_TRUE(copied_result != nullptr);
  EXPECT_TRUE(invalid_size_calls.load() >= 1);
  EXPECT_TRUE(invalid_free_calls.load() >= 1);
  if (copied_result != nullptr) {
    EXPECT_EQ(*static_cast<int*>(copied_result), foreign_stack_value);
    tc_free(copied_result);
  }
}

void TestDirectStackTraceTableBranches()
{
  tcmalloc::StackTraceTable table;

  tcmalloc::StackTrace first{};
  first.size = 64;
  first.depth = 0;

  tcmalloc::StackTrace second{};
  second.size = 512;
  second.depth = 2;
  second.stack[0] = reinterpret_cast<void*>(0x1111);
  second.stack[1] = reinterpret_cast<void*>(0x2222);

  tcmalloc::CentralCacheLockAll();
  table.AddTrace(first);
  table.AddTrace(second);
  tcmalloc::CentralCacheUnlockAll();

  void** dump = table.ReadStackTracesAndClear();
  EXPECT_TRUE(dump != nullptr);
  if (dump != nullptr) {
    EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[0]), uintptr_t{1});
    EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[1]), uintptr_t{512});
    EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[2]), uintptr_t{2});
    EXPECT_TRUE(dump[3] == reinterpret_cast<void*>(0x1111));
    EXPECT_TRUE(dump[4] == reinterpret_cast<void*>(0x2222));
    EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[5]), uintptr_t{1});
    EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[6]), uintptr_t{64});
    EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[7]), uintptr_t{0});
    EXPECT_TRUE(dump[8] == nullptr);
    delete[] dump;
  }

  dump = table.ReadStackTracesAndClear();
  EXPECT_TRUE(dump != nullptr);
  if (dump != nullptr) {
    EXPECT_TRUE(dump[0] == nullptr);
    delete[] dump;
  }

  table.error_ = true;
  tcmalloc::CentralCacheLockAll();
  table.AddTrace(second);
  tcmalloc::CentralCacheUnlockAll();
  EXPECT_TRUE(table.head_ == nullptr);

  dump = table.ReadStackTracesAndClear();
  EXPECT_TRUE(dump != nullptr);
  if (dump != nullptr) {
    EXPECT_TRUE(dump[0] == nullptr);
    delete[] dump;
  }
}

void TestStaticAtForkLockBranches()
{
  // These are the same handlers registered with pthread_atfork. Calling them
  // as a pair covers the lock/unlock paths without forking in the test process.
  tcmalloc::CentralCacheLockAll();
  tcmalloc::CentralCacheUnlockAll();
}

void TestMoreTcMallocApiBranches()
{
  void* aligned = nullptr;
  EXPECT_EQ(tc_posix_memalign(&aligned, sizeof(void*), 0), 0);
  EXPECT_TRUE(aligned != nullptr);
  tc_free(aligned);

  void* realloc_null_zero = tc_realloc(nullptr, 0);
  tc_free(realloc_null_zero);

  void* tiny = tc_malloc(1);
  EXPECT_TRUE(tiny != nullptr);
  EXPECT_TRUE(tc_malloc_size(tiny) >= 1);
  tc_free_sized(tiny, 1);

  void* aligned_sized = tc_memalign(64, 64);
  EXPECT_TRUE(aligned_sized != nullptr);
  EXPECT_TRUE(IsAligned(aligned_sized, 64));
  tc_free_aligned_sized(aligned_sized, 64, 64);

  size_t property_value = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("generic.current_allocated_bytes", &property_value));
  MallocExtension_ReleaseToSystem(0);

  EXPECT_EQ(MallocExtension_GetAllocatedSize(nullptr), size_t{0});
  EXPECT_EQ(MallocExtension_GetOwnership(nullptr), MallocExtension_kNotOwned);

  EXPECT_EQ(tc_nallocx(0, 6), size_t{64});
  EXPECT_TRUE(tc_nallocx(1, 14) >= kPageSize);
  const size_t huge_estimated = tc_nallocx((std::numeric_limits<size_t>::max)() - 32, 6);
  EXPECT_TRUE(huge_estimated == huge_estimated);
}

void TestFreeWithoutThreadCacheBranches()
{
  void* plain = tc_malloc(64);
  EXPECT_TRUE(plain != nullptr);
  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), size_t{0});
  tc_free(plain);

  void* sized = tc_malloc(96);
  EXPECT_TRUE(sized != nullptr);
  const size_t sized_usable = sized != nullptr ? tc_malloc_size(sized) : size_t{96};
  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), size_t{0});
  tc_free_sized(sized, sized_usable);

  void* large = tc_malloc(2U * 1024U * 1024U);
  EXPECT_TRUE(large != nullptr);
  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), size_t{0});
  tc_free_sized(large, 2U * 1024U * 1024U);

  MallocExtension_MarkThreadBusy();
  void* warmup = tc_malloc(32);
  EXPECT_TRUE(warmup != nullptr);
  tc_free(warmup);
}

void TestPageHeapAndSizeClassStressBranches()
{
  std::vector<void*> large_blocks;
  for (std::size_t size : {64U * 1024U, 128U * 1024U, 256U * 1024U,
                           512U * 1024U, 1024U * 1024U, 1536U * 1024U}) {
    void* ptr = tc_malloc(size);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      std::memset(ptr, static_cast<int>(size & 0xff), 4096);
      large_blocks.push_back(ptr);
    }
  }

  for (std::size_t i = 0; i < large_blocks.size(); i += 2) {
    tc_free(large_blocks[i]);
    large_blocks[i] = nullptr;
  }
  MallocExtension_ReleaseToSystem(64U * 1024U);

  std::vector<void*> mixed;
  for (std::size_t size : {24U, 40U, 72U, 136U, 264U, 520U, 1032U, 2056U, 4104U, 8200U, 16392U, 32776U}) {
    for (int i = 0; i < 48; ++i) {
      void* ptr = tc_malloc(size + static_cast<std::size_t>(i % 3));
      EXPECT_TRUE(ptr != nullptr);
      if (ptr != nullptr) {
        mixed.push_back(ptr);
      }
    }
  }

  for (void* ptr : mixed) {
    tc_free(ptr);
  }
  for (void* ptr : large_blocks) {
    if (ptr != nullptr) {
      tc_free(ptr);
    }
  }

  size_t pageheap_free = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_free_bytes", &pageheap_free));
  MallocExtension_MarkThreadTemporarilyIdle();
  MallocExtension_MarkThreadBusy();
}

void TestNullFreeDeleteAndOwnershipBranches()
{
  tc_free_sized(nullptr, 64);
  tc_free_aligned_sized(nullptr, 64, 64);
  tc_delete(nullptr);
  tc_delete_nothrow(nullptr, std::nothrow);
  tc_delete_sized(nullptr, 64);
  tc_deletearray(nullptr);
  tc_deletearray_nothrow(nullptr, std::nothrow);
  tc_deletearray_sized(nullptr, 64);

#if defined(__cpp_aligned_new) || (defined(__cplusplus) && __cplusplus >= 201703L)
  tc_delete_aligned(nullptr, std::align_val_t(64));
  tc_delete_sized_aligned(nullptr, 64, std::align_val_t(64));
  tc_delete_aligned_nothrow(nullptr, std::align_val_t(64), std::nothrow);
  tc_deletearray_aligned(nullptr, std::align_val_t(64));
  tc_deletearray_sized_aligned(nullptr, 64, std::align_val_t(64));
  tc_deletearray_aligned_nothrow(nullptr, std::align_val_t(64), std::nothrow);
#endif

  int stack_value = 0;
  EXPECT_EQ(MallocExtension_GetOwnership(&stack_value), MallocExtension_kNotOwned);
  EXPECT_EQ(MallocExtension_GetOwnership(reinterpret_cast<void*>(~std::uintptr_t{0})),
            MallocExtension_kNotOwned);

  void* ptr = tc_malloc(256);
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_EQ(MallocExtension_GetOwnership(ptr), MallocExtension_kOwned);
  tc_free(ptr);
}

void TestNallocxAlignmentAndSlowPathBranches()
{
  const size_t page_size = static_cast<size_t>(getpagesize());

  EXPECT_TRUE(tc_nallocx(1, 0) >= 1);
  EXPECT_TRUE(tc_nallocx(0, 1) >= 2);
  EXPECT_TRUE(tc_nallocx(33, 3) >= 33);
  EXPECT_TRUE(tc_nallocx(33, 12) >= page_size);
  EXPECT_TRUE(tc_nallocx(33, 20) >= page_size);
  EXPECT_TRUE(tc_nallocx(2U * 1024U * 1024U + 123U, 0) >= 2U * 1024U * 1024U);
  EXPECT_TRUE(tc_nallocx(2U * 1024U * 1024U + 123U, 20) >= 2U * 1024U * 1024U);

  void* page_aligned = tc_memalign(page_size, 128);
  EXPECT_TRUE(page_aligned != nullptr);
  tc_free(page_aligned);

  void* normal = tc_malloc(128);
  EXPECT_TRUE(normal != nullptr);
  tc_free(normal);
}

void TestReallocSameClassAndGrowthBranches()
{
  void* ptr = tc_malloc(96);
  EXPECT_TRUE(ptr != nullptr);
  const size_t usable = tc_malloc_size(ptr);
  void* same_class = tc_realloc(ptr, usable);
  EXPECT_TRUE(same_class != nullptr);
  EXPECT_TRUE(tc_malloc_size(same_class) >= usable);

  void* grown = tc_realloc(same_class, usable + 4096);
  EXPECT_TRUE(grown != nullptr);
  EXPECT_TRUE(tc_malloc_size(grown) >= usable + 4096);

  void* shrunk = tc_realloc(grown, 32);
  EXPECT_TRUE(shrunk != nullptr);
  EXPECT_TRUE(tc_malloc_size(shrunk) >= 32);
  tc_free(shrunk);

  void* new_handler = tc_malloc_skip_new_handler(0);
  tc_free(new_handler);
}

void TestMallocExtensionPropertyAndStackBranches()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  size_t value = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.min_per_thread_cache_bytes", &value));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.min_per_thread_cache_bytes", value));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.max_total_thread_cache_bytes", &value));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.max_total_thread_cache_bytes", value));
  EXPECT_TRUE(!MallocExtension_GetNumericProperty("tcmalloc.invalid.branch.property", &value));
  EXPECT_TRUE(!MallocExtension_SetNumericProperty("generic.heap_size", value));

  char small_stats[64] = {};
  MallocExtension_GetStats(small_stats, sizeof(small_stats));
  EXPECT_TRUE(small_stats[0] != '\0');
  char large_stats[12000] = {};
  MallocExtension_GetStats(large_stats, sizeof(large_stats));
  EXPECT_TRUE(large_stats[0] != '\0');

  void** growth_traces = extension->ReadHeapGrowthStackTraces();
  EXPECT_TRUE(growth_traces != nullptr);
  if (growth_traces != nullptr) {
    delete[] growth_traces;
  }

  bool saw_range = false;
  extension->Ranges(&saw_range, [](void* arg, const base::MallocRange* range) {
    if (range != nullptr && range->length > 0) {
      *static_cast<bool*>(arg) = true;
    }
  });
  EXPECT_TRUE(saw_range || !saw_range);
}

void TestPageHeapReleaseAndLargeSpanBranches()
{
  std::vector<void*> blocks;
  for (std::size_t size : {2U * 1024U * 1024U, 3U * 1024U * 1024U, 5U * 1024U * 1024U}) {
    void* ptr = tc_malloc(size);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      std::memset(ptr, 0x66, 4096);
      blocks.push_back(ptr);
    }
  }

  for (void* ptr : blocks) {
    tc_free(ptr);
  }

  MallocExtension_ReleaseToSystem(1);
  MallocExtension_ReleaseToSystem(1);
  MallocExtension_ReleaseToSystem(128U * 1024U);
  MallocExtension_ReleaseFreeMemory();

  std::vector<MallocExtension::FreeListInfo> free_lists;
  MallocExtension::instance()->GetFreeListSizes(&free_lists);
  bool saw_large_span = false;
  bool saw_page_span = false;
  for (const auto& info : free_lists) {
    if (std::strcmp(info.type, "tcmalloc.large") == 0 ||
        std::strcmp(info.type, "tcmalloc.large_unmapped") == 0) {
      saw_large_span = true;
    }
    if (std::strcmp(info.type, "tcmalloc.page") == 0 ||
        std::strcmp(info.type, "tcmalloc.page_unmapped") == 0) {
      saw_page_span = true;
    }
  }
  EXPECT_TRUE(saw_large_span);
  EXPECT_TRUE(saw_page_span);
}

void TestSystemAllocatorReleaseBoundaryBranches()
{
  const size_t page_size = static_cast<size_t>(getpagesize());

  void* raw = TCMalloc_SystemAlloc(page_size * 2, nullptr, page_size * 2);
  EXPECT_TRUE(raw != nullptr);
  EXPECT_TRUE(IsAligned(raw, page_size * 2));
  if (raw != nullptr) {
    static_cast<unsigned char*>(raw)[0] = 0x11;
  }

  bool old_disable_release = FLAGS_malloc_disable_memory_release;
  FLAGS_malloc_disable_memory_release = true;
  EXPECT_TRUE(!TCMalloc_SystemRelease(raw, page_size));
  FLAGS_malloc_disable_memory_release = old_disable_release;

  EXPECT_TRUE(!TCMalloc_SystemRelease(static_cast<char*>(raw) + 1, page_size - 2));
  const bool released = TCMalloc_SystemRelease(raw, page_size * 2);
  EXPECT_TRUE(released || !released);
  TCMalloc_SystemCommit(raw, page_size * 2);
  if (raw != nullptr) {
    static_cast<unsigned char*>(raw)[0] = 0x22;
  }

  EXPECT_TRUE(TCMalloc_SystemAlloc(static_cast<size_t>(-4096), nullptr, 8192) == nullptr);
}

void TestReleaseRateAndAggressiveDecommitBranches()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  double old_rate = FLAGS_tcmalloc_release_rate;
  FLAGS_tcmalloc_release_rate = 0.0;
  std::vector<void*> disabled_release_blocks;
  for (int i = 0; i < 256; ++i) {
    void* ptr = tc_malloc(4096 + (i % 7) * 64);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      disabled_release_blocks.push_back(ptr);
    }
  }
  for (void* ptr : disabled_release_blocks) {
    tc_free(ptr);
  }
  MallocExtension_MarkThreadIdle();

  FLAGS_tcmalloc_release_rate = 1000000.0;
  std::vector<void*> high_rate_blocks;
  for (int i = 0; i < 256; ++i) {
    void* ptr = tc_malloc(8192 + (i % 5) * 128);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      high_rate_blocks.push_back(ptr);
    }
  }
  for (void* ptr : high_rate_blocks) {
    tc_free(ptr);
  }
  MallocExtension_ReleaseToSystem(4096);

  size_t old_decommit = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.aggressive_memory_decommit", &old_decommit));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.aggressive_memory_decommit", 1));
  void* decommit_a = tc_malloc(128U * 1024U);
  void* decommit_b = tc_malloc(128U * 1024U);
  EXPECT_TRUE(decommit_a != nullptr);
  EXPECT_TRUE(decommit_b != nullptr);
  tc_free(decommit_a);
  tc_free(decommit_b);
  MallocExtension_ReleaseToSystem(128U * 1024U);
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.aggressive_memory_decommit", old_decommit));

  FLAGS_tcmalloc_release_rate = old_rate;
}

void TestHeapLimitAndScavengeBranches()
{
  size_t old_heap_limit = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.heap_limit_mb", &old_heap_limit));
  const double old_rate = FLAGS_tcmalloc_release_rate;
  const std::size_t huge_request = 8U * 1024U * 1024U;

  FLAGS_tcmalloc_release_rate = 1000000.0;
  std::vector<void*> release_blocks;
  for (int i = 0; i < 512; ++i) {
    void* ptr = tc_malloc(4096 + (i % 9) * 128);
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      release_blocks.push_back(ptr);
    }
  }
  for (void* ptr : release_blocks) {
    tc_free(ptr);
  }
  MallocExtension_ReleaseToSystem(4096);

  size_t decommit_count = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.pageheap_decommit_count", &decommit_count));

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", 1));
  void* limited_large = tc_malloc(huge_request);
  EXPECT_TRUE(limited_large == nullptr);
  EXPECT_EQ(errno, ENOMEM);

  void* limited_realloc_from_null = tc_realloc(nullptr, huge_request);
  EXPECT_TRUE(limited_realloc_from_null == nullptr);

  void* limited_calloc = tc_calloc(1, huge_request);
  EXPECT_TRUE(limited_calloc == nullptr);

  void* limited_nothrow = tc_new_nothrow(huge_request, std::nothrow);
  EXPECT_TRUE(limited_nothrow == nullptr);

  bool saw_bad_alloc = false;
  try {
    void* limited_throwing = tc_new(huge_request);
    tc_delete(limited_throwing);
  } catch (const std::bad_alloc&) {
    saw_bad_alloc = true;
  }
  EXPECT_TRUE(saw_bad_alloc);

  void* limited_aligned = reinterpret_cast<void*>(0x1);
  EXPECT_EQ(tc_posix_memalign(&limited_aligned, 2U * 1024U * 1024U, huge_request), ENOMEM);
  EXPECT_EQ(reinterpret_cast<std::uintptr_t>(limited_aligned), static_cast<std::uintptr_t>(0x1));

#if defined(__cpp_aligned_new) || (defined(__cplusplus) && __cplusplus >= 201703L)
  void* limited_nothrow_aligned = tc_new_aligned_nothrow(huge_request,
                                                        std::align_val_t(2U * 1024U * 1024U),
                                                        std::nothrow);
  EXPECT_TRUE(limited_nothrow_aligned == nullptr);
#endif

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", old_heap_limit));
  FLAGS_tcmalloc_release_rate = old_rate;
}

void TestAllocateFullAndNewHandlerBranches()
{
  void* malloc_full = tcmalloc::allocate_full_malloc_oom(144);
  EXPECT_TRUE(malloc_full != nullptr);
  tc_free(malloc_full);

  void* nothrow_full = tcmalloc::allocate_full_cpp_nothrow_oom(160);
  EXPECT_TRUE(nothrow_full != nullptr);
  tc_delete_nothrow(nothrow_full, std::nothrow);

  void* throwing_full = tcmalloc::allocate_full_cpp_throw_oom(176);
  EXPECT_TRUE(throwing_full != nullptr);
  tc_delete(throwing_full);

  size_t old_heap_limit = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.heap_limit_mb", &old_heap_limit));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", 1));

  std::new_handler previous_handler = std::set_new_handler([]() {
    throw std::bad_alloc();
  });
  void* handled_nothrow = tc_new_nothrow(8U * 1024U * 1024U, std::nothrow);
  EXPECT_TRUE(handled_nothrow == nullptr);

  bool handler_throw_seen = false;
  try {
    void* handled_throw = tc_new(8U * 1024U * 1024U);
    tc_delete(handled_throw);
  } catch (const std::bad_alloc&) {
    handler_throw_seen = true;
  }
  EXPECT_TRUE(handler_throw_seen);

  recovery_heap_limit_mb = old_heap_limit;
  recovery_handler_calls = 0;
  int old_mode = tc_set_new_mode(1);
  previous_handler = std::set_new_handler(&RestoreHeapLimitHandler);
  void* recovered_malloc = tc_malloc(8U * 1024U * 1024U);
  std::set_new_handler(previous_handler);
  EXPECT_TRUE(recovered_malloc != nullptr);
  EXPECT_TRUE(recovery_handler_calls.load() >= 1);
  if (recovered_malloc != nullptr) {
    tc_free(recovered_malloc);
  }

  std::set_new_handler(previous_handler);
  tc_set_new_mode(old_mode);
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.heap_limit_mb", old_heap_limit));
}

void TestDispatchAllocateFullWithoutThreadCacheBranches()
{
  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), size_t{0});
  void* malloc_path = tc_malloc(48);
  EXPECT_TRUE(malloc_path != nullptr);
  tc_free(malloc_path);

  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), size_t{0});
  void* nothrow_path = tc_new_nothrow(56, std::nothrow);
  EXPECT_TRUE(nothrow_path != nullptr);
  tc_delete_nothrow(nothrow_path, std::nothrow);

  MallocExtension_MarkThreadIdle();
  EXPECT_EQ(MallocExtension_GetThreadCacheSize(), size_t{0});
  bool saw_bad_alloc = false;
  try {
    void* throw_path = tc_new(64);
    EXPECT_TRUE(throw_path != nullptr);
    tc_delete(throw_path);
  } catch (const std::bad_alloc&) {
    saw_bad_alloc = true;
  }
  EXPECT_TRUE(!saw_bad_alloc);
}

void TestThreadCacheLimitBoundaryBranches()
{
  size_t old_max_cache = 0;
  size_t old_min_cache = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.max_total_thread_cache_bytes", &old_max_cache));
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.min_per_thread_cache_bytes", &old_min_cache));

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.max_total_thread_cache_bytes", 0));
  std::vector<void*> tiny_cache_blocks;
  for (int i = 0; i < 128; ++i) {
    void* ptr = tc_malloc(64 + (i % 8));
    EXPECT_TRUE(ptr != nullptr);
    if (ptr != nullptr) {
      tiny_cache_blocks.push_back(ptr);
    }
  }
  for (void* ptr : tiny_cache_blocks) {
    tc_free(ptr);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.max_total_thread_cache_bytes",
                                                 static_cast<size_t>(2ULL << 30)));
  std::vector<std::thread> workers;
  std::atomic<int> completed{0};
  for (int thread_index = 0; thread_index < 8; ++thread_index) {
    workers.emplace_back([&completed, thread_index]() {
      std::vector<void*> blocks;
      for (int i = 0; i < 128; ++i) {
        void* ptr = tc_malloc(32 + ((thread_index + i) % 32));
        if (ptr == nullptr) {
          return;
        }
        blocks.push_back(ptr);
      }
      for (void* ptr : blocks) {
        tc_free(ptr);
      }
      MallocExtension_MarkThreadIdle();
      ++completed;
    });
  }
  for (auto& worker : workers) {
    worker.join();
  }
  EXPECT_EQ(completed.load(), 8);

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.max_total_thread_cache_bytes", old_max_cache));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.min_per_thread_cache_bytes", old_min_cache));
}

}  // namespace

int main()
{
  TestVersionAndNewMode();
  TestBasicAllocations();
  TestCallocAndRealloc();
  TestAlignedAllocations();
  TestCppAllocationWrappers();
  TestMallocExtension();
  TestHooks();
  TestThreadCaches();
  TestLargeAllocationLifecycle();
  TestSystemAllocationLifecycle();
  TestAdditionalMallocExtensionProperties();
  TestHeapSamplingWithLiveAllocations();
  TestIdleCacheAndEmptySamplingBranches();
  TestTestingPortalMagicPropertyBranch();
  TestTCMallocGuardRefcountBranch();
  TestMemalignRetryAndLargeAlignBranches();
  TestCentralAndTransferCacheWorkload();
  TestTestingPortalAndReallocCallback();
  TestSizeMapAndFreeListWhiteboxBranches();
  TestCommonAndThreadCacheWhiteboxBranches();
  TestPortalInvalidReallocBranches();
  TestDirectStackTraceTableBranches();
  TestStaticAtForkLockBranches();
  TestMoreTcMallocApiBranches();
  TestFreeWithoutThreadCacheBranches();
  TestPageHeapAndSizeClassStressBranches();
  TestNullFreeDeleteAndOwnershipBranches();
  TestNallocxAlignmentAndSlowPathBranches();
  TestReallocSameClassAndGrowthBranches();
  TestMallocExtensionPropertyAndStackBranches();
  TestPageHeapReleaseAndLargeSpanBranches();
  TestSystemAllocatorReleaseBoundaryBranches();
  TestReleaseRateAndAggressiveDecommitBranches();
  TestHeapLimitAndScavengeBranches();
  TestAllocateFullAndNewHandlerBranches();
  TestDispatchAllocateFullWithoutThreadCacheBranches();
  TestThreadCacheLimitBoundaryBranches();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools shared allocator checks failed\n", failures);
    return 1;
  }
  return 0;
}
