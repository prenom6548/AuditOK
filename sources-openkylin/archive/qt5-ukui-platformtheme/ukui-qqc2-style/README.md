# kylin-qqc2-style

    kylin qquick control 2


## Description

kylin-qqc2-style 是参考 qqc2-desktop-style 实现的一个 qml 主题，主要包括以下几部分：
> 1. org.kylin.style: 提供 qml style org.kylin.style，可以通过 QQuickStyle::setStyle 或其他方式设置 qml 应用使用次样式
> 2. qqc2-style-plugin: 提供一个自定义的 QQuickItem 供 org.kylin.style 使用

## Depends

    qtbase
    qquick
    kirigami2-dev
    KIconLoader
    KColorScheme
    KSharedConfig
    KConfigGroup

## build & install

    mkdir build
    cd build
    qmake ..
    make
    sudo make install
