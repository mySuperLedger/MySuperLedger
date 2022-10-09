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

#include "ConfigureAccountMetadataCommand.h"

#include "../common_types.h"

namespace gringofts {
namespace ledger {

ConfigureAccountMetadataCommand::ConfigureAccountMetadataCommand(
    TimestampInNanos createdTimeInNanos,
    const protos::ConfigureAccountMetadata::Request &origRequest)
    : Command(CONFIGURE_ACCOUNT_METADATA_COMMAND, createdTimeInNanos), mOrigRequest(origRequest) {
  tryInitAccountMetadata();
}

ConfigureAccountMetadataCommand::ConfigureAccountMetadataCommand(TimestampInNanos createdTimeInNanos,
                                                                 const std::string &commandStr)
    : Command(CONFIGURE_ACCOUNT_METADATA_COMMAND, createdTimeInNanos) {
  decodeFromString(commandStr);
  tryInitAccountMetadata();
}

void ConfigureAccountMetadataCommand::tryInitAccountMetadata() {
  if (kVerifiedSuccess == verifyCommand()) {
    AccountMetadata accountMetadata;
    accountMetadata.initWith(mOrigRequest.metadata());
    mAccountMetadataOpt = std::make_optional<AccountMetadata>(accountMetadata);
  }
}

void ConfigureAccountMetadataCommand::onPersisted(const std::string &message) {
  auto *callData = getRequestHandle();
  if (callData == nullptr) return;

  callData->fillResultAndReply(200, message, std::nullopt);
}

void ConfigureAccountMetadataCommand::onPersistFailed(uint32_t code,
                                                      const std::string &errorMessage,
                                                      std::optional<uint64_t> reserved) {
  auto *callData = getRequestHandle();
  if (callData == nullptr) return;

  callData->fillResultAndReply(code, errorMessage, reserved);
}

}  ///  namespace ledger
}  ///  namespace gringofts
