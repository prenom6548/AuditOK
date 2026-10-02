# Quick reference

- The official hive docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# hive | openKylin
Current hive docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

The Apache Hive (TM) data warehouse software facilitates reading, writing, and managing large datasets residing in distributed storage using SQL.

Learn more on [hive website](https://hive.apache.org/).

# Supported tags and respective Dockerfile links
The tag of each hive docker image is consist of the version of hive and the version of basic image. The details are as follows
| Tags | Currently |  Architectures|
|--|--|--|
|[4.2.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Bigdata/hive/4.2.0/2.0/Dockerfile)| Apache hive 4.2.0 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
Start a hive instance by following command:
```bash
docker run -it \
    --name hive \
    -p 10000:10000 \
    -p 10002:10002 \
    -p 9083:9083
    openkylin/hive:latest
```

The following message indicates that the hive is ready :
```
SLF4J: Class path contains multiple SLF4J bindings.
SLF4J: Found binding in [jar:file:/usr/local/hive/lib/log4j-slf4j-impl-2.18.0.jar!/org/slf4j/impl/StaticLoggerBinder.class]
SLF4J: Found binding in [jar:file:/usr/local/hadoop/share/hadoop/common/lib/slf4j-reload4j-1.7.36.jar!/org/slf4j/impl/StaticLoggerBinder.class]
SLF4J: See http://www.slf4j.org/codes.html#multiple_bindings for an explanation.
SLF4J: Actual binding is of type [org.apache.logging.slf4j.Log4jLoggerFactory]
SLF4J: Class path contains multiple SLF4J bindings.
SLF4J: Found binding in [jar:file:/usr/local/hive/lib/log4j-slf4j-impl-2.18.0.jar!/org/slf4j/impl/StaticLoggerBinder.class]
SLF4J: Found binding in [jar:file:/usr/local/hadoop/share/hadoop/common/lib/slf4j-reload4j-1.7.36.jar!/org/slf4j/impl/StaticLoggerBinder.class]
SLF4J: See http://www.slf4j.org/codes.html#multiple_bindings for an explanation.
SLF4J: Actual binding is of type [org.apache.logging.slf4j.Log4jLoggerFactory]
Beeline version 4.0.1 by Apache Hive
beeline>

```

The default entrypoint is `hive`, users can use `--entrypoint` to change the entrypoint.

To get an interactive shell
```bash
docker exec -it hive /bin/bash
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
