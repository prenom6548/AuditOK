ARG BASE=openkylin/go:1.25.4-ok20
FROM ${BASE}

ARG TARGETARCH
ARG CGOFLAG=0
ARG VERSION=0.27.3
ARG FLANNEL_URL=https://github.com/flannel-io/flannel/archive/refs/tags/v${VERSION}.tar.gz

WORKDIR /app

RUN apt update && \
    apt install -y \
        conntrack \
        ipset \
        net-tools
    
RUN if [ ${TARGETARCH} = "amd64" ]; then \
        CGOFLAG=1; \
        apt install -y gcc ; \
    fi && \
    curl -fSL ${FLANNEL_URL} -o flannel.tar.gz && \
    tar -xzf flannel.tar.gz && \
    cd flannel-* && \
    CGO_ENABLED=${CGOFLAG} go build && \
    cp flannel /usr/bin/ && \
    cd .. && \
    rm -rf flannel-* flannel.tar.gz && \
    apt clean && \
    rm -rf /var/lib/apt/lists/* && \ 
    chmod a+x /usr/bin/flannel

ENTRYPOINT ["/usr/bin/flannel"]