input_file="/etc/ostree-release"

etc_path_ln="/etc"
lib_path_ln="/lib"
usr_share_ln="/usr/share"
bin_ln_V11="/opt/system/bin"
bin_ln_not_V11="/usr/bin"

# etc_path="/opt/system/conf"
etc_path="/etc"
lib_path="/opt/system/lib"
usr_share="/opt/system/resource"
bin_path="/opt/kaiming-tools/bin"
kaiminglogdir="/var/log/kaiming"

is_V11=0

if [ ! -d "${kaiminglogdir}" ]; then
	mkdir -p "${kaiminglogdir}"
	chmod o+rwx "${kaiminglogdir}"
fi

#if [ -e "$input_file" ]; then
#    is_V11=1
#fi

if [ $is_V11 -eq 0 ]; then

    # 安装 system dbus 运行所需文件，没有则会开启 dbus 失败
    if [ ! -e "$lib_path_ln/systemd/system/org.kylin.kaiming.service" ]; then
        if [ ! -d "$lib_path_ln/systemd/system" ]; then
            mkdir -p "$lib_path_ln/systemd/system"
        fi
        ln -s "$lib_path/systemd/system/org.kylin.kaiming.service" "$lib_path_ln/systemd/system/org.kylin.kaiming.service"
    fi

	if [ ! -e "$lib_path_ln/systemd/user/km-ses-dbusproxy.service" ]; then
        if [ ! -d "$lib_path_ln/systemd/user" ]; then
            mkdir -p "$lib_path_ln/systemd/user"
        fi
        ln -s "$lib_path/systemd/user/km-ses-dbusproxy.service" "$lib_path_ln/systemd/user/km-ses-dbusproxy.service"
    fi

    # 安装 system dbus 运行所需文件，没有则会开启 dbus 失败
    if [ ! -e "$usr_share_ln/dbus-1/system-services/org.kaiming.proxy.service" ]; then
        if [ ! -d "$usr_share_ln/dbus-1/system-services" ]; then
            mkdir -p "$usr_share_ln/dbus-1/system-services"
        fi
        ln -s "$usr_share/dbus-1/system-services/org.kaiming.proxy.service" "$usr_share_ln/dbus-1/system-services/org.kaiming.proxy.service"
    fi

    # 安装 session dbus 运行所需文件，没有则会开启 dbus 失败
    if [ ! -e "$usr_share_ln/dbus-1/services/org.kaiming.proxy.service" ]; then
        if [ ! -d "$usr_share_ln/dbus-1/services" ]; then
            mkdir -p "$usr_share_ln/dbus-1/services"
        fi
        ln -s "$usr_share/dbus-1/services/org.kaiming.proxy.service" "$usr_share_ln/dbus-1/services/org.kaiming.proxy.service"
    fi

    # 安装 kaiming-custom.xml
    if [ ! -e "$usr_share_ln/mime/packages/kaiming-custom.xml" ]; then
        if [ ! -d "$usr_share_ln/mime/packages" ]; then
            mkdir -p "$usr_share_ln/mime/packages"
        fi
        ln -s "$usr_share/mime/packages/kaiming-custom.xml" "$usr_share_ln/mime/packages/kaiming-custom.xml"
    fi

    if [ ! -e "$usr_share_ln/icons/hicolor/scalable/mimetypes/application-x-kaiming.svg" ]; then
        if [ ! -d "$usr_share_ln/icons/hicolor/scalable/mimetypes/" ]; then
            mkdir -p $usr_share_ln/icons/hicolor/scalable/mimetypes/
        fi
        ln -s "$usr_share/icons/hicolor/scalable/mimetypes/application-x-kaiming.svg" "$usr_share_ln/icons/hicolor/scalable/mimetypes/application-x-kaiming.svg"
    fi

    #安装org.kylin.kaiming.policy
    if [ ! -e "$usr_share_ln/polkit-1/actions/org.kylin.kaiming.policy" ]; then
        if [ ! -d "$usr_share_ln/polkit-1/actions" ]; then
            mkdir -p "$usr_share_ln/polkit-1/actions"
        fi
        ln -s "$usr_share/polkit-1/actions/org.kylin.kaiming.policy" "$usr_share_ln/polkit-1/actions/org.kylin.kaiming.policy"
    fi

    # 安装 .mo 文件到指定的系统目录
    if [ ! -e "$usr_share_ln/locale/zh_CN/LC_MESSAGES/kaiming.mo" ]; then
        if [ ! -d "$usr_share_ln/locale/zh_CN/LC_MESSAGES" ]; then
            mkdir -p "$usr_share_ln/locale/zh_CN/LC_MESSAGES"
        fi
        ln -s "$usr_share/locale/zh_CN/LC_MESSAGES/kaiming.mo" "$usr_share_ln/locale/zh_CN/LC_MESSAGES/kaiming.mo" 
    fi

    if [ ! -e "${bin_ln_not_V11}/kaiming" ]; then
        ln -s "${bin_path}/kaiming" "${bin_ln_not_V11}/kaiming"
    fi

    if [ ! -e "${bin_ln_not_V11}/km-cli" ]; then
        ln -s "${bin_path}/km-cli" "${bin_ln_not_V11}/km-cli"
    fi

    if [ ! -e "${bin_ln_not_V11}/km-run" ]; then
        ln -s "${bin_path}/km-run" "${bin_ln_not_V11}/km-run"
    fi

    if [ ! -e "${bin_ln_not_V11}/kaiming-system-dbus" ]; then
	    ln -s "${bin_path}/kaiming-system-dbus" "${bin_ln_not_V11}/kaiming-system-dbus"
    fi

    if [ ! -e "${bin_ln_not_V11}/kaiming-sessionproxy-dbus" ]; then
        ln -s "${bin_path}/kaiming-sessionproxy-dbus" "${bin_ln_not_V11}/kaiming-sessionproxy-dbus"
    fi

    if [ ! -e "${bin_ln_not_V11}/kaiming-systemproxy-dbus" ]; then
	    ln -s "${bin_path}/kaiming-systemproxy-dbus" "${bin_ln_not_V11}/kaiming-systemproxy-dbus"
    fi

    if command -v update-mime-database >/dev/null; then
        update-mime-database $usr_share_ln/mime
    fi

