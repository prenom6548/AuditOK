## DBUS接口

[TOC]



### 对应版本信息

|        软件包        |            目前版本             | 备注 |
| :------------------: | :-----------------------------: | :--: |
| kylin-system-updater | kylin-system-updater 1.4.16kord |      |
|      aptdaemon       |     1.1.1+bzr982-0kylin32.3     |      |
|                      |                                 |      |



### 描述

实现系统升级以python apt库和aptdeamon的形式



### Dbus接口信息

| 名称           | 含义                              |
| -------------- | --------------------------------- |
| BUS类型        | SYSTEM BUS                        |
| DBUS名称       | com.kylin.systemupgrade           |
| OBJECT路径     | /com/kylin/systemupgrade          |
| INTERFACES名称 | com.kylin.systemupgrade.interface |
| 属性名称       | org.freedesktop.DBus.Properties   |



### Apt-p2p配置项设置

### Dbus接口信息

| 名称           | 含义                              |
| -------------- | --------------------------------- |
| BUS类型        | SYSTEM BUS                        |
| DBUS名称       | com.kylin.systemupgrade           |
| OBJECT路径     | /com/kylin/systemupgrade          |
| INTERFACES名称 | com.kylin.systemupgrade.interface |
| 属性名称       | org.freedesktop.DBus.Properties   |



#### Get

- `简介：`获取属性的值

- `入参：` `s`iface：要设置的属性的接口， `s`property：要设置的属性名称

- `出参：` `Variant`变量

- `示例：`

  ```
  #获取p2p的配置
  
  Get(com.kylin.systemupgrade.interface,P2pBootstrap)
  ```

  

  

#### Set

- `简介：`设置属性的值

- `入参：`  `s`iiface：要设置的属性的接口， `s`iproperty：要设置的属性名称 `Variant`value：要设置的值

- `出参：`

- `示例：`

  ```
  #设置p2p的配置
  
  set("com.kylin.systemupgrade.interface","P2pBootstrap"，GLib.Variant('s', "test"))
  ```

  



### 方法列表

| Method Name        | Input Args | Output Args | means                             | 异步任务 |
| ------------------ | ---------- | ----------- | --------------------------------- | ------------------ |
| UpdateDetect       | 无         | b           | 更新cache，产生组升级列表JSON文件 |  |
| DistUpgradeAll  | b         | b           | 全部升级                          |  |
| DistUpgradePartial | b,as         | b           | 部分升级                          |  |
| DistUpgradeSystem  | b         | b           | 全盘升级                          |  |
| CancelDownload     | 无         | b           | 取消升级                          |  |
| InsertInstallState    | ss         | b           | 向display表插入数据                          |  |
| GtDownloadspeedLimitValue | 无     | b           | 获取当前限速                          |  |
| SetDownloadspeedMax     | sb         | b          | 设置限速                      |  |
| GetBackendStatus | 无 | i | 控制获取后端状态 |  |
| UnattendedUpgradeValue | ss  | bs   | 获取是否允许关机前更新   |  |
| PurgePackages | as | b | 卸载软件包 |  |
| InstalldebFile | ssbb | b | 安装本地deb包 |  |
| DataBackendCollect | ss | b | |  |
| CheckRebootRequired | s | b | 检查是否需要重启的方法，以及弹窗提示 |  |


### Method分析

#### UpdateDetect

