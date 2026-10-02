# Quick reference

- The official Gemini Cli docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Supported tags and respective Dockerfile links
The tag of each `gemini-cli` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |  Architectures   |
|----------|-------------|------------------|
|[0.36.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/gemini-cli/0.36.0/2.0/Dockerfile)| Gemini Cli 0.36.0  on openKylin 2.0 | amd64,arm64,riscv64 |

# Quick Start
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/gemini-cli` image from docker
```bash
# From Docker Hub
docker pull openkylin/gemini-cli:{Tag}
#From Quay
docker pull quay.io/openkylin/gemini-cli:{Tag}
```

- Launch and enter an interactive container
```bash
docker run -it --name my-gemini openkylin/gemini-cli:{Tag} 
```

| Option | Description |
|--|--|
| `--name my-gemini` | Names the container `my-gemini`. |
| `-it` | Starts the container in interactive mode with a terminal (bash). |
| `openkylin/gemini-cli:{Tag}` | Specifies the Docker image to run, replace `{Tag}` with the specific version or tag of the `openkylin/gemini-cli` image you want to use. |

- Run Gemini Cli

```bash
gemini
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).