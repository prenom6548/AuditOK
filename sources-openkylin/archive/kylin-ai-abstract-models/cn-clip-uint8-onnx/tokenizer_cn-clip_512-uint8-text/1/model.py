import numpy as np
from pathlib import Path
from transformers import AutoTokenizer

import os
os.environ["TOKENIZERS_PARALLELISM"] = "false"

# triton_python_backend_utils is available in every Triton Python model. You
# need to use this module to create inference requests and responses. It also
# contains some utility functions for extracting information from model_config
# and converting Triton input/output types to numpy types.

import triton_python_backend_utils as pb_utils

def adjust_array_length(arr, target_length=512):
    if len(arr) < target_length:
        # 如果数组长度小于目标长度，在末尾填充0
        arr = np.pad(arr, (0, target_length - len(arr)), 'constant', constant_values=0)
    else:
        # 如果数组长度大于目标长度，截断超出的部分
        arr = arr[:target_length]
    return arr

class TritonPythonModel:
    """Your Python model must use the same class name. Every Python model
    that is created must have "TritonPythonModel" as the class name.
    """

    def initialize(self, args):
        """`initialize` is called only once when the model is being loaded.
        Implementing `initialize` function is optional. This function allows
        the model to intialize any state associated with this model.
        Parameters
        ----------
        args : dict
          Both keys and values are strings. The dictionary keys and values are:
          * model_config: A JSON string containing the model configuration
          * model_instance_kind: A string containing model instance kind
          * model_instance_device_id: A string containing model instance device ID
          * model_repository: Model repository path
          * model_version: Model version
          * model_name: Model name
        """

        self.tokenizer = AutoTokenizer.from_pretrained(Path(__file__).parent.parent)

    def execute(self, requests):
        """`execute` MUST be implemented in every Python model. `execute`
        function receives a list of pb_utils.InferenceRequest as the only
        argument. This function is called when an inference request is made
        for this model. Depending on the batching configuration (e.g. Dynamic
        Batching) used, `requests` may contain multiple requests. Every
        Python model, must create one pb_utils.InferenceResponse for every
        pb_utils.InferenceRequest in `requests`. If there is an error, you can
        set the error argument when creating a pb_utils.InferenceResponse
        Parameters
        ----------
        requests : list
          A list of pb_utils.InferenceRequest
        Returns
        -------
        list
          A list of pb_utils.InferenceResponse. The length of this list must
          be the same as `requests`
        """

        if self.tokenizer is None:
            print("Tokenizer not available")
            return {}

        responses = []

        # Every Python backend must iterate over everyone of the requests
        # and create a pb_utils.InferenceResponse for each of them.
        for request in requests:
            # Get INPUT0
            input_strings = pb_utils.get_input_tensor_by_name(request, "product_reviews").as_numpy()

            first_string = input_strings[0]
            if isinstance(first_string, bytes):
              first_string = first_string.decode('utf-8')
            print("tokenize string ", first_string)

            # 将文本转换为 tokens
            tokens = self.tokenizer(first_string)
            print("tokens ", tokens)

            # 将列表转换为 NumPy 数组
            input_ids = np.array(tokens['input_ids'])
            attention_mask = np.array(tokens['attention_mask'])
            # 调整数组长度
            input_ids = adjust_array_length(input_ids)
            attention_mask = adjust_array_length(attention_mask)

            print("input_ids ", input_ids)
            print("attention_mask ", attention_mask)

            out_tensor_0 = pb_utils.Tensor("input_ids", input_ids.astype(np.int64))
            out_tensor_1 = pb_utils.Tensor("attention_mask", attention_mask.astype(np.int64))

            # Create InferenceResponse. You can set an error here in case
            # there was a problem with handling this inference request.
            # Below is an example of how you can set errors in inference
            # response:
            #
            # pb_utils.InferenceResponse(
            #    output_tensors=..., TritonError("An error occured"))
            inference_response = pb_utils.InferenceResponse(output_tensors=[out_tensor_0, out_tensor_1])
            responses.append(inference_response)

        # You should return a list of pb_utils.InferenceResponse. Length
        # of this list must match the length of `requests` list.
        return responses

    def finalize(self):
        """`finalize` is called only once when the model is being unloaded.
        Implementing `finalize` function is OPTIONAL. This function allows
        the model to perform any necessary clean ups before exit.
        """
        print('Cleaning up...')
