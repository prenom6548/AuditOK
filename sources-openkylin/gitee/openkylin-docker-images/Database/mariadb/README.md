# Quick reference

- The official mariadb docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# mariadb | openKylin
Current mariadb docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

MariaDB server is a community developed fork of MySQL server. Started by core members of the original MySQL team, MariaDB actively works with outside developers to deliver the most featureful, stable, and sanely licensed open SQL server in the industry.

Learn more about mariadb at [https://mariadb.org/](https://mariadb.org/).

# Supported tags and respective Dockerfile links
The tag of each `mariadb` docker image is consist of the version of `mariadb` and the version of basic image. The details are as follows
|    Tag   |  Currently  |   Architectures  |
|----------|-------------|------------------|
|[12.1.2-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Database/mariadb/12.1.2/2.0/Dockerfile)| mariadb 12.1.2 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage

- Launch the mariadb instance

	As applications talk to MariaDB, MariaDB needs to start in the same network as the application:	
	```
	docker network create some-network 
	docker run --detach --network some-network --name some-mariadb --env MARIADB_USER=example-user --env MARIADB_PASSWORD=my_cool_secret --env MARIADB_ROOT_PASSWORD=my-secret-pw openkylin/mariadb:latest
	```
	You can connect to MariaDB from the MariaDB command line client, more information about the MariaDB command-line client can be found in the [MariaDB Command Line Client⁠](https://mariadb.com/kb/en/mariadb-command-line-client/)

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).