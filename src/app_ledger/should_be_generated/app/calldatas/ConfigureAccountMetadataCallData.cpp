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

#include "../../domain/commands/ConfigureAccountMetadataCommand.h"

namespace gringofts {
namespace ledger {

template<>
void CallData<protos::ConfigureAccountMetadata::Request, protos::ConfigureAccountMetadata::Response>::proceed() {
  if (mCallStatus == CallStatus::CREATE) {
    mCallStatus = CallStatus::PROCESS;
    mService->RequestConfigureAccountMetadata(&mContext, &mRequest, &mResponder,
                                              mCompletionQueue, mCompletionQueue, this);
  } else if (mCallStatus == CallStatus::PROCESS) {
    new ConfigureAccountMetadataCallData(mService,
                                         mCompletionQueue,
                                         mCommandQueue);

    auto createdTimeInNanos = TimeUtil::currentTimeInNanos();
    auto command = std::make_shared<ConfigureAccountMetadataCommand>(createdTimeInNanos,
                                                                     mRequest);
    command->setRequestHandle(this);
    command->setCreatorId(app::AppInfo::subsystemId());
    command->setGroupId(app::AppInfo::groupId());
    command->setGroupVersion(app::AppInfo::groupVersion());
    const std::string verifyResult = command->verifyCommand();
    if (verifyResult != Command::kVerifiedSuccess) {
      SPDLOG_WARN("Request can not pass validation due to Error: {} Request: {}",
                  verifyResult, mRequest.DebugString());
      fillResultAndReply(HttpCode::BAD_REQUEST, verifyResult, std::nullopt);
      return;
    }
    // if the command is verified
    try {
      mCommandQueue.enqueue(command);
    }
    catch (const QueueStoppedException &e) {
      SPDLOG_WARN(e.what());
      fillResultAndReply(HttpCode::SERVICE_UNAVAILABLE, std::string(e.what()), std::nullopt);
    }
  } else {
    GPR_ASSERT(mCallStatus == CallStatus::FINISH);
    delete this;
  }
}

}  // namespace ledger
}  // namespace gringofts