- `简介：`更新cache对象，完成从之后拿到系统中所有可升级的包再经过源过滤、白名单等等的过滤，最后输出当前`可升级的包以及分组（JSON配置 输出目录： /var/lib/kylin-system-updater）

- `入参：`无
- `出参：`True or False 注意：不通过返回值来判断有没有执行成功 通过下面的信号
- `对应信号：`
  - `UpdateDetectStatusChanged:` 更新的进度信息和状态信息
  - `UpdateDetectFinished:`更新的完成的信号



#### DistUpgradePartial

- `简介：` 升级部分软件包或者分组

- `入参：` `b:` False模式：只进行获取升级列表以及计算修复依赖关系，以及计算是否存在删除的包，`True模式:`直接进行安装的操作 注意：必须选使用False模式获取升级列表以及计算依赖关系再进行True模式 

  `as:` 输入需要升级或者安装的分组 例如 升级系统组：["kylin-update-desktop-system"]

- `出参：`True or False 注意：不通过返回值来判断有没有执行成功 通过下面的信号

- `对应信号：`

  - `UpdateDependResloveStatus:` 升级计算依赖修复反馈信号
  - `UpdateDloadAndInstStaChanged:`升级安装过程的进度信号以及状态
  - `UpdateInstallFinished:` 升级安装完成的信号



#### DistUpgradeAll

- `简介：`升级全部可升级的分组

- `入参：` `b:` False模式：只进行获取升级列表以及计算修复依赖关系，以及计算是否存在删除的包，`True模式:`直接进行安装的操作 注意：必须选使用False模式获取升级列表以及计算依赖关系再进行True模式
- `出参：`True or False 注意：不通过返回值来判断有没有执行成功 通过下面的信号
- `对应信号：`
  - `UpdateDependResloveStatus:` 升级计算依赖修复反馈信号
  - `UpdateDloadAndInstStaChanged:`升级安装过程的进度信号以及状态
  - `UpdateInstallFinished:` 升级安装完成的信号




#### UpdateDownloadInfo

- `介绍：`  发送下载包信息信号

- `出参`:   `i:`当前正在下载的项，`i:`所有下载的项，`i:`当前下载的字节，`i:`总的需要下载的字节，`i:`下载速度

- `示例：`

  ```sh
  current_items = 1, total_items = 1, currenty_bytes = 45 kB, total_bytes = 45 kB, current_cps = 0 kB/s
  
  ```



#### GetBackendStatus

- `介绍：`  获取后端的状态，现在正在处理那些任务

- `入参：` `s:`当前用户的语言变量 例如传入语言环境变量`LANG` 的值`zh_CN.UTF-8` 就会将升级的语言切换为中文 同理其他也能相应设置

- `出参`:   `i:`当前任务ID，整型数字 

- `状态示例列表：`

  ```python
  ACTION_DEFUALT_STATUS = -1			#默认状态空闲状态
  ACTION_UPDATE = 0					#处于更新cache状态
  ACTION_INSTALL = 1					#包括部分升级、全部升级、q
  ACTION_INSTALL_DEB = 2				#处于安装deb的状态
  ACTION_CHECK_RESOLVER = 3			#处于计算依赖过程
  ACTION_DOWNLOADONLY = 4				#单独下载软件包过程
  ACTION_FIX_BROKEN = 5				#修复依赖的过程
  ACTION_REMOVE_PACKAGES = 6			#卸载包的状态中
  ```



#### UnattendedUpgradeValue

- `介绍：`  设置或获取是否允许关机前更新

- `入参`:   `s:`operation("get"/"set")，`s:`value将要设置的值

- `示例：`

  ```sh
  operation = "set", value = "false"
  
  ```



#### InstalldebFile

- `简介：`安装本地deb包

- `入参：` `source:(string)` 安装来源，`path:(string)`本地deb包绝对路径，`_check_local_dep:(bool)`出现依赖问题时是否查询本路径下是否存在满足的包，`_auto_satisfy:(bool)`出现依赖问题时是否通过网络下载并安装依赖包
- `出参：`True or False 
- `对应信号：`
   - `InstalldebStatusChanged`:安装过程的进度信号以及状态
   - `InstalldebFinished`:安装完成的信号
- `示例：`

  ```sh
  source = 'kylin-installer', path = '/home/kylin/kylin-video_3.1.0-94.5_amd64.deb', _check_local_dep = 0, _auto_satisfy = 1
  
  ```



#### PurgePackages

- `简介：`卸载系统中的软件包

- `入参：` `as:` 需要卸载的包列表  `s:`当前用户的用户名 例如:kylin用户就传入`kylin`字符串

- `出参：`True or False 出参值不做任何参考意义        `注意：`其中False的时候表示后端正在处理其他任务会报错，其中完成信号也会反馈结果，故不采用方法的返回值来判断错误类型

- `对应信号：`

  - `PurgePkgStatusChanged:`卸载过程的进度信号以及状态
  - `PurgePackagesFinished:` 卸载完成的信号

- `示例：`

  ```sh
  _purge_list = ['kylin-video','tree'] cur_user = 'kylin'
  ```



#### DataBackendCollect

- `介绍：`  后端数据采集

- `入参`:   `messageType: ` 消息埋点(string)， `uploadMessage: ` 上传数据(json格式字符串),必须包含的字段: "packageName"

- `示例：`

  ```sh
  messageType = "UpdateInfos", uploadMessage = "{\"packageName\":\"kylin-system-updater\",\"source\":\"kylin-system-updater\",\"status\":\"True\",\"errorCode\":\"\",\"versionOld\":\"1.2.13.2kord\",\"versionNew\":\"1.2.17.1kord\"}"
  
  messageType: 消息埋点(string) "UpdateInfos"、"InstallInfos"、"RemoveInfos"...
  source: 安装来源 "kylin-installer"、"unattented-upgrade"、"kylin-software-center"、"kylin-system-updater"...
  status: 安装或卸载状态 "True"/"False"
  errorCode: 错误信息 ""/"..."
  versionOld: 旧版本号 "1.2.13.2kord"
  versionNew: 新版本号 "1.2.17.1kord"
  
  ```



#### CheckRebootRequired

- `介绍：`  检查是否需要重启的方法，以及弹窗提示  
- `入参`:   `s:`标识那个应用调的此接口（如自动更新可填入字符串“autoyupgrade”）
- `出参：`True or False 执行的结果



#### SetConfigValue

- `简介：`设置配置文件的值 配置文件的目录`/var/lib/kylin-system-updater/system-updater.conf`

- `入参：` `section`, `option`,` value` 不管布尔、列表的数据类型都转化成字符串类型来写入 

- 例如传入"InstallModel","shutdown_install","False"

  ```
  [InstallMode]
  shutdown_install = True
  manual_install = False
  auto_install = False
  pkgs_install = 
  pkgs_upgrade = 
  pkgs_remove = 
  ```

- `出参：`True :设置值成功 False: 设置失败

#### GetConfigValue

- `简介：`获取配置文件的值 配置文件的目录`/var/lib/kylin-system-updater/system-updater.conf`

- `入参：` `section`,` option`  例如传入"InstallModel","shutdown_install" 

- `出参：` `bool:`True :获取值成功 False: 获取失败 `Value:`值都以字符串类型来返回 

  ```
  [InstallMode]
  shutdown_install = True
  manual_install = False
  auto_install = False
  pkgs_install = 
  pkgs_upgrade = 
  pkgs_remove = 
  ```



#### CheckInstallRequired

- `简介：`检查当前系统是否需要关机安装或者重启安装
- `入参：` 无
- `出参：` `int:`  类型返回值 表示当前系统是否需要安装 返回值数值含义如下。简要为0时不需要安装，不为零时需要进行提示安装
  - `1`  手动更新请求当前系统在关机时进行安装软件包
  -  `2`  自动更新请求当前系统在关机时进行安装软件包
  - `0` 当前系统不需要进行安装  



#### FixBrokenDepends

- `简介：` 修复当前的系统Apt环境，收到`UpdateFixBrokenStatus`后进行调用，类似于调用apt install -f 来进行修复
- `入参：` 无
- `出参：` True or False 执行的结果 无实际意义
- `对应信号：`
  - `FixBrokenStatusChanged:` 修复依赖的状态信号 可不使用



#### MountSquashfsSource

- `简介：` 挂载离线源squashfs
- `入参：` `s:` 挂载文件的位置
- `出参：` `b:` True or False 执行的结果，`s:` 字符类型错误信息描述



### Signal列表

| Signal Name            | Output Args | means                    |
| ---------------------------- | ----------- | ------------------------ |
| UpdateDetectStatusChanged    | i,s          | 更新进度信息以及状态信息 |
| UpdateDetectFinished         | b,as,s,s       | 更新完成信号             |
| UpdateDloadAndInstStaChanged | as,i,s,s | 升级的进度信号以及状态   |
| UpdateInstallFinished       | b,as,s,s       | 升级完成的信号           |
| UpdateDownloadInfo           | i,i,u,u,i | 发送下载包信息信号       |
| UpdateDependResloveStatus           | b,b,s       | 更新依赖修复信息       |
| DistupgradeDependResloveStatus           | b,s       | 更新全盘修复信息       |
| Cancelable                   | b           | 是否可取消               |
| UpdateSqlitSingle |  |  |
| FixBrokenStatusChanged | iiisss | 修复依赖的状态信号 |
| PurgePackagesFinished | iss | 卸载完成信号 |
| PurgePkgStatusChanged | bss | 卸载进度信息以及状态信息 |
| RebootLogoutRequired | s | 请求重启或者注销的信号 |
| UpdateInstallFinished | b,as,s,s | 下载完成的信号           |



### Signal分析

#### UpdateDetectStatusChanged

- `介绍：`更新的进度信息和状态信息

- `出参`:`i:`更新的进度信息从0-100%，`s:`更新的状态信息，

- `示例：`

  ```sh
  progress = 9 , status = 正在解决依赖关系
  progress = 92 , status = 正在载入软件列表
  progress = 92 , status = 完成
  ```

  

#### UpdateDetectFinished

- `介绍：`更新的完成的信号

- `出参`:   `b:`更新是否成功，`as:`可升级的组列表，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  success = True , upgrade_group = ['kylin-update-desktop-system', 'tree', 'texinfo', 'kylin-update-manager', 'dnsmasq-base', 'vino', 'dpkg-dev', 'ghostscript', 'atril', 'wpasupplicant', 'eom', 'eom-common', 'fcitx-bin', 'fcitx-data', 'fcitx-frontend-gtk2', 'wps-office'], error_string =  , error_desc = 
  
  error_string = 获取更新软件推送失败，请稍后再进行尝试更新 , error_desc = 推送服务器连接异常 
  
  ```

