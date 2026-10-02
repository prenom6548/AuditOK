# 开机动画

当你启动 Linux 计算机时，你可以看到一个启动画面。这个启动画面可以是一个图片，也可以是一个视频，可以让你在系统准备好之前等待。在这篇文章中，我们将讨论如何在 Linux 计算机上创建启动动画。

### 安装 Plymouth

Plymouth 是一个开源的启动任务程序，它可以用来创建图形启动画面。大多数 Linux 系统都预装了 Plymouth。如果你需要安装 Plymouth，你可以在终端中运行以下命令：
```
$ sudo apt-get install plymouth
```

### 选择主题

Plymouth 提供了一些预定义的主题，可以在系统启动时显示。你可以查看这些主题并选择其中之一。

```
$ ls /usr/share/plymouth/themes
````

你可以选择任何一个主题，并将其设置为默认主题：

```
$ sudo update-alternatives --install /usr/share/plymouth/themes/default.plymouth default.plymouth /usr/share/plymouth/themes/theme-name/theme-name.plymouth 100
```

请用你希望的主题名称替换 theme-name。

### 创建 Logo

你需要设计一个启动 Logo 并将其放置在 /usr/share/plymouth/themes/theme-name/ 目录下。Logo 可以是任何 PNG 格式的图片，并保证它的名称为 logo.png。

```
$ sudo cp /path/to/your/image/logo.png /usr/share/plymouth/themes/theme-name/
```

### 创建 Animation

在 /usr/share/plymouth/themes/theme-name/ 目录下创建一个新的目录，这个目录将存放你的动画文件。

```
$ sudo mkdir /usr/share/plymouth/themes/theme-name/new-animation/`
```

你需要一个 .plymouth 后缀的动画文件以及等待时显示的图片。这些图片的文件名应该以数字开头，并以 .png 结尾，比如 0.png，1.png，2.png 等等。

```
$ sudo vim /usr/share/plymouth/themes/theme-name/new-animation/new-animation.plymouth
```

### 编辑 Animation

在打开的编辑器中，将以下内容复制并粘贴到文件中，以创建默认的动画。

```
[Plymouth Theme]
Name=New Animation
Description=New Animation
ModuleName=script

[script]
ImageDir=/usr/share/plymouth/themes/theme-name/new-animation/
ScriptFile=/usr/share/plymouth/themes/theme-name/new-animation/script.script
```

你需要在 ImageDir 中指定图片所在的目录，ScriptFile 则应该列出用于运行动画的脚本文件的完整路径。
接下来，你需要创建一个名为 script.script 的文件，也将其放在 new-animation 子目录下，并将以下内容复制并粘贴到其中：

```
#!/bin/sh
exec /bin/plymouth show-splash
for i in `seq 0 7`; do
  /bin/plymouth display-message --text="Loading system, please wait."
  /bin/sleep 0.5
done
exec /bin/plymouth quit
```
该脚本将显示一条加载消息，然后在 8 秒后退出 Plymouth。

### 更新 Plymouth

现在你需要从你的新动画文件中更新 Plymouth。你可以检查 Plymouth 的配置文件，并且在确认你已经制作好你的 Logo 和 Animation 之后，就可以切换到你的新主题。

```
$ sudo update-initramfs -u
$ sudo update-alternatives --config default.plymouth
$ sudo update-initramfs -u
```

### 重启系统

重启你的 Linux 计算机，并看到你的新动画。现在在启动时，你将可以看到一个自定义的启动画面了！


