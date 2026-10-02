# Quick reference

- The official lammps docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# lammps | openKylin
Current lammps docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

LAMMPS is a classical molecular dynamics code with a focus on materials modeling. It's an acronym for Large-scale Atomic/Molecular Massively Parallel Simulator.

# Supported tags and respective Dockerfile links
The tag of each `lammps` docker image is consist of the version of `lammps` and the version of basic image. The details are as follows

| Tag                                                                                                                                  | Currently                                   | Architectures |
|--------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------|---------------|
| [22Jul2025-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/HPC/lammps/22Jul2025/2.0/Dockerfile) | lammps 22Jul2025 on openKylin 2.0 | amd64,arm64,riscv64  |

# Usage
In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/lammps` image from docker

	```bash
    # From Docker Hub
	docker pull openkylin/lammps:{Tag}
    # From Quay
    docker pull quay.io/openkylin/lammps:{Tag}
	```

- Start a lammps instance

    ```
    docker run -it --rm openkylin/lammps:{Tag}
    ```
    The `openkylin/lammps` image is used to verify the integration between the upstream lammps version and openKylin. 

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).