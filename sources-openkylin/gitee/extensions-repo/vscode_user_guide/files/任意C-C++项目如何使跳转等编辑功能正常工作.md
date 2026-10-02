# 任意C/C++工程，如何使跳转等编辑功能正常工作（clangd）
- 简单来说
  * 能构建编译数据库就构建编译数据库
  * 不能构建编译数据库时，在工作区中创建`.clangd`或`compile_flags.txt`文件，添加头文件包含路径

## clangd
- 着色（高亮）、跳转、语法检查、查找引用、补全等编辑功能是如何实现的？
  * 最关键的是需要解析代码，生成抽象语法树，要跨文件解析代还要知晓代码如何构建，整个处理过程类似编译器的处理过程。称提供这种服务的程序叫语言服务器
  * LSP（Language Server Protocol，语言服务器协议）用于编辑器和语言服务器之间的通信，是微软在2016年推出的协议。采用LSP，编辑器可以不开发自己的编辑功能，复用其他语言服务器的功能。在LSP结构中，主要由客户端和服务端组成，客户端在编辑器中，服务端是语言服务器
  * 具体到C/C++，clangd是一个开源的C/C++语言服务器，它基于Clang C++编译器，Clang是LLVM的一部分
  * Kylin-IDE和Kylin-Code推荐的C/C++编辑支持插件Kylin Clangd，正是基于clangd。所以，为了更好地使用使用C/C++编辑功能，需要了解一些clangd的配置方法
  * 安装，操作系统软件源中通常提供了clangd
    - 安装命令`apt install clangd`或`yum install clang-tools-extra`
    - 软件源中有时会有多个版本的clangd，尽量安装最新版本的clangd，桌面系统可以使用`apt search clangd`查询clangd版本，通常为软件包名为`clangd-n`，n表示版本
    - 如果使用Kylin-IDE或Kylin-Code的开发环境快速部署功能安装C/C++开发选项，则会自动安装clangd，而不需要手动安装
    - 已安装后，版本查看命令：`clangd --version`

## clangd 插件配置
- 设置入口：首先要安装Kylin Clangd插件，然后点击左下角齿轮图标，选择“设置”，在搜索框中输入clangd，即可显示clangd配置
  * 设置有两类，一类是“用户”，另一类是“工作区”
    - 用户：是当前操作系统用户的配置，对用户的所有项目生效；如果想对所有项目生效，则修改用户配置
    - 工作区：是当前工程的配置，仅对当前工程生效；如果仅想在当前工程使用当前设置，则修改工作区配置
    - 若工作区配置和用户配置有冲突，如果工作区配置中的某个条目后标记了“修改于用户”，表示以用户配置为准；若无此标记，则以工作区配置为准
- 插件主要配置参数含义
  * Arguments：为clangd server添加参数。具体参数可以执行命令查看：`clangd -h`
  * Head Insertion：控制clangd是否自动插入#include
    - iwyu：在编写代码时会根据当前代码符号自动插入头文件，即便头文件通过其他头文件包含了。有时，自动插入头文件会引起困惑，如不需此功能，请选择never
    - never：不自动插入头文件
  * On Config Changed：当编译数据库或配置文件变化时，如何处理。例如当`compile_commands.json`被修改后（修改了代码文件编译方法或添加了新代码文件）。当clangd版本是12或以上时，clangd会自动重载，可以忽略此参数，不过目前银河麒麟操作系统上的clangd版本通常是10或11，故此参数仍有效
    - restart：自动重启clangd server，以使修改生效
    - prompt：提示用户是否重启，用户确认重启后再重启
    - ignore：不重启
  * Path：clangd路径。如果clangd安装在了非标准路径，可以使用此参数定义clangd程序路径
  * Trace：指定一个文件路径，clangd会将自己运行时的性能数据写入到这个文件中，便于分析clangd的性能
  * Fallback Flags：Fallback编译参数是指在没有找到编译数据库时，clangd会使用的默认解析方法。Fallback编译参数为clangd提供了一种灵活的方式，以便在缺少特定编译信息时仍能进行代码分析和补全。在快速开发或小型项目中，可能不需要复杂的构建系统，Fallback编译参数可以提供基本的支持
    - 添加格式：直接添加参数即可，例如：
      ```
      -std=c++17
      -I/usr/local/include
      ```

