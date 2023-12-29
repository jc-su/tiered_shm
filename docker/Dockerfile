FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Add the LLVM repository
RUN apt-get update && \
    apt-get install -y software-properties-common wget jq curl gnupg2 && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/trusted.gpg.d/llvm-archive-keyring.gpg && \
    echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy main" > /etc/apt/sources.list.d/llvm.list && \
    echo "deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy main" >> /etc/apt/sources.list.d/llvm.list

# Install development tools, utilities, and zsh
RUN apt-get update && \
    apt-get install -y \
    clang-18 \
    clang-tidy-18 \
    clangd-18 \
    lldb \
    libstdc++-12-dev \
    libc++-18-dev \
    libc++abi-18-dev \
    git \
    unzip \
    zsh && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install latest CMake
RUN CMAKE_VERSION=$(wget -qO- "https://api.github.com/repos/Kitware/CMake/releases/latest" | jq -r .tag_name | tr -d 'v') && \
    wget -qO- "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-Linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local

RUN NINJA_VERSION=$(wget -qO- "https://api.github.com/repos/ninja-build/ninja/releases/latest" | jq -r .tag_name | tr -d 'v') && \
    wget -qO- "https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/ninja-linux.zip" | funzip > /usr/local/bin/ninja && \
    chmod +x /usr/local/bin/ninja

# print cmake version
RUN cmake --version

# Set clang as the default C++ compiler
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-18 100
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-18 100

# Install oh-my-zsh
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended

# COPY . /CPU_Sched
# Set working directory
COPY . /app

WORKDIR /app

# build
# RUN mkdir build && cd build && cmake .. -G Ninja && ninja

CMD ["/bin/zsh"]