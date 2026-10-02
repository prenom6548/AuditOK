// Copyright 2021, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "triton/backend/backend_common.h"
#include "triton/backend/backend_input_collector.h"
#include "triton/backend/backend_model.h"
#include "triton/backend/backend_model_instance.h"
#include "triton/backend/backend_output_responder.h"
#include "triton/core/tritonbackend.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <filesystem>
#include "ocr.h"

namespace triton { namespace backend { namespace minimal {

//
// Minimal backend that demonstrates the TRITONBACKEND API. This
// backend works for any model that has 1 input called "IN0" with
// INT32 datatype and shape [ 4 ] and 1 output called "OUT0" with
// INT32 datatype and shape [ 4 ]. The backend supports both batching
// and non-batching models.
//
// For each batch of requests, the backend returns the input tensor
// value in the output tensor.
//

/////////////

//
// ModelState
//
// State associated with a model that is using this backend. An object
// of this class is created and associated with each
// TRITONBACKEND_Model. ModelState is derived from BackendModel class
// provided in the backend utilities that provides many common
// functions.
//
class ModelState : public BackendModel {
 public:
  OCRDetector *detect_model; // 添加 OCRDetector 实例
  static TRITONSERVER_Error* Create(
      TRITONBACKEND_Model* triton_model, ModelState** state);
  virtual ~ModelState() = default;

 private:
  ModelState(TRITONBACKEND_Model* triton_model) : BackendModel(triton_model) {}
};

TRITONSERVER_Error*
ModelState::Create(TRITONBACKEND_Model* triton_model, ModelState** state)
{
  try {
    *state = new ModelState(triton_model);
  }
  catch (const BackendModelException& ex) {
    RETURN_ERROR_IF_TRUE(
        ex.err_ == nullptr, TRITONSERVER_ERROR_INTERNAL,
        std::string("unexpected nullptr in BackendModelException"));
    RETURN_IF_ERROR(ex.err_);
  }

    TRITONBACKEND_ArtifactType artifact_type;
    const char* model_location;
    TRITONSERVER_Error* err = TRITONBACKEND_ModelRepository(triton_model, &artifact_type, &model_location);
    if (err != nullptr) {
        std::cerr << "Error getting model repository path: " << TRITONSERVER_ErrorMessage(err) << std::endl;
        return err; // 返回错误
    }

    std::string model_location_str = model_location;
    model_location_str += "/1/";
    if (!std::filesystem::is_regular_file(model_location_str + "model_det.onnx")) {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INVALID_ARG,
            ("Invalid model file: " + model_location_str + "model_det.onnx").c_str());
    }
    if (!std::filesystem::is_regular_file(model_location_str + "model_rec.onnx")) {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INVALID_ARG,
            ("Invalid model file: " + model_location_str + "model_rec.onnx").c_str());
    }
    if (!std::filesystem::is_regular_file(model_location_str + "ppocr_keys_v1.txt")) {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INVALID_ARG,
            ("Invalid model file: " + model_location_str + "ppocr_keys_v1.txt").c_str());
    }
    (*state)->detect_model = new OCRDetector(model_location_str.c_str());


  return nullptr;  // success
}

extern "C" {

// Triton calls TRITONBACKEND_ModelInitialize when a model is loaded
// to allow the backend to create any state associated with the model,
// and to also examine the model configuration to determine if the
// configuration is suitable for the backend. Any errors reported by
// this function will prevent the model from loading.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInitialize(TRITONBACKEND_Model* model)
{
  // Create a ModelState object and associate it with the
  // TRITONBACKEND_Model. If anything goes wrong with initialization
  // of the model state then an error is returned and Triton will fail
  // to load the model.
  ModelState* model_state;

  RETURN_IF_ERROR(ModelState::Create(model, &model_state));
  RETURN_IF_ERROR(
      TRITONBACKEND_ModelSetState(model, reinterpret_cast<void*>(model_state)));

  return nullptr;  // success
}

// Triton calls TRITONBACKEND_ModelFinalize when a model is no longer
// needed. The backend should cleanup any state associated with the
// model. This function will not be called until all model instances
// of the model have been finalized.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelFinalize(TRITONBACKEND_Model* model)
{
  void* vstate;
  RETURN_IF_ERROR(TRITONBACKEND_ModelState(model, &vstate));
  ModelState* model_state = reinterpret_cast<ModelState*>(vstate);
  delete model_state;

  return nullptr;  // success
}

}  // extern "C"

