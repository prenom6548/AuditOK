# Quick reference

- The official hbase docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# hbase | openKylin
Current hbase docker images are built on the [openKylin](https://repo.openkylin.org/). This repository is free to use and exempted from per-user rate limits.

Apache HBase is an open-source, distributed, versioned, column-oriented store modeled after Google' Bigtable.

Learn more on [hbase website](https://hbase.apache.org/).

# Supported tags and respective Dockerfile links
The tag of each hbase docker image is consist of the version of hbase and the version of basic image. The details are as follows
| Tags | Currently |  Architectures|
|--|--|--|
|[2.6.4-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Bigdata/hbase/2.6.4/2.0/Dockerfile)| Apache HBase 2.6.4 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
Start a hbase instance by following command:
```bash
docker run -it \
    --name hbase \
    openkylin/hbase:latest
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
