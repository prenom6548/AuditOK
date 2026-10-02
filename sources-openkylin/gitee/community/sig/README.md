# SIG相关信息规范化存储

### 简介

本目录下存放的是openKylin社区中，所有的 SIG 组的相关信息。 

### 信息存储原则

1. 本目录下的每一个子目录，即为一个SIG组。每个 SIG 目录下都要求包含一个用于简单介绍对应 SIG 组的章程 README.md 文件,都要求要包含一个用于存储 SIG 组的负责人信息、开发者信息等信息的 sig.yaml 文件。

2. 信息的准确性，sig.yaml > 其他。 如果信息出现不一致，以 sig.yaml 中信息为标准

3. 原则上 sig.yaml 的更新由 SIG 主负责人来提交 PR ，经过技术委员会审核后合入。

### 格式规范
#### sig.yaml 文件格式

sig.yaml文件整体以 yaml 格式为主，包含以下基本元素：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | 字符串 | SIG组名称 |
| description | 字符串 | SIG组描述 |
| owner | 列表 | SIG组负责人 |
| maintainers | 列表 | SIG组成员 |
| packages | 列表 | SIG组维护包列表 |

#### maintainers列表格式说明
maintainers 列表中每一条记录代表一位 maintainer 的个人信息，maintainer 信息包含以下元素:

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | 字符串 | 成员姓名（必填） |
| displayname | 字符串 | 成员姓名或昵称（选填） |
| openkylinid | 字符串 | 成员的openkylinid用户名（选填） |
| email | 字符串 | 联系邮箱（选填） |

目前仅有 name 字段是必需的，`请注意 name 字段对应的是 gitee_id 号，请正确填写！`
附`gitee_id`查看方式：
![gitee_id查看方式](../assets/images/giteeID%E6%9F%A5%E7%9C%8B(1).png)

#### packages列表格式说明
packages 列表中按照所属权限不同划分为公共包和单独包。公共包为packages列表下的每一条记录，比如：
```
packages:
- ukui-session-manager
- kylin-miraclecast

```

而当sig管理的包仓库较多时，可以通过单独包将仓库权限分级管理。单独包通过packages的二级列表信息来配置，包含以下元素：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| package | 列表 | 单独包列表 |
| maintainer | 列表 | 维护单独包的成员 |

例如：
```

packages:
- ukui-session-manager	//公共包
- kylin-miraclecast 	//公共包
- package:
  - peony	//单独包
  - ukui-default-settings	//单独包
  maintainer: 
  - name: larue		//负责维护单独包的成员（package maintainer或简写为pm）
  
```
` 请注意sig maintainer对公共包仓库和单独包仓库都有管理员权限；package maintainer只对所维护的包仓库有管理员权限。`
#### sig.yaml 样例

```
name: UKUI
description: UKUI team
owner:
- wx-kylin
maintainers:
- name: handsome_feng
- name: zhaikangning
- name: MouseZhang
packages:
- ukui-session-manager
- kylin-miraclecast
- package:
  - peony
  - ukui-default-settings
  maintainer: 
  - name: larue
- package:
  - ukui-menu
  maintainer:
  - name: lixueman0110
  
```