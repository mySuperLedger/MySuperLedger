/************************************************************************
Copyright 2022 MySuperLedger
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    https://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_APP_REQUESTRECEIVER_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_APP_REQUESTRECEIVER_H_

#include <INIReader.h>
#include <grpcpp/grpcpp.h>

#include "../../../app_util/Service.h"
#include "../../../infra/es/Command.h"
#include "../../../infra/util/TlsUtil.h"
#include "../../generated/grpc/ledger.grpc.pb.h"

using ::grpc::ServerCompletionQueue;

namespace gringofts {
namespace ledger {

class RequestReceiver final : public app::Service {
 public:
  // disallow copy ctor and copy assignment
  RequestReceiver(const RequestReceiver &) = delete;
  RequestReceiver &operator=(const RequestReceiver &) = delete;

  // disallow move ctor and move assignment
  RequestReceiver(RequestReceiver &&) = delete;
  RequestReceiver &operator=(RequestReceiver &&) = delete;

  ~RequestReceiver() override = default;

  explicit RequestReceiver(const INIReader &reader,
                           uint32_t port,
                           BlockingQueue<std::shared_ptr<Command>> &commandQueue);  // NOLINT(runtime/references)

  void startListen();

  void start() override;

  void stop() override;

 private:
  void handleRpcs(uint64_t i);

  uint64_t mPreSpawn = 1;
  uint64_t mConcurrency = 1;
  std::string mIpPort;
  std::optional<TlsConf> mTlsConfOpt;
  BlockingQueue<std::shared_ptr<Command>> &mCommandQueue;
  std::unique_ptr<::grpc::Server> mServer;
  std::atomic<bool> mIsShutdown = false;
  std::vector<std::unique_ptr<ServerCompletionQueue>> mCompletionQueues;
  std::vector<std::thread> mRcvThreads;
  protos::LedgerService::AsyncService mService;
};

}  /// namespace ledger
}  /// namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_APP_REQUESTRECEIVER_H_
