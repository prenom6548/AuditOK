# 第三方图标规范：
根据freedesktop中描述，
> In order to have a place for third party applications to install their icons there should always exist a theme called "hicolor".

为了给第三方应用程序提供一个安装图标的位置，应该始终存在一个名为“hicolor”的主题。

根据freedesktop的继承关系中描述，
> If no theme is specified implementations are required to add the "hicolor" theme to the inheritance tree.

如果没有指定主题，则要求实现者将 "hicolor "主题添加到继承树中。当我们的自研应用存在于其他桌面环境时，应作为第三方应用>，将我们的图标应该放在hicolor主题里。

> So, you're an application author, and want to install application icons so that they work in the KDE and Gnome menus. Minimally you should install a 48x48 icon in the hicolor theme. This means installing a PNG file in $prefix/share/icons/hicolor/48x48/apps.

作为应用程序的开发者，为了在当前桌面环境菜单中正常显示，我们至少需要在hicolor中安装48x48尺寸的图标。$prefix/**share/icons/hicolor/48x48/apps**。为了更好的将图标显示（规避显示模糊的风险）以及表达我们的设计语言，请将svg图标，安装在$prefix/**share/icons/hicolor/scalable/** 下。程序内所需要的其他图标以及线性图标，同样的，根据设计提供的图标尺寸放在hicolor中
相应的尺寸目录下。

freedesktop规范原文链接：https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#icon_lookup
