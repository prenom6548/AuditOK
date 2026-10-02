# 主题制作工坊软件介绍
## 软件概述  
### 背景与意义  
主题制作工坊是一款专为 openKylin 系统设计的定制主题软件。openKylin  操作系统以其开源、自由的特性受到广泛欢迎，用户系统个性化的需求也非常重要。openKylin系统主题模块致力为整个麒麟操作系统提供优质装扮服务，包括主题、壁纸、光标、高亮色等多种美化类资源。主题制作工坊旨在提供用户一个灵活且直观的工具，满足用户对系统外观的高度自定义需求，帮助用户轻松定制个性化的系统外观。  
    
### 项目描述  
本项目基于openKylin操作系统，制作一款协助用户定制打包个性化主题的UI工具，并帮助用户在使用过程中了解熟悉系统主题模块。  
主题制作工坊采用前后端分离架构：前端负责获取用户信息、协助用户导入素材、协助用户定制个性化主题；后端获取前端提供信息制作相关主题包。该架构提高了开发效率、可维护性和可扩展性，同时也提供了更好的用户体验。  
  
### 使用场景  
主题制作工坊适用于个人用户、企业用户以及 openKylin 爱好者。个人用户可以通过该工具定制个性化的桌面环境，满足自身审美需求。企业用户可以在企业环境中使用该工具，为公司或团队创建统一的视觉风格。openKylin 爱好者则可以通过该工坊深度参与系统定制，探索更多创意和可能性。  
  
### 项目功能
该工具能够合理协助用户制作个性化系统主题，其中包含壁纸、光标、图标、系统样式、开机动画、grub界面、全局主题等。用户根据个人需求选择新建全局、图标、光标、开机动画、grub界面主题。其中全局主题包含图标、光标、开机动画、grub界面、系统样式和壁纸等主题，其他四个类型仅包含各自对应类型主题。  
  
在选择完需要制作的主题后，根据界面提示补充相关素材，点击制作补充用户相关信息后稍等一段时间，在右上角选择的保存路径中会生成对应主题包，点击安装后，在控制面板中即可进行选择。  
  
同时，当用户编辑了部分素材退出后会提示保存历史记录，如果选择保存，会需要补充部分用户信息用于记录，历史记录可保存30日，历史记录会在主界面中显示，点击历史记录即可再次编辑。  
  

### 特点
#### 多元主题修改  
主题制作工坊不仅提供全局主题的修改，还涵盖图标、光标、开机动画和 grub 界面等多个方面的定制。这一全面的主题修改功能使用户能够全方位地打造独一无二的系统外观。  
    
#### 直观用户界面    
软件采用直观、用户友好的界面设计，使得即使是初学者也能轻松上手。各个主题模块的设置清晰可见，用户通过简单的点击和拖拽即可完成各项修改，无需深入了解复杂的技术细节。
  
#### 支持多种格式  
壁纸模块支持 PNG 图片格式和 JPG 图片格式、开机动画模块支持 MP4 文件格式和 PNG 图片格式，为用户提供了更大的自由度。用户可以选择符合自己审美和需求的多种媒体格式。  
  
#### 实时预览即时生效  
用户在自定义主题设置中的每次修改都会立即反映在预览界面上，可以动态地查看和评估他们对主题的各种调整。  
  
  
## 主题制作工坊软件功能概览  
主题制作工坊是专为 openKylin 系统设计的定制主题软件。它提供了修改五个主题的功能，包括全局主题、图标主题、光标主题、开机动画和 grub 主题。接下来将逐个介绍这些功能以及相关功能的使用位置以及要求限制。
  
### 全局主题

  全局主题不仅包含图标、光标、开机动画和grub主题，还包含封面、壁纸、窗口、控件等系统样式设置。打开主题制作工坊，点击制作全局主题模块即可开始全局主题制作。除了全局主题模块主界面中的系统样式，左侧导航栏可以选择对图标、光标、开机动画和grub主题进行制作。

