FROM openkylin/openkylin:2.0

ARG TARGETARCH
ARG VERSION=2026.4.16

WORKDIR /root

RUN apt update && apt install -y curl git && \
    curl -LsSf https://astral.sh/uv/install.sh | sh && \
    . $HOME/.local/bin/env && \
    if [ "${TARGETARCH}" = "riscv64" ]; then \
        apt install -y gcc libffi-dev libssl-dev pkg-config && \
        curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y && \
        . "$HOME/.cargo/env" ; \
    fi && \
    git clone -b v$VERSION --recurse-submodules https://github.com/NousResearch/hermes-agent.git && \
    cd hermes-agent && uv venv venv --python 3.11 --seed && export VIRTUAL_ENV="$(pwd)/venv" && \
    export PATH="$VIRTUAL_ENV/bin:$PATH" && uv pip install -e "." && cd ..

WORKDIR /root/hermes-agent

ENV VIRTUAL_ENV=/root/hermes-agent/venv
ENV PATH="/root/hermes-agent/venv/bin:${PATH}"

CMD ["/bin/bash"]
