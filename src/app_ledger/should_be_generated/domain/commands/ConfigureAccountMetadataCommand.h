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

#ifndef SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_COMMANDS_CONFIGUREACCOUNTMETADATACOMMAND_H_
#define SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_COMMANDS_CONFIGUREACCOUNTMETADATACOMMAND_H_

#include "../../../../infra/es/Command.h"
#include "../../../../infra/es/ProcessCommandStateMachine.h"
#include "../AccountMetadata.h"

namespace gringofts {
namespace ledger {

class ConfigureAccountMetadataCommand : public Command {
 public:
  ConfigureAccountMetadataCommand(TimestampInNanos, const protos::ConfigureAccountMetadata::Request &origRequest);

  ConfigureAccountMetadataCommand(TimestampInNanos, const std::string &commandStr);

  std::string encodeToString() const override {
    return mOrigRequest.SerializeAsString();
  }

  void decodeFromString(std::string_view encodedString) override {
    mOrigRequest.ParseFromString(std::string(encodedString));
  }

  std::string verifyCommand() const override {
    const auto &metadata = mOrigRequest.metadata();
    if (metadata.low_inclusive() >= metadata.high_inclusive()) {
      return "low inclusive is larger than high inclusive";
    }
    return kVerifiedSuccess;
  }

  const std::optional<AccountMetadata> &accountMetadataOpt() const {
    return mAccountMetadataOpt;
  }

 private:
  void tryInitAccountMetadata();
  void onPersisted(const std::string &message) override;
  void onPersistFailed(uint32_t code, const std::string &errorMessage, std::optional<uint64_t> reserved) override;

  protos::ConfigureAccountMetadata::Request mOrigRequest;
  std::optional<AccountMetadata> mAccountMetadataOpt;
};

}  ///  namespace ledger
}  ///  namespace gringofts

#endif  // SRC_APP_LEDGER_SHOULD_BE_GENERATED_DOMAIN_COMMANDS_CONFIGUREACCOUNTMETADATACOMMAND_H_
