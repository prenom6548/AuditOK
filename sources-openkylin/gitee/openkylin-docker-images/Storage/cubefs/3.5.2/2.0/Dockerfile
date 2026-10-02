ARG BASE=openkylin/go:1.25.4-ok20
FROM $BASE AS builder

ARG TARGETARCH
ARG VERSION=3.5.2

WORKDIR /app
RUN apt update && \
    apt install -y git make cmake g++ maven && \
    git clone -b v${VERSION} https://github.com/cubefs/cubefs.git && \
    cd cubefs && \
    make 

FROM ${BASE}

COPY --from=builder /app/cubefs/build/bin/* /usr/local/bin/

WORKDIR /usr/local/bin