ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}

ARG TARGETARCH
ARG VERSION=4.20.4

RUN apt update && \
    apt install -y git curl build-essential && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

RUN git clone -b v${VERSION} https://atomgit.com/atomgit_atomcode/atomcode.git && \
    cd atomcode && . "$HOME/.cargo/env" && cargo build --release && \
    ln -s  "$PWD/target/release/atomcode" /usr/local/bin/ 

WORKDIR /root

CMD ["/bin/bash"]