#### 全局主题效果图
  
  系统使用位置：控制面板-个性化/主题，定制控制面板中全局主题的封面预览显示。  
  素材要求：  
   - 尺寸设置：   
     - 宽度： 1640 像素  
     - 高度： 1080 像素   
   - 文件大小限制： 5MB   
   - 文件格式：PNG   

  这样的参数设置将确保用户在定制控制面板显示时，能够上传满足特定尺寸和文件格式要求的图片。确保软件在接收用户上传的图片时，会检查并强制执行这些参数，以保证用户体验的一致性和最佳效果。在控制面板中点击全局主题效果图时可以控制一整套（图标、光标、壁纸、系统样式）的主题切换，全局主题效果图主要提供一个可选视图的作用。  
  
![](img/1.png)  

#### 壁纸定制  
     
  系统使用位置：控制面板-个性化/主题，定制控制面板中全局主题的壁纸。  
  素材要求：
   - 尺寸设置：    
     - 宽度： 3840 像素   
     - 高度： 2160 像素    
   - 文件大小限制： 10MB  
   - 支持的文件格式：JPG  
  
  这样的参数设置确保了用户在选择壁纸时能够使用符合指定尺寸和文件格式的图片。这有助于保持壁纸的质量和兼容性，并确保在不同屏幕上都能够正常显示。在控制面板中点击全局主题效果图时会生效，该壁纸不会额外显示在控制面板-个性化/壁纸中。  
  
![](img/2.png)  
  
![](img/3.png)  
  
![](img/4.png)  
  

#### 窗口外观
     
  系统使用位置：控制面板-个性化/主题：窗口透明度：调整窗口的透明度，实现更加个性化的窗口外观。  
  - 透明度范围：  
   - 最小值： 30  
   - 最大值： 100  
   - 默认透明度：65  
  
  这样的参数设置允许用户在一个 30 到 100 的范围内调整窗口的透明度，100 表示完全不透明。默认透明度设置为 65，以提供一个中等程度的透明效果。在控制面板-个性化/主题中可以选择设置。  
  
![](img/5.png)  
  
![](img/6.png)  
  
![](img/7.png)  
  
  系统使用位置：控制面板-个性化/主题：窗口圆角大小：定制窗口边角的圆角大小，营造不同的窗口风格。  
  - 大小限制：  
    - 大圆角：圆角大小： 12 像素 (px)  
    - 中圆角：圆角大小： 6 像素 (px)  
    - 小圆角：圆角大小： 0 像素 (px)  
    - 默认圆角：中圆角  
  
  这样的参数设置允许用户选择不同的圆角大小，分别为大、中、小三种。在控制面板-个性化/主题中可以选择设置。  
  
![](img/8.png)  
  
![](img/9.png)  
  
![](img/10.png)  
  
  系统使用位置：控制面板-个性化/主题：窗口深浅主题：定制窗口深浅主题  
  - 深主题：  
    - 提供深色调的主题，适合喜欢暗色调的用户。  
  - 浅主题：  
    - 提供浅色调的主题，适合喜欢明亮色调的用户。  
  - 默认主题：浅主题  
  
  用户可以在这两种主题之间进行选择，以满足用户对窗口外观的个性化喜好。在控制面板-个性化/主题中可以选择设置。  
  
![](img/11.png)  
  
![](img/12.png)  
  
#### 控件定制  
  
  系统使用位置：控制面板-个性化/主题：控件圆角大小：调整界面控件的圆角大小，增强整体视觉效果。  
  - 圆角大小范围：  
   - 最小值： 0 像素 (px)  
   - 最大值： 18 像素 (px)  
   - 默认圆角大小：6 像素 (px)  
   
  用户可以在 0 到 18 像素的范围内选择他们喜欢的圆角大小。默认圆角大小设置为 6 像素，以提供一个中等程度的圆角效果。在控制面板-个性化/主题中可以选择设置。    
  
![](img/13.png)  
  
![](img/14.png)  
  
