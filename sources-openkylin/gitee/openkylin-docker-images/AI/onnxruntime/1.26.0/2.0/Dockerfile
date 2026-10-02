ARG BASE=openkylin/python:3.12.12-ok20
ARG VERSION=v1.26.0

FROM $BASE AS builder 

ARG VERSION

ARG ONNXRUNTIME_REPO=https://github.com/microsoft/onnxruntime.git

RUN apt -y update && \
    apt install -y git ca-certificates build-essential cmake libatomic1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    git clone --recursive -b $VERSION $ONNXRUNTIME_REPO && \
    cd onnxruntime && \
    pip install --upgrade pip setuptools wheel && \
    pip install -r requirements.txt

WORKDIR /onnxruntime

RUN sed -i '1s/^/link_libraries(atomic)\n/' cmake/CMakeLists.txt

RUN ./build.sh --allow_running_as_root \
               --skip_submodule_sync \
               --config Release \
               --build_wheel \
               --update \
               --build \
               --no_kleidiai \
               --parallel \
               --skip_tests \
               --cmake_extra_defines CMAKE_CXX_FLAGS="-Wno-error=restrict" ONNXRUNTIME_VERSION=$(cat ./VERSION_NUMBER)

ARG BASE
FROM $BASE AS runtime

COPY --from=builder onnxruntime/build/Linux/Release/dist/*.whl /root

WORKDIR /root

RUN apt -y update && \
    apt install -y ca-certificates build-essential && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install /root/*.whl && \
    rm -rf /root/*.whl