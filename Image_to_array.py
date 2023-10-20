from PIL import Image
import numpy as np

# 加载图片并转换为灰度图像
image_path = "C:\\Users\\test\\Desktop\\7.png"
image = Image.open(image_path).convert("L")

# 定义块大小和数组大小
block_size = 10
array_size = 32

# 将图像转换为NumPy数组
image_array = np.array(image)

# 初始化32x32的数组
brightness_array = np.zeros((array_size, array_size), dtype=int)

# 划分图像成块，计算单位亮度，填充数组
for i in range(array_size):
    for j in range(array_size):
        block = image_array[i * block_size:(i + 1) * block_size, j * block_size:(j + 1) * block_size]
        average_brightness = int(np.mean(block))
        brightness_array[i, j] = average_brightness


# 定义块大小
block_size_2 = 2  # 4x4的块

# 计算方差的函数
def calculate_variance(block):
    mean = np.mean(block)
    variance = np.mean((block - mean) ** 2)
    return variance

# 初始化方差数组
variance_array = np.zeros((32, 32))

# 计算每个像素的相邻2x2块的方差
for i in range(32):
    for j in range(32):
        if i + block_size_2 <= 32 and j + block_size_2 <= 32:
            block = brightness_array[i:i + block_size_2, j:j + block_size_2]
            variance_array[i, j] = calculate_variance(block)

# 初始化新的32x32数组
result_array = np.zeros((32, 32),dtype=int)

# 根据方差数组赋值1或0
for i in range(32):
    for j in range(32):
        print(variance_array[i, j], end=' ')
        if variance_array[i, j] > 1:
            result_array[i, j] = 1
    print()

# 打印生成的结果数组
for i in range(32):
    for j in range(32):
        print(result_array[i, j], end=' ')
    print()



