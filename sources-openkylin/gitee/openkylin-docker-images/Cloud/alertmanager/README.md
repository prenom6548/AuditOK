# Quick reference

- The official Alertmanager docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Alertmanager | openKylin
Current Alertmanager docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

The Alertmanager handles alerts sent by client applications (such as a Prometheus server). It is responsible for deduplication, grouping, and routing them to the correct sink integration, such as email, PagerDuty, OpsGenie, or many other mechanisms with the help of a webhook receiver. It is also responsible for silencing and suppressing alerts. Read more on the [official documentation⁠](https://prometheus.io/docs/alerting/latest/alertmanager/).

# Supported tags and respective Dockerfile links
The tag of each `alertmanager` docker image is consist of the version of `alertmanager` and the version of basic image. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[0.29.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Cloud/alertmanager/0.29.0/2.0/Dockerfile)| Alertmanager 0.29.0 on openKylin 2.0 | amd64,arm64,riscv64|

# Usage
In this usage, users can select the corresponding `{Tag}` and `container startup options` based on their requirements.

- Pull the `openkylin/alertmanager` image from docker

	```bash
	# From Docker Hub
	docker pull openkylin/alertmanager:{Tag}
	# From Quay
	docker pull quay.io/openkylin/alertmanager:{Tag}
	```

- Start a alertmaanger instance

	```bash
	docker run -d --name my-alertmanager -p 9093:9093 openkylin/alertmanager:{Tag}
	```
	After the instance `my-alertmanager` is started, access the alertmanager service through `http://localhost:9093`.

- Container startup options

	| Option | Description |
	|--|--|
	| `-p 9093:9093` | Expose alertmanager on `localhost:9093`. |
	| `-v /local/path/to/website:/var/www/html` | Mount and serve a local website. |
	| `-v /path/to/alertmanager.yml:/etc/prometheus/alertmanager.yml`	| Local configuration file alertmanager.yml. |
	| `-v /path/to/persisted/data:/alertmaanger` | Persist data instead of initializing a new database for each newly launched container. |

- View container running logs

	```bash
	docker logs -f my-alertmaanger
	```

- To get an interactive shell

	```bash
	docker exec -it my-alertmaanger /bin/bash
	```
	
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
