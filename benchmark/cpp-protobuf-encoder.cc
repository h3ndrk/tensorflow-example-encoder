#include <fstream>
#include "example.pb.h"
#include "feature.pb.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
    return 1;

  tensorflow::Example example;
  (*example.mutable_features()->mutable_feature())["image"].mutable_int64_list()->add_value(42);

  int width = 0;
  int height = 0;
  int colors_per_pixel = 0;
  uint8_t *data = stbi_load(argv[1], &width, &height, &colors_per_pixel, 0);
  for (int i = 0; i < width * height * colors_per_pixel; ++i)
  {
    (*example.mutable_features()->mutable_feature())["image"].mutable_int64_list()->add_value(data[i]);
  }
  stbi_image_free(data);
  (*example.mutable_features()->mutable_feature())["image_shape"].mutable_int64_list()->add_value(height);
  (*example.mutable_features()->mutable_feature())["image_shape"].mutable_int64_list()->add_value(width);
  (*example.mutable_features()->mutable_feature())["image_shape"].mutable_int64_list()->add_value(colors_per_pixel);

  std::ofstream out;
  out.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  out.open(argv[2], std::ios_base::out | std::ios_base::trunc);
  if (out.is_open())
  {
    example.SerializePartialToOstream(&out);
  }
  out.close();

  return 0;
}
