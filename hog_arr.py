# import numpy as np
# import matplotlib.pyplot as plt
# from skimage import io, color, exposure
# from skimage.feature import hog

# # 读取图像
# image_path = "C:\\Users\\test\\Desktop\\6.jpg"
# image = io.imread(image_path)
# image = color.rgb2gray(image)  # 转换为灰度图像

# # 计算HOG特征
# hog_features, hog_image = hog(image, orientations=8, pixels_per_cell=(16, 16),
#                               cells_per_block=(1, 1), visualize=True, block_norm='L2-Hys')

# # 将HOG特征图进行强度归一化以便显示
# hog_image_rescaled = exposure.rescale_intensity(hog_image, in_range=(0, 10))

# # 打印特征向量的长度
# print("HOG特征向量的长度:", len(hog_features))

# # 显示原图、HOG特征图和HOG特征向量
# plt.figure(figsize=(10, 8))
# plt.subplot(131)
# plt.imshow(image, cmap=plt.cm.gray)
# plt.title("原图")

# plt.subplot(132)
# plt.imshow(hog_image_rescaled, cmap=plt.cm.gray)
# plt.title("HOG特征图")

# plt.subplot(133)
# plt.plot(hog_features)
# plt.title("HOG特征向量")
# plt.show()

# import numpy as np
# from skimage.feature import hog
# from skimage import io, color
# import matplotlib.pyplot as plt

# # 读取图像并转换为灰度图像
# image_path = "C:\\Users\\test\\Desktop\\4.jpg"
# image = io.imread(image_path)

# # 定义感兴趣区域的坐标
# roi_x = 0
# roi_y = 0
# roi_width = 160
# roi_height = 160

# # 从原始图像中提取感兴趣的区域
# roi_image = image[roi_y:roi_y+roi_height, roi_x:roi_x+roi_width]

# gray_image = color.rgb2gray(roi_image)

# # 将数组打印选项设置为完整输出，不截断
# np.set_printoptions(threshold=np.inf)

# # 计算HOG特征
# hog_features = hog(gray_image, orientations=8, pixels_per_cell=(8, 8),
#                    cells_per_block=(1, 1), block_norm='L2-Hys')

# # 将HOG特征存储在一维数组中
# hog_array = np.array(hog_features)
# #hog_array_int = hog_array.astype(int)

# # 打印HOG特征数组
# print("HOG特征数组长度:", len(hog_array))
# print("HOG特征数组内容:", hog_array)

# # 显示原图、HOG特征图和HOG特征向量
# plt.figure(figsize=(10, 8))
# plt.subplot(131)
# plt.imshow(gray_image, cmap=plt.cm.gray)
# plt.title("原图")

# plt.subplot(132)
# plt.imshow(hog_image_rescaled, cmap=plt.cm.gray)
# plt.title("HOG特征图")

# plt.subplot(133)
# plt.plot(hog_features)
# plt.title("HOG特征向量")
# plt.show()
import cv2
import numpy as np
from skimage.feature import hog
import matplotlib.pyplot as plt

# 读取原始图片
original_image = cv2.imread("C:\\Users\\test\\Desktop\\7.jpg", cv2.IMREAD_GRAYSCALE)

# 缩放图片
scaled_image = cv2.resize(original_image, (160, 160))

# 计算HOG特征
hog_features, hog_image = hog(scaled_image, pixels_per_cell=(8, 8), visualize=True)

# 将数组打印选项设置为完整输出，不截断
np.set_printoptions(threshold=np.inf)

# 打印HOG特征数组的内容
print("HOG Feature Array:")
print(hog_features)

# 显示缩放后的图片
plt.subplot(131)
plt.imshow(scaled_image, cmap='gray')
plt.title('Scaled Image')

# 显示HOG特征图
plt.subplot(132)
plt.imshow(hog_image, cmap='gray')
plt.title('HOG Feature')

# 显示HOG特征数组内容
plt.subplot(133)
plt.plot(hog_features)
plt.title('HOG Feature Array')

plt.tight_layout()
plt.show()
