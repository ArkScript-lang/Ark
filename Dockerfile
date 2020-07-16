FROM alpine:3.12 AS permissions-giver

# Make sure docker-entrypoint.sh is executable, regardless of the build host.
WORKDIR /out
COPY docker-entrypoint.sh .
RUN chmod +x docker-entrypoint.sh

FROM alpine:3.12 AS submodule-initializor

# Install git
RUN apk --no-cache add git 

WORKDIR /out
COPY .git .git
COPY submodules submodules
COPY .gitmodules .

# Get submodules and remove unneccesery files
RUN git submodule update --init --recursive \
    && rm -rf `find . -type d -name ".git"` \
    && rm .gitmodules

FROM alpine:3.12 AS organizer

# Files for the build process
WORKDIR /out/build
COPY include include
COPY lib lib
COPY src src
COPY thirdparty thirdparty
COPY CMakeLists.txt .
COPY --from=submodule-initializor /out /out/build

# The docker-entrypoint.sh file
COPY --from=permissions-giver /out/docker-entrypoint.sh /out/docker/docker-entrypoint.sh

FROM alpine:3.12 AS builder

# Install cmake
RUN apk --no-cache add cmake clang clang-dev make gcc g++ libc-dev linux-headers

# Build and install
COPY --from=organizer /out/build .
RUN cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=1 \
    && cmake --build build \
    && cmake --install build --config Release

COPY --from=organizer /out/docker /usr/local/bin
ENTRYPOINT [ "docker-entrypoint.sh" ]
CMD [ "Ark" ]