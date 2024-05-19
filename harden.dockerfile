FROM alpine:3.19 AS permissions-giver

WORKDIR /out

FROM alpine:3.19 AS submodule-initializor

# Install git
RUN apk --no-cache add git

WORKDIR /out
COPY .git .git
COPY lib lib
COPY .gitmodules .

RUN git rev-parse --short=8 HEAD > /rev
# Get submodules and remove unneccesary files
RUN git submodule update --init --recursive \
    && rm -rf `find . -type d -name ".git"` \
    && rm .gitmodules

FROM alpine:3.19 AS builder

# Install cmake
RUN apk --no-cache add cmake clang clang-dev make gcc g++ libc-dev linux-headers

# Build
COPY include include
COPY src src
COPY Installer.iss.in .
COPY CMakeLists.txt .
COPY cmake cmake
COPY --from=submodule-initializor /out .
COPY --from=submodule-initializor /rev .
RUN cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DARK_BUILD_EXE=On -DARK_ENABLE_SYSTEM=Off -DARK_COMMIT="$(cat rev)" \
    && cmake --build build --target arkscript -- -j $(nproc)

FROM alpine:3.19 AS organizer

# Files needed to run Ark
WORKDIR /out/ark
COPY --from=builder build build
COPY --from=builder include include
COPY --from=builder lib lib

FROM alpine:3.19 AS runner

# Install cmake
RUN apk --no-cache add cmake
RUN apk add --update duo_unix

# Install Ark
COPY --from=organizer /out/ark .
RUN cmake --install build --config Release
ENV LD_LIBRARY_PATH=/usr/local/lib64
ENV ARKSCRIPT_PATH=/usr/local/lib/Ark

COPY harden_docker.sh /usr/sbin/harden.sh
RUN chmod u+x /usr/sbin/harden.sh
RUN /usr/sbin/harden.sh

USER user
WORKDIR /home/user
ENTRYPOINT [ "arkscript" ]