![](img/15.png)  
  
  系统使用位置：控制面板-个性化/主题：强调色：选择强调色，用于突出显示界面中的重要元素。  
  - 可选的强调色：  
    - daybreakBlue、jamPurple、magentaSunRed、sunsetOrange、dustGold、polarGreen  
  - 默认强调色：  
    - daybreakBlue  

  用户可以在这些选项中选择他们喜欢的强调色，以使界面中的重要元素更为突出。在控制面板-个性化/主题中可以选择设置。    
  
![](img/16.png)  
  
![](img/17.png)  
  
![](img/18.png)  
  
![](img/19.png)  
  

### 图标主题

  打开主题制作工坊，点击制作全局主题模块或点击图标主题模块即可开始图标主题制作，点击图标主题模块制作时，无法选择其他模块进行制作。    
  系统使用位置：控制面板-个性化/主题：图标。 

  素材要求：SVG    

应用图标：定制每个应用的图标，支持常见的应用如 biometric-manager、box-manager 等。  
  
![](img/20.png)  
  
  系统图标：修改系统控制面板中的图标，包括 kylin-settings-account、kylin-settings-application 等。  
  
![](img/21.png)  

  文件夹与设备图标：修改桌面与文件管理器内显示的文件夹图标以及设备图标，包括computer、inode-directory等。
![](img/26.png) 

图标规范见附录
  
### 光标主题  
  
  打开主题制作工坊，点击制作全局主题模块或点击光标主题模块即可开始光标主题制作，点击光标主题模块制作时，无法选择其他模块进行制作。   
  系统使用位置：控制面板-个性化/主题：光标。   
  素材要求：SVG  
  光标样式：从预设的光标样式中选择，也支持导入自定义的光标。  
  
![](img/22.png)  

### 开机动画  
  
  打开主题制作工坊，点击制作全局主题模块或点击开机动画模块即可开始开机动画制作，点击开机动画模块制作时，无法选择其他模块进行制作。
  - 支持的格式  

    - MP4 文件格式：允许用户上传和应用 MP4 格式的开机动画

    - 文件大小限制：10MB

    - PNG 图片：支持将一张 PNG 图片作为开机动画，为用户提供更多创意空间。

    - 文件大小限制：20KB

  
![](img/23.png)

### Grub 主题
  
  打开主题制作工坊，点击制作全局主题模块或点击Grub主题模块即可开始开机动画制作，点击Grub主题模块制作时，无法选择其他模块进行制作。
  上传自定义图片：选择并上传用户喜欢的图片作为 Grub 界面的背景。

  - 尺寸设置：

    -  宽度： 3840 像素

    - 高度： 2160 像素

    - 文件大小限制： 10MB

    - 支持的文件格式：PNG
  
![](img/24.png)
  
![](img/25.png)
  
  
### 打包信息收集
  
  制作主题包前后、新建模块修改退出后、历史记录修改退出后会弹出相关信息收集页面  
  输入限制:  
    - 主题中文非必须内容，不可输入非中文内容、输入内容不可大于15个字节；
    - 主题英文为必须内容，不可输入非英文内容、不可输入英文大写、输入内容不可大于15个字节小于2个字节、不可输入系统中已有软件包名称；
    - 用户名为必须内容，不可输入非英文、输入内容不可大于15个字节；
    - 邮箱为必须内容，不可输入非英文，输入内容需要遵守邮箱规则不能大于35个字节不可小于6个字节；
  

    
通过主题制作工坊，用户能够将 openKylin 操作系统打造成符合自己独特品味和需求的个性化系统，从而提升整体的用户体验。  

## 关于上传
  当完成一个主题的制作后，点击上传主题平台按钮
  
  ![](img/27.png)

 1、登录应用软件分发平台

  ![](img/28.png)

 2、点击软件上架业务

  ![](img/29.png)

 3、填写平台相关信息

  ![](img/30.png)

 4、上传主题包，注意：软件源选择“openKylin”

  ![](img/31.png)

 5、等待人工审核

 审核通过后就可以在软件商店内找到自己制作的主题包！
