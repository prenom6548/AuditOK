## Embedded-SIG组

本SIG组将打造openKylin嵌入式操作系统，提供常见工控总线、软实时、硬实时、虚拟化、web管理、轻量级、安全性等多种特性，繁荣openKylin系统生态。

## 工作目标

- 对openKylin系统进行裁剪，并形成嵌入式系统版本，并对该系统进行版本生命周期维护
- 依托openKylin集成构建平台，打造嵌入式自动化构建体系
- 为其他厂商发行开源嵌入式系统或商业系统提供技术支撑
- 研发针对嵌入式系统的SDK，并进行版本生命周期维护
- 提供适用于嵌入式场景的实时性方案
- 提供适用于嵌入式场景的轻量级UKUI桌面
- 提供嵌入式混合部署方案
- 引入Preempt_RT软实时方案
- 引入Xenomai 3实时性方案
- 引入Xenomai 4实时性方案
- 引入常见工控协议，如Libmodbus、CANOpen、CanFestival、IGH-EtherCAT
- 引入RTHypervisor SIG的jailhouse虚拟化方案
- 回馈上游社区，如kernel、Xenomai、jailhouse等

## ROADMAP

- 2023年2月，引入工控协议
- 2023年4月，针对飞腾开源内核增加RT补丁
- 2023年5月，针对飞腾开源内核增加Xenomai3实时方案
- 2023年6月，完成嵌入式系统与openKylin集成构造环境对接
- 2023年8月，针对飞腾开源内核增加Xenomai4实时方案
- 2023年9月，完成openKylin Embedded系统V1.0发布并开源
- 2023年10月，完成openKylin Embedded系统SDK发布并开源
- 2023年12月，完成openKylin Embedded系统安全增强功能

## SIG成员

### Owner

