kylin-assistant
================

Attention
=========
It supports unity and mate for Ubuntu/Ubuntu Kylin.

Depends
=======
debhelper (>= 9),libqt5charts5-dev,qtbase5-dev (>= 5.1),libqt5svg5-dev,libqt5charts5-dev,qt5-qmake,qtscript5-dev,qttools5-dev-tools,libgsettings-qt-dev,pkg-config,libglib2.0-dev (>= 2.46.0),libgtop2-dev,libkf5windowsystem-dev,libqt5x11extras5-dev,libxrandr-dev,libudev-dev,libukui-log4qt-dev,libsdl2-dev,libepoxy-dev

GitHub
=======
https://github.com/KylinAppCenter/kylin-assistant

Internationalization
=======
lupdate kylin-assistant.pro
linguist kylin-assistant_zh_CN.ts
lrelease kylin-assistant.pro

Python Internationalization
=======
xgettext -k_ -o kylin-assistant.pot cpuinfo.py
kylin-assistant.pot:修改charset为utf-8
cp kylin-assistant.pot kylin-assistant.po
msgfmt -o kylin-assistant.mo kylin-assistant.po
(sudo cp kylin-assistant.mo /usr/share/locale/zh_CN/LC_MESSAGES/)




FT Version:
changelog like this:
youker-assistant (3.0.2-0) v101; urgency=medium

DEBUG:
gdb python
set args kasystemdbusmain.py
run
