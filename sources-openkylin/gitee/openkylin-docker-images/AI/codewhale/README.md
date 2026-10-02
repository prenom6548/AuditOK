# Quick reference

- The official CodeWhale docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Supported tags and respective Dockerfile links
The tag of each `codewhale` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |  Architectures   |
|----------|-------------|------------------|
|[0.8.51-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/codewhale/0.8.51/2.0/Dockerfile)| CodeWhale 0.8.51  on openKylin 2.0 | amd64,arm64,riscv64 |

# Quick Start
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/codewhale` image from docker
```bash
# From Docker Hub
docker pull openkylin/codewhale:{Tag}
#From Quay
docker pull quay.io/openkylin/codewhale:{Tag}
```

- Launch and enter an interactive container
```bash
docker run -it --name my-codewhale openkylin/codewhale:{Tag} 
```

| Option | Description |
|--|--|
| `--name my-codewhale` | Names the container `my-codewhale`. |
| `-it` | Starts the container in interactive mode with a terminal (bash). |
| `openkylin/codewhale:{Tag}` | Specifies the Docker image to run, replace `{Tag}` with the specific version or tag of the `openkylin/codewhale-cli` image you want to use. |

- Run CodeWhale

```bash
codewhale
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).