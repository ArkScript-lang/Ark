FROM alpine:3.23 AS permissions-giver

WORKDIR /out

FROM alpine:3.23 AS submodule-initializor

# Install git
RUN apk --no-cache add git

WORKDIR /out
COPY .git .git
COPY lib lib
COPY thirdparties thirdparties
COPY .gitmodules .

RUN git rev-parse --short=8 HEAD > /rev
# Get submodules and remove unneccesary files
RUN git submodule update --init --recursive \
    && rm -rf `find . -type d -name ".git"` \
    && rm .gitmodules

FROM alpine:3.23 AS builder

# Install cmake
RUN apk --no-cache add cmake clang make libc-dev linux-headers

# Build
COPY include include
COPY src src
COPY CMakeLists.txt .
COPY cmake cmake
COPY --from=submodule-initializor /out .
COPY --from=submodule-initializor /rev .
RUN cmake -H. -Bbuild \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DARK_BUILD_EXE=On \
    -DARK_UNITY_BUILD=On \
    -DARK_BUILD_MODULES=On \
    -DARK_REQUESTED_MODULES='re,hash,console,' \
    -DARK_COMMIT="$(cat rev)" \
    -DARK_BUILD_DATE="$(date +%Y-%m-%dT%H:%M:%SZ)" \
    && cmake --build build -- -j $(nproc)

FROM alpine:3.23 AS organizer

# Files needed to run Ark
WORKDIR /out/ark
COPY --from=builder build build
COPY --from=builder include include
COPY --from=builder lib lib

FROM alpine:3.23 AS runner

# Install cmake
RUN apk --no-cache add cmake

# Install Ark
COPY --from=organizer /out/ark .
RUN cmake --install build --strip --config Release
ENV LD_LIBRARY_PATH=/usr/local/lib64
ENV ARKSCRIPT_PATH=/usr/local/lib/Ark

ENTRYPOINT [ "arkscript" ]
