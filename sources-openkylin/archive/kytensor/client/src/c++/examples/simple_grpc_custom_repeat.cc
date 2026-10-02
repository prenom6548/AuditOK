// Copyright (c) 2020-2021, NVIDIA CORPORATION. All rights reserved.
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

#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "grpc_client.h"

namespace tc = triton::client;

using ResultMap =
    std::map<std::string, std::vector<std::shared_ptr<tc::InferResult>>>;
using ResultList = std::vector<std::shared_ptr<tc::InferResult>>;

// Global mutex to synchronize the threads
std::mutex mutex_;
std::condition_variable cv_;

#define FAIL_IF_ERR(X, MSG)                                        \
  {                                                                \
    tc::Error err = (X);                                           \
    if (!err.IsOk()) {                                             \
      std::cerr << "error: " << (MSG) << ": " << err << std::endl; \
      exit(1);                                                     \
    }                                                              \
  }

namespace {

void
Usage(char** argv, const std::string& msg = std::string())
{
  if (!msg.empty()) {
    std::cerr << "error: " << msg << std::endl;
  }

  std::cerr << "Usage: " << argv[0] << " [options]" << std::endl;
  std::cerr << "\t-v" << std::endl;
  std::cerr << "\t-u <URL for inference service and its gRPC port>"
            << std::endl;
  std::cerr
      << "For -H, header must be 'Header:Value'. May be given multiple times."
      << std::endl;
  std::cerr << "\t-r <the number of inference requests>" << std::endl;
  std::cerr << "\t-s <the number of inference response to generate per request>"
            << std::endl;
  std::cerr << "\t-o <data offset>" << std::endl;
  std::cerr << "\t-d <delay time between each response>" << std::endl;
  std::cerr << "\t-w <wait time before releasing the request>" << std::endl;
  exit(1);
}

}  // namespace

