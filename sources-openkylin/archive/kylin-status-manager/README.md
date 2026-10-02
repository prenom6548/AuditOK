# kylin-status-manager

## 1.引言
kylin-status-manager是UKUI桌面环境下维护设备状态的程序。状态管理器提供供外部组件（如侧边栏、控制面板）查询和设置设备状态的接口，且在设备模式状态（模式变化和自动切换开关变化）、设备旋转状态（自动旋转开关和设备方向变化）和物理键盘可用性变化时，通知其他系统组件做出相应改变，且状态管理器内部会对Kmre应用进行定制化处理。本文档为用户配置服务介绍，针对组件的项目依赖、影响组件、环境部署和目录结构等方面进行介绍。  

## 2.项目依赖
执行: sudo mk-build-deps -ir debian/control 会安装项目所有依赖  

## 3.部署方式
- 编译部署
1. mkdir build
2. cd build && qmake ..
3. make -j 
4. sudo make install
5. killall kylin-status-manager（状态管理器会在kill后自启）  

- 编包部署
1. 根目录下执行debuild命令，在上层目录生成 deb 文件
2. cd .. 回到上层目录
3. sudo dpkg -i kylin-status-manager*.deb
4. 安装后，重启机器或killall kylin-status-manager  

## 4.影响和依赖组件
- 影响组件
ukui-settings-daemon、切换平板模式的包、侧边栏、控制面板、虚拟键盘

- 依赖组件
依赖硬件设备支持tablet switch event 事件上报
iio-sensor-proxy  

## 5.目录结构描述
```
.
├── keyboardmanager
│   ├── keyboardmanager.cpp
│   └── keyboardmanager.h
├── ConfigurationManager
│   ├── configurationmanager.cpp
│   └── configurationmanager.h
├── COPYING
├── COPYING.LIB
├── data
│   ├── com.kylin.statusmanager.interface.service
│   ├── conf.ini
│   ├── kylin-status-manager.desktop
│   └── statusManagerConfig.ini
├── DbusManager
│   ├── com.kylin.statusmanager.interface.xml
│   ├── dbusservice.cpp
│   └── dbusservice.h
├── debian
│   ├── changelog
│   ├── control
│   ├── copyright
│   ├── install
│   ├── rules
│   └── source
│       └── format
├── DeviceModeManager
│   ├── devicemodemanager.cpp
│   └── devicemodemanager.h
├── devicerotationmanager.cpp
├── devicerotationmanager.h
├── DevicesStatusManager
│   └── Rotation_Monitoring
│       ├── rotationmonitor.cpp
│       └── rotationmonitor.h
├── KmreAppManager
│   ├── kmremanager.cpp
│   ├── kmremanager.h
│   ├── kmrenotificationwindowmanager.cpp
│   └── kmrenotificationwindowmanager.h
├── kylin-status-manager.pro
├── main.cpp
├── QtSingleApplication
│   ├── qtlocalpeer.cpp
│   ├── qtlocalpeer.h
│   ├── QtLockedFile
│   ├── qtlockedfile.cpp
│   ├── qtlockedfile.h
│   ├── qtlockedfile_unix.cpp
│   ├── qtlockedfile_win.cpp
│   ├── QtSingleApplication
│   ├── qtsingleapplication.cpp
│   ├── qtsingleapplication.h
│   ├── qtsingleapplication.pri
│   ├── qtsinglecoreapplication.cpp
│   ├── qtsinglecoreapplication.h
│   └── qtsinglecoreapplication.pri
├── README.md
├── src.qrc
├── StatusManager
│   ├── machinemodel.cpp
│   ├── statusmanager.cpp
│   └── statusmanager.h
└── translate
    ├── zh.qm
    └── zh.ts

13 directories, 52 files

```
## 6.模块介绍
|    模块     | 功能介绍                      |
|------------|------------------------------|
|状态管理器    | 管理着下述模块之间的交互|
|物理键盘管理器 | 记录着键盘的可用性，以及更新键盘可用性的功能|
|设备模式管理器 | 记录着模式状态、自动切换开关状态和唤起用户手册、弹窗等功能|
|设备旋转管理器 | 监听这QSensor传感器，获取、更新当前设备方向和自动旋转开关状态的维护|
|kmre管理器    | 处理关于kmre应用的相关功能|
|DBUS服务      | 暴露给外接设置和获取设备状态的接口，以及更新状态后发送信号通知其他组件|
|配置文件管理器  | 对设备状态做持久化处理|

## 7.DBus接口介绍
1. Dbus基本详情  

