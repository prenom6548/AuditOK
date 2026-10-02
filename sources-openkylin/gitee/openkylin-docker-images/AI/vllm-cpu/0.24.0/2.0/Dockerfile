ARG BASE=openkylin/pytorch:2.11.0-ok20
ARG VERSION=v0.24.0

FROM ${BASE} AS base

ARG VERSION

RUN apt update -y && \
    apt install -y \
    ccache \
    git \
    curl \
    wget \
    ca-certificates \
    gcc-13 \
    g++-13 \
    make \
    cmake \
    autoconf \
    automake \
    libtool \
    pkg-config \
    ffmpeg \
    jq \
    lsof \
    numactl \
    libnuma-dev \
    libffi-dev \
    libssl-dev \
    libjpeg-dev \
    zlib1g-dev \
    libpng-dev \
    libzmq3-dev \
    llvm-dev \
    clang \
    libprotobuf-dev \
    protobuf-compiler \
    libatomic1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y && \
    . "$HOME/.cargo/env"

ENV CC=gcc-13
ENV CXX=g++-13
ENV CFLAGS="-O2"
ENV CXXFLAGS="-O2"
ENV LDFLAGS="-latomic"
ENV CMAKE_BUILD_PARALLEL_LEVEL=4
ENV MAX_JOBS=4
 
WORKDIR /workspace

RUN git clone -b ${VERSION} https://github.com/vllm-project/vllm.git

WORKDIR /workspace/vllm

RUN . "$HOME/.cargo/env" && \ 
    export LLVM_CONFIG=$(which llvm-config) && \
    sed -i '/torch/d' requirements/cpu.txt && \
    sed -i '/numba/s/^/#/' requirements/cpu.txt && \
    pip install --extra-index https://download.pytorch.org/whl/cpu/ -r requirements/cpu.txt

FROM base AS build 

RUN apt update -y && \
    apt install -y libgomp1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN export LLVM_CONFIG=$(which llvm-config) && \
    sed -i '/torch/d' requirements/build/cpu.txt && \
    pip install -r requirements/build/cpu.txt

RUN set -eux; \
    GOMP_SO="$(g++-13 -print-file-name=libgomp.so)"; \
    echo "Using libgomp: ${GOMP_SO}"; \
    test "${GOMP_SO}" != "libgomp.so"; \
    test -f "${GOMP_SO}"; \
    if ! grep -q 'GNU OpenMP runtime' \
            cmake/cpu_extension.cmake; then \
        sed -i \
            "/find_library(OPEN_MP/i set(OPEN_MP \"${GOMP_SO}\" CACHE FILEPATH \"GNU OpenMP runtime for RISC-V\" FORCE)" \
            cmake/cpu_extension.cmake; \
    fi; \
    grep -n -B 2 -A 4 'OPEN_MP' cmake/cpu_extension.cmake; \
    . "$HOME/.cargo/env"; \
    export LLVM_CONFIG="$(command -v llvm-config)"; \
    MAX_JOBS=4 \
    VLLM_TARGET_DEVICE=cpu \
    python3 setup.py bdist_wheel

FROM base AS release

COPY --from=build /workspace/vllm/dist /workspace/vllm/dist

RUN pip install dist/*.whl

WORKDIR /workspace

ENTRYPOINT ["python", "-m", "vllm.entrypoints.openai.api_server"]