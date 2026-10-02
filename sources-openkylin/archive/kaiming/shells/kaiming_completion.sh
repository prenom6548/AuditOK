# Copyright (c) KylinSoft Co., Ltd. 2024. All rights reserved.
# kaiming is licensed under the GPL v2.0+.
# See the LICENSE file for more details.
#!/bin/bash
_kaiming() {
    local pre cur opts
    COMPREPLY=()
    pre="${COMP_WORDS[COMP_CWORD-1]}"
    cur="${COMP_WORDS[COMP_CWORD]}"

    case $COMP_CWORD in
        1)
            opts="install repair uninstall update upgrade list info print-meta changelog compare-versions enter run ps kill search which deltas build build-init build-finish build-export remote --help --version -h -v "
            ;;
        2)
            if [[ ${COMP_WORDS[1]} == "run" ]];then
                opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
                    -path '*/app/*' -o \
                    -path '*/base/*' -o \
                    -path '*/runtime/*' -o \
                    -path '*/extension/*' -o \
                    -path '*/depend/*' 2>/dev/null | \
                    xargs -I {} basename {} | sort | uniq) \
                    --help --verbose --devel --file-forwarding --command= -h -v -d"
            fi
            ;;
        3)
            if [[ "${COMP_WORDS[1]}" == "run" && "${COMP_WORDS[2]}" ]]; then
                opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
                    -path '*/app/*' -o \
                    -path '*/base/*' -o \
                    -path '*/runtime/*' -o \
                    -path '*/extension/*' -o \
                    -path '*/depend/*' 2>/dev/null | \
                    xargs -I {} basename {} | sort | uniq)"
            fi
            ;;
        4)
            if [[ "${COMP_WORDS[1]}" == "run" && "${COMP_WORDS[3]}" ]]; then
                opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
                    -path '*/app/*' -o \
                    -path '*/base/*' -o \
                    -path '*/runtime/*' -o \
                    -path '*/extension/*' -o \
                    -path '*/depend/*' 2>/dev/null | \
                    xargs -I {} basename {} | sort | uniq)"
            fi
            ;;
        5)
            if [[ "${COMP_WORDS[1]}" == "run" && "${COMP_WORDS[4]}" ]]; then
                opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
                    -path '*/app/*' -o \
                    -path '*/base/*' -o \
                    -path '*/runtime/*' -o \
                    -path '*/extension/*' -o \
                    -path '*/depend/*' 2>/dev/null | \
                    xargs -I {} basename {} | sort | uniq)"
            fi
    esac

    if [[ ${COMP_WORDS[1]} == "info" ]]; then
        if [[ ${COMP_CWORD} -ge 3 && ${COMP_WORDS[2]} == "-L" || ${COMP_WORDS[2]} == "--listfiles" ]]; then
            opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
                -path '*/app/*' -o \
                -path '*/base/*' -o \
                -path '*/runtime/*' -o \
                -path '*/extension/*' -o \
                -path '*/depend/*' 2>/dev/null | \
                xargs -I {} basename {} | sort | uniq )"
        else
            opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
                -path '*/app/*' -o \
                -path '*/base/*' -o \
                -path '*/runtime/*' -o \
                -path '*/extension/*' -o \
                -path '*/depend/*' 2>/dev/null | \
                xargs -I {} basename {} | sort | uniq ) \
                --help --verbose -h -v -L --listfiles"
        fi
    fi

    if [[ ${COMP_WORDS[1]} == "uninstall" ]];then
        opts="$(find /opt/kaiming/layers -mindepth 4 -maxdepth 4 -type d \
            -path '*/app/*' -o \
            -path '*/base/*' -o \
            -path '*/runtime/*' -o \
            -path '*/extension/*' -o \
            -path '*/depend/*' 2>/dev/null | \
            xargs -I {} basename {} | sort | uniq ) \
            --help --verbose --assumeyes --delete-data -h -y -v"
    fi

    if [[ ${COMP_WORDS[1]} == "list" ]];then
        opts="--help --verbose --kind --module -h -v"
    fi

    if [[ ${COMP_WORDS[1]} == "enter" ]];then
        opts="$(kaiming ps | awk 'NR>1 {print $1}') --help -h"
    fi

    if [[ ${COMP_WORDS[1]} == "ps" ]];then
        opts="--help -h"
    fi

    if [[ ${COMP_WORDS[1]} == "print-meta" || ${COMP_WORDS[1]} == "changelog" ]];then
        opts="$(ls *.ok 2>>/dev/null)"
    fi

    if [[ ${COMP_WORDS[1]} == "kill" ]];then
        opts="$(kaiming ps | awk 'NR>1 {print $1}') --help -h"
    fi

    if [[ ${COMP_WORDS[1]} == "build-init" ]];then
        opts="--help --arch= --runtime= --base --kind= --channel= --depend= --version= -h"
    fi

    if [[ ${COMP_WORDS[1]} == "build" ]];then
        opts="--help --build-mount= --build-dir= --env= --unset-env= --debug -h"
    fi

    if [[ ${COMP_WORDS[1]} == "build-finish" ]];then
        opts="--help -h"
    fi

    if [[ ${COMP_WORDS[1]} == "build-export" ]];then
        opts="--help --dest-dir= --all --bundle-filesystem= --ref-mode --comp= -h"
    fi

    if [[ ${COMP_WORDS[1]} == "install" ]]; then
      local package_opts=""
      if [[ -f /opt/kaiming/packages ]]; then
          package_opts="$(cat /opt/kaiming/packages | sort | uniq | tr '\n' ' ')"
      fi
      local flag_opts="--help --verbose --assumeyes -h -y -v --kind --force -f --skip-signature-verification --bundle --subpath"
      
      # 重置COMPREPLY
      COMPREPLY=()

      local search_path=""
      local prefix=""
      
      if [[ -z "$cur" ]]; then
          search_path="."
          prefix=""
      elif [[ "$cur" == */ ]]; then
          search_path="${cur%/}"
          prefix=""
          [[ ! -d "$search_path" ]] && search_path="."
      else
          local parent_dir="${cur%/*}"
          local base_name="${cur##*/}"

          if [[ "$parent_dir" == "$cur" || -z "$parent_dir" ]]; then
              search_path="."
          else
              search_path="$parent_dir"
              [[ ! -d "$search_path" ]] && search_path="."
          fi

          prefix="$base_name"
      fi

      local compgen_results=()
      if [[ -d "$search_path" ]]; then
          local match_pattern="${search_path}/${prefix}*"
          mapfile -t compgen_results < <(
              find "$search_path" -maxdepth 1 -mindepth 1 \
                  \( -type d -o -type f -name "*.ok" -o -type f -name "*.okx" -o -type f -name "*.okinstaller" \) \
                  -path "$match_pattern" |
              sed "s|^${search_path//\//\\/}/||"
          )
      fi

      local filtered=()
      for item in "${compgen_results[@]}"; do
          local full_path="${search_path}/${item}"

          if [[ -d "$full_path" ]] || [[ "$full_path" =~ \.ok$ ]] || [[ "$full_path" =~ \.okx$ ]] || [[ "$full_path" =~ \.okinstaller$ ]]; then
              if [[ "$search_path" == "/" ]]; then
                  filtered+=("/$item")
              elif [[ "$search_path" == /* ]]; then
                  filtered+=("$full_path")
              else
                  filtered+=("$full_path")
              fi
          fi
      done
      COMPREPLY=("${filtered[@]}")

      if [[ "$cur" != */* && "$cur" != ./* && "$cur" != ../* && "$cur" != ~/* ]]; then
          local all_opts="$package_opts $flag_opts"
          local opt_completions=($(compgen -W "$all_opts" -- "$cur"))
          COMPREPLY=("${COMPREPLY[@]}" "${opt_completions[@]}")
      fi
      
      compopt -o filenames -o bashdefault -o default -o nospace 2>/dev/null
      return 0
    fi

    if [[ ${COMP_WORDS[1]} == "update" ]];then
        if [[ -f /opt/kaiming/packages ]]; then
            opts="$(cat /opt/kaiming/packages | sort | uniq | tr '\n' ' ') --help --verbose --assumeyes -h -y -v --upgradable"
        else
            opts="--help --verbose --assumeyes -h -y -v --upgradable"
        fi
        COMPREPLY=( $(compgen -f -W "${opts}" -- "${cur}") )
        compopt -o filenames
        return 0
    fi

    if [[ ${COMP_WORDS[1]} == "upgrade" ]];then
        if [[ -f /opt/kaiming/packages ]]; then
            opts="$(cat /opt/kaiming/packages | sort | uniq | tr '\n' ' ') --help --assumeyes -h -y"
        else
            opts="--help --assumeyes -h -y"
        fi
    fi

    if [[ ${COMP_WORDS[1]} == "search" ]];then
        if [[ -f /opt/kaiming/packages ]]; then
            opts="$(cat /opt/kaiming/packages | sort | uniq | tr '\n' ' ') --help --verbose --assumeyes --module --kind -h -v"
        else
            opts="--help --verbose --module --kind -h -v"
        fi
    fi

    if [[ ${COMP_WORDS[1]} == "deltas" ]];then
        opts="--help -h --ok"
    fi

    if [[ ${COMP_WORDS[1]} == "which" ]];then
        opts="--help -h -v"
    fi

    if [[ ${COMP_WORDS[1]} == "remote" ]];then
        opts="--help -h --repo"
    fi

    # 其他命令的默认补全逻辑
    if [[ ${COMP_WORDS[1]} == "which" ]]; then
        COMPREPLY=( $(compgen -f -W "${opts}" -- ${cur}) )
        compopt -o nospace
    else
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
    fi
}

complete -F _kaiming kaiming
