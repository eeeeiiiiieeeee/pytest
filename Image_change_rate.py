from PIL import Image
import numpy as np

def cross_dilation(input_array, iterations):
    # 定义3x3的膨胀核
    dilation_kernel = np.array([[0, 1, 0],
                                [1, 1, 1],
                                [0, 1, 0]], dtype=np.uint8)
    
    # 复制输入数组，以免在操作时修改原数组
    dilated_array = np.copy(input_array)
    
    # 进行指定次数的膨胀操作
    for _ in range(iterations):
        temp_array = np.copy(dilated_array)
        for i in range(1, dilated_array.shape[0] - 1):
            for j in range(1, dilated_array.shape[1] - 1):
                if dilated_array[i, j] == 1:
                    temp_array[i - 1:i + 2, j - 1:j + 2] = 1
        dilated_array = temp_array
    
    return dilated_array



def cross_erosion(input_array, iterations):
    # 定义3x3的腐蚀核
    erosion_kernel = np.array([[0, 1, 0],
                               [1, 1, 1],
                               [0, 1, 0]], dtype=np.uint8)
    
    # 复制输入数组，以免在操作时修改原数组
    eroded_array = np.copy(input_array)
    
    # 进行指定次数的腐蚀操作
    for _ in range(iterations):
        temp_array = np.copy(eroded_array)
        for i in range(1, eroded_array.shape[0] - 1):
            for j in range(1, eroded_array.shape[1] - 1):
                if eroded_array[i, j] == 0:
                    temp_array[i - 1:i + 2, j - 1:j + 2] = 0
        eroded_array = temp_array
    
    return eroded_array


# 定义九宫格均值处理的函数
def apply_neighborhood_average(arr, x, y):
    neighborhood = [
        (x, y), (x, y - 1), (x - 1, y), (x + 1, y), (x, y + 1),
        (x - 1, y - 1), (x + 1, y + 1), (x + 1, y - 1), (x - 1, y + 1)
    ]
    total = 0
    count = 0
    for nx, ny in neighborhood:
        if 0 <= nx < arr.shape[0] and 0 <= ny < arr.shape[1]:
            total += arr[nx, ny]
            count += 1
    return total / count if count > 0 else 0


#均值
def process_brightness_array(brightness_array):
    array_size = brightness_array.shape[0]
    change_array = np.zeros_like(brightness_array)
    count_of_ones = 0  # 初始化统计变量

    for x in range(array_size):
        for y in range(array_size):
            neighborhood_avg = apply_neighborhood_average(brightness_array, x, y)
            change = brightness_array[x, y] - neighborhood_avg
            if abs(change) > 10:
                change_array[x, y] = 1
                count_of_ones += 1  # 统计变量增加

    return change_array, count_of_ones


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

def remove_continuous_rows_cols(array):
    new_array = np.copy(array)
    for x in range(array.shape[0]):
        for y in range(array.shape[1]):
            if array[x, y] == 1:
                # Check right direction
                continuous_right = 0
                for i in range(1, array.shape[1] - y):
                    if array[x, y + i] == 1:
                        continuous_right += 1
                    else:
                        break
                # Check down direction
                continuous_down = 0
                for i in range(1, array.shape[0] - x):
                    if array[x + i, y] == 1:
                        continuous_down += 1
                    else:
                        break
                if continuous_right > 20:
                    new_array[x, y:y + continuous_right + 1] = 0
                if continuous_down > 20:
                    new_array[x:x + continuous_down + 1, y] = 0
    return new_array

#亮度阀值处理
def apply_brightness_threshold(brightness_array):
    array_size = brightness_array.shape[0]
    thresholded_array = np.zeros_like(brightness_array)

    for x in range(array_size):
        for y in range(array_size):
            if brightness_array[x, y] > 230:
                neighborhood = [
                    (x, y), (x, y - 1), (x - 1, y), (x + 1, y), (x, y + 1),
                    (x - 1, y - 1), (x + 1, y + 1), (x + 1, y - 1), (x - 1, y + 1)
                ]
                for nx, ny in neighborhood:
                    if 0 <= nx < array_size and 0 <= ny < array_size:
                        thresholded_array[nx, ny] = 1
                        
    return thresholded_array

#全局边界减去超亮边界
def process_arrays(thresholded_array, average_array):
    new_array = np.copy(average_array)
    
    for x in range(new_array.shape[0]):
        for y in range(new_array.shape[1]):
            if thresholded_array[x, y] == 1 and average_array[x, y] == 1:
                new_array[x, y] = 0
                
    return new_array

#打印数组函数
def print_array(array):
    for i in range(array.shape[0]):
        for j in range(array.shape[1]):
            print(array[i, j], end=' ')
        print()
    print("\n")

#数组以字符串新式打印
def print_2d_array(arr):
    for row in arr:
        row_str = ','.join(map(str, row))
        print(row_str)

# 加载图片并转换为灰度图像
image_path = "C:\\Users\\test\\Desktop\\6.jpg"

# image_path = "C:\\Users\\test\\Desktop\\9.png"

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
print_2d_array(brightness_array)

