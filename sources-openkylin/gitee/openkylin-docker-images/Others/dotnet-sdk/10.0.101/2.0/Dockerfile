ARG BASE=openkylin/openkylin:2.0
FROM ${BASE}

ARG TARGETARCH
ARG VERSION=10.0.101

ENV ASPNETCORE_URLS=http://+:8080 \
    DOTNET_RUNNING_IN_CONTAINER=true \
    DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=true

RUN apt update && apt install -y curl && \
    if [ "$TARGETARCH" = "amd64" ]; then \
      curl -fSL -o dotnet.tar.gz https://builds.dotnet.microsoft.com/dotnet/Sdk/${VERSION}/dotnet-sdk-${VERSION}-linux-x64.tar.gz; \
    elif [ "$TARGETARCH" = "arm64" ]; then \
      curl -fSL -o dotnet.tar.gz https://builds.dotnet.microsoft.com/dotnet/Sdk/${VERSION}/dotnet-sdk-${VERSION}-linux-arm64.tar.gz; \
    elif [ "$TARGETARCH" = "riscv64" ]; then \
      curl -fSL -o dotnet.tar.gz https://github.com/filipnavara/dotnet-riscv/releases/download/${VERSION}/dotnet-sdk-${VERSION}-linux-riscv64.tar.gz; \
    fi && \
    mkdir -p /dotnet && tar -zxvf dotnet.tar.gz -C /dotnet && \
    mv /dotnet /usr/share/dotnet && \
    rm -f dotnet.tar.gz && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/share/dotnet/dotnet /usr/bin/dotnet

ENTRYPOINT [ "dotnet" ]

CMD [ "--info" ]