# Quick reference

- The official cubefs docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# cubefs(cubefs Storage Stack) | openKylin
Current cubefs(cubefs Storage Stack) docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

CubeFS is an open-source cloud-native distributed file & object storage system, hosted by the Cloud Native Computing Foundation (CNCF) as a graduated project.

Learn more about cubefs, please visit [https://www.cubefs.io/](https://www.cubefs.io/)⁠.

# Supported tags and respective Dockerfile links
The tag of each `cubefs` docker image is consist of the version of `cubefs` and the version of basic image. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[3.5.2-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Storage/cubefs/3.5.2/2.0/Dockerfile)| Cubefs 3.5.2 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage

Launch the cubefs container
```
docker run -it --name my-cubefs openkylin/cubefs:latest
```
Entering the container, you will see following files
```
[root@4d8f37230d55 /]#
access         cfs-authtool  cfs-client  cfs-preload  fdstore    libcubefs-1.0-SNAPSHOT-jar-with-dependencies.jar  scheduler
blobnode       cfs-bcache    cfs-deploy  cfs-server   libcfs.h   libcubefs-1.0-SNAPSHOT.jar
blobstore-cli  cfs-cli       cfs-fsck    clustermgr   libcfs.so  proxy
```
Please select to run one of the binary files to finish your job.

        
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).