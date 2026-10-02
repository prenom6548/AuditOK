FROM openkylin/node:25.2.1-ok20

ARG VERSION=0.36.0

WORKDIR /root

RUN npm install -g @google/gemini-cli@${VERSION} && \
    ln -s /opt/node/bin/gemini /usr/local/bin

CMD ["/bin/bash"]