郭皓[@guohaocs2c](https://gitee.com/guohaocs2c) ([guohao@kylinos.cn](mailto:guohao@kylinos.cn))

### Maintainer列表

| Maintainer                                              | 邮箱                                                        |
| ------------------------------------------------------- | ----------------------------------------------------------- |
| 郭皓[@guohaocs2c](https://gitee.com/guohaocs2c)         | [guohao@kylinos.cn](mailto:guohao@kylinos.cn)               |
| 吴春光[@wuchunguang](https://gitee.com/wuchunguang)     | [wuchunguang@kylinos.cn](mailto:wuchunguang@kylinos.cn)     |
| 张培[@pzhanggit](https://gitee.com/pzhanggit)           | [zp_cn@126.com](mailto:zp_cn@126.com)                       |
| 张志成[@zzc-kylin](https://gitee.com/zzc-kylin)         | [zhangzhicheng@kylinos.cn](mailto:zhangzhicheng@kylinos.cn) |
| 黄顺玉[@huang-shunyu](https://gitee.com/huang-shunyu)   | [huangshunyu@kylinos.cn](mailto:huangshunyu@kylinos.cn)     |
| 陶术松[@tao12345](https://gitee.com/tao12345)           | [taoshusong@kylinos.cn](mailto:taoshusong@kylinos.cn)       |
| 马玉昆[@kylin-mayukun](https://gitee.com/kylin-mayukun) | [mayukun@kylinos.cn](mailto:mayukun@kylinos.cn)             |
| 李铭乐[@eelxela](https://gitee.com/eelxela)             | [limingle@kylinos.cn](mailto:limingle@kylinos.cn)           |
| 刘仁学[@liurenxue](https://gitee.com/liurenxue)         | [liurenxue@kylinos.cn](mailto:liurenxue@kylinos.cn)         |
| 张远航[@zhangyh1992](https://gitee.com/zhangyh1992)     | [zhangyuanhang@kylinos.cn](mailto:zhangyuanhang@kylinos.cn) |
| 张玉[@zhangyuge001](https://gitee.com/zhangyuge001)     | [zhangyu4@kylinos.cn](mailto:zhangyu4@kylinos.cn)           |
| 李钰磊[@r2018](https://gitee.com/r2018)                 | [liyulei@kylinos.cn](mailto:liyulei@kylinos.cn)             |
| 陈登[@chend2019](https://gitee.com/chend2019)           | [chendeng@kylinos.cn](mailto:chendeng@kylinos.cn)             |
| 廖元垲[@liao-yuankai](https://gitee.com/liao-yuankai)   | [yuankai.liao@cdjrlc.com](mailto:yuankai.liao@cdjrlc.com)   |
| 廖茂益[@ixr](https://gitee.com/ixr)                     | [liaomaoyi@cdjrlc.com](mailto:liaomaoyi@cdjrlc.com)         |
| 李昂[@liqingning](https://gitee.com/liqingning)         | [hcnc.li@gmail.com](mailto:hcnc.li@gmail.com)               |

### Committer列表

## SIG维护包列表

- [openkylin-embedded-builder](https://gitee.com/openkylin/openkylin-embedded-builder)
- [bootfiles-embedded](https://gitee.com/openkylin/bootfiles-embedded)
- [rk-kernel-5.4](https://gitee.com/openkylin/rk-kernel-5.4)
- [riscv-kernel-5.4](https://gitee.com/openkylin/riscv-kernel-5.4)
- [loongarch-kernel-5.4](https://gitee.com/openkylin/loongarch-kernel-5.4)
- [phytium-kernel-4.19](https://gitee.com/openkylin/phytium-kernel-4.19)
- [lotus2-kernel-5.15](https://gitee.com/openkylin/lotus2-kernel-5.15)
- [rockchip-kernel-5.10](https://gitee.com/openkylin/rockchip-kernel-5.10)
- [visionfive2-kernel-5.10](https://gitee.com/openkylin/visionfive2-kernel-5.10)
- [raspberrypi-kernel-5.15](https://gitee.com/openkylin/raspberrypi-kernel-5.15)
- [phytiumpi-kernel-4.19](https://gitee.com/openkylin/phytiumpi-kernel-4.19)
- [industry-kernel-4.19](https://gitee.com/openkylin/industry-kernel-4.19)
- [ukui-embedded](https://gitee.com/openkylin/ukui-embedded)
- [preempt_rt](https://gitee.com/openkylin/preempt_rt)
- [rtcheck](https://gitee.com/openkylin/rtcheck)
- [rteval](https://gitee.com/openkylin/rteval)
- [rt-tests](https://gitee.com/openkylin/rt-tests)
- [xenomai3](https://gitee.com/openkylin/xenomai3)
- [xenomai4](https://gitee.com/openkylin/xenomai4)
- [ipipe](https://gitee.com/openkylin/ipipe)
- [canfestival-xenomai](https://gitee.com/openkylin/canfestival-xenomai)
- [canfestival](https://gitee.com/openkylin/canfestival)
- [igh-ethercat-xenomai](https://gitee.com/openkylin/igh-ethercat-xenomai)
- [ethercat-igh](https://gitee.com/openkylin/ethercat-igh)
- [libmodbus-xenomai](https://gitee.com/openkylin/libmodbus-xenomai)
- [libmodbus](https://gitee.com/openkylin/libmodbus)
- [canopennode](https://gitee.com/openkylin/canopennode)
- [libnetconf2](https://gitee.com/openkylin/libnetconf2)
- [libyang1](https://gitee.com/openkylin/libyang1)
- [netopeer2](https://gitee.com/openkylin/netopeer2)
- [soem](https://gitee.com/openkylin/soem)
- [soes](https://gitee.com/openkylin/soes)
- [sysrepo](https://gitee.com/openkylin/sysrepo)
- [atril1](https://gitee.com/openkylin/atril1)
- [caja1](https://gitee.com/openkylin/caja1)
- [caja-extensions](https://gitee.com/openkylin/caja-extensions)
- [cockpit](https://gitee.com/openkylin/cockpit)
- [cockpit-appstream](https://gitee.com/openkylin/cockpit-appstream)
- [fcitx1](https://gitee.com/openkylin/fcitx1)
- [fcitx-chewing](https://gitee.com/openkylin/fcitx-chewing)
- [gnome-shell-extension-arc-menu](https://gitee.com/openkylin/gnome-shell-extension-arc-menu)
- [gsettings-qt1](https://gitee.com/openkylin/gsettings-qt1)
- [libmatekbd1](https://gitee.com/openkylin/libmatekbd1)
- [libmatemixer1](https://gitee.com/openkylin/libmatemixer1)
- [libmateweather](https://gitee.com/openkylin/libmateweather)
- [libosinfo1](https://gitee.com/openkylin/libosinfo1)
- [libpfm](https://gitee.com/openkylin/libpfm)
- [marco](https://gitee.com/openkylin/marco)
- [mate-common](https://gitee.com/openkylin/mate-common)
- [mate-control-center](https://gitee.com/openkylin/mate-control-center)
- [mate-desktop1](https://gitee.com/openkylin/mate-desktop1)
- [mate-icon-theme](https://gitee.com/openkylin/mate-icon-theme)
- [mate-menus](https://gitee.com/openkylin/mate-menus)
- [mate-notification-daemon](https://gitee.com/openkylin/mate-notification-daemon)
- [mate-panel](https://gitee.com/openkylin/mate-panel)
- [mate-polkit1](https://gitee.com/openkylin/mate-polkit1)
- [mate-power-manager](https://gitee.com/openkylin/mate-power-manager)
- [mate-session-manager](https://gitee.com/openkylin/mate-session-manager)
- [mate-settings-daemon](https://gitee.com/openkylin/mate-settings-daemon)
- [mate-system-monitor](https://gitee.com/openkylin/mate-system-monitor)
- [mate-terminal1](https://gitee.com/openkylin/mate-terminal1)
- [mate-user-guide](https://gitee.com/openkylin/mate-user-guide)
- [mate-utils](https://gitee.com/openkylin/mate-utils)
- [migrationtools](https://gitee.com/openkylin/migrationtools)
- [openssl-ibmpkcs11](https://gitee.com/openkylin/openssl-ibmpkcs11)
- [osinfo-db1](https://gitee.com/openkylin/osinfo-db1)
- [pluma1](https://gitee.com/openkylin/pluma1)
- [pygtksourceview](https://gitee.com/openkylin/pygtksourceview)
- [python3-schedule](https://gitee.com/openkylin/python3-schedule)
- [python-requests-oauthlib](https://gitee.com/openkylin/python-requests-oauthlib)
- [python-xlib1](https://gitee.com/openkylin/python-xlib1)
- [sos](https://gitee.com/openkylin/sos)
- [uptimed](https://gitee.com/openkylin/uptimed)
- [webkitgtk](https://gitee.com/openkylin/webkitgtk)
- [xorg-x11-drv-amdgpu](https://gitee.com/openkylin/xorg-x11-drv-amdgpu)
- [adobe-source-code-pro-fonts](https://gitee.com/openkylin/adobe-source-code-pro-fonts)
- [fortune-mod](https://gitee.com/openkylin/fortune-mod)
- [paho-c](https://gitee.com/openkylin/paho-c)
- [pssh](https://gitee.com/openkylin/pssh)
- [smc-tools](https://gitee.com/openkylin/smc-tools)
- [imsettings](https://gitee.com/openkylin/imsettings)
- [optee_client](https://gitee.com/openkylin/optee_client)
- [kylin-control-center](https://gitee.com/openkylin/kylin-control-center)

## 邮件列表

## SIG组例会

