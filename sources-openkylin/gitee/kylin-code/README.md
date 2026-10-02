# Kylin-Code
- Kylin版“VSCodium”，面向国产平台，支持多种编程语言的编辑、构建、调试，支持本地开发和远程开发的集成开发环境。未来将逐步丰富开发解决方案，致力于麒麟生态建设

## Kylin-Code与Kylin-IDE
- **Kylin-Code**：openKylin社区版本IDE，主要支持X86架构。本仓库是其组成部分中的IDE基础平台，基于VSCode开源项目Code-OSS
    * [IDE基础平台-仓库链接](https://gitee.com/openkylin/kylin-code)
    * [IDE基础平台-下载链接](https://gitee.com/openkylin/kylin-code/releases)
- **Kylin-IDE**：面向银河麒麟操作系统V10的IDE，按使用方式分为桌面版、远程开发插件版、远程开发WebIDE版
    * [简介和链接](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/files/%E7%AE%80%E4%BB%8B.md)

## 帮助文档
- [帮助文档](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/%E7%9B%AE%E5%BD%95.md)
- [汇总仓库(openKylin extensions-repo)](https://gitee.com/openkylin/extensions-repo)
- [插件手动下载地址(extensions-repo KylinIDETeam目录)](https://gitee.com/openkylin/extensions-repo/tree/master/KylinIDETeam)

## openKylin Framework SIG组月度例会
   * 欢迎参加月度例会，讨论Kylin-Code及其插件规划和发展思路、讨论IDE发展方向
   * 例会时间：每个月的第2个周二下午14:00，如遇到节假日，顺延到下一个周二
   * 例会会议号会在openKylin官网公布（[点此链接，在主页找到“社区日程”，点击日历的对应日期查看](https://www.openkylin.top/index-cn.html)）

## 支持的平台
- 见[帮助文档支持的平台章节](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/files/%E6%94%AF%E6%8C%81%E7%9A%84%E5%B9%B3%E5%8F%B0.md)

## 组成
- 见[帮助文档简介章节桌面版](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/files/%E7%AE%80%E4%BB%8B.md)
- 仅安装IDE基础平台只能完成很有限的编辑功能，需要安装插件并在操作系统上安装插件依赖才能实现更多的开发功能
- IDE基础平台提供基本图形界面、插件架构，通过插件管理器检索、安装、管理插件，通过插件依赖管理器安装插件依赖
- IDE基础平台基于Visual Studio Code(VSCode)开源项目 [Code OSS](https://github.com/microsoft/vscode) ，默认接入 [OpenVSX](https://open-vsx.org/) 插件市场

## 新功能介绍
- 见[帮助文档新功能章节](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/files/%E6%96%B0%E5%8A%9F%E8%83%BD.md)

## 使用方法
- 见[帮助文档简介章节，使用方法小节](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/files/%E7%AE%80%E4%BB%8B.md#4-)

## OpenVSX插件市场使用注意
- 见[帮助文档简介章节，OpenVSX插件市场使用注意小节](https://gitee.com/openkylin/extensions-repo/blob/master/user-guide/files/%E7%AE%80%E4%BB%8B.md#31-openvsx)

# 常见问题

- 为什么基于Code-OSS

    VSCode有多个版本，从微软网站下载的二进制版本称之为vscode；从github网站下载源码后用户自行编译生成的二进制版本称之为code-oss。Code-OSS是VSCode的开源版本，遵循MIT协议。根据stackoverflow网站统计，VSCode持续多年在集成开发环境IDE领域排名第一，成为最受欢迎的IDE。VSCode采用插件架构，插件丰富，社区活跃度高。在比较了多个IDE开源软件后，我们决定基于Code-OSS进行二次开发、定制插件，为openKylin用户开发软件提供支持。

- 已有VSCode、VSCodium，为什么不直接拿来用

    这是很多人感到疑惑的问题。如果VSCode、VSCodium能满足所有需求，可以直接使用VSCode或VSCodium；但由于以下原因，我们还是决定基于Code-OSS来做定制开发。
	* 第一，VSCode协议限制较大。从微软网站下载的VSCode二进制发行版软件并非完全开源，它所使用的部分插件也不是开源软件，例如常用的C/C++支持插件、remote-ssh插件等。
	* 第二，我们希望支持更多的国产平台，VSCode和VSCodium仅支持X86和ARM平台，LoongArch、申威等国产平台需自行支持，重新编译构建Code-OSS则不可避免。
	* 第三，面向国产操作系统，我们有增加、改造、定制功能的需要，仅依靠VSCodium无法满足需要，需要对源码进行改造。例如为常用操作增加工具栏；从OpenVSX开源插件市场中检索到我们针对国产系统适配、改造的插件等。

- 为什么不使用最新版本

    目前版本基于Code-OSS1.85.2版本进行改造，之所以不使用最新版本，主要基于以下原因
	* 第一，Code-OSS代码迭代较快，通常2周发布一个版本，紧跟Code-OSS版本不太现实。
	* 第二，我们希望在多个国产平台上保持版本基本一致，Code-OSS依赖的Electron移植适配工作量较大，特别是LoongArch等国产平台，需要基于现有Electron版本或投入大量精力移植适配某个版本。
	* 第三，为用户在openKylin等国产操作系统上开发软件提供一个稳定好用的IDE是我们所要追求的目标。

- 如何加入Framework SIG组，一起思考、推进国产IDE的发展
	1. 在相关仓库反馈issues，[仓库列表](https://gitee.com/openkylin/community/tree/master/sig/framework#repository)
	2. 参与开发：您可以直接在我们的[仓库列表](https://gitee.com/openkylin/community/tree/master/sig/framework#repository)中 fork 自己的感兴趣的项目，提交pr
	3. 成为 Maintainer：您可以直接向目前的任何一个[Maintainer](https://gitee.com/openkylin/community/tree/master/sig/framework#maintainers)提出申请，在获得超过半数的 Maintainer 的支持之后，我们正式吸纳您成为 Framework SIG 的 Maintainer
        4. 参加openKylin Framework SIG组月度例会，例会时间见上文


# 构建步骤

## 环境

1. 安装[Git](https://git-scm.com) :`sudo apt-get install git`

2. 安装指定版本[Node.JS](https://nodejs.org/zh-cn/)：

	版本要求：>=22.15.1

	获取地址：https://nodejs.org/dist/

👉 **Tip!** 如果无法下载或者没有对应架构的nodejs安装包，请使用http-server中提供的nodejs安装包，获取地址参见[启动本地的http-server](#启动本地的http-server)章节

3. C/C++ compiler tool chain

    * On Debian-based Linux:  `sudo apt-get install build-essential g++ libx11-dev libxkbfile-dev libsecret-1-dev libkrb5-dev python-is-python3`
    * On Red Hat-based Linux:  `sudo yum groupinstall "Development Tools" && sudo yum install libX11-devel.x86_64 libxkbfile-devel.x86_64 libsecret-devel krb5-devel # or .i686.`.

4. 构建deb或者rpm包需要**fakeroot**和 **rpm**: `sudo apt-get install fakeroot rpm`

5. npm 设置

	设置淘宝镜像(解决下载electron慢的问题)，注意：淘宝源 https://registry.npm.taobao.org/ 地址已废弃，请使用新的 https://registry.npmmirror.com

```sh
npm config set registry https://registry.npmmirror.com
```

	在项目根目录文件.npmrc, 在其中添加 (注意，在.npmrc中必须小写，在环境变量中是大写)

```sh
disturl=https://registry.npmmirror.com/-/binary/node
sass_binary_site=https://registry.npmmirror.com/-/binary/node-sass
registry=https://registry.npmmirror.com
electron_mirror=https://registry.npmmirror.com/-/binary/electron/
```

## 运行

1. 获取源码：

```sh
git clone https://kylinos.zz.gitea.com/IDE/kylin-ide.git
```

2. 安装依赖
```sh
cd kylin-ide
npm ci
```

3. 监听文件修改:
```sh
npm run watch
```

4. 另启动一个系统终端运行:
```sh
./scripts/code.sh
./scripts/code-cli.sh # for running CLI commands (eg --version)
```

## 构建
源码中已提供构建脚本build.sh，可以使用系统终端中直接运行，执行完成后会构建出deb或者rpm安装包
```sh
./build.sh
```

## 配置商店地址

默认在源码根目录下的 product.json 中，已经配置 open vsx 开源插件商店，具体代码段如下：

```json
"extensionsGallery": {
	"serviceUrl": "https://open-vsx.org/vscode/gallery",
	"itemUrl": "https://open-vsx.org/vscode/item"
}
```
