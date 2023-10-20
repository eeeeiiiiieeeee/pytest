import numpy as np

def binary_difference_array(arr1, arr2, x):
    # 计算两个数组的差值
    diff = np.abs(arr1 - arr2)
    
    # 使用条件运算符创建二值化数组
    binary_array = np.where(diff > x, 1, 0)
    
    return binary_array

# 示例用法
array1 = np.array([1, 2, 3, 4, 9])
array2 = np.array([6, 7, 8, 9, 10])
threshold = 3  # 假设差值阈值为3

result = binary_difference_array(array1, array2, threshold)
print(result)