ARG BASE=openkylin/python:3.12.12-ok20
ARG VERSION=v1.19.1

FROM ${BASE}

ARG VERSION

RUN apt-get update && \
    apt-get install -y \
        autoconf \
        automake \
        libtool \
        build-essential \
        gfortran \
        libblas-dev \
        liblapack-dev \
        git \
        cmake \
        libprotoc-dev \
        protobuf-compiler && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

RUN git clone -b ${VERSION} https://github.com/onnx/onnx.git

WORKDIR /workspace/onnx

RUN git submodule update --init --recursive

ENV CMAKE_ARGS="-DONNX_USE_PROTOBUF_SHARED_LIBS=ON"

RUN find /usr -name "EXTERNALLY-MANAGED" | xargs rm -f

RUN pip install -e . -v -i https://pypi.tuna.tsinghua.edu.cn/simple
