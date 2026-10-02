## 如何编译

尽量提供了所有依赖资源，方便傻瓜式编译。

### PC端

1. 在麒麟操作系统上搭建Qt开发环境
   Qt版本>=5.12
2. 克隆该项目
3. 根据kylin-connectivity.pro 或 CmakeList.txt 打开项目
4. 编译，运行

### 依赖

* qtbase5-dev,
* qt5-qmake,
* qttools5-dev-tools,
* qttools5-dev,
* libkf5windowsystem-dev,
* libqt5x11extras5-dev,
* cmake,
* pkgconf,
* libukui-log4qt-dev,
*  libgsettings-qt-dev,
* adb,
*  libavformat-dev,
*  libswscale-dev,
*  libavcodec-dev,
*  libavutil-dev,
* libffmpegthumbnailer-dev,
* libqrencode-dev,
* libkysdk-applications(>=1.0.0kylin8),
*  vlc,
* libsystemd-dev,
*  libglib2.0-dev,
*  libkysdk-qtwidgets-dev

### 编译

```
mkdir build
cd build
cmake ../
make
sudo make install
```

