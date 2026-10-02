# Quick reference

- The official distroless-static docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# distroless-static | openKylin
This image contains a minimal Linux, glibc runtime for "mostly-statically compiled" languages like Rust and Go.

Statically compiled applications (Go) that do not require libc can use the `openkylin/distroless-static` image, which contains:
- ca-certificates
- tzdata

# Supported tags and respective Dockerfile links
The tag details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[2.0](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Distroless/distroless-static/2.0/Distrofile)| openKylin 2.0 static image | amd64,arm64,riscv64 |

# Usage
Users are expected to include their compiled application and set the correct cmd in their image. For [example](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Distroless/distroless-static/2.0/test)
```
# Dockerfile

FROM openkylin/go:1.26.1-ok20 AS build
WORKDIR /go/src/app
COPY . .
RUN go mod download
RUN go vet -v
RUN go test -v
RUN CGO_ENABLED=0 go build -o /go/bin/app

FROM openkylin/distroless-static:2.0
COPY --from=build /go/bin/app /
CMD ["/app"]

```

# Run Applications as a Non-Root User
For implementation details, refer to the [distroless-base-nonroot documentation](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Distroless/distroless-base-nonroot/README.md).
	
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
