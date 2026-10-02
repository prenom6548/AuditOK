FROM openkylin/openkylin:2.0

ARG TARGETARCH
ARG VERSION=0.8.15

WORKDIR /root

RUN apt update && \
    apt install -y curl wget pkg-config libdbus-1-dev libatomic1 build-essential cmake perl && \
    apt-get clean && rm -rf /var/lib/apt/lists/* && \
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y && \
    . "$HOME/.cargo/env"

RUN wget https://github.com/Hmbown/CodeWhale/archive/refs/tags/v${VERSION}.tar.gz && \
    tar -xf v${VERSION}.tar.gz && cd CodeWhale-${VERSION} && . "$HOME/.cargo/env" && \
    cargo install --path crates/cli --locked && \
    cargo install --path crates/tui --locked

CMD ["/bin/bash"]