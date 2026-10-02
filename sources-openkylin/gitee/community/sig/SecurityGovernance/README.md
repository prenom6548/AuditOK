# 安全治理兴趣小组（Sig）
openKylin安全治理兴趣小组（Security Governance, SIG) 通过接收和响应openKylin社区的产品安全问题报告、提供社区安全指导，开展安全治理等活动提升社区产品的安全性。  
SIG的使命：为openKylin用户提供最安全的产品和安全性保障。

## 工作目标

我们的工作目标主要有以下五个重点目标和一个长期目标：  
* **安全漏洞情报维护**  
  为openkylin社区维护安全漏洞数据信息。  
* **协助社区进行安全漏洞修复**  
  通过为各SIG组提供安全漏洞情报预警，为各软件包的安全更新提供支持，帮助openKylin用户系统及时修复安全漏洞，确保openKylin社区及时修复已知漏洞。  
* **安全技术研究**  
  追踪并研究国内外前沿安全技术。  
* **社区的安全漏洞挖掘/验证**  
  通过挖掘/接受openKylin相关的安全漏洞,并对其进行验证、分析，评估漏洞的危害与价值，协助开发者及时进行处置。  
* **制定社区的安全研发流程**  
  通过吸收、消化可信研发流程，结合社区的研发体系和流程，制定社区的可控、开源研发流程。

此外，我们的工作还包括一个持续目标：**响应社区的安全问题**  
用于总结、闭环社区的安全问题，从而根治社区的安全问题，通过响应社区的安全问题，跟踪安全问题的处理进展，总结分析问题根因，进而治理安全问题。

