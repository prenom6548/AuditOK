# -*- encoding:utf-8 -*-
import hashlib
import hmac
import base64
from socket import *
import json, time, threading
from websocket import create_connection
import websocket
from urllib.parse import quote
import logging
import pyaudio
import argparse
import tritonclient.grpc as grpcclient
from speech_client import *
from funasr import AutoModel
import random

recording_event = threading.Event()
asr_streaming_model_name = "ensemble-asr_paraformer_streaming"
# asr_streaming_model_name = "ensemble-asr_paraformer_large-streaming"
asr_nonstreaming_model_name = "ensemble-asr_paraformer_nonstreaming"
vad_model_name = "ensemble-asr_fsmn-vad_streaming"
punc_model_name = "ensemble-punc_ct-transformer_nonstreaming"
sequence_id = random.randint(0, 100)

class AudioProcessingManager:
    def __init__(self, triton_client, streaming_model_name, nonstreaming_model_name, protocol_client, args):
        self.total_frames = []
        self.offline_asr_frames = []
        self.text_print_2pass_online = ""
        self.text_print_2pass_offline = ""
        self.status_dict_vad = {"cache": {}, "chunk_size": 60}
        self.status_dict_punc = {"cache": {}}
        self.recording_time = 0  # ms
        self.is_speech_started = False
        self.is_vad_speech_start = False
        self.is_vad_speech_end = False
        self.triton_client = triton_client
        self.protocol_client = protocol_client
        self.low_cache = True
        # self.load_models()

        # 初始化客户端和音频资源
        self.speech_client_streaming = StreamingSpeechClient(triton_client, streaming_model_name, protocol_client, args)
        self.speech_client_nonstreaming = OfflineSpeechClient(triton_client, nonstreaming_model_name, protocol_client, args)
        self.audio = None
        self.stream = None

    def load_models(self):
        if not hasattr(self, 'model_vad'):
            self.model_vad = AutoModel(
                model="iic/speech_fsmn_vad_zh-cn-16k-common-pytorch",
                model_revision="v2.0.4",
                ngpu=0,
                ncpu=4,
                device="cpu",
                disable_pbar=True,
                disable_log=True,
                disable_update=True,
            )

    def bytes_to_float_array(self, data):
        return np.frombuffer(data, dtype=np.int16).astype(np.float32) / 32768.0

    def speechVad(self, audio_in, is_start = False, is_end = False):
        global vad_model_name, sequence_id
        # if is_start == True or is_end == True:
        #     print(f"speechVad: {sequence_id}, is_start: {is_start}, is_end: {is_end}")
        waveform = self.bytes_to_float_array(audio_in)
        waveform = np.expand_dims(waveform, axis=0) # 从(960, )变为(1, 960)
        inputs = [
            self.protocol_client.InferInput(
                "WAV", waveform.shape, np_to_triton_dtype(waveform.dtype)
            ),
        ]
        inputs[0].set_data_from_numpy(waveform)
        outputs = [
            self.protocol_client.InferRequestedOutput("OUTPUT0")
        ]
        try:
            response = self.triton_client.infer(model_name=vad_model_name,
                                    inputs=inputs,
                                    outputs=outputs,
                                    sequence_id=sequence_id,
                                    sequence_start=is_start,
                                    sequence_end=is_end,
            )
        except Exception as e:
            print(f"Triton 推理失败: {e}")
            return -1, -1

        segments_result = response.as_numpy("OUTPUT0")
        speech_start = -1
        speech_end = -1

        if segments_result.size == 0:
            return speech_start, speech_end
        speech_start = segments_result[0]
        speech_end = segments_result[1]
        return speech_start, speech_end

    def speechVadOld(self, audio_in):
        segments_result = self.model_vad.generate(input=audio_in, **self.status_dict_vad, is_final=False)[0]["value"]
        speech_start = -1
        speech_end = -1

        if not segments_result or len(segments_result) > 1:
            return speech_start, speech_end
        if segments_result[0][0] != -1:
            speech_start = segments_result[0][0]
        if segments_result[0][1] != -1:
            speech_end = segments_result[0][1]
        return speech_start, speech_end

    def textPunc(self, text):
        # print(f"模型输入：{text}")
        global punc_model_name
        input_array = np.array([[text]], dtype=object)
        inputs = [
            self.protocol_client.InferInput("text_no_punc", input_array.shape, "BYTES")
        ]
        inputs[0].set_data_from_numpy(input_array)
        outputs = [
            self.protocol_client.InferRequestedOutput("OUTPUT0")
        ]

        try:
            response = self.triton_client.infer(model_name=punc_model_name,
                                        model_version="",
                                        inputs=inputs,
                                        outputs=outputs,
                                        timeout=10000
            )

            output_data = response.as_numpy("OUTPUT0")[0]
            if isinstance(output_data, np.ndarray):
                result = b" ".join(output_data).decode("utf-8")
            else:
                result = output_data.decode("utf-8")
            # print(f"模型输出：{result}")

        except InferenceServerException as e:
            print("Triton 推理错误:", e)

        return result

    def processAudioFromMicrophone(self):
        global sequence_id
        FORMAT = pyaudio.paInt16    # 使用 16 位来表示每个采样点
        CHANNELS = 1
        SAMPLE_RATE = 16000
        CHUNK_SIZE_MS = 60             # 60ms，40ms不足以支撑vad模型
        CHUNK = int(SAMPLE_RATE / 1000 * CHUNK_SIZE_MS)
        self.audio = pyaudio.PyAudio()
        self.stream = self.audio.open(format=FORMAT, channels=CHANNELS, rate=SAMPLE_RATE, input=True, frames_per_buffer=CHUNK)
        try:
            while recording_event.is_set():
                data = self.stream.read(CHUNK)
                self.total_frames.append(data)
                self.recording_time += CHUNK_SIZE_MS
                if self.is_vad_speech_end == True:
                    self.speechVad(data, is_start = False, is_end = True)
                    self.is_vad_speech_end = False
                    continue
                speech_start_i, speech_end_i = self.speechVad(data, is_start = (self.is_speech_started == False), is_end = (self.is_vad_speech_end == True))
                # speech_start_i_old, speech_end_i_old = self.speechVadOld(data)
                self.is_speech_started = True
                # if speech_start_i != -1 or speech_end_i != -1 or speech_start_i_old != -1 or speech_end_i_old != -1:
                #     print(f"\nspeech_start: {speech_start_i}, speech_end: {speech_end_i}, speech_start_old: {speech_start_i_old}, speech_end_old: {speech_end_i_old}")

                # 将 bytes 转换为 numpy float32 数组
                waveform = self.bytes_to_float_array(data)
                result = self.speech_client_streaming.recognize(waveform, idx = sequence_id, is_end = speech_end_i != -1)
                if len(result) > 0:
                    self.text_print_2pass_online += "{}".format(result)
                    text_print = self.text_print_2pass_offline + self.text_print_2pass_online
                    print(f"\rresult: {text_print}", end="")

                if self.is_vad_speech_start == True:
                    self.offline_asr_frames.append(data)
                if speech_start_i != -1:
                    self.is_vad_speech_start = True
                    beg_bias = (self.recording_time - speech_start_i) // CHUNK_SIZE_MS
                    # print(f"self.recording_time: {self.recording_time}, speech_start_i: {speech_start_i}, beg_bias: {beg_bias}, frame_len: {len(self.total_frames)}")
                    frames_pre = self.total_frames[-beg_bias:]
                    self.offline_asr_frames.extend(frames_pre)
                if speech_end_i != -1:
                    audio = b"".join(self.offline_asr_frames)
                    waveform = self.bytes_to_float_array(audio)
                    result = self.speech_client_nonstreaming.recognize(waveform, idx = sequence_id)
                    # 空字符串不可以输入 punc 模型
                    if len(result) > 0:
                        result = self.textPunc(result)
                    self.text_print_2pass_offline += "{}".format(result)
                    text_print = self.text_print_2pass_offline
                    print(f"\rresult: {text_print}", end="")
                    self.text_print_2pass_online = ""
                    self.is_vad_speech_start = False
                    self.offline_asr_frames = []

                    if self.low_cache == True:
                        self.is_speech_started = False
                        self.is_vad_speech_end = True
                        self.recording_time = 0
                        self.total_frames = []
                        self.status_dict_vad["cache"] = {}

                time.sleep(0.005)
        except Exception as e:
            logging.error(f"Error during audio processing: {e}")
        finally:
            self.stopRecording()

    def stopRecording(self):
        logging.info("停止录音...")
        try:
            # 非阻塞地读取 stream 中剩余的音频数据
            read_available = self.stream.get_read_available()
            print("last recording", self.stream.is_active(), read_available)
            if self.stream.is_active() and self.stream.get_read_available() > 0:
                read_size = (read_available // 2) * 2
                data = self.stream.read(read_size, exception_on_overflow=False)
            else:
                data = b"  "
            speech_start_i, speech_end_i = self.speechVad(data, is_start = False, is_end = True)

        except Exception as e:
            logging.error(f"Error reading remaining audio data: {e}")


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        required=False,
        default=False,
        help="Enable verbose output",
    )
    parser.add_argument(
        "-u",
        "--url",
        type=str,
        required=False,
        default="localhost:8001",
        help="Inference server URL. Default is " "localhost:8001.",
    )
    parser.add_argument(
        "--sample_rate",
        type=int,
        required=False,
        default=16000,
        help="sample rate used in training",
    )
    parser.add_argument(
        "--frame_length_ms",
        type=int,
        required=False,
        default=25,
        help="frame length",
    )
    parser.add_argument(
        "--frame_shift_ms",
        type=int,
        required=False,
        default=10,
        help="frame shift length",
    )
    parser.add_argument(
        "--chunk_size",
        type=int,
        required=False,
        default=16,
        help="chunk size default is 16",
    )
    parser.add_argument(
        "--context",
        type=int,
        required=False,
        default=7,
        help="subsampling context",
    )
    parser.add_argument(
        "--subsampling",
        type=int,
        required=False,
        default=4,
        help="subsampling rate",
    )

    FLAGS = parser.parse_args()
    print(FLAGS)

    with grpcclient.InferenceServerClient(
            url=FLAGS.url, verbose=FLAGS.verbose
        ) as triton_client:
            protocol_client = grpcclient
            try:
                # 请求服务端加载模型
                triton_client.load_model(asr_streaming_model_name)
                triton_client.load_model(asr_nonstreaming_model_name)
                triton_client.load_model(vad_model_name)
                triton_client.load_model(punc_model_name)
            except InferenceServerException as e:
                logging.error(f"Failed to load model: {e}")
                exit(1)

            manager = AudioProcessingManager(
                triton_client=triton_client,
                streaming_model_name=asr_streaming_model_name,
                nonstreaming_model_name=asr_nonstreaming_model_name,
                protocol_client=grpcclient,
                args=FLAGS
            )

            try:
                logging.info("按回车开始实时转写...")
                input()
                recording_event.set()  # 开始录音
                recording_thread = threading.Thread(target=manager.processAudioFromMicrophone)
                recording_thread.daemon = True  # 设置为守护线程
                recording_thread.start()
                logging.info("正在聆听，按回车结束转写...")
                input()
                recording_event.clear()  # 结束录音
                recording_thread.join()  # 等待录音线程结束
            except Exception as e:
                logging.error(f"录音设备不可用：{e}")
