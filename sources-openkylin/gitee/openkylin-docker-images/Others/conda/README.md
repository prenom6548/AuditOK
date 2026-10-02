# Quick reference

- The official Conda docker image based on Miniforge

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Conda | openKylin

> Important: The current RISC-V version is only supported by the community. We are working with conda-forge to continuously improve RISC-V support on miniforge.

Current Conda docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Conda is an open source package management system and environment management system for installing multiple versions of software packages and their dependencies and switching easily between them. Miniforge is the preferred conda-forge installer and includes conda, mamba, and their dependencies.

Learn more about Conda on [anaconda](https://anaconda.org/channels/anaconda/packages/conda/overview) and Miniforge on [conda-forge](https://conda-forge.org/download/)

# Supported tags and respective Dockerfile links

The tag of each `conda` docker image is consist of the version of `conda` and the version of basic image. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[25.9.1-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/conda/25.9.1/2.0/Dockerfile) | conda 25.9.1 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage

In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/conda` image from docker

	```bash
	docker pull openkylin/conda:{Tag}
	```

- Start a conda instance

	```bash
	docker run -d --name my-conda openkylin/conda:{Tag} bash
	```

# Question and answering

If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).