# WebIDE解决方案

## 准备工作
### 远程机SSH配置
  - 获取远程主机的SSH用户和密码(或设置免秘登录)
  - 检查“远程机”的sshd服务是否正常，“/etc/ssh/sshd_config”中 “AllowTcpForwarding”为yes，如果“AllowTcpForwarding”为no，请修改配置，并重启sshd服务
  - 检查在客户机是否能够通过ssh连接到远程机

## 快速部署
- 客户端（kylin-ide-web-client）下载地址：[远程开发WebIDE客户端软件包地址](https://gitee.com/leven08/kylin-ide-web-client/releases/)
  - 开发机：本地开发机需要安装WebIDE客户端，客户端软件包下载[远程开发WebIDE客户端软件包](https://gitee.com/leven08/kylin-ide-web-client/releases/)，用户根据本地系统架构选择对应的客户端软件包进行下载。
  - 远程开发服务端支持自动下载部署，通常无需手动下载；在某些无网络环境下，可在客户端引导下手动部署。[手动下载地址](https://gitee.com/mcy-kylin/kylin-ide-server/releases/)

## 快速使用
WebIDE，与一般意义的Web IDE概念不同，并不在云端提供一套统一的Web服务，而是结合了浏览器访问和SSH认证，服务端随用部署的一套软件。用法和Remote SSH插件类似，但不需要安装桌面版IDE软件。为用户提供跨机器、跨平台远程编辑、远程编译、远程运行、远程调试等功能，具体介绍及使用方法参见[WebIDE帮助](远程开发WebIDE版指南.md)