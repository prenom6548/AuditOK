ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}

ARG TARGETARCH
ARG BUILDARCH
ARG FOLDERNAME
ARG VERSION=22.21.1
ARG DONWLOAD_URL

RUN apt update && apt install -y wget xz-utils libatomic1 && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN if [ ${TARGETARCH} = "amd64" ]; then \
        BUILDARCH="x64"; \
        FOLDERNAME="node-v${VERSION}-linux-${BUILDARCH}"; \
        DONWLOAD_URL="https://nodejs.org/download/release/v${VERSION}/${FOLDERNAME}.tar.xz"; \
    elif [ ${TARGETARCH} = "arm64" ]; then \
        BUILDARCH="arm64"; \
        FOLDERNAME="node-v${VERSION}-linux-${BUILDARCH}"; \
        DONWLOAD_URL="https://nodejs.org/download/release/v${VERSION}/${FOLDERNAME}.tar.xz"; \
    elif [ ${TARGETARCH} = "riscv64" ]; then \
        ARCH_RISCV64="${TARGETARCH}"; \
        BUILDARCH="${TARGETARCH}"; \
        FOLDERNAME="node-v${VERSION}-linux-${BUILDARCH}"; \
        DONWLOAD_URL="https://unofficial-builds.nodejs.org/download/release/v${VERSION}/${FOLDERNAME}.tar.xz"; \
    else \
        echo "Unsupported architecture: ${TARGETARCH}"; \
        exit 1; \
    fi && \
    wget ${DONWLOAD_URL} && \
    tar -xf ${FOLDERNAME}.tar.xz -C /opt && \
    mv /opt/${FOLDERNAME} /opt/node && \
    ln -s /opt/node/bin/* /usr/local/bin && \
    rm -rf ${FOLDERNAME}.tar.xz

CMD ["bash"]
