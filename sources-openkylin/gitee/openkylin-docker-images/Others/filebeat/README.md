# Quick reference

- The official Filebeat docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Filebeat | openKylin
Current Filebeat docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Filebeat is a lightweight shipper for forwarding and centralizing log data. Installed as an agent on your servers, Filebeat monitors the log files or locations that you specify, collects log events, and forwards them either to Elasticsearch or Logstash for indexing.

Learn more about Filebeat on [Filebeat Website](https://www.elastic.co/docs/reference/beats/filebeat/)⁠.

# Supported tags and respective Dockerfile links
The tag of each `filebeat` docker image is consist of the version of `filebeat` and the version of basic image. The details are as follows

|    Tag   | Currently                                 |   Architectures  |
|----------|-------------------------------------------|------------------|
|[9.2.3-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Others/filebeat/9.2.3/2.0/Dockerfile)| Filebeat 9.2.3 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/filebeat` image from docker

	```bash
    # From Docker Hub
	docker pull openkylin/filebeat:{Tag}
    # From Quay
    docker pull quay.io/openkylin/filebeat:{Tag}
	```

- Run with an interactive shell

    You can also start the container with an interactive shell to use filebeat.
    ```
    docker run -it --rm openkylin/filebeat:{Tag} bash
    ```
  
- Create the `filebeat.yml` file
  
    Here's a minimal Filebeat test configuration (filebeat.yml) in English for functional testing:
    ```
    filebeat.inputs:
    - type: filestream
        enabled: true
        paths: ["/var/log/log"]

    output.file:
        path: "/tmp/output"
        filename: "logs.json"
    ```
    Learn how to configure Filebeat on [Configure Filebeat](https://www.elastic.co/docs/reference/beats/filebeat/configuring-howto-filebeat).

- Run in Foreground (Debug Mode)

    ```   
    filebeat -e -c filebeat.yml
    ```

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).