> 1. 工作目标的变更与修订需要征求2/3的 [SIG-maintainers](https://gitee.com/openkylin/securitygovernance-management#sig-maintainers) 成员的支持并获取 [SIG-owner](hhttps://gitee.com/openkylin/securitygovernance-management#sig-owner) 的同意  
> 2. 每一个工作目标和职责，原则上都有一个 maintainer 负责主持相关的工作，掌控并规划技术发展方向  
> 3. 以上活动，需要通过 [SIG技术例会]() 进行公开讨论并表决（技术例会可以通过微信搜索小程序openKylin，进入后便可召开）  

## 加入我们

如果您对我们（Security Governance, SIG) 的工作目标感兴趣，并愿意承担对应的职责，社区会根据您的贡献给予 [相应的奖励](https://mp.weixin.qq.com/s/mvGasUtbgvOVaBCmv9hIxw)  ，根据您的兴趣和能力，我们会给予相应的帮助：
* **成为贡献者之一**  
  您可以直接向 [SIG-maintainers](https://gitee.com/openkylin/securitygovernance-management#sig-maintainers)任何一个成员提出申请，再获取该maintainer的引荐之后便可以成为贡献者之一并进入 [贡献者名单]()  
* **成为maintainer**  
  您可以直接向 [SIG-maintainers](https://gitee.com/openkylin/securitygovernance-management#sig-maintainers)任何一个成员提出申请，在获得半数maintainers成员公开投票支持之后我们正式吸纳您为进入  [SIG-maintainers](https://gitee.com/openkylin/securitygovernance-management#sig-maintainers)  
  > 1. SIG-maintainers 成员对所负责的方向应具备以下能力：  
  >    * 规划现有的技术路线  
  >    * 并主导未来技术的发展方向  
  > 2. SIG-maintainers成员，应主动承担以下义务和职责：
  >    * 审核所负责技术方向的成果  
  >    * 定期召开技术例会规划、讨论所负责的技术发展方向  
  > 3. 我们会每半年一次进行SIG-maintainers的资格审查，以确保安全治理SIG（Security Governance, SIG) 的正常运转，maintainer能力审核的维度不限于以下：  
  >    * 该技术方向的必要性审查  
  >    * maintainer技术规划能力审查  
  >    * maintainer技术方向的发展进度审查  
  >    * maintainer负责方向的现有技术成果质量审查  
  > 4. 增选maintainer
  >    * 如果您想成安全治理SIG（Security Governance, SIG)的一名maintainer，需要选定一个负责的技术方向，通过技术例会的公开表决，满足成为maintainer的条件后即可  
  > 5. 替换maintainer  
  >    * 当由于各种原因，无力主持相关的工作，可以主动提出替换申请并提交候补人选
## 工作内容  
为了实现我们的 [工作目标](https://gitee.com/openkylin/securitygovernance-management#%E5%B7%A5%E4%BD%9C%E7%9B%AE%E6%A0%87)，我们将五个重点工作目标对应成五个技术方向进行管理，每一个技术方向都有一个技术主导人来主持相关的工作，如果您对我们工作内容感兴趣，可联系相关的技术主导人或直接参与相关项目。  

####  :tw-1f680: 安全漏洞情报维护  
*  技术方向: 为有效维护openKylin安全漏洞情报搭建平台与开发工具。  
*  技术负责人: @luming2(luming@kylinos.cn) 
   
     > :fa-tags: **[openKylin漏洞感知大脑](https://gitee.com/openkylin/openkylin-vulnerabilities-percept-brain)**  \
     > 简介：openKylin vulnerabilities percept brain项目由openKylin Security Governance SIG设立，通过社区力量拓展开源漏洞自动化感知能力。   
     > 创建时间:  2022.10.25   
     > maintainer: @luming2(luming@kylinos.cn) 

     > :fa-tags: **[CVE manage BOT]()**  
     > 简介：CVE mange BOT项目由openKylin Security Governance SIG设立，此项目致力于漏洞信息在社区的自动化流转处理。  
     > 创建时间:  2022.10.25  
     > maintainer: @luming2(luming@kylinos.cn)   

     >  :fa-tags: **[openKylin情报共享](https://gitee.com/openkylin/openkylin-cve-tracer)**  
     >  简介：openKylin情报共享项目由openKylin Security Governance SIG设立，此项目致力于将格式化的openKylin漏洞情报信息与安全性相关的关键信息快速同步给有需要的组织与个人，建设openKylin受众自动化快速获取openKylin漏洞信息的能力。  
     >  创建时间:  2022.11.8  
     >  maintainer: @advander(gaotong@kylinos.cn)  
    
#### :tw-1f680: 协助安全漏洞修复 
*  技术方向: 定制社区产品的安全漏洞维护政策、安全漏洞准出标准、追踪漏洞的修复情况、协调社区的资源完成漏洞修复情况，由于安全漏洞的敏感性，部分仓库会限定范围内开放。    
*  技术负责人: @luo-yujia87(luoyujia@kylinos.cn) 
   
     > :fa-tags: **[openKylin产品安全政策&安全漏洞准出标准](https://gitee.com/openkylin/Product-security-poliy)**  
     > 简介：介绍openKylin社区的产品安全政策及安全漏洞准出标准，相关内容待社区确定后会正式对外开放  
     > 创建时间： 2022.11.21  
     > maintainer: @zhang-zixue(zhangzixue@kylinos.cn)
    
#### :tw-1f680: 安全技术研究  
  *  技术方向: 追踪国内外前沿安全技术，致力于网络安全、操作系统安全、攻防实战技术研究，从攻击者角度提升openKylin社区产品的安全性。
  *  技术负责人: @cn-lwj(liwenjie@kylinos.cn) 

     >  :fa-tags: **[Fuzzing](https://gitee.com/openkylin/fuzzing)**  
     >  简介：Fuzzing（模糊测试）是最热门的技术之一，也是一个经久不衰的研究热点。本项目托管与模糊测试相关的文档、教程、示例和其他资源。  
     >  创建时间: 2022.10.25  
     >  maintainer: @cn-lwj(liwenjie@kylinos.cn)  

     >  :fa-tags: ⭐️ **[Attack-Defense-ThinkTank](https://gitee.com/openkylin/attack-defense-think-tank)**   
     >  简介：「openKylin攻防智库」基于OS分享渗透攻防技术文章，从攻击者角度发现未知安全威胁。诚邀广大安全从业技术人员加入，与开源安全互联网共同成长。      
     >  创建时间: 2022.10.30  
     >  maintainer: @Set3r.Pan(panzhenhua@kylinos.cn)  

#### :tw-1f680: 安全漏洞挖掘/验证  
  *  技术方向: 建立openKylin社区开放漏洞库列表，并基于漏洞模型检测的安全漏洞挖掘理论，将安全技术研究转化，实现开发openKylin社区产品安全漏洞的自动化扫描框架。  
  *  技术负责人: @Set3r.Pan(panzhenhua@kylinos.cn)  
 
     >  :fa-tags: **[openkylin-exploit-db](https://gitee.com/openkylin/openkylin-exploit-db)**  
     > 简介：仓库收录openKylin发行版有效漏洞验证程序列表，同时我们也收集其它发行版的舆情漏洞验证程序列表。由于安全漏洞的敏感性，目前只收录公开漏洞。  
     >  创建时间: 2022.10.18  
     >  maintainer: @lastre3et(yangjipeng@kylinos.cn)  

     >  :fa-tags: **[Chthonian](https://gitee.com/openkylin/chthonian)**  
     >  简介：项目用于检测openKylin漏洞信息，主要分为内部漏洞和CVE漏洞检测，帮助openKylin用户自动化检测漏洞威胁。  
     >  创建时间:  2022.10.18  
     >  maintainer: @song-bang-cheng-jin(songbangchengjin@kylinos.cn)  

     >  :fa-tags: ⭐️ **[GenMai](https://gitee.com/openkylin/genmai)**   
     >  简介：”诊脉”漏洞扫描框架（genmai），力图实现包含主机漏洞扫描和网络漏洞扫描功能的安全扫描框架。  
     >  创建时间:  2022.10.24  
     >  maintainer: @a-alpha(chenxinquan@kylinos.cn)、@song-bang-cheng-jin(songbangchengjin@kylinos.cn)  

#### :tw-1f680: 社区的安全研发流程  
  *  技术方向: openKylin SDL安全开发流程，是openKylin社区根据软件行业通用安全开发流程，结合社区化开发的特点，形成的一套社区安全开发体系。整个安全开发体系可分为四个阶段：安全设计阶段、安全开发阶段、版本安全交付阶段、版本安全维护阶段。由于社区安全开发体系的建设是一项目长期、复杂、庞大的系统工程，不论是参与安全开发体系建设，还是对安全开发流程建议，还是作为开发者参与安全开发体系的培训、讲座，积极遵循安全开发体系流程进行社区项目开发，所有的活动，都需要每一个社区开发者参与进来。为此我们创建了以下安全开发体系的社区项目。  
  * 技术负责人: @kylin-xujian(xujian@kylinos.cn)  

     >  :fa-tags: **[openKylin SDL安全开发流程](https://gitee.com/openkylin/security-development-lifecycle)**  
     >  简介：该项目为社区安全开发体系的总项目，负责openKylin SDL安全开发流程的总体规划，对子项目的工作内容做出规划和指导，定期跟踪子项目的工作进度，协调子项目的资源。  
     >  创建时间:  2022.10.21  
     >  maintainer: @kylin-xujian(xujian@kylinos.cn)  

     >  :fa-tags: **[openKylin 安全设计](https://gitee.com/openkylin/sdl-security-design)**  
     >  简介：负责安全设计阶段的规范和流程制定、工具开发工作，内容涵盖：安全设计规范、安全设计辅助工具、安全设计知识库、安全设计平台工具等。  
     >  创建时间:  2022.10.21  
     >  maintainer: @kylin-xujian(xujian@kylinos.cn)  

     >  :fa-tags: **[openKylin 安全开发](https://gitee.com/openkylin/sdl-security-develop)**  
     >  简介：负责安全开发阶段的规范和流程制定、工具开发工作，内容涵盖：安全编码规范、通用编码规范、代码安全扫描工具、安全开发平台工具等。  
     >  创建时间:  2022.10.21  
     >  maintainer: @suxin123456(suxin@kylinos.cn)  
    
     >  :fa-tags: **[openKylin 安全测试](https://gitee.com/openkylin/sdl-security-test)**  
     >  简介：负责安全测试规范、测试过程和测试能力的建设和维护。内容涵盖：安全测试规范、安全测试策略、安全测试流程、安全用例库、安全测试自动化平台、安全质量评价标准等。  
     >  创建时间:  2022.11.25   
     >  maintainer: @wanghongxian(wanghongxian@kylinos.cn)
	 

## SIG成员

### SIG-owner

* @luo-yujia87(luoyujia@kylinos.cn)

### SIG-maintainers

 * @kylin-xujian(xujian@kylinos.cn)
 * @cn-lwj(liwenjie@kylinos.cn)
 * @panexiang(panzhenhua@kylinos.cn)
 * @luming2(luming@kylinos.cn)
 * @a-alpha(chenxinquan@kylinos.cn)
 * @song-bang-cheng-jin(songbangchengjin@kylinos.cn)
 * @lastre3et(yangjipeng@kylinos.cn)
 * @xu_wen_xiu(xuwenxui@kylinos.cn)
 * @advander(gaotong@kylinos.cn)
 * @zhang-zixue(zhangzixue@kylinos.cn)
 * @wanghongxian(wanghongxian@kylinos.cn)
 * @davidwongiiss(wangdawei@syberos.com)
 * @xiaolandan(mawanjun@syberos.com)
 * @icehole(zhangmingming@syberos.com)
