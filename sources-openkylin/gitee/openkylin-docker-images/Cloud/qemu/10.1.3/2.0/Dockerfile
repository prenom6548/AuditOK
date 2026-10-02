ARG BASE=openkylin/python:3.12.12-ok20
FROM $BASE

ARG VERSION=10.1.3

RUN apt update && \
    apt install -y wget libfuse3-dev libdaxctl-dev \
    libkeyutils-dev liburing-dev libnfs-dev \
    libcurl4-openssl-dev libudev-dev libncurses-dev \
    libasound2-dev libjack-dev libpulse-dev \
    g++ make ninja-build meson pkg-config libglib2.0-dev \
    libfdt-dev libpixman-1-dev zlib1g-dev flex bison \
    libaio-dev libcap-ng-dev libiscsi-dev \
    libselinux-dev libseccomp-dev libusb-1.0-0-dev libzstd-dev && \
    apt clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN wget https://download.qemu.org/qemu-${VERSION}.tar.xz \
    && tar xvJf qemu-${VERSION}.tar.xz \
    && rm -f qemu-${VERSION}.tar.xz

WORKDIR /opt/qemu-${VERSION}

RUN ./configure \
    && make -j \
    && make install

CMD sh -c 'exec qemu-system-$(uname -m) --help'
