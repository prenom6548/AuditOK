ARG BASE=openkylin/openkylin:2.0
FROM ${BASE} AS builder

ARG VERSION=6.6.2

RUN apt update && \
    apt install -y git g++ cmake wget libselinux1-dev \
        meson intltool python3-gi desktop-file-utils valac \
        libxml2-dev libsm-dev libexif-dev libgsf-1-dev libexempi-dev \
        libgirepository1.0-dev libnotify-dev iso-codes \
        libgtk-3-dev libjson-glib-dev xkb-data libgnomekbd-dev \
        libpulse-dev libsystemd-dev libxkbfile-dev \
        libdbusmenu-gtk3-dev libudev-dev python-gi-dev libgail-3-dev && \
    apt clean all && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN git clone https://github.com/linuxmint/cinnamon-desktop.git \
    && cd cinnamon-desktop \
    && meson setup build \
    && ninja -C build \
    && ninja -C build install

RUN git clone https://github.com/linuxmint/xapp.git \
    && cd xapp \
    && meson setup build \
    && ninja -C build \
    && ninja -C build install

RUN wget https://github.com/linuxmint/nemo/archive/refs/tags/${VERSION}.tar.gz \
    && tar -zxvf ${VERSION}.tar.gz \
    && rm -f ${VERSION}.tar.gz


ENV PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH

WORKDIR /opt/nemo-${VERSION}

RUN meson setup build -Ddeprecated_warnings=false -Dgtk_doc=false -Dselinux=true \
    && meson compile -C build \
    && meson install -C build

ENV LD_LIBRARY_PATH=/usr/local/lib64:$LD_LIBRARY_PATH

CMD ["nemo", "--help"]