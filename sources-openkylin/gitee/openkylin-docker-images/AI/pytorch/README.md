# Quick reference

- The official PyTorch docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# PyTorch | openKylin
Current PyTorch docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

PyTorch is an optimized tensor library for deep learning using GPUs and CPUs.

Learn more about on [PyTorch Website](https://pytorch.org/).

# Supported tags and respective Dockerfile links
The tag of each `PyTorch` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[2.6.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/pytorch/2.6.0/2.0/Dockerfile)| PyTorch 2.6.0 on openKylin 2.0 | arm64,amd64,riscv64 |
|[2.11.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/pytorch/2.11.0/2.0/Dockerfile)| PyTorch 2.11.0 on openKylin 2.0 | arm64,amd64,riscv64 |
|[2.12.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/pytorch/2.12.0/2.0/Dockerfile)| PyTorch 2.12.0 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
1. Ensure that you have Docker installed, or are using Docker for Linux containers if on Windows.

2. Obtain the PyTorch ecosystem docker image. There are two ways to do this:

- Pull the pre-built Docker image
  ```
  # From DockerHub
  docker pull openkylin/pytorch
  # From Quay
  docker pull quay.io/openkylin/pytorch
  ```

- Clone the [source repository](https://gitee.com/openkylin/openkylin-docker-images)⁠. Navigate to the `AI/pytorch` folder and build the image locally with the following command.

  ```docker build . -t openkylin/pytorch```

3. Run the Docker container to launch the PyTorch developer image.

- ```docker run -it openkylin/pytorch```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