#### UpdateDependResloveStatus

- `介绍：`升级计算依赖修复反馈信号

- `出参`:   `b:`修复依赖关系是否成功，`b:`是否存在升级需要卸载的包，`as:`卸载的包列表，`as:`卸载的包的描述信息，`as:`卸载此包的原因 升级安装那些包导致的，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  UpdateDependResloveStatus:resolver_status = True , remove_status = True , remove_pkgs = ['kylin-burner-i18n'],pkg_raw_description = ['Sophisticated CD/DVD burning application - localizations files'] ,delete_desc = ['kylin-burner-i18n 将要被删除，由于升级 kylin-burner'],error_string =  , error_desc =  
  
  ```




#### UpdateFixBrokenStatus

- `介绍：`更新检查过程中是否需要修复系统Apt的环境，提示是否存在卸载软件包的情况

- `出参`:   `b:`是否能修复系统环境，`b:`是否存在修复需要卸载的包，`as:`卸载的包列表，`as:`卸载的包的描述信息，`as:`卸载此包的原因，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  UpdateDependResloveStatus:resolver_status = True , remove_status = True , remove_pkgs = ['kylin-burner-i18n'],pkg_raw_description = ['Sophisticated CD/DVD burning application - localizations files'] ,delete_desc = ['kylin-burner-i18n 将要被删除，由于升级 kylin-burner'],error_string =  , error_desc =  
  
  ```




