#include <cstdint>
#include <cstdio>
#include <limits>

#include "base/commandlineflags.h"
#include "gperftools/malloc_extension.h"
#include "gperftools/tcmalloc.h"
#define private public
#include "page_heap.h"
#include "static_vars.h"
#undef private

DECLARE_bool(malloc_disable_memory_release);
DECLARE_double(tcmalloc_release_rate);

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

void EnsureAllocatorInitialized()
{
  void* ptr = tc_malloc(16);
  EXPECT_TRUE(ptr != nullptr);
  tc_free(ptr);
}

void TestRoundUpAndAlignedOverflowBranches()
{
  tcmalloc::PageHeap heap(2);
  EXPECT_EQ(heap.RoundUpSize(1), static_cast<Length>(2));
  EXPECT_EQ(heap.RoundUpSize(2), static_cast<Length>(2));
  EXPECT_EQ(heap.RoundUpSize(3), static_cast<Length>(4));

  const Length huge = (std::numeric_limits<Length>::max)();
  const Length rounded = heap.RoundUpSize(huge);
  EXPECT_TRUE(rounded >= huge || rounded == ((std::numeric_limits<Length>::max)() & ~static_cast<Length>(1)));

  tcmalloc::Span* overflow = heap.NewAligned(huge, huge);
  EXPECT_TRUE(overflow == nullptr);

  tcmalloc::PageHeap failing_heap(1);
  EXPECT_TRUE(failing_heap.New(kMaxValidPages + 1) == nullptr);
}

void TestAllocateDeleteAndRangeQueries()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.New(4);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  EXPECT_EQ(span->length, static_cast<Length>(4));
  EXPECT_TRUE(heap.GetDescriptor(span->start) == span);
  EXPECT_TRUE(heap.GetDescriptor(span->start + 1) != span);

  tcmalloc::PageHeap aligned_heap(1);
  tcmalloc::Span* aligned = aligned_heap.NewAligned(3, 4);
  EXPECT_TRUE(aligned != nullptr);
  if (aligned != nullptr) {
    EXPECT_EQ(aligned->start % 4, static_cast<PageID>(0));
  }

  heap.RegisterSizeClass(span, 1);
  span->refcount = 1;
  EXPECT_TRUE(heap.GetDescriptor(span->start + 1) == span);
  EXPECT_EQ(heap.GetSizeClassOrZero(span->start), uint32_t{0});
  heap.SetCachedSizeClass(span->start, 1);
  EXPECT_EQ(heap.GetSizeClassOrZero(span->start), uint32_t{1});
  uint32_t cached = 0;
  EXPECT_TRUE(heap.TryGetSizeClass(span->start, &cached));
  EXPECT_EQ(cached, uint32_t{1});
  heap.InvalidateCachedSizeClass(span->start);
  EXPECT_EQ(heap.GetSizeClassOrZero(span->start), uint32_t{0});

  base::MallocRange range{};
  {
    SpinLockHolder lock(heap.pageheap_lock());
    EXPECT_TRUE(heap.GetNextRange(span->start, &range));
    EXPECT_TRUE(range.length > 0);
    EXPECT_TRUE(range.fraction > 0.0);
  }

  tcmalloc::PageHeap::Stats before = heap.StatsLocked();
  heap.Delete(span);
  tcmalloc::PageHeap::Stats after = heap.StatsLocked();
  EXPECT_TRUE(after.free_bytes >= before.free_bytes);

  {
    SpinLockHolder lock(heap.pageheap_lock());
    EXPECT_TRUE(heap.GetNextRange(span->start, &range));
    EXPECT_TRUE(range.type == base::MallocRange::FREE || range.type == base::MallocRange::UNMAPPED);
  }

  if (aligned != nullptr) {
    aligned_heap.Delete(aligned);
  }

  {
    SpinLockHolder lock(heap.pageheap_lock());
    EXPECT_TRUE(heap.Check());
  }
}

