# ukui-greeter

![Check build](https://github.com/ukui/ukui-greeter/workflows/Check%20build/badge.svg?branch=master)

ukui-greeter是UKUI桌面环境的基于Lightdm的登录程序

### 依赖

------

### 编译依赖

- pkg-config
- qtbase5-dev
- libqt5x11extras5-dev
- libqt5svg5-dev
- liblightdm-qt5-3-dev
- libx11-dev
- libxtst-dev
- libxrandr-dev
- libglib2.0-dev
- libopencv-dev
- qttools5-dev-tools
- libxi-dev
- libimlib2-dev
- libgsettings-qt-dev

### 运行依赖

- lightdm
- ethtool

### 编译

------

```shell
$ cd ukui-greeter
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
    - [x] 界面美化
  - **TROUBLE**
    - 无
  - **TODO**
    - 功能插件