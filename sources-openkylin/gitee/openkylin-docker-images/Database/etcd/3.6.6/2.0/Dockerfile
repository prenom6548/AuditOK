ARG BASE=openkylin/go:1.25.4-ok20
FROM $BASE AS builder

ARG VERSION=3.6.6
ARG TARGETARCH

RUN curl -fSL -o etcd.tar.gz https://github.com/etcd-io/etcd/archive/refs/tags/v${VERSION}.tar.gz && \
    mkdir -p /etcd && \
    tar -zxf etcd.tar.gz -C /etcd --strip-components=1 && \
    rm -rf etcd.tar.gz && \
    cd /etcd && \
    make -j$nproc

FROM openkylin/openkylin:2.0

COPY --from=builder /etcd/bin/etcd /usr/bin/etcd
COPY --from=builder /etcd/bin/etcdctl /usr/bin/etcdctl
COPY --from=builder /etcd/bin/etcdutl /usr/bin/etcdutl

EXPOSE 2379 2380
CMD [ "etcd" ]