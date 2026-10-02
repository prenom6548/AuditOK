## 推理服务层API单元测试

libtriton-client-unittest用于测试推理服务API接口，目前完成主要接口测试，接口说明参考《统一推理服务层API》。

## 单元测试情况
### 推理接口
| 推理接口                         | GRPC | HTTP | 测试用例                |
|------------------------------|------|------|---------------------|
| Create                       | 支持   | 支持   | client_timeout_test |
| IsServerLive                 | 支持   | 暂不支持 | client_timeout_test |
| IsServerReady                | 支持   | 暂不支持 | client_timeout_test |
| IsModelReady                 | 支持   | 暂不支持 | client_timeout_test |
| ServerMetadata               | 支持   | 暂不支持 | client_timeout_test |
| ModelMetadata                | 支持   | 暂不支持 | client_timeout_test |
| ModelConfig                  | 支持   | 暂不支持 | client_timeout_test |
| UnloadModel                  | 支持   | 暂不支持 | client_timeout_test |
| ModelInferenceStatistics     | 支持   | 暂不支持 | client_timeout_test |
| SystemSharedMemoryStatus     | 支持   | 暂不支持 | client_timeout_test |
| RegisterSystemSharedMemory   | 暂不支持 | 暂不支持 |                     |
| UnregisterSystemSharedMemory | 支持   | 暂不支持 | client_timeout_test |
| Infer                        | 支持   | 支持   | client_timeout_test |
| AsyncInfer                   | 支持   | 支持   | client_timeout_test |
| InferMulti                   | 支持   | 支持   | cc_client_test      |
| AsyncInferMulti              | 支持   | 支持   | cc_client_test      |
| StartStream                  | 支持   | 暂不支持 | cc_client_test      |
| StopStream                   | 支持   | 暂不支持 | cc_client_test      |
| AsyncStreamInfer             | 支持   | 支持   | cc_client_test      |

### 推理输入接口
| 推理输入接口           | GRPC | HTTP | 测试用例           |
|------------------|------|------|----------------|
| Create           | 支持   | 支持   | cc_client_test |
| Name             | 暂不支持 | 暂不支持 |                |
| Datatype         | 暂不支持 | 暂不支持 |                |
| Shape            | 暂不支持 | 暂不支持 |                |
| SetShape         | 暂不支持 | 暂不支持 |                |
| Reset            | 暂不支持 | 暂不支持 |                |
| AppendRaw        | 支持   | 支持   | cc_client_test |
| SetSharedMemory  | 暂不支持 | 暂不支持 |                |
| IsSharedMemory   | 暂不支持 | 暂不支持 |                |
| SharedMemoryInfo | 暂不支持 | 暂不支持 |                |
| RawData          | 暂不支持 | 暂不支持 |                |
| ByteSize         | 暂不支持 | 暂不支持 |                |
| BinaryData       | 暂不支持 | 暂不支持 |                |
| SetBinaryData    | 暂不支持 | 暂不支持 |                |

### 推理输出接口
| 推理输出接口           | GRPC | HTTP | 测试用例           |
|------------------|------|------|----------------|
| Create           | 支持   | 支持   | cc_client_test |
| Name             | 暂不支持 | 暂不支持 |                |
| SetSharedMemory  | 暂不支持 | 暂不支持 |                |
| IsSharedMemory   | 暂不支持 | 暂不支持 |                |
| SharedMemoryInfo | 暂不支持 | 暂不支持 |                |
| BinaryData       | 暂不支持 | 暂不支持 |                |
| SetBinaryData    | 暂不支持 | 暂不支持 |                |

### 推理输出结果接口

| 推理输出结果接口        | GRPC | HTTP | 测试用例                |
|-----------------|------|------|---------------------|
| ModelName       | 暂不支持 | 暂不支持 |                     |
| ModelVersion    | 暂不支持 | 暂不支持 |                     |
| Id              | 暂不支持 | 暂不支持 |                     |
| Shape           | 暂不支持 | 暂不支持 |                     |
| Datatype        | 支持   | 支持   | client_timeout_test |
| RawData         | 支持   | 支持   | cc_client_test      |
| IsFinalResponse | 暂不支持 | 暂不支持 |                     |
| IsNullResponse  | 暂不支持 | 暂不支持 |                     |
| StringData      | 暂不支持 | 暂不支持 |                     |
| RequestStatus   | 支持   | 支持   | cc_client_test      |


## 安装

```
## 安装测试包
sudo dpkg -i libtriton-client-unittest_2.49.0_amd64.deb

## 进入安装目录
cd /opt/appdata/kylin-ai/libtriton-client-unittest/
```

## 测试使用说明

### cc_client_test测试

```
## 首先杀掉系统已有的推理服务，再起推理服务
killall tritonserver
tritonserver --model-repository=/opt/appdata/kylin-ai/libtriton-client-unittest/unittest_models

## 然后运行测试程序
./cc_client_test
```
测试结果如下：
```
[ RUN      ] HTTP/ClientTest/0.AsyncInferMulti
[       OK ] HTTP/ClientTest/0.AsyncInferMulti (1 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiDifferentOutputs
[       OK ] HTTP/ClientTest/0.AsyncInferMultiDifferentOutputs (1 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiDifferentOptions
[       OK ] HTTP/ClientTest/0.AsyncInferMultiDifferentOptions (1 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiOneOption
[       OK ] HTTP/ClientTest/0.AsyncInferMultiOneOption (1 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiOneOutput
[       OK ] HTTP/ClientTest/0.AsyncInferMultiOneOutput (0 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiNoOutput
[       OK ] HTTP/ClientTest/0.AsyncInferMultiNoOutput (1 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiMismatchOptions
[       OK ] HTTP/ClientTest/0.AsyncInferMultiMismatchOptions (0 ms)
[ RUN      ] HTTP/ClientTest/0.AsyncInferMultiMismatchOutputs
[       OK ] HTTP/ClientTest/0.AsyncInferMultiMismatchOutputs (0 ms)
[----------] 16 tests from HTTP/ClientTest/0 (14 ms total)

[----------] Global test environment tear-down
[==========] 35 tests from 3 test suites ran. (28 ms total)
[  PASSED  ] 35 tests.
```
所有测试都通过。

### client_timeout_test测试

```
## 首先杀掉系统已有的推理服务，再起推理服务
killall tritonserver
tritonserver --model-repository=/opt/appdata/kylin-ai/libtriton-client-unittest/unittest_models --model-control-mode=explicit  --load-model=custom_identity_int32

## 然后运行测试程序
./client_timeout_test -i grpc -p
```
测试结果如下：
```
testing other apis
OK: Success on IsServerLive:
OK: Success on IsServerReady:
OK: Success on IsModelReady:
OK: Success on ServerMetadata:
OK: Success on ModelMetadata:
OK: Success on ModelConfig:
OK: Success on ModelRepositoryIndex:
OK: Success on ModelInferenceStatistics:
OK: Success on LoadModel:
OK: Success on UnloadModel:
OK: Success on SystemSharedMemoryStatus:
error: Failed on RegisterSystemSharedMemory: Unable to open shared memory region: ''
OK: Success on UnregisterSystemSharedMemory:
error count: 1 which is not 0

```
有一个错误，但RegisterSystemSharedMemory暂不支持测试，可以忽略。

