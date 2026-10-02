# Quick reference

- The official Minimax Cli docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Supported tags and respective Dockerfile links
The tag of each `mmx-cli` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |  Architectures   |
|----------|-------------|------------------|
|[1.0.5-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/mmx-cli/1.0.5/2.0/Dockerfile)| Minimax Cli 1.0.5  on openKylin 2.0 | amd64,arm64,riscv64 |

# Quick Start
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/mmx-cli` image from docker
```bash
# From Docker Hub
docker pull openkylin/mmx-cli:{Tag}
#From Quay
docker pull quay.io/openkylin/mmx-cli:{Tag}
```

- Launch and enter an interactive container
```bash
docker run -it --name my-mmx openkylin/mmx-cli:{Tag} 
```

| Option | Description |
|--|--|
| `--name my-mmx` | Names the container `my-mmx`. |
| `-it` | Starts the container in interactive mode with a terminal (bash). |
| `openkylin/mmx-cli:{Tag}` | Specifies the Docker image to run, replace `{Tag}` with the specific version or tag of the `openkylin/mmx-cli` image you want to use. |

- Run Minimax Cli

```bash
mmx
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).