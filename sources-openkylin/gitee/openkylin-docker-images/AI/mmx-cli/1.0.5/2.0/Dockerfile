FROM openkylin/node:24.14.1-ok20

ARG VERSION=1.0.5

WORKDIR /root

RUN npm install -g mmx-cli@${VERSION} && \
    ln -s /opt/node/bin/mmx /usr/local/bin

CMD ["/bin/bash"]