void TestSplitScavengeAndReleaseBranches()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.New(8);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  tcmalloc::Span* tail = heap.SplitForTest(span, 3);
  EXPECT_TRUE(tail != nullptr);
  if (tail != nullptr) {
    EXPECT_EQ(span->length, static_cast<Length>(3));
    EXPECT_EQ(tail->length, static_cast<Length>(5));
    heap.Delete(tail);
  }

  heap.PrepareAndDelete(span, [&]() {
    EXPECT_TRUE(heap.GetDescriptor(span->start) == span);
  });

  tcmalloc::PageHeap::SmallSpanStats small{};
  tcmalloc::PageHeap::LargeSpanStats large{};
  {
    SpinLockHolder lock(heap.pageheap_lock());
    heap.GetSmallSpanStatsLocked(&small);
    heap.GetLargeSpanStatsLocked(&large);
    EXPECT_TRUE(large.spans >= 0);
  }

  {
    SpinLockHolder lock(heap.pageheap_lock());
    const Length released_none = heap.ReleaseAtLeastNPages(0);
    EXPECT_TRUE(released_none >= 0);
  }

  heap.SetAggressiveDecommit(true);
  tcmalloc::Span* big = heap.New(kMaxPages + 2);
  EXPECT_TRUE(big != nullptr);
  PageID big_start = 0;
  if (big != nullptr) {
    big_start = big->start;
    heap.Delete(big);
  }
  {
    SpinLockHolder lock(heap.pageheap_lock());
    const Length released_some = heap.ReleaseAtLeastNPages(1);
    EXPECT_TRUE(released_some >= 0);
    if (big_start != 0) {
      base::MallocRange range{};
      EXPECT_TRUE(heap.GetNextRange(big_start, &range));
      EXPECT_TRUE(range.type == (released_some > 0 ? base::MallocRange::UNMAPPED : base::MallocRange::FREE));
    }
  }
  heap.SetAggressiveDecommit(false);

  {
    SpinLockHolder lock(heap.pageheap_lock());
    EXPECT_TRUE(heap.Check());
    EXPECT_TRUE(heap.CheckExpensive());
  }
}

void TestUnknownRangeTypeBranch()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.New(1);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  const unsigned int original_location = span->location;
  {
    SpinLockHolder lock(heap.pageheap_lock());
    span->location = 3;
    base::MallocRange range{};
    EXPECT_TRUE(heap.GetNextRange(span->start, &range));
    EXPECT_TRUE(range.type == base::MallocRange::UNKNOWN);
    span->location = original_location;
  }

  heap.Delete(span);
}

void TestReturnedFreeListReuseBranches()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.New(2);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  heap.Delete(span);
  {
    SpinLockHolder lock(heap.pageheap_lock());
    const Length released = heap.ReleaseAtLeastNPages(1);
    EXPECT_TRUE(released >= 1);
  }

  tcmalloc::Span* reused = heap.New(1);
  EXPECT_TRUE(reused != nullptr);
  if (reused != nullptr) {
    EXPECT_EQ(reused->location, tcmalloc::Span::IN_USE);
    heap.Delete(reused);
  }
}

void TestLargeReturnedFreeListReuseBranches()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.New(kMaxPages + 2);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  heap.Delete(span);
  {
    SpinLockHolder lock(heap.pageheap_lock());
    const Length released = heap.ReleaseAtLeastNPages(1);
    EXPECT_TRUE(released >= 1);
  }

  tcmalloc::Span* reused = heap.New(kMaxPages + 1);
  EXPECT_TRUE(reused != nullptr);
  if (reused != nullptr) {
    EXPECT_EQ(reused->location, tcmalloc::Span::IN_USE);
    heap.Delete(reused);
  }
}

void TestPreMergeAndScavengeDelayBranches()
{
  tcmalloc::PageHeap heap(1);

  tcmalloc::Span returned{};
  returned.start = 100;
  returned.length = 1;
  returned.location = tcmalloc::Span::ON_RETURNED_FREELIST;

  tcmalloc::Span normal{};
  normal.start = 101;
  normal.length = 1;
  normal.location = tcmalloc::Span::ON_NORMAL_FREELIST;

  {
    SpinLockHolder lock(heap.pageheap_lock());
    EXPECT_TRUE(heap.CheckAndHandlePreMerge(&returned, &normal) == nullptr);
  }

  const double old_rate = FLAGS_tcmalloc_release_rate;
  FLAGS_tcmalloc_release_rate = 0.0;
  {
    SpinLockHolder lock(heap.pageheap_lock());
    heap.scavenge_counter_ = 0;
    heap.IncrementalScavenge(1);
    EXPECT_EQ(heap.scavenge_counter_, tcmalloc::PageHeap::kDefaultReleaseDelay);
  }

  tcmalloc::Span* releasable = heap.New(1);
  EXPECT_TRUE(releasable != nullptr);
  if (releasable != nullptr) {
    heap.Delete(releasable);
    {
      SpinLockHolder lock(heap.pageheap_lock());
      heap.scavenge_counter_ = 0;
      FLAGS_tcmalloc_release_rate = 0.0001;
      heap.IncrementalScavenge(1);
      EXPECT_EQ(heap.scavenge_counter_, tcmalloc::PageHeap::kMaxReleaseDelay);
    }
  }
  FLAGS_tcmalloc_release_rate = old_rate;
}

