ARG BASE=openkylin/openkylin:2.0
FROM ${BASE} AS builder

RUN groupadd -g 65532 nonroot && \
    useradd -u 65532 -g 65532 -s /sbin/nologin nonroot

FROM openkylin/distroless-base:2.0

COPY --from=builder /etc/passwd /etc/passwd
COPY --from=builder /etc/group /etc/group

USER nonroot