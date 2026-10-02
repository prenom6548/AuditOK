ARG BASE=openkylin/python:3.12.12-ok20
ARG VERSION=2.12.0

FROM ${BASE} AS builder 

ARG VERSION
ARG PYTORCH_REPO=https://github.com/pytorch/pytorch.git

RUN apt -y update && \
    apt install -y git ca-certificates build-essential cmake ninja-build libopenblas-dev libatomic1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    git clone --recursive -b v$VERSION $PYTORCH_REPO && \
    cd pytorch && \
    pip install --upgrade pip setuptools wheel && \
    pip install -r requirements.txt

ENV USE_CUDA=0 \
    USE_ROCM=0 \
    USE_CUDNN=0 \
    USE_MKLDNN=0 \
    USE_QNNPACK=0 \
    WITH_BLAS=openblas \
    CMAKE_GENERATOR=Ninja \
    MAX_JOBS=4 \
    BUILD_IGNORE_SVE_UNAVAILABLE=1

WORKDIR /pytorch

RUN sed -i '1s/^/link_libraries(atomic)\n/' CMakeLists.txt

RUN export BUILD_TEST=0 && \
    python setup.py bdist_wheel

ARG BASE
FROM $BASE AS runtime

COPY --from=builder pytorch/dist/*.whl /root

WORKDIR /root

RUN apt -y update && \
    apt install -y ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install /root/*.whl && \
    rm -rf /root/*.whl