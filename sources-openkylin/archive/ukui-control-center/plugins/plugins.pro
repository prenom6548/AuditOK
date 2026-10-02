##对预编译宏做处理
include(../env.pri)

TEMPLATE = subdirs

SUBDIRS = devices/printer \
          devices/mouse \
          devices/shortcut \
          devices/keyboard \
          personalized/wallpaper \
          personalized/theme \
          time-language/datetime \
          time-language/area \
          application/defaultapp \
          application/autoboot \
          system/display \
          system/vino \
          system/about \
          personalized/fonts \
          account/userinfo \
          security-updates/backup \
          commoninfo/boot \
          commoninfo/accessibility \
