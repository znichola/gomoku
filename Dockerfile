# ---- Build stage ----
FROM alpine:3.24 AS builder

RUN apk add --no-cache g++ make nodejs npm

WORKDIR /app
COPY . .

# -Werror dropped here: g++ is pickier than clang apparently
# -pthread needed because it's on alpine which uses musl
RUN make -C back fclean \
 && make -C back gomoku CXX=g++ CXXFLAGS="-Wall -Wextra -std=c++20 -O2 -pthread" \
 && strip back/gomoku \
 && make -C front build

# ---- Final image ----
FROM alpine:3.24

# libstdc++: runtime lib the binary needs
RUN apk add --no-cache libstdc++

WORKDIR /app
COPY --from=builder /app/back/gomoku back/gomoku
COPY --from=builder /app/front/dist front/dist

EXPOSE 9012

WORKDIR /app/back
ENTRYPOINT ["./gomoku"]

# docker build -t znichola/gomoku:latest .

# test locally
# docker run --rm -p 9012:9012 znichola/gomoku

# docker push znichola/gomoku:latest
