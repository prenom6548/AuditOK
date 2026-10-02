// Copyright (C) 2019-2024 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "milvus_service_impl.h"
#include <iostream>
#include "log/Log.h"
#include "string_util.hpp"
#include <string>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include <unistd.h>

int
main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    if (!(argc == 1 || argc == 2 || argc == 3)) {
        return -1;
    }

    std::string address = "unix:/tmp/kylin-ai-vector-engine-" +
                          std::to_string(geteuid()) + ".sock";
    std::string log_level = "ERROR";
    if (argc >= 2) {
        log_level = argv[1];
    }
    if (log_level == "INFO") {
        google::SetStderrLogging(google::INFO);
    } else {
        google::SetStderrLogging(google::ERROR);
    }

    std::string conf_file =
        (argc >= 3) ? argv[2]
                    : "/usr/share/kylin-ai-vector-engine/vector_engine.json";

    ::milvus::local::MilvusServiceImpl service(conf_file);
    ::grpc::ServerBuilder builder;
    builder.SetMaxReceiveMessageSize(268435456);
    builder.SetMaxSendMessageSize(536870912);
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<::grpc::Server> server(builder.BuildAndStart());
    LOG_INFO("Start milvus-local success...");

    server->Wait();
    return 0;
}
