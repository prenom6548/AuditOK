# Quick reference

- The official distroless-base docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# distroless-base | openKylin
This image contains a minimal Linux, glibc-based system. It is intended for use directly by "mostly-statically compiled" languages like Go, Rust or D.

Statically compiled applications (Go) that do not require libc can use the `openkylin/distroless-static` image, which contains:
- ca-certificates
- tzdata

Most other applications (and Go apps that require libc/cgo) should start with `openkylin/distroless-base`, which contains all of the packages in `openkylin/distroless-static`, and
- glibc
- openssl_libs

# Supported tags and respective Dockerfile links
The tag of each `distroless-base` docker image is consist of the version of `glibc` and version of openKylin. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[2.0](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Distroless/distroless-base/2.0/Dockerfile)| Glibc 4.1.4 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
Users are expected to include their compiled application and set the correct cmd in their image. For [example](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Distroless/distroless-base/2.0/test)
```
# Dockerfile

FROM openkylin/openkylin:2.0 AS build-env
COPY . /app
WORKDIR /app
RUN yum install -y gcc g++
RUN cc hello.c -o hello

FROM openkylin/distroless-base:2.38-oe2403lts
COPY --from=build-env /app /app
WORKDIR /app
CMD ["./hello"]
```

# Run Applications as a Non-Root User
For implementation details, refer to the [distroless-base-nonroot documentation](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Distroless/distroless-base-nonroot/README.md).
	
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
