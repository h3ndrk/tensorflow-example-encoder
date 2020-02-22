# Tensorflow Example Encoder

Currently, this project is the result of a one day programming idea. The `benchmark`-directory contains some tests about different approaches on encoding/serializing [`tf.train.Example`](https://www.tensorflow.org/tutorials/load_data/tfrecord#tfexample), the [Protocol Buffers](https://developers.google.com/protocol-buffers) format commonly used in Tensorflow's [TFRecord format](https://www.tensorflow.org/tutorials/load_data/tfrecord#tfrecords_format_details). TFRecords are commonly used for storing machine learning training data.

## Benchmark results

In all benchmarks, a PNG image (640x480x3 `uint8`) gets encoded into a protobuf message. The shape of the image data is also stored in a separate feature.

| Approach | Measured time |
|---|---|
| Standard [`tf.train.Example`](https://www.tensorflow.org/tutorials/load_data/tfrecord#tfexample) in Python | 1m 19.747s |
| Python Protobuf Serialization | 0m 0.723s |
| C++ Protobuf Serialization | 0m 0.282s |
| Custom C++ Protobuf Serialization | 0m 0.162s |

The normal Protocol Buffers implementation use generated code from the `*.proto` files of the Tensorflow project compiled via `protoc`.

The custom C++ Protocol Buffers Serialization uses a custom and hand-crafted implementation of the message serialization instead of the generated code. It is written for larger lists of data (for the PNG images).
