### 项目简介
 
ISO镜像定制工具 是 openKylin 社区 UKUI SIG 下的一个桌面小工具，用于对 openKylin/Kylin 操作系统的 ISO 镜像进行可视化定制。通过图形化界面（基于 Python Tkinter），用户可以方便地向 ISO 镜像中添加 DEB 包、授权文件、字体、KMS 配置、无人值守参数等，快速生成适用于特定场景（如区县政务、行业客户）的定制化安装镜像。

程序最终会重新打包生成一个新的 .iso 文件，可直接用于安装部署。
### 功能特性
 **1. ISO 镜像选择与自动识别** 
通过文件对话框选择基础 ISO 镜像
自动识别华为版本镜像（通过文件名关键字：kirin、9006、9000、pangu、wayland）
自动识别 panguw 镜像并启用对应参数处理
选择 ISO 后自动挂载、复制内容、卸载，扫描 ISO 中已有的 DEB 包
 **2. DEB 包管理** 
third-party：支持添加/删除/清空 DEB 包，自动复制到 ISO 的 third-party 目录（华为机器自动放入 third-party/common）
apps-third：支持添加/删除/清空 DEB 包，自动生成 Packages 索引和 .cfg 配置文件（Default=yes）
支持通过 dpkg-scanpackages 自动扫描并索引软件包
删除 apps-third 包时仅将配置中 Default=yes 改为 no，不删除源文件
 **3. 无人值守安装配置** 
可启用/禁用无人值守安装
支持设置：
用户名、密码、主机名
数据盘设备及分区格式
自动登录、出厂备份、安装后重启
自动修改 ky-installer.cfg / kyple-installer.cfg 和 grub.cfg
华为机器特殊处理：自动添加 automatic / test-mod 参数，移除 oem-config/enable=true
 **4. 百信双硬盘策略（独立于无人值守）** 
可独立启用
可配置目标数据盘大小（GB）
生成后置脚本自动检测目标硬盘并创建数据分区
 **5. 授权文件管理** 
支持添加 .kyinfo、LICENSE 文件
支持 ZIP 压缩包自动解压并提取授权文件
自动将授权文件复制到 ISO 根目录
 **6. 系统字体添加** 
支持 .ttf 字体文件
支持 ZIP 字体包自动解压提取
安装后自动执行 fc-cache -fv
 **7. KMS 配置** 
可启用 KMS 激活
配置 KMS 服务器 IP
生成后置脚本，安装后自动检查服务器可达性并执行激活
自动更新 /usr/share/kylin-activation/activation_conf.ini 中的 ServerIp 和 serverip 配置
 **8. 安全中心设置** 
支持一键关闭以下安全功能（写入 .kylin-post-actions）：
防火墙
应用保护
网络保护
设备安全
进程保护
文件保护
签名状态
kid 状态
 **9. kpb 补丁集成** 
支持添加 .kpb 补丁文件
自动复制到 ISO 根目录的 patches 目录
 **10. hosts 文件修改** 
图形化添加 IP 地址与域名映射
支持 IPv4 和 IPv6 地址校验
生成后置脚本自动更新 /etc/hosts
 **11. 自定义配置** 
自定义文件夹：将文件或文件夹复制到 ISO 根目录的 custom 文件夹（保留原有层级）
自定义脚本：用户自定义脚本会合并追加到 .kylin-post-actions 末尾，与镜像中原有自定义脚本合并，不会覆盖
 **12. squashfs 软件管理** 
删除奇安信杀毒软件（dpkg -P qaxsafe）
安装自定义 squashfs 软件包（chroot 环境下 dpkg -i）
统一解压、安装、重新打包流程
 **13. 执行回显与状态显示** 
