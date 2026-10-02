#  <center> KMRE

## <font face="Courier New">引言
<font face="Courier New">麒麟移动运行环境，简称KMRE，旨在将丰富的Android应用生态迁移到Linux桌面操作系统上，提供桌面化的操作体验。本项目为麒麟软件维护的对KMRE支持的仓库，主要包含了Linux侧和Android侧的修改支持，其余组件直接从上游AOSP仓库拉取。

---
<BR/>


## <font face="Courier New">一.搭建Android开发环境

#### <font face="Courier New">1.1 硬件配置要求
<font face="Courier New">多核X86主机，内存最低配置要求16G，硬盘大小最低配置要求512G。追求编译速度，硬件配置可以翻倍。

#### <font face="Courier New">1.2 操作系统要求
<font face="Courier New">Ubuntu 18.04、20.04或22.04等，本项目推荐使用Ubuntu 20.04。

#### <font face="Courier New">1.3 编译环境安装和配置
###### <font face="Courier New">1.3.1 工具包安装

```bash
  sudo apt-get install git git-lfs curl python zip
```

###### <font face="Courier New">1.3.2 编译依赖包安装

```bash
  sudo apt install libgl1-mesa-dev g++-multilib flex bison gperf build-essential tofrodos python-markdown libxml2-utils xsltproc dpkg-dev libsdl1.2-dev gnupg flex bison gperf build-essential zlib1g-dev gcc-multilib g++-multilib libc6-dev-i386 lib32ncurses-dev x11proto-core-dev libx11-dev libgl1-mesa-dev libxml2-utils xsltproc unzip m4 lib32z1-dev ccache libssl-dev device-tree-compiler liblz4-tool libncurses5
```

###### <font face="Courier New">1.3.3 配置PATH环境变量

```bash
  mkdir ~/bin
  echo "PATH=~/bin:\$PATH" >> ~/.bashrc
  source ~/.bashrc
```

###### <font face="Courier New">1.3.4 下载repo脚本文件

```bash
  curl https://mirrors.tuna.tsinghua.edu.cn/git/git-repo > ~/bin/repo
  chmod a+x ~/bin/repo
  vim ~/.bashrc
  export REPO_URL='https://mirrors.tuna.tsinghua.edu.cn/git/git-repo/'
  source ~/.bashrc
```

###### <font face="Courier New">1.3.5 修改默认Python版本
  <font face="Courier New">将默认Python版本指定位Python2 ，执行如下命令

```bash
   ln -s /usr/bin/python2 /usr/bin/python  
```

###### <font face="Courier New">1.3.6 配置git账户

```bash
  git config --global user.email "xxxxxxx@kylinos.com"
  git config --global user.name "xxxxx"
```

---

<BR/>


## <font face="Courier New">二.构建Android镜像包
#### <font face="Courier New">2.1 AOSP源代码下载

```bash
  mkdir kmre-aosp-src
  cd kmre-aosp-src
  repo init -u https://gitee.com/openkylin/platform_manifest.git -b kmre-opensource-devel
  （或：repo init -u git@gitee.com:openkylin/platform_manifest.git -b kmre-opensource-devel ）
  repo sync -c --no-repo-verify --no-tags --no-clone-bundle --force-sync -j8
```


<font face="Courier New">下载完成后的目录结构如下:（或者tree -L 1 kmre-aosp-src 查看）
```bash
  kmre-aosp-src$ ls
  Android.bp  bootstrap.bash  dalvik       frameworks  libcore          pdk               system
  art         build           development  hardware    libnativehelper  platform_testing  test
  bionic      compatibility   device       kernel      Makefile         prebuilts         toolchain
  bootable    cts             external     kmre        packages         sdk               tools
```

#### <font face="Courier New">2.2 lunch编译项目

