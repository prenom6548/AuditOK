# 一、构建您的的第一个开明应用

## 1.1 安装开明命令包和开明编译包

```bash
$ sudo apt install kaiming kaiming-builder
```

``kaiming`` 包提供了相关开明命令，``kaiming-builder`` 提供了开明包对应的编译命令。

## 1.2 安装运行时和配套的SDK

开明包要求每个应用程序指定一个用于其基本依赖项的运行时。每个运行时都有一个匹配的 SDK（软件开发工具包），其中包含运行时中的所有内容和开发工具。构建运行时应用程序需要此 SDK。

```bash
sudo kaiming install top.openkylin.Sdk top.openkylin.Platform  # 默认编译所需最小的运行时
```

## 1.3 创建应用程序

进入你的项目根目录。

```shell
#!/bin/sh
echo "Hello world, start your first kaiming app!"
```

现在将其粘贴到一个空文件中并将其另存为`hello.sh`.

## 1.4 配置源码所需清单

每个开明包都是使用清单文件构建的，该文件提供有关应用程序的基本信息以及如何构建它的说明。要将清单添加到 world 应用程序，请将以下内容添加到空文件中：

```yaml
id: top.openkylin.Hello # 应用程序名
branch: stable # 应用分支
runtime: top.openkylin.Platform # 所需运行时
runtime-version: '0.4' # 运行时版本
sdk: top.openkylin.Sdk # 配套的SDK
command: hello.sh # 运行时执行的命令

modules: # 编译模块
  - name: hello  # 模块名称
    buildsystem: simple # 使用的构建系统。如autotools、cmake、meson、simple、qmake等
    build-commands:
      - install -D hello.sh /app/bin/hello.sh  # 执行 install 命令将hello.sh 安装到可执行目录
    sources: # 在构建期间运行的一组命令
      - type: file   # 本地源码类型
        path: hello.sh # 和type格式对齐，表示将复制到源目录的本地文件的路径
```

现在将文件保存在旁边`hello.sh`并命名为 `top.openkylin.Hello.yaml`.

在更复杂的应用程序中，清单将列出多个模块。更多清单解释请参考 [清单文件详解](./manifest.md)。

## 1.5 构建应用程序

现在应用程序有了清单，`kaiming-builder`可以用来构建它。这是通过指定清单文件和目标目录来完成的。

如果您想共享该应用程序，可以将其放入存储库中。这是通过将`--repo`参数传递给来`kaiming-builder`来完成`：

```shell
$ rm -rf build/ repo/ .kaiming-builder/ # 第二次编译时需要，删除当前的编译目录、缓存和存储库repo
$ kaiming-builder build --repo=repo top.openkylin.Hello.yaml
```

此命令将构建清单中列出的每个模块并将其安装到本地`repo`目录中。 

恭喜，您已经制作了一个应用程序！

# 二、测试您的构建应用

## 2.1 安装应用程序

现在我们准备添加刚刚创建的存储库并安装应用程序。这是通过`kaiming install`命令完成的。

要验证构建是否可以成功安装，请运行以下命令：

```shell
$ sudo kaiming install --repo=repo top.openkylin.Hello # top.openkylin.Hello 为 top.openkylin.Hello.yaml 清单文件中的应用程序名
$ kaiming run top.openkylin.Hello
```

## 2.2 运行应用程序

剩下的就是尝试运行该应用程序。这可以通过以下命令来完成：

```shell
$ kaiming run top.openkylin.Hello
```

这将运行该应用程序，以便它打印“Hello world, start your first kaiming app!”。
