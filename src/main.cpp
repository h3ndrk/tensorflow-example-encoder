#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <png.h>

struct Varint
{
  uint64_t value;
  uint64_t size() const {
    if (value >= (1UL << (7 * 8)))
      return 9;
    else if (value >= (1UL << (7 * 7)))
      return 8;
    else if (value >= (1UL << (7 * 6)))
      return 7;
    else if (value >= (1UL << (7 * 5)))
      return 6;
    else if (value >= (1UL << (7 * 4)))
      return 5;
    else if (value >= (1UL << (7 * 3)))
      return 4;
    else if (value >= (1UL << (7 * 2)))
      return 3;
    else if (value >= (1UL << 7))
      return 2;
    return 1;
  }
  friend std::ostream& operator<<(std::ostream& out, const Varint& varint)
  {
    uint64_t value_nonconst = varint.value;
    while (value_nonconst > 0x7f)
    {
      out << static_cast<char>((value_nonconst & 0x7f) | ((value_nonconst > 0x7f) << 7));
      value_nonconst >>= 7;
    }

    out << static_cast<char>((value_nonconst & 0x7f) | ((value_nonconst > 0x7f) << 7));

    return out;
  }
  Varint(uint64_t value) : value(value) {}
};

template<typename T>
struct MapFieldEntry
{
  std::string key;
  std::vector<T> list;
  uint64_t sizeof_list_value_packed;
  uint64_t sizeof_list_value;
  uint64_t sizeof_list;
  uint64_t sizeof_feature;
  uint64_t sizeof_key;
  uint64_t size;
  void calculate_size() {
    sizeof_list_value = calculate_sizeof_list_value();
    sizeof_list = sizeof(uint8_t) + Varint{sizeof_list_value}.size() + sizeof_list_value;
    sizeof_feature = sizeof(uint8_t) + Varint{sizeof_list}.size() + sizeof_list;
    sizeof_key = sizeof(uint8_t) + Varint{key.size()}.size() + key.size();
    size = sizeof(uint8_t) + Varint{sizeof_key + sizeof_feature}.size() + sizeof_key + sizeof_feature;
  }
  uint64_t calculate_sizeof_list_value();
  friend std::ostream& operator<<(std::ostream& out, const MapFieldEntry& mfe) {
    // MapFieldEntry message header
    out << static_cast<char>(1 << 3 | 2) << Varint{mfe.sizeof_key + mfe.sizeof_feature};
    // string key = 1;
    out << static_cast<char>(1 << 3 | 2) << Varint{mfe.key.size()} << mfe.key;
    // Feature value = 2; Feature message header
    out << static_cast<char>(2 << 3 | 2) << Varint{mfe.sizeof_list};
    // oneof kind ...
    return mfe.encode_list(out);
  }
  std::ostream& encode_list(std::ostream& out) const;
  MapFieldEntry(const std::string& key, const std::vector<T>& list) : key(key), list(list) {
    calculate_size();
  }
};

template <>
uint64_t MapFieldEntry<std::string>::calculate_sizeof_list_value()
{
  uint64_t size = 0;
  for (auto& item : list)
  {
    size += sizeof(uint8_t) + Varint{item.size()}.size() + item.size();
  }
  return size;
}

template<>
std::ostream& MapFieldEntry<std::string>::encode_list(std::ostream& out) const
{
  // BytesList bytes_list = 1; BytesList message header
  out << static_cast<char>(1 << 3 | 2) << Varint{sizeof_list_value};
  // repeated bytes = 1;
  for (auto& item : list)
  {
    out << static_cast<char>(1 << 3 | 2) << Varint{item.size()} << item;
  }
  return out;
}

template <>
uint64_t MapFieldEntry<float>::calculate_sizeof_list_value()
{
  sizeof_list_value_packed = list.size() * sizeof(float);
  return sizeof(uint8_t) + Varint{sizeof_list_value_packed}.size() + sizeof_list_value_packed;
}

