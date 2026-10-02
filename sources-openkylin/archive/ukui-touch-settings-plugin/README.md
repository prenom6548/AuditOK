# ukui-touch-settings-plugin
    ukui-touch-settings-plugin 是 ukui-control 的一个触摸相关设备的插件集合
    其中包括了三个插件：
        touchscreen-settings
        touchpen-settings
        touchpad-settings

## build depend
    qtbase5-dev
    libgsettings-qt-dev,
    libukcc-dev,
    libxi-dev,
    libqt5x11extras5-dev,
    qttools5-dev-tools

## make
    mkdir build
    cd build
    qmake ..
    make
    sudo make install

## CommonInterface

由 libukcc 提供的基类，继承这个基类并实现其规定的接口，再将编译出的 *.so 文件安装到对应位置即可显示在控制面板中

## touchscreen-settings

touchscreen-settings 是负责展示触摸屏手势的控制面板插件，可以展示不同的手势对应的快捷操作

## touchpen-settings

touchpen-settings 是负责展示手写笔信息的控制面板插件，未来增加了手写笔配置也会加在这个插件上