```bash
  kmre-aosp-src$ source build/envsetup.sh && lunch kmre_x86_64-user

  ============================================
  PLATFORM_VERSION_CODENAME=REL
  PLATFORM_VERSION=11
  TARGET_PRODUCT=kmre_x86_64
  TARGET_BUILD_VARIANT=user
  TARGET_BUILD_TYPE=release
  TARGET_ARCH=x86_64
  TARGET_ARCH_VARIANT=x86_64
  TARGET_2ND_ARCH=x86
  TARGET_2ND_ARCH_VARIANT=x86_64
  HOST_ARCH=x86_64
  HOST_2ND_ARCH=x86
  HOST_OS=linux
  HOST_OS_EXTRA=Linux-5.15.0-105-generic-x86_64-Ubuntu-20.04.4-LTS
  HOST_CROSS_OS=windows
  HOST_CROSS_ARCH=x86
  HOST_CROSS_2ND_ARCH=x86_64
  HOST_BUILD_TYPE=release
  BUILD_ID=RP1A.201005.006
  OUT_DIR=out
  PRODUCT_SOONG_NAMESPACES=device/generic/goldfish device/generic/goldfish-opengl hardware/google/camera hardware/google/camera/devices/EmulatedCamera device/generic/goldfish device/generic/goldfish-opengl
  ============================================
```
如果编译ARM版本，则使用`lunch kmre_arm64-user`

#### <font face="Courier New">2.2 安卓源码编译
```bash
  kmre-aosp-src$ make img -j12
```

整个编译过程耗时较长，请耐心等待。编译成功后，可以在 out/target/product/kmre_x86_64 目录下查看到编译生成的相关镜像文件system.sfs。如果是ARM平台，则镜像路径为 out/target/product/kmre_arm64/system.sfs。如下：

```bash
kmre-aosp-src$ cd out/target/product/kmre_x86_64/
kmre-aosp-src/out/target/product/kmre_x86_64$ ls
android-info.txt       build_thumbprint.txt  gen                        obj                       symbols
apex                   clean_steps.mk        installed-files.json       obj_x86                   system
appcompat              data                  installed-files-root.json  previous_build_config.mk  system.sfs
build_fingerprint.txt  debug_ramdisk         installed-files-root.txt   recovery                  testcases
build.prop             fake_packages         installed-files.txt        root
```
----

<BR/>


## <font face="Courier New">三. 构建Linux侧KMRE安装包
以下以x86_64平台为例进行说明，推荐使用银河麒麟桌面操作系统V10 SP1、Ubuntu 20.04和Ubuntu 22.04。x86_64平台上编译生成的deb包只能安装到x86_64平台上，如果需要arm64平台上的deb包，则需要在arm64平台上对源码进行编译。

####  <font face="Courier New">3.1 下载Linux侧KMRE各模块源码包

```bash
  sudo apt-get update
  sudo apt install git devscripts
  mkdir kmre-host-src
  cd kmre-host-src
  git clone https://gitee.com/openkylin/kylin-kmre-manager.git
  git clone https://gitee.com/openkylin/kylin-kmre-window.git
  git clone https://gitee.com/openkylin/kylin-kmre-daemon.git
  git clone https://gitee.com/openkylin/kylin-kmre-emugl.git
  git clone https://gitee.com/openkylin/kylin-kmre-display-control.git
  git clone https://gitee.com/openkylin/kylin-kmre-image-data-x64.git
  git clone https://gitee.com/openkylin/libkylin-kmre.git
  git clone https://gitee.com/openkylin/kylin-kmre-apk-installer.git
  git clone https://gitee.com/openkylin/kylin-kmre-make-image.git
  git clone https://gitee.com/openkylin/kylin-kmre-modules-dkms.git （该组件仅Ubuntu系统下需要，Ubuntu20.04版本下使用kmre-kernel-modules-ubuntu-20.04分支，Ubuntu22.04版本下使用kmre-kernel-modules-ubuntu-22.04分支）
  ......
```

<font face="Courier New">代码clone完成后，用命令查看会有如下目录:

```bash
  lixiang@kylin-pc3:/build1/lixiang$$ tree  -L 1 kmre-host-src/
  kmre-host-src/
  ├── kylin-kmre-apk-installer
  ├── kylin-kmre-daemon
  ├── kylin-kmre-display-control
  ├── kylin-kmre-emugl
  ├── kylin-kmre-image-data-x64
  ├── kylin-kmre-make-image
  ├── kylin-kmre-manager
  ├── kylin-kmre-window
  └── libkylin-kmre

  9 directories, 0 files
```

####  <font face="Courier New">3.2 生成Linux侧KMRE各模块deb包