实时显示每一步执行命令和输出
右侧状态栏汇总显示当前所有配置项
进度提示与错误弹窗
### 技术栈
语言：Python 3
GUI 框架：Tkinter / ttk
### 依赖工具：
mount / umount（ISO 挂载）
rsync（文件复制）
unsquashfs / mksquashfs（文件系统解压与打包）
genisoimage（ISO 封装）
dpkg-deb / dpkg-scanpackages / dpkg（DEB 包管理）
chroot（系统环境切换）
du（大小计算）
md5sum（校验和生成）
### 运行环境
操作系统：openKylin / Kylin V10 SP1
Python：Python 3.x
权限：必须以 root 权限运行（涉及挂载、chroot、系统目录修改等操作）
依赖工具：上述依赖工具需已安装
### 目录结构

程序运行时会在 /opt/iso-customizer-work 下创建以下工作目录：
text
/opt/iso-customizer-work/
├── mnt/              # ISO 挂载点
├── livecd/           # ISO 内容复制目录
├── tool/             # 工具目录
└── squashfs-root/    # 解压后的文件系统根目录
日志文件：/var/log/iso_customizer.log
### 使用方法
 **1. 以 root 权限启动** 
bash
sudo python3 iso_customizer.py
或者如果是打包后的可执行文件：
bash
sudo ./iso-customizer
 **2. 基本流程** 
选择 ISO 文件：点击“浏览”选择基础 ISO 镜像，程序会自动开始准备（挂载、复制、扫描）
 **配置镜像：** 
- 添加 DEB 包（third-party / apps-third）
- 设置无人值守安装参数
- 添加授权文件、字体、kpb 补丁
- 配置 KMS、安全中心、hosts、自定义文件夹/脚本
- 管理 squashfs 软件
- 填写输出配置：填写区县名称，预览最终输出文件名
- 开始制作：点击“开始制作”按钮，等待进度完成
- 获取镜像：制作完成后，生成的 ISO 文件位于 /opt/iso-customizer-work/ 目录下
 **3. 输出文件命名** 
输出文件名格式为：{原ISO文件名}-{区县名称}.iso
例如：原文件 Kylin-Desktop-V10-SP1.iso，区县名称 shandong，则输出为 Kylin-Desktop-V10-SP1-shandong.iso
### 版本历史
v2.0：取消 filesystem 解压场景；支持 DEB 包两种安装方式；后置操作统一放入 .kylin-post-actions；优化界面显示
v2.1：优化华为镜像自动识别；修复无人值守参数设置；修复百信双硬盘模式
v2.2：百信双硬盘策略独立于无人值守；新增 KMS 激活命令更新及 sync 落盘；取消默认用户名密码主机名设置
v2.3：修复 panguw 镜像识别失败；修复启动 KMS 后 U 盘安装失败
v2.4：修复 panguw 下 grub 无人值守参数问题
v2.5：修复华为镜像判断字段添加 Wayland；优化百信双硬盘格式化命令；优化 license 文件夹
v2.6：修复多次封装后 .kylin-post-actions 被刷新；新增 kpb 补丁集成；新增关闭 kid 状态；新增 hosts 修改
v2.7：新增字体格式选择；新增 third-party / apps-third 包选择；新增 .kylin-post-actions 自定义；新增删除奇安信软件
v2.8：修复 apps-third 删除功能异常；新增自定义文件夹功能；优化自定义脚本功能
### 注意事项
必须以 root 权限运行，否则挂载、chroot、系统目录修改等操作会失败
运行前请确保系统已安装所有依赖工具（rsync、squashfs-tools、genisoimage、dpkg 等）
工作目录 /opt/iso-customizer-work 会在每次开始制作前清理，请勿在此目录存放重要文件
生成的 ISO 镜像默认位于工作目录下，请及时移动到安全位置
华为版本镜像需要额外的 preinstalled-apps 包来处理 apps-third 包，程序会弹出对话框要求选择
如遇权限或日志写入问题，程序会降级为控制台日志输出
程序运行日志位于 /var/log/iso_customizer.log，排查问题时请优先查看此日志
### 贡献与反馈
本项目属于 openKylin 社区 UKUI SIG，欢迎通过以下方式参与贡献：
提交 Issue 反馈问题或建议
提交 Pull Request 贡献代码
参与 UKUI SIG 邮件列表讨论
仓库地址：https://gitee.com/openkylin/ukui-iso-customer