# ukui-notification

## 介绍
ukui-notificaton包含UKUI桌面环境的通知服务、通知开发接口和通知控制插件。
- ukui-notification-server: 通知服务
- libukui-notification: 通知开发接口
- notification-ukcc-plugin: 通知控制插件

## 目录
- [通知服务](#通知服务)
- [通知开发接口](#通知开发接口)
- [通知控制插件](#通知控制插件)
- [贡献指南](#贡献指南)

## 通知服务
ukui-notifiaction-server提供了遵循freedesktop[通知协议](https://specifications.freedesktop.org/notification-spec/notification-spec-latest.html)的通知服务接口。

### 支持的Capabilities
| 能力 | 描述 |
|------|------|
| actions | 支持通知操作按钮 |
| body | 支持富文本内容 |
| body-hyperlinks | 支持内容中的超链接 |
| persistence | 支持持久化通知 |
| sound | 支持通知声音提示 |

### 扩展hints
| Hint名称 | 描述 |
|---------|------|
| x-ukui-display | 标识通知弹窗应该显示在哪个DISPLAY上 |

### 扩展接口
| 接口名称 | 参数 | 返回值 | 描述 |
|---------|------|--------|------|
| org.ukui.NotificationServer.UpdateUnreadMessagesNumber | String desktop_entry, UInt32 number | void | 更新应用的未读消息数量 |
| org.ukui.NotificationServer.RegisterNotificationControl | String desktop_entry | void | 注册应用的通知控制 |
## 通知开发接口
### 使用cmake:
```cmake
find_package(ukui-notification)
target_link_libraries(yourapp ukui-notification)
```
### 使用qmake:
```qmake
CONFIG += link_pkgconfig
PKGCONFIG += ukui-notification
```
### 关键接口：
| 头文件 | 功能描述 | 相关类 |
|--------|----------|--------|
| notification-client.h | 通知中心客户端注册、消息同步与action执行 | NotificationClient |
| popup-notification.h | 单条通知的所有信息管理 | PopupNotification |
| notification-global-settings.h | 全局通知设置 | NotificationGlobalSettings |
| applications-settings.h | 应用通知行为设置 | ApplicationSettings |
| single-application-settings.h | 单个应用的通知设置 | SingleApplicationSettings |

## 通知控制插件
控制面板通知插件，方便用户设置应用通知的权限

## 贡献指南
### 依赖
debhelper-compat (= 12),
libukui-search-dev(>=4.0.1.5-ok0),
pkgconf,
qtbase5-dev,
qtchooser,
qtdeclarative5-dev,
qtscript5-dev,
qttools5-dev,
cmake,
libkysdk-qtwidgets-dev,
 libukcc-dev,
libgsettings-qt-dev,
extra-cmake-modules,
doxygen
### 编译：
```
mkdir build && cd build
cmake ..
make -j$(nproc)
```