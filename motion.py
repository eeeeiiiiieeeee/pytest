import cv2
import numpy as np
from PIL import Image

def compute_average_brightness(image, block_size=10, array_size=32):
    # 调整图像大小为320x320像素
    image = cv2.resize(image, (320, 320))
    
    # 将图像转换为NumPy数组
    image_array = np.array(image)

    # 初始化数组
    brightness_array = np.zeros((array_size, array_size), dtype=int)

    # 划分图像成块，计算单位亮度，填充数组
    for i in range(array_size):
        for j in range(array_size):
            block = image_array[i * block_size:(i + 1) * block_size, j * block_size:(j + 1) * block_size]
            average_brightness = int(np.mean(block))
            brightness_array[i, j] = average_brightness

    return brightness_array


def binary_difference_array(arr1, arr2, x):
    # 计算两个数组的差值
    diff = np.abs(arr1 - arr2)
    
    # 使用条件运算符创建二值化数组
    binary_array = np.where(diff > x, 255, 0)
    
    return binary_array

def binary_difference_and_match(arr1, arr2, threshold1, threshold2):
    # 计算两个数组的差值
    diff = np.abs(arr1 - arr2)

    # 使用条件运算符创建二值化数组
    binary_array = np.where(diff > threshold1, 255, 0)

    # 初始化新数组来记录差值大于阈值的位置
    diff_positions = np.zeros_like(arr1, dtype=bool)

    # 记录差值大于阈值的位置
    diff_positions[diff > threshold1] = True

    # 创建新数组来匹配九宫格
    matched_array = np.zeros_like(arr1)

    # 遍历数组1，匹配九宫格
    for i in range(1, arr1.shape[0] - 1):
        for j in range(1, arr1.shape[1] - 1):
            if diff_positions[i, j]:
                reference_value = arr1[i, j]

                # 定义九宫格
                neighborhood = arr2[i-1:i+2, j-1:j+2]

                # 计算九宫格元素与参考值的差值
                neighborhood_diff = np.abs(neighborhood - reference_value)

                # 检查九宫格中是否有元素差值小于阈值的情况
                if np.any(neighborhood_diff <= threshold2):
                    matched_array[i, j] = 255

    return binary_array, matched_array

# 打开摄像头
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    
    # 如果没有更多帧，则退出循环
    if not ret:
        break

    result = compute_average_brightness(frame)

    # 如果不是第一帧，则与上一帧比较
    if 'previous_frame' in locals():

        # modified_array = binary_difference_array(result,previous_frame, 10)

        modified_array,test_arr=binary_difference_and_match(previous_frame, result, 10,3)

        #print_array(modified_array)
        # 将差异处理后的数组转化为32x32的图像
        processed_frame = modified_array.reshape(32, 32).astype(np.uint8)
        test_frame=test_arr.reshape(32, 32).astype(np.uint8)
        
        # 将差异处理后的图像放大到与原图像相同的尺寸
        processed_frame = cv2.resize(processed_frame, (320, 320), interpolation=cv2.INTER_NEAREST)
        test_frame=cv2.resize(test_frame, (320, 320), interpolation=cv2.INTER_NEAREST)
        # 显示原图像和差异处理后的图像
        frame = cv2.resize(frame, (320, 320))
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Processed Frame', processed_frame)
        cv2.imshow('test_frame', test_frame)
        # 按下 ESC 键退出
    if cv2.waitKey(1) & 0xFF == 27:
        break

    previous_frame = result

    # 关闭视频文件
cap.release()
cv2.destroyAllWindows()
