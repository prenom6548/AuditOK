ARG BASE=openkylin/openkylin:2.0
ARG VERSION=3.12.12

FROM ${BASE} AS builder

ARG VERSION
ARG TARGETARCH

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        make \
        cmake \
        curl \
        pkg-config \
        zlib1g-dev \
        libbz2-dev \
        libssl-dev \
        libncurses-dev \
        libsqlite3-dev \
        libreadline-dev \
        tk-dev \
        libgdbm-dev \
        libgdbm-compat-dev \
        libpcap-dev \
        xz-utils \
        liblzma-dev \
        libev-dev \
        libexpat1-dev \
        libffi-dev \
        systemtap-sdt-dev \
        ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN PY_HOME=/usr/local/python${VERSION} && \
    PY_INSTALLER_DIR=Python-${VERSION} && \
    PY_INSTALLER_TGZ=Python-${VERSION}.tgz && \
    curl -fsSL -o "/tmp/${PY_INSTALLER_TGZ}" "https://www.python.org/ftp/python/${VERSION}/${PY_INSTALLER_TGZ}" && \
    tar -xf /tmp/${PY_INSTALLER_TGZ} -C /tmp && \
    cd /tmp/${PY_INSTALLER_DIR} && \
    mkdir -p /${PY_HOME}/lib && \
    ./configure --prefix=${PY_HOME} --enable-shared LDFLAGS="-Wl,-rpath ${PY_HOME}/lib" && \
    make -j "$(nproc)" && \
    make altinstall && \
    rm -rf /tmp/*

FROM ${BASE} AS official

ARG VERSION

COPY --from=builder /usr/local/python${VERSION} /usr/local/python${VERSION}

RUN VERSION_SHORT=$(echo "${VERSION}" | cut -d. -f1,2) && \
    VERSION_MAJOR=$(echo "${VERSION}" | cut -d. -f1) && \
    PY_HOME=/usr/local/python${VERSION} && \
    ln -sf ${PY_HOME}/bin/python${VERSION_SHORT} ${PY_HOME}/bin/python${VERSION_MAJOR} && \
    ln -sf ${PY_HOME}/bin/pip${VERSION_SHORT} ${PY_HOME}/bin/pip${VERSION_MAJOR} && \
    ln -sf ${PY_HOME}/bin/python${VERSION_MAJOR} ${PY_HOME}/bin/python && \
    ln -sf ${PY_HOME}/bin/pip${VERSION_MAJOR} ${PY_HOME}/bin/pip

RUN echo "${VERSION}" > /test.txt

ENV PATH=/usr/local/python${VERSION}/bin:${PATH}