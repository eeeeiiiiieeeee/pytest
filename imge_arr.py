
import cv2
import numpy as np

# 加载第一张图像
image1 = cv2.imread('C:\\Users\\test\Desktop\\frame_0057.jpg')
# 调整图像大小为320x320像素
image1 = cv2.resize(image1, (320, 320))

# 加载第二张图像
image2 = cv2.imread('C:\\Users\\test\Desktop\\frame_0056.jpg')
# 调整图像大小为320x320像素
image2 = cv2.resize(image2, (320, 320))

# 打印两张图像的（0,160）到（320,160）这一行的亮度值
print("第一张图像（0,160）到（320,160）这一行的亮度值:")
for j in range(320):
    pixel1 = image1[200, j]
    brightness1 = int(np.mean(pixel1))
    print(brightness1, end=' ')
print()

print("第二张图像（0,160）到（320,160）这一行的亮度值:")
for j in range(320):
    pixel2 = image2[200, j]
    brightness2 = int(np.mean(pixel2))
    print(brightness2, end=' ')
print()