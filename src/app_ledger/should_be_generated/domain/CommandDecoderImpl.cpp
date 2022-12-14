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

#include "CommandDecoderImpl.h"

#include <spdlog/spdlog.h>

#include "commands/ConfigureAccountMetadataCommand.h"
#include "commands/CreateAccountCommand.h"
#include "commands/RecordJournalEntryCommand.h"
#include "common_types.h"

namespace gringofts {
namespace ledger {

std::unique_ptr<Command> CommandDecoderImpl::decodeCommandFromString(
    const CommandMetaData &metaData, std::string_view payload) const {
  std::unique_ptr<Command> command;
  switch (metaData.getType()) {
    case CREATE_ACCOUNT_COMMAND: {
      command = std::make_unique<CreateAccountCommand>(
          metaData.getCreatedTimeInNanos(), std::string(payload));
      break;
    }
    case CONFIGURE_ACCOUNT_METADATA_COMMAND: {
      command = std::make_unique<ConfigureAccountMetadataCommand>(
          metaData.getCreatedTimeInNanos(), std::string(payload));
      break;
    }
    case RECORD_JOURNAL_ENTRY_COMMAND: {
      command = std::make_unique<RecordJournalEntryCommand>(
          metaData.getCreatedTimeInNanos(), std::string(payload));
      break;
    }
    default:SPDLOG_INFO("Unknown command type: {}", metaData.getType());
      return nullptr;
  }
  command->setPartialMetaData(metaData);
  return std::move(command);
}

}  /// namespace ledger
}  /// namespace gringofts
