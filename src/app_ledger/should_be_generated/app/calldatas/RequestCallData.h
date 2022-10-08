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

#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_APP_CALLDATAS_REQUESTCALLDATA_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_APP_CALLDATAS_REQUESTCALLDATA_H_

#include <grpcpp/grpcpp.h>

#include "../../../../app_util/AppInfo.h"
#include "../../../../infra/grpc/RequestHandle.h"
#include "../../../../infra/util/HttpCode.h"
#include "../../domain/common_types.h"

using ::grpc::Server;
using ::grpc::ServerAsyncResponseWriter;
using ::grpc::ServerContext;
using ::grpc::ServerCompletionQueue;

namespace gringofts {
namespace ledger {

//////////////////////////// CallData ////////////////////////////

struct CallDataBase : public RequestHandle {
  CallDataBase(protos::LedgerService::AsyncService *service,
               grpc::ServerCompletionQueue *completionQueue)
      : mService(service), mCompletionQueue(completionQueue) {}

  virtual ~CallDataBase() = default;

  virtual void proceed() = 0;
  virtual void failOver() = 0;

  protos::LedgerService::AsyncService *mService;
  grpc::ServerCompletionQueue *mCompletionQueue;
};

template<typename RequestType, typename ResponseType>
struct CallData : public CallDataBase {
  CallData(protos::LedgerService::AsyncService *service,
           grpc::ServerCompletionQueue *completionQueue,
           BlockingQueue<std::shared_ptr<Command>> &commandQueue)  // NOLINT[runtime/references]
      : CallDataBase(service, completionQueue),
        mCommandQueue(commandQueue),
        mResponder(&mContext) {
    /// Attention, call virtual function in Ctor/Dtor is not recommended.
    /// However, we do not rely on polymorphism here.
    proceed();
  }

  void proceed() override { assert(0); }

  void failOver() override {
    SPDLOG_WARN("Fail over for CallData");
    new CallData<RequestType, ResponseType>(mService, mCompletionQueue, mCommandQueue);
    delete this;
  }

  void fillResultAndReply(uint32_t code,
                          const std::string &message,
                          std::optional<uint64_t> leaderId) override {
    mResponse.set_code(code);
    mResponse.set_message(message);
    if (code == HttpCode::MOVED_PERMANENTLY && leaderId) {
      mResponse.set_reserved(std::to_string(*leaderId));
    }

    mCallStatus = CallStatus::FINISH;
    mResponder.Finish(mResponse, grpc::Status::OK, this);
  }

  BlockingQueue<std::shared_ptr<Command>> &mCommandQueue;
  RequestType mRequest;
  ResponseType mResponse;

  enum class CallStatus { CREATE, PROCESS, FINISH };
  CallStatus mCallStatus = CallStatus::CREATE;

  grpc::ServerContext mContext;
  grpc::ServerAsyncResponseWriter<ResponseType> mResponder;
};

using ConfigureAccountMetadataCallData = CallData<protos::ConfigureAccountMetadata::Request,
                                       protos::ConfigureAccountMetadata::Response>;
using CreateAccountCallData = CallData<protos::CreateAccount::Request,
                                       protos::CreateAccount::Response>;

//////////////////////////// CallData ////////////////////////////

}  /// namespace ledger
}  /// namespace gringofts

#include "ConfigureAccountMetadataCallData.cpp"
#include "CreateAccountCallData.cpp"

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_APP_CALLDATAS_REQUESTCALLDATA_H_
