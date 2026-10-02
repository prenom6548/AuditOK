# Quick reference

- The official Apache Flink docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Apache Flink | openKylin
Current Apache Flink docker images are built on the [openKylin](https://repo.openkylin.org/). This repository is free to use and exempted from per-user rate limits.

Apache Flink is a framework and distributed processing engine for stateful computations over unbounded and bounded data streams. Flink has been designed to run in all common cluster environments, perform computations at in-memory speed and at any scale.

Learn more on [Apache Flink Website](https://flink.apache.org/).

# Supported tags and respective Dockerfile links
The tag of each flink docker image is consist of the version of flink and the version of basic image. The details are as follows

| Tags                                                                                                                            | Currently                                     | Architectures |
|---------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------|---------------|
|[2.2.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Bigdata/flink/2.2.0/2.0/Dockerfile) | flink 2.2.0 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage

In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Starting a Session Cluster on Docker:

    A Flink Session cluster can be used to run multiple jobs. Each job needs to be submitted to the cluster after the cluster has been deployed. To deploy a Flink Session cluster with Docker, you need to start a JobManager container. To enable communication between the containers, we first set a required Flink configuration property and create a network:
    ```
    $ FLINK_PROPERTIES="jobmanager.rpc.address: jobmanager"
    $ docker network create flink-network
    ```

    Then we launch the JobManager:
    ```
    $ docker run \
        --rm \
        --name=jobmanager \
        --network flink-network \
        --publish 8081:8081 \
        --env FLINK_PROPERTIES="${FLINK_PROPERTIES}" \
        openkylin/flink:{Tag} jobmanager
    ```
  
    and one or more TaskManager containers:
    ```
    $ docker run \
        --rm \
        --name=taskmanager \
        --network flink-network \
        --env FLINK_PROPERTIES="${FLINK_PROPERTIES}" \
        openkylin/flink:{Tag} taskmanager
    ```
    
    The web interface is now available at localhost:8081.

    Learn more about [how to use Apache Flink with Docker](https://nightlies.apache.org/flink/flink-docs-master/docs/deployment/resource-providers/standalone/docker/)
  
# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
