# Quick reference

- The official flannel docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# flannel | openKylin
Current flannel images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Flannel is a simple and easy way to configure a layer 3 network fabric designed for Kubernetes.

# Supported tags and respective dockerfile links
The tag of each `flannel` docker image is consist of the version of `flannel` and the version of basic image. The details are as follows

| Tag                                                                                                                               | Currently                                 | Architectures |
|-----------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------|---------------|
|[0.27.4-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Cloud/flannel/0.27.4/2.0/Dockerfile) | flannel 0.27.4 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/flannel` image from docker

	```
    # From Docker Hub
	docker pull openkylin/flannel:{Tag}
	# From Quay
	docker pull quay.io/openkylin/flannel:{Tag}
	```

- Start a flannel instance

    ```
    docker run -it --rm openkylin/flannel:{Tag}
    ```
    The `openkylin/flannel` image is used to verify the integration between the upstream flannel version and openKylin. 

  
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
