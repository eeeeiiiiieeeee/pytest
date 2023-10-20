import cv2
import numpy as np


StateSwitch  = 0

def compute_average_brightness(image, block_size, Step):
    #根据下一帧的速度算亮度计算的起始点
    global StateSwitch
    if StateSwitch==0:
        x=round(Step/1.8)
        y=0
        StateSwitch=1
    else :
        x=0
        y=0
        StateSwitch=0

    # 调整图像大小为320x320像素
    image = cv2.resize(image, (320, 320))
    # 使用切片操作来裁剪图像
    cropped_image = image[y:y+320, x:x+310]
    
    height, width, _ = cropped_image.shape
    array_x_size=width//block_size
    array_y_size=height//block_size
    # 将图像转换为NumPy数组
    image_array = np.array(cropped_image)

    # 初始化数组
    brightness_array = np.zeros((array_y_size, array_x_size), dtype=int)

    # 划分图像成块，计算单位亮度，填充数组
    for i in range(array_y_size):
        for j in range(array_x_size):
            block = image_array[i * block_size:(i + 1) * block_size, j * block_size:(j + 1) * block_size]
            average_brightness = int(np.mean(block))
            brightness_array[i, j] = average_brightness

    return brightness_array

# 将左边的值减去右边的值，算边界
def compare_and_create_array(array, threshold):
    # 计算差值
    diff_array = array[:, :-1] - array[:, 1:]

    # 根据差值大小创建新数组
    new_array = np.where(diff_array > threshold, 255, 0)

    # 将倒数第二列的数据赋值给倒数第一列
    new_array[:, -1] = new_array[:, -2]

    return new_array


# 数组相消，相同部分消除
def process_arrays(thresholded_array, average_array):
    new_array = np.copy(average_array)
    for x in range(new_array.shape[0]):
        for y in range(new_array.shape[1]):
            if thresholded_array[x, y] == 255 and average_array[x, y] == 255:
                new_array[x, y] = 0
                
    return new_array

def binary_difference_array(arr1, arr2, x):
    # 计算两个数组的差值
    diff = np.abs(arr1 - arr2)
    
    # 使用条件运算符创建二值化数组
    binary_array = np.where(diff > x, 255, 0)
    
    return binary_array

# 打开视频文件
# cap = cv2.VideoCapture('C:\\Users\\test\\Desktop\\2.mp4')

# 打开摄像头
cap = cv2.VideoCapture(0)
PIXEL_BLOCK=10
PIXEL_STEP=0

while True:
    ret, frame = cap.read()
    
    # 如果没有更多帧，则退出循环
    if not ret:
        break

    if StateSwitch==0:
        previous_frame = compute_average_brightness(frame,PIXEL_BLOCK,PIXEL_STEP)
        test_previous_frame=compare_and_create_array(previous_frame,0)
        # test_previous_frame=process_brightness_array(previous_frame,10)
        # previous_frame=process_array(previous_frame,10)
    else:
        result= compute_average_brightness(frame,PIXEL_BLOCK,PIXEL_STEP)
        # print_array(result)
        mean_value=compare_and_create_array(result,0)
        # result=process_array(result,10)
        modified_array = binary_difference_array(result,previous_frame, 20)
        # brightness_array =convert_ones_to_255(modified_array)
        # brightness_array =compare_arrays(test_previous_frame,mean_value)
        brightness_array =process_arrays(test_previous_frame,mean_value)
        # 使用shape属性获取数组的长和宽
        num_rows, num_cols = modified_array.shape
        processed_frame = modified_array.reshape(num_rows, num_cols).astype(np.uint8)
        processed_frame = cv2.resize(processed_frame, (num_rows*PIXEL_BLOCK, num_cols*PIXEL_BLOCK), interpolation=cv2.INTER_NEAREST)

        num_rows_b, num_cols_b = brightness_array.shape
        brightness_array = brightness_array.reshape(num_rows_b, num_cols_b).astype(np.uint8)
        brightness_array = cv2.resize(brightness_array, (num_rows_b*PIXEL_BLOCK, num_cols_b*PIXEL_BLOCK), interpolation=cv2.INTER_NEAREST)
        # 显示原图像和差异处理后的图像
        frame = cv2.resize(frame, (320, 320))
        cv2.imshow('Original Frame', frame)
        cv2.imshow('Processed Frame', processed_frame)
        cv2.imshow('brightness_array', brightness_array)
        
        # 按下 ESC 键退出
    if cv2.waitKey(1) & 0xFF == 27:
        break

    # 关闭视频文件
cap.release()
cv2.destroyAllWindows()