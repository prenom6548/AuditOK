ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}

ARG TARGETARCH
ARG BUILDARCH
ARG VERSION=25.9.1-0

RUN apt update && \
    apt install -y wget && \
    apt clean && \
    rm -rf /var/lib/apt/lists/*

RUN if [ "$TARGETARCH" = "amd64" ]; then \
        wget https://github.com/conda-forge/miniforge/releases/download/${VERSION}/Miniforge3-${VERSION}-Linux-x86_64.sh -O ~/miniforge.sh; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
        wget https://github.com/conda-forge/miniforge/releases/download/${VERSION}/Miniforge3-${VERSION}-Linux-aarch64.sh -O ~/miniforge.sh; \
    elif [ "$TARGETARCH" = "riscv64" ]; then \
        wget https://conda.openkylin.top/miniforge/preinstall.sh.tar && \
        tar -xf preinstall.sh.tar && bash preinstall.sh && rm preinstall.sh.tar && \
        wget https://conda.openkylin.top/miniforge/Miniforge3-25.9.1-0-Linux-riscv64.sh.tar && \
        tar -xf Miniforge3-25.9.1-0-Linux-riscv64.sh.tar && mv Miniforge3-25.9.1-0-Linux-riscv64.sh ~/miniforge.sh && rm Miniforge3-25.9.1-0-Linux-riscv64.sh.tar; \
    fi; \
    bash ~/miniforge.sh -b -p /usr/local/miniforge && rm -f ~/miniforge.sh
    
RUN bash -lc "/usr/local/miniforge/bin/conda init && source /root/.bashrc"    

CMD ["/bin/bash"]
