#!/bin/bash

# Link to the binary
ln -sf '/opt/kylin-connectivity/tools/kylin-screencast.AppImage' '/usr/bin/kylin-screencast.AppImage'
ln -sf '/opt/kylin-connectivity/tools/anti-control-server' '/usr/bin/anti-control-server'
ln -sf '/opt/kylin-connectivity/tools/anti-control-client' '/usr/bin/anti-control-client'
ln -sf '/opt/kylin-connectivity/connectivity_softbus_server' '/usr/bin/connectivity_softbus_server'
ln -sf '/opt/kylin-connectivity/connectivity_softbus_client' '/usr/bin/connectivity_softbus_client'
mv /opt/kylin-connectivity/tools/libffmpeg /opt/kylin-connectivity/tools/libffmpeg.so
mv /opt/kylin-connectivity/tools/libvulkan /opt/kylin-connectivity/tools/libvulkan.so
mv /opt/kylin-connectivity/tools/libvk_swiftshader /opt/kylin-connectivity/tools/libvk_swiftshader.so
mv /opt/kylin-connectivity/tools/libGLESv2 /opt/kylin-connectivity/tools/libGLESv2.so
mv /opt/kylin-connectivity/tools/libEGL /opt/kylin-connectivity/tools/libEGL.so
mv /opt/kylin-connectivity/tools/swiftshader/libEGL /opt/kylin-connectivity/tools/libEGL.so
mv /opt/kylin-connectivity/tools/swiftshader/libGLESv2 /opt/kylin-connectivity/tools/libGLESv2.so
