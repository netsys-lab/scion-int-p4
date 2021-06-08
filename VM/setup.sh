# Development Environment Setup Script
# ------------------------------------


### Install Docker (for SCION)
# See install_docker.sh


### Install SCION development environment
#### https://scion.docs.anapaya.net/en/latest/build/setup.html
cd ~
git clone https://github.com/scionproto/scion
cd scion
./tools/install_bazel
APTARGS='-y' ./env/deps
sudo apt install -y python-is-python3
source ~/.profile
./scion.sh bazel_remote
./scion.sh build
docker stop bazel-remote-cache


### Install go (for SCION Apps)
#### https://golang.org/doc/install
cd ~
curl -fsSL -O https://golang.org/dl/go1.16.4.linux-amd64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.16.4.linux-amd64.tar.gz
rm go1.16.4.linux-amd64.tar.gz
echo 'PATH=$PATH:/usr/local/go/bin' >> ~/.profile
source ~/.profile


### Install SCION Apps
#### https://github.com/netsec-ethz/scion-apps
cd ~
git clone https://github.com/netsec-ethz/scion-apps.git
cd scion-apps

#### Dependencies
sudo apt-get install -y libpam0g-dev
curl -fsSL -O https://raw.githubusercontent.com/golangci/golangci-lint/master/install.sh
sudo sh ./install.sh -b /usr/local/go/bin v1.40.1
rm install.sh

### Build
make -j $(nproc)
sudo cp -t /usr/local/go/bin bin/scion-*


### Install nanomsg
#### https://github.com/nanomsg/nanomsg
cd ~
git clone https://github.com/nanomsg/nanomsg.git
cd nanomsg
sudo apt-get install -y gcc cmake
mkdir build
cd build
cmake ..
cmake --build .
sudo cmake --build . --target install
sudo ldconfig


### Install bmv2
#### https://github.com/p4lang/behavioral-model
cd ~
git clone https://github.com/p4lang/behavioral-model.git
cd behavioral-model

#### Dependencies
sudo apt-get install -y automake cmake libjudy-dev libgmp-dev libpcap-dev \
libboost-dev libboost-test-dev libboost-program-options-dev libboost-system-dev \
libboost-filesystem-dev libboost-thread-dev libevent-dev libtool flex bison \
pkg-config g++ libssl-dev
sudo apt-get install -y thrift-compiler libthrift-dev libnanomsg-dev
sudo pip3 install thrift

#### Build
./autogen.sh
./configure --enable-debugger --with-nanomsg --with-thrift
make -j $(nproc)
sudo make install
sudo ldconfig


### Install PI
#### https://github.com/p4lang/PI
cd ~
git clone --recursive https://github.com/p4lang/PI.git
cd PI

#### Dependencies
sudo apt-get install -y libprotobuf-dev libgrpc-dev libgrpc++-dev protobuf-compiler \
protobuf-compiler-grpc libjudy-dev libboost-thread-dev libreadline-dev

#### Fix for building with Apache Thrift 0.13.0 (see https://github.com/p4lang/PI/issues/533)
sed -i -e 's#::stdcxx::shared_ptr#std::shared_ptr#g' targets/bmv2/conn_mgr.cpp

#### Build
./autogen.sh
./configure --with-bmv2 --with-proto --with-fe-cpp --with-internal-rpc --with-cli
make -j $(nproc)
sudo make install
sudo ldconfig


### Install bmv2 simple_switch_grpc
cd ~/behavioral-model
./configure --enable-debugger --with-nanomsg --with-thrift --with-pi
make -j $(nproc)
sudo make install
sudo ldconfig
cd ~/behavioral-model/targets/simple_switch_grpc
./autogen.sh
./configure --with-thrift
make -j $(nproc)
sudo make install
sudo ldconfig


### Install P4 Compiler
#### https://github.com/p4lang/p4c
cd ~
git clone --recursive https://github.com/p4lang/p4c.git
cd p4c

#### Dependencies
sudo apt-get install -y cmake g++ git automake libtool libgc-dev bison flex \
libfl-dev libgmp-dev libboost-dev libboost-iostreams-dev \
libboost-graph-dev llvm pkg-config tcpdump doxygen graphviz

#### eBPF backend dependencies
sudo apt-get install -y clang llvm libpcap-dev libelf-dev

#### Python dependencies
sudo pip3 install scapy ply

#### Build
python3 backends/ebpf/build_libbpf
mkdir build
cd build
cmake ..
make -j $(nproc)
sudo make install


### Install P4 examples
#### https://github.com/lschulz/p4-examples
cd ~
git clone https://github.com/lschulz/p4-examples.git
cd p4-examples

#### Dependencies
sudo apt-get install -y jq yq mininet
sudo pip3 install mininet

#### Build
cd simple_switch/l2_switch_grpc
make
