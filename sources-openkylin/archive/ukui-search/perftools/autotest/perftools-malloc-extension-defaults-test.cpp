#include <cstddef>
#include <cstdio>
#include <cstring>
#include <vector>

#include "gperftools/malloc_extension.h"
#include "static_vars.h"
#include "thread_cache.h"

namespace tcmalloc {

bool Static::inited_ = true;

void ThreadCache::InitModule() {}

}  // namespace tcmalloc

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

class RecordingMallocExtension : public MallocExtension
{
public:
  void ReleaseToSystem(std::size_t num_bytes) override
  {
    release_called = true;
    released_bytes = num_bytes;
  }

  bool release_called = false;
  std::size_t released_bytes = 0;
};

bool RangeCallbackCalled = false;

void RangeCallback(void*, const base::MallocRange*)
{
  RangeCallbackCalled = true;
}

void TestDefaultVerificationAndStats()
{
  MallocExtension extension;
  int stack_value = 0;

  EXPECT_TRUE(extension.VerifyAllMemory());
  EXPECT_TRUE(extension.VerifyNewMemory(&stack_value));
  EXPECT_TRUE(extension.VerifyArrayNewMemory(&stack_value));
  EXPECT_TRUE(extension.VerifyMallocMemory(&stack_value));

  char stats[] = "unchanged";
  extension.GetStats(stats, sizeof(stats));
  EXPECT_EQ(stats[0], '\0');

  int blocks = -1;
  std::size_t total = 99;
  int histogram[kMallocHistogramSize];
  for (int& bucket : histogram) {
    bucket = 7;
  }
  EXPECT_TRUE(extension.MallocMemoryStats(&blocks, &total, histogram));
  EXPECT_EQ(blocks, 0);
  EXPECT_EQ(total, static_cast<std::size_t>(0));
  for (int bucket : histogram) {
    EXPECT_EQ(bucket, 0);
  }
}

void TestDefaultPropertiesAndOwnership()
{
  MallocExtension extension;
  int stack_value = 0;
  std::size_t value = 123;

  EXPECT_TRUE(!extension.GetNumericProperty("generic.current_allocated_bytes", &value));
  EXPECT_EQ(value, static_cast<std::size_t>(123));
  EXPECT_TRUE(!extension.SetNumericProperty("tcmalloc.max_total_thread_cache_bytes", 4096));

  EXPECT_EQ(extension.GetSystemAllocator(), static_cast<SysAllocator*>(nullptr));
  extension.SetSystemAllocator(reinterpret_cast<SysAllocator*>(0x1234));
  extension.MarkThreadIdle();
  extension.MarkThreadBusy();
  extension.MarkThreadTemporarilyIdle();
  extension.SetMemoryReleaseRate(4.0);
  EXPECT_TRUE(extension.GetMemoryReleaseRate() < 0.0);
  EXPECT_EQ(extension.GetEstimatedAllocatedSize(345), static_cast<std::size_t>(345));
  EXPECT_EQ(extension.GetOwnership(&stack_value), MallocExtension::kUnknownOwnership);
  EXPECT_EQ(extension.GetAllocatedSize(&stack_value), static_cast<std::size_t>(0));
  EXPECT_EQ(extension.GetThreadCacheSize(), static_cast<std::size_t>(0));

  std::vector<MallocExtension::FreeListInfo> free_lists;
  free_lists.push_back({1, 2, 3, "old"});
  extension.GetFreeListSizes(&free_lists);
  EXPECT_TRUE(free_lists.empty());
}

void TestDefaultReleaseFreeMemoryDelegatesToReleaseToSystem()
{
  RecordingMallocExtension extension;
  extension.ReleaseFreeMemory();

  EXPECT_TRUE(extension.release_called);
  EXPECT_EQ(extension.released_bytes, static_cast<std::size_t>(-1));
}

void TestDefaultSamplingErrorsAndRanges()
{
  MallocExtension extension;
  int sample_period = 77;

  EXPECT_EQ(extension.ReadStackTraces(&sample_period), static_cast<void**>(nullptr));
  EXPECT_EQ(sample_period, 77);
  EXPECT_EQ(extension.ReadHeapGrowthStackTraces(), static_cast<void**>(nullptr));

  MallocExtensionWriter sample;
  extension.GetHeapSample(&sample);
  EXPECT_TRUE(sample.find("does not support sampling") != std::string::npos);

  MallocExtensionWriter growth;
  extension.GetHeapGrowthStacks(&growth);
  EXPECT_TRUE(growth.find("does not support ReadHeapGrowthStackTraces") != std::string::npos);

  RangeCallbackCalled = false;
  extension.Ranges(nullptr, &RangeCallback);
  EXPECT_TRUE(!RangeCallbackCalled);
}

}  // namespace

int main()
{
  MallocExtension::Initialize();

  TestDefaultVerificationAndStats();
  TestDefaultPropertiesAndOwnership();
  TestDefaultReleaseFreeMemoryDelegatesToReleaseToSystem();
  TestDefaultSamplingErrorsAndRanges();

  if (failures != 0) {
    std::fprintf(stderr, "%d malloc extension default checks failed\n", failures);
    return 1;
  }
  return 0;
}
