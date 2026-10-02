# Quick reference

- The official Kafka docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Kafka | openKylin
Current Kafka docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Apache Kafka is an open-source distributed event streaming platform used by thousands of companies for high-performance data pipelines, streaming analytics, data integration, and mission-critical applications.

Learn more on [Kafka website](https://kafka.apache.org/).

# Supported tags and respective Dockerfile links
The tag of each kafka docker image is consist of the version of kafka and the version of basic image. The details are as follows
| Tags | Currently |  Architectures|
|--|--|--|
|[4.1.1-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Bigdata/kafka/4.1.1/2.0/Dockerfile) | kafka 4.1.1 on openKylin 2.0 | amd64,arm64,riscv64 |


# Usage
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/kafka` image
	```bash
	# From DockerHub
	docker pull openkylin/kafka:{Tag}
	# From Quay
	docker pull quay.io/openkylin/kafka:{Tag}
	```
- Start a kafka instance

	```bash
	docker run -d --name my-kafka -p 9092:9092 openkylin/kafka:{Tag}
	```
	After the instance `my-kafka` is started, access the kafka service through `http://localhost:9092`.

- Container startup options
	| Option | Description |
	|--|--|
	| `-p 9092:9092`	 | 	Expose Apache Kafka server on `localhost:9092`. |
    | `-e ZOOKEEPER_HOST=<zookeeper>` | Hostname for the related Zookeeper instance. |
    | `-e ZOOKEEPER_PORT=2181`	| 	Port for the related Zookeeper instance. |
    | `-v /path/to/config/file:/etc/kafka/server.properties` | Local Kafka configuration file. |
    | `-v kafkaData:/var/lib/kafka` | "Persist data in a docker volume named `kafkaData`. " "Make sure that the mount point is consistent with the configuration property `logs.dirs`. |

- View container running logs
	```bash
	docker logs -f my-kafka
	```
- To get an interactive shell
	```bash
	docker exec -it my-kafka /bin/bash
	```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
