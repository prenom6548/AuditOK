# LXQt 兴趣小组（SIG）

LXQt(LXQt Desktop Environment) SIG小组致力于LXQt桌面环境相关软件包的规划、维护和升级工作。
## 工作目标

- 现阶段只是代码搬运工，以后将会：
- 负责 LXQt相关软件包的规划、维护和升级
- 及时响应用户反馈，解决相关问题

## 维护包列表
- LXQt is a lightweight Qt desktop environment.

  It will not get in your way. It will not hang or slow down your system. It is focused on being a classic desktop with a modern look and feel.

  - Modular components
  - Powerful file manager
  - Customize appearance everywhere
  - Panel(s) with many plugins and settings
  - Window Manager agnostic

  ### Applications

  - PcManFm-qt - File Manager
  - Lximage-qt - Image Viewer
  - QTerminal - Terminal Emulator
  - Qps - Process Viewer
  - Screengrab - Screen Capturer
  - LXQt-archiver - Archive Manager
  - LXQt-runner - Application launcher and calculator

   

  LXQt is available for all major Linux and BSD distributions so you may just try it out on your system or in a VM.
  More information about installing can be found in the [LXQt GitHub Wiki](https://github.com/lxqt/lxqt/wiki/).

  Historically, LXQt was the product of the merge between LXDE-Qt, an initial Qt flavour of LXDE, and Razor-qt, a project aiming to develop a Qt based desktop environment.
  More on this topic can be found in the [Wiki](https://github.com/lxqt/lxqt/wiki/History).

  ### LXQt介绍

  LXQt是一个轻量级的桌面环境，它是由LXDE和Razor-qt两个项目合并而成的。LXQt使用Qt作为图形界面库，支持多种窗口管理器，如Openbox、Xfwm4或KWin等。LXQt的目标是提供一个快速、美观、易用的桌面环境，同时保持低资源占用和高可定制性。

  LXQt由许多模块化的组件构成，一些组件依赖于Qt和KDE Frameworks 5。这些组件包括：

  - QTerminal：一个终端模拟器，可以使用命令行。
  - Falkon：一个基于QtWebEngine的网络浏览器。
  - SDDM：一个使用QML编写的简单桌面显示管理器，为LXQt定制。
  - LXImage-Qt：一个图像查看器。
  - lxmenu-data：一些用于freedesktop.org桌面菜单的文件。
  - lxqt-about：一个显示关于信息的对话框。
  - lxqt-admin：一个系统管理工具。
  - lxqt-archiver：一个文件压缩和解压缩工具。
  - lxqt-common：一些通用的文件（图形文件、主题、桌面入口文件等）。
  - lxqt-config：一个系统设置中心，可以调整屏幕分辨率、主题、鼠标、键盘等设置。
  - lxqt-globalkeys：一个全局快捷键注册的守护进程和库。
  - lxqt-notificationd：一个通知守护进程，可以显示各种应用程序的通知信息。
  - lxqt-openssh-askpass：一个用于openssh密码提示的工具。
  - lxqt-panel：一个桌面面板（任务栏），可以显示应用程序菜单、系统托盘、时钟等小部件。
  - lxqt-policykit：一个Polkit认证代理，可以控制用户对系统资源的访问权限。
  - lxqt-powermanagement：一个电源管理守护进程，可以监控电池状态、调节屏幕亮度、设置睡眠和关机等操作。
  - lxqt-qtplugin：一个Qt平台集成插件，可以让所有基于Qt的程序采用LXQt的设置。
  - lxqt-runner：一个应用程序启动器，可以快速地运行任何命令或程序。
  - lxqt-session：一个会话管理器，可以启动和关闭LXQt桌面环境。
  - lxqt-sudo：一个GUI前端，可以使用sudo或su命令以其他用户身份运行程序。
  - menu-cache：一个加速菜单生成的库。
  - ObConf-Qt：一个基于Qt的Openbox配置工具。
  - pavucontrol-qt：一个基于Qt的PulseAudio音量控制器。
  - compton-conf：一个基于Qt的Compton复合管理器配置工具。

  如果你想了解更多关于LXQt的信息，你可以访问它的[官方网站](^1^)或[维基百科页面](^2^)。

  (1) LXQt - The Lightweight Qt Desktop Environment. https://lxqt-project.org/.
  (2) LXQt - Wikipedia. https://en.wikipedia.org/wiki/LXQt.
  (3) LXQt - The Lightweight Qt Desktop Environment. https://lxqt-project.org/.
  (4) LXQt - Wikipedia. https://en.wikipedia.org/wiki/LXQt.
  (5) LXQt - ArchWiki. https://wiki.archlinux.org/title/LXQt.

## SIG成员
### Owner
- XXTXTOP（[XXTXTOP (XXTXTOP) - Gitee.com](https://gitee.com/XXTXTOP) ）

### Maintainers
- XXTXTOP
- DSOE1024 
- rtlhq 

### Committers
- XXTXTOP

## 邮件列表
lxqt@lists.openkylin.top
