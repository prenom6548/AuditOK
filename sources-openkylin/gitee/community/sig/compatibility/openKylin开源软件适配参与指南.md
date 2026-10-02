# openKylin开源软件适配参与指南

openKylin社区作为一个开放、协作、创新的平台，关注openKylin与各类应用软件兼容性相关能力的探索，欢迎业界广大开发者积极参与开源软件与openKylin的适配，并提交贡献，共同构筑社区生态体系。

## 参与流程

- 注册Gitee账号
- [签署CLA](https://cla.openkylin.top/cla/default/index)
- [下载并安装openKylin镜像](https://gitee.com/openkylin/docs/blob/master/%E5%BC%80%E5%A7%8B%E8%B4%A1%E7%8C%AE/openKylin%E4%B8%8B%E8%BD%BD%E5%9C%B0%E5%9D%80%E5%92%8C%E5%AE%89%E8%A3%85%E6%96%B9%E6%B3%95.md)
- 完成与openKylin的适配测试
- 提交贡献

- 社区详细参与指南请参考[ :point_right: ](https://gitee.com/openkylin/docs/blob/master/%E5%BC%80%E5%A7%8B%E8%B4%A1%E7%8C%AE/openKylin%E4%B8%AA%E4%BA%BA%E5%BC%80%E5%8F%91%E8%80%85%E5%8F%82%E4%B8%8E%E6%8C%87%E5%8D%97.md#4-%E7%94%B3%E8%AF%B7%E5%8D%95%E5%8C%85%E7%BB%B4%E6%8A%A4%E8%80%85)

## 适配测试

### 适配测试指导

- 下载并安装openKylin镜像
- 待适配软件打包——[openKylin打包实践操作指南](https://gitee.com/openkylin/community/blob/master/sig/packaging/%E6%89%93%E5%8C%85%E5%AE%9E%E8%B7%B5%E6%93%8D%E4%BD%9C%E6%8C%87%E5%8D%97.md)
- 执行适配测试用例——测试用例需包含安装、启动、停止、卸载及主要功能验证；
- 输出适配测试报告

### 适配测试报告

- 适配完成后请新建名称为“openKylin适配测试报告.md”的文档，存放于应用软件首层目录，与软件一并提交，

测试报告内容模板如下，务必保证信息完整性：

```
【产品名称】
【产品版本】
【软件分类】
【功能描述】
【环境信息】
    硬件信息：
    1）CPU
    2）内存
    3）硬盘容量
    软件信息：
    1）操作系统版本及分支
    2）内核信息
    3）数据库（可选）
    4）中间件（可选）
【测试内容】
    1）安装
    2）安装后开始菜单及桌面图标显示正常
    3）启动/停止
    4）基本功能（不少于5条）
    5）卸载
【测试结果】
    根据测试内容描述测试结果并提交截图
【备注】
    1）测试过程中遇到的问题以及解决方法
    2）建议 
```


## 提交贡献

### 申请包库

- 通过修改gitee中的openkylin/community仓库实现新增软件包的具体操作步骤如下：
1. 点击community仓库，找到sig，进入兼容性sig组：compatibility，将其fork到自己的仓库。
2. 将fork完成的community仓库clone到本地。
3. 修改本地仓库的`community/sig/compatibility/sig.yaml`文件，在`packages`列表添加新建仓库的名称。
    
    例如新建仓库名称为firefox
```
description: Compatibility team
owner:
- ppf1130
maintainers:
- name: ppf1130
packages:
- opensource-kylin
- firefox
```
4. 将修改完成的内容保存并推送到个人的gitee仓库中。

### 申请单包维护者

由于社区的SIG成员权限过⾼，在openKylin社区中如果想要加⼊SIG，需要在社区有⼀定的贡献之后，技术委员会才可能会通过您的SIG加⼊申请，因此对于⼀般的开发者来说，建议最开始申请为单包维护者。

- 申请成为新的单包维护者

    在`community/sig/compatibility/`路径下新增`firefox.yaml`⽂件，并按照实际情况输⼊如下内容

```
name: firefox
path: firefox
maintainers:
- name: zhangsan # gitee id
  openkylinid:
  displayname: 张三
  email: zhangsan@gmail.com
```

- 申请成为已有的单包维护者

    假设您想成为已有包firefox的包维护者，在`community/sig/compatibility/firefox.yaml`⽂件中新增如下内容即可

```
name: firefox
path: firefox
maintainers:
- name: zhangsan # gitee id
  openkylinid:
  displayname: 张三
  email: zhangsan@gmail.com
# 新增您的个人信息
- name: lisi # gitee id
  openkylinid:
  displayname: 李四
  email: lisi@gmail.com
```

### 提交PR

- 将所有修改内容推送成功后，在个人gitee上提交`Pull Requests`，注意目标仓库为`openkylin/community`，等待sig组管理员进行审核。
    
- 审核通过后，`openkylin/firefox`仓库会自动生成，就可以上传我们适配好的代码啦！
