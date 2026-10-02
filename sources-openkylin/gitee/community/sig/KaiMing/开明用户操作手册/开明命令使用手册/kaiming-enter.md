# kaiming 进入运行沙箱内部环境

``kaiming enter`` 命令可以进入运行沙箱内部环境。

## 用法

查看 ``kaiming enter`` 命令的帮助信息：

```bash
$ kaiming enter instance/pid 命令  # instance/pid 为 kaiming ps 显示的运行中的应用信息，沙箱中可用linux命令: bash, ls ...

```

## 离线导出仓库示例

运行``kaiming enter`` 命令可以进入运行沙箱内部环境。

```bash
$ kaiming ps # 获取运行中应用信息
instance    pid    app                  runtime
4025818826  27901  top.openkylin.Clock  runtime/top.openkylin.Platform/x86_64/2.0

$ kaiming enter 4025818826 ls # 使用 ls 查看信息
公共的   音乐  模板    桌面  视频  图片   文档  下载

$ kaiming enter 27901 ls # 使用 ls 查看信息
公共的   音乐  模板    桌面  视频  图片   文档  下载

$ kaiming enter 27901 bash # bash 为进入沙箱bash环境

```

