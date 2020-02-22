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
# real  1m20.933s
# user  1m20.012s
# sys   0m0.562s

################################################################################
# test custom C++ protobuf implementation
cp ../build/tensorflow-example-encoder .
time ./tensorflow-example-encoder test.png tensorflow-example-encoder.pb
# real  0m0.126s
# user  0m0.092s
# sys   0m0.032s

################################################################################
# test python protobuf encoding
if [ ! -d venv ]; then
    virtualenv venv
fi
source venv/bin/activate
pip install imageio protobuf

protoc --python_out=. example.proto feature.proto

time python python-protobuf-encoder.py test.png python-protobuf-encoder.pb
# real  0m0.732s
# user  0m0.712s
# sys   0m0.276s

################################################################################
# test C++ protobuf encoding
g++ -c -o example.pb.o example.pb.cc
g++ -c -o feature.pb.o feature.pb.cc
g++ -c -o cpp-protobuf-encoder.o cpp-protobuf-encoder.cc
g++ -o cpp-protobuf-encoder example.pb.o feature.pb.o cpp-protobuf-encoder.o -lprotobuf

time ./cpp-protobuf-encoder test.png cpp-protobuf-encoder.pb
# real  0m0.255s
# user  0m0.237s
# sys   0m0.016s
