FROM openkylin/go:1.26.1-ok20 AS build
WORKDIR /app
COPY . .
RUN go mod download
RUN CGO_ENABLED=0 go build -o hello

FROM openkylin/distroless-static:2.0
WORKDIR /app
COPY --from=build /app/hello .
CMD ["./hello"]
