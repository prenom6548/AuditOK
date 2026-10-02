# 语音助手

#### 介绍
**语音助手**是基于**百度开源模型库paddlespeech**开发的一款便于用户快捷操作的应用。

#### 主要功能
进行语音识别用户的语音输入。

目前支持快速打开应用。


#### 运行

默认开机自启，放置到系统托盘

#### 使用说明

例如：打开**语音助手**，说“打开计算器”，就可以快速打开计算器

#### 语音拉起配置

在项目中新建一个**xxx-startup.config**文件，在安装应用时将config文件放入**/usr/share/kylin-asrassistant/config/**目录下。

**xxx-startup.config**文件格式如下：

```json
{

"keyword": "唤醒关键词1;唤醒关键词2",

"path": "可执行文件路径， 可加参数"

}
```

如：

需要拉起**多端协同**(kylin-connectivity)

在项目中新建**kylin-connectivity-startup.config**文件

![](data/readme_png/1.png)

**startup.config**文件添加如下json字段：

![](data/readme_png/2.png)

最后在install文件中添加如下项

```
config/kylin-connectivity-startup.config /usr/share/kylin-asrassistant/config/
```

或在CMakeLists.txt中添加

```
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/kylin-connectivity-startup.config DESTINATION  /usr/share/kylin-asrassistant/config/)
```

#### 编译

```
cd kylin-asrassistant
mkdir build
cd build
cmake ..
make
sudo make install 
```

#### 运行命令

```
./kylin-asrassistant
```

#### 调试

**麒麟语音助手**目前并采用ukui-log4qt模块的日志功能。日志默认保存在**~/.log/kylin-asrassistant.log**中

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request