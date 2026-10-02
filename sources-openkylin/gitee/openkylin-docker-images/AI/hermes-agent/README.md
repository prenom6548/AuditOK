# Quick reference

- The official Hermes Agent docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Supported tags and respective Dockerfile links
The tag of each `hermes-agent` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |  Architectures   |
|----------|-------------|------------------|
|[2026.4.16-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/hermes-agent/2026.4.16/2.0/Dockerfile)| Hermes Agent 2026.4.16 on openKylin 2.0 | amd64,arm64,riscv64 |
|[2026.7.7.2-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/hermes-agent/2026.7.7.2/2.0/Dockerfile)| Hermes Agent 2026.7.7.2 on openKylin 2.0 | amd64,arm64,riscv64 |

# Quick Start
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/hermes-agent` image from docker
```bash
# From Docker Hub
docker pull openkylin/hermes-agent:{Tag}
#From Quay
docker pull quay.io/openkylin/hermes-agent:{Tag}
```

- Launch and enter an interactive container
```bash
docker run -it --name my-hermes openkylin/hermes-agent:{Tag} 
```

| Option | Description |
|--|--|
| `--name my-hermes` | Names the container `my-hermes`. |
| `-it` | Starts the container in interactive mode with a terminal (bash). |
| `openkylin/hermes-agent:{Tag}` | Specifies the Docker image to run, replace `{Tag}` with the specific version or tag of the `openkylin/hermes-agent` image you want to use. |

- Run Hermes Agent

```bash
hermes
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).