#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>
#include <vector>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>

#include "base/logging.h"
#include "base/spinlock.h"
#include "base/spinlock_internal.h"
#include "base/commandlineflags.h"
#include "common.h"
#include "malloc_hook-inl.h"
#include "sampler.h"
#include "stack_trace_table.h"
#include "system-alloc.h"
#include "gperftools/malloc_hook.h"
#include "gperftools/malloc_extension.h"
#include "gperftools/malloc_hook.h"

DECLARE_int64(tcmalloc_sample_parameter);
DECLARE_int32(verbose);
DECLARE_bool(malloc_skip_sbrk);
DECLARE_bool(malloc_skip_mmap);
DECLARE_bool(malloc_disable_memory_release);

SysAllocator::~SysAllocator() {}

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

struct TraceNode
{
  tcmalloc::StackTrace trace{};
  const TraceNode* next = nullptr;
};

std::atomic<int> new_hook_calls{0};
std::atomic<int> delete_hook_calls{0};

const tcmalloc::StackTrace* NextTrace(const void** current)
{
  const auto* node = static_cast<const TraceNode*>(*current);
  *current = node->next;
  return &node->trace;
}

void NewHook(const void* ptr, size_t size)
{
  EXPECT_TRUE(ptr != nullptr);
  EXPECT_TRUE(size > 0);
  ++new_hook_calls;
}

void DeleteHook(const void* ptr)
{
  EXPECT_TRUE(ptr != nullptr);
  ++delete_hook_calls;
}

}  // namespace

namespace tcmalloc {

class SamplerTest
{
public:
  static ssize_t BytesUntilSample(const Sampler& sampler) { return sampler.bytes_until_sample_; }
  static bool Initialized(const Sampler& sampler) { return sampler.initialized_; }
};

}  // namespace tcmalloc

namespace {

void TestSamplerDisabledAndFastPathRollback()
{
  FLAGS_tcmalloc_sample_parameter = 0;

  tcmalloc::Sampler sampler;
  sampler.Init(12345);
  EXPECT_TRUE(tcmalloc::SamplerTest::Initialized(sampler) == false);
  EXPECT_TRUE(tcmalloc::SamplerTest::BytesUntilSample(sampler) > 0);
  EXPECT_EQ(tcmalloc::Sampler::GetSamplePeriod(), 0);

  const ssize_t before_fast = tcmalloc::SamplerTest::BytesUntilSample(sampler);
  EXPECT_TRUE(sampler.TryRecordAllocationFast(1));
  EXPECT_EQ(tcmalloc::SamplerTest::BytesUntilSample(sampler), before_fast - 1);

  EXPECT_TRUE(!sampler.TryRecordAllocationFast(static_cast<size_t>(before_fast)));
  EXPECT_EQ(tcmalloc::SamplerTest::BytesUntilSample(sampler), before_fast - 1);

  EXPECT_TRUE(sampler.RecordAllocation(static_cast<size_t>(tcmalloc::SamplerTest::BytesUntilSample(sampler)) + 1));
  EXPECT_TRUE(tcmalloc::SamplerTest::Initialized(sampler));
  EXPECT_TRUE(tcmalloc::SamplerTest::BytesUntilSample(sampler) >= 0);
}

void TestSamplerEnabledSamplingPath()
{
  FLAGS_tcmalloc_sample_parameter = 1;

  tcmalloc::Sampler sampler;
  sampler.Init(6789);
  EXPECT_EQ(tcmalloc::Sampler::GetSamplePeriod(), 1);

  bool saw_sample = false;
  for (int i = 0; i < 200 && !saw_sample; ++i) {
    saw_sample = !sampler.RecordAllocation(1024);
  }
  EXPECT_TRUE(saw_sample);
  EXPECT_TRUE(tcmalloc::SamplerTest::BytesUntilSample(sampler) >= 0);

  const ssize_t next = sampler.PickNextSamplingPoint();
  EXPECT_TRUE(next >= 0);
}

void TestSamplerRandomSequenceIsDeterministic()
{
  uint64_t value = 1;
  value = tcmalloc::Sampler::NextRandom(value);
  EXPECT_EQ(value, uint64_t{0x5deece678ULL});
  value = tcmalloc::Sampler::NextRandom(value);
  EXPECT_EQ(value, uint64_t{0xbb61488df123ULL});
}

void TestStackTraceDumpEmptyList()
{
  std::unique_ptr<void*[]> dump = tcmalloc::ProduceStackTracesDump(&NextTrace, nullptr);
  EXPECT_TRUE(dump != nullptr);
  EXPECT_TRUE(dump[0] == nullptr);
}

void TestStackTraceDumpMultipleEntries()
{
  TraceNode second;
  second.trace.size = 2048;
  second.trace.depth = 1;
  second.trace.stack[0] = reinterpret_cast<void*>(0x3333);

  TraceNode first;
  first.trace.size = 512;
  first.trace.depth = 2;
  first.trace.stack[0] = reinterpret_cast<void*>(0x1111);
  first.trace.stack[1] = reinterpret_cast<void*>(0x2222);
  first.next = &second;

  std::unique_ptr<void*[]> dump = tcmalloc::ProduceStackTracesDump(&NextTrace, &first);
  EXPECT_TRUE(dump != nullptr);
  EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[0]), uintptr_t{1});
  EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[1]), uintptr_t{512});
  EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[2]), uintptr_t{2});
  EXPECT_EQ(dump[3], reinterpret_cast<void*>(0x1111));
  EXPECT_EQ(dump[4], reinterpret_cast<void*>(0x2222));
  EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[5]), uintptr_t{1});
  EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[6]), uintptr_t{2048});
  EXPECT_EQ(reinterpret_cast<uintptr_t>(dump[7]), uintptr_t{1});
  EXPECT_EQ(dump[8], reinterpret_cast<void*>(0x3333));
  EXPECT_TRUE(dump[9] == nullptr);
}

