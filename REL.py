# import numpy as np
# from PIL import Image
# import matplotlib.pyplot as plt

# def threshold_binarization(image, threshold=90):
#     binary_image = np.where(image > threshold, 1, 0).astype(np.uint8)
#     return binary_image

# def merge_binary_data(binary_image, block_size=32):
#     height, width = binary_image.shape
#     merged_width = width // block_size
#     merged_data = np.zeros((height, merged_width), dtype=np.uint32)

#     for i in range(height):
#         for j in range(merged_width):
#             block = binary_image[i, j * block_size:(j + 1) * block_size]
#             merged_value = int(''.join(map(str, block)), 2)
#             merged_data[i, j] = merged_value

#     return merged_data

# def unmerge_binary_data(merged_data, block_size=32, original_width=320):
#     height, merged_width = merged_data.shape
#     binary_image = np.zeros((height, original_width), dtype=np.uint8)

#     for i in range(height):
#         for j in range(merged_width):
#             merged_value = merged_data[i, j]
#             binary_block = format(merged_value, f'0{block_size}b')
#             binary_image[i, j * block_size:(j + 1) * block_size] = list(map(int, binary_block))

#     return binary_image

# # 加载黑白图像
# original_image = Image.open("C:/Users/test/Desktop/RELTEST.jpg").convert("L")
# image_array = np.array(original_image)

# # 进行Otsu二值化
# # binary_image = otsu_binarization(image_array)
# # block_size = 64
# # compressed_data = merge_binary_data(binary_image, block_size)
# # decoded_image = unmerge_binary_data(compressed_data, block_size, original_width=image_array.shape[1])
# binary_image = threshold_binarization(image_array)

# compressed_data = merge_binary_data(binary_image)
# decoded_image = unmerge_binary_data(compressed_data)

# # 显示原始图像、Otsu's二值化后的图像和解压缩后的图像
# plt.figure(figsize=(16, 4))

# plt.subplot(1, 3, 1)
# plt.imshow(image_array, cmap='gray')
# plt.title('Original Image')

# plt.subplot(1, 3, 2)
# plt.imshow(binary_image, cmap='gray')
# plt.title("Otsu's Binarization")

# plt.subplot(1, 3, 3)
# plt.imshow(decoded_image, cmap='gray')
# plt.title('Decompressed Image')

# plt.show()



import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

def sauvola_threshold(image, block_size, k, R):
    height, width = image.shape
    binary_image = np.zeros((height, width), dtype=np.uint8)

    for x in range(0, height, block_size):
        for y in range(0, width, block_size):
            block = image[x:x+block_size, y:y+block_size]
            block_mean = np.mean(block)
            block_std = np.std(block)
            threshold = block_mean * (1 + k * ((block_std / R) - 1))
            threshold_block = (block > threshold)
            binary_image[x:x+block_size, y:y+block_size] = threshold_block

    return binary_image

def merge_binary_data(binary_image, block_size=32):
    height, width = binary_image.shape
    merged_width = width // block_size
    merged_data = np.zeros((height, merged_width), dtype=np.uint32)

    for i in range(height):
        for j in range(merged_width):
            block = binary_image[i, j * block_size:(j + 1) * block_size]
            merged_value = int(''.join(map(str, block)), 2)
            merged_data[i, j] = merged_value

    return merged_data

def unmerge_binary_data(merged_data, block_size=32, original_width=320):
    height, merged_width = merged_data.shape
    binary_image = np.zeros((height, original_width), dtype=np.uint8)

    for i in range(height):
        for j in range(merged_width):
            merged_value = merged_data[i, j]
            binary_block = format(merged_value, f'0{block_size}b')
            binary_image[i, j * block_size:(j + 1) * block_size] = list(map(int, binary_block))

    return binary_image

def compute_average_brightness(image, block_size=2, array_size=160):
    
    # # 将图像转换为NumPy数组
    # image_array = np.array(image)

    # 初始化数组
    brightness_array = np.zeros((array_size, array_size), dtype=int)

    # 划分图像成块，计算单位亮度，填充数组
    for i in range(array_size):
        for j in range(array_size):
            block = image[i * block_size:(i + 1) * block_size, j * block_size:(j + 1) * block_size]
            average_brightness = int(np.mean(block))
            brightness_array[i, j] = average_brightness

    return brightness_array

# def threshold_binarization(image, threshold=90):
#     binary_image = np.where(image > threshold, 1, 0).astype(np.uint8)
#     return binary_image


# 加载黑白图像
original_image = Image.open("C:/Users/test/Desktop/10.jpg").convert("L")
image_array = np.array(original_image)



# Sauvola自适应二值化
block_size = 64  # 调整块大小
k = 0.1  # 调整增益因子
R = 128  # 调整参数R
brightness=compute_average_brightness(image_array)
binary_image = sauvola_threshold(brightness, 16, k, R)
# binary_image =threshold_binarization(brightness)
compressed_data = merge_binary_data(binary_image)
decoded_image = unmerge_binary_data(compressed_data)

# 显示原始图像、Sauvola二值化后的图像和解压缩后的图像
plt.figure(figsize=(16, 4))

plt.subplot(1, 3, 1)
plt.imshow(image_array, cmap='gray')
plt.title('Original Image')

plt.subplot(1, 3, 2)
plt.imshow(binary_image, cmap='gray')
plt.title("Sauvola Binarization")

plt.subplot(1, 3, 3)
plt.imshow(decoded_image, cmap='gray')
plt.title('Decompressed Image')

plt.show()