template<>
std::ostream& MapFieldEntry<float>::encode_list(std::ostream& out) const
{
  // FloatList float_list = 2; FloatList message header
  out << static_cast<char>(2 << 3 | 2) << Varint{sizeof_list_value};
  // repeated float = 1 [packed = true];
  out << static_cast<char>(1 << 3 | 2) << Varint{sizeof_list_value_packed};
  for (auto item : list)
  {
    union { float float_in; char bytes_out[sizeof(float)]; } u;
    u.float_in = item;
    out << std::string{u.bytes_out, sizeof(float)};
  }
  return out;
}

template <>
uint64_t MapFieldEntry<int64_t>::calculate_sizeof_list_value()
{
  uint64_t size = 0;
  for (auto item : list)
  {
    union { int64_t signed_in; uint64_t unsigned_out; } u;
    u.signed_in = item;
    size += Varint{u.unsigned_out}.size();
  }
  sizeof_list_value_packed = size;
  return sizeof(uint8_t) + Varint{size}.size() + size;
}

template<>
std::ostream& MapFieldEntry<int64_t>::encode_list(std::ostream& out) const
{
  // Int64List float_list = 3; Int64List message header
  out << static_cast<char>(3 << 3 | 2) << Varint{sizeof_list_value};
  // repeated int64 = 1 [packed = true];
  out << static_cast<char>(1 << 3 | 2) << Varint{sizeof_list_value_packed};
  for (auto item : list)
  {
    union { int64_t signed_in; uint64_t unsigned_out; } u;
    u.signed_in = item;
    out << Varint{u.unsigned_out};
  }
  return out;
}