void TestLoggingRawFileAndVerboseBranches()
{
  char tmp_template[] = "/tmp/ukui-search-perftools-log-XXXXXX";
  int fd = mkstemp(tmp_template);
  EXPECT_TRUE(fd >= 0);
  if (fd >= 0) {
    close(fd);
  }

  RawFD raw = RawOpenForWriting(tmp_template);
  EXPECT_TRUE(raw != kIllegalRawFD);
  const char payload[] = "raw-write";
  RawWrite(raw, payload, sizeof(payload) - 1);
  RawWrite(raw, "", 0);
  RawClose(raw);

  FILE* in = std::fopen(tmp_template, "rb");
  EXPECT_TRUE(in != nullptr);
  if (in != nullptr) {
    char buffer[32] = {};
    const size_t read = std::fread(buffer, 1, sizeof(buffer), in);
    EXPECT_EQ(read, sizeof(payload) - 1);
    EXPECT_TRUE(std::memcmp(buffer, payload, sizeof(payload) - 1) == 0);
    std::fclose(in);
  }
  std::remove(tmp_template);

  FLAGS_verbose = -10;
  RAW_LOG(INFO, "this message is filtered");
  LOG_IF(INFO, false, "this message is also filtered");
  EXPECT_TRUE(!VLOG_IS_ON(INFO));

  FLAGS_verbose = 1;
  RAW_LOG(INFO, "perftools whitebox info");
  RAW_VLOG(INFO, "perftools whitebox vlog");
  LOG_IF(INFO, true, "perftools whitebox conditional");
  EXPECT_TRUE(VLOG_IS_ON(INFO));
}

void TestSpinLockFastSlowAndInternalDelayBranches()
{
  SpinLock lock;
  EXPECT_TRUE(!lock.IsHeld());
  EXPECT_TRUE(lock.TryLock());
  EXPECT_TRUE(lock.IsHeld());
  EXPECT_TRUE(!lock.TryLock());
  lock.Unlock();
  EXPECT_TRUE(!lock.IsHeld());

  {
    SpinLockHolder holder(&lock);
    EXPECT_TRUE(lock.IsHeld());
  }
  EXPECT_TRUE(!lock.IsHeld());

  tcmalloc::TrivialOnce once;
  int once_calls = 0;
  EXPECT_TRUE(once.RunOnce([&]() { ++once_calls; }));
  EXPECT_TRUE(!once.RunOnce([&]() { ++once_calls; }));
  EXPECT_EQ(once_calls, 1);

  std::atomic<int> word{1};
  base::internal::SpinLockDelay(&word, 1, 0);
  word.store(0);
  base::internal::SpinLockDelay(&word, 1, -1);
  base::internal::SpinLockDelay(&word, 1, 1);
  base::internal::SpinLockDelay(&word, 1, 8);
  base::internal::SpinLockDelay(&word, 1, 33);
  base::internal::SpinLockWake(&word, false);
  base::internal::SpinLockWake(&word, true);
}

