from PIL import Image

# 打开要压缩的图像
input_image_path = "C:/Users/test/Desktop/10.jpg"
output_image_path = "C:/Users/test/Desktop/JPEGTEST.jpg"

image = Image.open(input_image_path)

# 设置压缩质量（1-95之间，1最差，95最佳）
quality = 10  # 根据需要进行调整

# 保存压缩后的图像
image.save(output_image_path, "JPEG", quality=quality)



# 打开原始图像和解压后的图像
original_image = Image.open(input_image_path)
compressed_image = Image.open(output_image_path)

# 显示原始图像
original_image.show(title="原始图像")

# 显示解压后的图像
compressed_image.show(title="解压后的图像")
# 关闭图像
image.close()