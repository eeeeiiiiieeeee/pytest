# import cv2
# import numpy as np

# # 读取视频文件（或摄像头流）
# cap = cv2.VideoCapture(0)  # 替换为您的视频文件路径或摄像头索引

# # 创建Lucas-Kanade光流估计器
# lk_params = dict(winSize=(15, 15),  # 窗口大小，用于搜索最佳匹配点
#                  maxLevel=2,       # 金字塔层级
#                  criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

# # 选择要跟踪的初始特征点
# ret, first_frame = cap.read()
# gray_first_frame = cv2.cvtColor(first_frame, cv2.COLOR_BGR2GRAY)
# p0 = cv2.goodFeaturesToTrack(gray_first_frame, maxCorners=100, qualityLevel=0.3, minDistance=7)

# # 创建颜色以绘制轨迹
# color = np.random.randint(0, 255, (100, 3))

# while True:
#     ret, frame = cap.read()
#     if not ret:
#         break

#     gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

#     # 计算光流
#     p1, st, err = cv2.calcOpticalFlowPyrLK(gray_first_frame, gray_frame, p0, None, **lk_params)

#     if p1 is not None:
#         # 选择好的光流点
#         good_new = p1[st == 1]
#         good_old = p0[st == 1]

#         # 绘制轨迹
#         for i, (new, old) in enumerate(zip(good_new, good_old)):
#             a, b = new.ravel()
#             c, d = old.ravel()
#             mask = cv2.line(np.zeros_like(frame), (int(a), int(b)), (int(c), int(d)), color[i].tolist(), 2)
#             frame = cv2.add(frame, mask)

#     cv2.imshow('Optical Flow', frame)

#     # 更新特征点
#     p0 = good_new.reshape(-1, 1, 2)

#     if cv2.waitKey(30) & 0xFF == 27:
#         break

# cap.release()
# cv2.destroyAllWindows()
import cv2
import numpy as np

# 打开摄像头
cap = cv2.VideoCapture(0)  # 0 表示默认摄像头，可以根据需要更改

if not cap.isOpened():
    print("无法打开摄像头")
    exit()

# 创建一个光流估计器
lk_params = dict(winSize=(15, 15), maxLevel=2, criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

# 初始化前一帧和前一帧的特征点
ret, prev_frame = cap.read()
prev_gray = cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY)
prev_points = cv2.goodFeaturesToTrack(prev_gray, mask=None, maxCorners=100, qualityLevel=0.3, minDistance=7, blockSize=7)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # 将当前帧转换为灰度图像
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # 计算光流
    current_points, status, _ = cv2.calcOpticalFlowPyrLK(prev_gray, gray, prev_points, None, **lk_params)

    # 仅保留有效的光流点
    good_new = current_points[status == 1]
    good_prev = prev_points[status == 1]

    # 创建一个黑色图像，用于可视化光流
    flow_image = np.zeros_like(frame)

    # 根据光流运动大小设置颜色
    for i, (new, prev) in enumerate(zip(good_new, good_prev)):
        a, b = new.ravel()
        c, d = prev.ravel()
        mask = cv2.line(flow_image, (int(a), int(b)), (int(c), int(d)), (0, 255, 0), 2)
        
        # 计算光流向量的大小
        flow_magnitude = np.sqrt((a - c)**2 + (b - d)**2)
        
        # 根据光流大小设置像素颜色
        if flow_magnitude > 2:  # 调整阈值以匹配您的需求
            frame_with_flow = cv2.circle(frame, (int(a), int(b)), 2, (255, 255, 255), -1)
        else:
            frame_with_flow = frame

    # 显示带有光流效果的图像
    cv2.imshow("Frame with Optical Flow", frame_with_flow)

    # 更新前一帧和前一帧的特征点
    prev_gray = gray.copy()
    prev_points = good_new.reshape(-1, 1, 2)

    # 退出循环的条件（按下 'q' 键）
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# 释放摄像头并关闭窗口
cap.release()
cv2.destroyAllWindows()