void TestSpinLockContentionCoversSlowLockAndWake()
{
  SpinLock lock;
  std::promise<void> locked;
  std::promise<void> release;
  auto release_future = release.get_future();

  std::thread holder([&]() {
    lock.Lock();
    locked.set_value();
    release_future.wait();
    lock.Unlock();
  });

  locked.get_future().wait();
  EXPECT_TRUE(lock.IsHeld());
  release.set_value();
  lock.Lock();
  EXPECT_TRUE(lock.IsHeld());
  lock.Unlock();
  holder.join();
}

void TestSystemAllocFailureAndReleaseBranches()
{
  const bool old_skip_sbrk = FLAGS_malloc_skip_sbrk;
  const bool old_skip_mmap = FLAGS_malloc_skip_mmap;
  const bool old_disable_release = FLAGS_malloc_disable_memory_release;

  FLAGS_malloc_skip_sbrk = true;
  FLAGS_malloc_skip_mmap = true;
  size_t actual = 0;
  EXPECT_TRUE(TCMalloc_SystemAlloc(4096, &actual, 4096) == nullptr);

  FLAGS_malloc_skip_sbrk = old_skip_sbrk;
  FLAGS_malloc_skip_mmap = old_skip_mmap;

  const size_t page = static_cast<size_t>(getpagesize());
  actual = 0;
  void* raw = TCMalloc_SystemAlloc(page * 2, &actual, page * 2);
  EXPECT_TRUE(raw != nullptr);
  EXPECT_TRUE(actual >= page * 2);
  EXPECT_TRUE((reinterpret_cast<uintptr_t>(raw) & (page * 2 - 1)) == 0);

  FLAGS_malloc_disable_memory_release = true;
  EXPECT_TRUE(!TCMalloc_SystemRelease(raw, page));
  FLAGS_malloc_disable_memory_release = false;
  EXPECT_TRUE(!TCMalloc_SystemRelease(static_cast<char*>(raw) + 1, page - 2));

  const bool released = TCMalloc_SystemRelease(raw, page);
  EXPECT_TRUE(released || !released);
  TCMalloc_SystemCommit(raw, page);

  FLAGS_malloc_skip_sbrk = false;
  FLAGS_malloc_skip_mmap = true;
  actual = 0;
  void* sbrk_only = TCMalloc_SystemAlloc(123, &actual, 64);
  EXPECT_TRUE(sbrk_only != nullptr || sbrk_only == nullptr);
  if (sbrk_only != nullptr) {
    EXPECT_TRUE(actual >= 123);
    EXPECT_TRUE((reinterpret_cast<uintptr_t>(sbrk_only) & (64 - 1)) == 0);
  }

  FLAGS_malloc_skip_sbrk = true;
  FLAGS_malloc_skip_mmap = false;
  actual = 0;
  void* mmap_only = TCMalloc_SystemAlloc(321, &actual, page * 2);
  if (mmap_only != nullptr) {
    EXPECT_TRUE(actual >= 321);
    EXPECT_TRUE((reinterpret_cast<uintptr_t>(mmap_only) & ((page * 2) - 1)) == 0);
    const bool mmap_released = TCMalloc_SystemRelease(mmap_only, actual);
    EXPECT_TRUE(mmap_released || !mmap_released);
    TCMalloc_SystemCommit(mmap_only, actual);
  }

  FLAGS_malloc_skip_sbrk = false;
  FLAGS_malloc_skip_mmap = false;
  EXPECT_TRUE(TCMalloc_SystemAlloc(static_cast<size_t>(-4096), &actual, 8192) == nullptr);

  actual = 0;
  void* min_aligned = TCMalloc_SystemAlloc(17, &actual, 1);
  EXPECT_TRUE(min_aligned != nullptr);
  if (min_aligned != nullptr) {
    EXPECT_TRUE(actual >= 17);
    EXPECT_TRUE((reinterpret_cast<uintptr_t>(min_aligned) & (sizeof(MemoryAligner) - 1)) == 0);
  }

  FLAGS_malloc_disable_memory_release = old_disable_release;
  FLAGS_malloc_skip_sbrk = old_skip_sbrk;
  FLAGS_malloc_skip_mmap = old_skip_mmap;
}

