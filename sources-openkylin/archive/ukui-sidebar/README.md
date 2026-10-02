# ukui-sidebar 
## 简介
ukui-sidebar是ukui桌面环境中的侧边栏快捷工具，主要包括控制中心和通知中心两个部分。
### 编译
```shell
# 使用不同发行版对应的包管理器安装编译依赖，以debian系举个栗子：
# 1.0. 侧边栏目前的依赖如下(可能会有改动)：
pkgconf libxcb1-dev libgsettings-qt-dev qtbase5-dev qt5-qmake qtchooser qttools5-dev-tools qtdeclarative5-dev libkf5windowsystem-dev libqt5x11extras5-dev libkysdk-waylandhelper-dev libopencv-dev
# 1.1. 直接使用apt下载并安装全部依赖
sudo apt install pkgconf libxcb1-dev libgsettings-qt-dev qtbase5-dev qt5-qmake qtchooser qttools5-dev-tools qtdeclarative5-dev libkf5windowsystem-dev libqt5x11extras5-dev libkysdk-waylandhelper-dev libopencv-dev

# 2.0. 也可以使用mk-build-deps工具，检查并安装编译依赖，该工具包含在devscripts包中
sudo apt install devscripts
# 2.1. 自动安装依赖，在源码根目录执行：
sudo mk-build-deps -i

# 2.2. 手动检查并安装依赖，在源码根目录执行：
sudo mk-build-deps
# 2.3. 查看生成的 xxx.deb包(xxx是版本号)，使用dpkg -I解析deb包的编译依赖
dpkg -I ukui-sidebar-build-deps_xxx_all.deb

# 3. 编译：进入源码根目录，打开终端，编译侧边栏
mkdir build; cd build; qmake ..; make; sudo make install
```

### 控制中心
控制主要功能是提供了一组系统设置快捷键，类似手机上的下滑控制中心。快捷键按钮均基于libukui-shortcut-dev提供的插件接口实现。
### 通知中心
通知中心可以显示用户最近的消息通知，提供了便捷的消息分组查看和删除功能。
## 运行
ukui-sidebar作为ukui桌面环境组件，默认开机自启，且无法关闭。侧边栏有pc模式和平板模式两种显示模式，pc模式下，侧边栏可通过点击托盘图标或从屏幕右边缘左滑唤起，也可以通过快捷键 super + a 唤起。平板模式下，侧边栏分为控制中心和通知中心两部分，控制中心唤起方式和上面提到的pc模式的三种方法相同，通知中心需要从屏幕上边缘下滑唤起。
## 快捷键，命令行和交互接口
### 快捷键
目前侧边栏只有 super + a 一组快捷键。
### 命令行
```shell
Options: 
  -S, --state          Show the current state of the sidebar.
  -s, --show <option>  There are two options, 'notify' and 'control'.
  -q, --quit           Quit sidebar.
  -h, --help           Displays this help.
  -v, --version        Displays version information.
```
### 交互接口
#### 控制中心
控制中心中的快捷键插件使用很多ukui桌面环境其他组件的接口，多为gsettings或dbus形式。

========此处待补充========

#### 通知中心
侧边栏启动之后会注册一个dbus接口：
```
service:"org.ukui.Sidebar"
path:"/org/ukui/Sidebar/notification"
interface:"/org/ukui/Sidebar/notification"
sidebarNotification (String appName, String appIcon, String summary, String body, String urlStr, String action) ↦ ()
```
ukui-notification-center(ukui通知中心组件)会按照一定的规则，调用这个接口发送消息给侧边栏通知中心。目前侧边栏支持的通知类型比较单一，不支持其他复杂的如常驻通知，实时刷新状态的通知等。
我们正在考虑将通知中心组件（ukui-notification-center）的功能整合进侧边栏，并基于freedesktop协议，丰富通知中心的功能。
##### 通知中心计划
=======todo=======
## 编译与调试
### 编译
```shell
git clone 
cd ukui-sidebar
mkdir build;cd build
qmake ..
make
```

ukui-sidebar的项目接口主要分为libukui-shortcut（快捷键插件管理器）和src（ukui-sidebar应用本体）两部分。
libukui-short提供了侧边栏控制中心快捷键插件接口，src为侧边栏应用主体。编译完成后执行sudo make install即可完成安装。
### 调试
安装完成后，终端执行ukui-sidebar即可启动侧边栏。在~/.config/org.ukui/目录新建ukui-sidebar.log日志文件，应用日志会自动输出到此文件。
> 暂时没有日志自动记录，备份或删除机制
## 开发接口
基于Qt插件框架，目前侧边栏提供了一个控制中心快捷操作的插件接口：
```C++
class UKUISHORTCUT_EXPORT Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = nullptr) : QObject(parent) {}
    //插件标识
    virtual QString pluginId() = 0;
    //插件类型（样式）, 需要指定不同模式下的按钮
    virtual QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() = 0;
    //实现响应前端用户操作
    virtual void active(PluginMetaType::Action action) = 0;
    //进度条样式需要重写该函数
    virtual void setValue(int value) {Q_UNUSED(value)}
    //返回插件当前的状态信息
    virtual const StatusInfo currentStatus() = 0;
    //是否启用
    virtual bool isEnable() { return true; }

Q_SIGNALS:
    void statusChanged(const StatusInfo &info);
    void enableStatusChanged(bool isEnable);
};
```
基于Qt插件框架，此接口可以提供三种按钮格式的接口:
```C++
enum PluginType
{
    Icon = 0,    //图标样式
    ProgressBar, //进度条样式
    MenuButton   //可展开菜单按钮样式
};

```
快捷面板插件demo位于https://gitee.com/qiqi49/shortcut-plugin-test，实现了一个按钮类型的插件，功能为点击即打开默认浏览器访问B站。

### Use with CMake:
```cmake
find_package(PkgConfig)
pkg_check_modules(ukui-shortcut REQUIRED ukui-shortcut)
include_directories(${ukui-shortcut_INCLUDE_DIRS})
target_link_libraries(yourapp ukui-shortcut)
```
### Use with Qmake：
```qmake
CONFIG += link_pkgconfig
PKGCONFIG += ukui-shortcut
```
