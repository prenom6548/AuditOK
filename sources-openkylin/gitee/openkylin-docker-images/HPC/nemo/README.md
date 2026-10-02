# Quick reference

- The official Nemo docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Nemo | openKylin
Current Nemo docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Nemo is a free and open-source software and official file manager of the Cinnamon desktop environment. It is a fork of GNOME Files (formerly named Nautilus).

# Supported tags and respective Dockerfile links
The tag of each `nemo` docker image is consist of the version of `nemo` and the version of basic image. The details are as follows

| Tag                                                                                                                        | Currently                             | Architectures |
|----------------------------------------------------------------------------------------------------------------------------|---------------------------------------|---------------|
|[6.6.2-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/HPC/nemo/6.6.2/2.0/Dockerfile) | nemo 6.6.2 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/nemo` image from docker

	```bash
    # From Docker Hub
	docker pull openkylin/nemo:{Tag}
    # From Quay
    docker pull quay.io/openkylin/nemo:{Tag}
	```

- Start a nemo instance

    ```
    docker run -it --rm openkylin/nemo:{Tag}
    ```
    The `openkylin/nemo` image is used to verify the integration between the upstream nemo version and openKylin.

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).