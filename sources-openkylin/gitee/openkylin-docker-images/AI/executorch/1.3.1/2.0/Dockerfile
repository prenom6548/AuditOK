ARG BASE=openkylin/python:3.12.12-ok20
ARG VERSION=1.3.1
ARG EXECUTORCH_URL=https://github.com/pytorch/executorch.git

FROM ${BASE}

ARG VERSION
ARG EXECUTORCH_URL

RUN apt update -y && \
    apt install -y git build-essential && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN git clone -b v${VERSION} ${EXECUTORCH_URL} && \
    cd executorch && \
    ./install_executorch.sh

CMD ["/bin/bash"]