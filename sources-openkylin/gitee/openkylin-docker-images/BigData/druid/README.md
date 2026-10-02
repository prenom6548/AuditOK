# Quick reference

- The official druid docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Druid | openKylin
Current druid docker images are built on the [openKylin](https://www.openkylin.top/). This repository is free to use and exempted from per-user rate limits.

Druid is a high performance real-time analytics database. Druid's main value add is to reduce time to insight and action.

Learn more about Druid on [Druid Website](https://druid.apache.org/)⁠.

# Supported tags and respective Dockerfile links
The tag of each `druid` docker image is consist of the version of `druid` and the version of basic image. The details are as follows

|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[35.0.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Bigdata/druid/35.0.0/2.0/Dockerfile) | druid 35.0.0 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage

In this usage, users can select the corresponding `{Tag}` based on their requirements.

Run with the single server deployment, which automatically configures various memory-related parameters based on the 
available processors and memory, using the default JVM arguments and service-specific runtime properties.

- Pull the `openkylin/druid` image from docker

  ```bash
  # From Docker Hub
  docker pull openkylin/druid:{Tag}
  # From Quay
  docker pull quay.io/openkylin/druid:{Tag}
  ```

- Start a druid instance

  ```bash
  docker run -d -p 8888:8888 --name my-druid openkylin/druid:{Tag}
  ```
  After the instance `my-druid` is started, access the web console through `http://{ip}:8888`.

- View container running logs

  ```bash
  docker logs -f my-druid
  ```
  
- To get an interactive shell

	```bash
	docker exec -it my-druid /bin/bash
	```
        
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
