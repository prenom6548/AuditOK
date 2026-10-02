FROM openkylin/openkylin:2.0 AS build-env
WORKDIR /app
COPY . .
RUN apt update && apt install -y gcc g++
RUN cc hello.c -o hello

FROM openkylin/distroless-base:2.0
WORKDIR /app
COPY --from=build-env hello .
CMD ["./hello"]
