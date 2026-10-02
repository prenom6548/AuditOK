#!/bin/sh
# Use a pinned, modern emulator for CMake without changing host binfmt.
# tonistiigi's binfmt build expects an explicit guest argv[0].
# SIGILL has occurred intermittently during configuration under both emulator
# versions. Retry only this signal, at most three times; all ordinary CMake
# failures remain fatal and the image still requires runtime validation.
for attempt in 1 2 3; do
  /usr/local/libexec/qemu-riscv64-cmake /usr/bin/cmake.real /usr/bin/cmake "$@"
  result=$?
  if [ "$result" -ne 132 ]; then
    exit "$result"
  fi
  printf 'CMake SIGILL under RISC-V emulation (attempt %s/3)\n' "$attempt" >&2
done
exit 132
