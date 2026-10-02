ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}

ARG TARGETARCH
ARG VERSION=2.2.0

RUN apt update && \
    apt install -y wget && \
    apt clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /usr/local/containerd

RUN wget https://github.com/containerd/containerd/releases/download/v${VERSION}/containerd-${VERSION}-linux-${TARGETARCH}.tar.gz \
    && tar -zxvf containerd-${VERSION}-linux-${TARGETARCH}.tar.gz \
    && rm -f containerd-${VERSION}-linux-${TARGETARCH}.tar.gz \
    && ln -sf bin/* /usr/local/bin/

WORKDIR /

CMD ["containerd", "--help"]