# Copyright (c) 2021, NVIDIA CORPORATION.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from tritonclient.utils import np_to_triton_dtype, InferenceServerException
import numpy as np
import math
import time

class OfflineSpeechClient(object):
    def __init__(self, triton_client, model_name, protocol_client, args):
        self.triton_client = triton_client
        self.protocol_client = protocol_client
        self.model_name = model_name

    def recognize(self, audio, idx=0):
        samples = np.array([audio])
        lengths = np.array([[len(audio)]], dtype=np.int32)
        # better pad waveform to nearest length here
        # target_seconds = math.cel(len(waveform) / sample_rate)
        # target_samples = np.zeros([1, target_seconds  * sample_rate])
        # target_samples[0][0: len(waveform)] = waveform
        # samples = target_samples
        sequence_id = 10086 + idx
        result = ""
        inputs = [
            self.protocol_client.InferInput(
                "WAV", samples.shape, np_to_triton_dtype(samples.dtype)
            ),
            self.protocol_client.InferInput(
                "WAV_LENS", lengths.shape, np_to_triton_dtype(lengths.dtype)
            ),
        ]
        inputs[0].set_data_from_numpy(samples)
        inputs[1].set_data_from_numpy(lengths)
        outputs = [self.protocol_client.InferRequestedOutput("TRANSCRIPTS")]
        try:
            response = self.triton_client.infer(
                self.model_name,
                inputs,
                request_id=str(sequence_id),
                outputs=outputs,
            )
        except InferenceServerException as e:
            print(f"Triton Error: {e.__dict__}")
        decoding_results = response.as_numpy("TRANSCRIPTS")[0]
        if type(decoding_results) == np.ndarray:
            result = b" ".join(decoding_results).decode("utf-8")
        else:
            result = decoding_results.decode("utf-8")
        return result


class StreamingSpeechClient(object):
    def __init__(self, triton_client, model_name, protocol_client, args):
        self.triton_client = triton_client
        self.protocol_client = protocol_client
        self.model_name = model_name
        self.is_begin = True
        self.cached_length = 0
        self.expect_input = None
        self.sample_rate = args.sample_rate
        chunk_size = args.chunk_size
        subsampling = args.subsampling
        context = args.context
        frame_shift_ms = args.frame_shift_ms
        frame_length_ms = args.frame_length_ms
        # for the first chunk
        # we need additional frames to generate
        # the exact first chunk length frames
        # since the subsampling will look ahead several frames
        self.first_chunk_length = (chunk_size - 1) * subsampling + context
        # 应该不需要，先删除试试？
        self.add_frames = math.ceil((frame_length_ms - frame_shift_ms) / frame_shift_ms)
        self.other_chunk_length = chunk_size * subsampling
        first_chunk_ms = (self.first_chunk_length + self.add_frames) * frame_shift_ms
        other_chunk_ms = self.other_chunk_length * frame_shift_ms
        self.first_chunk_in_secs = first_chunk_ms / 1000
        self.other_chunk_in_secs = other_chunk_ms / 1000
        print(f"first_chunk_in_secs: {self.first_chunk_in_secs}, other_chunk_in_secs: {self.other_chunk_in_secs}")

    def recognize(self, audio, idx=0, is_end=False):
        sequence_id = idx + 10086
        if self.is_begin == True:
            chunk_samples = int(self.first_chunk_in_secs * self.sample_rate)
            if self.cached_length == 0:
                self.expect_input = np.zeros((1, chunk_samples), dtype=np.float32)

            if self.cached_length + len(audio) < chunk_samples:
                cached_length = self.cached_length + len(audio)
                self.expect_input[0][self.cached_length:cached_length] = audio
                self.cached_length = cached_length
                if is_end != True:
                    return []
                input0_data = self.expect_input
                input1_data = np.array([[self.cached_length]], dtype=np.int32)
            else:
                part_length = chunk_samples - self.cached_length
                self.expect_input[0][self.cached_length:] = audio[0:part_length]
                input0_data = self.expect_input
                input1_data = np.array([[chunk_samples]], dtype=np.int32)
                chunk_samples = int(self.other_chunk_in_secs * self.sample_rate)
                self.expect_input = np.zeros((1, chunk_samples), dtype=np.float32)
                left_length = len(audio) - part_length
                self.expect_input[0][0:left_length] = audio[part_length:]
                self.cached_length = left_length
        else:
            if self.cached_length + len(audio) < self.expect_input.shape[1]:
                cached_length = self.cached_length + len(audio)
                self.expect_input[0][self.cached_length:cached_length] = audio
                self.cached_length = cached_length
                if is_end != True:
                    return []
                input0_data = self.expect_input
                input1_data = np.array([[self.cached_length]], dtype=np.int32)
            else:
                part_length = self.expect_input.shape[1] - self.cached_length
                self.expect_input[0][self.cached_length:] = audio[0:part_length]
                input0_data = self.expect_input
                input1_data = np.array([[self.expect_input.shape[1]]], dtype=np.int32)
                left_length = len(audio) - part_length
                self.expect_input[0][0:left_length] = audio[part_length:]
                self.cached_length = left_length


        inputs = [
            self.protocol_client.InferInput(
                "WAV",
                input0_data.shape,
                np_to_triton_dtype(input0_data.dtype),
            ),
            self.protocol_client.InferInput(
                "WAV_LENS",
                input1_data.shape,
                np_to_triton_dtype(input1_data.dtype),
            ),
        ]
        print(f"input0_data.shape: ", input0_data.shape)

        inputs[0].set_data_from_numpy(input0_data)
        inputs[1].set_data_from_numpy(input1_data)
        outputs = [self.protocol_client.InferRequestedOutput("TRANSCRIPTS")]

        try:
            # print(f"online recognize, is_start: {self.is_begin}, is_end: {is_end}")
            response = self.triton_client.infer(
                self.model_name,
                inputs,
                outputs=outputs,
                sequence_id=sequence_id,
                sequence_start=self.is_begin,
                sequence_end=is_end,
            )
            self.is_begin = False
        except InferenceServerException as e:
            print(f"Triton Error: {e.__dict__}")

        if is_end:
            self.is_begin = True
            self.cached_length = 0
        result = response.as_numpy("TRANSCRIPTS")[0].decode("utf-8")
        return result
