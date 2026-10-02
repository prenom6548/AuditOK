ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}

ARG VERSION=11.9.0

RUN apt update && \
    apt install -y gcc ninja-build meson libxml2-utils xsltproc libyajl-dev systemd-dev libcap-ng-dev libnl-3-dev \
    wget xz-utils libcurl4-openssl-dev libssh2-1-dev libgnutls28-dev libxslt1-dev pkg-config perl \
    libglib2.0-dev libtirpc-dev libacl1-dev libaudit-dev libblkid-dev libbsd-dev libdevmapper-dev \
    libfuse3-dev libpciaccess-dev libselinux1-dev libiscsi-dev librados-dev librbd-dev \
    libpcap-dev bash-completion dmidecode iproute2 open-iscsi kmod \
    python3-docutils python3-pytest python3-flake8 iproute2  &&\
    apt clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN wget https://download.libvirt.org/libvirt-${VERSION}.tar.xz \
    && tar -xvf libvirt-${VERSION}.tar.xz \
    && rm -f libvirt-${VERSION}.tar.xz

WORKDIR /opt/libvirt-${VERSION}

RUN meson setup build --prefix=/usr/local \
    && ninja -C build \
    && ninja -C build install

CMD ["libvirtd", "--help"]