# X-TeeOS兴趣小组（SIG）

X-TeeOS（Trusted Execution Environment OS）特别兴趣小组（SIG），致力于围绕“双体系架构 + Rust 全栈”构建下一代安全可信子系统。在当前操作系统可信化与软硬件协同可信的发展趋势下，X-TEEOS 旨在探索并落地一套面向关键业务场景的**轻量级、高可靠、可验证、永不中断（Never-Die）**的可信执行环境。

## 工作目标

X-TEEOS 的核心目标包括：

- 打造双体系安全框架: 在原有主系统之上构建独立可信环境，通过隔离、监控与降级机制，为关键服务提供稳定执行底座。

- 推进 Rust 全栈可信实现: 以 Rust 语言为核心构建内核、运行时、驱动、系统服务，充分利用其内存安全特性，降低缺陷密度与攻击面。

- 构建可验证的安全子系统: 包括任务调度、资源隔离、状态持续记录、关键事件追踪，为安全场景提供高可信保障。

- 支持多场景的可信运行与实时监测: 为嵌入式、终端设备等不同形态的系统提供统一的可信组件。

## SIG组件&人员介绍

### Owner

guo-weikang（guoweikang@kylinos.cn）

### X-TEEOS 关键组件及维护人员

|序号| 组件                       | 介绍                               | maintainers               |committers                 |
|----|----------------------------|------------------------------------|---------------------------|---------------------------|
|1   | x-kernel                  | 可信内核主仓                       | guo-weikang  luodeb       | kylinyubo wangzhen76      |
|2   | arceos                     | 可信内核核心组件                   | guo-weikang luodeb        | kylinyubo wangzhen76      |
|3   | starry-test-harness        | 可信内核测试框架                   | yeanwang、sunhaosheng     |                           |
|4   | axplat-aarch64-crosvm-virt | 可信内核平台组件                   | guo-weikang luodeb        |                           |
|5   | rust-mbedtls               | 可信算法核心库                     | kylinyubo                 | wangzhen76  du-yq221 zhangyoujing666  |
|6   | rust-dice                  | 可信轻量信任链组件                 | kylinyubo                 | wangzhen76  du-yq221 zhangyoujing666  |
|7   | axwatchdog                 | 可信内核看门狗组件                 | luodeb, wxyy1             |                           |
|8   | axplat_crates              | 可信内核平台依赖库集合             | guo-weikang luodeb        |                           |
|9   | microkylin_manager         | 可信轻量信任链服务                 | du-yq221                  |                           |
|10  | axdriver_crates            | 可信内核驱动核心组件               | guo-weikang luodeb        |                           |
|11  | arm-gic                    | gicv3驱动组件                      | guo-weikang luodeb        |                           |
|12  | syscalls                   | 可信内核系统调用第三方组件         | guo-weikang kylinyubo     |                           |
|13  | fdtree-rs                  | 可信内核设备树解析组件             | guo-weikang               |                           |
|14  | realtek-drivers            | 可信内核网卡驱动组件               | guo-weikang kylinyubo     |                           |
|15  | tee_raw_sys                | 可信系统调用参数描述组件           | kylinyubo                 |                           |
|16  | starry-strace              | 可信系统strace组件                 | jose47                    |                           |
|17  | aarch64-pmuv3              | 可信内核pmu驱动组件                | luodeb, wxyy1             |                           |
|18  | optee_os                   | OPTEE内核                          | withcherthewhite kylinyubo|wangzhen76  du-yq221 zhangyoujing666 |
|19  | optee-utee                 | 可信应用TA开发sdk                  | withcherthewhite kylinyubo|wangzhen76  du-yq221 zhangyoujing666 |
|20  | optee-utee-build           | 可信应用TA开发生成脚本             | withcherthewhite kylinyubo|wangzhen76  du-yq221 zhangyoujing666 |
|21  | yocto-starry               | 基于yocto的可信内核镜像构建系统    | yeanwang                  |                           |
|22  | xbox-backend               | X-box子系统后端服务组件            | luodeb                    |                           |
|23  | xbox-frontend              | X-box子系统前端交互组件            | luodeb                    |                           |
|24  | xbox-aiagent               | X-box子系统AI智能助手组件          | luodeb                    |                           |
|25  | xbox-blackbox              | X-box子系统黑匣子组件              | luodeb                    |                           |
|26  | xbox-monitor               | X-box子系统监控组件                | luodeb                    |                           |
|27  | rust-libutee               | TEE内部核心API的Rust实现           | withcherthewhite kylinyubo buhenxihuan                  |wangzhen76 du-yq221 zhangyoujing666                          |
|28  | rust-libteec               | TEE用户空间应用程序API的Rust实现   | withcherthewhite kylinyubo buhenxihuan gitee_fuxi                  |wangzhen76 du-yq221 zhangyoujing666                          |
|29  | rust-libtipc               | TEE IPC通信API的Rust实现           | kylinyubo gitee_fuxi      |                           |
|30  | tasign                     | TA ELF 文件签名验签工具            | kylinyubo                 |                           |
|31  | xtee-rust-sdk              | x-kernel tee SDK                   | withcherthewhite kylinyubo | withcherthewhite kylinyubo gitee_fuxi |
|32  | xtee-apps                  | x-kernel tee 可信应用              | withcherthewhite kylinyubo gitee_fuxi | withcherthewhite kylinyubo gitee_fuxi wangzhen76 du-yq221 |
|33  | virga                      | 基于 VSock 的通信库                | sj_huang                  |                           |
|34  | xtee-storage               | trusty tee storage                 | kylinyubo                 |                           |

## SIG邮件列表

x-teeos@lists.openkylin.top

## SIG 组例会

周会
