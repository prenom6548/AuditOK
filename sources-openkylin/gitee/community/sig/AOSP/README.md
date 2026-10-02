# AOSP兴趣小组（SIG）

AOSP SIG组负责AOSP在云手机和安卓硬件生态设备上的适配开发、版本维护、源码仓库管理和开发手册编写等工作，致力于打造基于AOSP的开源智能终端操作系统。


## 工作目标

- 提供在瑞芯微、展锐、Pixel系列等硬件平台上的内核，支撑AOSP运行;
- 完成Android 12等AOSP版本在瑞芯微、展锐、Pixel系列等硬件平台上的新特性开发；
- 实现安卓系统裁剪、定制改造和性能优化，提供安卓系统解决方案；
- 适配新的安卓硬件生态设备元器件，实现对安卓HAL层的扩展;
- 基于Cuttlefish虚拟化套件，在X86和ARM架构上虚拟出高保真的Android设备;
- 实现麒麟移动运行环境（KMRE），支持安卓应用在Linux上高效稳定运行。


## SIG成员

### Owner

- Duo Zhang(zhangduo@kylinos.cn)
- [balloonflower](https://gitee.com/balloonflower)

### Maintainers

- [balloonflower](https://gitee.com/balloonflower)
- [kmre-clom](https://gitee.com/kmre-clom)
- [mccaaab](https://gitee.com/mccaaab)
- [binghaiwanglifang](https://gitee.com/binghaiwanglifang)
- [zhuisu930](https://gitee.com/zhuisu930)
- [xsjkiver](https://gitee.com/xsjkiver)
- [Spirituality_Tao](https://gitee.com/Spirituality_Tao)
- [ikunXY](https://gitee.com/ikunXY)
- [youyuan_2022](https://gitee.com/youyuan_2022)
- [kylin-huanglei](https://gitee.com/kylin-huanglei)
- [winter91](https://gitee.com/winter91)
- [linkgong](https://gitee.com/linkgong)
- [threewater000](https://gitee.com/threewater000)
- [jason.R.xie](https://gitee.com/jason.R.xie)
- [agor](https://gitee.com/agor)
- [lhqqwerasdg](https://gitee.com/lhqqwerasdg)
- [moomhalf](https://gitee.com/moomhalf)


## SIG维护包列表

- [platform_manifest](https://gitee.com/openkylin/platform_manifest)
- [bootable_newinstaller](https://gitee.com/openkylin/bootable_newinstaller)
- [device_generic_goldfish_opengl](https://gitee.com/openkylin/device_generic_goldfish_opengl)
- [device_generic_kmre](https://gitee.com/openkylin/device_generic_kmre)
- [device_generic_kmre_arm64](https://gitee.com/openkylin/device_generic_kmre_arm64)
- [device_generic_kmre_x86_64](https://gitee.com/openkylin/device_generic_kmre_x86_64)
- [kmre_apps_inputmethod](https://gitee.com/openkylin/kmre_apps_inputmethod)
- [kmre_apps_kmrelauncher](https://gitee.com/openkylin/kmre_apps_kmrelauncher)
- [kmre_apps_kmremanager](https://gitee.com/openkylin/kmre_apps_kmremanager)
- [kmre_device_audio](https://gitee.com/openkylin/kmre_device_audio)
- [kmre_gps](https://gitee.com/openkylin/kmre_gps)
- [kmre_lib_kmrecore](https://gitee.com/openkylin/kmre_lib_kmrecore)
- [kmre_sensor](https://gitee.com/openkylin/kmre_sensor)
- [packages_apps_settings](https://gitee.com/openkylin/packages_apps_settings)
- [platform_art](https://gitee.com/openkylin/platform_art)
- [platform_bionic](https://gitee.com/openkylin/platform_bionic)
- [platform_bootable_recovery](https://gitee.com/openkylin/platform_bootable_recovery)
- [platform_build](https://gitee.com/openkylin/platform_build)
- [platform_build_soong](https://gitee.com/openkylin/platform_build_soong)
- [platform_external_bluetooth_bluez](https://gitee.com/openkylin/platform_external_bluetooth_bluez)
- [platform_external_busybox](https://gitee.com/openkylin/platform_external_busybox)
- [platform_external_display_control](https://gitee.com/openkylin/platform_external_display_control)
- [platform_external_drm_gralloc](https://gitee.com/openkylin/platform_external_drm_gralloc)
- [platform_external_ffmpeg](https://gitee.com/openkylin/platform_external_ffmpeg)
- [platform_external_goldfish_post](https://gitee.com/openkylin/platform_external_goldfish_post)
- [platform_external_googleanalytics](https://gitee.com/openkylin/platform_external_googleanalytics)
- [platform_external_libdrm](https://gitee.com/openkylin/platform_external_libdrm)
- [platform_external_llvm90](https://gitee.com/openkylin/platform_external_llvm90)
- [platform_external_llvm-project](https://gitee.com/openkylin/platform_external_llvm-project)
- [platform_external_mesa](https://gitee.com/openkylin/platform_external_mesa)
- [platform_external_noto-fonts](https://gitee.com/openkylin/platform_external_noto-fonts)
- [platform_external_robolectric-shadows](https://gitee.com/openkylin/platform_external_robolectric-shadows)
- [platform_external_s2tc](https://gitee.com/openkylin/platform_external_s2tc)
- [platform_external_selinux](https://gitee.com/openkylin/platform_external_selinux)
- [platform_external_skia](https://gitee.com/openkylin/platform_external_skia)
- [platform_external_stagefright-plugins](https://gitee.com/openkylin/platform_external_stagefright-plugins)
- [platform_external_syslinux](https://gitee.com/openkylin/platform_external_syslinux)
- [platform_frameworks_av](https://gitee.com/openkylin/platform_frameworks_av)
- [platform_frameworks_base](https://gitee.com/openkylin/platform_frameworks_base)
- [platform_frameworks_native](https://gitee.com/openkylin/platform_frameworks_native)
- [platform_frameworks_opt_net_wifi](https://gitee.com/openkylin/platform_frameworks_opt_net_wifi)
- [platform_frameworks_opt_telephony](https://gitee.com/openkylin/platform_frameworks_opt_telephony)
- [platform_hardware_gps](https://gitee.com/openkylin/platform_hardware_gps)
- [platform_hardware_intel_common_libva](https://gitee.com/openkylin/platform_hardware_intel_common_libva)
- [platform_hardware_intel_common_vaapi](https://gitee.com/openkylin/platform_hardware_intel_common_vaapi)
- [platform_hardware_interfaces](https://gitee.com/openkylin/platform_hardware_interfaces)
- [platform_hardware_libcamera](https://gitee.com/openkylin/platform_hardware_libcamera)
- [platform_hardware_libhardware](https://gitee.com/openkylin/platform_hardware_libhardware)
- [platform_hardware_libsensors](https://gitee.com/openkylin/platform_hardware_libsensors)
- [platform_hardware_memtrack](https://gitee.com/openkylin/platform_hardware_memtrack)
- [platform_libcore](https://gitee.com/openkylin/platform_libcore)
- [platform_packages_apps_camera2](https://gitee.com/openkylin/platform_packages_apps_camera2)
- [platform_packages_apps_documentsui](https://gitee.com/openkylin/platform_packages_apps_documentsui)
- [platform_packages_apps_gallery2](https://gitee.com/openkylin/platform_packages_apps_gallery2)
- [platform_packages_apps_launcher3](https://gitee.com/openkylin/platform_packages_apps_launcher3)
- [platform_packages_apps_packageinstaller](https://gitee.com/openkylin/platform_packages_apps_packageinstaller)
- [platform_packages_modules_networkstack](https://gitee.com/openkylin/platform_packages_modules_networkstack)
- [platform_packages_providers_downloadprovider](https://gitee.com/openkylin/platform_packages_providers_downloadprovider)
- [platform_packages_providers_mediaprovider](https://gitee.com/openkylin/platform_packages_providers_mediaprovider)
- [platform_prebuilts_abi-dumps_vndk](https://gitee.com/openkylin/platform_prebuilts_abi-dumps_vndk)
- [platform_prebuilts_sdk](https://gitee.com/openkylin/platform_prebuilts_sdk)
- [platform_system_bpf](https://gitee.com/openkylin/platform_system_bpf)
- [platform_system_bt](https://gitee.com/openkylin/platform_system_bt)
- [platform_system_connectivity_wificond](https://gitee.com/openkylin/platform_system_connectivity_wificond)
- [platform_system_core](https://gitee.com/openkylin/platform_system_core)
- [platform_system_extras](https://gitee.com/openkylin/platform_system_extras)
- [platform_system_hardware_interfaces](https://gitee.com/openkylin/platform_system_hardware_interfaces)
- [platform_system_linkerconfig](https://gitee.com/openkylin/platform_system_linkerconfig)
- [platform_system_netd](https://gitee.com/openkylin/platform_system_netd)
- [platform_system_sepolicy](https://gitee.com/openkylin/platform_system_sepolicy)
- [platform_system_vold](https://gitee.com/openkylin/platform_system_vold)
- [platform_packages_modules_wifi](https://gitee.com/openkylin/platform_packages_modules_wifi)
- [platform_packages_modules_connectivity](https://gitee.com/openkylin/platform_packages_modules_connectivity)
- [device_google_redfin](https://gitee.com/openkylin/device_google_redfin)
- [platform_kernel-5.15](https://gitee.com/openkylin/platform_kernel-5.15)
- [device_google_cuttlefish](https://gitee.com/openkylin/device_google_cuttlefish)
- [platform_external_crosvm](https://gitee.com/openkylin/platform_external_crosvm)
- [device_generic_vulkan-cereal](https://gitee.com/openkylin/device_generic_vulkan-cereal)
- [kylin-kmre-manager](https://gitee.com/openkylin/kylin-kmre-manager)
- [kylin-kmre-window](https://gitee.com/openkylin/kylin-kmre-window)
- [kylin-kmre-daemon](https://gitee.com/openkylin/kylin-kmre-daemon)
- [kylin-kmre-emugl](https://gitee.com/openkylin/kylin-kmre-emugl)
- [kylin-kmre-display-control](https://gitee.com/openkylin/kylin-kmre-display-control)
- [kylin-kmre-image-data](https://gitee.com/openkylin/kylin-kmre-image-data)
- [kylin-kmre-image-data-x64](https://gitee.com/openkylin/kylin-kmre-image-data-x64)
- [libkylin-kmre](https://gitee.com/openkylin/libkylin-kmre)
- [kylin-kmre-apk-installer](https://gitee.com/openkylin/kylin-kmre-apk-installer)
- [kylin-kmre-make-image](https://gitee.com/openkylin/kylin-kmre-make-image)
- [kmre](https://gitee.com/openkylin/kmre)
- [kylin-kmre-modules-dkms](https://gitee.com/openkylin/kylin-kmre-modules-dkms)


## SIG邮件列表

aosp@lists.openkylin.top
