# Quick reference

- The official ExecuTorch docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# ExecuTorch | openKylin
**ExecuTorch** is PyTorch's unified solution for deploying AI models on-device—from smartphones to microcontrollers—built for privacy, performance, and portability. It powers Meta's on-device AI across **Instagram, WhatsApp, Quest 3, Ray-Ban Meta Smart Glasses**, and [more](https://docs.pytorch.org/executorch/main/success-stories.html).

Deploy **LLMs, vision, speech, and multimodal models** with the same PyTorch APIs you already know—accelerating research to production with seamless model export, optimization, and deployment. No manual C++ rewrites. No format conversions. No vendor lock-in.

- **🔒 Native PyTorch Export** — Direct export from PyTorch. No .onnx, .tflite, or intermediate format conversions. Preserve model semantics.
- **⚡ Production-Proven** — Powers billions of users at [Meta with real-time on-device inference](https://engineering.fb.com/2025/07/28/android/executorch-on-device-ml-meta-family-of-apps/).
- **💾 Tiny Runtime** — 50KB base footprint. Runs on microcontrollers to high-end smartphones.
- **🚀 [12+ Hardware Backends](https://docs.pytorch.org/executorch/main/backends-overview.html)** — Open-source acceleration for Apple, Qualcomm, ARM, MediaTek, Vulkan, and more.
- **🎯 One Export, Multiple Backends** — Switch hardware targets with a single line change. Deploy the same model everywhere.

# Supported tags and respective Dockerfile links
The tag of each `ExecuTorch` docker image is consist of the complete software stack version. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[1.3.1-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/AI/executorch/1.3.1/2.0/Dockerfile)| ExecuTorch 1.3.1 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
1. Ensure that you have Docker installed, or are using Docker for Linux containers if on Windows.

2. Obtain the ExecuTorch ecosystem docker image. There are two ways to do this:

- Pull the pre-built Docker image
  ```
  # From DockerHub
  docker pull openkylin/executorch
  # From Quay
  docker pull quay.io/openkylin/executorch
  ```

- Clone the [source repository](https://gitee.com/openkylin/openkylin-docker-images)⁠. Navigate to the `AI/executorch` folder and build the image locally with the following command.

  ```docker build . -t openkylin/executorch```

3. Run the Docker container to launch the ExecuTorch developer image.

- ```docker run -it openkylin/executorch```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
