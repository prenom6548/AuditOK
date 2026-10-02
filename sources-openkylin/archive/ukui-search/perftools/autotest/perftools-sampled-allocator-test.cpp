#include <cstdint>
#include <cstring>
#include <cstdio>
#include <new>
#include <string>

#include "gperftools/malloc_extension_c.h"
#include "gperftools/malloc_extension.h"
#include "gperftools/tcmalloc.h"
#define private public
#include "common.h"
#include "static_vars.h"
#include "thread_cache.h"
#undef private
#include "thread_cache_ptr.h"
#include "testing_portal.h"

extern "C" size_t tc_nallocx(size_t size, int flags);

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
      std::fprintf(stderr, "EXPECT_EQ failed at %s:%d: %lld != %lld\n", __FILE__, __LINE__,            \
                   static_cast<long long>(actual_value), static_cast<long long>(expected_value));      \
      ++failures;                                                                                      \
    }                                                                                                  \
  } while (0)

bool IsSampledAllocation(void* ptr)
{
  if (ptr == nullptr) {
    return false;
  }
  const PageID page_id = reinterpret_cast<std::uintptr_t>(ptr) >> kPageShift;
  const tcmalloc::Span* span = tcmalloc::Static::pageheap()->GetDescriptor(page_id);
  return span != nullptr && span->sample;
}

void ForceNextAllocationToSample()
{
  MallocExtension_MarkThreadBusy();
  tcmalloc::ThreadCache* cache = tcmalloc::ThreadCachePtr::GetIfPresent();
  EXPECT_TRUE(cache != nullptr);
  if (cache != nullptr) {
    cache->sampler_.initialized_ = true;
    cache->sampler_.bytes_until_sample_ = 0;
  }
}