/////////////

//
// ModelInstanceState
//
// State associated with a model instance. An object of this class is
// created and associated with each
// TRITONBACKEND_ModelInstance. ModelInstanceState is derived from
// BackendModelInstance class provided in the backend utilities that
// provides many common functions.
//
class ModelInstanceState : public BackendModelInstance {
 public:
  static TRITONSERVER_Error* Create(
      ModelState* model_state,
      TRITONBACKEND_ModelInstance* triton_model_instance,
      ModelInstanceState** state);
  virtual ~ModelInstanceState() = default;

  // Get the state of the model that corresponds to this instance.
  ModelState* StateForModel() const { return model_state_; }

 private:
  ModelInstanceState(
      ModelState* model_state,
      TRITONBACKEND_ModelInstance* triton_model_instance)
      : BackendModelInstance(model_state, triton_model_instance),
        model_state_(model_state)
  {
  }

  ModelState* model_state_;
};

TRITONSERVER_Error*
ModelInstanceState::Create(
    ModelState* model_state, TRITONBACKEND_ModelInstance* triton_model_instance,
    ModelInstanceState** state)
{
  try {
    *state = new ModelInstanceState(model_state, triton_model_instance);
  }
  catch (const BackendModelInstanceException& ex) {
    RETURN_ERROR_IF_TRUE(
        ex.err_ == nullptr, TRITONSERVER_ERROR_INTERNAL,
        std::string("unexpected nullptr in BackendModelInstanceException"));
    RETURN_IF_ERROR(ex.err_);
  }

  return nullptr;  // success
}

extern "C" {

// Triton calls TRITONBACKEND_ModelInstanceInitialize when a model
// instance is created to allow the backend to initialize any state
// associated with the instance.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInstanceInitialize(TRITONBACKEND_ModelInstance* instance)
{
  // Get the model state associated with this instance's model.
  TRITONBACKEND_Model* model;
  RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceModel(instance, &model));

  void* vmodelstate;
  RETURN_IF_ERROR(TRITONBACKEND_ModelState(model, &vmodelstate));
  ModelState* model_state = reinterpret_cast<ModelState*>(vmodelstate);

  // Create a ModelInstanceState object and associate it with the
  // TRITONBACKEND_ModelInstance.
  ModelInstanceState* instance_state;
  RETURN_IF_ERROR(
      ModelInstanceState::Create(model_state, instance, &instance_state));
  RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceSetState(
      instance, reinterpret_cast<void*>(instance_state)));

  return nullptr;  // success
}

// Triton calls TRITONBACKEND_ModelInstanceFinalize when a model
// instance is no longer needed. The backend should cleanup any state
// associated with the model instance.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInstanceFinalize(TRITONBACKEND_ModelInstance* instance)
{
  void* vstate;
  RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceState(instance, &vstate));
  ModelInstanceState* instance_state =
      reinterpret_cast<ModelInstanceState*>(vstate);
  delete instance_state;

  return nullptr;  // success
}

}  // extern "C"

/////////////

