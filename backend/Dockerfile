FROM alpine:3.19
# Install build tools, pkg-config, C/C++ compilers, and cJSON dev package
RUN apk add --no-cache build-base pkgconfig libmicrohttpd-dev curl-dev cjson-dev libsodium-dev make python3
WORKDIR /app
COPY . .
RUN make
CMD ["./backend"]
EXPOSE 8000