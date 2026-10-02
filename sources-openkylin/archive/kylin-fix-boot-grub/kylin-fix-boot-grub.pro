QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(src/src.pri)

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \

HEADERS += \


RESOURCES += \
    resource.qrc

TRANSLATIONS += translations/kylin-fix-boot-grub_zh_CN.ts

translation.files += translations/*.qm
translation.path = /usr/share/kylin-fix-boot-grub/

# 安装shell修复脚本
bash_fix_grub.path = /usr/share/kylin-fix-boot-grub/fix-bash
bash_fix_grub.files += ./fix-bash/fix-boot-bash
bash_fix_grub.files += ./fix-bash/fix-grub-real
INSTALLS += bash_fix_grub

# 安装grub修复启动脚本
start_bash_fix_grub.path = /usr/bin/
start_bash_fix_grub.files += ./fix-bash/start-kylin-fix-boot-grub
INSTALLS += start_bash_fix_grub

# 安装system服务文件，用于启动时使用
system_fix_service.path = /lib/systemd/system
system_fix_service.files += ./data/system/kylin-fix-boot-grub.service
INSTALLS += system_fix_service

# 安装wayland产线，特有的session拉起UI界面的配置文件
wayland_conf_file.path = /usr/share/kylin-fix-boot-grub/data/kvwayland/
wayland_conf_file.files += ./data/kvWayland/99-w-wayland-fix-boot-grub.conf
INSTALLS += wayland_conf_file

# 安装wayland产线，特有的session拉起UI界面启动文件
wayland_boot_file.path = /usr/share/kylin-fix-boot-grub/data/kvwayland/
wayland_boot_file.files += ./data/kvWayland/w-wayland-fix-boot-grub
INSTALLS += wayland_boot_file

# 安装wayland产线，特有的session拉起UI界面启动文件
wayland_shell_wrapper.path = /usr/share/kylin-fix-boot-grub/data/kvwayland/
wayland_shell_wrapper.files += ./data/kvWayland/wrapper-kylin-fix-boot-grub
INSTALLS += wayland_shell_wrapper

# 安装wayland产线，特有的session拉起UI界面启动文件
wayland_desktop_file.path = /usr/share/kylin-fix-boot-grub/data/kvwayland/
wayland_desktop_file.files += ./data/kvWayland/wayland-kylin-fix-boot-grub.desktop
INSTALLS += wayland_desktop_file

# 安装system_generators脚本，根据cmdline启动参数，设置kylin-fix-boot-grub.service 服务able状态
#system_generators_fix_service.path = /lib/systemd/system-generators
#system_generators_fix_service.files += ./data/system-generators/kylin-fix-boot-grub
#INSTALLS += system_generators_fix_service

desktopfile.files = ./data/kylin-fix-boot-grub.desktop
desktopfile.path += /usr/share/applications/
INSTALLS += desktopfile

# Default rules for deployment.

target.path = /usr/bin
INSTALLS += target translation