## 编译数据库
- clangd正常解析代码，需要编译数据库
- 需要先理解编译数据库这个概念
  * 一个常见的情况：打开一个C/C++ 源代码文件之后，为什么编辑器中报了很多警告和错误，而代码明明没有任何问题
    - 解析源代码需要一定的上下文，为了让 clangd 能够正确解析项目中的 C/C++ 文件，使其自动补全，跳转等特性生效，clangd 需要知道如何编译源代码文件
    - 即 clangd 需要知道代码文件的编译命令
  * 通常，clangd 从**编译数据库**中获取基本的编译命令。也就是说，如果存在编译数据库，并且编译数据库和代码对应，则编辑功能就可以正常使用。如果没有编译数据库或其他配置，则代码通常不能正常解析
- 编译数据库是一个文本文件，它描述了代码库的编译命令。它通常是：
  * 一个名为 `compile_commands.json` 的文件，列出了项目中每个文件的编译命令。该文件通常由像 CMake 这样的构建系统自动生成
  * clangd首先在包含源文件的目录中和此目录下的build目录下检查编译数据库，然后向上遍历父目录
- 编译数据库的生成方法
  * 方法一：构建工具生成。如果工程是使用构建工具构建的，则构建工具可以生成编译数据库
    - CMake: 使用 `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` 选项生成 `compile_commands.json` 文件, 但是该选项仅在 Generator 是 Makefile 或者 Ninja 的时候有效果。
    - Bazel: 使用 `--experimental_action_listener=//tools/clang:generate_compile_commands` 选项生成 `compile_commands.json` 文件。
  * 方法二：bear工具
    - 原理：bear工具跟踪编译过程，记录编译命令，将编译命令记录到`compile_commands.json` 文件中
    - 使用：需要在构建命令前面加上 `bear` ，命令格式如下
      ```bash
      bear -- <your-build-command>
      例子：
      make clean //需要重新构建才能执行所有构建命令
      bear -- make
      ```
    - 缺点：bear需要全程跟踪编译过程，需要编译一遍才能够生成编译数据库，对于某些编译时间很长的大型项目，耗时较多
    - 安装方法：操作系统软件源中通常提供了 bear 工具，可以直接安装。例如，桌面操作系统：
      ```bash
      sudo apt-get install bear
      ```

## 无法生成编译数据库怎么办？

如果项目不是通过 CMake/Bazel 等构建工具构建的，还可以通过下面的方法来配置 clangd，使其能够正确解析源代码，实现代码跳转等功能。

1. 在命令面板中搜索命令：`Create clangd configuration file in workspace folder`，然后按回车键
2. 如果 clangd 版本大于等于 11，则快速选择的第一个选项是 `.clangd`。如果 clangd 版本小于 11，则快速选择的第一个选项是 `compile_flags.txt`
3. 确保选中第一个选项，按下回车键。会在工作区创建文件 .clangd 或 compile_flags.txt
4. .clangd 文件是一个 yaml 文件，是 clangd 11 以及以上版本的配置文件。该文件创建后具有默认的配置和注释，并支持自动补全，根据注释在该文件中相应的位置添加头文件包含路径即可。
   ```yaml
    CompileFlags:
      Add: [
              "-I/usr/include",
              "-I/home/user/workspace/project/include",
           ]
   ```
5. compile_flags.txt 文件是一个文本文件，每行一个编译参数。如果所有代码文件使用的编译参数相同，可以把使用的编译参数一个一行放在此配置文件中。针对简单的项目，可以采用该方式。需要注意：如果`compile_commands.json`已经存在，clangd 会忽略 `compile_flags.txt` 文件。例如：
   ```txt
    -I/usr/include
    -I/home/user/workspace/project/include
    -std=c++17
   ```

### clangd 配置的自动补全
clangd语言服务器可以读取工作区中的三个配置文件，分别是：.clangd, .clang-tidy, .clang-format。.clangd 文件用于配置 clangd 语言服务器，.clang-tidy 用于控制 clang 诊断，.clang-format 文件用于控制代码格式化。这三个文件都是 yaml 格式，自动补全功能需要安装插件 `redhat.vscode-yaml`。Kylin Clangd 插件内置了这三个文件的 json schema, 配合 `redhat.vscode-yaml` 插件可以实现编辑这三个文件时的自动补全及验证功能。

## 参考链接
- https://clangd.llvm.org/