#!/bin/bash
set -e

# Install required packages
sudo apt update
sudo apt install -y \
  git \
  clang \
  lld \
  make \
  mingw-w64 \
  cargo \
  build-essential

# Clone musl repository
git clone https://github.com/kraj/musl.git
cd musl

# Create an out-of-tree build directory
mkdir -p build
cd build

# Configure musl for installation (default to /usr/local/musl)
../configure --prefix=/usr/local/musl

# Build and install
make -j$(nproc)
sudo make install