先解决编译依赖问题，通过如下命令可以查询到对应源码包在系统上缺失的编译依赖包：
```bash
  kmre-host-src/libkylin-kmre$ debuild -S
   dpkg-buildpackage -us -uc -ui -S
  dpkg-buildpackage: info: 源码包 libkylin-kmre
  dpkg-buildpackage: info: 源码版本 1.3.4
  dpkg-buildpackage: info: source distribution v101
  dpkg-buildpackage: info: 源码修改者 lixiang <lixiang@kylinos.cn>
   dpkg-source --before-build .
  dpkg-checkbuilddeps: 错误: Unmet build dependencies: debhelper g++ cmake libprotobuf-dev
  dpkg-buildpackage: 警告: build dependencies/conflicts unsatisfied; aborting
  dpkg-buildpackage: 警告: (使用 -d 参数来忽略)
  debuild: fatal error at line 1182:
  dpkg-buildpackage -us -uc -ui -S failed
```

根据缺失依赖信息，解决编译依赖问题：
```bash
  sudo apt install debhelper g++ cmake libprotobuf-dev
```

执行编译命令：
```bash
  kmre-host-src/libkylin-kmre$ debuild -j6
    dpkg-buildpackage -us -uc -ui -j6
  dpkg-buildpackage: info: 源码包 libkylin-kmre
  dpkg-buildpackage: info: 源码版本 3.0.0.1
  dpkg-buildpackage: info: source distribution v101
  dpkg-buildpackage: info: 源码修改者 lixiang <lixiang@kylinos.cn>
  dpkg-source --before-build .
  dpkg-buildpackage: info: 主机架构 amd64
  fakeroot debian/rules clean
  dh  clean --parallel
    dh_auto_clean -O--parallel
    make -j6 clean
  make[1]: 进入目录“/build1/lixiang/kmre-host-src/libkylin-kmre”
  rm -f *.o
  rm -f KmreCore.pb.*
  rm -f libkmre.so
  make[1]: 离开目录“/build1/lixiang/kmre-host-src/libkylin-kmre”
    dh_clean -O--parallel
  dpkg-source -b .
  dpkg-source: info: using source format '3.0 (native)'
  dpkg-source: info: building libkylin-kmre in libkylin-kmre_3.0.0.1.tar.xz
  dpkg-source: info: building libkylin-kmre in libkylin-kmre_3.0.0.1.dsc
  debian/rules build
  dh  build --parallel
    dh_update_autotools_config -O--parallel
    dh_auto_configure -O--parallel
    dh_auto_build -O--parallel
    make -j6
  make[1]: 进入目录“/build1/lixiang/kmre-host-src/libkylin-kmre”
  protoc -I=./ --cpp_out=./ KmreCore.proto
  g++ -fPIC -shared main.cc kmre_socket.cc KmreCore.pb.cc -std=c++14 -fpermissive -g -o libkmre.so `pkg-config --cflags --libs protobuf` -ldl
  make[1]: 离开目录“/build1/lixiang/kmre-host-src/libkylin-kmre”
    dh_auto_test -O--parallel
  fakeroot debian/rules binary
  dh  binary --parallel
    dh_testroot -O--parallel
    dh_prep -O--parallel
    dh_auto_install -O--parallel
    dh_install -O--parallel
    dh_installdocs -O--parallel
    dh_installchangelogs -O--parallel
    dh_installinit -O--parallel
    dh_perl -O--parallel
    dh_link -O--parallel
    dh_strip_nondeterminism -O--parallel
    dh_compress -O--parallel
    dh_fixperms -O--parallel
    dh_missing -O--parallel
    dh_strip -O--parallel
    dh_makeshlibs -O--parallel
    dh_shlibdeps -O--parallel
    dh_installdeb -O--parallel
    dh_gencontrol -O--parallel
    dh_md5sums -O--parallel
    dh_builddeb -O--parallel
  dpkg-deb: 正在 '../libkylin-kmre_3.0.0.1_amd64.deb' 中构建软件包 'libkylin-kmre'。
  dpkg-deb: 正在 'debian/.debhelper/scratch-space/build-libkylin-kmre/libkylin-kmre-dbgsym_3.0.0.1_amd64.deb' 中构建软件包 'libkylin-kmre-dbgsym'。
    Renaming libkylin-kmre-dbgsym_3.0.0.1_amd64.deb to libkylin-kmre-dbgsym_3.0.0.1_amd64.ddeb
  dpkg-genbuildinfo
  dpkg-genchanges  >../libkylin-kmre_3.0.0.1_amd64.changes
  dpkg-genchanges: info: 上传数据中包含完整的原始代码
  dpkg-source --after-build .
  dpkg-buildpackage: info: full upload; Debian-native package (full source is included)
  Now signing changes and any dsc files...
  signfile dsc libkylin-kmre_3.0.0.1.dsc B6C2001D583687B2AAFF54A287F699B85704752E

  fixup_buildinfo libkylin-kmre_3.0.0.1.dsc libkylin-kmre_3.0.0.1_amd64.buildinfo
  signfile buildinfo libkylin-kmre_3.0.0.1_amd64.buildinfo B6C2001D583687B2AAFF54A287F699B85704752E

  fixup_changes dsc libkylin-kmre_3.0.0.1.dsc libkylin-kmre_3.0.0.1_amd64.changes
  fixup_changes buildinfo libkylin-kmre_3.0.0.1_amd64.buildinfo libkylin-kmre_3.0.0.1_amd64.changes
  signfile changes libkylin-kmre_3.0.0.1_amd64.changes B6C2001D583687B2AAFF54A287F699B85704752E

  Successfully signed dsc, buildinfo, changes files
```

