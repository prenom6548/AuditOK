# UKUI4\.0桌面环境通知协议说明V1\.3

Iaom zhangpengfei@kylinos\.cn ukui sig组

changelog v1\.0\-\>v1\.1：超时时间统一为单位为ms

changelog v1\.1\-\>v1\.2：新增注册通知管理接口

changelog v1\.1\-\>v1\.3：新增是否允许应用发送通知时播放声音的接口

# **接口形式**

协议接口采用dbus形式，基本定义如下：

服务名：org\.freedesktop\.Notifications

对象路径：/org/freedesktop/Notifications

接口：org\.freedesktop\.Notifications

org\.ukui\.NotificationServer

org\.ukui\.NotificationSettings

本协议描述UKUI4\.0桌面环境中‘ukui\-notification'组件提供的通知服务接口协议。

本协议基于freedesktop中的‘[Desktop Notifications Specification](https://specifications.freedesktop.org/notification-spec/notification-spec-latest.html)’部分扩展而来，兼容原有协议1\.2版本功能。

具体接口如表1\.1所示：

表 1\.1

# **具体说明**

## **2\.1 获取通知服务功能**

应用通过调用org\.freedesktop\.Notifications\.GetCapabilities接口获取当前通知服务实现的能力。返回值包括：



## **2\.2 通知发送**

应用通过调用org\.freedesktop\.Notifications\.Notify接口发送通知消息给系统通知服务。

输入字段具体说明如下：

### **2\.2\.1 app\_name**

string类型，表示通知发送者的应用名称，会显示在通知弹窗上，通知中心会根据此字段进行通知的分组。

### **2\.2\.2 replaces\_id**

uint32类型，用于标识当前这条通知要替换的通知，通知中心会用新的通知内容替换旧的通知内容。如果为0,则标识当前的通知是新通知。

### **2\.2\.3 summary**

string类型，表示通知的摘要文本，会直接显示在通知弹窗上。

### **2\.2\.4 body**

string类型，消息正文，会直接显示在通知弹窗上。body可以包含部分HTML标记：

表2\.1\.4\.1

说明：

1\.Hyperlinks

超链接允许将文本链接到URL。

2\.Images

可实现在通知消息同放入图片功能，尺寸不能超过200\*100，且必须通过alt="\.\.\."提供可替代的文本，消息文本中不能嵌入图片数据。引用的图片必须是本地文件。

### **2\.2\.5 actions**

as类型，表示通知包含的actions及执行动作信息，列表中两个为一组，偶数位（从0开始）表示一条action的唯一标识，奇数位表示要显示给用户的的内容。

一般点击消息体产生的action为“default”，对应的内容无要求，一般不会显示给用户（但是需要在发送协议中和action拼成一组）。有额外需要用户操作的action则会在通知弹窗里以按钮形式展示。

The default action \(usually invoked by clicking the notification\) should have a key named "default"\. The name can be anything, though implementations are free not to display it\.

### **2\.2\.6 hints**

a\{sv\}类型，hints提供了向通知服务发送一些额外数据的功能，以键值对列表形式传递，具体定义如表2\.1\.6\.1所示（）：

表2\.2\.6\.1

其中通知种类为：

表2\.2\.6\.2

紧急程度：

表2\.1\.6\.3

UKUI环境下设计要求：

![Image](https://internal-api-drive-stream.feishu.cn/space/api/box/stream/download/authcode/?code=YjlhYzViN2Q0YjcyNWNlOTk2ZGEzZTk1MTY1YWQ2YzhfYTU5ZDdlMmVmODY3NzE1ZTc0OTVhNzc0MWY1NThlMDhfSUQ6NzM4MDI1NDE2NjUyODY4ODEzMF8xNzgxMTYzNjc1OjE3ODEyNTAwNzVfVjM)

action状态：

表2\.1\.6\.4

### **2\.2\.6 expire\_timeout**

uint32类型，表示通知自动删除的时间。

## **2\.3 通知关闭**

应用通过调用org\.freedesktop\.Notifications\.CloseNotification接口，可通知通知服务删除一条消息。

## **2\.4 获取服务信息**

应用通过调用org\.freedesktop\.Notifications\.GetServerInformation接口，可获取通知服务信息。

表2\.4\.1

## **2\.5 更新未读消息数量**

应用通过org\.ukui\.NotificationServer\.UpdateUnreadMessagesNumber 更新未读消息数量

## **2\.6 注册通知管控**

应用通过org\.ukui\.NotificationServer\.RegisterNotificationControl， 主动注册到控制面板通知管控， 注册后用户可通过控制面板通知页去配置应用通知的相关配置。

## **2\.7 获取闹铃提示状态**

通过org\.ukui\.NotificationSettings\.IsAlarmSoundAllowed ，获取当前是否允许闹钟播放闹铃的状态

## **2\.8 获取应用是否允许播放通知提示音**

通过org\.ukui\.NotificationSettings\.IsSoundAllowed ，获取当前查询的应用是否允许播放通知提示音

## **2\.****9 ****通知关闭信号**

应用通过监听org\.freedesktop\.Notifications\.NotificationClosed信号，可获取通知关闭信号，参数说明如下：

## **2\.****10**** 动作执行信号**

动作执行信号org\.freedesktop\.Notifications\.ActionInvoked在用户执行由通知发送这指定的actions时发送，参数id表示触发action的通知id，action\_key表示action列表中唯一的一条action。

## **2\.****11**** ActivationToken信号**

此信号暂不支持

## **2\.12 AlarmSoundAllowedChanged信号**

更新闹钟是否可以播放闹铃状态的信号org\.ukui\.NotificationSettings\.AlarmSoundAllowedChanged，此信号在允许闹钟闹铃播放状态改变时发出。



