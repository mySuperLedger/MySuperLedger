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

#include "RecordJournalEntryCommand.h"

#include "../common_types.h"

namespace gringofts {
namespace ledger {

RecordJournalEntryCommand::RecordJournalEntryCommand(TimestampInNanos createdTimeInNanos,
                                                     const protos::RecordJournalEntry::Request &origRequest)
    : Command(RECORD_JOURNAL_ENTRY_COMMAND, createdTimeInNanos), mOrigRequest(origRequest) {
  tryInitJournalEntry();
}

RecordJournalEntryCommand::RecordJournalEntryCommand(TimestampInNanos createdTimeInNanos, const std::string &commandStr)
    : Command(RECORD_JOURNAL_ENTRY_COMMAND, createdTimeInNanos) {
  decodeFromString(commandStr);
  tryInitJournalEntry();
}

void RecordJournalEntryCommand::tryInitJournalEntry() {
  if (kVerifiedSuccess == verifyCommand()) {
    JournalEntry journalEntry;
    journalEntry.initWith(mOrigRequest.journal_entry());
    mJournalEntryOpt = std::make_optional<JournalEntry>(journalEntry);
  }
}

void RecordJournalEntryCommand::onPersisted(const std::string &message) {
  auto *callData = getRequestHandle();
  if (callData == nullptr) return;

  callData->fillResultAndReply(200, message, std::nullopt);
}

void RecordJournalEntryCommand::onPersistFailed(
    uint32_t code,
    const std::string &errorMessage,
    std::optional<uint64_t> reserved) {
  auto *callData = getRequestHandle();
  if (callData == nullptr) return;

  callData->fillResultAndReply(code, errorMessage, reserved);
}

}  ///  namespace ledger
}  ///  namespace gringofts
