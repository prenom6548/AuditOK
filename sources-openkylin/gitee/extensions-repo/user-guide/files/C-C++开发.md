# C-C++开发
在VSCode插件市场中，最流行的C/C++开发支持插件是cpp-tools，它是微软提供的，但协议对开源并不友好。微软不允许Cursor等AI IDE使用其微软协议的C/C++开发支持插件，这些微软协议插件成为微软的竞争手段。KylinIdeTeam提供的Extension Pack for C/C++，插件组中的插件均为非微软协议，可以在Cursor等AI IDE中使用。

# 1.安装插件和依赖软件
- 安装插件：在插件市场中检索安装Extension Pack for C/C++，Publisher为KylinIdeTeam，安装此插件会将编辑、调试、CMake支持等插件一并安装
  * 具体插件列表见下文
- 安装依赖软件
  - 安装clangd（Kylin Clangd代码编辑插件依赖，x86、arm以外架构需要安装，x86、arm架构已内置到插件中）
    * 命令举例：`apt install clangd`或`yum install clang-tools-extra`
    * 建议尽量安装较新版本，功能更强。目前系统默认安装clangd-10，可以手动安装更新版本，例如`sudo apt install clangd-11`
  - 安装编译构建工具
    * 命令举例：`apt install build-essential cmake ninja-build`或`yum install gcc gcc-c++ cmake ninja-build`
  - 安装gdb（调试插件依赖）
    * 命令举例：`apt install gdb`或`yum install gdb`

# 2.C/C++项目操作
## 2.1 已有项目
- 对于已有项目，在IDE基础平台中中打开项目文件夹
  * 点击文件主菜单 -> 打开文件夹
## 2.2 新建项目
- 方法1：自行创建文件、组织文件结构；可借助AI编程插件创建
- 方法2：可以搜索安装 Kylin Project Manager 插件，使用该插件创建基础项目

# 3.编辑、构建、调试、运行配置
- 使用Extension Pack for C/C++后，对于cmake类型项目，其编辑、构建、调试、运行会自动配置，通过按钮可以直接操作
- 但对于非cmake类型的C/C++项目，需要配置才能使其编辑、构建、调试功能正常运行，需要了解基本的配置方法
  - 编辑支持需要了解Kylin Clangd插件和系统软件clangd的基本配置方法，具体见如下章节
    * [任意C/C++工程，如何使跳转等编辑功能正常工作（clangd）](./任意C-C++项目如何使跳转等编辑功能正常工作.md)
  - 项目的构建、调试、运行配置请参考以下章节
    * [如何配置工程的构建、调试、运行](./如何配置工程的构建-调试-运行.md)
  - C/C++ Debug(gdb)插件的launch.json具体如何配置，见插件readme
    * 在最左侧点击插件图标，打开插件视图，在列表中找到插件条目，点击插件条目即可查看插件readme

# 4.C/C++基础插件组列表
- [Extension Pack for C/C++](https://open-vsx.org/extension/KylinIdeTeam/kylin-cpp-pack)：插件包，安装此插件自动安装整个插件组
- [C/C++ Debug](https://open-vsx.org/extension/KylinIdeTeam/cppdebug)：调试支持插件
- [Kylin Clangd](https://open-vsx.org/extension/KylinIdeTeam/kylin-clangd)：编辑支持插件
- [CMake IntelliSense](https://open-vsx.org/extension/KylinIdeTeam/cmake-intellisence)：cmake编辑支持插件
- [Kylin CMake Workflow](https://open-vsx.org/extension/KylinIdeTeam/kylin-cmake-tools)：cmake项目自动配置插件