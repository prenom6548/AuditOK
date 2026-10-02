# openKylin Kernel

openKylin 社区是在开源、自愿、平等和协作的基础上，由基础软硬件企业、非营利性组织、社团组织、高等院校、科研机构和个人开发者共同创立的一个开源社区，致力于通过开源、开放的社区合作，构建桌面操作系统开源社区，推动Linux开源技术及其软硬件生态繁荣发展。


## 贡献补丁指南

如果您想对openKylin kernel项目贡献补丁，请参考以下指南。

- 提交[issue](https://gitee.com/openkylin/community/issues/new)描述。 在issue中尽可能的描述清楚，issue可以是bug，也可以是特性，一个issue可以对应多个补丁。
  - 该问题的触发条件、重现步骤、以及报错信息等。
  - 测试环境，如平台、CPU、操作系统等等，越详细越好。
  - 如果在其他内核上是否也有类似问题，也可以进行说明。
  - 如果有对应的脚本或者工具也请描述清楚。
- 测试环境搭建和建议，请联系邮件列表(kernel@lists.openkylin.top)

## 补丁合入指南

哪些补丁可以合入，请参考以下指南。
- 问题修复以及CVE安全漏洞补丁
- 性能优化补丁
- 功能增强补丁
- 主线的bug修复补丁、优化补丁或其他有用的特性
- 新设备适配补丁

## 开发者指南

开发者可以做哪些事情呢，请参考以下指南。
- 修复[issue](https://gitee.com/openkylin/community/issues)上面反馈的问题。
- CVE补丁提交。
- 新设备适配。
- kernel帮助文档修订。
- 如果您想承担一下更重要的开发工作，请先在邮件列表(kernel@lists.openkylin.top)上对此进行简要描述，我们评估之后，会及时与您联系。

## 测试者指南

我们也欢迎测试介入，请参考以下指南。
- 在您的设备上搭建测试环境进行测试，如果发现bug，请[在这里](https://gitee.com/openkylin/community/issues/new) 提交。
- 可以在本地搭建CI环境，对发现的问题进行提交。
- 在本地部署功能、性能、稳定性测试环境。
- 构建专项调试、测试环境。
- 在不同的设备、环境、场景上进行测试验证。
- 测试脚本、测试工具贡献请联系邮件列表

## 补丁提交指南

### 补丁提交建议

为了您提交的补丁能够尽快被合并，有以下建议：
1. 将大的更改拆分为更小的、耦合度、复杂度更低的提交，一个补丁只做一件事，便于更快的审查和迭代。
2. 尽可能的在提交时说明为什么要提交这个补丁，并在代码中酌情注释。
3. 保持代码、测试、评论、文档、日志等风格与现有风格一致。
4. 提交的代码应该是经过测试验证的。

### 代码格式规范

为了您提交的补丁与项目风格一致，且便于K2CI平台检查，有以下规范：
- 补丁标题【必填项】对整个补丁的一个概括性描述，自研补丁抬头需要以KYLIN起始，如果来自UBUNTU、GITHUB、PHYTIUM等（需要全部大写），也可以加上对应的开头，直接移植上游则无需修改他的标题。Module和submodule是用来标识修改的子系统，这样可以很清晰的标注代码的影响范围。
- Mainline【必填项】如果补丁移植来自上游的，需要标记其commit id，如果为自研补丁则直接填写KYLIN-only，例如：Mainline：KYLIN-only|commit-id
- From【必填项】如果带来补丁移植来自上游或者任何可以查找的来源的，都必须标记其来源tag点，如果没有则无需填写，默认为NA则省略。
- Severity【必填项】该项标识补丁重要程度，分为四个等级，从轻到重为：Low/Moderate/Important/Critical。
- CVE【可选项】如果补丁是CVE修复，则加上CVE编号，如果不是则无需填写，默认为NA则省略。
- 正文 【必填项】正文段落就是整个补丁的详细信息描述区域，需要详细描述补丁的作用。
  - 我们为什么需要这个补丁？
  - 他解决了什么问题？
  - 如何重现这个错误或者如何测试？
- K2CI-Arch【推荐项】不填默认为All，该项用来标识对该补丁需要做哪些架构的编译工作，可选值：None/{Arm64|Amd64|Mips64|Loongarch|Sunway64}/All
- Signed-off-by 【必填项】本签名表示本补丁的传递过程，第一个Signed者一般表示为作者本人，后面跟随者一般为各分支Maintainer传递过程。
- Reported-by 【推荐项】对于测试人员发现的问题，也请在提交时注明测试人员。例如：Reported-by: zhangsan

### 代码提交示例

> KYLIN: changelog: update changelog  
> Mainline: KYLIN-only  
> Severity: Low  
> fix format and version info bug in changelog file  
> Change-Id: Ie9fbce3bbdc15757c9270f6837cbcfd4ba68bb7c  
> Signed-off-by: chen zhang <chenzhang@kylinos.cn>   

## 邮件列表

如果有其他疑问或者建议，请联系。
邮件列表(kernel@lists.openkylin.top)
感谢您对openKylin Kernel项目的支持。

## SIG成员

### Owner
- 张铎

### Maintainers
- Chen Zhang
- 刘云
- Leoliu-oc
- ChangXing Li #新增
- 黄伟（daway）
- 朱兵
- Qunsheng Li
- ChengKai Jiang(zzuqax)
- Liyuan Xue

## SIG维护包列表
- linux
- linux-firmware
- ketones