void TestDirectLargeFreeListMutationBranches()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.New(kMaxPages + 2);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  heap.Delete(span);
  {
    SpinLockHolder lock(heap.pageheap_lock());
    const uint64_t free_before = heap.stats_.free_bytes;
    heap.RemoveFromFreeList(span);
    EXPECT_TRUE(heap.stats_.free_bytes < free_before);
    heap.PrependToFreeList(span);
    EXPECT_EQ(heap.stats_.free_bytes, free_before);
  }

  heap.SetAggressiveDecommit(true);
  {
    SpinLockHolder lock(heap.pageheap_lock());
    const Length released = heap.ReleaseAtLeastNPages(1);
    EXPECT_TRUE(released >= 1);
    EXPECT_TRUE(span->location == tcmalloc::Span::ON_RETURNED_FREELIST);

    const uint64_t unmapped_before = heap.stats_.unmapped_bytes;
    heap.RemoveFromFreeList(span);
    EXPECT_TRUE(heap.stats_.unmapped_bytes < unmapped_before);
    heap.PrependToFreeList(span);
    EXPECT_EQ(heap.stats_.unmapped_bytes, unmapped_before);
  }
  heap.SetAggressiveDecommit(false);
}

void ConvertNormalFreeSpanToReturned(tcmalloc::PageHeap* heap, tcmalloc::Span* span)
{
  SpinLockHolder lock(heap->pageheap_lock());
  heap->RemoveFromFreeList(span);
  const bool decommitted = heap->DecommitSpan(span);
  EXPECT_TRUE(decommitted);
  if (!decommitted) {
    span->location = tcmalloc::Span::ON_NORMAL_FREELIST;
    heap->PrependToFreeList(span);
    return;
  }
  span->location = tcmalloc::Span::ON_RETURNED_FREELIST;
  heap->PrependToFreeList(span);
}

void TestLargeReturnedAllocLargeBranches()
{
  const bool old_disable_release = FLAGS_malloc_disable_memory_release;
  FLAGS_malloc_disable_memory_release = false;

  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* normal = heap.New(kMaxPages + 6);
  tcmalloc::Span* separator = heap.New(1);
  tcmalloc::Span* returned = heap.New(kMaxPages + 3);
  EXPECT_TRUE(normal != nullptr);
  EXPECT_TRUE(separator != nullptr);
  EXPECT_TRUE(returned != nullptr);
  if (normal == nullptr || separator == nullptr || returned == nullptr) {
    FLAGS_malloc_disable_memory_release = old_disable_release;
    return;
  }

  heap.Delete(normal);
  heap.Delete(returned);
  ConvertNormalFreeSpanToReturned(&heap, returned);

  tcmalloc::Span* carved = nullptr;
  {
    SpinLockHolder lock(heap.pageheap_lock());
    carved = heap.AllocLarge(kMaxPages + 2);
  }

  EXPECT_TRUE(carved == returned);
  if (carved != nullptr) {
    EXPECT_EQ(carved->location, tcmalloc::Span::IN_USE);
    EXPECT_EQ(carved->length, static_cast<Length>(kMaxPages + 2));
    heap.Delete(carved);
  }
  heap.Delete(separator);
  FLAGS_malloc_disable_memory_release = old_disable_release;
}