|    TYPE       |              INFO                |
|---------------|----------------------------------|
|DBUS TYPE      |         SESSION BUS              |
|DBUS NAME      | com.kylin.statusmanager.interface|
|OBJECT PATH    |               /                  |
|INTERFACE NAME |com.kylin.statusmanager.interface |

2. 获取方法  

|       method call         |          args          |                           return                         |                      means                      |
|---------------------------|------------------------|----------------------------------------------------------|-------------------------------------------------|
|get_inputmethod_mode()     |          NULL          | 返回值（int） 切换虚拟键盘（0） 切换物理键盘（1） 返回值错误（-1） |              获得当前系统应该显示的键盘模式          |
|get_current_tabletmode()   |          NULL          |          返回值（bool）平板（true）pc（false）                   |                    获得当前的机器模式              |
|get_current_rotation()     |          NULL          |         返回值（string）normal upside-down left right      |                    获得当前的旋转情况              |
|get_modemonitor()          |          NULL          |        返回值（bool）开启自动切换（true）关闭自动切换（false）   |用于控制翻折时的平板与pc的自动切换，获得是否开启自动切换情况|
|get_auto_rotation()        |          NULL          |        返回值（bool）开启自动旋转（true）关闭自动旋转（false）   | 用于控制旋转时的屏幕状态监测，获得是否开启自动旋转情况    | 
|is_supported_autorotation()|          NULL          |         返回值（bool）支持（true）不支持（false）                 |                 当前设备是否支持方向传感器           |

3. 设置方法  

|                        method call                         |                                     args                                          |             return                |           means            |
|------------------------------------------------------------|-----------------------------------------------------------------------------------|-----------------------------------|----------------------------|
|set_tabletmode(bool tablet_mode,QString who,QString why)    |bool tablet_mode：(true/false),QString who：使用该接口的组件名,QString why：为使用的目的 | 返回值（bool）设置成功（1）设置失败（0) |        设置当前的机器模式    |
|set_rotation(QString rotation,QString who,QString why)      |bool rotation：（normal/upside-down/left/right）,QString who：同上,QString why：同上  | 返回值（bool）设置成功（1）设置失败（0） |        设置当前的旋转情况    |
|set_modemonitor(bool modemonitor,QString who,QString why)   |          bool modemonitor：(true/false),QString who：同上,QString why：同上         | 返回值（bool）设置成功（1）设置失败（0   |        设置自动切换开关      |
|set_auto_rotation(bool autorotation,QString who,QString why)|                 bool autorotation,QString who,QString why                         | 返回值（bool）设置成功（1）设置失败（0） |         设置自动旋转开关     |

4. 信号

|                        method call                  |                          return/args                        |                  means            |
|-----------------------------------------------------|-------------------------------------------------------------|-----------------------------------|
|        inputmethod_change_signal(bool)              |               切换虚拟键盘（false）切换物理键盘（true）             |  系统状态改变发送当前应该显示的键盘模式  |
|       rotations_change_signal(QString rotations)    |                 normal upside-down left right               |            返回当前的旋转模式         |
|       mode_change_signal(bool tablet_mode)          |                 平板模式（true）PC模式（false）                     |    根据当前翻折情况返回当前的机器模式    |
|  void modemonitor_change_signal(bool modemonitor)   |              开启自动切换（true）关闭自动切换（false）               |       自动切换状态改变时发送信号       |
| void auto_rotation_change_signal(bool auto_rotation)|              开启自动旋转（true）关闭自动旋转（false）               |       自动旋转状态改变时发送信号       |

## 8.配置文件
~/.config/statusManager.ini 存储这当前用户设置的设备状态；
- 字段说明  

[DeviceMode]               ——  设备模式字段  

autoSwitchEnable=true      ——  自动切换是否开启  

tabletMode=false           ——  当前设备模式  

[Rotation]                 ——  设备旋转方向字段  

autoRotateEnbale=false     ——  自动旋转是都开启  

deviceOrientation=normal   ——  设备的屏幕方向  

[StartupOptions]           ——  启动项字段  

firstSwitchDeviceMode=true ——  记录是否第一次切换设备模式  

## changed
1、修改启动顺序，在desktop设置为、
OnlyShowIn=UKUI;
X-UKUI-Autostart-Phase=Initialization
X-UKUI-AutoRestart=true
并修改调用session的dbus,告知session程序已启动

## 接口链接  
https://docs.qq.com/doc/DUVpOU2t5Tm1VYklj （内部使用）

https://docs.qq.com/doc/DUUx6WXBvV0dsZ09D
