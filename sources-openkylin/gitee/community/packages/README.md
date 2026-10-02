# Per-package-maintainer相关信息规范化存储

### 简介

本目录下存放的是openKylin社区中，单独的包维护者（per-package-maintainer）的相关信息。

### 信息存储原则

1. 本目录下的每一个yaml文件，即为一个软件包。

2. 单独包维护者对于相应仓库拥有管理权限，与对应SIG成员对仓库的权限相同。

3. 申请成为单独软件包维护者，需要提交PR在packages中创建或修改yaml定义文件，经过技术委员会审核后合入，对于已有SIG关注的软件包，还需要相应SIG的维护者审核通过。

### 格式规范
#### \<package-name\>.yaml 文件格式

整体以 yaml 格式为主，包含以下基本元素

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | 字符串 | 软件包名称 |
| path | 字符串 | 软件包仓库路径，与name相同时可以省略。考虑到软件包的名称不一定符合gitee仓库命名规范，为避免创建仓库失败，可以另外指定路径 |
| maintainers | 列表 | 维护者成员 |

其中 packages 列表中每一条记录代表一款软件包。maintainers 列表中每一条记录代表一位 maintainer 的个人信息，mantainers 信息包含以下元素:

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | 字符串 | 成员gitee_id号 |
| displayname | 字符串 | 成员姓名或昵称 |
| openkylinid | 字符串 | 成员的openkylinid用户名 |
| email | 字符串 | 联系邮箱 |

目前仅有 name 字段是必需的，请注意 name 字段对应的是 gitee_id 号，gitee_id 可在个人主页查询，请正确填写！

#### \<package-name\>.yaml 样例

[sample.yaml](sample.yaml)