void TestReleaseDisabledBranches()
{
  const bool old_disable_release = FLAGS_malloc_disable_memory_release;
  FLAGS_malloc_disable_memory_release = true;

  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* first = heap.New(1);
  tcmalloc::Span* second = heap.New(1);
  EXPECT_TRUE(first != nullptr);
  EXPECT_TRUE(second != nullptr);
  if (first == nullptr || second == nullptr) {
    FLAGS_malloc_disable_memory_release = old_disable_release;
    return;
  }

  heap.Delete(second);
  {
    SpinLockHolder lock(heap.pageheap_lock());
    heap.scavenge_counter_ = 0;
    EXPECT_EQ(heap.ReleaseAtLeastNPages(1), static_cast<Length>(0));
    EXPECT_EQ(second->location, tcmalloc::Span::ON_NORMAL_FREELIST);
    EXPECT_EQ(heap.stats_.unmapped_bytes, uint64_t{0});

    heap.SetAggressiveDecommit(true);
    first->location = tcmalloc::Span::ON_RETURNED_FREELIST;
    EXPECT_TRUE(heap.CheckAndHandlePreMerge(first, second) == nullptr);
    first->location = tcmalloc::Span::IN_USE;
    heap.SetAggressiveDecommit(false);

    heap.scavenge_counter_ = 0;
    heap.IncrementalScavenge(1);
    EXPECT_EQ(heap.scavenge_counter_, tcmalloc::PageHeap::kDefaultReleaseDelay);
  }

  heap.Delete(first);
  FLAGS_malloc_disable_memory_release = old_disable_release;
}

void TestNewWithSizeClassBranches()
{
  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* span = heap.NewWithSizeClass(3, 1);
  EXPECT_TRUE(span != nullptr);
  if (span == nullptr) {
    return;
  }

  EXPECT_EQ(span->sizeclass, uint32_t{1});
  EXPECT_TRUE(heap.GetDescriptor(span->start) == span);
  EXPECT_TRUE(heap.GetDescriptor(span->start + 1) == span);
  EXPECT_TRUE(heap.GetDescriptor(span->start + 2) == span);
  EXPECT_EQ(heap.GetSizeClassOrZero(span->start), uint32_t{0});

  heap.Delete(span);
}

void TestForcedCoalesceRetryBranches()
{
  const bool old_disable_release = FLAGS_malloc_disable_memory_release;
  FLAGS_malloc_disable_memory_release = false;

  tcmalloc::PageHeap heap(1);
  tcmalloc::Span* first = heap.New(4);
  tcmalloc::Span* second = heap.New(4);
  EXPECT_TRUE(first != nullptr);
  EXPECT_TRUE(second != nullptr);
  if (first == nullptr || second == nullptr) {
    FLAGS_malloc_disable_memory_release = old_disable_release;
    return;
  }

  EXPECT_EQ(second->start, first->start + first->length);
  if (second->start != first->start + first->length) {
    heap.Delete(first);
    heap.Delete(second);
    FLAGS_malloc_disable_memory_release = old_disable_release;
    return;
  }

  heap.Delete(second);
  ConvertNormalFreeSpanToReturned(&heap, second);
  heap.Delete(first);

  {
    SpinLockHolder lock(heap.pageheap_lock());
    const uint64_t fake_system_bytes =
        (128ULL * 1024ULL * 1024ULL) - (static_cast<uint64_t>(6) << kPageShift) + kPageSize;
    EXPECT_TRUE(fake_system_bytes > heap.stats_.system_bytes);
    if (fake_system_bytes > heap.stats_.system_bytes) {
      const uint64_t delta = fake_system_bytes - heap.stats_.system_bytes;
      heap.stats_.system_bytes += delta;
      heap.stats_.committed_bytes += delta;
    }
  }

  tcmalloc::Span* merged = heap.New(6);
  EXPECT_TRUE(merged != nullptr);
  if (merged != nullptr) {
    EXPECT_TRUE(merged->length >= static_cast<Length>(6));
    heap.Delete(merged);
  }

  FLAGS_malloc_disable_memory_release = old_disable_release;
}

}  // namespace

int main()
{
  EnsureAllocatorInitialized();
  TestRoundUpAndAlignedOverflowBranches();
  TestAllocateDeleteAndRangeQueries();
  TestSplitScavengeAndReleaseBranches();
  TestUnknownRangeTypeBranch();
  TestReturnedFreeListReuseBranches();
  TestLargeReturnedFreeListReuseBranches();
  TestPreMergeAndScavengeDelayBranches();
  TestDirectLargeFreeListMutationBranches();
  TestLargeReturnedAllocLargeBranches();
  TestReleaseDisabledBranches();
  TestNewWithSizeClassBranches();
  TestForcedCoalesceRetryBranches();

  if (failures != 0) {
    std::fprintf(stderr, "%d perftools page heap whitebox checks failed\n", failures);
    return 1;
  }
  return 0;
}
