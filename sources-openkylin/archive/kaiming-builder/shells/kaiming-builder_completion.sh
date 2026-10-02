#!/bin/bash

_kaiming-builder() {
  local pre cur opts
  COMPREPLY=()
  pre="${COMP_WORDS[COMP_CWORD-1]}"
  cur="${COMP_WORDS[COMP_CWORD]}"

  case "${pre}" in
    "kaiming-builder")
    opts=$(echo "create build enter create-from-debsource kch hostmode-build --help -h --bundle-filesystem=squashfs"  | tr '' '\n' | sort | tr '\n' ' ')
      ;;
    "build")
      opts="--help -h --verbose -v --arch --build-only --finish-only --export-only --export-all --force-clean -f --keep-build-dirs -k --stop-at --jobs --bundle-filesystem=squashfs --comp -o --output"
      ;;
    "enter")
      opts="--help -h"
      ;;
    "-v" | "-f" | "-k")
      opts="--export-only --export-all"
      ;;
    "--export-all")
      opts="--bundle-filesystem=squashfs"
      ;;
    "kch")
      opts="--help -h --create -c --increment -i --append -a --last-version --changelog-file"
      ;;
    "hostmode-build")
      opts="--help -h --verbose -v --force-clean -f --jobs --bundle-filesystem=squashfs --comp -o --output"
      ;;
    *)
      opts=""
      ;;
  esac

  # 只要确定是kaiming-builder build就补全为kaiming-builder build -v -f -k
  if [[ "${pre}" == "kaiming-builder" && "${cur}" =~ ^b(uild)? ]]; then
    COMPREPLY=("build -f -k")
  else
  # kaiming-builder build 显示后面可以跟的参数选项
      COMPREPLY=( $(compgen -f -W "${opts}" -- ${cur}) )
  fi

  return 0

}

complete -F _kaiming-builder kaiming-builder


