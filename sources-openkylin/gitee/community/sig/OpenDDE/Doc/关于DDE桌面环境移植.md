# 关于DDE桌面环境移植openkylin

> OpenDDE SIG 移植工作简介文档

## 简介

深度桌面环境是由国人开发的美观易用、极简操作、功能丰富的桌面环境，主要由桌面、启动器、任务栏、控制中心、窗口管理器等组成

[移植参考文档 -- Deepin wiki](https://wiki.deepin.org/zh/%E5%BE%85%E5%88%86%E7%B1%BB/02_deepin%E6%B7%B1%E5%85%A5/02_DDE%E7%9B%B8%E5%85%B3/00_DDE%E6%A1%8C%E9%9D%A2%E7%A7%BB%E6%A4%8D/%E8%BD%AF%E4%BB%B6%E5%8C%85%E5%88%86%E7%B1%BB%E4%B8%8E%E7%AE%80%E4%BB%8B)

---
## 关于软件包分级

我们根据软件包对移植工作的重要性分为一级，二级，三级

一级：能让DDE桌面正常工作的包，重要优先级

二级：DDE桌面的应用

三级：Deepin系统中的可选功能应用

---
## 核心组件（一级）

### dtkcommon
- 介绍：一个用来构建DTK库的公共项目，DTK框架
- 源码地址：https://github.com/linuxdeepin/dtkcommon

### dtkcore
- 介绍：Deepin Tool Kit Core(DtkCore) 是所有C++/Qt开发人员在Deepin上工作的基础开发工具
- 源码地址：https://github.com/linuxdeepin/dtkcore

### dtkgui
- 介绍：Deepin Tool Kit Gui(DtkGui) 提供开发图形用户界面应用程序所需的功能
- 源码地址：https://github.com/linuxdeepin/dtkgui

### dtkwidget
- 介绍：Deepin Tool Kit Widget(DtkWidget) 提供各种UOS风格dtk基础控件
- 源码地址：https://github.com/linuxdeepin/dtkwidget

### qt5integration
- 介绍：qt5integration 是用于深度桌面环境的Qt平台主题集成插件。它包括多个Qt5插件使DDE对Qt5应用程序更加友好

### qt5platform-plugins
- 介绍：深度桌面环境的Qt平台集成插件
- 源码地址：https://github.com/linuxdeepin/qt5platform-plugins

### dde-qt-dbus-factory
- 介绍：dde-qt-dbus-factory 是用来统一存放 dde 使用到的自动生成的 Qt DBus 代码的仓库
- 源码地址：https://github.com/linuxdeepin/dde-qt-dbus-factory

---
## 应用组件

### dde-file-manager
- 介绍：深度桌面环境的多功能文件管理工具
- 源码地址：https://github.com/linuxdeepin/dde-file-manager

### deepin-devicemanager
- 介绍：深度桌面设备管理器，用于查看、管理硬件设备的工具软件
- 源码地址：https://github.com/linuxdeepin/deepin-devicemanager

### dde-control-center
- 介绍：深度桌面环境的系统设置工具
- 源码地址：https://github.com/linuxdeepin/dde-control-center

### dtkdeclarative
- 介绍：是基于 QtQuick/QtQml 实现的控件开发库，它是用于代替已有的 dtkwidget 编程而开发出来的一个全新的 DTK 模块
- 源码地址：https://github.com/linuxdeepin/dtkdeclarative

### deepin-log-viewer
- 介绍：系统日志查看的软件工具
- 源码地址：https://github.com/linuxdeepin/deepin-log-viewer

### dde-application-manager
- 介绍：深度桌面环境的应用程序管理器
- 源码地址：https://github.com/linuxdeepin/dde-application-manager

### deepin-desktop-theme
- 介绍：深度桌面环境的主题包
- 源码地址：https://github.com/linuxdeepin/deepin-desktop-theme

### dde-dock
- 介绍：DDE Dock 是深度桌面环境的任务栏组件
- 源码地址：https://github.com/linuxdeepin/dde-dock

### deepin-camera
- 介绍：深度桌面环境的摄像机应用
- 源码地址：https://github.com/linuxdeepin/deepin-camera

### dde-network-core
- 介绍：深度桌面环境网络核心框架
- 源码地址：https://github.com/linuxdeepin/dde-network-core

### dde-kwin
- 介绍：深度桌面 Xorg 窗口系统（Wayland，X11）
- 源码地址：https://github.com/linuxdeepin/deepin-kwin

### dde-session-shell
- 介绍：DDE session shell 提供两个应用程序：dde-lock 和 lightdm-deepin-greeter，dde-lock提供锁屏界面，lightdm-deepin-greeter提供登录界面
- 源码地址：https://github.com/linuxdeepin/dde-session-shell

### deepin-diskmanager
- 介绍：深度桌面环境磁盘管理器
- 源码地址：https://github.com/linuxdeepin/deepin-diskmanager

### deepin-album
- 介绍：深度桌面环境的相册
- 源码地址：https://github.com/linuxdeepin/deepin-album

### deepin-screen-recorder
- 介绍：深度桌面环境的录屏工具
- 源码地址：https://github.com/linuxdeepin/deepin-screen-recorder

### deepin-deb-installer
- 介绍：深度桌面环境的deb软件包安装器
- 源码地址：https://github.com/linuxdeepin/deepin-deb-installer

### deepin-unioncode
- 介绍：深度开发环境
- 源码地址：https://github.com/linuxdeepin/deepin-unioncode


### dde-appearance
- 介绍：dde-appearance 是一个用来设置DDE桌面主题和外观的程序
- 源码地址：https://github.com/linuxdeepin/dde-appearance


### deepin-music
- 介绍：深度桌面环境的音乐播放器
- 源码地址：https://github.com/linuxdeepin/deepin-music

### deepin-compressor
- 介绍：深度桌面环境的归档管理器，提供对文件解压、压缩常用功能的软件工具
- 源码地址：https://github.com/linuxdeepin/deepin-compressor

### deepin-movie-reborn
- 介绍：深度桌面环境的视频播放器
- 源码地址：https://github.com/linuxdeepin/deepin-movie-reborn



### deepin-desktop-base
- 介绍：深度桌面环境的主要包
- 源码地址：https://github.com/linuxdeepin/deepin-desktop-base

### deepin-image-viewer
- 介绍：深度桌面环境的图片查看器
- 源码地址：https://github.com/linuxdeepin/deepin-image-viewer

### deepin-draw
- 介绍：深度桌面环境的图片查看器
- 源码地址：https://github.com/linuxdeepin/deepin-draw

### deepin-system-monitor
- 介绍：深度桌面环境的系统监视器
- 源码地址：https://github.com/linuxdeepin/deepin-system-monitor

### deepin-graphics-driver-manager
- 介绍：深度桌面环境的显卡驱动管理器
- 源码地址：https://github.com/linuxdeepin/deepin-graphics-driver-manager

### deepin-calculator
- 介绍：深度桌面环境的计算器
- 源码地址：https://github.com/linuxdeepin/deepin-calculator

### dde-wloutput-daemon
- 介绍：DDE KWayland 桌面环境下用来进行显示设置的守护程序
- 源码地址：https://github.com/linuxdeepin/dde-wloutput-daemon

### dde-wallpapers
- 介绍：Debian系统上深度桌面环境壁纸
- 源码地址：https://github.com/linuxdeepin/dde-wloutput-daemon

### deepin-turbo
- 介绍：支持更快地启动基于dtk的应用程序
- 源码地址：https://github.com/linuxdeepin/deepin-turbo

### dtksystemsettings
- 介绍：基于Qt风格的DDE系统设置开发库
- 源码地址：https://github.com/linuxdeepin/dtksystemsettings

### deepin-anything
- 介绍：闪电般速度的文件名搜索功能
- 源码地址：https://github.com/linuxdeepin/deepin-anything

### dde-clipboard
- 介绍：一个设计精美、简单的剪贴板管理组件
- 源码地址：https://github.com/linuxdeepin/dde-clipboard

### dde-display
- 介绍：深度桌面环境的显示管理模块，提供显示服务
- 源码地址：https://github.com/linuxdeepin/dde-display

### deepin-plymouth
- 介绍：由 deepin 设计的 plymouth 主题
- 源码地址：https://github.com/linuxdeepin/dtknotifications

### dtknotifications
- 介绍：基于Qt风格的DDE通知开发库
- 源码地址：https://github.com/linuxdeepin/dtknotifications

### dde-wayland-config
- 介绍：深度桌面环境的wayland配置
- 源码地址：https://github.com/linuxdeepin/dde-wayland-config

### go-x11-client
- 介绍：go-x11-client是一个用go语言实现与X11协议进行绑定的项目
- 源码地址：https://github.com/linuxdeepin/go-x11-client

### disomaster
- 介绍：DISOMaster提供基本的光盘驱动器操作和磁盘上的文件系统操作
- 源码地址：https://github.com/linuxdeepin/disomaster

### deepin-picker
- 介绍：简单的取色器工具
- 源码地址：https://github.com/linuxdeepin/deepin-picker


### dde-introduction
- 介绍：深度桌面环境的介绍应用
- 源码地址：https://github.com/linuxdeepin/dde-introduction

### dde-manual-content
- 介绍：深度桌面环境帮助手册
- 源码地址：https://github.com/linuxdeepin/dde-introduction

### default-settings
- 介绍：深度桌面环境的默认设置
- 源码地址：https://github.com/linuxdeepin/dde-introduction

### dde-printer
- 介绍：深度桌面环境的打印机管理工具
- 源码地址：https://github.com/linuxdeepin/dde-printer

### deepin-clone
- 介绍：深度桌面环境的备份/还原磁盘数据的工具
- 源码地址：https://github.com/linuxdeepin/deepin-clone

### deepin-voice-note
- 介绍：深度桌面环境的备忘录软件，带有文本和语音记录，可以将录音保存为MP3格式或文本
- 源码地址：https://github.com/linuxdeepin/deepin-voice-note

### dde-device-formatter
- 介绍：用于在块设备中创建文件系统的简单图形界面。 最初是dde文件管理器的子项目
- 源码地址：https://github.com/linuxdeepin/dde-device-formatter

### deepin-downloader
- 介绍：深度桌面环境下载管理器
- 源码地址：https://github.com/linuxdeepin/deepin-downloader

### dtkdevice
- 介绍：未知
- 源码地址：https://github.com/linuxdeepin/dtkdevice

### dde-polkit-agent
- 介绍：DDE Polkit Agent 是 polkit 在深度桌面环境上的代理
- 源码地址：https://github.com/linuxdeepin/dde-polkit-agent

### deepin-ocr
- 介绍：DDE 基础文字识别功能
- 源码地址：https://github.com/linuxdeepin/deepin-ocr

### deepin-ocr
- 介绍：DDE 基础文字识别功能
- 源码地址：https://github.com/linuxdeepin/deepin-ocr

### deepin-boot-maker
- 介绍：启动盘制作工具是一款帮助用户快速轻松创建可启动u盘的工具
- 源码地址：https://github.com/linuxdeepin/deepin-boot-maker

### deepin-wallpapers
- 介绍：为深度桌面环境提供壁纸
- 源码地址：https://github.com/linuxdeepin/deepin-wallpapers

### dde-app-services
- 介绍：dde-app-services提供dde-applications的服务集合，包括配置中心
- 源码地址：https://github.com/linuxdeepin/dde-app-services

### deepin-face
- 介绍：生物特征的人脸数据录入、验证与数据管理
- 源码地址：https://github.com/linuxdeepin/deepin-face

### dde-session
- 介绍：dde-session 是一个用于启动 DDE 组件 systemd 服务的项目。 该项目涉及了一部分 GNOME 的会话文档和文件
- 源码地址：https://github.com/linuxdeepin/dde-session

### deepin-sound-theme
- 介绍：深度桌面环境的桌面声音主题
- 源码地址：https://github.com/linuxdeepin/deepin-sound-theme

### dtkbluetooth
- 介绍：Bluetooth模块，使用Qt对BlueZ DBus API进行封装
- 源码地址：https://github.com/linuxdeepin/dtkbluetooth

### deepin-service-manager
- 介绍：Deepin 平台上的服务管理程序
- 源码地址：https://github.com/linuxdeepin/deepin-service-manager

### deepin-gtk-theme
- 介绍：深度桌面环境主题
- 源码地址：https://github.com/linuxdeepin/deepin-gtk-theme

### deepin-reader
- 介绍：深度桌面环境的文档查看器，支持PDF和DJVU格式
- 源码地址：https://github.com/linuxdeepin/deepin-reader

### deepin-screensaver
- 介绍：深度桌面环境的屏保应用
- 源码地址：https://github.com/linuxdeepin/deepin-screensaver

### dde-session-ui
- 介绍：深度桌面环境关机对话框页面
- 源码地址：https://github.com/linuxdeepin/dde-session-ui


