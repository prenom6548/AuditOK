FROM openkylin/node:25.2.1-ok20

ARG VERSION=0.12.6

WORKDIR /root

RUN apt update && apt install -y jq && \
    npm install -g @qwen-code/qwen-code@${VERSION} && \
    ln -s /opt/node/bin/qwen /usr/local/bin

CMD ["/bin/bash"]
