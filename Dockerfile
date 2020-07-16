FROM alpine:3.12 AS organizer

# Files for the build process
WORKDIR /out/build
COPY .git .git
COPY include include
COPY lib lib
COPY src src
COPY submodules submodules
COPY thirdparty thirdparty
COPY .gitmodules .
COPY CMakeLists.txt .

# The docker-entrypoint.sh file
WORKDIR /out/docker
COPY docker-entrypoint.sh .
RUN chmod +x docker-entrypoint.sh

FROM alpine:3.12 AS builder

# Install git and cmake
RUN apk --no-cache add git cmake clang clang-dev make gcc g++ libc-dev linux-headers

# Build
COPY --from=organizer /out/build .
RUN git submodule update --init --recursive \
    && cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=1 \
    && cmake --build build \
    && cmake --install build --config Release

COPY --from=organizer /out/docker /usr/local/bin
ENTRYPOINT [ "docker-entrypoint.sh" ]
CMD [ "Ark" ]