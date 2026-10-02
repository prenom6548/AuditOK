## HTF
将 HTF（harmony_timeflow）组织的开源项目与 openKylin 进行深度集成，并实现对相关 AI 功能的支持。

SIG组简介

## 工作目标

- 将maximum_timeflow作为标准系统组件集成到openKylin的软件仓库供开发者直接调用。作为纯用户态库不影响系统稳定性。系统级AI推理库到加速具身应用开发，开发者在openKylin上apt install后直接调用C API实现 视觉识别—>运动控制到闭环，无需移植PyTorch模型可以直接使用INT8量化模型。

- 将maximum_timeflow从“第三方库”升级为openKylin操作系统的内置智能能力实现系统级调度与安全管控。将maximum_timeflow作为完全独立、零依赖的静态库由上层应用直接嵌入到自己的openKylin应用中，不依赖系统服务、不注册全局组件、无后台进程。形成你用你的系统，我只提供推理能力。独立嵌入式AI引擎可支撑高安全具身终端，零网络、零后台、零动态库，符合“物理隔离”安全规范并可在128MB RAM的ARM板上运行。

- 定义htf_inspect在openKylin生态中的角色，作为系统级开发工具包（SDK）组件
将htf_inspect打包为RPM/DEB包纳入openKylin官方软件源：
sudo apt install htf-inspect    #或 yum install htf-inspect
安装后提供：
/usr/bin/htf_inspect（命令行工具）
/usr/lib/libhtf_inspect.so（供其他工具调用）
/usr/share/doc/htf_inspect/（文档、示例模型）
目标用户：AI应用开发者、系统集成商、安全审计人员。

- 现有harmony_timeflow的开发板兼容openKylin系统，HTF_matrix_nano_borad_1.0作为通信子板，通过排针连接外部主控板

- 相关活动的举办

- 其他



## SIG成员

SIG-owner

袁睿(16651797882@163.com)

SIG-maintainer


## SIG邮件列表
> 格式：SIG名小写@lists.openkylin.top（SIG申请审核通过后，管理员将创建该邮件列表）

## SIG组例会
> 明确SIG组例会召开时间和周期，如周例会、双周例会或月会等