struct Example
{
  std::vector<MapFieldEntry<std::string>> bytes_lists;
  std::vector<MapFieldEntry<float>> float_lists;
  std::vector<MapFieldEntry<int64_t>> int64_lists;
  uint64_t sizeof_lists;
  void calculate_size() {
    sizeof_lists = 0;
    for (auto& bytes_list : bytes_lists)
    {
      sizeof_lists += bytes_list.size;
    }
    for (auto& float_list : float_lists)
    {
      sizeof_lists += float_list.size;
    }
    for (auto& int64_list : int64_lists)
    {
      sizeof_lists += int64_list.size;
    }
  }
  friend std::ostream& operator<<(std::ostream& out, const Example& example) {
    out << static_cast<char>(1 << 3 | 2) << Varint{example.sizeof_lists};
    for (auto& bytes_list : example.bytes_lists)
    {
      out << bytes_list;
    }
    for (auto& float_list : example.float_lists)
    {
      out << float_list;
    }
    for (auto& int64_list : example.int64_lists)
    {
      out << int64_list;
    }
    return out;
  }
  static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
  {
    reinterpret_cast<std::istream*>(png_get_io_ptr(png_ptr))->read(reinterpret_cast<char*>(data), length);
  }
  bool add_png(std::istream& in, const std::string& image_key,
               const std::string& image_shape_key)
               {
                 png_structp png_ptr =
                     png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                 if (!png_ptr)
                 {
                   std::cerr << "Failed to allocate PNG pointer" << std::endl;
                   return false;
                 }

                 png_infop info_ptr = png_create_info_struct(png_ptr);
                 if (!info_ptr)
                 {
                   std::cerr << "Failed to allocate info pointer" << std::endl;
                   png_destroy_read_struct(&png_ptr, NULL, NULL);
                   return false;
                 }

                 if (setjmp(png_jmpbuf(png_ptr)))
                 {
                   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                   return false;
                 }

                 png_set_read_fn(png_ptr, reinterpret_cast<png_voidp>(&in), &Example::png_read_data);
                 png_read_info(png_ptr, info_ptr);

                 png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
                 png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
                 png_byte color_type = png_get_color_type(png_ptr, info_ptr);
                 png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

                 // strip 16-bit data (convert to 8-bit)
                 if (bit_depth == 16)
                 {
                   png_set_strip_16(png_ptr);
                 }

                 // convert palette data to RGB
                 if (color_type == PNG_COLOR_TYPE_PALETTE)
                 {
                   png_set_palette_to_rgb(png_ptr);
                 }

                 // expand lower bit depths
                 if (bit_depth < 8)
                 {
                   png_set_expand_gray_1_2_4_to_8(png_ptr);
                 }

                 // read tRNS info
                 if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
                 {
                   png_set_tRNS_to_alpha(png_ptr);
                 }

                 png_read_update_info(png_ptr, info_ptr);
                 color_type = png_get_color_type(png_ptr, info_ptr);
                 bit_depth = png_get_bit_depth(png_ptr, info_ptr);

                 if (color_type != PNG_COLOR_TYPE_GRAY && color_type != PNG_COLOR_TYPE_RGB)
                 {
                   std::cerr << "Unknown color_type: " << color_type << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_GRAY: " << PNG_COLOR_TYPE_GRAY << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_PALETTE: " << PNG_COLOR_TYPE_PALETTE << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_RGB: " << PNG_COLOR_TYPE_RGB << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_RGB_ALPHA: " << PNG_COLOR_TYPE_RGB_ALPHA << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_GRAY_ALPHA: " << PNG_COLOR_TYPE_GRAY_ALPHA << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_RGBA: " << PNG_COLOR_TYPE_RGBA << std::endl;
                   std::cerr << "  PNG_COLOR_TYPE_GA: " << PNG_COLOR_TYPE_GA << std::endl;
                   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                   return false;
                 }

                 std::vector<png_byte> pixels(height * png_get_rowbytes(png_ptr, info_ptr));
                 std::vector<png_bytep> row_pointers;
                 for (png_uint_32 y = 0; y < height; ++y)
                 {
                   row_pointers.emplace_back(pixels.data() + png_get_rowbytes(png_ptr, info_ptr));
                 }

                 png_read_image(png_ptr, row_pointers.data());

                 png_byte colors_per_pixel = color_type == PNG_COLOR_TYPE_GRAY ? 1 : 3;
                 uint64_t color_values_size = width * height * colors_per_pixel;

                 std::cout << "width: " << width << std::endl;
                 std::cout << "height: " << height << std::endl;
                 std::cout << "color_type: " << color_type << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_GRAY: " << PNG_COLOR_TYPE_GRAY << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_PALETTE: " << PNG_COLOR_TYPE_PALETTE << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_RGB: " << PNG_COLOR_TYPE_RGB << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_RGB_ALPHA: " << PNG_COLOR_TYPE_RGB_ALPHA << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_GRAY_ALPHA: " << PNG_COLOR_TYPE_GRAY_ALPHA << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_RGBA: " << PNG_COLOR_TYPE_RGBA << std::endl;
                 std::cout << "  PNG_COLOR_TYPE_GA: " << PNG_COLOR_TYPE_GA << std::endl;
                 std::cout << "bit_depth: " << bit_depth << std::endl;
                 
                 int count = 0;
                 for (auto pixel : pixels)
                 {
                   std::cout << static_cast<int>(pixel) << std::endl;
                   if (count++ > 100)
                     break;
                 }
                 
                 int64_lists.emplace_back(image_key, std::vector<int64_t>{pixels.begin(), pixels.end()});
                 int64_lists.emplace_back(image_shape_key, std::vector<int64_t>{height, width, colors_per_pixel});

                 png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
                 
                 return true;
               }
};

int main(void)
{
  Example example;
  std::ifstream in;
  in.open("../topImage_748772.png");
  if (in.is_open())
  {
    std::cout << "success: " << example.add_png(in, "image", "image_shape") << std::endl;
  }
  in.close();
  // example.add_png()
  // example.bytes_lists.emplace_back("image_path", std::vector<std::string>{"/root/image.png"});
  // example.int64_lists.emplace_back("image_shape", std::vector<int64_t>{480, 640, 3});
  // example.int64_lists.emplace_back("image", std::vector<int64_t>{480, 640, 3, 1337, 42});
  // example.float_lists.emplace_back("boxes", std::vector<float>{480.245, 640.4535, 3.345435, 1337.234534, 42.34534});
  example.calculate_size();
  std::ofstream f;
  f.open("example.pb", std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
  if (f.is_open())
  {
    f << example;
  }
  f.close();
  return 0;
}
