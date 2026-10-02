FROM scratch
ARG TARGETARCH
ADD openKylin-docker-rootfs.$TARGETARCH.tar.xz /
RUN ln -sf /usr/share/zoneinfo/UTC /etc/localtime && \
    sed -i "s/TMOUT=300/TMOUT=0/g" /etc/bash.bashrc
CMD ["/bin/bash"]