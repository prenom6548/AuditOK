# kaiming 安装应用

``kaiming install`` 命令可以用来安装开明应用。

查看 ``kaiming install`` 命令的帮助信息：

```bash
$ kaiming install --help
用法：
  kaiming install [选项…] [位置/远程] [引用…] - 安装应用程序或运行时

帮助选项：
  -h, --help               显示帮助选项

应用程序选项：
  -y          对所有问题自动回答是
  --sideload-repo 设置离线安装时仓库导入的路径，需要绝对路径
  ```

## 在线安装示例

运行 ``kaiming install`` 命令安装开明应用。

```bash
$ sudo kaiming install org.kde.kcalc
```

``kaiming install`` 命令需要输入应用完整的 ``appid``, 如果仓库有多个应用版本，则会默认安装最新版本。

## 离线安装示例

运行 ``kaiming install`` 命令安装开明应用。

```bash
$ sudo kaiming install --sideload=/home/kylin/kylin-offline/.ostree/repo org.kde.kcalc # /home/kylin/kylin-offline/.ostree/repo 为 `kaiming export` 离线导出时的仓库路径
```

``kaiming install`` 命令需要输入应用完整的 ``appid``。