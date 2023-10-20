import cv2
import numpy as np
from PIL import Image

def compute_average_brightness(image, block_size=4, array_size=80):
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
    binary_array = np.where(diff > x, 1, 0)
    
    return binary_array

#左边减右边
def calculate_diff_array(input_array, threshold):
    diff_array = np.zeros_like(input_array, dtype=int)
    
    for i in range(input_array.shape[0]):
        for j in range(input_array.shape[1] - 1):  # 遍历到倒数第二列
            diff = input_array[i, j] - input_array[i, j+1]
            if abs(diff) > threshold:
                diff_array[i, j] = 1
        
        # 将倒数第二列的值赋给倒数第一列
        diff_array[i, -1] = diff_array[i, -2]
    
    return diff_array


#当前帧减去上一帧边界
def process_arrays(thresholded_array, average_array):
    new_array = np.copy(average_array)
    
    for x in range(new_array.shape[0]):
        for y in range(new_array.shape[1]):
            if thresholded_array[x, y] == 1:
                new_array[x, y] = 0
                
    return new_array

def shift_array_right(array):
    # 创建一个新数组来存储右移后的数据
    shifted_array = np.zeros_like(array)
    
    # 右移每个位置的数据
    for i in range(array.shape[0]):
        for j in range(array.shape[1]):
            # 计算右移后的位置，考虑边界情况
            new_i = (i + 1) % array.shape[0]
            shifted_array[new_i, j] = array[i, j]
    
    return shifted_array

#打印数组函数
def print_array(array):
    for i in range(array.shape[0]):
        for j in range(array.shape[1]):
            print(array[i, j], end=' ')
        print()
    print("\n")


def convert_ones_to_255(array):
    # 创建一个新的数组，与输入数组大小相同，但将数值为1的位置变为255
    result_array = np.where(array == 1, 255, array)
    return result_array


# 打开视频文件
# cap = cv2.VideoCapture('C:\\Users\\test\\Desktop\\2.mp4')

# 打开摄像头
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    
    # 如果没有更多帧，则退出循环
    if not ret:
        break
    # frame = cv2.resize(frame, (320, 320))
    # cv2.imshow('Original Frame', frame)
    # 调整帧大小为320x320像素
    #frame = cv2.resize(frame, (320, 320))

    result = compute_average_brightness(frame)
    # modified_array=result
    # modified_array = binary_difference_array(result,previous_frame, 10)

    #左边减右边
    # diff_threshold = 30
    # diff_array = calculate_diff_array(result, diff_threshold)

    # shifted_array = shift_array_right(diff_array)

    # print_array(diff_array)

    # 如果不是第一帧，则与上一帧比较
    if 'previous_frame' in locals():

        modified_array = binary_difference_array(result,previous_frame, 10)
        # 使用上面的函数处理数组并获取结果
        # region_array = process_arrays(previous_frame,diff_array)
        brightness_array =convert_ones_to_255(modified_array)

        #print_array(region_array)
        # 将差异处理后的数组转化为32x32的图像
        processed_frame = brightness_array.reshape(80, 80).astype(np.uint8)
        
        # 将差异处理后的图像放大到与原图像相同的尺寸
        processed_frame = cv2.resize(processed_frame, (320, 320), interpolation=cv2.INTER_NEAREST)
        # 显示原图像和差异处理后的图像
        frame = cv2.resize(frame, (320, 320))
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Processed Frame', processed_frame)
        
        # 按下 ESC 键退出
    if cv2.waitKey(1) & 0xFF == 27:
        break

    previous_frame = result

    # 关闭视频文件
cap.release()
cv2.destroyAllWindows()
