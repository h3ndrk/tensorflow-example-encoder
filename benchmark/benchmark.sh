#!/bin/bash

################################################################################
# test native tensorflow encoding
docker run \
    --interactive \
    --rm \
    --volume "$(pwd):/data" \
    --user "$(id -u):$(id -g)" \
    --workdir /data \
    tensorflow/tensorflow:latest-py3 \
    bash -c "time python native-tensorflow-encode.py test.png native-tensorflow-encode.pb"
# real  1m19.747s
# user  1m18.083s
# sys   0m0.832s

################################################################################
# test python protobuf encoding
if [ ! -d venv ]; then
    virtualenv venv
fi
source venv/bin/activate
pip install imageio protobuf

protoc --python_out=. example.proto feature.proto

time python python-protobuf-encoder.py test.png python-protobuf-encoder.pb
# real  0m0.723s
# user  0m0.702s
# sys   0m0.305s

################################################################################
# test C++ protobuf encoding
g++ -c -o example.pb.o example.pb.cc
g++ -c -o feature.pb.o feature.pb.cc
g++ -c -o cpp-protobuf-encoder.o cpp-protobuf-encoder.cc
g++ -o cpp-protobuf-encoder example.pb.o feature.pb.o cpp-protobuf-encoder.o -lprotobuf

time ./cpp-protobuf-encoder test.png cpp-protobuf-encoder.pb
# real  0m0.282s
# user  0m0.268s
# sys   0m0.013s

################################################################################
# test custom C++ protobuf encoding
cd custom-protobuf-encoder
rm -Rf build
mkdir build
cd build
cmake ..
make
cd ../..
time custom-protobuf-encoder/build/custom-protobuf-encoder test.png custom-protobuf-encoder.pb
# real  0m0.109s
# user  0m0.101s
# sys   0m0.007s
