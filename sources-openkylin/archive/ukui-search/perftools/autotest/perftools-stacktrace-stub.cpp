#include <algorithm>
#include <execinfo.h>

namespace tcmalloc {

int GrabBacktrace(void** result, int max_depth, int skip_count)
{
  const int depth = backtrace(result, max_depth);
  const int used = std::max(0, depth - skip_count - 1);
  for (int i = 0; i < used; ++i) {
    result[i] = result[i + skip_count + 1];
  }
  return used;
}

}  // namespace tcmalloc

int GetStackFrames(void** result, int* sizes, int max_depth, int skip_count)
{
  const int used = tcmalloc::GrabBacktrace(result, max_depth, skip_count);
  for (int i = 0; i < used; ++i) {
    sizes[i] = 0;
  }
  return used;
}

int GetStackFramesWithContext(void** result, int* sizes, int max_depth, int skip_count, const void*)
{
  return GetStackFrames(result, sizes, max_depth, skip_count);
}

int GetStackTrace(void** result, int max_depth, int skip_count)
{
  return tcmalloc::GrabBacktrace(result, max_depth, skip_count);
}

int GetStackTraceWithContext(void** result, int max_depth, int skip_count, const void*)
{
  return GetStackTrace(result, max_depth, skip_count);
}