extern "C" {

// When Triton calls TRITONBACKEND_ModelInstanceExecute it is required
// that a backend create a response for each request in the batch. A
// response may be the output tensors required for that request or may
// be an error that is returned in the response.
//

TRITONSERVER_Error*
PrepareInferenceRequest(ModelState* model_state, 
    TRITONBACKEND_Request* bls_request,
    TRITONSERVER_InferenceRequest** irequest, const std::string model_name)
{
  // Get request_id, correlation_id, and flags from the current request
  // for preparing a new inference request that we will send to 'addsub_python'
  // or 'addsub_tf' model later.
  const char* request_id;
  uint64_t correlation_id;
  uint32_t flags;
  RETURN_IF_ERROR(TRITONBACKEND_RequestId(bls_request, &request_id));
  RETURN_IF_ERROR(
      TRITONBACKEND_RequestCorrelationId(bls_request, &correlation_id));
  RETURN_IF_ERROR(TRITONBACKEND_RequestFlags(bls_request, &flags));

  // Create an inference request object. The inference request object
  // is where we set the name of the model we want to use for
  // inference and the input tensors.
  RETURN_IF_ERROR(TRITONSERVER_InferenceRequestNew(
      irequest, model_state->TritonServer(), model_name.c_str(), -1 /* model_version */));

  return nullptr;  // success
}

cv::Mat request_to_mat(
    TRITONBACKEND_Request* bls_request, TRITONSERVER_InferenceRequest* irequest)
{
  // Get the properties of the two inputs from the current request.
  // Then, add the two input tensors and append the input data to the new
  // request.
  uint32_t input_count;
  TRITONBACKEND_RequestInputCount(bls_request, &input_count);
    // std::cout << "execute_custom---------------" << input_count << std::endl;

  TRITONBACKEND_Input* input;
  const char* name;
  TRITONSERVER_DataType datatype;
  const int64_t* shape;
  uint32_t dims_count;
  size_t data_byte_size;
  TRITONSERVER_MemoryType data_memory_type;
  int64_t data_memory_id;
  const char* data_buffer;

  for (size_t count = 0; count < input_count; count++) {
    TRITONBACKEND_RequestInputByIndex(
        bls_request, count /* index */, &input);
    TRITONBACKEND_InputProperties(
        input, &name, &datatype, &shape, &dims_count, nullptr, nullptr);
    TRITONBACKEND_InputBuffer(
        input, 0 /* idx */, reinterpret_cast<const void**>(&data_buffer),
        &data_byte_size, &data_memory_type, &data_memory_id);
  }
  
  // cv::Mat srcimg(height, width, cv::CV_8UC3, (void*)input_buffer);
  cv::Mat srcimg(shape[0], shape[1], CV_8UC3, (void*)data_buffer);
  return srcimg;  // success
}

TRITONSERVER_Error*
TRITONBACKEND_ModelInstanceExecute(
    TRITONBACKEND_ModelInstance* instance, TRITONBACKEND_Request** requests,
    const uint32_t request_count)
{
  // Triton will not call this function simultaneously for the same
  // 'instance'. But since this backend could be used by multiple
  // instances from multiple models the implementation needs to handle
  // multiple calls to this function at the same time (with different
  // 'instance' objects). Best practice for a high-performance
  // implementation is to avoid introducing mutex/lock and instead use
  // only function-local and model-instance-specific state.
  ModelInstanceState* instance_state;
  RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceState(
      instance, reinterpret_cast<void**>(&instance_state)));
  ModelState* model_state = instance_state->StateForModel();

  // 'responses' is initialized as a parallel array to 'requests',
  // with one TRITONBACKEND_Response object for each
  // TRITONBACKEND_Request object. If something goes wrong while
  // creating these response objects, the backend simply returns an
  // error from TRITONBACKEND_ModelInstanceExecute, indicating to
  // Triton that this backend did not create or send any responses and
  // so it is up to Triton to create and send an appropriate error
  // response for each request. RETURN_IF_ERROR is one of several
  // useful macros for error handling that can be found in
  // backend_common.h.

  std::vector<TRITONBACKEND_Response*> responses;
  responses.reserve(request_count);
  for (uint32_t r = 0; r < request_count; ++r) {
    TRITONBACKEND_Request* request = requests[r];
    TRITONBACKEND_Response* response;
    RETURN_IF_ERROR(TRITONBACKEND_ResponseNew(&response, request));
    responses.push_back(response);
  }

  // At this point, the backend takes ownership of 'requests', which
  // means that it is responsible for sending a response for every
  // request. From here, even if something goes wrong in processing,
  // the backend must return 'nullptr' from this function to indicate
  // success. Any errors and failures must be communicated via the
  // response objects.
  //
  // To simplify error handling, the backend utilities manage
  // 'responses' in a specific way and it is recommended that backends
  // follow this same pattern. When an error is detected in the
  // processing of a request, an appropriate error response is sent{
//   for (size_t count = 0; count < input_count; count++) {
//     input_data_fp32[count] = reinterpret_cast<const float*>(data_buffer);
//   }

//     for (size_t i = 0; i < sizeof(input_data_fp32); i++)
//     {
//       std::cout << input_data_fp32[0][i] + input_data_fp32[1][i] << " ";
//       userp[0][i] = input_data_fp32[0][i] + input_data_fp32[1][i];
//       userp[1][i] = input_data_fp32[0][i] - input_data_fp32[1][i];
//     }
//     std::cout << "\n" << std::endl;
// }
  // backend_common.h that assist in this management of response
  // objects.

  // The backend could iterate over the 'requests' and process each
  // one separately. But for performance reasons it is usually
  // preferred to create batched input tensors that are processed
  // simultaneously. This is especially true for devices like GPUs
  // that are capable of exploiting the large amount parallelism
  // exposed by larger data sets.
  //
  // The backend utilities provide a "collector" to facilitate this
  // batching process. The 'collector's ProcessTensor function will
  // combine a tensor's value from each request in the batch into a
  // single contiguous buffer. The buffer can be provided by the
  // backend or 'collector' can create and manage it. In this backend,
  // there is not a specific buffer into which the batch should be
  // created, so use ProcessTensor arguments that cause collector to
  // manage it.

  BackendInputCollector collector(
      requests, request_count, &responses, model_state->TritonMemoryManager(),
      false /* pinned_enabled */, nullptr /* stream*/);

  // To instruct ProcessTensor to "gather" the entire batch of IN0
  // input tensors into a single contiguous buffer in CPU memory, set
  // the "allowed input types" to be the CPU ones (see tritonserver.h
  // in the triton-inference-server/core repo for allowed memory
  // types).
  std::vector<std::pair<TRITONSERVER_MemoryType, int64_t>> allowed_input_types =
      {{TRITONSERVER_MEMORY_CPU_PINNED, 0}, {TRITONSERVER_MEMORY_CPU, 0}};

  const char* input_buffer;
  size_t input_buffer_byte_size;
  TRITONSERVER_MemoryType input_buffer_memory_type;
  int64_t input_buffer_memory_type_id;

  RESPOND_ALL_AND_SET_NULL_IF_ERROR(
      responses, request_count,
      collector.ProcessTensor(
          "IN0", nullptr /* existing_buffer */,
          0 /* existing_buffer_byte_size */, allowed_input_types, &input_buffer,
          &input_buffer_byte_size, &input_buffer_memory_type,
          &input_buffer_memory_type_id));

  // Finalize the collector. If 'true' is returned, 'input_buffer'
  // will not be valid until the backend synchronizes the CUDA
  // stream or event that was used when creating the collector. For
  // this backend, GPU is not supported and so no CUDA sync should
  // be needed; so if 'true' is returned simply log an error.
  const bool need_cuda_input_sync = collector.Finalize();
  if (need_cuda_input_sync) {
    LOG_MESSAGE(
        TRITONSERVER_LOG_ERROR,
        "'minimal' backend: unexpected CUDA sync required by collector");
  }

  // 'input_buffer' contains the batched "IN0" tensor. The backend can
  // implement whatever logic is necessary to produce "OUT0". This
  // backend simply returns the IN0 value in OUT0 so no actual
  // computation is needed.

  LOG_MESSAGE(
      TRITONSERVER_LOG_INFO,
      (std::string("model ") + model_state->Name() + ": requests in batch " +
       std::to_string(request_count))
          .c_str());
  // std::string tstr;
  // IGNORE_ERROR(BufferAsTypedString(
  //     tstr, input_buffer, input_buffer_byte_size, TRITONSERVER_TYPE_UINT8));

  //   LOG_MESSAGE(
  //     TRITONSERVER_LOG_INFO,
  //     (std::string("tstr.size(): ") + std::to_string(tstr.size())).c_str());


  TRITONSERVER_InferenceRequest* irequest = nullptr;
  PrepareInferenceRequest(model_state, *requests, &irequest, "minimal");
  
  cv::Mat srcimg = request_to_mat(*requests, irequest);
  cv::Mat img_resize;
  cv::resize(srcimg, img_resize, Size(srcimg.cols / 1, srcimg.rows / 1));
  std::string results = model_state->detect_model->Ocrdet(img_resize);

  const char* output_buffer = results.c_str();

  TRITONSERVER_MemoryType output_buffer_memory_type = input_buffer_memory_type;
  int64_t output_buffer_memory_type_id = input_buffer_memory_type_id;

  // This backend supports models that batch along the first dimension
  // and those that don't batch. For non-batch models the output shape
  // will be [ 4 ]. For batch models the output shape will be [ -1, 4
  // ] and the backend "responder" utility below will set the
  // appropriate batch dimension value for each response.
  std::vector<int64_t> output_batch_shape;
  bool supports_first_dim_batching;
  RESPOND_ALL_AND_SET_NULL_IF_ERROR(
      responses, request_count,
      model_state->SupportsFirstDimBatching(&supports_first_dim_batching));
  if (supports_first_dim_batching) {
    output_batch_shape.push_back(-1);
  }
  output_batch_shape.push_back(results.size());

  // Because the OUT0 values are concatenated into a single contiguous
  // 'output_buffer', the backend must "scatter" them out to the
  // individual response OUT0 tensors.  The backend utilities provide
  // a "responder" to facilitate this scattering process.

  // The 'responders's ProcessTensor function will copy the portion of
  // 'output_buffer' corresponding to each request's output into the
  // response for that request.

  BackendOutputResponder responder(
      requests, request_count, &responses, model_state->TritonMemoryManager(),
      supports_first_dim_batching, false /* pinned_enabled */,
      nullptr /* stream*/);

  responder.ProcessTensor(
      "OUT0", TRITONSERVER_TYPE_UINT8, output_batch_shape, output_buffer,
      output_buffer_memory_type, output_buffer_memory_type_id);

  // Finalize the responder. If 'true' is returned, the OUT0
  // tensors' data will not be valid until the backend synchronizes
  // the CUDA stream or event that was used when creating the
  // responder. For this backend, GPU is not supported and so no
  // CUDA sync should be needed; so if 'true' is returned simply log
  // an error.
  const bool need_cuda_output_sync = responder.Finalize();
  if (need_cuda_output_sync) {
    LOG_MESSAGE(
        TRITONSERVER_LOG_ERROR,
        "'minimal' backend: unexpected CUDA sync required by responder");
  }

  // Send all the responses that haven't already been sent because of
  // an earlier error.
  for (auto& response : responses) {
    if (response != nullptr) {
      LOG_IF_ERROR(
          TRITONBACKEND_ResponseSend(
              response, TRITONSERVER_RESPONSE_COMPLETE_FINAL, nullptr),
          "failed to send response");
    }
  }

  // Done with the request objects so release them.
  for (uint32_t r = 0; r < request_count; ++r) {
    auto& request = requests[r];
    LOG_IF_ERROR(
        TRITONBACKEND_RequestRelease(request, TRITONSERVER_REQUEST_RELEASE_ALL),
        "failed releasing request");
  }

  return nullptr;  // success
}

}  // extern "C"

}}}  // namespace triton::backend::minimal
