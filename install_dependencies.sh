#!/bin/bash
set -e

sudo apt update
sudo apt install -y \
  git \
  clang \
  lld \
  make \
  mingw-w64 \
  cargo \
  build-essential

git clone https://github.com/kraj/musl.git
cd musl

mkdir -p build
cd build

../configure --prefix=/usr/local/musl

make -j$(nproc)
sudo make install
