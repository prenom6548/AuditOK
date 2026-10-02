ARG BASE=openkylin/openkylin:2.0
FROM $BASE

ARG VERSION=4.2.0

# install java env
RUN apt update && \
    apt install -y openjdk-17-jdk-headless findutils curl && \
    apt clean && \
    rm -rf /var/lib/apt/lists/*

# install hive
RUN curl -fSL -o hive.tar.gz https://dlcdn.apache.org/hive/hive-${VERSION}/apache-hive-${VERSION}-bin.tar.gz; \
    mkdir -p /usr/local/hive && \
    tar -zxf hive.tar.gz -C /usr/local/hive --strip-components=1 && \
    rm -rf hive.tar.gz
ENV PATH=$PATH:/usr/local/hive/bin

# install dependent hadoop
ARG HADOOP_VERSION=3.4.1
RUN curl -fSL -o hadoop.tar.gz https://dlcdn.apache.org/hadoop/common/hadoop-${HADOOP_VERSION}/hadoop-${HADOOP_VERSION}.tar.gz; \
    mkdir -p /usr/local/hadoop && \
    tar -zxf hadoop.tar.gz -C /usr/local/hadoop --strip-components=1 && \
    rm -rf hadoop.tar.gz
ENV PATH=$PATH:/usr/local/hadoop/bin
ENV HADOOP_HOME=/usr/local/hadoop/

WORKDIR /usr/local/hive/bin
EXPOSE 10000 10002 9083
CMD ["hive"]