#### UpdateDloadAndInstStaChanged

- `介绍：`  升级安装过程的进度信号以及状态

- `出参`:     `as:`当前那些组在升级安装   `i:`更新的进度信息从0-100%，`s:`更新的状态信息  `s:`下载的细节信息

- ` 示例：`

  ```sh
  groups_list = ['kylin-update-desktop-system'] progress = 15 , status = 下载中 current_details = 下载中 tree
  ```

  

#### UpdateInstallFinished

- `介绍：`  升级安装完成的信号

- `出参`:   `b:`升级是否成功，`as:`可升级的组列表，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  pdateInstallFinished success = True , upgrade_group = ['tree'], error_string = 系统升级完成。 , error_desc =
  
  ```



#### UpdateDownloadFinished

- `介绍：`  下载完成的信号

- `出参`:   `b:`下载是否成功，`as:`可升级的组列表，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  pdateInstallFinished success = True , upgrade_group = ['tree'], error_string = 系统升级完成。 , error_desc =
  
  ```



#### FixBrokenStatusChanged

- `介绍：`  修复依赖过程中的状态反馈信号

- `出参`:     `i:`修复依赖是否完成   `i:`修复依赖过程是否成功`注意：只有修复完成时，修复依赖是否成功才有意义`，`i:`修复的进度信息  `s:`修复的状态信息 `s:`产生错误的结果，`s:`产生错误的原因

- ` 示例：`

  ```sh
  emit FixBrokenStatusChanged finished = False , success = True,progress = 66 , status = 正在应用更改,error_string =  , error_desc = 
  ```

- 



#### PurgePkgStatusChanged

- `介绍：`卸载的进度信息和状态信息以及状态的细节信息

- `出参`:`i:`卸载的进度信息从0-100%，`s:`卸载的状态信息，`s:`卸载的细节信息

- `示例：`

  ```sh
  INFO:emit PurgePkgStatusChanged progress = 63 , status = 正在应用更改 ,current_details = 正在准备删除 kylin-video
  INFO:emit PurgePkgStatusChanged progress = 76 , status = 正在应用更改 ,current_details = 正在卸载 kylin-video
  
  ```




