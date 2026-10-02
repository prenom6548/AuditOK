# 安装 kaiming

## 基础环境要求

使用 ``uname -r`` 查看内核版本：

```bash
$ uname -r
6.2.0-37-generic
```

内核版本要求 >= 5.4.18

## kaiming deb 安装教程

openkylin 已预装 kaiming 环境。

也可以在openkylin的 [kylin-pty ppa](https://build.openkylin.top/~kylinsoft/+archive/openkylin/kylin-pty/+packages?field.name_filter=&field.status_filter=published&field.series_filter=) 上直接下载 kaiming deb 包，用以下命令安装：

```bash
$ sudo apt-get install \
    kaiming_*_amd64.deb \
    libkaiming-dev_*_amd64.deb

$ reboot
```