## 附录
### 应用图标规范名
| 图标 | 图标名 | 
| ---- | ---- | 
|  <img src="resource/appicons/biometric-manager.png" width="48" height="48"> | biometric-manager   | 
| <img src="resource/appicons/box-manager.png" width="48" height="48"> | box-manager   | 
| <img src="resource/appicons/burner.png" width="48" height="48">  |  burner  | 
| <img src="resource/appicons/indicator-china-weather.png" width="48" height="48">  | indicator-china-weather   | 
| <img src="resource/appicons/ksc-defender.png" width="48" height="48">  | ksc-defender   | 
| <img src="resource/appicons/kylin-alarm-clock.png" width="48" height="48">  | kylin-alarm-clock  | 
| <img src="resource/appicons/kylin-assistant.png" width="48" height="48">  | kylin-assistant   | 
| <img src="resource/appicons/kylin-calculator.png" width="48" height="48">  | kylin-calculator   | 
| <img src="resource/appicons/kylin-camera.png" width="48" height="48">  | kylin-camera   | 
| <img src="resource/appicons/kylin-connectivity.png" width="48" height="48">  | kylin-connectivity   | 
| <img src="resource/appicons/kylin-font-viewer.png" width="48" height="48">  | kylin-font-viewer   | 
| <img src="resource/appicons/kylin-installer.png" width="48" height="48">  | kylin-installer   | 
| <img src="resource/appicons/kylin-ipmsg.png" width="48" height="48">  | kylin-ipmsg   | 
| <img src="resource/appicons/kylin-log-viewer.png" width="48" height="48">  | kylin-log-viewer   | 
| <img src="resource/appicons/kylin-music.png" width="48" height="48">  | kylin-music   | 
| <img src="resource/appicons/kylin-new-function-introduction.png" width="48" height="48">  | kylin-new-function-introduction   | 
| <img src="resource/appicons/kylin-notebook.png" width="48" height="48">  | kylin-notebook   | 
| <img src="resource/appicons/kylin-os-manager.png" width="48" height="48">  | kylin-os-manager   | 
| <img src="resource/appicons/kylin-photo-viewer.png" width="48" height="48">  | kylin-photo-viewer   | 
| <img src="resource/appicons/kylin-printer.png" width="48" height="48">  | kylin-printer   | 
| <img src="resource/appicons/kylin-recorder.png" width="48" height="48">  | kylin-recorder   | 
| <img src="resource/appicons/kylin-screenshot.png" width="48" height="48">  | kylin-screenshot   | 
| <img src="resource/appicons/kylin-service-support.png" width="48" height="48">  | kylin-service-support   | 
| <img src="resource/appicons/kylin-software-center.png" width="48" height="48">  | kylin-software-center   | 
| <img src="resource/appicons/kylin-usb-creator.png" width="48" height="48">  | kylin-usb-creator   | 
| <img src="resource/appicons/kylin-user-guide.png" width="48" height="48">  | kylin-user-guide   | 
| <img src="resource/appicons/kylin-video.png" width="48" height="48">  | kylin-video   | 
| <img src="resource/appicons/system-file-manager.png" width="48" height="48">  | system-file-manager   | 
| <img src="resource/appicons/ukui-control-center.png" width="48" height="48">  | ukui-control-center   | 
| <img src="resource/appicons/ukui-system-monitor.png" width="48" height="48">  | ukui-system-monitor   | 
| <img src="resource/appicons/utilities-terminal.png" width="48" height="48">  | utilities-terminal   | 
| <img src="resource/appicons/yhkylin-backup-tools.png" width="48" height="48">  | yhkylin-backup-tools   | 
### 系统图标规范名
| 图标 | 图标名 | 
| ---- | ---- | 
| <img src="resource/systemicons/kylin-settings-account.png" width="48" height="48"> | kylin-settings-account   | 
| <img src="resource/systemicons/kylin-settings-application.png" width="48" height="48"> | kylin-settings-application   | 
| <img src="resource/systemicons/kylin-settings-commoninfo.png" width="48" height="48"> | kylin-settings-commoninfo   | 
| <img src="resource/systemicons/kylin-settings-datetime.png" width="48" height="48"> | kylin-settings-datetime   | 
| <img src="resource/systemicons/kylin-settings-devices.png" width="48" height="48"> | kylin-settings-devices   | 
| <img src="resource/systemicons/kylin-settings-network.png" width="48" height="48"> | kylin-settings-network   | 
| <img src="resource/systemicons/kylin-settings-personalized.png" width="48" height="48"> | kylin-settings-personalized   | 
| <img src="resource/systemicons/kylin-settings-search.png" width="48" height="48"> | kylin-settings-search   |
| <img src="resource/systemicons/kylin-settings-security.png" width="48" height="48"> | kylin-settings-security   | 
| <img src="resource/systemicons/kylin-settings-system.png" width="48" height="48"> | kylin-settings-system   | 
| <img src="resource/systemicons/kylin-settings-update.png" width="48" height="48"> | kylin-settings-update   | 
### 文件夹图标规范名
| 图标 | 图标名 | 
| ---- | ---- | 
| <img src="resource/places/document-open-recent.png" width="48" height="48"> | document-open-recent   |  
| <img src="resource/places/folder-documents.png" width="48" height="48"> | folder-documents   |  
| <img src="resource/places/folder-download.png" width="48" height="48"> | folder-download   |  
| <img src="resource/places/folder-dropbox.png" width="48" height="48"> | folder-dropbox   |  
| <img src="resource/places/folder-music.png" width="48" height="48"> | folder-music   |  
| <img src="resource/places/folder-open.png" width="48" height="48"> | folder-open   |  
| <img src="resource/places/folder-pictures.png" width="48" height="48"> | folder-pictures   |  
| <img src="resource/places/folder-publicshare.png" width="48" height="48"> | folder-publicshare   |  
| <img src="resource/places/folder-templates.png" width="48" height="48"> | folder-templates   |  
| <img src="resource/places/folder-videos.png" width="48" height="48"> | folder-videos   |  
| <img src="resource/places/inode-directory.png" width="48" height="48"> | inode-directory   |  
| <img src="resource/places/user-desktop.png" width="48" height="48"> | user-desktop   |  
| <img src="resource/places/user-home.png" width="48" height="48"> | user-home   |  
| <img src="resource/places/user-trash-full.png" width="48" height="48"> | user-trash-full   |  
| <img src="resource/places/user-trash.png" width="48" height="48"> | user-trash   |  
### 文件夹图标规范名
| 图标 | 图标名 | 
| ---- | ---- | 
| <img src="resource/devices/computer.png" width="48" height="48"> | computer | 
| <img src="resource/devices/cupsprinter.png" width="48" height="48"> | cupsprinter |
| <img src="resource/devices/drive-harddisk.png" width="48" height="48"> | drive-harddisk |  
| <img src="resource/devices/drive-removable-media-usb.png" width="48" height="48"> | drive-removable-media-usb | 
| <img src="resource/devices/input-keyboard.png" width="48" height="48"> | input-keyboard | 
| <img src="resource/devices/input-mouse.png" width="48" height="48"> | input-mouse | 
| <img src="resource/devices/scanner.png" width="48" height="48"> | scanner | 
### 光标图标规范名
| 图标 | 图标名 | 
| ---- | ---- | 
| <img src="resource/cursor/left_ptr.png" width="48" height="48"> | left_ptr | 
| <img src="resource/cursor/dnd-none.png" width="48" height="48"> | dnd-none | 
| <img src="resource/cursor/hand1.png" width="48" height="48"> | hand1 | 
| <img src="resource/cursor/hand2.png" width="48" height="48"> | hand2 | 
| <img src="resource/cursor/right_ptr.png" width="48" height="48"> | right_ptr | 
| <img src="resource/cursor/dnd-ask.png" width="48" height="48"> | dnd-ask | 
| <img src="resource/cursor/dnd-copy.png" width="48" height="48"> | dnd-copy | 
| <img src="resource/cursor/dnd-link.png" width="48" height="48"> | dnd-link | 
| <img src="resource/cursor/h_double_arrow.png" width="48" height="48"> | h_double_arrow | 
| <img src="resource/cursor/v_double_arrow.png" width="48" height="48"> | v_double_arrow | 
| <img src="resource/cursor/ul_angle.png" width="48" height="48"> | ul_angle | 
| <img src="resource/cursor/ur_angle.png" width="48" height="48"> | ur_angle | 
| <img src="resource/cursor/left_tee.png" width="48" height="48"> | left_tee |  
| <img src="resource/cursor/bottom_tee.png" width="48" height="48"> | bottom_tee | 
| <img src="resource/cursor/top_tee.png" width="48" height="48"> | top_tee | 
| <img src="resource/cursor/right_tee.png" width="48" height="48"> | right_tee | 
| <img src="resource/cursor/cross.png" width="48" height="48"> | cross | 
| <img src="resource/cursor/tcross.png" width="48" height="48"> | tcross | 
| <img src="resource/cursor/sb_up_arrow.png" width="48" height="48"> | sb_up_arrow | 
| <img src="resource/cursor/sb_left_arrow.png" width="48" height="48"> | sb_left_arrow | 
| <img src="resource/cursor/sb_down_arrow.png" width="48" height="48"> | sb_down_arrow | 
| <img src="resource/cursor/sb_right_arrow.png" width="48" height="48"> | sb_right_arrow | 
| <img src="resource/cursor/all-scroll.png" width="48" height="48"> | all-scroll | 
| <img src="resource/cursor/sb_h_double_arrow.png" width="48" height="48"> | sb_h_double_arrow | 
| <img src="resource/cursor/sb_v_double_arrow.png" width="48" height="48"> | sb_v_double_arrow | 
| <img src="resource/cursor/vertical_text.png" width="48" height="48"> | vertical_text | 
| <img src="resource/cursor/xterm.png" width="48" height="48"> | xterm | 
| <img src="resource/cursor/zoom-in.png" width="48" height="48"> | zoom-in | 
| <img src="resource/cursor/zoom-out.png" width="48" height="48"> | zoom-out | 
### 等待光标图标规范名
| 图标 | 图标名 | 
| ---- | ---- | 
| <img src="resource/time-cursor/left_ptr_watch_1.png" width="48" height="48"> | left_ptr_watch_1 |
| <img src="resource/time-cursor/left_ptr_watch_2.png" width="48" height="48"> | left_ptr_watch_2 |
| <img src="resource/time-cursor/left_ptr_watch_3.png" width="48" height="48"> | left_ptr_watch_3 |
| <img src="resource/time-cursor/left_ptr_watch_4.png" width="48" height="48"> | left_ptr_watch_4 |
| <img src="resource/time-cursor/left_ptr_watch_5.png" width="48" height="48"> | left_ptr_watch_5 |
| <img src="resource/time-cursor/watch_1.png" width="48" height="48"> | watch_1 |
| <img src="resource/time-cursor/watch_2.png" width="48" height="48"> | watch_2 |
| <img src="resource/time-cursor/watch_3.png" width="48" height="48"> | watch_3 |
| <img src="resource/time-cursor/watch_4.png" width="48" height="48"> | watch_4 |
| <img src="resource/time-cursor/watch_5.png" width="48" height="48"> | watch_5 |
| <img src="resource/time-cursor/watch_6.png" width="48" height="48"> | watch_6 |
| <img src="resource/time-cursor/watch_7.png" width="48" height="48"> | watch_7 |
| <img src="resource/time-cursor/watch_8.png" width="48" height="48"> | watch_8 |
| <img src="resource/time-cursor/watch_9.png" width="48" height="48"> | watch_9 |
| <img src="resource/time-cursor/watch_10.png" width="48" height="48"> | watch_10 |
