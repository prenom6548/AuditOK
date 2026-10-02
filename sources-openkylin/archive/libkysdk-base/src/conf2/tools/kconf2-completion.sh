_kconf2() {  
    local local_IFS=$IFS
    IFS=$' \t\n'
    COMPREPLY=()  
    kconf2_opts='help list-apps list-versions list-schemas list-keys list-children list-recursively range describe get set reset writable monitor reload'  

    case ${COMP_WORDS[COMP_CWORD - 1]} in  
        kconf2 | help)  
            COMPREPLY=( $( compgen -W "$kconf2_opts" "${COMP_WORDS[COMP_CWORD]}" ) )  
            ;;  

        list-schemas | list-versions)  
            apps=$(kconf2 list-apps)  
            COMPREPLY=( $( compgen -W "$apps" "${COMP_WORDS[COMP_CWORD]}" ) )  
            ;;  

        list-keys | list-children | list-recursively | range | describe | get | set | reset | writable | monitor)  
            get_opts=()  
            apps=$(kconf2 list-apps)   
            for app in $apps; do  
                versions=$(kconf2 list-versions "$app")  
                for version in $versions; do  
                    schemas=$(kconf2 list-schemas "$app" "$version")  
                    for schema in $schemas; do  
                        id="$schema:$version"  
                        get_opts+=("$id")  
                    done  
                done  
            done  

            COMPREPLY=( $( compgen -W "${get_opts[*]}" "${COMP_WORDS[COMP_CWORD]}" ) )  
            ;;  

        *)  
            list="range describe get set reset writable monitor"  
            if [[ "${COMP_WORDS[1]}" == "list-schemas" ]]; then  
                versions=$(kconf2 list-versions "$prev")  
                if [[ $? -eq 0 ]]; then  
                    COMPREPLY=( $( compgen -W "$versions" "${COMP_WORDS[COMP_CWORD]}" ) )  
                fi  
            fi  
            if [[ $list == *"${COMP_WORDS[1]}"* ]]; then  
                keys=$(kconf2 list-keys "${COMP_WORDS[2]}")  
                if [[ $? -eq 0 ]]; then  
                    COMPREPLY=( $( compgen -W "$keys" "${COMP_WORDS[COMP_CWORD]}" ) )  
                fi  
            fi  
            ;;  
    esac  
    IFS=$local_IFS
}  

# 应用自动补全函数到 'kconf2' 命令  
complete -F _kconf2 kconf2