else
    if [ ! -e "${bin_ln_V11}/kaiming" ]; then
        if [ ! -d "${bin_ln_V11}" ]; then
            mkdir -p "${bin_ln_V11}"
        fi
        ln -s "${bin_path}/kaiming" "${bin_ln_V11}/kaiming"
    fi

    if [ ! -e "${bin_ln_V11}/km-cli" ]; then
	    ln -s "${bin_path}/km-cli" "${bin_ln_V11}/km-cli"
    fi

    if [ ! -e "${bin_ln_V11}/km-run" ]; then
	    ln -s "${bin_path}/km-run" "${bin_ln_V11}/km-run"
    fi

    if [ ! -e "${bin_ln_V11}/kaiming-system-dbus" ]; then
	    ln -s "${bin_path}/kaiming-system-dbus" "${bin_ln_V11}/kaiming-system-dbus"
    fi
    if [ ! -e "${bin_ln_V11}/kaiming-sessionproxy-dbus" ]; then
        ln -s "${bin_path}/kaiming-sessionproxy-dbus" "${bin_ln_V11}/kaiming-sessionproxy-dbus"
    fi
    if [ ! -e "${bin_ln_V11}/kaiming-systemproxy-dbus" ]; then
	    ln -s "${bin_path}/kaiming-systemproxy-dbus" "${bin_ln_V11}/kaiming-systemproxy-dbus"
    fi

    if command -v update-mime-database >/dev/null; then
        update-mime-database $usr_share/mime
    fi
fi