int
main_async(int argc, char** argv)
{
  bool verbose = false;
  std::string url("localhost:8001");
  tc::Headers http_headers;
  int request_count = 1;
  int repeat_count = 1;
  int data_offset = 100;
  uint32_t delay_time = 1000;
  uint32_t wait_time = 500;

  // Parse commandline...
  int opt;
  while ((opt = getopt(argc, argv, "vu:H:r:s:o:d:w:")) != -1) {
    switch (opt) {
      case 'v':
        verbose = true;
        break;
      case 'u':
        url = optarg;
        break;
      case 'H': {
        std::string arg = optarg;
        std::string header = arg.substr(0, arg.find(":"));
        http_headers[header] = arg.substr(header.size() + 1);
        break;
      }
      case 'r':
        request_count = std::stoi(optarg);
        break;
      case 's':
        repeat_count = std::stoi(optarg);
        break;
      case 'o':
        data_offset = std::stoi(optarg);
        break;
      case 'd':
        delay_time = std::stoi(optarg);
        break;
      case 'w':
        wait_time = std::stoi(optarg);
        break;
      case '?':
        Usage(argv);
        break;
    }
  }

  tc::Error err;

  // We use the custom "repeat_int32" model which takes 3 inputs and
  // 1 output. For a single request the model will generate 'repeat_count'
  // responses. See is src/backends/backend/examples/repeat.cc.
  std::string model_name = "repeat_int32";
  std::atomic<int32_t> received_response(0);

  // Create a InferenceServerGrpcClient instance to communicate with the
  // server using gRPC protocol.
  std::unique_ptr<tc::InferenceServerGrpcClient> client;
  FAIL_IF_ERR(
      tc::InferenceServerGrpcClient::Create(&client, url, verbose),
      "unable to create grpc client");

  ResultMap result_map;

  // Note that client side statistics should be disabled in case of
  // of decoupled model.
  FAIL_IF_ERR(
      client->StartStream(
          [&](tc::InferResult* result) {
            {
              std::shared_ptr<tc::InferResult> result_ptr(result);
              std::lock_guard<std::mutex> lk(mutex_);
              std::string request_id;
              result->Id(&request_id);
              auto it = result_map.find(request_id);
              if (it == result_map.end()) {
                result_map[request_id] = ResultList();
              }
              result_map[request_id].push_back(result_ptr);
              received_response++;
            }
            cv_.notify_all();
          },
          false /*enable_stats*/, 0 /* stream_timeout */, http_headers),
      "unable to establish a streaming connection to server");

  // Prepare the data for the tensors
  std::vector<int32_t> in_data;
  std::vector<uint32_t> delay_data;
  std::vector<uint32_t> wait_data;
  for (int i = 0; i < repeat_count; i++) {
    in_data.push_back(data_offset + i);
    delay_data.push_back(delay_time);
  }
  wait_data.push_back(wait_time);

  // Initialize the inputs with the data.
  tc::InferInput* in;
  std::vector<int64_t> shape{repeat_count};
  FAIL_IF_ERR(
      tc::InferInput::Create(&in, "IN", shape, "INT32"),
      "unable to create 'IN'");
  std::shared_ptr<tc::InferInput> in_ptr(in);
  FAIL_IF_ERR(in_ptr->Reset(), "unable to reset 'IN'");
  FAIL_IF_ERR(
      in_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&in_data[0]),
          sizeof(int32_t) * repeat_count),
      "unable to set data for 'IN'");

  tc::InferInput* delay;
  FAIL_IF_ERR(
      tc::InferInput::Create(&delay, "DELAY", shape, "UINT32"),
      "unable to create 'DELAY'");
  std::shared_ptr<tc::InferInput> delay_ptr(delay);
  FAIL_IF_ERR(delay_ptr->Reset(), "unable to reset 'DELAY'");
  FAIL_IF_ERR(
      delay_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&delay_data[0]),
          sizeof(uint32_t) * repeat_count),
      "unable to set data for 'DELAY'");

  tc::InferInput* wait;
  shape[0] = 1;
  FAIL_IF_ERR(
      tc::InferInput::Create(&wait, "WAIT", shape, "UINT32"),
      "unable to create 'WAIT'");
  std::shared_ptr<tc::InferInput> wait_ptr(wait);
  FAIL_IF_ERR(wait_ptr->Reset(), "unable to reset 'WAIT'");
  FAIL_IF_ERR(
      wait_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&wait_data[0]), sizeof(uint32_t)),
      "unable to set data for 'WAIT'");

  std::vector<tc::InferInput*> inputs = {
      in_ptr.get(), delay_ptr.get(), wait_ptr.get()};

  tc::InferOptions options(model_name);

  auto start_total = std::chrono::high_resolution_clock::now();

  for (int id = 0; id < request_count; id++) {
    options.request_id_ = std::to_string(id);
    // Send inference request to the inference server.
    FAIL_IF_ERR(
        client->AsyncStreamInfer(options, inputs), "unable to run model");
  }

  // Wait until all callbacks are invoked
  {
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [&]() {
      if (received_response >= (repeat_count * request_count)) {
        return true;
      } else {
        return false;
      }
    });
  }

  auto end_total = std::chrono::high_resolution_clock::now();
  double total_time = std::chrono::duration<double>(end_total - start_total).count();
  std::cout << "Total time: " << total_time << " seconds" << std::endl;

  for (int i = 0; i < request_count; i++) {
    std::string id(std::to_string(i));
    if (repeat_count == 0) {
      auto it = result_map.find(id);
      if (it != result_map.end()) {
        std::cerr << "received unexpected response for request id " << id
                  << std::endl;
        exit(1);
      }
    } else {
      int32_t expected_output = data_offset;
      auto it = result_map.find(id);
      if (it == result_map.end()) {
        std::cerr << "response for request id " << id << " not received"
                  << std::endl;
        exit(1);
      }
      if (it->second.size() != (uint32_t)repeat_count) {
        std::cerr << "expected " << repeat_count << " many responses, got "
                  << it->second.size() << std::endl;
        exit(1);
      }
      for (auto this_result : it->second) {
        int32_t* output_data;
        size_t output_byte_size;
        FAIL_IF_ERR(
            this_result->RawData(
                "OUT", (const uint8_t**)&output_data, &output_byte_size),
            "unable to get result data for 'OUT'");
        if (output_byte_size != 4) {
          std::cerr << "error: received incorrect byte size for 'OUT': "
                    << output_byte_size << std::endl;
          exit(1);
        }
        if (*output_data != expected_output) {
          std::cerr << "error: incorrect result returned, expected "
                    << expected_output << ", got " << *output_data << std::endl;
          exit(1);
        }
        expected_output++;
      }
    }
  }

  return 0;
}

