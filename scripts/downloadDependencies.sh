#!/bin/bash
# This script downloads all external dependencies

checkLastSuccess() {
  # shellcheck disable=SC2181
  if [[ $? -ne 0 ]]; then
    echo "Install Error: $1"
    exit 1
  fi
}

mkdir ~/temp

# preparation
sudo apt-get update -y &&
  sudo apt-get install -y wget tar git build-essential apt-utils &&
  # To work around "E: The method driver /usr/lib/apt/methods/https could not be found." issue
  sudo apt-get install -y apt-transport-https ca-certificates &&
  # download cmake 3.24
  cd ~/temp && version=3.24 && build=1 &&
  wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz &&
  tar -xzvf cmake-$version.$build.tar.gz &&
  mv cmake-$version.$build cmake
# download and install clang/clang++ 6.0.1
CLANG6=$(clang-6.0 --version | grep "6.0")
if [ -z "$CLANG6" ]; then
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add - &&
    sudo apt-get install -y software-properties-common &&
    sudo apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" &&
    sudo apt-get update -y &&
    sudo apt-get install -y clang-6.0
  checkLastSuccess "install clang 6.0 fails"
else
  echo "clang 6.0 has been installed, skip"
fi
# download and install gcc/g++
GCC9=$(g++-9 --version | grep "9")
if [ -z "$GCC9" ]; then
  sudo apt-get install -y software-properties-common &&
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y &&
    sudo apt-get update -y &&
    sudo apt-get install -y gcc-9 g++-9 gdb
  checkLastSuccess "install g++ 9 fails"
else
  echo "g++ 9.3 has been installed, skip"
fi
# download prometheus cpp client
sudo apt-get install -y libcurl4-gnutls-dev &&
  cd ~/temp && version=v0.4.2 &&
  git clone -b $version https://github.com/jupp0r/prometheus-cpp.git &&
  cd prometheus-cpp/ && git submodule init && git submodule update
# download grpc and related components
# https://stackoverflow.com/questions/6842687/the-remote-end-hung-up-unexpectedly-while-git-cloning
git config --global http.postBuffer 1048576000
git config --global https.postBuffer 1048576000
git config --global --unset http.proxy
git config --global --unset https.proxy
git config --global http.sslVerify false
cd ~/temp && version=1.16 && build=1 &&
  git clone https://github.com/grpc/grpc &&
  cd grpc && git fetch --all --tags --prune &&
  git checkout tags/v$version.$build -b v$version.$build &&
  git submodule update --init
# download and install pre-requisites for protobuf and grpc
sudo apt-get install -y autoconf automake libtool curl make unzip libssl-dev
# download and install pre-requisites for rocksdb
sudo apt-get install -y libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev
# download rocksdb
cd ~/temp &&
  git clone https://github.com/facebook/rocksdb.git &&
  cd rocksdb &&
  git checkout v6.5.2 &&
  git submodule update --init --recursive
# download and install tools for code coverage
sudo apt-get install -y lcov
# download and install tools required by gringofts
sudo apt-get install -y libcrypto++-dev &&
  sudo apt-get install -y doxygen &&
  sudo apt-get install -y python=3 &&
  sudo apt-get install -y python3-pip &&
  sudo ln -s /usr/bin/python3 /usr/bin/python &&
  # Must use gcovr 4.2-1 as later version will create *.gcov files under tmp dir and remove them afterwards.
  # The consequence is we cannot upload these gcov files to sonarcube
  sudo apt-get install -y gcovr=4.2-1
# download and install sqlite3
sudo apt-get install -y sqlite3 libsqlite3-dev
# download and install boost
sudo apt-get install -y libboost-all-dev
# download and install gettext (for envsubst)
sudo apt-get install -y gettext

# cleanup
sudo apt autoremove
