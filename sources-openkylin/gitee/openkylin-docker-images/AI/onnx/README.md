# Quick reference

- The official ONNX docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# ONNX | openKylin
Current ONNX docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Open Neural Network Exchange (ONNX) is an open ecosystem that empowers AI developers to choose the right tools as their project evolves. ONNX provides an open source format for AI models, both deep learning and traditional ML. It defines an extensible computation graph model, as well as definitions of built-in operators and standard data types. Currently we focus on the capabilities needed for inferencing (scoring).

Learn more about on [ONNX Website](https://onnx.ai/).

# Supported tags and respective Dockerfile links
The tag of each `ONNX` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[1.19.1-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/onnx/1.19.1/2.0/Dockerfile)| ONNX 1.19.1 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
1. Ensure that you have Docker installed, or are using Docker for Linux containers if on Windows.

2. Obtain the ONNX ecosystem docker image. There are two ways to do this:

- Pull the pre-built Docker image
  ```
  # From DockerHub
  docker pull openkylin/onnx
  # From Quay
  docker pull quay.io/openkylin/onnx
  ```

- Clone the [source repository](https://gitee.com/openkylin/openkylin-docker-images)⁠. Navigate to the `AI/onnx` folder and build the image locally with the following command.

  ```docker build . -t openkylin/onnx```

3. Run the Docker container to launch the ONNX developer image.

- ```docker run -it openkylin/onnx```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
