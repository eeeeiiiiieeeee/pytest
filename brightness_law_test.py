import cv2
import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

def process_image(image_path, block_size=10, array_size=32):
    # 加载图片并转换为灰度图像
    image = Image.open(image_path).convert("L")

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

    return brightness_array

def process_array(input_array):
    # 计算相邻位置的差值
    diff_array = np.diff(input_array, axis=1)  # 沿列方向计算差值

    # 初始化新数组，将差值映射为1、0或-1
    new_array = np.where(diff_array > 0, 1, np.where(diff_array < 0, -1, 0))

    # 计算每行的第一个数减去上一行的最后一个数
    row_diff = input_array[:, 0] - np.roll(input_array[:, -1], shift=1)

    # 将结果添加到新数组的第一列
    new_array[:, 0] = row_diff

    return new_array

def binary_difference_array(arr1, arr2, x):
    # 计算两个数组的差值
    diff = np.abs(arr1 - arr2)
    
    # 使用条件运算符创建二值化数组
    binary_array = np.where(diff > x, 255, 0)
    
    return binary_array


def compare_arrays(array1, array2):
    # 检查两个输入数组的形状是否相同
    if array1.shape != array2.shape:
        raise ValueError("输入数组的形状不匹配")

    # 创建新数组，初始化为255
    result_array = np.ones(array1.shape, dtype=np.uint8) * 255

    # 找出相同位置的值相等的情况，将相应位置赋值为0
    result_array[array1 == array2] = 0

    return result_array

# 调用函数并传入两张图片的路径
image_path1 = "C:\\Users\\test\\Desktop\\8.jpg"
image_path2 = "C:\\Users\\test\\Desktop\\6.jpg"

result1 = process_image(image_path1)
result2 = process_image(image_path2)

result1_test=process_array(result1)
result2_test=process_array(result2)

# 调用函数并传入输入数组
# result_array = binary_difference_array(result1, result2,10)
result_array =compare_arrays(result1_test, result2_test)
PIXEL_BLOCK=10
# 获取数组的形状
height, width = result_array.shape
processed_frame = result_array.reshape(height, width).astype(np.uint8)
processed_frame = cv2.resize(processed_frame, (height*PIXEL_BLOCK, height*PIXEL_BLOCK), interpolation=cv2.INTER_NEAREST)

# 创建x轴的值
x_values = np.arange(result1.size)

# 将result1和result2数组展平，然后绘制曲线图，分别用红色和绿色表示
plt.plot(x_values, result1.flatten(), color='red', label='Image 1', marker='o', linestyle='-')
plt.plot(x_values, result2.flatten(), color='green', label='Image 2', marker='o', linestyle='-')

plt.title('亮度曲线图')
plt.xlabel('X轴坐标')
plt.ylabel('亮度值')
plt.grid(True)
plt.legend()  # 添加图例

cv2.imshow('processed_frame', processed_frame)
# 显示图形
plt.show()

# 使用OpenCV显示图像
cv2.waitKey(0)
cv2.destroyAllWindows()