执行编译命令编译完成后，在上一级目录生成对应的deb包。所有源码都以上述方式解决依赖和编程生成deb包。


####  <font face="Courier New">3.3 将Android镜像包转换成deb包
```bash
kmre-host-src$ cp ${path_to_android_image} ~/system.sfs
上一步将之前编译生成的Android镜像文件"system.sfs"拷贝到当前用户$HOME路径下。

kmre-host-src$ cd kylin-kmre-make-image

kylin-kmre-make-image$ sudo make
make完成后会在当前目录下生成kmre3_'tag_date-time'.tar镜像压缩包，例如：kmre3_v3.0-240423.10_2024.04.23-19.11.tar，tag为v3.0-240423.10，date为2024.04.23，time为19.11 。

kylin-kmre-make-image$ cp kmre3_*.tar ../kylin-kmre-image-data-x86/data/amd64/kmre-container-image.tar
注：如果是arm64平台，则使用以下命令进行拷贝：
kylin-kmre-make-image$ cp kmre3_*.tar ../kylin-kmre-image-data/data/arm64/kmre-container-image.tar
上一步命令将生成的kmre3_'tag_date-time'.tar镜像压缩包拷贝到deb制作工具路径下并重命名为“kmre-container-image.tar”。

kylin-kmre-make-image$ cd ../kylin-kmre-image-data-x86 （arm64平台为kylin-kmre-image-data）

kylin-kmre-image-data-x86$ vi data/kmre.conf
[image]‘’；
repo=kmre3
tag=v3.0-240423.10
上一步更新“data/kmre.conf”配置文件中的tag标签，例如：tag=v3.0-240423.10

kylin-kmre-image-data-x86$ dch -n （或者直接手动修改debian/changelog文件）
上一步更新版本号，建议把对应的tag去除v做成deb的版本号，如kylin-kmre-image-data-x64 (3.0-231108.10) xxx。

kylin-kmre-image-data-x86$ debuild -j6
执行完毕后会在上一级目录下生成对应的Android镜像的deb包。
```

-----

<BR/>

##  <font face="Courier New">四.安装和启动KMRE
####  <font face="Courier New">4.1 安装deb包

```bash
  kmre-host-src$ sudo dpkg -i kylin-kmre-daemon_3.0.0.0-0k0.4_amd64.deb kylin-kmre-display-control_3.0.0.0-0k0.1_amd64.deb kylin-kmre-image-data_3.0-231108.10_amd64.deb kylin-kmre-manager_3.0.0.0-0k0.7_amd64.deb kylin-kmre-window_3.0.0.0-0k1.0_amd64.deb libkylin-kmre_3.0.0.0-0k0.1_amd64.deb libkylin-kmre-emugl_3.0.0.0-0k0.1_amd64.deb
  注：Ubuntu系统下还需额外安装kylin-kmre-modules-dkms_3.0.0.0-0k0.1_amd64.deb这个包
```

####  <font face="Courier New">4.2 重启系统
保存用户文件后，再重启系统。

```bash
  kmre-host-src$ sudo reboot
```

####  <font face="Courier New">4.3 启动KMRE环境
保存用户文件后，再重启系统。

```bash
  startapp com.android.settings
```

如果KMRE环境正常，上述命令将会正常弹出安装的设置应用界面，后续可以通过麒麟软件商店安装和更新应用，或在本地双击apk包安装应用。

-----

<BR/>
