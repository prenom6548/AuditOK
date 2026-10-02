ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}
ARG VERSION=6.15.2

RUN apt-get update && \
    apt-get install -y gcc cmake perl git && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN git clone -b V1.0.75 --depth 1 https://github.com/happyfish100/libfastcommon.git \
    && cd libfastcommon \
    && ./make.sh clean \
    && ./make.sh \
    && ./make.sh install \
    && cd .. \
    && rm -rf libfastcommon

RUN git clone -b V1.2.5 --depth 1 https://github.com/happyfish100/libserverframe.git \
    && cd libserverframe \
    && ./make.sh clean \
    && ./make.sh \
    && ./make.sh install \
    && cd .. \
    && rm -rf libserverframe

RUN git clone -b V${VERSION} --depth 1 https://github.com/happyfish100/fastdfs.git \
    && cd fastdfs \
    && ./make.sh clean \
    && ./make.sh \
    && ./make.sh install \
    && cd .. \
    && rm -rf fastdfs

CMD ["fdfs_storaged", "--help"]