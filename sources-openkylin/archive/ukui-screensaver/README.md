## ukui-screensaver

![build](https://github.com/ukui/ukui-screensaver/tree/master)

ukui-screensaver是UKUI桌面环境的锁屏及屏保

### 依赖

------

### 编译依赖

- KF5
  - libkf5windowsystem-dev

- cmake (>=2.6)
- qtbase5-dev
- libqt5x11extras5-dev
- libpam0g-dev
- qttools5-dev
- qttools5-dev-tools
- libglib2.0-dev
- libopencv-dev
- libx11-dev
- libxtst-dev
- libqt5svg5-dev
- libgsettings-qt-dev
- libmatemixer-dev
- libukui-log4qt-dev

### 运行依赖

- ukui-session-manager
- ethtool
- mate-desktop-common

### 编译

------

```shell
$ cd ukui-screensaver
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### 安装

------

```shell
$ sudo make install
```

### 主体框架
  - **InProgress**
    - [x] 界面绘制
    - [x] 功能实现
    - [x] 界面美化
  - **TROUBLE**
    - 无
  - **TODO**
    - 功能插件
    - 应用窗口置顶
