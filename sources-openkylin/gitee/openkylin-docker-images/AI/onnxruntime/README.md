# Quick reference

- The official ONNX RUNTIME docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# ONNX RUNTIME | openKylin
Current ONNX RUNTIME docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

ONNX Runtime is a cross-platform inference and training machine-learning accelerator.

ONNX Runtime inference can enable faster customer experiences and lower costs, supporting models from deep learning frameworks such as PyTorch and TensorFlow/Keras as well as classical machine learning libraries such as scikit-learn, LightGBM, XGBoost, etc. ONNX Runtime is compatible with different hardware, drivers, and operating systems, and provides optimal performance by leveraging hardware accelerators where applicable alongside graph optimizations and transforms. [Learn more](https://www.onnxruntime.ai/docs/#onnx-runtime-for-inferencing)

# Supported tags and respective Dockerfile links
The tag of each `ONNX RUNTIME` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[1.26.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/onnxruntime/1.26.0/2.0/Dockerfile)| ONNX RUNTIME 1.26.0 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
1. Ensure that you have Docker installed, or are using Docker for Linux containers if on Windows.

2. Obtain the ONNX RUNTIME ecosystem docker image. There are two ways to do this:

- Pull the pre-built Docker image
  ```
  # From DockerHub
  docker pull openkylin/onnxruntime
  # From Quay
  docker pull quay.io/openkylin/onnxruntime  
  ```

- Clone the [source repository](https://gitee.com/openkylin/openkylin-docker-images)⁠. Navigate to the `AI/onnxruntime` folder and build the image locally with the following command.

  ```docker build . -t openkylin/onnxruntime```

3. Run the Docker container to launch the ONNX RUNTIME developer image.

- ```docker run -it openkylin/onnxruntime```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