void TestMallocHookListAndNoopBranches()
{
  new_hook_calls = 0;
  delete_hook_calls = 0;

  EXPECT_TRUE(!MallocHook_AddNewHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveNewHook(nullptr));
  EXPECT_TRUE(!MallocHook_AddDeleteHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveDeleteHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveNewHook(&NewHook));
  EXPECT_TRUE(!MallocHook_RemoveDeleteHook(&DeleteHook));

  for (int i = 0; i < base::internal::kHookListMaxValues; ++i) {
    EXPECT_TRUE(MallocHook_AddNewHook(&NewHook));
    EXPECT_TRUE(MallocHook_AddDeleteHook(&DeleteHook));
  }
  EXPECT_TRUE(!MallocHook_AddNewHook(&NewHook));
  EXPECT_TRUE(!MallocHook_AddDeleteHook(&DeleteHook));

  int local = 0;
  tcmalloc::InvokeNewHook(&local, sizeof(local));
  tcmalloc::InvokeDeleteHook(&local);
  EXPECT_EQ(new_hook_calls.load(), base::internal::kHookListMaxValues);
  EXPECT_EQ(delete_hook_calls.load(), base::internal::kHookListMaxValues);

  for (int i = 0; i < base::internal::kHookListMaxValues; ++i) {
    EXPECT_TRUE(MallocHook_RemoveNewHook(&NewHook));
    EXPECT_TRUE(MallocHook_RemoveDeleteHook(&DeleteHook));
  }
  EXPECT_TRUE(!MallocHook_RemoveNewHook(&NewHook));
  EXPECT_TRUE(!MallocHook_RemoveDeleteHook(&DeleteHook));

  EXPECT_TRUE(MallocHook_SetNewHook(&NewHook) == nullptr);
  EXPECT_TRUE(MallocHook_SetDeleteHook(&DeleteHook) == nullptr);
  tcmalloc::InvokeNewHook(&local, sizeof(local));
  tcmalloc::InvokeDeleteHook(&local);
  EXPECT_TRUE(MallocHook_SetNewHook(nullptr) == &NewHook);
  EXPECT_TRUE(MallocHook_SetDeleteHook(nullptr) == &DeleteHook);
  EXPECT_TRUE(base::internal::new_hooks_.empty());
  EXPECT_TRUE(base::internal::delete_hooks_.empty());

  void* stack[8] = {};
  EXPECT_EQ(MallocHook_GetCallerStackTrace(stack, 0, 0), 0);
  EXPECT_TRUE(MallocHook_GetCallerStackTrace(stack, 8, 0) >= 0);

  EXPECT_TRUE(!MallocHook_AddPreMmapHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemovePreMmapHook(nullptr));
  EXPECT_TRUE(!MallocHook_SetMmapReplacement(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveMmapReplacement(nullptr));
  EXPECT_TRUE(!MallocHook_AddMmapHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveMmapHook(nullptr));
  EXPECT_TRUE(!MallocHook_AddMunmapHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveMunmapHook(nullptr));
  EXPECT_TRUE(!MallocHook_SetMunmapReplacement(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveMunmapReplacement(nullptr));
  EXPECT_TRUE(!MallocHook_AddMremapHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveMremapHook(nullptr));
  EXPECT_TRUE(!MallocHook_AddPreSbrkHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemovePreSbrkHook(nullptr));
  EXPECT_TRUE(!MallocHook_AddSbrkHook(nullptr));
  EXPECT_TRUE(!MallocHook_RemoveSbrkHook(nullptr));
  EXPECT_TRUE(MallocHook_SetPreMmapHook(nullptr) == nullptr);
  EXPECT_TRUE(MallocHook_SetMmapHook(nullptr) == nullptr);
  EXPECT_TRUE(MallocHook_SetMunmapHook(nullptr) == nullptr);
  EXPECT_TRUE(MallocHook_SetMremapHook(nullptr) == nullptr);
  EXPECT_TRUE(MallocHook_SetPreSbrkHook(nullptr) == nullptr);
  EXPECT_TRUE(MallocHook_SetSbrkHook(nullptr) == nullptr);
  EXPECT_TRUE(MallocHook::UnhookedMMap(nullptr, 4096, 0, 0, -1, 0) == MAP_FAILED);
  EXPECT_EQ(MallocHook::UnhookedMUnmap(nullptr, 4096), -1);
}

}  // namespace

int main()
{
  unsetenv("TCMALLOC_SAMPLE_PARAMETER");

  TestSamplerDisabledAndFastPathRollback();
  TestSamplerEnabledSamplingPath();
  TestSamplerRandomSequenceIsDeterministic();
  TestStackTraceDumpEmptyList();
  TestStackTraceDumpMultipleEntries();
  TestLoggingRawFileAndVerboseBranches();
  TestSpinLockFastSlowAndInternalDelayBranches();
  TestSpinLockContentionCoversSlowLockAndWake();
  TestSystemAllocFailureAndReleaseBranches();
  TestMallocHookListAndNoopBranches();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools whitebox checks failed\n", failures);
    return 1;
  }
  return 0;
}
