# 环境变量

kylin-wlcom合成器支持多个环境变量，可对内部流程进行修改。

## 日志

* *KYWC_LOG_LEVEL*：设置日志等级，可选值为：DEBUG|INFO|WARN|ERROR|FATAL|SILENT

## 显示

* *KYWC_BACKEND*：设置显示后端，可选值为：fbdev
* *KYWC_FB_DEVICES*：设置FBDEV后端输出节点，如/dev/fb0:/dev/fb1
* *KYWC_DRM_DEVICES*：设置DRM后端输出节点，如/dev/dri/card1:/dev/dri/card0，第一个设备为默认渲染设备
* *KYWC_DRM_NO_ATOMIC*：设置DRM后端禁用原子提交，默认关闭
* *KYWC_DRM_NO_MODIFIERS*：设置DRM后端禁用modifiers，默认关闭
* *KYWC_SOFTWARE_COLOR*：设置DRM后端使用软件色彩（亮度、色温、色彩管理）,默认关闭
* *KYWC_OUTPUT_FILL_WALLPAPER*：在合成器启动和模式切换的第一帧填充壁纸，默认关闭

## 渲染

* *KYWC_RENDERER*：设置渲染后端，可选值为：gl，gles2，pixman，vulkan
* *KYWC_EGL_NO_MODIFIERS*：关闭egl中的modifiers支持
* *KYWC_RENDERER_ALLOW_SOFTWARE*：允许使用软件渲染器，如llvmpipe
* *KYWC_ENABLE_TIMELINE*：开启drm_syncobj_v1协议，默认关闭，置1生效

## 输出

* *KYWC_DISABLE_DIRECT_SCANOUT*：强制去使能全屏应用直接输出，默认关闭
* *KYWC_USE_LAYOUT_MANAGER*：使用合成器内部的屏幕配置管理，默认关闭

## 应用

这部分环境变量供应用程序使用，例如vkcube，glmark2，glxgears，glxinfo等

* *DRI_PRIME*：为应用指定渲染设备，值为pci-area_bus_dev_func，如pci-0000_01_00_0
* *__VK_LAYER_NV_optimus*：VULKAN的加速卡限制为NVIDIA或者非NVIDIA，可选值为：NVIDIA_only|non_NVIDIA_only
* *__NV_PRIME_RENDER_OFFLOAD*：VULKAN或者EGL的加速卡是否首选NVIDIA GPU，可选值为：0|1
* *__GLX_VENDOR_LIBRARY_NAME*：指定GLX的渲染库，如nvidia

## 其他

* *XCURSOR_THEME*：设置光标主题 
* *XCURSOR_SIZE*：设置光标大小
* *KYWC_FORCE_QUIRKS*：设置wlcom定制行为，输入要求十进制，如2，表示强制软件鼠标
