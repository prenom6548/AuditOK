cmake_minimum_required(VERSION 3.8)

project(ukui-air-gesture)

add_subdirectory(app)
add_subdirectory(plugin)

include(GNUInstallDirs)
install(FILES ${CMAKE_SOURCE_DIR}/org.ukui.air.gesture.gschema.xml
	DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas
)

# 配置UINPUT规则，用于手势进程启动时创建模拟输入虚拟设备
install(FILES ${CMAKE_SOURCE_DIR}/99-ukui-air-gesture-uinput.rules
	DESTINATION /lib/udev/rules.d
)

# 手势进程启动时创建模拟摄像头设备的脚本
install(PROGRAMS ${CMAKE_SOURCE_DIR}/ensure-vcam.sh
	DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/ukui-air-gesture
	RENAME ensure-vcam
)
# 配置Polkit策略，用于进程内部启动模拟摄像头时授权
install(FILES ${CMAKE_SOURCE_DIR}/org.ukui.airgesture.ensure-vcam.policy
	DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/actions
)
