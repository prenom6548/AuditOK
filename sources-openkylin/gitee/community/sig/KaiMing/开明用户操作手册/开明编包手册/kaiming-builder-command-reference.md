# 构建命令说明

``kaiming-builder build`` 命令用来构建开明应用。它可自动构建应用程序及其依赖项。这是您可以用来构建应用程序的选项之一。

查看 ``kaiming-builder build`` 命令的帮助信息：

```bash
$ kaiming-builder --help 

用法
 	kaiming-builder [OPTION…] DIRECTORY MANIFEST - 通过清单文件编译开明包

选项:

帮助选项:
  -h [ --help ]                     显示帮助选项并退出。

应用选项:
  -v [ --verbose ]                  在命令处理期间打印调试信息。
  --version                         打印版本信息并退出。
  --arch arg                        指定要构建的机器架构。如果未指定架构，将自动检测主机架构。只能指定主机兼容的体系结构。
  --default-branch arg              设置默认分支。
  --ccache                          使用ccache
  --build-only                      不要执行清理和完成阶段，如果您想在应用程序中构建更多内容，这会很有用。
  --finish-only                     只执行清理、完成和导出阶段，​​从 --build-only 命令停止的地方继续。
  --export-only                     仅执行导出阶段，​​从先前的构建中获取构建结果。
  --repo arg                        构建完成后，运行**kaiming build-export**将结果导出到存储库 DIR 。如果DIR存在，那么它一定是一个OSTree存储库；否则将创建一个新的。
  -s [ --subject ] arg              提交消息的一行主题。导出构建结果时使用。
  -b [ --body ] arg                 提交消息的完整描述。导出构建结果时使用。
  --force-clean                     如果 DIRECTORY 不为空，则删除之前的内容。
  --sandbox                         禁用指定传递给 kaiming 构建的构建参数的可能性。这意味着构建过程无法脱离其沙箱，并且在构建不太受信任的软件时非常有用。
  --stop-at arg                     停止在指定的模块上，在“下载”和“构建”阶段忽略它以及所有后续模块。这对于调试和开发很有用。例如，您可以构建所有依赖项，但在主应用程序处停止，以便您可以从预先存在的签出进行构建。意味着仅构建。

  --jobs arg                        限制构建期间并行作业的数量。默认值是计算机上的 CPU 数量。
  
位置参数:
  DIRECTORY                         构建目录
  MANIFEST                          构建清单文件，支持 .yml/.yaml/.json 等格式

```

```kaiming-builder build``命令必须运行在工程的根目录，即 kaiming.yaml文件所在位置。