void TestSmallSampledAllocationBranches()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 1));

  ForceNextAllocationToSample();
  const size_t request = 257;
  void* sampled = tc_malloc(request);

  EXPECT_TRUE(IsSampledAllocation(sampled));
  if (sampled != nullptr) {
    const size_t usable = tc_nallocx(request, 0);
    EXPECT_EQ(tc_malloc_size(sampled), usable);
    EXPECT_EQ(MallocExtension_GetAllocatedSize(sampled), usable);

    int sample_period = -1;
    void** traces = extension->ReadStackTraces(&sample_period);
    EXPECT_TRUE(sample_period > 0);
    EXPECT_TRUE(traces != nullptr);
    if (traces != nullptr) {
      EXPECT_TRUE(traces[0] != nullptr);
      delete[] traces;
    }

    void* grown = tc_realloc(sampled, usable + 16384);
    EXPECT_TRUE(grown != nullptr);
    if (grown != nullptr) {
      tc_free(grown);
    }

    traces = extension->ReadStackTraces(&sample_period);
    EXPECT_TRUE(traces != nullptr);
    if (traces != nullptr) {
      EXPECT_TRUE(traces[0] == nullptr);
      delete[] traces;
    }
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestSampledReallocInPlaceAndZeroBranches()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 1));

  ForceNextAllocationToSample();
  const size_t request = 193;
  void* sampled = tc_malloc(request);
  EXPECT_TRUE(IsSampledAllocation(sampled));
  if (sampled != nullptr) {
    const size_t usable = tc_nallocx(request, 0);
    void* same = tc_realloc(sampled, request);
    EXPECT_TRUE(same == sampled);
    EXPECT_EQ(tc_malloc_size(same), usable);

    int sample_period = -1;
    void** traces = extension->ReadStackTraces(&sample_period);
    EXPECT_TRUE(traces != nullptr);
    if (traces != nullptr) {
      EXPECT_TRUE(traces[0] != nullptr);
      delete[] traces;
    }

    EXPECT_TRUE(tc_realloc(same, 0) == nullptr);

    traces = extension->ReadStackTraces(&sample_period);
    EXPECT_TRUE(traces != nullptr);
    if (traces != nullptr) {
      delete[] traces;
    }
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestLargeSampledAllocationBranches()
{
  MallocExtension* extension = MallocExtension::instance();
  EXPECT_TRUE(extension != nullptr);

  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 1));

  ForceNextAllocationToSample();
  const size_t request = kMaxSize + 12345;
  void* sampled = tc_malloc(request);

  EXPECT_TRUE(IsSampledAllocation(sampled));
  if (sampled != nullptr) {
    EXPECT_EQ(tc_malloc_size(sampled), tc_nallocx(request, 0));

    MallocExtensionWriter sample;
    extension->GetHeapSample(&sample);
    EXPECT_TRUE(sample.find("heap profile:") != std::string::npos);
    EXPECT_TRUE(sample.find("%warn") == std::string::npos);

    tc_free_sized(sampled, request);

    int sample_period = -1;
    void** traces = extension->ReadStackTraces(&sample_period);
    EXPECT_TRUE(traces != nullptr);
    if (traces != nullptr) {
      delete[] traces;
    }
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestSampledCppAndCallocBranches()
{
  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 1));

  ForceNextAllocationToSample();
  void* sampled_new = tc_new(128);
  EXPECT_TRUE(IsSampledAllocation(sampled_new));
  if (sampled_new != nullptr) {
    tc_delete(sampled_new);
  }

  ForceNextAllocationToSample();
  void* sampled_new_nothrow = tc_new_nothrow(144, std::nothrow);
  EXPECT_TRUE(IsSampledAllocation(sampled_new_nothrow));
  if (sampled_new_nothrow != nullptr) {
    tc_delete_nothrow(sampled_new_nothrow, std::nothrow);
  }

  ForceNextAllocationToSample();
  auto* sampled_calloc = static_cast<unsigned char*>(tc_calloc(4, 64));
  EXPECT_TRUE(IsSampledAllocation(sampled_calloc));
  if (sampled_calloc != nullptr) {
    for (size_t i = 0; i < 4U * 64U; ++i) {
      EXPECT_EQ(sampled_calloc[i], 0);
    }
    tc_cfree(sampled_calloc);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestNonSampledSizedFreeInSampledBuild()
{
  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", static_cast<size_t>(1U << 20)));

  void* regular = tc_malloc(96);
  EXPECT_TRUE(regular != nullptr);
  EXPECT_TRUE(!IsSampledAllocation(regular));
  if (regular != nullptr) {
    std::memset(regular, 0x5a, 96);
    tc_free_sized(regular, 96);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestInvalidateSizeClassCacheFreeBranches()
{
  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", static_cast<size_t>(1U << 20)));

  void* regular = tc_malloc(96);
  EXPECT_TRUE(regular != nullptr);
  EXPECT_TRUE(!IsSampledAllocation(regular));
  if (regular != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(regular) >> kPageShift;
    uint32_t cached_size_class = 0;
    EXPECT_TRUE(tcmalloc::Static::pageheap()->TryGetSizeClass(page_id, &cached_size_class));
    EXPECT_TRUE(cached_size_class > 0);

    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_TRUE(!tcmalloc::Static::pageheap()->TryGetSizeClass(page_id, &cached_size_class));

    MallocExtension_MarkThreadIdle();
    EXPECT_EQ(MallocExtension_GetThreadCacheSize(), static_cast<size_t>(0));
    tc_free(regular);

    cached_size_class = 0;
    EXPECT_TRUE(tcmalloc::Static::pageheap()->TryGetSizeClass(page_id, &cached_size_class));
    EXPECT_TRUE(cached_size_class > 0);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadBusy();
}

void TestInvalidateSizeClassCacheSizeQueryBranches()
{
  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", static_cast<size_t>(1U << 20)));

  void* regular = tc_malloc(96);
  EXPECT_TRUE(regular != nullptr);
  EXPECT_TRUE(!IsSampledAllocation(regular));
  if (regular != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(regular) >> kPageShift;
    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_EQ(tc_malloc_size(regular), tc_nallocx(96, 0));
    tc_free(regular);
  }

  ForceNextAllocationToSample();
  const size_t sampled_request = 321;
  void* sampled = tc_malloc(sampled_request);
  EXPECT_TRUE(IsSampledAllocation(sampled));
  if (sampled != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(sampled) >> kPageShift;
    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_EQ(tc_malloc_size(sampled), tc_nallocx(sampled_request, 0));
    tc_free(sampled);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 0));
  const size_t large_request = kMaxSize + 12345;
  void* large = tc_malloc(large_request);
  EXPECT_TRUE(large != nullptr);
  EXPECT_TRUE(!IsSampledAllocation(large));
  if (large != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(large) >> kPageShift;
    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_TRUE(tc_malloc_size(large) >= large_request);
    tc_free(large);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

void TestInvalidateSizeClassCacheOwnershipBranches()
{
  size_t old_sample = 0;
  EXPECT_TRUE(MallocExtension_GetNumericProperty("tcmalloc.sample_parameter", &old_sample));
  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", static_cast<size_t>(1U << 20)));

  void* regular = tc_malloc(128);
  EXPECT_TRUE(regular != nullptr);
  if (regular != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(regular) >> kPageShift;
    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_EQ(MallocExtension_GetOwnership(regular), MallocExtension_kOwned);
    tc_free(regular);
  }

  ForceNextAllocationToSample();
  void* sampled = tc_malloc(257);
  EXPECT_TRUE(IsSampledAllocation(sampled));
  if (sampled != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(sampled) >> kPageShift;
    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_EQ(MallocExtension_GetOwnership(sampled), MallocExtension_kOwned);
    tc_free(sampled);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", 0));
  void* large = tc_malloc(kMaxSize + 8192);
  EXPECT_TRUE(large != nullptr);
  if (large != nullptr) {
    const PageID page_id = reinterpret_cast<std::uintptr_t>(large) >> kPageShift;
    tcmalloc::Static::pageheap()->InvalidateCachedSizeClass(page_id);
    EXPECT_EQ(MallocExtension_GetOwnership(large), MallocExtension_kOwned);
    tc_free(large);
  }

  EXPECT_TRUE(MallocExtension_SetNumericProperty("tcmalloc.sample_parameter", old_sample));
  MallocExtension_MarkThreadIdle();
}

}  // namespace

int main()
{
  TestSmallSampledAllocationBranches();
  TestSampledReallocInPlaceAndZeroBranches();
  TestLargeSampledAllocationBranches();
  TestSampledCppAndCallocBranches();
  TestNonSampledSizedFreeInSampledBuild();
  TestInvalidateSizeClassCacheFreeBranches();
  TestInvalidateSizeClassCacheSizeQueryBranches();
  TestInvalidateSizeClassCacheOwnershipBranches();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools sampled allocator checks failed\n", failures);
    return 1;
  }
  return 0;
}
