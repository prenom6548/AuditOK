# UKUI-Bluetooth
## Simple bluetooth tool for ukui desktop environment

### 依赖
- KF5
  - libkf5bluezqt6
  - libkf5bluezqt-dev
  - libkf5windowsystem-dev
  - libqt5x11extras5-dev
  - libgsettings-qt-dev
  - libkf5bluezqt-data

- libglib2.0-dev,
- libxrandr-dev,
- libxinerama-dev,
- libxi-dev,
- libxcursor-dev,
- libukui-log4qt-dev
- bluez
- bluez-obexd

### 编译

------

```shell
$ cd ukui-bluetooth
$ mkdir build
$ cd build
$ qmake ..
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
    - [x] 基础DBus接口提供
  - **TROUBLE**
    - 无
  - **TODO**
    - 无

### 功能插件
##### 蓝牙服务bluetoothService
  - **InProgress**
    - [x] 功能实现
    - [x] 基础DBus接口提供
  - **TODO**
    - 无
##### 任务栏蓝牙ukui-bluetooth
  - **InProgress**
    - [x] 界面绘制
    - [x]  功能实现
  - **TODO**
    - 无
##### 控制面板蓝牙ukcc-bluetooth
  - **InProgress**
    - [x] 界面绘制
    - [x]  功能实现
  - **TODO**
    - 无
