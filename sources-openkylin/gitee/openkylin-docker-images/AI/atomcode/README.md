# Quick reference

- The official AtomCode docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Supported tags and respective Dockerfile links
The tag of each `atomcode` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |  Architectures   |
|----------|-------------|------------------|
|[4.20.4-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/atomcode/4.20.4/2.0/Dockerfile)| AtomCode 4.20.4 on openKylin 2.0 | amd64,arm64,riscv64 |

# Quick Start
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/atomcode` image from docker
```bash
docker pull openkylin/atomcode:{Tag}
```

- Launch and enter an interactive container
```bash
docker run -it --name my-atomcode openkylin/atomcode:{Tag} 
```

| Option | Description |
|--|--|
| `--name my-atomcode` | Names the container `my-atomcode`. |
| `-it` | Starts the container in interactive mode with a terminal (bash). |
| `openkylin/atomcode:{Tag}` | Specifies the Docker image to run, replace `{Tag}` with the specific version or tag of the `openkylin/atomcode` image you want to use. |

- Run AtomCode

```bash
atomcode
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).