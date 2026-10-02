# Quick reference

- The official Qwen Code docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Supported tags and respective Dockerfile links
The tag of each `qwen-code` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |  Architectures   |
|----------|-------------|------------------|
|[0.12.6-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/qwen-code/0.12.6/2.0/Dockerfile)| Qwen-Code 0.12.6  on openKylin 2.0 | amd64,arm64,riscv64 |

# Quick Start
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/qwen-code` image from docker
```bash
docker pull openkylin/qwen-code:{Tag}
```

- Launch and enter an interactive container
```bash
docker run -it --name my-qwen openkylin/qwen-code:{Tag} 
```

| Option | Description |
|--|--|
| `--name my-qwen` | Names the container `my-qwen`. |
| `-it` | Starts the container in interactive mode with a terminal (bash). |
| `openkylin/qwen-code:{Tag}` | Specifies the Docker image to run, replace `{Tag}` with the specific version or tag of the `openkylin/qwen-code` image you want to use. |

- Run Qwen Code

```bash
qwen
```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).