# Quick reference

- The official Python docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Python | openKylin
Current Python docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Python is a high-level, general-purpose programming language. Its design philosophy emphasizes code readability with the use of significant indentation.

Python is dynamically type-checked and garbage-collected. It supports multiple programming paradigms, including structured (particularly procedural), object-oriented and functional programming. It is often described as a "batteries included" language due to its comprehensive standard library.

# Supported tags and respective Dockerfile links
The tag of each Python docker image is consist of the version of Python and the version of basic image. The details are as follows
| Tags | Currently |  Architectures|
|--|--|--|
|[3.12.12-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/Others/python/3.12.12/2.0/Dockerfile)| Python 3.12.12 on openKylin 2.0 | amd64,arm64,riscv64 |
|[3.10.17-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/Others/python/3.10.17/2.0/Dockerfile)| Python 3.10.17 on openKylin 2.0 | amd64,arm64,riscv64 |


# Usage
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/python` image from docker

	```bash
	# From Docker Hub
	docker pull openkylin/python:{Tag}
	# From Quay
	docker pull quay.io/openkylin/python:{Tag}
	```

- Start a Python container from the image

	```bash
	docker run -it --name my-python openkylin/python:{Tag}
	```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
