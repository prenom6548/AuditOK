ARG BASE=openkylin/openkylin:2.0
FROM $BASE
ARG TARGETARCH
ARG VERSION=2.6.4

RUN apt update && apt install -y curl && \
    curl -fSL -o hbase.tar.gz https://dlcdn.apache.org/hbase/${VERSION}/hbase-${VERSION}-bin.tar.gz; \
    mkdir -p /usr/local/hbase && \
    tar -zxf hbase.tar.gz -C /usr/local/hbase --strip-components=1 && \
    rm -rf hbase.tar.gz

ENV PATH=$PATH:/usr/local/hbase/bin

RUN apt-get update && apt-get install -y openjdk-11-jdk-headless findutils && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-$TARGETARCH

EXPOSE 8080 8085 9090 9095 2181 16010
ENTRYPOINT ["hbase", "version"]