int
main(int argc, char** argv)
{
  bool verbose = false;
  std::string url("localhost:8001");
  tc::Headers http_headers;
  grpc_compression_algorithm compression_algorithm =
      grpc_compression_algorithm::GRPC_COMPRESS_NONE;
  int request_count = 1;
  int repeat_count = 1;
  int data_offset = 100;
  uint32_t delay_time = 1000;
  uint32_t wait_time = 500;

  // Parse commandline...
  int opt;
  while ((opt = getopt(argc, argv, "vu:H:r:s:o:d:w:")) != -1) {
    switch (opt) {
      case 'v':
        verbose = true;
        break;
      case 'u':
        url = optarg;
        break;
      case 'H': {
        std::string arg = optarg;
        std::string header = arg.substr(0, arg.find(":"));
        http_headers[header] = arg.substr(header.size() + 1);
        break;
      }
      case 'r':
        request_count = std::stoi(optarg);
        break;
      case 's':
        repeat_count = std::stoi(optarg);
        break;
      case 'o':
        data_offset = std::stoi(optarg);
        break;
      case 'd':
        delay_time = std::stoi(optarg);
        break;
      case 'w':
        wait_time = std::stoi(optarg);
        break;
      case '?':
        Usage(argv);
        break;
    }
  }

  tc::Error err;

  // We use the custom "repeat_int32" model which takes 3 inputs and
  // 1 output. For a single request the model will generate 'repeat_count'
  // responses. See is src/backends/backend/examples/repeat.cc.
  std::string model_name = "repeat_int32";
  std::atomic<int32_t> received_response(0);

  // Create a InferenceServerGrpcClient instance to communicate with the
  // server using gRPC protocol.
  std::unique_ptr<tc::InferenceServerGrpcClient> client;
  FAIL_IF_ERR(
      tc::InferenceServerGrpcClient::Create(&client, url, verbose),
      "unable to create grpc client");

  ResultMap result_map;

  // Prepare the data for the tensors
  std::vector<int32_t> in_data;
  std::vector<uint32_t> delay_data;
  std::vector<uint32_t> wait_data;
  for (int i = 0; i < repeat_count; i++) {
    in_data.push_back(data_offset + i);
    delay_data.push_back(delay_time);
  }
  wait_data.push_back(wait_time);

  // Initialize the inputs with the data.
  tc::InferInput* in;
  std::vector<int64_t> shape{repeat_count};
  FAIL_IF_ERR(
      tc::InferInput::Create(&in, "IN", shape, "INT32"),
      "unable to create 'IN'");
  std::shared_ptr<tc::InferInput> in_ptr(in);
  FAIL_IF_ERR(in_ptr->Reset(), "unable to reset 'IN'");
  FAIL_IF_ERR(
      in_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&in_data[0]),
          sizeof(int32_t) * repeat_count),
      "unable to set data for 'IN'");

  tc::InferInput* delay;
  FAIL_IF_ERR(
      tc::InferInput::Create(&delay, "DELAY", shape, "UINT32"),
      "unable to create 'DELAY'");
  std::shared_ptr<tc::InferInput> delay_ptr(delay);
  FAIL_IF_ERR(delay_ptr->Reset(), "unable to reset 'DELAY'");
  FAIL_IF_ERR(
      delay_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&delay_data[0]),
          sizeof(uint32_t) * repeat_count),
      "unable to set data for 'DELAY'");

  tc::InferInput* wait;
  shape[0] = 1;
  FAIL_IF_ERR(
      tc::InferInput::Create(&wait, "WAIT", shape, "UINT32"),
      "unable to create 'WAIT'");
  std::shared_ptr<tc::InferInput> wait_ptr(wait);
  FAIL_IF_ERR(wait_ptr->Reset(), "unable to reset 'WAIT'");
  FAIL_IF_ERR(
      wait_ptr->AppendRaw(
          reinterpret_cast<uint8_t*>(&wait_data[0]), sizeof(uint32_t)),
      "unable to set data for 'WAIT'");

  // Generate the outputs to be requested.
    tc::InferRequestedOutput* output0;

    FAIL_IF_ERR(
        tc::InferRequestedOutput::Create(&output0, "OUT"),
        "unable to get 'OUT'");
    std::shared_ptr<tc::InferRequestedOutput> output0_ptr;
    output0_ptr.reset(output0);

  std::vector<tc::InferInput*> inputs = {
      in_ptr.get(), delay_ptr.get(), wait_ptr.get()};
  std::vector<const tc::InferRequestedOutput*> outputs = {
      output0_ptr.get()};

  tc::InferOptions options(model_name);

  //预热连接
  tc::InferResult* results;
  FAIL_IF_ERR(
        client->Infer(
            &results, options, inputs, outputs, http_headers,
            compression_algorithm),
        "unable to run model");

  std::vector<double> latencies;
  latencies.reserve(request_count);
  auto start_total = std::chrono::high_resolution_clock::now();

  for (int id = 0; id < request_count; id++) {
    auto start = std::chrono::high_resolution_clock::now();

    options.request_id_ = std::to_string(id);
    // Send inference request to the inference server.
    tc::InferResult* results;
    FAIL_IF_ERR(
        client->Infer(
            &results, options, inputs, outputs, http_headers,
            compression_algorithm),
        "unable to run model");

      auto end = std::chrono::high_resolution_clock::now();
      double latency = std::chrono::duration<double, std::milli>(end - start).count();
      latencies.push_back(latency);
  }

  auto end_total = std::chrono::high_resolution_clock::now();
  double total_time = std::chrono::duration<double>(end_total - start_total).count();
  // 计算统计信息
  double avg_latency = 0;
  double min_latency = INFINITY, max_latency = 0;
  for (double lat : latencies) {
      avg_latency += lat;
      if (lat < min_latency) min_latency = lat;
      if (lat > max_latency) max_latency = lat;
  }
  avg_latency /= request_count;

  double throughput = request_count / total_time;

  std::cout << "Latency (ms):\n"
            << "  Avg: " << avg_latency << "\n"
            << "  Min: " << min_latency << "\n"
            << "  Max: " << max_latency << "\n"
            << "Throughput: " << throughput << " requests/sec" << std::endl;

  return 0;
}
