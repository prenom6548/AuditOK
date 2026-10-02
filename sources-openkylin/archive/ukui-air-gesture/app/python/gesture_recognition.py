"""
Copyright (C) 2026, KylinSoft Co., Ltd.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
import cv2
import mediapipe as mp
from collections import namedtuple
from pathlib import Path

from gesture_classifier import GestureClassifier
from virtual_camera import (
    activate_shared_camera,
    find_real_camera_node,
    open_loopback_writer,
)
from gesture_lru import LRUCache

from mediapipe.tasks import python as mp_python
from mediapipe.tasks.python import vision

from PIL import Image
import imagehash

# 摄像头采集配置
CAMERA_FRAME_WIDTH = 1280
CAMERA_FRAME_HEIGHT = 720
CAMERA_FPS = 10
SHOW_PREVIEW = False        # 后续无窗口模式时可保持False

# 每隔N帧进行检测识别 跳帧处理 若下游触发变迟钝可设为 1
DETECT_EVERY_N_FRAME = 2
# 汉明距离
HAMMING_THRESHOLD = 8
# 无手模式下，距上次真实检测超过该时长(毫秒)后强制重新检测
FORCE_REDETECT_INTERVAL_MS = 1000

class CacheImage:
    def __init__(self, h, hm, hn, df=None):
        self.hash = h                  # 图像哈希值
        self.handmarks = hm            # 关键点快照
        self.handedness = hn
        self.detect_frame = df         # 缩放后的帧

def _snapshot_landmarks(handmarks):
    if handmarks is None:
        return None
    return [(lm.x, lm.y, lm.z) for lm in handmarks]

_Landmark = namedtuple("_Landmark", "x y z")

def _restore_landmarks(snap):
    if snap is None:
        return None
    return [_Landmark(x, y, z) for x, y, z in snap]

# 将图像转换为感知哈希
def get_image_phash(img):
    if img is None:
        return None
    pil = Image.fromarray(img)
    return imagehash.phash(pil)

# 解析模型文件路径
def resolve_model_path(model_name="hand_landmarker.task"):
    script_dir = Path(__file__).resolve().parent
    model_path = script_dir.parent / "models" / model_name

    if model_path.exists():
        return str(model_path)

    print(f"Model file not found: {model_path}")
    return None

# 初始化手势识别配置
def initialize_hand_detector():
    model_path = resolve_model_path()
    if model_path is None:
        return None

    # 给mediapipe的类取别名
    BaseOptions = mp_python.BaseOptions                    # 基础配置类
    HandLandmarkerOptions = vision.HandLandmarkerOptions   # 手势识别配置类
    
    # 定义手势识别配置类对象    
    options = HandLandmarkerOptions(
        # 参数1：基础配置类，这里定义一个基础配置类对象并给其内部的参数传入模型文件路径
        base_options = BaseOptions(model_asset_path=model_path),
        # 参数2：手的最大检测数量
        num_hands = 1,
        # 参数3：手部检测的最小置信度，检测结果小于50%的手被忽略
        min_hand_detection_confidence=0.5,
        # 参数4：手部存在的最小置信度，识别结果高于50%才能确定这个手的持续存在
        min_hand_presence_confidence=0.5,
        # 参数5：开启视频流模式
        running_mode = vision.RunningMode.VIDEO,
    )
    # 根据配置类对象创建手势识别类对象并返回
    return vision.HandLandmarker.create_from_options(options)


# 缩放图像到合适的尺寸
def resize_image_for_detect(image, max_dim=640):
    h, w = image.shape[:2]
    if max(h, w) <= max_dim:
        return image
    scale = max_dim / max(h, w)
    new_w = int(w * scale)
    new_h = int(h * scale)

    return cv2.resize(image, (new_w, new_h), interpolation=cv2.INTER_AREA)


# 从单帧图像里识别手部坐标
def get_hand_landmarks_from_frame(frame, hand_detector, timestamp_ms):
    # 将OpenCV默认的BGR格式的帧图片转为RGB格式
    img_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    # 根据OpenCV的图片初始化由mediapipe处理的图片
    mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=img_rgb)
    # 启动手势识别（VIDEO 模式递增的 timestamp_ms）
    result = hand_detector.detect_for_video(mp_image, timestamp_ms)
    
    # res.hand_marks是所有手的所有关键点
    if result.hand_landmarks:
        # 获取第一个手的所有关键点
        landmarks = result.hand_landmarks[0]
        handedness = None

        # 获取左右手信息
        if result.handedness and len(result.handedness) > 0 and len(result.handedness[0]) > 0:
            handedness = result.handedness[0][0].category_name

        return landmarks, handedness
    return None, None

# 调试使用
def process_frame(landmarks, frame):
    # 使图像水平翻转，使图像实现镜面
    frame = cv2.flip(frame, 1)
    if landmarks:
        h, w = frame.shape[:2] # 获取当前图像宽高
        for lm in landmarks:
            cx = int((1 - lm.x) * w)
            cy = int(lm.y * h)
            color=(0,255,0)
            cv2.circle(frame, (cx, cy), 3, color, -1)

    return frame


def configure_camera_capture(cap, width=CAMERA_FRAME_WIDTH, height=CAMERA_FRAME_HEIGHT):
    # 请求设置采集分辨率，驱动可能回退到最接近的支持值
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
    cap.set(cv2.CAP_PROP_FPS, CAMERA_FPS)

    actual_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    actual_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    actual_fps = cap.get(cv2.CAP_PROP_FPS)
    print(
        "Camera capture config: "
        f"{actual_width}x{actual_height}, "
        f"fps={actual_fps:.1f}"
    )
    return actual_width, actual_height, actual_fps


def recognize_gesture():
    hand_detector = initialize_hand_detector()
    if hand_detector is None:
        print("Failed to initialize hand detector.")
        return

    activate_shared_camera()

    real_camera = find_real_camera_node()
    if not real_camera:
        print("Failed to find real camera.")
        return

    print(f"Opening real camera: {real_camera}")
    cap = cv2.VideoCapture(real_camera, cv2.CAP_V4L2)
    if not cap.isOpened():
        print(f"Failed to open camera: {real_camera}")
        return

    actual_width, actual_height, actual_fps = configure_camera_capture(cap)
    writer_fps = actual_fps if actual_fps and actual_fps > 0 else CAMERA_FPS
    loopback_writer = open_loopback_writer(actual_width, actual_height, writer_fps)

    classifier = GestureClassifier()

    frame_index = 0
    handmarks = None
    handedness = None
    last_cache_img = None
    last_real_detect_ms = 0        # 最近一次真实检测的时间戳(ms)
    lru_cache = LRUCache(80)

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            if loopback_writer is not None:
                if not loopback_writer.write(frame):
                    loopback_writer = None

            # VIDEO 模式：为每帧维护一个严格递增的时间戳（帧号 × 每帧毫秒数）
            timestamp_ms = int(frame_index * 1000 / max(writer_fps, 1))

            # 每隔 DETECT_EVERY_N_FRAME 帧识别一次（首帧也识别）
            if frame_index % DETECT_EVERY_N_FRAME == 0:
                # 先缩放识别用的帧，减少后续mediapipe内部处理压力
                detect_frame = resize_image_for_detect(frame)
                cur_phash = get_image_phash(detect_frame)
                hit = False

                if last_cache_img is not None:
                    # 对于有手的帧，判断 lru 中是否存在相似的帧
                    if last_cache_img.handmarks is not None:
                        hit = (
                            last_cache_img.detect_frame is not None
                            and lru_cache.get(cur_phash) is not None
                        )
                    else:
                        # 对于无手的帧，直接比对上一帧；
                        hit = (
                            (last_cache_img.hash - cur_phash) <= HAMMING_THRESHOLD
                            and (timestamp_ms - last_real_detect_ms) < FORCE_REDETECT_INTERVAL_MS
                        )

                if hit:
                    if last_cache_img.handmarks is not None:
                        handinfo = lru_cache.get(cur_phash)
                        handmarks = _restore_landmarks(handinfo.handmarks)
                        handedness = handinfo.handedness
                    else:
                        handmarks = _restore_landmarks(last_cache_img.handmarks)
                        handedness = last_cache_img.handedness

                if not hit:
                    handmarks, handedness = get_hand_landmarks_from_frame(
                        detect_frame, hand_detector, timestamp_ms)
                    # 记录本次真实检测时间戳，供无手帧的强制重新检测使用
                    last_real_detect_ms = timestamp_ms
                    # 如果是有手的帧，则应存入lru中
                    if handmarks is not None:
                        lru_cache.put(cur_phash, _snapshot_landmarks(handmarks), handedness)

                last_cache_img = CacheImage(
                    cur_phash,
                    _snapshot_landmarks(handmarks),
                    handedness,
                    detect_frame,
                )

                classifier.classify(handmarks, handedness, frame_index)


            frame_index += 1

            if SHOW_PREVIEW:
                preview = process_frame(handmarks, frame)
                cv2.imshow("hand", preview)
                key = cv2.waitKey(1) & 0xFF
                if key in (27, ord('q')):
                    break
    finally:
        if loopback_writer is not None:
            loopback_writer.close()
        cap.release()
        cv2.destroyAllWindows()