# # 初始化新数组
# change_array = np.zeros_like(brightness_array)

# 调用函数并传入亮度数组作为参数
# processed_array, ones_count = process_brightness_array(brightness_array)
# print("Count of ones:", ones_count)
# print_array(processed_array)
#左边减右边
diff_threshold = 10
diff_array = calculate_diff_array(brightness_array, diff_threshold)
print_array(diff_array)



#亮度阀值数组
thresholded_array=apply_brightness_threshold(brightness_array)
print_array(thresholded_array)


# 假设你已经有了亮度阈值处理得到的数组 thresholded_array 和亮度均值数组 average_array
# 使用上面的函数处理数组并获取结果
result_array = process_arrays(thresholded_array,diff_array)
print_array(result_array)
# # 遍历亮度数组并执行九宫格均值处理，进行变化判断
# for x in range(brightness_array.shape[0]):
#     for y in range(brightness_array.shape[1]):
#         neighborhood_avg = apply_neighborhood_average(brightness_array, x, y)
#         change = brightness_array[x, y] - neighborhood_avg
#         if abs(change) > 10:
#             change_array[x, y] = 1
# # 打印生成的结果数组
# print_array(change_array)

# # 初始化新的32x32的数组来存储变化率的结果
# change_array = np.zeros((32, 32), dtype=int)

# # 计算变化率并赋值到新数组
# for i in range(1, 32):
#     for j in range(1, 32):
#         # 计算相邻元素的变化率
#         change_rate = abs(brightness_array[i, j] - brightness_array[i-1, j-1]) / brightness_array[i-1, j-1]
        
#         # 如果变化率大于0.1，设置新数组对应位置为1，否则为0
#         if change_rate > 0.2:
#             change_array[i, j] = 1
#         else:
#             change_array[i, j] = 0

# 移除连续的行和列
final_array = remove_continuous_rows_cols(result_array)
print_array(final_array)

# 对array_to_dilate数组进行十字膨胀处理，迭代3次
dilated_array = cross_dilation(final_array, iterations=1)
print_array(dilated_array)
# 创建一个新的32x32的数组
# dilated_array = np.zeros((32, 32), dtype=int)


# # 对change_array数组进行十字膨胀处理
# for i in range(32):
#     for j in range(32):
#         if change_array[i, j] == 1:
#             dilated_array[i, j] = 1  # 当前位置变为1
#             if i - 1 >= 0:
#                 dilated_array[i - 1, j] = 1  # 上方位置变为1
#             if i + 1 < 32:
#                 dilated_array[i + 1, j] = 1  # 下方位置变为1
#             if j - 1 >= 0:
#                 dilated_array[i, j - 1] = 1  # 左方位置变为1
#             if j + 1 < 32:
#                 dilated_array[i, j + 1] = 1  # 右方位置变为1

dilated_array = remove_continuous_rows_cols(dilated_array)

eroded_array = cross_erosion(dilated_array, iterations=1)
print_array(dilated_array)
# dilated_array = remove_continuous_rows_cols(eroded_array)

# print_array(eroded_array)


# # 创建一个新的32x32的数组
# eroded_array = np.ones((32, 32), dtype=int)

# # 对dilated_array数组进行十字腐蚀处理
# for i in range(32):
#     for j in range(32):
#         if dilated_array[i, j] == 0:
#             eroded_array[i, j] = 0  # 当前位置变为0
#             if i - 1 >= 0:
#                 eroded_array[i - 1, j] = 0  # 上方位置变为0
#             if i + 1 < 32:
#                 eroded_array[i + 1, j] = 0  # 下方位置变为0
#             if j - 1 >= 0:
#                 eroded_array[i, j - 1] = 0  # 左方位置变为0
#             if j + 1 < 32:
#                 eroded_array[i, j + 1] = 0  # 右方位置变为0



# img = cv2.imread("002.png")
# img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
# binaryMap = []
# for i in range(img.shape[0]):
#     temp = 0x00
#     for j in range(img.shape[1]):
#         if (img[i][j] == 0):
#             temp |= 0x01
#         temp <<= 1
#     print("{:032b}".format(temp))
#     binaryMap.append(temp)

# fushiResult = []
# print("\n{:032b}".format(0x00))
# for i in range(1, len(binaryMap) - 1):
#     temp = binaryMap[i]
#     temp = 0xFFFFFFFF & binaryMap[i - 1] & binaryMap[i] & (binaryMap[i] << 1) & (binaryMap[i] >> 1) & (binaryMap[i + 1])
#     #temp = 0x00000000 | binaryMap[i - 1] | binaryMap[i] | (binaryMap[i] << 1) | (binaryMap[i] >> 1) | (binaryMap[i + 1])
#     fushiResult.append(temp)
#     print("{:032b}".format(temp))
# print("{:032b}\n".format(0x00))

# for i in range(len(binaryMap)):
#     print("{:2}:{:032b}".format(i,binaryMap[i] & 0xFFFFFFFF), end = " ")
#     print("{:032b}".format(fushiResult[i] & 0xFFFFFFFF))