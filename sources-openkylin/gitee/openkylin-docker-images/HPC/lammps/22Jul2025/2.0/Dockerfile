ARG BASE=openkylin/openkylin:2.0
FROM ${BASE} AS builder

ARG VERSION=22Jul2025

RUN apt update && \
    apt install -y wget vim build-essential libopenmpi-dev openmpi-bin libmpich-dev && \
    apt clean all && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN wget https://github.com/lammps/lammps/archive/refs/tags/stable_${VERSION}.tar.gz \
    && tar -zxvf stable_${VERSION}.tar.gz \
    && rm -f stable_${VERSION}.tar.gz

WORKDIR /opt/lammps-stable_${VERSION}

RUN cp examples/melt/in.melt  src/

WORKDIR /opt/lammps-stable_${VERSION}/src

RUN make mpi -j

CMD ["./lmp_mpi", "-in", "in.melt"]