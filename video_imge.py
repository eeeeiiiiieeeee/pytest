import cv2
import os

# 打开视频文件
video_path = 'C:\\Users\\test\\Desktop\\5.mp4'  # 替换为你的视频文件路径
cap = cv2.VideoCapture(video_path)

# 检查视频是否成功打开
if not cap.isOpened():
    print("无法打开视频文件")
    exit()

# 创建保存图片的文件夹
output_folder = 'frames_output'
if not os.path.exists(output_folder):
    os.makedirs(output_folder)

# 循环读取视频帧并保存为图片
frame_count = 0
while True:
    ret, frame = cap.read()
    if not ret:
        break
    
    # 保存帧为图片
    image_filename = os.path.join(output_folder, f'frame_{frame_count:04d}.jpg')
    cv2.imwrite(image_filename, frame)
    
    frame_count += 1

# 关闭视频流
cap.release()


