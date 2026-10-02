ARG BASE=openkylin/go:1.25.4-ok20
FROM ${BASE} AS builder

ARG VERSION=9.2.3
RUN apt update && \
    apt install -y make g++ python3 wget && \
    apt clean all && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN wget https://github.com/elastic/beats/archive/refs/tags/v${VERSION}.tar.gz \
    && tar -zxvf v${VERSION}.tar.gz \
    && rm -f v${VERSION}.tar.gz

WORKDIR /opt/beats-${VERSION}/filebeat

RUN make -j && mv filebeat /usr/local/bin/

FROM ${BASE}

COPY --from=builder /usr/local/bin/filebeat /usr/local/bin/filebeat

CMD ["filebeat", "--help"]