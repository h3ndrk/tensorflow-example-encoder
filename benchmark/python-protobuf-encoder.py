import sys
import imageio
import example_pb2
import feature_pb2

if len(sys.argv) != 3:
    raise ValueError(f'usage: {sys.argv[0]} (input.png) (output.pb)')

example = example_pb2.Example()

image = imageio.imread(sys.argv[1])
example.features.feature["image"].int64_list.value[:] = image.ravel()
example.features.feature["image_shape"].int64_list.value[:] = image.shape

with open(sys.argv[2], "wb") as f:
    f.write(example.SerializeToString())

# with open("build/bytes.pb") as f:
#     features = Features()
#     features.ParseFromString(f.read().encode('utf-8'))
#     print(features)

# with open("build/int64.pb") as f:
#     features = Features()
#     features.ParseFromString(f.read().encode('utf-8'))
#     print(features)

# with open("build/float.pb", "rb") as f:
#     features = Features()
#     features.ParseFromString(f.read())
#     print(features)

# with open("build/example.pb", "rb") as f:
#     example = Example()
#     example.ParseFromString(f.read())
#     print(example)
