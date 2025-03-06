from PIL import Image


def image_to_cpp_array(image_path, output_file, array_name="bitmap"):
    # 打开图片并转换为1位像素（黑白）
    img = Image.open(image_path).convert("1")  # "1"表示1位像素（黑白）

    # 确保图片的尺寸与 OLED 屏幕的尺寸匹配
    img = img.resize((64, 64))  # 这里假设 OLED 分辨率为 64*64

    # 获取图像的像素数据
    pixel_data = list(img.getdata())

    # 打开文件准备写入 C++ 数组
    with open(output_file, "w") as f:
        f.write(f"const uint8_t {array_name}[{img.height}][{img.width}] = {{\n")

        for y in range(img.height):
            f.write("  {")
            for x in range(img.width):
                # 获取像素的值，0表示黑色，1表示白色
                pixel_value = 1 if img.getpixel((x, y)) == 255 else 0
                f.write(f"{pixel_value}, ")
            f.write("},\n")
        f.write("};\n")

    print(f"C++ array saved to {output_file}")


# 使用示例
if __name__ == "__main__":
    # 使用示例
    image_to_cpp_array("bing_generated_qrcode.png", "bitmap_data.h")