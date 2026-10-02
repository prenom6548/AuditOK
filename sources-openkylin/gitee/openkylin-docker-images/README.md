中文 | [English](README.en.md)

# openKylin官方容器镜像仓

## 介绍

这里存放着由openKylin官方提供的容器镜像Dockerfile，包含openKylin基础镜像、应用镜像。

## 基础镜像

openKylin的基础镜像由社区官方发布，目前发布在[openKylin镜像站](http://archive.build.openkylin.top/openkylin/)，其中包含`openkylin:latest` 是最新可用的长期稳定镜像。

社区官方镜像发布后，会同步推送到各个远端容器镜像仓，信息如下：

### 1.仓库及镜像名
基础镜像在第三方镜像托管平台的仓库名/镜像名为：`openkylin/openkylin`

### 2.可用镜像的Tags
* `2.0`

### 3.存放路径：
Base/openkylin/Dockerfile

## 应用镜像

基于openKylin基础镜像内置应用，发布openKylin的应用镜像。

### 1.仓库及镜像名
应用镜像在第三方镜像托管平台的仓库名/镜像名为：`openkylin/[应用名]`

### 2.存放路径
应用镜像按场景分为7类，按场景目录存放镜像构建文件：
  - 大数据:    `Bigdata/`
  - AI:       `AI/`
  - 分布式存储: `Storage/`
  - 云服务:    `Cloud/`
  - 其他:      `Others/`

#### 2.1 整体目录
由于应用镜像功能的复杂性，存放文件的路径深度具有不确定性，整体存放路径如下：
```
openkylin-docker-images/
└── AI/
    |── image-list.yml
	|── AppName_1/
	|    |── AppVersion_1/
	|	 |    └── OSVersion_1/
	|	 |    └── OSVersion_2/
	|	 └── AppVersion_1/
	|	      └── OSVersion_1/
	|	      └── OSVersion_2/
	|		  └── OSVersion_3/
	|───AppName_2/
	└───AppName_3/
```

每个场景目录下必须包含一个`image-list.yml`用以描述每个应用镜像**最小目录单元**（即，示例中的所有`Image_i/`）的起始路径，其格式如下：
```
# image-list.yml示例
images:
	Image_1: Image_1/
	Image_2: Image_2/
	Image_3: Image_3/
```
`image-list.yml`的存在：
- 有助于检查镜像目录完整性；

#### 2.2 最小目录
本仓库要求**最小目录单元**`Image_i/`严格遵循以下结构：
```
# 以Image_i == nginx为例
nginx/
	|── README.md
	|── meta.yml
	|── doc/ (可选)
	|    |── img/
	|	 |    └── logo.png
	|	 └── image-info.yml
	└── 1.28.1/
	     |── 2.0
		      └── Dockerfile
```
在每个**最小目录单元**`Image_i/`下，包含以下内容：
- Dockerfile：

    存放路径为`[应用的版本号]/[openKylin的版本号]/Dockerfile`。示例：基于openKylin `2.0`的nginx `1.28.0`应用镜像Dockerfile位于`1.28.0/2.0/Dockerfile`。

- README.md文件

	按顺序涵盖以下信息：
	- `Quick reference`：相关链接信息
	- `[应用名] | openKylin`：描述应用的功能
	- `Supported tags and respective Dockerfile links`：描述当前应用容器镜像的tags及Dockerfile链接，新增镜像时**必须更新**
	- `Usage`：描述该应用容器镜像的使用方法，尽量出给一个能够简单运行的测试用例
	- `Question and answering`：提供Issue链接
	
	README会同步发布到第三方Hub镜像详情页面的`Overview`或`Description`，贡献PR时需认真对待。

- meta.yml文件

	描述每个镜像的tag信息和Dockerfile存放路径等信息，文件路径为:`[应用名]/meta.yml`。文件格式如下所示：
	```
	# nginx/meta.yml
	1.28.0-ok20: 
		path: nginx/1.28.0/2.0/Dockerfile
	```
	上述文件中，每一对`<key, value>`描述一个镜像的构建发布规则，其中：
	- key：表示镜像的tag，建议tag命名格式为：`[应用版本号]-[openKylin版本号]`，如`1.28.0-ok20`表示`openkylin/nginx`镜像的不同tag
	- value: 用于描述如何构建镜像，说明如下
		| 配置项 | 是否必选 | 功能说明 | 示例 |
		|--|--|--|--|
		| path | 是 | 描述构建镜像的Dockerfile相对路径 | nginx/1.28.1/2.0/Dockerfile |
		| architectures | 是 | 用于发布指定架构镜像时，指定构建架构可选`amd64`或`arm64`或`riscv64` | amd64,arm64,riscv64 |

	备注： **镜像tag更新时，需要同步更新上述配置**。

- （可选）`doc/`目录

	存放该镜像的图文信息，用于在`openKylin应用商店应用镜像板块`展示该镜像。无需展示在软件中心时，可不填写本部分内容。
	- `doc/image-info.yml`，内容如下：
			名称（name）：应用名
			分类（category）：应用镜像功能分类，可选：bigdata、ai、storage、cloud、others
			功能简介（description）
			镜像标签（tags）
			获取方式（download）
			使用方式（usage）
			LICENSE（license）
	- `doc/img/`：

		存放与应用特征相关的图片，如应用的logo或典型场景的运行时截图
	
	备注：**上述`doc/`目录的文件和内容必须同时存在才可在应用商店正确显示该镜像**。

## 镜像托管平台

目前支持的第三方镜像托管平台有：
- [hub.docker.com](https://hub.docker.com/u/openkylin)
- [quay.io](https://quay.io/organization/openkylin)

以基础镜像`openkylin/openkylin:latest`为例，从指定托管平台获取镜像的方式如下：
```
# 从hub.docker.com获取镜像
docker pull openkylin/openkylin:latest

# 从quay.io获取镜像
docker pull quay.io/openkylin/openkylin:latest
```
注意：由于国内用户访问`hub.docker.com`受限，建议从`quay.io`拉取所需的镜像。

