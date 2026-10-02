# SPDX-License-Identifier: Apache-2.0

# Build the inference-agent binary
ARG BASE=openkylin/go:1.24.1-ok20
ARG VERSION=v0.15.2

FROM ${BASE} AS builder

ARG VERSION

RUN apt update -y && \
    apt install -y git && \
    apt clean all && \
    rm -rf /var/lib/apt/lists/*

RUN groupadd -g 1001 kserve && \
    useradd -u 1001 -g 1001 -s /sbin/nologin --badname 65532

# Copy in the go src
WORKDIR /go/src/github.com/kserve/kserve

ARG KSERVE_REPO=https://github.com/kserve/kserve.git
RUN git clone -b $VERSION $KSERVE_REPO && \
    cp -r kserve/go.mod go.mod && \
    cp -r kserve/go.sum go.sum && \
    cp -r kserve/cmd cmd && \
    cp -r kserve/pkg pkg && \
    cp -r kserve/LICENSE LICENSE && \
    rm -fr kserve

RUN go mod download

# Build
RUN CGO_ENABLED=0 GOOS=linux go build -a -o agent ./cmd/agent

# Generate third-party licenses
RUN go install github.com/google/go-licenses@latest
# Forbidden Licenses: https://github.com/google/licenseclassifier/blob/e6a9bb99b5a6f71d5a34336b8245e305f5430f99/license_type.go#L341
RUN go-licenses check ./cmd/... ./pkg/... --disallowed_types="forbidden,unknown"
RUN go-licenses save --save_path third_party/library ./cmd/agent

# Copy the inference-agent into a thin image
FROM openkylin/distroless-base-noroot:2.0

COPY --from=builder /etc/passwd /etc/passwd
COPY --from=builder /etc/group /etc/group

COPY --from=builder /go/src/github.com/kserve/kserve/third_party /third_party
WORKDIR /ko-app
COPY --from=builder /go/src/github.com/kserve/kserve/agent /ko-app/
ENTRYPOINT ["/ko-app/agent"]
