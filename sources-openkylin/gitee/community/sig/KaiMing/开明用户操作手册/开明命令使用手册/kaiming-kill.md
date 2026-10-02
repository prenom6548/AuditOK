# kaiming 强制退出运行的应用

``kaiming kill`` 命令可以强制退出运行的应用。

## 用法

查看 ``kaiming kill`` 命令的帮助信息：

```bash
$ kaiming kill instance # instance 为 kaiming ps 显示的信息

```

## 强制退出运行的应用示例

运行``kaiming kill`` 命令可以强制退出运行的应用。

```bash
$ kaiming ps # 获取运行中应用信息
instance    pid    app                  runtime
4025818826  27901  top.openkylin.Clock  runtime/top.openkylin.Platform/x86_64/2.0

$ kaiming kill 4025818826
```
