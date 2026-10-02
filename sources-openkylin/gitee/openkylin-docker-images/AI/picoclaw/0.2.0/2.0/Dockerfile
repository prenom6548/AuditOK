# ============================================================
# Stage 1: Build the picoclaw binary
# ============================================================
FROM openkylin/go:1.26.1-ok20 AS builder

RUN apt-get update && apt-get install -y wget git make && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /

RUN wget https://github.com/sipeed/picoclaw/archive/refs/tags/v0.2.0.tar.gz && \
    tar -xzf v0.2.0.tar.gz && rm -rf v0.2.0.tar.gz && cd picoclaw-0.2.0 && \
    go mod download && make build

# ============================================================
# Stage 2: Node.js-based runtime with full MCP support
# ============================================================
FROM openkylin/node:25.2.1-ok20

RUN apt-get update && apt-get install -y ca-certificates curl git python3 python3-pip && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN curl -LsSf https://astral.sh/uv/install.sh | sh && \
    ln -s /root/.local/bin/uv /usr/local/bin/uv && \
    ln -s /root/.local/bin/uvx /usr/local/bin/uvx && \
    uv --version

COPY --from=builder /picoclaw-0.2.0/build/picoclaw /usr/local/bin/picoclaw

RUN /usr/local/bin/picoclaw onboard

ENTRYPOINT ["picoclaw"]
CMD ["gateway"]