# ukui-panel

##  简介
ukui-panel是ukui桌面环境中的"任务栏"组件，它是一个在屏幕边缘的dock窗口，
提供了的功能包括应用程序启动器，系统托盘，任务管理，系统时间等。

ukui-panel基于ukui-quick-framework提供的插件机制开发。

## 项目结构

### **panel**

#### 任务栏（经典模式）
  * org.ukui.panel
  * 屏幕边缘的dock窗口
  * 可置于屏幕4个边缘
  * 可拖动改变位置
#### 任务栏（三岛模式）
  * org.ukui.panelNext
  * 屏幕下边缘的dock窗口
  * 不可改变位置
  * 分为数据岛，应用岛，配置岛三部分
  * 不会占用屏幕可用区域
  * 智能隐藏
#### 顶栏
  * org.ukui.panelTopBar
  * 屏幕上边缘的dock窗口
  * 不可改变位置
  * 可以将三岛任务栏上的数据岛或配置岛放到顶栏，当没有放置任何岛时，顶栏隐藏
  * 会占用屏幕可用区域

### **widgets**
  * org.ukui.panel.starter(开始菜单按钮插件)
  * org.ukui.panel.calendar(日历插件)
  * org.ukui.panel.search(搜索按钮插件)
  * org.ukui.panel.separator(分割线插件)
  * org.ukui.panel.showDesktop(显示桌面按钮插件)
  * org.ukui.panel.taskView(多任务视图插件)
  * org.ukui.taskManager(任务管理器插件)
