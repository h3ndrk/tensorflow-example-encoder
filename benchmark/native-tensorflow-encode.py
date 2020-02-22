import tensorflow as tf
import sys

# def _bytes_feature(value):
#    """Returns a bytes_list from a string / byte."""
#    if isinstance(value, type(tf.constant(0))):
#        value = value.numpy() # BytesList won't unpack a string from an EagerTensor.
#    return tf.train.Feature(bytes_list=tf.train.BytesList(value=value))

# def _float_feature(value):
#     """Returns a float_list from a float / double."""
#     return tf.train.Feature(float_list=tf.train.FloatList(value=value))

def _int64_feature(value):
    """Returns an int64_list from a bool / enum / int / uint."""
    return tf.train.Feature(int64_list=tf.train.Int64List(value=value))

if len(sys.argv) != 3:
    raise ValueError(f'usage: {sys.argv[0]} (input.png) (output.pb)')

image = tf.io.decode_png(tf.io.read_file(sys.argv[1]))
image_shape = tf.shape(image)
with open(sys.argv[2], "wb") as f:
    f.write(tf.train.Example(features=tf.train.Features(feature={
        'image': _int64_feature(tf.reshape(image, [-1])),
        'image_shape': _int64_feature(image_shape),
    })).SerializeToString())
