import cv2
import numpy as np

# 打开摄像头（通常0表示默认摄像头）
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()

    if not ret:
        break

    # 将图像转换为HSV颜色空间
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # 定义肤色的HSV范围
    lower_skin = np.array([0, 20, 70], dtype=np.uint8)
    upper_skin = np.array([20, 255, 255], dtype=np.uint8)

    # 根据肤色范围创建掩码
    mask = cv2.inRange(hsv, lower_skin, upper_skin)

    # 对掩码进行形态学操作，以去除噪声
    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.dilate(mask, kernel, iterations=1)
    mask = cv2.erode(mask, kernel, iterations=1)

    # 寻找手掌轮廓
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # 寻找最大的轮廓，即手掌轮廓
    if len(contours) > 0:
        max_contour = max(contours, key=cv2.contourArea)

        # 获取手掌的外接矩形框
        x, y, w, h = cv2.boundingRect(max_contour)

        # 绘制手掌框
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    # 显示图像
    cv2.imshow("Hand Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
