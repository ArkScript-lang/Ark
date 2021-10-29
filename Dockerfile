FROM alpine:3.12 AS permissions-giver

WORKDIR /out

FROM alpine:3.12 AS submodule-initializor

# Install git
RUN apk --no-cache add git

WORKDIR /out
COPY .git .git
COPY lib lib
COPY .gitmodules .

# Get submodules and remove unneccesery files
RUN git submodule update --init --recursive \
    && rm -rf `find . -type d -name ".git"` \
    && rm .gitmodules

FROM alpine:3.12 AS builder

# Install cmake
RUN apk --no-cache add cmake clang clang-dev make gcc g++ libc-dev linux-headers

# Build
COPY include include
COPY src src
COPY Installer.iss.in .
COPY CMakeLists.txt .
COPY --from=submodule-initializor /out .
RUN cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=On \
    && cmake --build build --target arkscript

FROM alpine:3.12 AS organizer

# Files needed to run Ark
WORKDIR /out/ark
COPY --from=builder build build
COPY --from=builder include include
COPY --from=builder lib lib

FROM alpine:3.12 AS runner

# Install cmake
RUN apk --no-cache add cmake

# Install Ark
COPY --from=organizer /out/ark .
RUN cmake --install build --config Release

ENTRYPOINT [ "arkscript" ]
