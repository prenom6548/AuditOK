# Node.js 

# Quick reference

- The official Node.js docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Build reference

1. Build images and push:
```shell
docker buildx build -t "openkylin/node:$VERSION" --platform linux/amd64,linux/arm64 . --push
```

We are using `buildx` in here to generate multi-arch images, see more in [Docker Buildx](https://docs.docker.com/buildx/working-with-buildx/)

2. Run:
```shell
docker run -it openkylin/node:$VERSION bash
```

# Supported tags and respective Dockerfile links
The tag of each `next` docker image is consist of the version of `next` and the version of basic image. The details are as follows

| Tag                                                                                                                                   | Currently                                 | Architectures |
|---------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------|---------------|
|[22.21.1-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/node/22.12.1/2.0/Dockerfile) | node 22.21.1 on openKylin 2.0 | amd64,arm64,riscv64 |
|[24.14.1-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/node/24.14.1/2.0/Dockerfile) | node 24.14.1 on openKylin 2.0 | amd64,arm64,riscv64 |