#### PurgePackagesFinished

- `介绍：`卸载的完成的信号

- `出参`:   `b:`卸载是否成功，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  #卸载完成
  PurgePackagesFinished success = True , error_string = 卸载完成。 , error_desc = 
  
  #卸载失败
  PurgePackagesFinished success = False , error_string = 软件包不存在 , error_desc = 检查包名的拼写是否正确，以及是否启用了相应的仓库。
  PurgePackagesFinished success = False , error_string = 软件包没有安装 , error_desc = 不需要进行卸载。
  
  #卸载失败 由于正在处理其他任务也同样会报错
  PurgePackagesFinished success = False , error_string = 其他任务正在更新升级中，请稍后再卸载。 , error_desc = 
  ```

  

#### InstalldebStatusChanged

- `介绍：`安装的进度信息和状态信息以及状态的细节信息

- `出参`:`i:`安装的进度信息从0-100%，`s:`安装的状态信息，`s:`安装的细节信息

- `示例：`

  ```sh
  InstalldebStatusChanged progress = 57 , status = 正在应用更改 ,current_details = 正在配置 python3-bandit
  InstalldebStatusChanged progress = 57 , status = 正在应用更改 ,current_details = python3-bandit 已安装
  
  ```




#### InstalldebFinished

- `介绍：`安装的完成的信号

- `出参`:   `b:`安装是否成功，`s:`产生错误的结果，`s:`产生错误的原因

- `示例：`

  ```sh
  #安装完成
  InstalldebFinished success = True , error_string =  , error_desc =
  
  #安装失败 缺少依赖的
  InstalldebFinished success = False , error_string = bandit dependency is not satisfied  , error_desc = python3-bandit
  
  #安装失败 选择从网络拉依赖 网络断开 报网络错误
  InstalldebFinished success = False , error_string = 下载软件包文件失败 , error_desc = 检查您的网络连接。
  ```




#### RebootLogoutRequired

- `介绍：`请求重启和注销的信号

- `出参`:   `s:` "reboot" 表示重启 "logout"表示注销

- `示例：`

  ```sh
  Emitting RebootLogoutRequired required_status = reboot
  ```



#### InstallDetectStatus

- `介绍：`下载安装前的状态检查

- `出参`:   `b:`检查出错时为`False`，没有错误`success`，`s:`产生错误的码

- 错误码示例：

  ```python
  ERROR_NOT_DISK_SPACE = "error-not-disk-space"
  ```

- `示例：`

  ```sh
  #表示出现磁盘已满的错误z
  InstallDetectStatus success = False , error_code = "error-not-disk-space"
  ```








后端日志：`/var/log/kylin-system-updater/kylin-system-updater.log.1`

### 更新过程报错信息总结

| 错误信息             | 错误原因                                                     | 解决办法                                 |
| -------------------- | ------------------------------------------------------------ | ---------------------------------------- |
| 下载软件仓库信息失败 | 源存在问题，使用apt update检查，若存在报错则更新管理器无问题 | 检查源是否可以使用                       |
| 无法访问源管理服务器 | 源管理服务器存在问题                                         | 源管理服务器是否可用或者检查源服务器配置 |
| 软件索引已经损坏     | 当前系统中cache被破坏，apt无法使用                           | 终端检查错误原因进行解决                 |
| 无法初始化软件包信息 | apt存在某些问题                                              | 具体错误原因查看日志相应解决             |
| 无法获取组配置软件包 | 源中不存在kylin-update-desktop-config                        | 将此包放入源仓库或者写配置文件不强制安装 |
| 无法读取推送升级列表 | 读取推送列表出现问题                                         | 检查important.list是否存在               |
| 获取软件推送失败     | 老版本文案同 无法访问源管理服务器解决                        |                                          |



### 安装过程报错信息总结

| 错误信息           | 错误原因                       | 解决办法                           |
| ------------------ | ------------------------------ | ---------------------------------- |
| 软件包操作失败     | 被升级的软件包有问题           | 检查后端log日志查看那个包存在问题  |
| 下载软件包文件失败 | 网络原因的或者这个软件包的仓库 | 检查网络以及源仓库                 |
| 磁盘空间不足       | 磁盘的空间不足                 | 查看日志那些目录空间不足           |
| 不能计算升级       | 无法计算依赖关系               | 检查日志那个包出现的问题，相应解决 |
|                    |                                |                                    |























